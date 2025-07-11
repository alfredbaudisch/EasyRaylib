# Default mode is debug/dev
# Call with: `make` (when empty defaults to DEBUG) or `make MODE=RELEASE`
MODE ?= DEBUG

# =====================================
# OS Detection
# =====================================
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    OS = MACOS
else ifeq ($(OS),Windows_NT)
    OS = WINDOWS
else
    # Assume Windows if uname is not available (common in Windows environments)
    OS = WINDOWS
endif

# =====================================
# Variables
# =====================================
# OS-specific commands
RM = rm -f
RMDIR = rm -rf
MKDIR = mkdir -p

BUILD_DIR := $(CURDIR)/build
RAYLIB_PATH = ./deps/raylib
GRAPHIC_API = GRAPHICS_API_OPENGL_33

# OS-specific settings
ifeq ($(OS),MACOS)
    CC = clang
    LDFLAGS = -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL $(RAYLIB_PATH)/src/libraylib.a
    EXECUTABLE_EXT =
    RAYLIB_MAKE_PLATFORM = PLATFORM=PLATFORM_DESKTOP
else
    CC = gcc
    COMPILER_PATH = C:/Dev/msys2/mingw64
    LDFLAGS = -L$(RELEASE_PATH)/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    EXECUTABLE_EXT = .exe
    RAYLIB_MAKE_PLATFORM = PLATFORM=PLATFORM_DESKTOP
endif

ifeq ($(MODE),RELEASE)
    RAYLIB_CFLAGS = -std=c99 -O3 -march=native -DNDEBUG -flto -Wall -DPLATFORM_DESKTOP -D$(GRAPHIC_API) -I$(RAYLIB_PATH)/src -Iexternal
    CFLAGS = -std=c99 -Wall -DPLATFORM_DESKTOP -D$(GRAPHIC_API) -I$(RAYLIB_PATH)/src -Iexternal
else
    CFLAGS = -std=c99 -Wall -DPLATFORM_DESKTOP -D$(GRAPHIC_API) -I$(RAYLIB_PATH)/src -Iexternal
    RAYLIB_CFLAGS = $(CFLAGS)
endif

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
all: info raylib game

info:
	@echo "> Detected OS: $(OS)"
	@echo "> Using compiler: $(CC)"

raylib:
	@echo "> Compiling raylib for $(OS)..."
	cd $(RAYLIB_PATH)/src && $(MAKE) $(RAYLIB_MAKE_PLATFORM)

game: $(OBJECTS)
	@echo "> Linking project..."
ifeq ($(OS),MACOS)
	$(CC) -o $(BUILD_DIR)/$(EXECUTABLE_NAME)$(EXECUTABLE_EXT) $(OBJECTS) $(LDFLAGS)
else
	$(CC) -o $(BUILD_DIR)/$(EXECUTABLE_NAME)$(EXECUTABLE_EXT) $(OBJECTS) $(CFLAGS) $(LDFLAGS)
endif
	$(BUILD_DIR)/$(EXECUTABLE_NAME)$(EXECUTABLE_EXT)

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
ifeq ($(OS),MACOS)
	@cd $(RAYLIB_PATH)/src && $(MAKE) clean
else
	@cd $(RAYLIB_PATH)/src && $(RM) *.o && $(RM) libraylib.a
endif

# =====================================

# The .PHONY directive tells make that these targets
# are not associated with actual files
.PHONY: all info raylib game clean

-include $(BUILD_DIR)/*.d