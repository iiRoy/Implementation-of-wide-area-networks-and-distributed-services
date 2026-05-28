#!/bin/bash
set -e

pyuic6 editables/PixFarm.ui -o ui/ui_pixfarm.py
pyuic6 editables/dialog_colores.ui -o ui/ui_dialog_colores.py
pyuic6 editables/dialog_seleccion_imagenes.ui -o ui/ui_dialog_seleccion_imagenes.py
pyuic6 editables/dialog_progreso.ui -o ui/ui_dialog_progreso.py

gcc func/para_image.c -o func/para_image -fopenmp -O3
mpicc func/para_image_mpi.c -o func/para_image_mpi -fopenmp -O3

echo "UI convertidas correctamente."
python main.py