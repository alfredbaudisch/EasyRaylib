@echo off

set GAME_RUNNING=false

:: OUT_DIR is for everything except the exe. The exe needs to stay in root
:: folder so it sees the resources folder, without having to copy it.
set OUT_DIR=build\hot_reload
set GAME_PDBS_DIR=%OUT_DIR%\game_pdbs

set EXE=game_hot_reload.exe

:: Check if game is running
FOR /F %%x IN ('tasklist /NH /FI "IMAGENAME eq %EXE%"') DO IF %%x == %EXE% set GAME_RUNNING=true

if not exist %OUT_DIR% mkdir %OUT_DIR%

:: If game isn't running then:
:: - delete all game_XXX.dll files
:: - delete all PDBs in pdbs subdir
:: - optionally create the pdbs subdir
:: - write 0 into pdbs\pdb_number so game.dll PDBs start counting from zero
::
:: This makes sure we start over "fresh" at PDB number 0 when starting up the
:: game and it also makes sure we don't have so many PDBs laying around.
if %GAME_RUNNING% == false (
	del /q /s %OUT_DIR% >nul 2>nul
	if not exist "%GAME_PDBS_DIR%" mkdir %GAME_PDBS_DIR%
	echo 0 > %GAME_PDBS_DIR%\pdb_number
)

:: Build raylib first if it doesn't exist (as static library)
if not exist "deps\raylib\src\libraylib.a" (
    echo Building raylib...
    cd deps\raylib\src
    mingw32-make PLATFORM=PLATFORM_DESKTOP
    cd ..\..\..
)

:: Load PDB number from file, increment and store back. For as long as the game
:: is running the pdb_number file won't be reset to 0, so we'll get a PDB of a
:: unique name on each hot reload.
set /p PDB_NUMBER=<%GAME_PDBS_DIR%\pdb_number
set /a PDB_NUMBER=%PDB_NUMBER%+1
echo %PDB_NUMBER% > %GAME_PDBS_DIR%\pdb_number

:: Enable delayed expansion for variable expansion in loop
setlocal enabledelayedexpansion

:: Collect game source files
set GAME_SOURCES=
for /r src %%f in (*.c) do (
    set "FILE=%%~nxf"
    if not "!FILE!"=="main.c" (
        if not "!FILE!"=="main_hot_reload.c" (
            if not "!FILE!"=="file_version_builder.c" (
                if not "!FILE!"=="platform_tools.c" (
                    set "GAME_SOURCES=!GAME_SOURCES! %%f"
                )
            )
        )
    )
)

:: Build file version builder
echo Building file version builder...
gcc -g -O0 -std=c99 ^
    src\hot_reload\file_version_builder.c ^
    src\hot_reload\platform_tools.c ^
    -o %OUT_DIR%\file_version_builder.exe
IF %ERRORLEVEL% NEQ 0 exit /b 1

:: Run file version builder
%OUT_DIR%\file_version_builder.exe
IF %ERRORLEVEL% NEQ 0 exit /b 1

:: Build game dll WITHOUT linking raylib (raylib symbols will come from main executable)
:: Note: No raylib linking here - the main executable will provide raylib symbols
echo Building game.dll
gcc -shared -g -O0 -std=c99 ^
    -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -DHOT_RELOAD ^
    -Ideps\raylib\src -Isrc ^
    !GAME_SOURCES! ^
    -o %OUT_DIR%\game_tmp.dll
IF %ERRORLEVEL% NEQ 0 exit /b 1

:: Move temp file to final location (atomic operation)
move %OUT_DIR%\game_tmp.dll %OUT_DIR%\game.dll >nul
IF %ERRORLEVEL% NEQ 0 exit /b 1

:: If game.exe already running: Then only compile game.dll and exit cleanly
if %GAME_RUNNING% == true (
	echo Hot reloading... && exit /b 0
)

:: Build game.exe - Link raylib ONLY to the main executable
:: The main executable exports raylib symbols to the shared library
echo Building %EXE%
gcc -g -O0 -std=c99 ^
    -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 ^
    -Ideps\raylib\src -Isrc ^
    src\main_hot_reload.c ^
    src\hot_reload\platform_tools.c ^
    deps\raylib\src\libraylib.a ^
    -Wl,--export-all-symbols ^
    -lopengl32 -lgdi32 -lwinmm ^
    -o %OUT_DIR%\%EXE%
IF %ERRORLEVEL% NEQ 0 exit /b 1

echo Build complete!
echo   Main executable: %OUT_DIR%\%EXE%
echo   Game library: %OUT_DIR%\game.dll
echo.

if "%~1"=="run" (
	echo Running %EXE%...
	start %OUT_DIR%\%EXE%
) 