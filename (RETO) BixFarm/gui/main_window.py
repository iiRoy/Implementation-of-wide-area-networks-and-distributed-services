from dataclasses import dataclass
from pathlib import Path
import shutil
import subprocess
import time

from PyQt6.QtCore import Qt, QSize
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


@dataclass(frozen=True)
class Rule:
    use_r: bool
    use_g: bool
    use_b: bool
    use_gray: bool
    effect: str   # "inv" o "des"
    value: str    # texto del giro o kernel como string


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

        self._configurar_ui()
        self._conectar_senales()
        self._actualizar_estado_imagenes()
        self._actualizar_preview_nombre()
        self._actualizar_label_imagenes_destino()

    def _configurar_ui(self):
        self.ui.txtTiempoEjecucion.setReadOnly(True)
        self.ui.txtRutaGuardado.setText(str(self.default_output_dir))
        self.ui.txtRutaGuardado.setReadOnly(True)
        self.ui.txtRutaGuardado.setCursor(Qt.CursorShape.PointingHandCursor)

        # Abrir selector de carpeta al hacer click en la ruta
        self.ui.txtRutaGuardado.mousePressEvent = self._abrir_selector_ruta_guardado

        self.ui.rulesTable.setColumnCount(4)
        self.ui.rulesTable.setHorizontalHeaderLabels(["Color", "Efecto", "Valor", "Resumen"])
        self.ui.rulesTable.verticalHeader().setVisible(False)
        self.ui.rulesTable.setSelectionBehavior(self.ui.rulesTable.SelectionBehavior.SelectRows)
        self.ui.rulesTable.setSelectionMode(self.ui.rulesTable.SelectionMode.SingleSelection)
        self.ui.rulesTable.setAlternatingRowColors(True)
        self.ui.rulesTable.horizontalHeader().setStretchLastSection(True)

        self._actualizar_stacked_valor()

    def _conectar_senales(self):
        self.ui.btnCargarImagenes.clicked.connect(self.cargar_imagenes)
        self.ui.btnEliminarImagen.clicked.connect(self.eliminar_imagenes_seleccionadas)
        self.ui.btnLimpiarLista.clicked.connect(self.limpiar_lista_imagenes)

        self.ui.btnSeleccionColor.clicked.connect(self.abrir_dialogo_colores)
        self.ui.btnSeleccionarImagenesDestino.clicked.connect(self.abrir_dialogo_imagenes)

        self.ui.btnAgregarRegla.clicked.connect(self.agregar_regla)
        self.ui.btnEditarRegla.clicked.connect(self.editar_regla)
        self.ui.btnEliminarRegla.clicked.connect(self.eliminar_regla)

        self.ui.btnEjecutarPrograma.clicked.connect(self.ejecutar_trabajos)

        self.ui.cmbEfecto.currentIndexChanged.connect(self._actualizar_stacked_valor)
        self.ui.cmbEfecto.currentIndexChanged.connect(self._actualizar_preview_nombre)
        self.ui.cmbGiro.currentTextChanged.connect(self._actualizar_preview_nombre)
        self.ui.spinKernel.valueChanged.connect(self._actualizar_preview_nombre)

        self.ui.rulesTable.itemSelectionChanged.connect(self._cargar_regla_seleccionada_en_editor)

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

        if not files:
            return

        nuevos = 0
        for file_path in files:
            normalized = str(Path(file_path).resolve())
            if normalized in self.loaded_images:
                continue

            self.loaded_images.append(normalized)
            self.selected_images.append(normalized)
            self._agregar_item_imagen(normalized)
            nuevos += 1

        self._actualizar_estado_imagenes()
        self._actualizar_label_imagenes_destino()
        self._actualizar_preview_nombre()

        if nuevos == 0:
            QMessageBox.information(
                self,
                "Sin cambios",
                "Las imágenes seleccionadas ya estaban cargadas.",
            )

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
            QMessageBox.information(self, "Aviso", "Selecciona una o más imágenes para eliminarlas.")
            return

        paths_to_remove = {item.data(Qt.ItemDataRole.UserRole) for item in items}

        for item in items:
            row = self.ui.listImagenes.row(item)
            self.ui.listImagenes.takeItem(row)

        self.loaded_images = [p for p in self.loaded_images if p not in paths_to_remove]
        self.selected_images = [p for p in self.selected_images if p not in paths_to_remove]

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
            QMessageBox.critical(self, "Error", "La regla ya existe y no se puede repetir.")
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
                QMessageBox.critical(self, "Error", "La regla editada duplicaría una regla existente.")
                return

        self.rules[row] = new_rule
        self.actualizar_tabla_reglas()
        self.ui.rulesTable.selectRow(row)

    def eliminar_regla(self):
        selected_rows = self._selected_rule_rows()
        if not selected_rows:
            QMessageBox.information(self, "Aviso", "Selecciona una regla para eliminar.")
            return

        for row in sorted(selected_rows, reverse=True):
            del self.rules[row]

        self.editing_rule_row = None
        self.actualizar_tabla_reglas()
        self._actualizar_preview_nombre()

    def _selected_rule_rows(self):
        rows = sorted({index.row() for index in self.ui.rulesTable.selectionModel().selectedRows()})
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
            color_key = self.build_color_key(rule.use_r, rule.use_g, rule.use_b, rule.use_gray)
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
        color_key = self.build_color_key(rule.use_r, rule.use_g, rule.use_b, rule.use_gray)
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
        color_key = self.build_color_key(rule.use_r, rule.use_g, rule.use_b, rule.use_gray)
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
    # EJECUCIÓN C
    # -----------------------------
    def effect_value_to_c(self, rule: Rule):
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

    def run_c_job(self, image_path: str, output_name: str, rule: Rule):
        executable = self.project_root / "func" / "para_image"

        if not executable.exists():
            raise FileNotFoundError(
                f"No se encontró el ejecutable en: {executable}\n"
                "Compílalo primero, por ejemplo:\n"
                "gcc func/para_image.c -o func/para_image"
            )

        cmd = [
            str(executable),
            "--input", str(image_path),
            "--output", output_name,
            "--effect", rule.effect,
            "--value", str(self.effect_value_to_c(rule)),
            "--r", str(int(rule.use_r)),
            "--g", str(int(rule.use_g)),
            "--b", str(int(rule.use_b)),
            "--gray", str(int(rule.use_gray)),
        ]

        return subprocess.run(
            cmd,
            cwd=self.project_root,
            capture_output=True,
            text=True,
        )

    def ejecutar_trabajos(self):
        if not self.rules:
            QMessageBox.warning(self, "Aviso", "No hay reglas para ejecutar.")
            return

        if not self.selected_images:
            QMessageBox.warning(self, "Aviso", "No hay imágenes seleccionadas.")
            return

        output_dir = Path(self.ui.txtRutaGuardado.text().strip() or self.default_output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)

        start = time.perf_counter()
        errores = []
        generados = []

        for image_path in self.selected_images:
            base_name = Path(image_path).stem

            for rule in self.rules:
                output_name = self.build_output_name(base_name, rule)

                try:
                    result = self.run_c_job(image_path, output_name, rule)
                except FileNotFoundError as e:
                    QMessageBox.critical(self, "Ejecutable no encontrado", str(e))
                    return

                if result.returncode != 0:
                    errores.append(result.stderr.strip() or f"Error con {output_name}")
                    continue

                generated_file = self.project_root / "Resultados" / f"{output_name}.bmp"
                if generated_file.exists():
                    final_file = output_dir / generated_file.name
                    if generated_file.resolve() != final_file.resolve():
                        if final_file.exists():
                            final_file.unlink()
                        shutil.move(str(generated_file), str(final_file))
                    generados.append(str(final_file))
                else:
                    errores.append(f"No se encontró el archivo generado para {output_name}")

        elapsed = time.perf_counter() - start
        self.ui.txtTiempoEjecucion.setText(f"{elapsed:.4f} s")

        if errores:
            QMessageBox.critical(self, "Errores", "\n".join(errores[:10]))
        else:
            QMessageBox.information(
                self,
                "Éxito",
                f"Se procesaron correctamente {len(generados)} archivo(s)."
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
