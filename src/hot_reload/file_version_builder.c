#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "platform_tools.h"

#define MAX_FILES 256

typedef struct {
    char path[512];
    time_t modification_time;
} FileVersion;

int should_ignore_file(const char* filename) {
    return strcmp(filename, "main.c") == 0 ||
           strcmp(filename, "main_hot_reload.c") == 0;
}

int should_ignore_directory(const char* dirname) {
    return strcmp(dirname, "hot_reload") == 0;
}

int has_extension(const char* filename, const char* ext) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    return strcmp(dot, ext) == 0;
}

int is_directory(const char* path) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
#endif
}

int scan_directory_recursive(const char* dir_path, FileVersion** versions, int* version_count, int* capacity) {
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    HANDLE hFind;
    char search_path[512];
    char full_path[512];
    
    snprintf(search_path, sizeof(search_path), "%s\\*", dir_path);
    
    hFind = FindFirstFileA(search_path, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s\\%s", dir_path, findData.cFileName);
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (should_ignore_directory(findData.cFileName)) {
                continue;
            }

            // Recursively scan subdirectory
            scan_directory_recursive(full_path, versions, version_count, capacity);
        } else {
            // Process file
            if (should_ignore_file(findData.cFileName)) {
                continue;
            }
            
            if (!has_extension(findData.cFileName, ".c") && !has_extension(findData.cFileName, ".h")) {
                continue;
            }
            
            time_t mod_time = platform_get_modification_time(full_path);
            if (mod_time != 0) {
                // Expand array if needed
                if (*version_count >= *capacity) {
                    *capacity *= 2;
                    *versions = realloc(*versions, (*capacity) * sizeof(FileVersion));
                    if (!*versions) {
                        fprintf(stderr, "[FILE_VERSIONS] Error: Could not reallocate memory\n");
                        FindClose(hFind);
                        return -1;
                    }
                }
                
                strncpy((*versions)[*version_count].path, full_path, sizeof((*versions)[*version_count].path) - 1);
                (*versions)[*version_count].path[sizeof((*versions)[*version_count].path) - 1] = '\0';
                (*versions)[*version_count].modification_time = mod_time;
                (*version_count)++;
            }
        }
    } while (FindNextFileA(hFind, &findData));
    
    FindClose(hFind);
    
#else
    DIR *dir;
    struct dirent *entry;
    char full_path[512];
    
    dir = opendir(dir_path);
    if (dir == NULL) {
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        if (is_directory(full_path)) {
            if (should_ignore_directory(entry->d_name)) {
                continue;
            }

            // Recursively scan subdirectory
            scan_directory_recursive(full_path, versions, version_count, capacity);
        } else {
            // Process file
            if (should_ignore_file(entry->d_name)) {
                continue;
            }
            
            if (!has_extension(entry->d_name, ".c") && !has_extension(entry->d_name, ".h")) {
                continue;
            }
            
            time_t mod_time = platform_get_modification_time(full_path);
            if (mod_time != 0) {
                // Expand array if needed
                if (*version_count >= *capacity) {
                    *capacity *= 2;
                    *versions = realloc(*versions, (*capacity) * sizeof(FileVersion));
                    if (!*versions) {
                        fprintf(stderr, "[FILE_VERSIONS] Error: Could not reallocate memory\n");
                        closedir(dir);
                        return -1;
                    }
                }
                
                strncpy((*versions)[*version_count].path, full_path, sizeof((*versions)[*version_count].path) - 1);
                (*versions)[*version_count].path[sizeof((*versions)[*version_count].path) - 1] = '\0';
                (*versions)[*version_count].modification_time = mod_time;
                (*version_count)++;
            }
        }
    }
    closedir(dir);
#endif
    
    return 0;
}

int main() {
    FILE *data_file;
    FileVersion *versions = NULL;
    int version_count = 0;
    int capacity = MAX_FILES;
    
    versions = malloc(capacity * sizeof(FileVersion));
    if (!versions) {
        fprintf(stderr, "[FILE_VERSIONS] Error: Could not allocate memory\n");
        return 1;
    }

    if (scan_directory_recursive("src", &versions, &version_count, &capacity) < 0) {
        free(versions);
        return 1;
    }

    data_file = fopen("src/hot_reload/file_versions.dat", "w");
    if (data_file == NULL) {
        fprintf(stderr, "[FILE_VERSIONS] Error: Could not create file_versions.dat\n");
        free(versions);
        return 1;
    }
    
    fprintf(data_file, "%d\n", version_count);
    for (int i = 0; i < version_count; i++) {
        fprintf(data_file, "%s %ld\n", versions[i].path, (long)versions[i].modification_time);
    }
    fclose(data_file);
    
    free(versions);
    
    printf("[FILE_VERSIONS] Generated file_versions.dat with %d files\n", version_count);
    return 0;
}
