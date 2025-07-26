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

# Flags for main project 
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GAME_FLAGS -flto -DNDEBUG -Wall)
else ()
    set(GAME_FLAGS -Wall)
endif ()

# Include raylib
include_directories(${RAYLIB_PATH}/src)

# Build raylib using its own CMakeLists.txt
# Configure raylib build options
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)    # don't build the supplied example games

# Set minimum version policy for raylib compatibility
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW)

# Add raylib subdirectory
add_subdirectory(${RAYLIB_PATH} raylib)

# Game target
file(GLOB GAME_SOURCES src/*.c)
list(REMOVE_ITEM GAME_SOURCES "${CMAKE_SOURCE_DIR}/src/main_hot_reload.c")
add_executable(${PROJECT_NAME} ${GAME_SOURCES})

# Platform-specific linking
if(APPLE)
    # macOS frameworks
    target_link_libraries(${PROJECT_NAME} raylib)
    target_link_libraries(${PROJECT_NAME} 
        "-framework CoreVideo" 
        "-framework IOKit" 
        "-framework Cocoa" 
        "-framework GLUT" 
        "-framework OpenGL"
    )
elseif(WIN32)
    # Windows libraries
    target_link_libraries(${PROJECT_NAME} raylib -lopengl32 -lgdi32 -lwinmm)
else()
    # Linux/other Unix systems
    target_link_libraries(${PROJECT_NAME} raylib -lGL -lm -lpthread -ldl -lrt -lX11)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE ${GAME_FLAGS})

# Reduce file size in release mode
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if(NOT APPLE) # strip doesn't work the same way on macOS
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>)
    endif()
endif ()

# Copy resources
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)

# Platform-specific settings
if (WIN32)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif ()

# Print build information
message(STATUS "Building for: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Executable will be: $<TARGET_FILE:${PROJECT_NAME}>")
