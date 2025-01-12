@Echo On

cd /D "%~dp0"

"C:\Users\Sorin\AppData\Local\Arduino15\packages\esp32\tools\mkspiffs\0.2.3\mkspiffs.exe" -c data -b 4096 -p 256 -s 0x160000 spiffs.bin
"C:\Users\Sorin\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\4.5.1\esptool.exe" --chip esp32s3 --port COM4 --baud 921600 write_flash -z 0x290000 spiffs.bin

pause