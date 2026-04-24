#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cambiar_color_flags(
    unsigned char *r,
    unsigned char *g,
    unsigned char *b,
    int use_r,
    int use_g,
    int use_b,
    int use_gray
) {
    unsigned char orig_r = *r;
    unsigned char orig_g = *g;
    unsigned char orig_b = *b;

    if (use_gray) {
        unsigned char gray = (unsigned char)(0.21 * orig_r + 0.72 * orig_g + 0.07 * orig_b);
        *r = use_r ? gray : 0;
        *g = use_g ? gray : 0;
        *b = use_b ? gray : 0;
    } else {
        *r = use_r ? orig_r : 0;
        *g = use_g ? orig_g : 0;
        *b = use_b ? orig_b : 0;
    }
}

extern void inv_img_flags(
    const char *mask,
    const char *path,
    int use_r,
    int use_g,
    int use_b,
    int use_gray,
    int inv
)
{
    FILE *image, *outputImage;
    char add_char[80] = "./Resultados/";
    strcat(add_char, mask);
    strcat(add_char, ".bmp");

    image = fopen(path, "rb");
    outputImage = fopen(add_char, "wb");

    if (!image || !outputImage) {
        fprintf(stderr, "[inv] Error abriendo archivos.\n");
        if (image) fclose(image);
        if (outputImage) fclose(outputImage);
        return;
    }

    long ancho, alto;
    unsigned char r, g, b;
    unsigned char xx[54];

    for (int i = 0; i < 54; i++) {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    ancho = (long)xx[20]*65536 + (long)xx[19]*256 + (long)xx[18];
    alto  = (long)xx[24]*65536 + (long)xx[23]*256 + (long)xx[22];

    printf("[inv] ancho=%ld alto=%ld inv=%d\n", ancho, alto, inv);
    fflush(stdout);

    int total_pixels = (int)(ancho * alto);

    unsigned char *arr_in_b  = (unsigned char*)malloc(total_pixels);
    unsigned char *arr_in_g  = (unsigned char*)malloc(total_pixels);
    unsigned char *arr_in_r  = (unsigned char*)malloc(total_pixels);

    unsigned char *arr_out_b = (unsigned char*)malloc(total_pixels);
    unsigned char *arr_out_g = (unsigned char*)malloc(total_pixels);
    unsigned char *arr_out_r = (unsigned char*)malloc(total_pixels);

    if (!arr_in_b || !arr_in_g || !arr_in_r || !arr_out_b || !arr_out_g || !arr_out_r) {
        fprintf(stderr, "[inv] Error reservando memoria.\n");
        fclose(image);
        fclose(outputImage);
        free(arr_in_b); free(arr_in_g); free(arr_in_r);
        free(arr_out_b); free(arr_out_g); free(arr_out_r);
        return;
    }

    for (int j = 0; j < total_pixels; j++) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        if (feof(image)) break;

        cambiar_color_flags(&r, &g, &b, use_r, use_g, use_b, use_gray);

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;
    }

    switch (inv) {
        case 1:  // normal
            #pragma omp parallel for schedule(static)
            for (int i = 0; i < total_pixels; i++) {
                arr_out_b[i] = arr_in_b[i];
                arr_out_g[i] = arr_in_g[i];
                arr_out_r[i] = arr_in_r[i];
            }
            break;

        case 2:  // espejo horizontal
            #pragma omp parallel for schedule(static)
            for (int y = 0; y < alto; y++) {
                for (int x = 0; x < ancho; x++) {
                    int src = y * ancho + (ancho - 1 - x);
                    int dst = y * ancho + x;
                    arr_out_b[dst] = arr_in_b[src];
                    arr_out_g[dst] = arr_in_g[src];
                    arr_out_r[dst] = arr_in_r[src];
                }
            }
            break;

        case 3:  // vertical
            #pragma omp parallel for schedule(static)
            for (int y = 0; y < alto; y++) {
                for (int x = 0; x < ancho; x++) {
                    int src = (alto - 1 - y) * ancho + x;
                    int dst = y * ancho + x;
                    arr_out_b[dst] = arr_in_b[src];
                    arr_out_g[dst] = arr_in_g[src];
                    arr_out_r[dst] = arr_in_r[src];
                }
            }
            break;

        case 4:  // vertical + espejo
            #pragma omp parallel for schedule(static)
            for (int y = 0; y < alto; y++) {
                for (int x = 0; x < ancho; x++) {
                    int src = (alto - 1 - y) * ancho + (ancho - 1 - x);
                    int dst = y * ancho + x;
                    arr_out_b[dst] = arr_in_b[src];
                    arr_out_g[dst] = arr_in_g[src];
                    arr_out_r[dst] = arr_in_r[src];
                }
            }
            break;

        default:
            fprintf(stderr, "[inv] Modo de inversion no valido: %d\n", inv);
            fclose(image);
            fclose(outputImage);
            free(arr_in_b); free(arr_in_g); free(arr_in_r);
            free(arr_out_b); free(arr_out_g); free(arr_out_r);
            return;
    }

    // Escritura final secuencial
    for (int i = 0; i < total_pixels; i++) {
        fputc(arr_out_b[i], outputImage);
        fputc(arr_out_g[i], outputImage);
        fputc(arr_out_r[i], outputImage);
    }

    fclose(image);
    fclose(outputImage);

    free(arr_in_b); free(arr_in_g); free(arr_in_r);
    free(arr_out_b); free(arr_out_g); free(arr_out_r);
}

