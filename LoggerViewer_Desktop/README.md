# LoggerViewer Desktop

Desktop application for FluxyLogger data viewing and management, built with Electron.

## Features

- **Tabbed Interface**: Single window with Manager and Viewer tabs for easy navigation
- **Serial Communication**: Connect to FluxyLogger devices via USB/Serial ports
- **Data Management**: Download, view, and export log files from the device
- **Real-time Monitoring**: View live sensor data with Arduino plotter protocol support
- **Data Visualization**: Advanced CSV viewer with charting capabilities using Chart.js
- **Keyboard Shortcuts**: Ctrl+1 for Manager, Ctrl+2 for Viewer
- **Cross-platform**: Works on Linux, Windows, and macOS

## Prerequisites

- Node.js 18.x or higher
- npm or yarn package manager

## Installation

1. Navigate to the project directory:
```bash
cd LoggerViewer_Desktop
```

2. Install dependencies:
```bash
npm install
```

## Running the Application

### Development Mode

To run the application in development mode with DevTools enabled:

```bash
npm run dev
```

### Production Mode

To run the application normally:

```bash
npm start
```

## Building the Application

### Build for Current Platform

```bash
npm run build
```

### Build for Specific Platforms

**Linux:**
```bash
npm run build:linux
```

This creates:
- `dist/LoggerViewer-x.x.x.AppImage` - Portable AppImage
- `dist/LoggerViewer_x.x.x_amd64.deb` - Debian package

**Windows:**
```bash
npm run build:win
```

This creates:
- `dist/LoggerViewer Setup x.x.x.exe` - NSIS installer
- `dist/LoggerViewer x.x.x.exe` - Portable executable

**macOS:**
```bash
npm run build:mac
```

This creates:
- `dist/LoggerViewer-x.x.x.dmg` - DMG installer
- `dist/LoggerViewer-x.x.x-mac.zip` - Portable ZIP

## Usage

### Connecting to a Device

1. Click "Connect to Serial/USB" button
2. Select your device's port from the dialog
3. The application will connect at the configured baud rate (default: 19200)

### Managing Log Files

1. After connecting, click "Read Logs" to see available log files
2. Use the download button (📥) to save a file locally
3. Use the chart button (📈) to open the file in the viewer
4. Use the delete button (❌) to remove files from the device

### Viewing Data

1. In the viewer, click "Open CSV File" to load a log file
2. Configure calibration settings in the sidebar if needed
3. Enable heating compensation for MQ-2 sensor data if applicable
4. Export processed data using "Export CSV"

### Real-time Monitoring

1. Click "Start Real-Time" to enable plotter mode
2. Live sensor values will be displayed in large format
3. Click "Stop Real-Time" to return to terminal mode

## Project Structure

```
LoggerViewer_Desktop/
├── main.js                    # Electron main process
├── preload.js                 # Preload script (IPC bridge)
├── package.json               # Project configuration
├── README.md                  # This file
└── src/
    ├── assets/                # Images and icons
    │   ├── icon.png
    │   └── favicon.ico
    └── renderer/              # Renderer process files
        ├── loggermanager.html # Main manager interface
        ├── loggerviewer.html  # Data viewer interface
        ├── loggerviewer.js    # Viewer logic
        ├── electron-adapter.js # Electron API adapter
        └── loader.js          # Runtime adaptations
```

## Key Technologies

- **Electron**: Desktop application framework
- **Node.js SerialPort**: Serial communication
- **Chart.js**: Data visualization
- **Native File Dialogs**: File operations

## Serial Communication

The application uses Node.js SerialPort for reliable communication with FluxyLogger devices. Key features:

- Automatic port detection
- Configurable baud rate (9600, 19200, 57600, 115200)
- Real-time data streaming
- Automatic reconnection on disconnect

## Development

### Architecture

The application follows Electron's multi-process architecture:

- **Main Process** (`main.js`): Handles system-level operations, serial communication, and file I/O
- **Renderer Process** (HTML/JS files): UI and user interactions
- **Preload Script** (`preload.js`): Secure bridge between main and renderer

### Adding Features

1. Add IPC handlers in `main.js` for new functionality
2. Expose APIs through `preload.js` using `contextBridge`
3. Use exposed APIs in renderer scripts via `window.electronAPI`

### Debugging

- Development mode includes Chrome DevTools
- Console logs from main process appear in terminal
- Renderer process logs appear in DevTools

## Troubleshooting

### Serial Port Access Issues

**Linux:**
```bash
# Add user to dialout group for serial port access
sudo usermod -a -G dialout $USER
# Log out and log back in for changes to take effect
```

**Windows:**
- Ensure device drivers are installed
- Check Device Manager for port conflicts

**macOS:**
- Grant permissions when prompted
- Check System Preferences > Security & Privacy

### Build Issues

If builds fail:
```bash
# Clear node_modules and reinstall
rm -rf node_modules package-lock.json
npm install
```

## Contributing

This project is part of the FluxyLogger NASO ecosystem. For contributions:

1. Ensure serial communication remains compatible with FluxyLogger firmware
2. Test on multiple platforms before submitting changes
3. Follow existing code style and structure

## License

MIT

## Support

For issues, questions, or donations, visit:
https://www.paypal.com/donate/?business=TKQWLKGENEP7L&no_recurring=0&item_name=Progetto+FluxyLogger+NASO&currency_code=EUR

## Changelog

### Version 1.0.0
- Initial desktop release
- Serial communication support
- File management capabilities
- Data visualization with Chart.js
- Cross-platform builds
- MQ-2 sensor calibration tools
- Real-time monitoring support
