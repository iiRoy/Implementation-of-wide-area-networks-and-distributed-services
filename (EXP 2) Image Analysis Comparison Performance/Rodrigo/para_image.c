#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "./lib/imgTrans.h"

//sudo auto-cpufreq --turbo active

#define NUM_THREADS 6
//#define NUM_THREADS 12
//#define NUM_THREADS 18

int main(){
    /*
    FILE *fptr;
    char data[80] = "arc1.txt";
    fptr = fopen(data, "w");
    if (fptr == NULL){
        printf("Error\n");
        exit(1);
    }
    fprintf(fptr, "Ejemplo excribir\n");
    fprintf(fptr, "Rodrigo López Guerra");
    fclose(fptr);
     */

    /* COLOR MODES */
    // Mode 1: Only Red
    // Mode 2: Only Green
    // Mode 3: Only Blue
    // Mode 4: Color Inverter
    // Mode 5: Switch Red & Blue
    // Mode 6: Black & White
    // Other number: Color
    //
    /* INVERSION MODE */
    // Mode 1: Normal
    // Mode 2: Mirror Inversion
    // Mode 3: Vertical Inversion
    // Mode 4: Vertical Mirror Inversion
    omp_set_num_threads(NUM_THREADS);
    const double ST = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            inv_img("GVG1", "./Images/Image1.bmp", 6, 3);
            #pragma omp section
            inv_img("GVN1", "./Images/Image1.bmp", 0, 3);
            #pragma omp section
            desenfoque("./Images/Image1.bmp", "DG1", 34, 6);
            #pragma omp section
            desenfoque("./Images/Image1.bmp", "DN1", 34, 0);
            #pragma omp section
            inv_img("GMG1", "./Images/Image1.bmp", 6, 2);
            #pragma omp section
            inv_img("GMN1", "./Images/Image1.bmp", 0, 2);
            #pragma omp section
            inv_img("GVMG1", "./Images/Image1.bmp", 6, 4);
            #pragma omp section
            inv_img("GVMN1", "./Images/Image1.bmp", 0, 4);
            #pragma omp section
            inv_img("GVG2", "./Images/Image2.bmp", 6, 3);
            #pragma omp section
            inv_img("GVN2", "./Images/Image2.bmp", 0, 3);
            #pragma omp section
            desenfoque("./Images/Image2.bmp", "DG2", 34, 6);
            #pragma omp section
            desenfoque("./Images/Image2.bmp", "DN2", 34, 0);
            #pragma omp section
            inv_img("GMG2", "./Images/Image2.bmp", 6, 2);
            #pragma omp section
            inv_img("GMN2", "./Images/Image2.bmp", 0, 2);
            #pragma omp section
            inv_img("GVMG2", "./Images/Image2.bmp", 6, 4);
            #pragma omp section
            inv_img("GVMN2", "./Images/Image2.bmp", 0, 4);
            #pragma omp section
            inv_img("GVG3", "./Images/Image3.bmp", 6, 3);
            #pragma omp section
            inv_img("GVN3", "./Images/Image3.bmp", 0, 3);
            #pragma omp section
            desenfoque("./Images/Image3.bmp", "DG3", 34, 6);
            #pragma omp section
            desenfoque("./Images/Image3.bmp", "DN3", 34, 0);
            #pragma omp section
            inv_img("GMG3", "./Images/Image3.bmp", 6, 2);
            #pragma omp section
            inv_img("GMN3", "./Images/Image3.bmp", 0, 2);
            #pragma omp section
            inv_img("GVMG3", "./Images/Image3.bmp", 6, 4);
            #pragma omp section
            inv_img("GVMN3", "./Images/Image3.bmp", 0, 4);
        }
    } //*/
    const double STOP = omp_get_wtime();
    printf("Tiempo = %lf \n", (STOP - ST));
    /*
     inv_img("RedInv", "./Images/Image.bmp", 1, 1);
     inv_img("GreenInv", "./Images/Image.bmp", 2, 2);
     inv_img("BlueInv", "./Images/Image.bmp", 3, 3);
     inv_img("InvertedInv", "./Images/Image.bmp", 4, 4);
     inv_img("BGRInv", "./Images/Image.bmp", 5, 1);
     inv_img("BnWInv", "./Images/Image.bmp", 6, 2);
     inv_img("NormalInv", "./Images/Image.bmp", 0, 3);
     desenfoque("./Images/Image.bmp", "NormalBlur", 34, 0);
     //*/
    return 0;
}
