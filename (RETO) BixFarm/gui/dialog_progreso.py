from PyQt6.QtWidgets import QDialog
from ui.ui_dialog_progreso import Ui_DialogProgreso


class DialogProgreso(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.ui = Ui_DialogProgreso()
        self.ui.setupUi(self)

        self.setModal(True)
        self.setWindowTitle("Procesando imágenes")

        self.ui.progressBar.setMinimum(0)
        self.ui.progressBar.setMaximum(100)
        self.ui.progressBar.setValue(0)
        self.ui.progressBar.setTextVisible(False)

    def formatear_tiempo(self, seconds: float) -> str:
        seconds = max(0.0, float(seconds))

        if seconds < 1:
            return f"{seconds:.1f}s"

        total_seconds = int(seconds)
        minutes, sec = divmod(total_seconds, 60)
        hours, minutes = divmod(minutes, 60)

        if hours > 0:
            return f"{hours}h {minutes}m {sec}s"
        if minutes > 0:
            return f"{minutes}m {sec}s"
        return f"{sec}s"

    def actualizar(
        self,
        completadas: int,
        total: int,
        elapsed: float,
        eta,
        ultimo_evento: str = "",
    ):
        porcentaje = int((completadas / total) * 100) if total > 0 else 0

        self.ui.progressBar.setValue(porcentaje)
        self.ui.lblPorcentaje.setText(f"{porcentaje}%")
        self.ui.lblEstado.setText(
            f"Procesadas {completadas} de {total} transformaciones"
        )

        if eta is None:
            eta_text = "calculando..."
        else:
            eta_text = self.formatear_tiempo(eta)

        self.ui.lblTiempo.setText(
            f"Tiempo transcurrido: {self.formatear_tiempo(elapsed)}   |   "
            f"Tiempo estimado restante: {eta_text}"
        )

        if ultimo_evento:
            self.ui.lblUltimoEvento.setText(ultimo_evento[:180])