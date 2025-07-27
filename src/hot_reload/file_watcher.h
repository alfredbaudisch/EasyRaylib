#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "platform_tools.h"

typedef struct {
    const char* path;
    time_t modification_time;
} FileVersion;

FileVersion *file_watcher;
int file_watcher_count;

bool file_watcher_reload() {
    FILE *file = fopen("src/hot_reload/file_versions.dat", "r");
    if (!file) {
        printf("[FILE_WATCHER] Failed to open file_versions.dat\n");
        return false;
    }
    
    int new_count;
    if (fscanf(file, "%d\n", &new_count) != 1) {
        printf("[FILE_WATCHER] Failed to read count from data file\n");
        fclose(file);
        return false;
    }

    if(new_count != file_watcher_count) {
        free(file_watcher);
        file_watcher = (FileVersion*)malloc(new_count * sizeof(FileVersion));
        file_watcher_count = new_count;
    }

    for (int i = 0; i < new_count; i++) {
        char path[512];
        long long mod_time_ll;
        
        if (fscanf(file, "%511s %lld\n", path, &mod_time_ll) != 2) {
            printf("[FILE_WATCHER] Failed to read file entry %d\n", i);
            fclose(file);  
            return false;
        }

        file_watcher[i].path = strdup(path);
        file_watcher[i].modification_time = (time_t)mod_time_ll;
    }

    fclose(file);
    return true;
}

bool file_watcher_check() {
    for (int i = 0; i < file_watcher_count; i++) {
        time_t mod_time = platform_get_modification_time(file_watcher[i].path);
        if (mod_time == 0) {
            printf("[FILE_WATCHER] Failed getting modification time of %s, maybe deleted? Rebuilding file versions and hot reloading...\n", file_watcher[i].path);

#ifdef _WIN32
            int build_result = system("build_hot_reload.bat");
#else
            int build_result = system("./build_hot_reload.sh");
#endif
            if (build_result != 0) {
                printf("[FILE_WATCHER] Build failed with exit code: %d\n", build_result);
                exit(1);
                return false;
            }
        }
        if (mod_time != file_watcher[i].modification_time) {
            printf("[FILE_WATCHER] File %s has changed (disk: %lld, memory: %lld)\n", 
                   file_watcher[i].path, (long long)mod_time, (long long)file_watcher[i].modification_time);
            return true;
        }
    }
    return false;
}
