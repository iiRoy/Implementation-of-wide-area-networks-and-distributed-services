#include <stdio.h>
#include <stdlib.h>

int espejo_vertical_color(FILE *image, FILE *outputImage) {
  long ancho, alto;
  unsigned char r, g, b;
  unsigned char *ptrb, *ptrg, *ptrr;

  unsigned char xx[54];
  int cuenta = 0;

  for (int i = 0; i < 54; i++) {
    xx[i] = fgetc(image);
    fputc(xx[i], outputImage);
  }
  ancho = (long)xx[20] * 65536 + (long)xx[19] * 256 + (long)xx[18];
  alto  = (long)xx[24] * 65536 + (long)xx[23] * 256 + (long)xx[22];

  int padding = (4 - (ancho * 3) % 4) % 4;

  ptrb = (unsigned char *)malloc(alto * ancho * sizeof(unsigned char));
  ptrg = (unsigned char *)malloc(alto * ancho * sizeof(unsigned char));
  ptrr = (unsigned char *)malloc(alto * ancho * sizeof(unsigned char));

  while (cuenta < alto * ancho) {
    b = fgetc(image);
    g = fgetc(image);
    r = fgetc(image);
    ptrb[cuenta] = b;
    ptrg[cuenta] = g;
    ptrr[cuenta] = r;
    cuenta++;
  }

  for (int y = 0; y < alto; ++y) {
    for (int x = 0; x < ancho; ++x) {
      int i = (alto - 1 - y) * ancho + x;
      fputc(ptrb[i], outputImage);
      fputc(ptrg[i], outputImage);
      fputc(ptrr[i], outputImage);
    }
    for (int p = 0; p < padding; p++) {
      fputc(0, outputImage);
    }
  }

  free(ptrb);
  free(ptrg);
  free(ptrr);
  fclose(image);
  fclose(outputImage);
  return 0;
}
