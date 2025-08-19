// utils/file_utils.h
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

char** find_png_files_in_user_pictures(int* count);
void free_file_list(char** file_list, int count);

#endif // FILE_UTILS_H