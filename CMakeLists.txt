cmake_minimum_required(VERSION 3.10)

project(GameProject)

# Default mode is debug
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif ()

# Variables
set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)
set(RAYLIB_PATH ${CMAKE_SOURCE_DIR}/deps/raylib)
set(GRAPHIC_API GRAPHICS_API_OPENGL_33)

# Global flags
add_definitions(-DPLATFORM_DESKTOP)
add_definitions(-D${GRAPHIC_API})
set(CMAKE_C_STANDARD 99)

# Flags for raylib and main project 
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    # -march=native is optimized to run in the machine it was compiled with
    set(RAYLIB_FLAGS -O3 -DNDEBUG -flto -Wall)
    set(GAME_FLAGS -flto -DNDEBUG -Wall)
else ()
    set(RAYLIB_FLAGS -Wall)
    set(GAME_FLAGS -Wall)
endif ()

include_directories(${RAYLIB_PATH}/src)

# Compile Raylib
file(GLOB RAYLIB_SOURCES ${RAYLIB_PATH}/src/*.c)
add_library(raylib STATIC ${RAYLIB_SOURCES})
target_include_directories(raylib PRIVATE ${RAYLIB_PATH}/src/external/glfw/include)
target_compile_options(raylib PRIVATE ${RAYLIB_FLAGS})

# Game target
file(GLOB GAME_SOURCES src/*.c)
add_executable(${PROJECT_NAME} ${GAME_SOURCES})
target_link_libraries(${PROJECT_NAME} raylib -lopengl32 -lgdi32 -lwinmm)
target_compile_options(${PROJECT_NAME} PRIVATE ${GAME_FLAGS})

# Reduce file size
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>)
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)

# On Windows, this might be necessary
if (WIN32)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif ()
