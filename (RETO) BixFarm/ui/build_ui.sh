#!/bin/bash
pyuic6 Respaldo/PixFarm.ui -o ui/ui_pixfarm.py
pyuic6 Respaldo/dialog_colores.ui -o ui/ui_dialog_colores.py
pyuic6 Respaldo/dialog_seleccion_imagenes.ui -o ui/ui_dialog_seleccion_imagenes.py
echo "UI convertidas correctamente."
