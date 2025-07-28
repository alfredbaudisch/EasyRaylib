cmake_minimum_required(VERSION 3.5)

# Handle CMake 4.0+ compatibility with older raylib CMakeLists.txt
if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
endif()

project(GameProject)

# Set policy for compatibility with older CMake versions in dependencies
cmake_policy(SET CMP0054 NEW)
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

# Build options
option(HOT_RELOAD "Build with hot reload support for debugging" OFF)

# Default mode is debug
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif ()

set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)
set(RAYLIB_PATH ${CMAKE_SOURCE_DIR}/deps/raylib)
set(GRAPHIC_API GRAPHICS_API_OPENGL_33)

# Global flags
add_definitions(-DPLATFORM_DESKTOP)
add_definitions(-D${GRAPHIC_API})
set(CMAKE_C_STANDARD 99)

# Flags for main project 
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GAME_FLAGS -flto -DNDEBUG -Wall)
else ()
    set(GAME_FLAGS -Wall -g -O0)  # Added debug flags
endif ()

# raylib
include_directories(${RAYLIB_PATH}/src)

# raylib build options
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

# Minimum version policy for raylib compatibility
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW)

# raylib subdirectory
add_subdirectory(${RAYLIB_PATH} raylib)

if(HOT_RELOAD)
    # Hot Reload Build Setup
    message(STATUS "Building with hot reload support")

    set(HOT_RELOAD_DIR ${CMAKE_BINARY_DIR}/hot_reload)
    file(MAKE_DIRECTORY ${HOT_RELOAD_DIR})
    
    # Build file version builder first
    add_executable(file_version_builder
        src/hot_reload/file_version_builder.c
        src/hot_reload/platform_tools.c
    )
    target_compile_options(file_version_builder PRIVATE ${GAME_FLAGS})
    
    # Custom command to run file version builder
    add_custom_target(generate_file_versions
        COMMAND file_version_builder
        DEPENDS file_version_builder
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating file versions"
    )
    
    # Game sources (excluding main files and hot reload utilities)
    file(GLOB_RECURSE GAME_SOURCES src/*.c)
    list(REMOVE_ITEM GAME_SOURCES 
        "${CMAKE_SOURCE_DIR}/src/main.c"
        "${CMAKE_SOURCE_DIR}/src/main_hot_reload.c"
        "${CMAKE_SOURCE_DIR}/src/hot_reload/file_version_builder.c"
        "${CMAKE_SOURCE_DIR}/src/hot_reload/platform_tools.c"
    )
    
    # Game shared library
    add_library(game_shared SHARED ${GAME_SOURCES})
    target_compile_definitions(game_shared PRIVATE -DHOT_RELOAD)
    target_compile_options(game_shared PRIVATE ${GAME_FLAGS})
    target_include_directories(game_shared PRIVATE src)
    add_dependencies(game_shared generate_file_versions)
    
    # Set output properties for the shared library
    set_target_properties(game_shared PROPERTIES
        OUTPUT_NAME "game"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${HOT_RELOAD_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${HOT_RELOAD_DIR}
    )
    
    # Platform-specific shared library settings
    if(APPLE)
        set_target_properties(game_shared PROPERTIES
            SUFFIX ".dylib"
        )
    elseif(WIN32)
        set_target_properties(game_shared PROPERTIES
            SUFFIX ".dll"
        )
    else()
        set_target_properties(game_shared PROPERTIES
            SUFFIX ".so"
        )
    endif()
    
    # Hot reload host executable
    add_executable(${PROJECT_NAME}_hot_reload
        src/main_hot_reload.c
        src/hot_reload/platform_tools.c
    )
    target_compile_definitions(${PROJECT_NAME}_hot_reload PRIVATE 
        -DHOT_RELOAD_FILE_WATCHER
        -DBUILD_TYPE_DEBUG
        -DGAME_DLL_DIR=\"build_debug/hot_reload/\"
    )
    target_compile_options(${PROJECT_NAME}_hot_reload PRIVATE ${GAME_FLAGS})
    target_include_directories(${PROJECT_NAME}_hot_reload PRIVATE src)
    
    # Platform-specific linking for hot reload executable
    if(APPLE)
        target_link_libraries(${PROJECT_NAME}_hot_reload raylib)
        target_link_libraries(${PROJECT_NAME}_hot_reload 
            "-framework CoreVideo" 
            "-framework IOKit" 
            "-framework Cocoa" 
            "-framework GLUT" 
            "-framework OpenGL"
            dl
        )
        # Export symbols for dynamic loading
        target_link_options(${PROJECT_NAME}_hot_reload PRIVATE -rdynamic)
    elseif(WIN32)
        target_link_libraries(${PROJECT_NAME}_hot_reload raylib -lopengl32 -lgdi32 -lwinmm)
    else()
        target_link_libraries(${PROJECT_NAME}_hot_reload raylib -lGL -lm -lpthread -ldl -lrt -lX11)
        # Export symbols for dynamic loading
        target_link_options(${PROJECT_NAME}_hot_reload PRIVATE -Wl,-export-dynamic)
    endif()
    
    # Set output directory for hot reload executable
    set_target_properties(${PROJECT_NAME}_hot_reload PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${HOT_RELOAD_DIR}
    )
    
    # Make sure game_shared is built before the hot reload executable
    add_dependencies(${PROJECT_NAME}_hot_reload game_shared)

else()
    # Regular Build
    message(STATUS "Building regular executable")
    
    # Game target
    file(GLOB GAME_SOURCES src/*.c)
    list(REMOVE_ITEM GAME_SOURCES "${CMAKE_SOURCE_DIR}/src/main_hot_reload.c")
    add_executable(${PROJECT_NAME} ${GAME_SOURCES})

    # Platform-specific linking
    if(APPLE)
        target_link_libraries(${PROJECT_NAME} raylib)
        target_link_libraries(${PROJECT_NAME} 
            "-framework CoreVideo" 
            "-framework IOKit" 
            "-framework Cocoa" 
            "-framework GLUT" 
            "-framework OpenGL"
        )
    elseif(WIN32)
        target_link_libraries(${PROJECT_NAME} raylib -lopengl32 -lgdi32 -lwinmm)
    else()
        target_link_libraries(${PROJECT_NAME} raylib -lGL -lm -lpthread -ldl -lrt -lX11)
    endif()

    target_compile_options(${PROJECT_NAME} PRIVATE ${GAME_FLAGS})

    # Copy resources
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)

endif()

# Reduce file size in release mode
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if(NOT APPLE) # strip doesn't work the same way on macOS
        if(HOT_RELOAD)
            add_custom_command(TARGET ${PROJECT_NAME}_hot_reload POST_BUILD
                    COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}_hot_reload>)
            add_custom_command(TARGET game_shared POST_BUILD
                    COMMAND ${CMAKE_STRIP} $<TARGET_FILE:game_shared>)
        else()
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>)
        endif()
    endif()
endif ()

# Platform-specific settings
if (WIN32)
    if(HOT_RELOAD)
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}_hot_reload)
    else()
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif()
endif ()

# Print build information
message(STATUS "Building for: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Hot reload: ${HOT_RELOAD}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
if(HOT_RELOAD)
    message(STATUS "Hot reload executable will be: ${HOT_RELOAD_DIR}/${PROJECT_NAME}_hot_reload")
    message(STATUS "Game shared library will be: ${HOT_RELOAD_DIR}/game")
else()
    message(STATUS "Executable will be: $<TARGET_FILE:${PROJECT_NAME}>")
endif()
