# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FluxyLogger NASO (Novel Areal Sensing Observer) is an Arduino-based datalogger for atmospheric tracer tracking in caves. The project tracks air flows using gas sensors (primarily MQ-2 for butane/propane detection) and logs data to SD cards for speleological research.

## Architecture

### Main Components

- **FluxyLogger.ino**: Primary Arduino sketch with complete sensor management, SD card logging, RTC functionality, and optional Bluetooth Low Energy (BLE) support
- **sensor_BMP280/**: Temperature, pressure, and humidity sensor library
- **sensor_SGP40/**: VOC (Volatile Organic Compounds) sensor library for indoor air quality monitoring
- **.vscode/c_cpp_properties.json**: Arduino UNO WiFi R4 configuration for development

### Hardware Support

The system supports multiple sensor configurations:
- **MQ-2**: Primary gas sensor for butane/propane detection (analog)
- **BMP280**: Temperature and pressure sensor (I2C)
- **SGP40**: VOC sensor (I2C)
- **SGP30**: Alternative VOC sensor (I2C)
- **Wind sensors**: Optional wind speed measurement
- **LCD/OLED displays**: Optional visual output
- **RTC DS1307**: Real-time clock for timestamp logging
- **SD card**: Data storage on microSD
- **BLE**: Bluetooth Low Energy communication (Arduino UNO WiFi R4 only)

### Configuration System

The device uses a `CONFIG.INI` file for persistent settings:
- `log_interval_s`: Data logging interval in seconds
- `zerogas`: Calibration value for MQ-2 sensor baseline

### Data Format

Logs are saved as CSV files with timestamps and sensor readings:
```
"date Y-m-d m:s";"gas adc";"LPG PPM";"temp";"pressure";"VOCIndex";"VCC"
```

## Development Environment

### Target Hardware
- **Primary**: Arduino UNO WiFi R4 (Renesas architecture)
- **Legacy**: Arduino UNO R3 (AVR architecture)
- **Compiler**: ARM GCC 7.2.1 for R4, AVR GCC for R3

### Required Libraries
- SdFat: SD card file system
- Wire: I2C communication
- RTClib: Real-time clock support
- ArduinoBLE: Bluetooth Low Energy (R4 only)
- Custom sensor libraries included in project

### Build Configuration
The code uses extensive `#define` flags for feature control:
- `BLE_SUPPORT`: Enable Bluetooth functionality
- `BMP280_PRESENT`: Enable temperature/pressure sensor
- `SGP40_PRESENT`/`SGP30_PRESENT`: Enable VOC sensors
- `MQ2SENSOR_PRESENT`: Enable gas sensor
- `LCD_I2C_ENABLED`: Enable LCD display
- `OLED_PRESENT`: Enable OLED display
- `DEBUGSENSOR`: Enable sensor debugging

## Development Commands

### Compilation
Use Arduino IDE or arduino-cli with the project's .vscode configuration. The c_cpp_properties.json file contains all necessary include paths and compiler flags for Arduino UNO WiFi R4.

**Arduino CLI Path**: `/home/speleoalex/Applications/arduino-ide_nightly-20241212_Linux_64bit/resources/app/lib/backend/resources/arduino-cli`

**Compile Command**:
```bash
/home/speleoalex/Applications/arduino-ide_nightly-20241212_Linux_64bit/resources/app/lib/backend/resources/arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi FluxyLogger.ino
```

### Serial Communication
- **Baud rate**: 19200 (configurable via `BOUDRATE` define)
- **Commands**: Interactive command system via Serial or BLE
  - `?`: Help menu
  - `v`: Firmware version
  - `logs`: File listing and download
  - `settime`: Set RTC clock
  - `setconfig`: Configure logging parameters
  - `autocalib`: MQ-2 sensor calibration
  - `plotter start/stop`: Arduino plotter mode

### Calibration Process
The MQ-2 sensor requires zero-gas calibration in clean air using the `autocalib` command. The system waits for stable readings before setting the baseline value.

## File Organization

- **FluxyLogger.ino**: Main application code (~1900 lines)
- **sensor_BMP280/**: BMP280 sensor driver
- **sensor_SGP40/**: SGP40 sensor driver with VOC algorithm
- **../LoggerViewer/**: Web-based log file viewer application
- **../manuals/**: User documentation in Italian and English
- **../doc/**: Technical documentation and schematics

## Important Notes

- The codebase is primarily in English with some Italian comments
- Hardware-specific code uses conditional compilation for different Arduino models
- Real-time clock is essential for timestamped logging
- SD card storage is organized with automatic file rotation (max 10,000 rows per file)
- Power management includes VCC monitoring for battery-powered operation
- BLE support enables wireless configuration and data retrieval

## Testing and Validation

The project has been field-tested in cave environments since 2020, with successful air flow tracing applications in Italian cave systems. Sensor calibration and data validation procedures are documented in the manuals directory.
