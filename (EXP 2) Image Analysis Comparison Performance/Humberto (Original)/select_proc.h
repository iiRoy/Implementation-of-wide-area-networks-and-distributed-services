#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void inv_img(const char *mask, const char *path){
    FILE *image, *outputImage, *lecturas, *fptr;

    char output_path[100];
    snprintf(output_path, sizeof(output_path), "./Results/%s.bmp", mask);
    printf("%s\n", output_path);

    image = fopen(path,"rb");
    outputImage = fopen(output_path, "wb");

    if(!image || !outputImage){
        fprintf(stderr, "Error opening file\n");
        return;
    }

    int i, j, k;
    long ancho, tam, bpp, alto;
    unsigned char r,g,b, pixel;

    unsigned char xx[54];
    for(int i=0; i<54; i++){
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    tam = (long)xx[4]*65536+(long)xx[3]*256+(long)xx[2];
    bpp = (long)xx[29]*256+(long)xx[28];
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));

    j = 0;

    int total_pixels = ancho * alto;

    for (j = 0; j < total_pixels; j++) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        if (feof(image)) break;

        unsigned char pixel = 0.21*r+0.72*g+0.07*b;
        arr_in_b[j] = pixel;
        arr_in_g[j] = pixel;
        arr_in_r[j] = pixel;
    }

    printf("Lectura de datos: %d\n", j*3);
    printf("Elementos faltantes: %d\n", tam - (j*3));

    for(int i = 0; i < ancho*alto; i++){

        fputc(arr_in_b[(ancho * alto - 1) - i], outputImage);
        fputc(arr_in_g[(ancho * alto - 1) - i], outputImage);
        fputc(arr_in_r[(ancho * alto - 1) - i], outputImage);
    }

    free(arr_in_b);
    free(arr_in_g);
    free(arr_in_r);

    fclose(image);
    fclose(outputImage);
}

extern void inv_img_color(const char *mask, const char *path){
    FILE *image, *outputImage, *lecturas, *fptr;

    char output_path[100];
    snprintf(output_path, sizeof(output_path), "./Results/%s.bmp", mask);
    printf("%s\n", output_path);

    image = fopen(path,"rb");
    outputImage = fopen(output_path, "wb");

    if(!image || !outputImage){
        fprintf(stderr, "Error opening file\n");
        return;
    }

    int i, j, k;
    long ancho, tam, bpp, alto;
    unsigned char r,g,b, pixel;

    unsigned char xx[54];
    for(int i=0; i<54; i++){
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    tam = (long)xx[4]*65536+(long)xx[3]*256+(long)xx[2];
    bpp = (long)xx[29]*256+(long)xx[28];
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));

    j = 0;

    int total_pixels = ancho * alto;

    for (j = 0; j < total_pixels; j++) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        if (feof(image)) break;

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;
    }

    printf("Lectura de datos: %d\n", j*3);
    printf("Elementos faltantes: %d\n", tam - (j*3));

    for(int i = 0; i < ancho*alto; i++){

        fputc(arr_in_b[(ancho * alto - 1) - i], outputImage);
        fputc(arr_in_g[(ancho * alto - 1) - i], outputImage);
        fputc(arr_in_r[(ancho * alto - 1) - i], outputImage);
    }

    free(arr_in_b);
    free(arr_in_g);
    free(arr_in_r);

    fclose(image);
    fclose(outputImage);
}

extern void fliph(const char *mask, const char *path){
    FILE *image, *outputImage, *lecturas, *fptr;

    char output_path[100];
    snprintf(output_path, sizeof(output_path), "./Results/%s.bmp", mask);
    printf("%s\n", output_path);

    image = fopen(path,"rb");
    outputImage = fopen(output_path, "wb");

    if(!image || !outputImage){
        fprintf(stderr, "Error opening file\n");
        return;
    }

    int i, j, k;
    long ancho, tam, bpp, alto;
    unsigned char r,g,b, pixel;

    unsigned char xx[54];
    for(int i=0; i<54; i++){
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    tam = (long)xx[4]*65536+(long)xx[3]*256+(long)xx[2];
    bpp = (long)xx[29]*256+(long)xx[28];
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));

    j = 0;

    int total_pixels = ancho * alto;

    for (j = 0; j < total_pixels; j++) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        if (feof(image)) break;

        unsigned char pixel = 0.21*r+0.72*g+0.07*b;
        arr_in_b[j] = pixel;
        arr_in_g[j] = pixel;
        arr_in_r[j] = pixel;
    }

    printf("Lectura de datos: %d\n", j*3);
    printf("Elementos faltantes: %d\n", tam - (j*3));

    for(int row = 0; row < alto ; row++){
        for (int column = ancho - 1; column >= 0; column--) {
            int idx = (ancho * row) + column;
            fputc(arr_in_b[idx], outputImage);
            fputc(arr_in_g[idx], outputImage);
            fputc(arr_in_r[idx], outputImage);
        }
    }

    free(arr_in_b);
    free(arr_in_g);
    free(arr_in_r);

    fclose(image);
    fclose(outputImage);
}

