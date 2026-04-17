#include <stdio.h>
#include <omp.h>
#include "funciones/inv_img.h"
#include "funciones/inv_imgColor.h"
#include "funciones/desenfoque.h"
#include "funciones/espejo.h"
#include "funciones/espejo_bn.h"
#include "funciones/espejo_vertical_color.h"
#include "funciones/espejo_vertical_bn.h"

#define NUM_THREADS 18 //cambiar a 12 y 18

int main(){
    omp_set_num_threads(NUM_THREADS);

    double ST = omp_get_wtime();

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            FILE *img = fopen("Image1.bmp", "rb");
            espejo_vertical_bn(img, fopen("outputs/outputEspejoVerticalBN1.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image1.bmp", "rb");
            espejo_vertical_color(img, fopen("outputs/outputEspejoVerticalColor1.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image1.bmp", "rb");
            desenfoque(img, fopen("outputs/outputDesenfoque1.bmp", "wb"), 27);
        }

        #pragma omp section
        {
            FILE *img = fopen("Image1.bmp", "rb");
            espejo(img, fopen("outputs/outputEspejoHorizontalColor1.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image1.bmp", "rb");
            espejo_bn(img, fopen("outputs/outputEspejoHorizontalBN1.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image1.bmp", "rb");
            inv_img(img, fopen("outputs/outputImagenInvertidaBN1.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image1.bmp", "rb");
            inv_imgColor(img, fopen("outputs/outputImagenInvertidaColor1.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image2.bmp", "rb");
            espejo_vertical_bn(img, fopen("outputs/outputEspejoVerticalBN2.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image2.bmp", "rb");
            espejo_vertical_color(img, fopen("outputs/outputEspejoVerticalColor2.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image2.bmp", "rb");
            desenfoque(img, fopen("outputs/outputDesenfoque2.bmp", "wb"), 27);
        }

        #pragma omp section
        {
            FILE *img = fopen("Image2.bmp", "rb");
            espejo(img, fopen("outputs/outputEspejoHorizontalColor2.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image2.bmp", "rb");
            espejo_bn(img, fopen("outputs/outputEspejoHorizontalBN2.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image2.bmp", "rb");
            inv_img(img, fopen("outputs/outputImagenInvertidaBN2.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image2.bmp", "rb");
            inv_imgColor(img, fopen("outputs/outputImagenInvertidaColor2.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image3.bmp", "rb");
            espejo_vertical_bn(img, fopen("outputs/outputEspejoVerticalBN3.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image3.bmp", "rb");
            espejo_vertical_color(img, fopen("outputs/outputEspejoVerticalColor3.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image3.bmp", "rb");
            desenfoque(img, fopen("outputs/outputDesenfoque3.bmp", "wb"), 27);
        }

        #pragma omp section
        {
            FILE *img = fopen("Image3.bmp", "rb");
            espejo(img, fopen("outputs/outputEspejoHorizontalColor3.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image3.bmp", "rb");
            espejo_bn(img, fopen("outputs/outputEspejoHorizontalBN3.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image3.bmp", "rb");
            inv_img(img, fopen("outputs/outputImagenInvertidaBN3.bmp", "wb"));
        }

        #pragma omp section
        {
            FILE *img = fopen("Image3.bmp", "rb");
            inv_imgColor(img, fopen("outputs/outputImagenInvertidaColor3.bmp", "wb"));
        }

    }

    double STOP = omp_get_wtime(); // <-- fin
    printf("Tiempo = %lf segundos\n", STOP - ST);
    return 0;
}
