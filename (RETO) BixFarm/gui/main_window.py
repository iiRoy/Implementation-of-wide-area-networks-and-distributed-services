import shutil
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path

from PyQt6.QtCore import QSize, Qt, QEvent, QProcess, QTimer
from PyQt6.QtGui import QIcon, QPixmap
from PyQt6.QtWidgets import (
    QFileDialog,
    QListWidgetItem,
    QMainWindow,
    QMessageBox,
    QTableWidgetItem,
)
from ui.ui_pixfarm import Ui_MainWindow

from gui.dialog_colores import DialogColores
from gui.dialog_seleccion_imagenes import DialogSeleccionImagenes
from gui.dialog_progreso import DialogProgreso

MAX_IMAGES = 600

@dataclass(frozen=True)
class Rule:
    use_r: bool
    use_g: bool
    use_b: bool
    use_gray: bool
    effect: str  # "inv" o "des"
    value: str  # texto del giro o kernel como string


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        self.project_root = Path(__file__).resolve().parent.parent
        self.default_output_dir = self.project_root / "Resultados"
        self.default_output_dir.mkdir(parents=True, exist_ok=True)

        self.rules: list[Rule] = []
        self.loaded_images: list[str] = []
        self.selected_images: list[str] = []

        self.current_color_selection = {
            "r": True,
            "g": True,
            "b": True,
            "gray": False,
        }

        self.editing_rule_row: int | None = None

        self.mpi_process = None
        self.progress_dialog = None
        self.total_jobs = 0
        self.completed_jobs = 0
        self.progress_start_time = 0.0
        self.mpi_stdout_buffer = ""
        self.mpi_stderr_buffer = ""
        self.current_log_file = None

        self.progress_timer = None
        self.last_progress_line = ""

        self.cancel_requested = False
        self.current_cmd = []
        self.current_job = None

        self._configurar_ui()
        self._conectar_senales()
        self._actualizar_estado_imagenes()
        self._actualizar_preview_nombre()
        self._actualizar_label_imagenes_destino()
        
        self.local_jobs = []
        self.local_job_index = 0
        self.local_running = False

    def _configurar_ui(self):

        self.setAcceptDrops(True)
        self.ui.listImagenes.setAcceptDrops(True)
        self.ui.listImagenes.installEventFilter(self)

        self.ui.txtTiempoEjecucion.setReadOnly(True)
        self.ui.txtRutaGuardado.setText(str(self.default_output_dir))
        self.ui.txtRutaGuardado.setReadOnly(True)
        self.ui.txtRutaGuardado.setCursor(Qt.CursorShape.PointingHandCursor)

        # Abrir selector de carpeta al hacer click en la ruta
        self.ui.txtRutaGuardado.mousePressEvent = self._abrir_selector_ruta_guardado

        self.ui.txtNumberThreads.setMinimum(2)
        self.ui.txtNumberThreads.setMaximum(47)
        self.ui.txtNumberThreads.setValue(47)
        self.ui.txtNumberThreads.setSingleStep(2)

        self.ui.rulesTable.setColumnCount(4)
        self.ui.rulesTable.setHorizontalHeaderLabels(
            ["Color", "Efecto", "Valor", "Resumen"]
        )
        self.ui.rulesTable.verticalHeader().setVisible(False)
        self.ui.rulesTable.setSelectionBehavior(
            self.ui.rulesTable.SelectionBehavior.SelectRows
        )
        self.ui.rulesTable.setSelectionMode(
            self.ui.rulesTable.SelectionMode.SingleSelection
        )
        self.ui.rulesTable.setAlternatingRowColors(True)
        self.ui.rulesTable.horizontalHeader().setStretchLastSection(True)

        self._actualizar_stacked_valor()

    def _conectar_senales(self):
        self.ui.btnCargarImagenes.clicked.connect(self.cargar_imagenes)
        self.ui.btnEliminarImagen.clicked.connect(self.eliminar_imagenes_seleccionadas)
        self.ui.btnLimpiarLista.clicked.connect(self.limpiar_lista_imagenes)

        self.ui.btnSeleccionColor.clicked.connect(self.abrir_dialogo_colores)
        self.ui.btnSeleccionarImagenesDestino.clicked.connect(
            self.abrir_dialogo_imagenes
        )

        self.ui.btnAgregarRegla.clicked.connect(self.agregar_regla)
        self.ui.btnEditarRegla.clicked.connect(self.editar_regla)
        self.ui.btnEliminarRegla.clicked.connect(self.eliminar_regla)

        self.ui.btnEjecutarPrograma.clicked.connect(self.ejecutar_trabajos)

        self.ui.cmbEfecto.currentIndexChanged.connect(self._actualizar_stacked_valor)
        self.ui.cmbEfecto.currentIndexChanged.connect(self._actualizar_preview_nombre)
        self.ui.cmbGiro.currentTextChanged.connect(self._actualizar_preview_nombre)
        self.ui.spinKernel.valueChanged.connect(self._actualizar_preview_nombre)

        self.ui.rulesTable.itemSelectionChanged.connect(
            self._cargar_regla_seleccionada_en_editor
        )

    def count_slots_from_machinefile(self, machinefile: Path) -> int:
        total = 0

        for line in machinefile.read_text().splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue

            slots = 1
            for part in line.split():
                if part.startswith("slots="):
                    slots = int(part.split("=", 1)[1])

            total += slots

        return total

    def build_local_jobs(self, output_dir: Path):
        jobs = []

        for image_path in self.selected_images:
            base_name = Path(image_path).stem

            for rule in self.rules:
                output_name = self.build_output_name(base_name, rule)

                jobs.append({
                    "image_path": str(image_path),
                    "output_dir": output_dir,
                    "output_name": output_name,
                    "rule": rule,
                })

        return jobs

    def count_total_jobs(self) -> int:
        return len(self.selected_images) * len(self.rules)
    
    def build_jobs_file(self, jobs_path: Path, output_dir: Path):
        with jobs_path.open("w", encoding="utf-8") as f:
            for image_path in self.selected_images:
                base_name = Path(image_path).stem

                for rule in self.rules:
                    output_name = self.build_output_name(base_name, rule)
                    value = self.effect_value_to_c(rule)

                    f.write("\t".join([
                        str(image_path),
                        str(output_dir),
                        output_name,
                        rule.effect,
                        str(value),
                        str(int(rule.use_r)),
                        str(int(rule.use_g)),
                        str(int(rule.use_b)),
                        str(int(rule.use_gray)),
                    ]) + "\n")

    # -----------------------------
    # CARGA Y GESTIÓN DE IMÁGENES
    # -----------------------------
    def cargar_imagenes(self):
        files, _ = QFileDialog.getOpenFileNames(
            self,
            "Seleccionar imágenes BMP",
            str(self.project_root),
            "Imágenes BMP (*.bmp);;Todos los archivos (*)",
        )

        self.agregar_imagenes_desde_paths(files)

    def _agregar_item_imagen(self, image_path: str):
        path = Path(image_path)
        item = QListWidgetItem(path.name)
        item.setData(Qt.ItemDataRole.UserRole, image_path)
        item.setToolTip(image_path)

        pixmap = QPixmap(image_path)
        if not pixmap.isNull():
            thumb = pixmap.scaled(
                QSize(120, 120),
                Qt.AspectRatioMode.KeepAspectRatio,
                Qt.TransformationMode.SmoothTransformation,
            )
            item.setIcon(QIcon(thumb))

        self.ui.listImagenes.addItem(item)

    def eliminar_imagenes_seleccionadas(self):
        items = self.ui.listImagenes.selectedItems()
        if not items:
            QMessageBox.information(
                self, "Aviso", "Selecciona una o más imágenes para eliminarlas."
            )
            return

        paths_to_remove = {item.data(Qt.ItemDataRole.UserRole) for item in items}

        for item in items:
            row = self.ui.listImagenes.row(item)
            self.ui.listImagenes.takeItem(row)

        self.loaded_images = [p for p in self.loaded_images if p not in paths_to_remove]
        self.selected_images = [
            p for p in self.selected_images if p not in paths_to_remove
        ]

        self._actualizar_estado_imagenes()
        self._actualizar_label_imagenes_destino()
        self._actualizar_preview_nombre()

    def limpiar_lista_imagenes(self):
        if not self.loaded_images:
            return

        self.ui.listImagenes.clear()
        self.loaded_images.clear()
        self.selected_images.clear()

        self._actualizar_estado_imagenes()
        self._actualizar_label_imagenes_destino()
        self._actualizar_preview_nombre()

    def _actualizar_estado_imagenes(self):
        self.ui.lblCantidadImagenes.setText(f"{len(self.loaded_images)} imágenes")

    # -----------------------------
    # DIÁLOGO DE COLORES
    # -----------------------------
    def abrir_dialogo_colores(self):
        dialogo = DialogColores(self, self.current_color_selection)
        if dialogo.exec():
            self.current_color_selection = dialogo.selected_state()
            self._actualizar_preview_nombre()

    # -----------------------------
    # DIÁLOGO DE SELECCIÓN DE IMÁGENES
    # -----------------------------
    def abrir_dialogo_imagenes(self):
        if not self.loaded_images:
            QMessageBox.information(self, "Aviso", "Primero carga imágenes.")
            return

        dialogo = DialogSeleccionImagenes(
            self,
            image_paths=self.loaded_images,
            selected_paths=self.selected_images,
        )

        if dialogo.exec():
            self.selected_images = dialogo.get_selected_images()
            self._actualizar_label_imagenes_destino()

    def _actualizar_label_imagenes_destino(self):
        total = len(self.selected_images)
        if total == 0:
            self.ui.label.setText("No se seleccionaron imágenes.")
        elif total == 1:
            self.ui.label.setText("1 imagen seleccionada.")
        else:
            self.ui.label.setText(f"{total} imágenes seleccionadas.")

    # -----------------------------
    # REGLAS
    # -----------------------------
    def _actualizar_stacked_valor(self):
        effect_text = self.ui.cmbEfecto.currentText().strip()
        if effect_text == "Invertir imagen":
            self.ui.stackedValor.setCurrentWidget(self.ui.pageValorInvertir)
        else:
            self.ui.stackedValor.setCurrentWidget(self.ui.pageValorDesenfoque)
        self._actualizar_preview_nombre()

    def _leer_regla_desde_editor(self) -> Rule:
        effect_text = self.ui.cmbEfecto.currentText().strip()

        if effect_text == "Invertir imagen":
            effect = "inv"
            value = self.ui.cmbGiro.currentText().strip()
        else:
            effect = "des"
            value = str(self.ui.spinKernel.value())

        return Rule(
            use_r=self.current_color_selection["r"],
            use_g=self.current_color_selection["g"],
            use_b=self.current_color_selection["b"],
            use_gray=self.current_color_selection["gray"],
            effect=effect,
            value=value,
        )

    def agregar_regla(self):
        rule = self._leer_regla_desde_editor()

        if rule in self.rules:
            QMessageBox.critical(
                self, "Error", "La regla ya existe y no se puede repetir."
            )
            return

        self.rules.append(rule)
        self.editing_rule_row = None
        self.actualizar_tabla_reglas()

    def editar_regla(self):
        selected_rows = self._selected_rule_rows()
        if not selected_rows:
            QMessageBox.information(self, "Aviso", "Selecciona una regla para editar.")
            return

        row = selected_rows[0]
        new_rule = self._leer_regla_desde_editor()

        for idx, existing in enumerate(self.rules):
            if idx != row and existing == new_rule:
                QMessageBox.critical(
                    self, "Error", "La regla editada duplicaría una regla existente."
                )
                return

        self.rules[row] = new_rule
        self.actualizar_tabla_reglas()
        self.ui.rulesTable.selectRow(row)

    def eliminar_regla(self):
        selected_rows = self._selected_rule_rows()
        if not selected_rows:
            QMessageBox.information(
                self, "Aviso", "Selecciona una regla para eliminar."
            )
            return

        for row in sorted(selected_rows, reverse=True):
            del self.rules[row]

        self.editing_rule_row = None
        self.actualizar_tabla_reglas()
        self._actualizar_preview_nombre()

    def _selected_rule_rows(self):
        rows = sorted(
            {
                index.row()
                for index in self.ui.rulesTable.selectionModel().selectedRows()
            }
        )
        return rows

    def _cargar_regla_seleccionada_en_editor(self):
        selected_rows = self._selected_rule_rows()
        if not selected_rows:
            return

        row = selected_rows[0]
        rule = self.rules[row]

        self.current_color_selection = {
            "r": rule.use_r,
            "g": rule.use_g,
            "b": rule.use_b,
            "gray": rule.use_gray,
        }

        if rule.effect == "inv":
            self.ui.cmbEfecto.setCurrentText("Invertir imagen")
            self.ui.cmbGiro.setCurrentText(rule.value)
        else:
            self.ui.cmbEfecto.setCurrentText("Desenfocado")
            self.ui.spinKernel.setValue(int(rule.value))

        self.editing_rule_row = row
        self._actualizar_preview_nombre()

    def actualizar_tabla_reglas(self):
        self.ui.rulesTable.setRowCount(len(self.rules))

        for row, rule in enumerate(self.rules):
            color_key = self.build_color_key(
                rule.use_r, rule.use_g, rule.use_b, rule.use_gray
            )
            effect_text = "Invertir imagen" if rule.effect == "inv" else "Desenfocado"
            value_text = rule.value
            resumen = self.build_rule_summary(rule)

            values = [color_key, effect_text, value_text, resumen]
            for col, value in enumerate(values):
                item = QTableWidgetItem(value)
                item.setFlags(item.flags() & ~Qt.ItemFlag.ItemIsEditable)
                self.ui.rulesTable.setItem(row, col, item)

        self.ui.rulesTable.resizeRowsToContents()
        self._actualizar_preview_nombre()

    def build_rule_summary(self, rule: Rule) -> str:
        color_key = self.build_color_key(
            rule.use_r, rule.use_g, rule.use_b, rule.use_gray
        )
        if rule.effect == "inv":
            return f"{color_key} | Invertir | {rule.value}"
        return f"{color_key} | Desenfoque | Kernel {rule.value}"

    def build_color_key(self, use_r, use_g, use_b, use_gray):
        base = ""
        if use_r:
            base += "R"
        if use_g:
            base += "G"
        if use_b:
            base += "B"
        if not base:
            base = "RGB"
        return f"{base}-G" if use_gray else base

    def build_output_name(self, base_name: str, rule: Rule):
        color_key = self.build_color_key(
            rule.use_r, rule.use_g, rule.use_b, rule.use_gray
        )
        effect_key = "INV" if rule.effect == "inv" else "DES"

        if rule.effect == "inv":
            inv_map = {
                "Giro Espejo": "HOR",
                "Giro Vertical": "VER",
                "Giro Vertical Espejo": "VH",
                "Giro espejo": "HOR",
                "Giro vertical": "VER",
                "Giro vertical espejo": "VH",
            }
            value_key = inv_map[rule.value]
        else:
            value_key = str(rule.value)

        return f"{base_name}_{color_key}_{effect_key}_{value_key}"

    def _actualizar_preview_nombre(self):
        base_name = "IMAGEN"
        if self.selected_images:
            base_name = Path(self.selected_images[0]).stem
        elif self.loaded_images:
            base_name = Path(self.loaded_images[0]).stem

        rule = self._leer_regla_desde_editor()
        preview = self.build_output_name(base_name, rule)
        self.ui.lblPreviewNombre.setText(f"Nombre de salida: {preview}")


    # -----------------------------
    # EJECUCIÓN MPI / PROGRESO
    # -----------------------------

    def effect_value_to_c(self, rule: Rule):
        """
        Convierte el valor visible de la regla a lo que espera el código C.
        """
        if rule.effect == "inv":
            mapping = {
                "Giro Espejo": 2,
                "Giro Vertical": 4,
                "Giro Vertical Espejo": 3,
                "Giro espejo": 2,
                "Giro vertical": 4,
                "Giro vertical espejo": 3,
            }
            return mapping[rule.value]
        return int(rule.value)

    def ejecutar_trabajos(self):
        """
        Ejecuta SOLO en modo distribuido por red usando MPI.

        Flujo:
            1. La GUI genera jobs.tsv.
            2. mpiexec lanza func/para_image_mpi en pcA/pcB/pcC.
            3. para_image_mpi reparte trabajos dinámicamente.
            4. La GUI lee stdout/stderr en vivo y actualiza el progreso.
        """
        if not self.rules:
            QMessageBox.warning(self, "Aviso", "No hay reglas para ejecutar.")
            return

        if not self.selected_images:
            QMessageBox.warning(self, "Aviso", "No hay imágenes seleccionadas.")
            return

        output_dir = Path(self.ui.txtRutaGuardado.text().strip() or self.default_output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)

        machinefile = Path.home() / "machinefile"
        if not machinefile.exists():
            QMessageBox.critical(self, "Error", f"No existe machinefile: {machinefile}")
            return

        total_slots = self.count_slots_from_machinefile(machinefile)
        virtual_threads = int(self.ui.txtNumberThreads.value())

        if total_slots < 2:
            QMessageBox.critical(
                self,
                "Configuración MPI inválida",
                "Necesitas al menos 2 procesos MPI: 1 master y 1 worker.",
            )
            return

        jobs_file = self.project_root / "jobs.tsv"
        self.build_jobs_file(jobs_file, output_dir)

        executable = self.project_root / "func" / "para_image_mpi"
        if not executable.exists():
            QMessageBox.critical(
                self,
                "Ejecutable no encontrado",
                f"No existe: {executable}\nCompila con build_ui.sh.",
            )
            return

        self.total_jobs = self.count_total_jobs()
        self.completed_jobs = 0
        self.progress_start_time = time.perf_counter()
        self.mpi_stdout_buffer = ""
        self.mpi_stderr_buffer = ""
        self.current_log_file = self.project_root / "mpi_run.log"
        self.last_progress_line = "Iniciando procesamiento distribuido..."
        self.cancel_requested = False
        self.current_cmd = []
        self.current_job = None
        self.local_running = False

        cmd = [
            "mpiexec",
            "--bind-to", "none",
            "--mca", "btl_tcp_if_include", "192.168.133.0/24",
            "--mca", "oob_tcp_if_include", "192.168.133.0/24",
            "--tag-output",
            "-n", str(total_slots),
            "--hostfile", str(machinefile),
            str(executable),
            "--jobs", str(jobs_file),
            "--threads", str(virtual_threads),
        ]
        self.current_cmd = cmd

        self.progress_dialog = DialogProgreso(self)
        self.progress_dialog.ui.btnCancelar.clicked.connect(self.cancelar_mpi)
        self.progress_dialog.show()

        self.iniciar_timer_progreso()
        self.actualizar_progreso_mpi(self.last_progress_line)
        self.btn_estado_ejecucion(False)

        self.mpi_stdout_buffer += "COMANDO MPI:\n" + " ".join(cmd) + "\n\nSTDOUT:\n"

        self.mpi_process = QProcess(self)
        self.mpi_process.setWorkingDirectory(str(self.project_root))
        self.mpi_process.setProgram(cmd[0])
        self.mpi_process.setArguments(cmd[1:])
        self.mpi_process.readyReadStandardOutput.connect(self.leer_stdout_mpi)
        self.mpi_process.readyReadStandardError.connect(self.leer_stderr_mpi)
        self.mpi_process.finished.connect(self.mpi_finalizado)
        self.mpi_process.start()

    def mpi_finalizado(self, exit_code, exit_status):
        elapsed = time.perf_counter() - self.progress_start_time
        self.detener_timer_progreso()

        self.ui.txtTiempoEjecucion.setText(f"{elapsed:.4f} s")

        if self.current_log_file is not None:
            self.current_log_file.write_text(
                self.mpi_stdout_buffer + "\n\nSTDERR:\n" + self.mpi_stderr_buffer,
                encoding="utf-8",
            )

        self.btn_estado_ejecucion(True)
        log_text = f"\n\nLog completo:\n{self.current_log_file}"

        if self.cancel_requested:
            if self.progress_dialog is not None:
                self.progress_dialog.close()
            QMessageBox.information(
                self,
                "Cancelado",
                f"Procesamiento cancelado por el usuario.\n"
                f"Transformaciones procesadas: {self.completed_jobs}/{self.total_jobs}\n"
                f"Tiempo total: {elapsed:.2f} s" + log_text,
            )
            return

        if exit_code != 0:
            if self.progress_dialog is not None:
                self.progress_dialog.close()
            QMessageBox.critical(
                self,
                "Error MPI",
                f"mpiexec terminó con código: {exit_code}\n\n"
                f"Comando:\n{' '.join(self.current_cmd)}\n\n"
                f"STDERR reciente:\n{(self.mpi_stderr_buffer or 'Sin stderr')[-2500:]}\n\n"
                f"STDOUT reciente:\n{(self.mpi_stdout_buffer or 'Sin stdout')[-2500:]}"
                + log_text,
            )
            return

        if self.progress_dialog is not None:
            self.progress_dialog.actualizar(
                completadas=self.total_jobs,
                total=self.total_jobs,
                elapsed=elapsed,
                eta=0.0,
                ultimo_evento="Procesamiento distribuido terminado correctamente.",
            )
            self.progress_dialog.close()

        QMessageBox.information(
            self,
            "Éxito",
            f"Procesamiento distribuido terminado correctamente.\n"
            f"Transformaciones procesadas: {self.completed_jobs}/{self.total_jobs}\n"
            f"Tiempo total: {elapsed:.2f} s" + log_text,
        )

    def iniciar_timer_progreso(self):
        """
        Actualiza tiempo transcurrido y ETA cada segundo.
        No incrementa completed_jobs.
        """
        self.detener_timer_progreso()
        self.progress_timer = QTimer(self)
        self.progress_timer.timeout.connect(self.actualizar_progreso_mpi)
        self.progress_timer.start(1000)

    def detener_timer_progreso(self):
        if self.progress_timer is not None:
            self.progress_timer.stop()
            self.progress_timer = None

    def btn_estado_ejecucion(self, habilitado: bool):
        self.ui.btnEjecutarPrograma.setEnabled(habilitado)
        self.ui.btnCargarImagenes.setEnabled(habilitado)
        self.ui.btnEliminarImagen.setEnabled(habilitado)
        self.ui.btnLimpiarLista.setEnabled(habilitado)
        self.ui.btnAgregarRegla.setEnabled(habilitado)
        self.ui.btnEditarRegla.setEnabled(habilitado)
        self.ui.btnEliminarRegla.setEnabled(habilitado)

    def leer_stdout_mpi(self):
        if self.mpi_process is None:
            return
        data = bytes(self.mpi_process.readAllStandardOutput()).decode("utf-8", errors="replace")
        self.mpi_stdout_buffer += data
        for line in data.splitlines():
            self.procesar_linea_mpi(line)

    def leer_stderr_mpi(self):
        if self.mpi_process is None:
            return
        data = bytes(self.mpi_process.readAllStandardError()).decode("utf-8", errors="replace")
        self.mpi_stderr_buffer += data
        for line in data.splitlines():
            self.procesar_linea_mpi(line)

    def procesar_linea_mpi(self, line: str):
        """
        Cuenta trabajos terminados desde stdout/stderr.

        para_image_mpi imprime:
            [rank X][host Y] FIN output=...
        """
        line = line.strip()
        if not line:
            return

        if " FIN " in line or "] FIN " in line or " OK " in line:
            if self.completed_jobs < self.total_jobs:
                self.completed_jobs += 1
            self.actualizar_progreso_mpi(line)

    def actualizar_progreso_mpi(self, ultima_linea: str = ""):
        if self.progress_dialog is None:
            return

        if ultima_linea:
            self.last_progress_line = ultima_linea

        elapsed = time.perf_counter() - self.progress_start_time

        MIN_JOBS_FOR_ETA = 5
        if self.completed_jobs >= MIN_JOBS_FOR_ETA:
            avg_time_per_job = elapsed / self.completed_jobs
            remaining_jobs = max(self.total_jobs - self.completed_jobs, 0)
            eta = avg_time_per_job * remaining_jobs
            if remaining_jobs > 0 and eta < 1.0:
                eta = 1.0
        else:
            eta = None

        self.progress_dialog.actualizar(
            completadas=self.completed_jobs,
            total=self.total_jobs,
            elapsed=elapsed,
            eta=eta,
            ultimo_evento=self.last_progress_line,
        )

    def cancelar_mpi(self):
        self.cancel_requested = True
        self.detener_timer_progreso()

        if self.mpi_process is not None:
            self.mpi_process.kill()

        if self.progress_dialog is not None:
            elapsed = time.perf_counter() - self.progress_start_time
            self.progress_dialog.actualizar(
                completadas=self.completed_jobs,
                total=self.total_jobs,
                elapsed=elapsed,
                eta=0.0,
                ultimo_evento="Cancelando procesamiento...",
            )

    # -----------------------------
    # RUTA DE GUARDADO
    # -----------------------------
    def _abrir_selector_ruta_guardado(self, event):
        ruta = QFileDialog.getExistingDirectory(
            self,
            "Seleccionar carpeta de salida",
            self.ui.txtRutaGuardado.text().strip() or str(self.default_output_dir),
        )
        if ruta:
            self.ui.txtRutaGuardado.setText(ruta)
        if event is not None:
            event.accept()

    def agregar_imagenes_desde_paths(self, files):
        if not files:
            return

        nuevos = 0
        limite_alcanzado = False

        for file_path in files:
            if len(self.loaded_images) >= MAX_IMAGES:
                limite_alcanzado = True
                break

            path = Path(file_path).resolve()

            if path.suffix.lower() != ".bmp":
                continue

            normalized = str(path)

            if normalized in self.loaded_images:
                continue

            self.loaded_images.append(normalized)
            self.selected_images.append(normalized)
            self._agregar_item_imagen(normalized)
            nuevos += 1

        self._actualizar_estado_imagenes()
        self._actualizar_label_imagenes_destino()
        self._actualizar_preview_nombre()

        if limite_alcanzado:
            QMessageBox.warning(
                self,
                "Límite alcanzado",
                f"Solo puedes cargar un máximo de {MAX_IMAGES} imágenes."
            )

        elif nuevos == 0:
            QMessageBox.information(
                self,
                "Sin cambios",
                "No se agregaron imágenes nuevas. Verifica que sean archivos .bmp.",
            )
                

    def eventFilter(self, obj, event):
        if obj == self.ui.listImagenes:
            if event.type() == QEvent.Type.DragEnter:
                if event.mimeData().hasUrls():
                    event.acceptProposedAction()
                    return True

            elif event.type() == QEvent.Type.Drop:
                files = []

                for url in event.mimeData().urls():
                    if url.isLocalFile():
                        files.append(url.toLocalFile())

                self.agregar_imagenes_desde_paths(files)
                event.acceptProposedAction()
                return True

        return super().eventFilter(obj, event)