extern void flipv(const char *mask, const char *path){
    FILE *image, *outputImage, *lecturas, *fptr;

    char output_path[100];
    snprintf(output_path, sizeof(output_path), "./Results/%s.bmp", mask);
    printf("%s\n", output_path);

    image = fopen(path,"rb");
    outputImage = fopen(output_path, "wb");

    if(!image || !outputImage){
        fprintf(stderr, "Error opening file\n");
        return;
    }

    int i, j, k;
    long ancho, tam, bpp, alto;
    unsigned char r,g,b, pixel;

    unsigned char xx[54];
    for(int i=0; i<54; i++){
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    tam = (long)xx[4]*65536+(long)xx[3]*256+(long)xx[2];
    bpp = (long)xx[29]*256+(long)xx[28];
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));

    j = 0;

    int total_pixels = ancho * alto;

    for (j = 0; j < total_pixels; j++) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        if (feof(image)) break;

        unsigned char pixel = 0.21*r+0.72*g+0.07*b;
        arr_in_b[j] = pixel;
        arr_in_g[j] = pixel;
        arr_in_r[j] = pixel;
    }

    printf("Lectura de datos: %d\n", j*3);
    printf("Elementos faltantes: %d\n", tam - (j*3));

    for (int row = alto - 1; row >= 0; row--) {
        for (int column = 0; column < ancho; column++) {
            int idx = (ancho * row) + column;
            fputc(arr_in_b[idx], outputImage);
            fputc(arr_in_g[idx], outputImage);
            fputc(arr_in_r[idx], outputImage);
        }
    }

    free(arr_in_b);
    free(arr_in_g);
    free(arr_in_r);

    fclose(image);
    fclose(outputImage);
}

extern void flipv_color(const char *mask, const char *path){
    FILE *image, *outputImage, *lecturas, *fptr;

    char output_path[100];
    snprintf(output_path, sizeof(output_path), "./Results/%s.bmp", mask);
    printf("%s\n", output_path);

    image = fopen(path,"rb");
    outputImage = fopen(output_path, "wb");

    if(!image || !outputImage){
        fprintf(stderr, "Error opening file\n");
        return;
    }

    int i, j, k;
    long ancho, tam, bpp, alto;
    unsigned char r,g,b, pixel;

    unsigned char xx[54];
    for(int i=0; i<54; i++){
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    tam = (long)xx[4]*65536+(long)xx[3]*256+(long)xx[2];
    bpp = (long)xx[29]*256+(long)xx[28];
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));

    j = 0;

    int total_pixels = ancho * alto;

    for (j = 0; j < total_pixels; j++) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        if (feof(image)) break;

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;
    }

    printf("Lectura de datos: %d\n", j*3);
    printf("Elementos faltantes: %d\n", tam - (j*3));

    for (int row = alto - 1; row >= 0; row--) {
        for (int column = 0; column < ancho; column++) {
            int idx = (ancho * row) + column;
            fputc(arr_in_b[idx], outputImage);
            fputc(arr_in_g[idx], outputImage);
            fputc(arr_in_r[idx], outputImage);
        }
    }

    free(arr_in_b);
    free(arr_in_g);
    free(arr_in_r);

    fclose(image);
    fclose(outputImage);
}

