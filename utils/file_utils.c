// utils/file_utils.c
#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <ShlObj.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <strings.h> // Para strcasecmp
#endif

static char** add_to_list(char** list, int* count, const char* path) {
    char** new_list = (char**)realloc(list, (*count + 1) * sizeof(char*));
    if (!new_list) {
        fprintf(stderr, "Erro de alocação de memória!\n");
        return list;
    }
    new_list[*count] = (char*)malloc(strlen(path) + 1);
    if (!new_list[*count]) {
        fprintf(stderr, "Erro de alocação de memória!\n");
        return new_list;
    }
    strcpy(new_list[*count], path);
    (*count)++;
    return new_list;
}

#ifdef _WIN32

void scan_directory_recursive(const WCHAR* path, char*** list, int* count) {
    WIN32_FIND_DATAW find_data;
    WCHAR search_path[MAX_PATH];
    swprintf(search_path, MAX_PATH, L"%s\\*", path);

    HANDLE hFind = FindFirstFileW(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (wcscmp(find_data.cFileName, L".") == 0 || wcscmp(find_data.cFileName, L"..") == 0) continue;
        if (*count >= 200) { FindClose(hFind); return; } // Limite para não explodir a memória

        WCHAR full_path[MAX_PATH];
        swprintf(full_path, MAX_PATH, L"%s\\%s", path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_directory_recursive(full_path, list, count);
        } else {
            const WCHAR* ext = wcsrchr(find_data.cFileName, L'.');
            if (ext && (_wcsicmp(ext, L".png") == 0)) {
                char mb_path[MAX_PATH * 4] = {0};
                WideCharToMultiByte(CP_UTF8, 0, full_path, -1, mb_path, sizeof(mb_path), NULL, NULL);
                *list = add_to_list(*list, count, mb_path);
            }
        }
    } while (FindNextFileW(hFind, &find_data) != 0);

    FindClose(hFind);
}

char** find_png_files_in_user_pictures(int* count) {
    *count = 0;
    char** file_list = NULL;
    
    PWSTR pictures_path_w = NULL;
    HRESULT hr = SHGetKnownFolderPath(&FOLDERID_Pictures, 0, NULL, &pictures_path_w);

    if (SUCCEEDED(hr)) {
        printf("Procurando imagens em: %ls\n", pictures_path_w);
        scan_directory_recursive(pictures_path_w, &file_list, count);
        CoTaskMemFree(pictures_path_w);
    } else {
        fprintf(stderr, "Erro: Não foi possível encontrar a pasta 'Imagens'.\n");
    }
    
    return file_list;
}

#else

void scan_directory_recursive(const char* path, char*** list, int* count) {
    DIR* dir = opendir(path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (*count >= 200) { closedir(dir); return; } // Limite para não explodir a memória

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat entry_stat;
        if (stat(full_path, &entry_stat) == -1) continue;

        if (S_ISDIR(entry_stat.st_mode)) {
            scan_directory_recursive(full_path, list, count);
        } else {
            const char* ext = strrchr(entry->d_name, '.');
            if (ext && (strcasecmp(ext, ".png") == 0)) {
                *list = add_to_list(*list, count, full_path);
            }
        }
    }
    closedir(dir);
}

char** find_png_files_in_user_pictures(int* count) {
    *count = 0;
    char** file_list = NULL;

    const char* home_dir = getenv("HOME");
    if (home_dir) {
        char pictures_path[1024] = {0};
        FILE *fp = popen("xdg-user-dir PICTURES 2>/dev/null", "r");
        if(fp && fgets(pictures_path, sizeof(pictures_path)-1, fp) != NULL && strlen(pictures_path) > 1) {
            pictures_path[strcspn(pictures_path, "\n")] = 0;
        } else {
            snprintf(pictures_path, sizeof(pictures_path), "%s/Pictures", home_dir);
        }
        if(fp) pclose(fp);
        
        printf("Procurando imagens em: %s\n", pictures_path);
        scan_directory_recursive(pictures_path, &file_list, count);
    } else {
         fprintf(stderr, "Erro: Não foi possível encontrar a pasta 'Home' do usuário.\n");
    }
    
    return file_list;
}

#endif

void free_file_list(char** file_list, int count) {
    if (!file_list) return;
    for (int i = 0; i < count; i++) {
        if (file_list[i]) free(file_list[i]);
    }
    free(file_list);
}