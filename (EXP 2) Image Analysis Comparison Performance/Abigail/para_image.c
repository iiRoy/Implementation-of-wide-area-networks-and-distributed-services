#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "select_proc.h"
#define NUM_THREADS 18
// gcc para_image.c select_proc.c -o para_image -fopenmp

int main(){
    FILE *fptr;
    char data[80] = "arc1.txt";
    int images[3] = {1, 2, 3};

    fptr = fopen(data, "w");
    if(fptr == NULL){ printf("Error\n"); exit(1); }
    fclose(fptr);

    // Separate mask sets for each image
    char asset[3][80], m_inv[3][80], m_inv_color[3][80];
    char m_fliph[3][80], m_flipv[3][80], m_flipv_color[3][80];
    char m_fliph_color[3][80], m_blurry[3][80], m_blurry_color[3][80];

    for(int i = 0; i < 3; i++){
        int n = images[i];
        snprintf(asset[i],        sizeof(asset[i]),        "Assets/sample%d.bmp",     n);
        snprintf(m_inv[i],        sizeof(m_inv[i]),        "inv%d",                   n);
        snprintf(m_inv_color[i],  sizeof(m_inv_color[i]),  "inv_color%d",             n);
        snprintf(m_fliph[i],      sizeof(m_fliph[i]),      "flippedh_%d",             n);
        snprintf(m_flipv[i],      sizeof(m_flipv[i]),      "flippedv_%d",             n);
        snprintf(m_flipv_color[i],sizeof(m_flipv_color[i]),"flippedv_color_%d",       n);
        snprintf(m_fliph_color[i],sizeof(m_fliph_color[i]),"flippedh_color_%d",       n);
        snprintf(m_blurry[i],     sizeof(m_blurry[i]),     "sample%d_blurry",         n);
        snprintf(m_blurry_color[i],sizeof(m_blurry_color[i]),"sample%d_blurry_color", n);
    }

    omp_set_num_threads(NUM_THREADS);
    const double ST = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp sections
        {
            // --- Image 1 ---
            #pragma omp section
            inv_img(m_inv[0], asset[0]);
            #pragma omp section
            inv_img_color(m_inv_color[0], asset[0]);
            #pragma omp section
            blur(asset[0], m_blurry[0], 34);
            #pragma omp section
            blur_color(asset[0], m_blurry_color[0], 34);
            #pragma omp section
            fliph(m_fliph[0], asset[0]);
            #pragma omp section
            fliph_color(m_fliph_color[0], asset[0]);
            #pragma omp section
            flipv(m_flipv[0], asset[0]);
            #pragma omp section
            flipv_color(m_flipv_color[0], asset[0]);

            // --- Image 2 ---
            #pragma omp section
            inv_img(m_inv[1], asset[1]);
            #pragma omp section
            inv_img_color(m_inv_color[1], asset[1]);
            #pragma omp section
            blur(asset[1], m_blurry[1], 34);
            #pragma omp section
            blur_color(asset[1], m_blurry_color[1], 34);
            #pragma omp section
            fliph(m_fliph[1], asset[1]);
            #pragma omp section
            fliph_color(m_fliph_color[1], asset[1]);
            #pragma omp section
            flipv(m_flipv[1], asset[1]);
            #pragma omp section
            flipv_color(m_flipv_color[1], asset[1]);

            // --- Image 3 ---
            #pragma omp section
            inv_img(m_inv[2], asset[2]);
            #pragma omp section
            inv_img_color(m_inv_color[2], asset[2]);
            #pragma omp section
            blur(asset[2], m_blurry[2], 34);
            #pragma omp section
            blur_color(asset[2], m_blurry_color[2], 34);
            #pragma omp section
            fliph(m_fliph[2], asset[2]);
            #pragma omp section
            fliph_color(m_fliph_color[2], asset[2]);
            #pragma omp section
            flipv(m_flipv[2], asset[2]);
            #pragma omp section
            flipv_color(m_flipv_color[2], asset[2]);
        }
    }

    const double STOP = omp_get_wtime();
    printf("Tiempo = %lf \n", (STOP - ST));
    return 0;
}