extern void fliph_color(const char *mask, const char *path){
    FILE *image, *outputImage, *lecturas, *fptr;

    char output_path[100];
    snprintf(output_path, sizeof(output_path), "./Results/%s.bmp", mask);
    printf("%s\n", output_path);

    image = fopen(path,"rb");
    outputImage = fopen(output_path, "wb");

    if(!image || !outputImage){
        fprintf(stderr, "Error opening file\n");
        return;
    }

    int i, j, k;
    long ancho, tam, bpp, alto;
    unsigned char r,g,b, pixel;

    unsigned char xx[54];
    for(int i=0; i<54; i++){
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    tam = (long)xx[4]*65536+(long)xx[3]*256+(long)xx[2];
    bpp = (long)xx[29]*256+(long)xx[28];
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));

    j = 0;

    int total_pixels = ancho * alto;

    for (j = 0; j < total_pixels; j++) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        if (feof(image)) break;

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;
    }

    printf("Lectura de datos: %d\n", j*3);
    printf("Elementos faltantes: %d\n", tam - (j*3));

    for(int row = 0; row < alto ; row++){
        for (int column = ancho - 1; column >= 0; column--) {
            int idx = (ancho * row) + column;
            fputc(arr_in_b[idx], outputImage);
            fputc(arr_in_g[idx], outputImage);
            fputc(arr_in_r[idx], outputImage);
        }
    }

    free(arr_in_b);
    free(arr_in_g);
    free(arr_in_r);

    fclose(image);
    fclose(outputImage);
}

/* Imprime la imagen en una orientación invertida
for(int i = 0; i < ancho*alto; i++){

        fputc(arr_in_b [(ancho * alto) - i], outputImage);
        fputc(arr_in_g [(ancho * alto) - i], outputImage);
        fputc(arr_in_r [(ancho * alto) - i], outputImage);
    }
*/

/* Imprime la imagen en su orientación natural
for(int i = ancho*alto; i > 0 ; i--){
        fputc(arr_in_b [(ancho * alto) - i], outputImage);
        fputc(arr_in_g [(ancho * alto) - i], outputImage);
        fputc(arr_in_r [(ancho * alto) - i], outputImage);
    }
*/

void blur(const char *input_path, const char *name_output, int kernel_size)
{
    FILE *image, *outputImage;
    char output_path[100];
    snprintf(output_path, sizeof(output_path), "./Results/%s.bmp", name_output);
    image = fopen(input_path, "rb");
    outputImage = fopen(output_path, "wb");

    if (!image || !outputImage)
    {
        printf("Error abriendo archivos.\n");
        return;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, image);
    fwrite(header, sizeof(unsigned char), 54, outputImage);

    int width = *(int *)&header[18];
    int height = *(int *)&header[22];
    int row_padded = (width * 3 + 3) & (~3);

    unsigned char **input_rows = (unsigned char **)malloc(height * sizeof(unsigned char *));
    unsigned char **output_rows = (unsigned char **)malloc(height * sizeof(unsigned char *));
    unsigned char **temp_rows = (unsigned char **)malloc(height * sizeof(unsigned char *));

    for (int i = 0; i < height; i++)
    {
        input_rows[i] = (unsigned char *)malloc(row_padded);
        output_rows[i] = (unsigned char *)malloc(row_padded);
        temp_rows[i] = (unsigned char *)malloc(row_padded);
        fread(input_rows[i], sizeof(unsigned char), row_padded, image);
    }

    int k = kernel_size / 2;

    // Horizontal blur pass
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int sumB = 0, sumG = 0, sumR = 0, count = 0;

            for (int dx = -k; dx <= k; dx++)
            {
                int nx = x + dx;
                if (nx >= 0 && nx < width)
                {
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

        for (int p = width * 3; p < row_padded; p++)
        {
            temp_rows[y][p] = input_rows[y][p];
        }
    }

    // Vertical blur pass + grayscale conversion
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int sumB = 0, sumG = 0, sumR = 0, count = 0;

            for (int dy = -k; dy <= k; dy++)
            {
                int ny = y + dy;
                if (ny >= 0 && ny < height)
                {
                    int idx = x * 3;
                    sumB += temp_rows[ny][idx + 0];
                    sumG += temp_rows[ny][idx + 1];
                    sumR += temp_rows[ny][idx + 2];
                    count++;
                }
            }

            int index = x * 3;
            unsigned char blurB = sumB / count;
            unsigned char blurG = sumG / count;
            unsigned char blurR = sumR / count;
            unsigned char gray = 0.21*blurR + 0.72*blurG + 0.07*blurB;
            output_rows[y][index + 0] = gray;
            output_rows[y][index + 1] = gray;
            output_rows[y][index + 2] = gray;
        }

        for (int p = width * 3; p < row_padded; p++)
        {
            output_rows[y][p] = temp_rows[y][p];
        }
    }

    // Write output
    for (int i = 0; i < height; i++)
    {
        fwrite(output_rows[i], sizeof(unsigned char), row_padded, outputImage);
        free(input_rows[i]);
        free(temp_rows[i]);
        free(output_rows[i]);
    }

    FILE *outputLog = fopen("output_log.txt", "a");
    if (outputLog == NULL)
    {
        fprintf(stderr, "Error: No se pudo crear o abrir el archivo de registro.\n");
        fclose(image);
        fclose(outputImage);
        return;
    }

    fprintf(outputLog, "Función: blur byn, con %s\n", input_path);
    fprintf(outputLog, "Localidades totales leídas: %d\n", width * height);
    fprintf(outputLog, "Localidades totales escritas: %d\n", width * height);
    fprintf(outputLog, "-------------------------------------\n");
    fclose(outputLog);

    free(input_rows);
    free(temp_rows);
    free(output_rows);
    fclose(image);
    fclose(outputImage);
}

