#include <stdio.h>
#include <stdlib.h>

int espejo_bn(FILE *image, FILE *outputImage) {
  long ancho, alto;
  unsigned char r, g, b;
  unsigned char *ptr;

  unsigned char xx[54];
  int cuenta = 0;

  for (int i = 0; i < 54; i++) {
    xx[i] = fgetc(image);
    fputc(xx[i], outputImage);
  }
  ancho = (long)xx[20] * 65536 + (long)xx[19] * 256 + (long)xx[18];
  alto  = (long)xx[24] * 65536 + (long)xx[23] * 256 + (long)xx[22];

  int padding = (4 - (ancho * 3) % 4) % 4;

  ptr = (unsigned char *)malloc(alto * ancho * sizeof(unsigned char));

  while (cuenta < alto * ancho) {
    b = fgetc(image);
    g = fgetc(image);
    r = fgetc(image);
    ptr[cuenta] = (unsigned char)(0.21*r + 0.72*g + 0.07*b);
    cuenta++;
  }

  for (int y = 0; y < alto; ++y) {
    for (int x = 0; x < ancho; ++x) {
      int i = y * ancho + (ancho - 1 - x);  
      fputc(ptr[i], outputImage);            // B
      fputc(ptr[i], outputImage);            // G
      fputc(ptr[i], outputImage);            // R
    }
    for (int p = 0; p < padding; p++) {
      fputc(0, outputImage);
    }
  }

  free(ptr);
  fclose(image);
  fclose(outputImage);
  return 0;
}
