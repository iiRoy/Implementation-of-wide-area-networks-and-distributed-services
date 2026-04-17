#include <stdio.h>
#include <stdlib.h>

int inv_imgColor(FILE* image, FILE* outputImage)
{
    long ancho;
    long alto;
    unsigned char r, g, b;               
    unsigned char* ptr;

    unsigned char xx[54];
    int cuenta = 0;
    for(int i=0; i<54; i++) {
      xx[i] = fgetc(image);
      fputc(xx[i], outputImage);   
    }
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];
    unsigned char* ptrb = (unsigned char*)malloc(alto*ancho*sizeof(unsigned char));
    unsigned char* ptrg = (unsigned char*)malloc(alto*ancho*sizeof(unsigned char));
    unsigned char* ptrr = (unsigned char*)malloc(alto*ancho*sizeof(unsigned char));

    while(cuenta < alto*ancho){
      b = fgetc(image);
      g = fgetc(image);
      r = fgetc(image);

      ptrb[cuenta] = b;
      ptrg[cuenta] = g;
      ptrr[cuenta] = r;

      cuenta++;

    }
    for (int i = 0; i < alto*ancho; ++i) {
      fputc(ptrb[(ancho * alto) - i], outputImage);
      fputc(ptrg[(ancho * alto) - i], outputImage);
      fputc(ptrr[(ancho * alto) - i], outputImage);
    }
    free(ptrb);
    free(ptrg);
    free(ptrr);
    fclose(image);
    fclose(outputImage);
    return 0;
}
