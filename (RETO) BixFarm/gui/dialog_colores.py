from copy import deepcopy

from PyQt6.QtWidgets import QDialog, QMessageBox
from ui.ui_dialog_colores import Ui_DialogColores


class DialogColores(QDialog):
    DEFAULT_STATE = {"r": True, "g": True, "b": True, "gray": False}

    def __init__(self, parent=None, initial_state=None):
        super().__init__(parent)
        self.ui = Ui_DialogColores()
        self.ui.setupUi(self)

        self._result_state = deepcopy(self.DEFAULT_STATE)
        if initial_state:
            self._result_state.update(initial_state)

        self.ui.btnAplicarColor.clicked.connect(self.aplicar)
        self.ui.btnCancelarColor.clicked.connect(self.reject)

        self.ui.chkGris.toggled.connect(self._actualizar_estilo_dependiente_gris)

        self.set_state(self._result_state)
        self._actualizar_estilo_dependiente_gris()

    def set_state(self, state: dict):
        state = {**self.DEFAULT_STATE, **(state or {})}
        self.ui.chkRojo.setChecked(bool(state["r"]))
        self.ui.chkVerde.setChecked(bool(state["g"]))
        self.ui.chkAzul.setChecked(bool(state["b"]))
        self.ui.chkGris.setChecked(bool(state["gray"]))
        self._actualizar_estilo_dependiente_gris()

    def get_state(self) -> dict:
        return {
            "r": self.ui.chkRojo.isChecked(),
            "g": self.ui.chkVerde.isChecked(),
            "b": self.ui.chkAzul.isChecked(),
            "gray": self.ui.chkGris.isChecked(),
        }

    def aplicar(self):
        state = self.get_state()

        if not (state["r"] or state["g"] or state["b"]):
            QMessageBox.warning(
                self,
                "Selección inválida",
                "Debes dejar al menos uno de los canales R, G o B activado.",
            )
            return

        self._result_state = state
        self.accept()

    def selected_state(self) -> dict:
        return deepcopy(self._result_state)

    def _actualizar_estilo_dependiente_gris(self):
        gray_active = self.ui.chkGris.isChecked()

        rojo_checked = "#9ca3af" if gray_active else "#ef4444"
        verde_checked = "#9ca3af" if gray_active else "#22c55e"
        azul_checked = "#9ca3af" if gray_active else "#3b82f6"

        self.ui.chkRojo.setStyleSheet(self._checkbox_style(rojo_checked, "#ff6b6b"))
        self.ui.chkVerde.setStyleSheet(self._checkbox_style(verde_checked, "#4ade80"))
        self.ui.chkAzul.setStyleSheet(self._checkbox_style(azul_checked, "#60a5fa"))
        self.ui.chkGris.setStyleSheet(self._checkbox_style("#9ca3af", "#a3a3a3"))

    @staticmethod
    def _checkbox_style(checked_color: str, hover_color: str) -> str:
        return f"""
QCheckBox {{
    color: #e6edf7;
    spacing: 8px;
    background: transparent;
}}

QCheckBox::indicator {{
    width: 18px;
    height: 18px;
    border-radius: 4px;
    border: 1px solid #385174;
    background-color: #0a1423;
}}

QCheckBox::indicator:hover {{
    border: 1px solid {hover_color};
}}

QCheckBox::indicator:checked {{
    background-color: {checked_color};
    border: 1px solid {checked_color};
}}
"""
