from pathlib import Path

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QDialog, QListWidgetItem
from ui.ui_dialog_seleccion_imagenes import Ui_DialogSeleccionImagenes


class DialogSeleccionImagenes(QDialog):
    def __init__(self, parent=None, image_paths=None, selected_paths=None):
        super().__init__(parent)
        self.ui = Ui_DialogSeleccionImagenes()
        self.ui.setupUi(self)

        self._image_paths = []
        self._selected_paths = set(selected_paths or [])

        self.ui.btnAceptarSeleccion.clicked.connect(self.accept)
        self.ui.btnCancelarSeleccion.clicked.connect(self.reject)

        if image_paths:
            self.set_images(image_paths, selected_paths or [])

    def set_images(self, image_paths, selected_paths=None):
        self._image_paths = [str(Path(p)) for p in image_paths]
        selected_set = {str(Path(p)) for p in (selected_paths or [])}

        self.ui.listSeleccionImagenes.clear()

        for image_path in self._image_paths:
            item = QListWidgetItem(Path(image_path).name)
            item.setData(Qt.ItemDataRole.UserRole, image_path)
            item.setFlags(item.flags() | Qt.ItemFlag.ItemIsUserCheckable)
            item.setCheckState(
                Qt.CheckState.Checked if image_path in selected_set else Qt.CheckState.Unchecked
            )
            self.ui.listSeleccionImagenes.addItem(item)

    def get_selected_images(self):
        selected = []
        for row in range(self.ui.listSeleccionImagenes.count()):
            item = self.ui.listSeleccionImagenes.item(row)
            if item.checkState() == Qt.CheckState.Checked:
                selected.append(item.data(Qt.ItemDataRole.UserRole))
        return selected