void blur_color(const char *input_path, const char *name_output, int kernel_size)
{
    FILE *image, *outputImage;
    char output_path[100];
    snprintf(output_path, sizeof(output_path), "./Results/%s.bmp", name_output);
    image = fopen(input_path, "rb");
    outputImage = fopen(output_path, "wb");

    if (!image || !outputImage)
    {
        printf("Error abriendo archivos.\n");
        return;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, image);
    fwrite(header, sizeof(unsigned char), 54, outputImage);

    int width = *(int *)&header[18];
    int height = *(int *)&header[22];
    int row_padded = (width * 3 + 3) & (~3);

    unsigned char **input_rows = (unsigned char **)malloc(height *sizeof(unsigned char *));
    unsigned char **output_rows = (unsigned char **)malloc(height *sizeof(unsigned char *));
    unsigned char **temp_rows = (unsigned char **)malloc(height *sizeof(unsigned char *));

    for (int i = 0; i < height; i++)
    {
        input_rows[i] = (unsigned char *)malloc(row_padded);
        output_rows[i] = (unsigned char *)malloc(row_padded);
        temp_rows[i] = (unsigned char *)malloc(row_padded);
        fread(input_rows[i], sizeof(unsigned char), row_padded, image);
    }

    int k = kernel_size / 2;
    // Paso intermedio: desenfoque horizontal
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int sumB = 0, sumG = 0, sumR = 0, count = 0;

            for (int dx = -k; dx <= k; dx++)
            {
                int nx = x + dx;
                if (nx >= 0 && nx < width)
                {
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
        for (int p = width * 3; p < row_padded; p++)
        {
            temp_rows[y][p] = input_rows[y][p];
        }
    }

    // Paso final: desenfoque vertical
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int sumB = 0, sumG = 0, sumR = 0, count = 0;
            for (int dy = -k; dy <= k; dy++)
            {
                int ny = y + dy;
                if (ny >= 0 && ny < height)
                {
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
        }
        // Copiar padding sin modificar
        for (int p = width * 3; p < row_padded; p++)
        {
            output_rows[y][p] = temp_rows[y][p];
        }
    }
    // Escritura final
    for (int i = 0; i < height; i++)
    {
        fwrite(output_rows[i], sizeof(unsigned char), row_padded,
               outputImage);
        free(input_rows[i]);
        free(temp_rows[i]);
        free(output_rows[i]);
    }
    // Escritura en archivo de registro
    FILE *outputLog = fopen("output_log.txt", "a");
    if (outputLog == NULL)
    {
        fprintf(stderr, "Error: No se pudo crear o abrir el archivo deregistro.\n");
        fclose(image);
        fclose(outputImage);
        return;
    }

    fprintf(outputLog, "Función: desenfoque, con %s\n", input_path);
    fprintf(outputLog, "Localidades totales leídas: %d\n", width * height);
    fprintf(outputLog, "Localidades totales escritas: %d\n", width * height);
    fprintf(outputLog, "-------------------------------------\n");
    fclose(outputLog);
    free(input_rows);
    free(temp_rows);
    free(output_rows);
    fclose(image);
    fclose(outputImage);
}
