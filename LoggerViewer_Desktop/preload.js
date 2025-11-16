const { contextBridge, ipcRenderer } = require('electron');

// Expose protected methods that allow the renderer process to use
// the ipcRenderer without exposing the entire object
contextBridge.exposeInMainWorld('electronAPI', {
  // Serial Communication
  serial: {
    list: () => ipcRenderer.invoke('serial:list'),
    connect: (portPath, baudRate) => ipcRenderer.invoke('serial:connect', portPath, baudRate),
    disconnect: () => ipcRenderer.invoke('serial:disconnect'),
    write: (data) => ipcRenderer.invoke('serial:write', data),
    isConnected: () => ipcRenderer.invoke('serial:isConnected'),
    onData: (callback) => {
      ipcRenderer.on('serial:data', (event, data) => callback(data));
    },
    onError: (callback) => {
      ipcRenderer.on('serial:error', (event, error) => callback(error));
    },
    onDisconnected: (callback) => {
      ipcRenderer.on('serial:disconnected', () => callback());
    }
  },

  // File System Operations
  fs: {
    saveFile: (filename, content) => ipcRenderer.invoke('fs:saveFile', filename, content),
    openFile: () => ipcRenderer.invoke('fs:openFile')
  },

  // Window Management
  window: {
    switchToViewer: () => {
      // Send message to parent window to switch tab
      if (window.parent !== window) {
        window.parent.postMessage({ action: 'switchTab', tab: 'viewer' }, '*');
      }
    },
    switchToManager: () => {
      // Send message to parent window to switch tab
      if (window.parent !== window) {
        window.parent.postMessage({ action: 'switchTab', tab: 'manager' }, '*');
      }
    }
  },

  // Platform Info
  platform: process.platform,

  // Storage (using localStorage is fine, but we can add IPC if needed)
  storage: {
    setItem: (key, value) => {
      localStorage.setItem(key, value);
    },
    getItem: (key) => {
      return localStorage.getItem(key);
    },
    removeItem: (key) => {
      localStorage.removeItem(key);
    }
  }
});
