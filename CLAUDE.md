# Project: ESPBASE

## Architecture
- Xiao ESP32S3 board, PlatformIO, Arduino + FreeRTOS
- C:\Development\Repos\ESPBASE\src contains main modules# Project: ESPBASE
- C:\Development\Repos\ESPBASE\.pio\libdeps\seeed_xiao_esp32s3 contains module libraries 
- C:\Development\Repos\ESPBASE\data\server contains web server modules 

## Conventions
- Use camelCase for variables

## Documents
- docs\ MCU, sensors, board specifications.

## key files
- src\main.cpp - entry point
- src\drivers.cpp - IIC, CAN, RS232, UART, LED, SPI, GPIO definitions
- src\includes.h - task structures
- src\networking.cpp - Ethernet, Wifi, UDP drivers
- src\utils.cpp - File system, File access, Task control, log print
- src\wbserver.cpp - Web server, FOTA.

## Do not touch
- C:\Development\Repos\ESPBASE\.pio\libdeps\seeed_xiao_esp32s3 third party, dont modify 
