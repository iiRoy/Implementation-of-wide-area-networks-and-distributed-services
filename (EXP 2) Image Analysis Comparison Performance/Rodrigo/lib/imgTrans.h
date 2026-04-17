#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Realizar funciones:
// 1. Giro Vertical Gris
// 2. Giro Vertical Normal
// 3. Desenfoque Normal
// 4. Desenfoque Gris
// 5. Giro Mirror Gris
// 6. Giro Mirror Normal
void cambiar_color(unsigned char *r, unsigned char *g, unsigned char *b, int modo)
{
    unsigned char temp;

    switch (modo) {
        case 1:
            *g = 0;
            *b = 0;
            break;

        case 2:
            *r = 0;
            *b = 0;
            break;

        case 3:
            *r = 0;
            *g = 0;
            break;

        case 4:
            *r = 255 - *r;
            *g = 255 - *g;
            *b = 255 - *b;
            break;

        case 5:
            temp = *r;
            *r = *b;
            *b = temp;
            break;

        case 6: {
            unsigned char pixel = (unsigned char)(0.21 * (*r) + 0.72 * (*g) + 0.07 * (*b));
            *r = pixel;
            *g = pixel;
            *b = pixel;
            break;
        }

        default:
            break;
    }
}

extern void inv_img(char *mask, char *path, int mode, int inv)
{
    FILE *image, *outputImage, *lecturas;
    char add_char[80] = "./Resultados/";
    strcat(add_char, mask);
    strcat(add_char, ".bmp");
    image = fopen(path,"rb");          //Imagen original a transformar
    outputImage = fopen(add_char,"wb");    //Imagen transformada
    long ancho;
    long alto;
    unsigned char r, g, b;               //Pixel

    unsigned char xx[54];
    int j = 0;
    for(int i=0; i<54; i++) {
      xx[i] = fgetc(image);
      fputc(xx[i], outputImage);   //Copia cabecera a nueva imagen
    }
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];
    printf("largo img %li\n",alto);
    printf("ancho img %li\n",ancho);

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));

    int total_pixels = ancho * alto;

    for (j = 0; j < total_pixels; j++) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        if (feof(image)) break;

        cambiar_color(&r, &g, &b, mode);

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;
    }

    printf("%d\n", j);
    j = 0;
    switch (inv) {
        case 1:
            for (int i = 0; i < alto*ancho; ++i) {
                fputc(arr_in_b[i], outputImage);
                fputc(arr_in_g[i], outputImage);
                fputc(arr_in_r [i], outputImage);
                j++;
                if (j == 0){
                    j = ancho;
                }
            }
            break;
        case 2:
            for (int i = 0; i < alto; i++) {
                for (j = ancho; j > 0; j--) {
                    int index = (i*ancho) + j;
                    fputc(arr_in_b [index], outputImage);
                    fputc(arr_in_g [index], outputImage);
                    fputc(arr_in_r [index], outputImage);
                }
            }
            break;
        case 3:
                for (int i = 0; i < alto*ancho; ++i) {
                  fputc(arr_in_b[(ancho * alto - 1 - i)], outputImage);
                  fputc(arr_in_g[(ancho * alto - 1 - i)], outputImage);
                  fputc(arr_in_r[(ancho * alto - 1 - i)], outputImage);
                  j++;
                  if (j == 0){
                    j = ancho;
                  }
            }
            break;
        case 4:
            for (int i = alto; i > 0; i--) {
                for (j = 0; j < ancho; j++) {
                    int index = (i*ancho) + j;
                    fputc(arr_in_b [index], outputImage);
                    fputc(arr_in_g [index], outputImage);
                    fputc(arr_in_r [index], outputImage);
                }
            }
            break;
    }
    fclose(image);
    fclose(outputImage);
}

extern void desenfoque(const char* input_path, const char* name_output, int kernel_size, int modo) {
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

            cambiar_color(&output_rows[y][index + 2], &output_rows[y][index + 1], &output_rows[y][index + 0], modo);
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

/* extern void inv_img_color(char *mask, char *path)
{
    FILE *image, *outputImage, *lecturas;
    char add_char[80] = "./Resultados/";
    strcat(add_char, mask);
    strcat(add_char, ".bmp");
    image = fopen(path,"rb");          //Imagen original a transformar
    outputImage = fopen(add_char,"wb");    //Imagen transformada
    long ancho;
    long alto;
    unsigned char r, g, b;               //Pixel
    unsigned char* ptr;

    unsigned char xx[54];
    int cuenta = 0;
    for(int i=0; i<54; i++) {
      xx[i] = fgetc(image);
      fputc(xx[i], outputImage);   //Copia cabecera a nueva imagen
    }
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];
    printf("largo img %li\n",alto);
    printf("ancho img %li\n",ancho);

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));

    while(!feof(image)){
      b = fgetc(image);
      g = fgetc(image);
      r = fgetc(image);

      arr_in_b[cuenta] = b;
      arr_in_g[cuenta] = g;
      arr_in_r[cuenta] = r;

      cuenta++;

    }                                        //Grises
    printf("%d\n",cuenta);
    cuenta = 0;
    for (int i = 0; i < alto*ancho; ++i) {
      fputc(arr_in_b [(ancho * alto) - i], outputImage);
      fputc(arr_in_g [(ancho * alto) - i], outputImage);
      fputc(arr_in_r [(ancho * alto) - i], outputImage);
      cuenta++;
      if (cuenta == 0){
        cuenta = ancho;
      }
    }
    free(ptr);
    fclose(image);
    fclose(outputImage);
} */
