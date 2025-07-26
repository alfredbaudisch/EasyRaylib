#ifndef HOT_RELOAD_PLATFORM_TOOLS_H
#define HOT_RELOAD_PLATFORM_TOOLS_H

#include <time.h>
#include <stdbool.h>

// Opaque handle types (to avoid windows.h, which conflicts with raylib)
typedef void* dll_handle_t;
typedef void* dll_symbol_t;

dll_handle_t platform_load_library(const char* path);
dll_symbol_t platform_get_symbol(dll_handle_t handle, const char* symbol);
void platform_free_library(dll_handle_t handle);

time_t platform_get_modification_time(const char* path);
bool platform_delete_file(const char* path);
void platform_create_directory(const char* path);

#endif // HOT_RELOAD_PLATFORM_TOOLS_H