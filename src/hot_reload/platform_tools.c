#include "platform_tools.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <direct.h>
#else
    #include <dlfcn.h>
    #include <unistd.h>
    #include <sys/stat.h>
#endif

// Cross-platform dynamic library loading
dll_handle_t platform_load_library(const char* path) {
#ifdef _WIN32
    HMODULE handle = LoadLibraryA(path);
    return (void*)handle;
#else
    return dlopen(path, RTLD_LAZY);
#endif
}

dll_symbol_t platform_get_symbol(dll_handle_t handle, const char* symbol) {
#ifdef _WIN32
    HMODULE win_handle = (HMODULE)handle;
    FARPROC proc = GetProcAddress(win_handle, symbol);
    return (void*)proc;
#else
    return dlsym(handle, symbol);
#endif
}

void platform_free_library(dll_handle_t handle) {
#ifdef _WIN32
    if (handle) {
        HMODULE win_handle = (HMODULE)handle;
        FreeLibrary(win_handle);
    }
#else
    if (handle) dlclose(handle);
#endif
}

// Cross-platform file operations
time_t platform_get_modification_time(const char* path) {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &fileInfo) == 0) {
        return 0;
    }
    
    ULARGE_INTEGER ull;
    ull.LowPart = fileInfo.ftLastWriteTime.dwLowDateTime;
    ull.HighPart = fileInfo.ftLastWriteTime.dwHighDateTime;
    
    // Convert to time_t (seconds since epoch)
    return (time_t)(ull.QuadPart / 10000000ULL - 11644473600ULL);
#else
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return st.st_mtime;
#endif
}

bool platform_delete_file(const char* path) {
#ifdef _WIN32
    return DeleteFileA(path) != 0;
#else
    return unlink(path) == 0;
#endif
}

void platform_create_directory(const char* path) {
#ifdef _WIN32
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "if not exist \"%s\" mkdir \"%s\"", path, path);
    system(cmd);
#else
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", path);
    system(cmd);
#endif
}
