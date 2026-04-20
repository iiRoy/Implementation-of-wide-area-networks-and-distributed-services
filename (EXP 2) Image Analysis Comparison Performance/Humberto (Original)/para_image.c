#include <stdio.h>
#include <omp.h>
#include "select_proc.h"

//sudo auto-cpufreq --turbo active

//#define NUM_THREADS 6
#define NUM_THREADS 100
// #define NUM_THREADS 18

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
//          ------   IMAGE 1   ------
            #pragma omp section
            inv_img("GVG1", "./Assets/Image1.bmp");
            #pragma omp section
            inv_img_color("GVN1", "./Assets/Image1.bmp");
            #pragma omp section
            blur("./Assets/Image1.bmp", "DG1", 34);
            #pragma omp section
            blur_color("./Assets/Image1.bmp", "DN1", 34);
            #pragma omp section
            fliph("GMG1", "./Assets/Image1.bmp");
            #pragma omp section
            fliph_color("GMN1", "./Assets/Image1.bmp");
            #pragma omp section
            flipv("GVMG1", "./Assets/Image1.bmp");
            #pragma omp section
            flipv_color("GVMN1", "./Assets/Image1.bmp");
//          ------   IMAGE 2   ------
            #pragma omp section
            inv_img("GVG2", "./Assets/Image2.bmp");
            #pragma omp section
            inv_img_color("GVN2", "./Assets/Image2.bmp");
            #pragma omp section
            blur("./Assets/Image2.bmp", "DG2", 34);
            #pragma omp section
            blur_color("./Assets/Image2.bmp", "DN2", 34);
            #pragma omp section
            fliph("GMG2", "./Assets/Image2.bmp");
            #pragma omp section
            fliph_color("GMN2", "./Assets/Image2.bmp");
            #pragma omp section
            flipv("GVMG2", "./Assets/Image2.bmp");
            #pragma omp section
            flipv_color("GVMN2", "./Assets/Image2.bmp");
//          ------   IMAGE 3   ------
            #pragma omp section
            inv_img("GVG3", "./Assets/Image3.bmp");
            #pragma omp section
            inv_img_color("GVN3", "./Assets/Image3.bmp");
            #pragma omp section
            blur("./Assets/Image3.bmp", "DG3", 34);
            #pragma omp section
            blur_color("./Assets/Image3.bmp", "DN3", 34);
            #pragma omp section
            fliph("GMG3", "./Assets/Image3.bmp");
            #pragma omp section
            fliph_color("GMN3", "./Assets/Image3.bmp");
            #pragma omp section
            flipv("GVMG3", "./Assets/Image3.bmp");
            #pragma omp section
            flipv_color("GVMN3", "./Assets/Image3.bmp");
        }
    } //*/
    const double STOP = omp_get_wtime();
    printf("Tiempo = %lf \n", (STOP - ST));
    return 0;
}
