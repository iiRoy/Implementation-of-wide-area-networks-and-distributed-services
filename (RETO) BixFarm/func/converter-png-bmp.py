import os, glob
from PIL import Image

entrada = "/mnt/data/Wallpapers/"
salida  = "/mnt/data/Wallpapers/BMP/"

os.makedirs(salida, exist_ok=True)
archivos = sorted(glob.glob(os.path.join(entrada, "*.png")))

for i, ruta in enumerate(archivos, start=1):
    with Image.open(ruta) as img:
        if img.mode in ("RGBA", "LA", "P"):
            img = img.convert("RGB")
        img.save(os.path.join(salida, f"C{i}.bmp"), format="BMP")
    print(f"C{i}.bmp  ←  {os.path.basename(ruta)}")

print(f"\n✅ {len(archivos)} imágenes convertidas en {salida}")