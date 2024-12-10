@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

:: set the folder to check  
set "FOLDER_PATH=%~dp0"

:: set the max file size in bytes 
set MAX_SIZE=104857600

:: var to track found files
set "FILES_FOUND=0"

:: 
echo Checking folder "%FOLDER_PATH%" for files larger than 100 МБ...

:: 
for /r "%FOLDER_PATH%" %%f in (*) do (
    set "FILE_PATH=%%f"
    set "FILE_SIZE=%%~zf"

    set /a FILE_SIZE_MB=!FILE_SIZE! / 1048576

    if !FILE_SIZE! gtr %MAX_SIZE% (
        echo Large file found: "!FILE_PATH!" size: !FILE_SIZE_MB! MB
        set /a FILES_FOUND+=1
    )
)

:: 
if %FILES_FOUND% equ 0 (
    echo No files larger than 100 МБ were found.
) else (
    echo Total large file found: %FILES_FOUND%.
)

:: 
echo Check complete. Press any key to exit.
pause