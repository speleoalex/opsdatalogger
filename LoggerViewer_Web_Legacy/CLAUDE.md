# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

OpsDataLogger LoggerViewer is a web-based interface for communicating with and managing data from FluxyLogger hardware devices. It provides tools for connecting to dataloggers via serial, USB, or Bluetooth connections, managing files on the devices, and visualizing logged data.

The application is built as a Progressive Web App (PWA) using HTML, CSS, and JavaScript, designed to run in Chrome or Chromium browsers.

## Project Structure

- `loggermanager.htm` - Main entry point for the application, serving as a terminal interface
- `loggerviewer.htm` - Data visualization component for viewing and analyzing log files
- `terminal.htm` and `terminal_V2.htm` - Terminal interfaces for direct communication with devices
- `serial.html` - Simple serial communication interface
- `service-worker.js` - PWA service worker for offline functionality
- `manifest.json` - PWA manifest file
- `run-linux.sh` - Linux launch script 
- `run_windows.bat` - Windows launch script
- `run-linux_terminal.sh` - Linux launch script for terminal version

## How to Run the Application

### On Linux

```bash
# Run the main application
./run-linux.sh

# Run the terminal interface
./run-linux_terminal.sh
```

### On Windows

```batch
# Run the main application
run_windows.bat
```

These scripts launch the application using Chrome/Chromium with specific flags to enable hardware access (Web Serial API, WebUSB, etc.).

## Application Components

1. **Terminal Interface (loggermanager.htm/terminal_V2.htm)**
   - Connects to datalogger devices via Serial, USB, or BLE
   - Sends commands to the device
   - Downloads and manages log files
   - Real-time monitoring capabilities

2. **Data Viewer (loggerviewer.htm)**
   - Visualizes data from log files
   - CSV data parsing and charting
   - Data analysis tools

## Web APIs Used

- Web Serial API (`navigator.serial`) - For serial port connections
- WebUSB API (`navigator.usb`) - For USB device connections
- Web Bluetooth API - For BLE connections
- File APIs - For reading/writing local files
- Progressive Web App APIs - For offline functionality

## Development Notes

- The application is designed to be run as a local web application through Chrome/Chromium with special flags to enable hardware access
- Code is primarily contained in HTML files with embedded JavaScript
- No separate build process is required - edit HTML/JS files directly
- Testing requires access to FluxyLogger hardware devices

## Architecture Overview

### Device Communication Layer
The application uses a unified abstraction for multiple connection types:
- `sendToDevice(data)` - Universal function that routes data to appropriate connection type
- State variables track connection status: `connected_serial`, `connected_usb`, `connected_BLE`
- Connection functions: `connect` (Serial/USB), `connectBLE` (Bluetooth)
- Reading loops: `readLoop()` (Serial), `readLoopUsb()` (USB), `handleNotifications()` (BLE)

### File Transfer Protocol  
Custom protocol for reliable file transfer with device:
- Download initiated via `downloadFileClick(filename, size, view)`
- Protocol uses markers: "Start transmission:" and "End transmission:"
- File parsing handled by `findFileContents(testo)`
- Browser download via `scaricaFile(nomeFile, contents)`

### Real-time Data Display
- Arduino Plotter Protocol support via `isLastLineArduinoPlotterProtocol()`
- Data formatting: `plotterLineToHtmlTable(plotterLine)`
- Terminal output highlighting: `HighLighting(testo)`

### Key Global State Variables
- `inDownload` - File download in progress
- `inListFile` - File listing in progress  
- `isFullScreen` - UI state tracking
- `DevicePort`, `DeviceUsb`, `DeviceBle` - Device connection objects

### Chrome/Chromium Launch Flags Required
The run scripts use these essential flags:
- `--enable-experimental-web-platform-features` - Enables Web Serial API
- `--allow-running-insecure-content` - Required for local file access
- `--user-data-dir` - Isolated Chrome profile for device permissions