extern void desenfoque_flags(
    const char* input_path,
    const char* name_output,
    int kernel_size,
    int use_r,
    int use_g,
    int use_b,
    int use_gray
) {
    FILE *image, *outputImage;
    char output_path[100];
    snprintf(output_path, sizeof(output_path), "./Resultados/%s.bmp", name_output);
    image = fopen(input_path, "rb");
    outputImage = fopen(output_path, "wb");
    if (!image || !outputImage) {
        printf("Error abriendo archivos.\n");
        return;
    }
    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, image);
    fwrite(header, sizeof(unsigned char), 54, outputImage);
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int row_padded = (width * 3 + 3) & (~3);
    unsigned char** input_rows = (unsigned char**)malloc(height * sizeof(unsigned char*));
    unsigned char** output_rows = (unsigned char**)malloc(height * sizeof(unsigned char*));
    unsigned char** temp_rows = (unsigned char**)malloc(height * sizeof(unsigned char*));
    for (int i = 0; i < height; i++) {
        input_rows[i] = (unsigned char*)malloc(row_padded);
        output_rows[i] = (unsigned char*)malloc(row_padded);
        temp_rows[i] = (unsigned char*)malloc(row_padded);
        fread(input_rows[i], sizeof(unsigned char), row_padded, image);
    }
    int k = kernel_size / 2;
    // Paso intermedio: desenfoque horizontal
    #pragma omp parallel for schedule(static)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int sumB = 0, sumG = 0, sumR = 0, count = 0;
                for (int dx = -k; dx <= k; dx++) {
                    int nx = x + dx;
                    if (nx >= 0 && nx < width) {
                        int idx = nx * 3;
                        sumB += input_rows[y][idx + 0];
                        sumG += input_rows[y][idx + 1];
                        sumR += input_rows[y][idx + 2];
                        count++;
                    }
                }
                int index = x * 3;
                temp_rows[y][index + 0] = sumB / count;
                temp_rows[y][index + 1] = sumG / count;
                temp_rows[y][index + 2] = sumR / count;
            }
            // Copiar padding sin modificar
            for (int p = width * 3; p < row_padded; p++) {
                temp_rows[y][p] = input_rows[y][p];
            }
        }
    // Paso final: desenfoque vertical
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sumB = 0, sumG = 0, sumR = 0, count = 0;
            for (int dy = -k; dy <= k; dy++) {
                int ny = y + dy;
                if (ny >= 0 && ny < height) {
                    int idx = x * 3;
                    sumB += temp_rows[ny][idx + 0];
                    sumG += temp_rows[ny][idx + 1];
                    sumR += temp_rows[ny][idx + 2];
                    count++;
                }
            }
            int index = x * 3;
            output_rows[y][index + 0] = sumB / count;
            output_rows[y][index + 1] = sumG / count;
            output_rows[y][index + 2] = sumR / count;

            cambiar_color_flags(
                &output_rows[y][index + 2],
                &output_rows[y][index + 1],
                &output_rows[y][index + 0],
                use_r, use_g, use_b, use_gray
            );
        }
        // Copiar padding sin modificar
        for (int p = width * 3; p < row_padded; p++) {
            output_rows[y][p] = temp_rows[y][p];
        }
    }
    // Escritura final
    for (int i = 0; i < height; i++) {
        fwrite(output_rows[i], sizeof(unsigned char), row_padded, outputImage);
        free(input_rows[i]);
        free(temp_rows[i]);
        free(output_rows[i]);
    }
    // Escritura en archivo de registro
    FILE *outputLog = fopen("output_log.txt", "a");
    if (outputLog == NULL) {
        fprintf(stderr, "Error: No se pudo crear o abrir el archivo de registro.\n");
            fclose(image);
            fclose(outputImage);
        return;
    }
    fprintf(outputLog, "FunciÃ³n: desenfoque, con %s\n", input_path);
    fprintf(outputLog, "Localidades totales leÃ-das: %d\n", width * height);
    fprintf(outputLog, "Localidades totales escritas: %d\n", width * height);
    fprintf(outputLog, "-------------------------------------\n");
    fclose(outputLog);
    free(input_rows);
    free(temp_rows);
    free(output_rows);
    fclose(image);
    fclose(outputImage);
}
