@echo off
cd /d "%~dp0"

cd ..
cd ..
cd ..
cd ..
cd ..
copy "Bootloader\project\at_start_f415\Torrio_Case_Bootloader\iar_v8.2\Torrio_Case_Bootloader_Debug\Exe\Torrio_Case_Bootloader.bin" "App\project\at_start_f415\Torrio_Case_App\gcc\Torrio_Case_App_Debug"
cd App
cd project
cd at_start_f415
cd Torrio_Case_App
cd gcc

set "FILES=_UPDATE_Torrio.bat srec_cat.exe update_file_config_Torrio.json update_tool.exe"

echo Copying files to "gcc" ...
for %%F in (%FILES%) do (
    if exist "%%~F" (
        copy /Y "%%~F" "Torrio_Case_App_Debug\">nul
        echo  - Copied %%~F
    ) else (
        echo  - Warning: %%~F not found
    )
)

cd Torrio_Case_App_Debug

srec_cat.exe -generate 0x00000 0xE000 --constant 0xFF -o Flash_Base_56KB.bin -binary >nul 2>&1

srec_cat.exe Torrio_Case_App.bin -binary -crop 0x00000000 0xDFFC -fill 0xFF 0x00000000 0xDFFC -crc32-l-e 0xDFFC -o Torrio_Case_App.bin -binary >nul 2>&1

srec_cat.exe Torrio_Case_Bootloader.bin -binary Torrio_Case_App.bin -binary -offset 0x4000 -o Torrio_Artery_Image.bin --binary >nul 2>&1

srec_cat.exe Torrio_Artery_Image.bin -binary Flash_Base_56KB.bin -binary -offset 0x12000 -o Torrio_Artery_Image.bin --binary >nul 2>&1

del Flash_Base_56KB.bin
del Torrio_Case_Bootloader.bin

for /f "usebackq delims=" %%t in (`powershell -NoProfile -Command "Get-Date -Format 'yyyy-MM-dd-HH-mm-ss'"`) do set "TIMESTAMP=%%t"
set "FOLDERNAME=Torrio_Case_App_Update_Debug_packing_%TIMESTAMP%"

mkdir "%FOLDERNAME%"

set "FILES=_UPDATE_Torrio.bat srec_cat.exe Torrio_Case_App.bin update_file_config_Torrio.json update_tool.exe Torrio_Artery_Image.bin"

echo Copying files to "%FOLDERNAME%" ...
for %%F in (%FILES%) do (
    if exist "%%~F" (
        copy /Y "%%~F" "%FOLDERNAME%\">nul
        echo  - Copied %%~F
    ) else (
        echo  - Warning: %%~F not found
    )
)

echo.
echo Done. Packed files are in: "%CD%\%FOLDERNAME%"

endlocal

cd ..