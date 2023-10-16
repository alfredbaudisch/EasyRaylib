# Default mode is debug/dev
# Call with: `make` (when empty defaults to DEBUG) or `make MODE=RELEASE`
MODE ?= DEBUG

# =====================================
# Variables
# =====================================
# OS-specific commands
RM = rm -f
RMDIR = rm -rf
MKDIR = mkdir -p

BUILD_DIR := $(CURDIR)/build
RAYLIB_PATH = ./deps/raylib
COMPILER_PATH = C:/Dev/msys2/mingw64
CC = gcc
GRAPHIC_API = GRAPHICS_API_OPENGL_33

ifeq ($(MODE),RELEASE)
    RAYLIB_CFLAGS = -std=c99 -O3 -march=native -DNDEBUG -flto -Wall -DPLATFORM_DESKTOP -D$(GRAPHIC_API) -I$(RAYLIB_PATH)/src -Iexternal
    CFLAGS = -std=c99 -Wall -DPLATFORM_DESKTOP -D$(GRAPHIC_API) -I$(RAYLIB_PATH)/src -Iexternal
else
    CFLAGS = -std=c99 -Wall -DPLATFORM_DESKTOP -D$(GRAPHIC_API) -I$(RAYLIB_PATH)/src -Iexternal
    RAYLIB_CFLAGS = $(CFLAGS)
endif

LDFLAGS = -L$(RELEASE_PATH)/lib -lraylib -lopengl32 -lgdi32 -lwinmm
RELEASE_PATH = $(BUILD_DIR)
CURRENT_DIRECTORY = .
EXECUTABLE_NAME = game

# List all .c files in the src subfolder
SOURCES = $(wildcard src/*.c)
# Generate a list of object files from source files
OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.c=.o)))
# =====================================

# =====================================
# Targets
# =====================================
all: raylib game

raylib:
	@echo "> Compiling raylib..."
	@$(MKDIR) $(BUILD_DIR)/include
	@$(MKDIR) $(BUILD_DIR)/lib
	@cd $(RAYLIB_PATH)/src && \
	$(RM) *.o && \
	$(RM) libraylib.a && \
	$(CC) -c rcore.c -Iexternal/glfw/include $(RAYLIB_CFLAGS) && \
	$(CC) -c rglfw.c $(RAYLIB_CFLAGS) && \
	$(CC) -c rshapes.c $(RAYLIB_CFLAGS) && \
	$(CC) -c rtextures.c $(RAYLIB_CFLAGS) && \
	$(CC) -c rtext.c $(RAYLIB_CFLAGS) && \
	$(CC) -c rmodels.c $(RAYLIB_CFLAGS) && \
	$(CC) -c raudio.c $(RAYLIB_CFLAGS) && \
	$(CC) -c utils.c $(RAYLIB_CFLAGS) && \
	ar rcs libraylib.a rcore.o rglfw.o rshapes.o rtextures.o rtext.o rmodels.o raudio.o utils.o && \
	cp raylib.h $(BUILD_DIR)/include && \
	cp libraylib.a $(BUILD_DIR)/lib

game: $(OBJECTS)
	@echo "> Linking project..."
	$(CC) -o $(BUILD_DIR)/$(EXECUTABLE_NAME).exe $(OBJECTS) $(CFLAGS) $(LDFLAGS) 
	$(BUILD_DIR)/$(EXECUTABLE_NAME).exe

# The -MMD option tells gcc to generate dependency files.
# The -MF $(@:.o=.d) option specifies the name of the dependency file.
# With this, when a .h file is modified, any .c file that includes it
# will be also recompiled.
$(BUILD_DIR)/%.o: src/%.c
	@echo "> Compiling $<..."
	@$(MKDIR) $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAGS) -MMD -MF $(@:.o=.d)

clean:
	@echo "> Cleaning up..."
	@$(RMDIR) $(BUILD_DIR)
	@cd $(RAYLIB_PATH)/src && $(RM) *.o && $(RM) libraylib.a

# =====================================

# The .PHONY directive tells make that these targets
# are not associated with actual files
.PHONY: all raylib game clean

-include $(BUILD_DIR)/*.d