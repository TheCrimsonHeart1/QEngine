@echo off
setlocal

:: =================================================================
:: --- User Configuration ---
:: =================================================================
set "BUILD_CONFIG=Release"
set "GAME_EXE_NAME=QEngine.exe"
set "ASSETS_SOURCE_DIR=assets"
set "EXPORT_TARGET_DIR=Export"
:: List any required DLLs here:
set "DLLS_TO_COPY=glfw3.dll glew32.dll SDL3_image.dll lua.dll SDL3.dll glm.dll OpenGl.dll"

:: =================================================================
:: --- Script Logic ---
:: =================================================================
set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%cmake-build-%BUILD_CONFIG%"
set "EXPORT_DIR=%PROJECT_DIR%%EXPORT_TARGET_DIR%"

echo.
echo --- Starting export of build files ---
echo.

:: --- Step 1: Clean previous export (optional) ---
if exist "%EXPORT_DIR%" rmdir /s /q "%EXPORT_DIR%"
mkdir "%EXPORT_DIR%"

:: --- Step 2: Copy executable ---
echo Copying game executable...
copy "%BUILD_DIR%\%GAME_EXE_NAME%" "%EXPORT_DIR%"
if %errorlevel% neq 0 (
    echo Failed to copy executable.
    exit /b %errorlevel%
)

:: --- Step 3: Copy DLLs ---
echo Copying required DLLs...
for %%F in (%DLLS_TO_COPY%) do (
    if exist "%BUILD_DIR%\%%F" (
        copy "%BUILD_DIR%\%%F" "%EXPORT_DIR%"
    )
)

:: --- Step 4: Copy assets ---
echo Copying assets...
xcopy "%PROJECT_DIR%%ASSETS_SOURCE_DIR%" "%EXPORT_DIR%\%ASSETS_SOURCE_DIR%" /s /e /i /q

echo.
echo --- Export complete! Output is in "%EXPORT_DIR%" ---
echo.
exit /b 0