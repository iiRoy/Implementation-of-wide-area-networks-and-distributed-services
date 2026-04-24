#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "./lib/imgTrans.h"

#define DEFAULT_THREADS 12

int main(int argc, char *argv[]) {
    char input[512] = {0};

    char output_inv[256] = {0};
    char output_des[256] = {0};

    char effect[16] = {0};

    int value_inv = 0;
    int value_des = 0;

    int use_r = 0, use_g = 0, use_b = 0, use_gray = 0;
    int num_threads = DEFAULT_THREADS;

    int do_inv = 0;
    int do_des = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            strcpy(input, argv[++i]);

        } else if (strcmp(argv[i], "--effect") == 0 && i + 1 < argc) {
            strcpy(effect, argv[++i]);

        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            char temp_output[256] = {0};
            strcpy(temp_output, argv[++i]);
            strcpy(output_inv, temp_output);
            strcpy(output_des, temp_output);

        } else if (strcmp(argv[i], "--output-inv") == 0 && i + 1 < argc) {
            strcpy(output_inv, argv[++i]);

        } else if (strcmp(argv[i], "--output-des") == 0 && i + 1 < argc) {
            strcpy(output_des, argv[++i]);

        } else if (strcmp(argv[i], "--value") == 0 && i + 1 < argc) {
            int temp_value = atoi(argv[++i]);
            value_inv = temp_value;
            value_des = temp_value;

        } else if (strcmp(argv[i], "--value-inv") == 0 && i + 1 < argc) {
            value_inv = atoi(argv[++i]);

        } else if (strcmp(argv[i], "--value-des") == 0 && i + 1 < argc) {
            value_des = atoi(argv[++i]);

        } else if (strcmp(argv[i], "--r") == 0 && i + 1 < argc) {
            use_r = atoi(argv[++i]);

        } else if (strcmp(argv[i], "--g") == 0 && i + 1 < argc) {
            use_g = atoi(argv[++i]);

        } else if (strcmp(argv[i], "--b") == 0 && i + 1 < argc) {
            use_b = atoi(argv[++i]);

        } else if (strcmp(argv[i], "--gray") == 0 && i + 1 < argc) {
            use_gray = atoi(argv[++i]);

        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            num_threads = atoi(argv[++i]);
        }
    }

    if (strlen(input) == 0 || strlen(effect) == 0) {
        fprintf(stderr, "Faltan argumentos obligatorios.\n");
        return 1;
    }

    if (num_threads < 1) {
        fprintf(stderr, "Numero de threads invalido: %d\n", num_threads);
        return 1;
    }

    if (strcmp(effect, "inv") == 0) {
        do_inv = 1;
        do_des = 0;

        if (strlen(output_inv) == 0 || value_inv == 0) {
            fprintf(stderr, "Faltan --output o --value para efecto inv.\n");
            return 1;
        }

    } else if (strcmp(effect, "des") == 0) {
        do_inv = 0;
        do_des = 1;

        if (strlen(output_des) == 0 || value_des == 0) {
            fprintf(stderr, "Faltan --output o --value para efecto des.\n");
            return 1;
        }

    } else if (strcmp(effect, "both") == 0 || strcmp(effect, "ambos") == 0) {
        do_inv = 1;
        do_des = 1;

        if (strlen(output_inv) == 0 || strlen(output_des) == 0) {
            fprintf(stderr, "Faltan --output-inv o --output-des para effect=both.\n");
            return 1;
        }

        if (value_inv == 0 || value_des == 0) {
            fprintf(stderr, "Faltan --value-inv o --value-des para effect=both.\n");
            return 1;
        }

    } else {
        fprintf(stderr, "Efecto no valido: %s\n", effect);
        return 1;
    }

    omp_set_num_threads(num_threads);

    const double ST = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                if (do_inv) {
                    int tid = omp_get_thread_num();
                    inv_img_flags(output_inv, input, use_r, use_g, use_b, use_gray, value_inv);
                }
            }

            #pragma omp section
            {
                if (do_des) {
                    int tid = omp_get_thread_num();
                    desenfoque_flags(input, output_des, value_des, use_r, use_g, use_b, use_gray);
                }
            }
        }
    }

    const double STOP = omp_get_wtime();
    printf("[OMP] Tiempo total = %.6f s\n", STOP - ST);
    fflush(stdout);

    return 0;
}
