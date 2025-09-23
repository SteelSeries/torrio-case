@echo off

cd ..
cd ..
cd ..
cd ..
cd ..
copy "Bootloader\project\at_start_f415\Torrio_Case_Bootloader\iar_v8.2\Torrio_Case_Bootloader\Exe\Torrio_Case_Bootloader.bin" "App\project\at_start_f415\Torrio_Case_App\iar_v8.2\Torrio_Case_App\Exe"
cd App
cd project
cd at_start_f415
cd Torrio_Case_App
cd iar_v8.2
cd Torrio_Case_App
cd Exe

srec_cat.exe Torrio_Case_App.bin -binary -crop 0x00000000 0x1BFFC -fill 0xFF 0x00000000 0x1BFFC -crc32-l-e 0x1BFFC -o Torrio_Case_App.bin -binary
srec_cat.exe Torrio_Case_Bootloader.bin -binary Torrio_Case_App.bin -binary -offset 0x4000 -o Torrio_Artery_Image.bin --binary
del Torrio_Case_Bootloader.bin

for /f "usebackq delims=" %%t in (`powershell -NoProfile -Command "Get-Date -Format 'yyyy-MM-dd-HH-mm-ss'"`) do set "TIMESTAMP=%%t"
set "FOLDERNAME=Torrio_Case_App_Update_packing_%TIMESTAMP%"

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