const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');
const fs = require('fs');
const { SerialPort } = require('serialport');

let mainWindow;
let currentPort = null;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
    },
    icon: path.join(__dirname, 'src/assets/icon.png')
  });

  mainWindow.loadFile('src/renderer/index.html');

  // Open DevTools in development mode
  if (process.argv.includes('--dev')) {
    mainWindow.webContents.openDevTools();
  }

  mainWindow.on('closed', () => {
    mainWindow = null;
    if (currentPort && currentPort.isOpen) {
      currentPort.close();
    }
  });
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

// IPC Handlers for Serial Communication
ipcMain.handle('serial:list', async () => {
  try {
    const ports = await SerialPort.list();
    return { success: true, ports: ports.map(p => ({
      path: p.path,
      manufacturer: p.manufacturer,
      serialNumber: p.serialNumber,
      productId: p.productId,
      vendorId: p.vendorId
    })) };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

ipcMain.handle('serial:connect', async (event, portPath, baudRate = 19200) => {
  try {
    if (currentPort && currentPort.isOpen) {
      await currentPort.close();
    }

    currentPort = new SerialPort({
      path: portPath,
      baudRate: baudRate,
      dataBits: 8,
      stopBits: 1,
      parity: 'none'
    });

    currentPort.on('data', (data) => {
      mainWindow.webContents.send('serial:data', data.toString());
    });

    currentPort.on('error', (err) => {
      mainWindow.webContents.send('serial:error', err.message);
    });

    currentPort.on('close', () => {
      mainWindow.webContents.send('serial:disconnected');
      currentPort = null;
    });

    return { success: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

ipcMain.handle('serial:disconnect', async () => {
  try {
    if (currentPort && currentPort.isOpen) {
      await currentPort.close();
      currentPort = null;
    }
    return { success: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

ipcMain.handle('serial:write', async (event, data) => {
  try {
    if (!currentPort || !currentPort.isOpen) {
      return { success: false, error: 'Port not open' };
    }

    currentPort.write(data);
    return { success: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

ipcMain.handle('serial:isConnected', () => {
  return currentPort && currentPort.isOpen;
});

// File System Operations
ipcMain.handle('fs:saveFile', async (event, filename, content) => {
  try {
    // Focus the main window first
    if (mainWindow) {
      mainWindow.focus();
    }

    const { filePath } = await dialog.showSaveDialog(mainWindow, {
      defaultPath: filename,
      modal: true,
      filters: [
        { name: 'CSV Files', extensions: ['csv'] },
        { name: 'Text Files', extensions: ['txt'] },
        { name: 'All Files', extensions: ['*'] }
      ]
    });

    if (filePath) {
      fs.writeFileSync(filePath, content, 'utf8');
      return { success: true, path: filePath };
    }
    return { success: false, cancelled: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

ipcMain.handle('fs:openFile', async () => {
  try {
    // Focus the main window first
    if (mainWindow) {
      mainWindow.focus();
      mainWindow.moveTop();
    }

    const { filePaths } = await dialog.showOpenDialog(mainWindow, {
      properties: ['openFile'],
      modal: true,
      filters: [
        { name: 'CSV Files', extensions: ['csv'] },
        { name: 'Text Files', extensions: ['txt'] },
        { name: 'All Files', extensions: ['*'] }
      ]
    });

    if (filePaths && filePaths.length > 0) {
      const content = fs.readFileSync(filePaths[0], 'utf8');
      return { success: true, content, filename: path.basename(filePaths[0]) };
    }
    return { success: false, cancelled: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

// Window Management - Tab switching handled in renderer
