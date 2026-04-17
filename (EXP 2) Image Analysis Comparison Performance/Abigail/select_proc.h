#ifndef SELECT_PROC_H
#define SELECT_PROC_H

void inv_img(const char *mask, const char *path);
void inv_img_color(const char *mask, const char *path);
void fliph(const char *mask, const char *path);
void flipv(const char *mask, const char *path);
void flipv_color(const char *mask, const char *path);
void fliph_color(const char *mask, const char *path);
void blur(const char* input_path, const char* name_output, int kernel_size);
void blur_color(const char* input_path, const char* name_output, int kernel_size);

#endif