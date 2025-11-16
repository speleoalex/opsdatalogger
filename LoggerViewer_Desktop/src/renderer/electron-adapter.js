// Electron API Adapter
// This file provides a compatibility layer between Web APIs and Electron APIs

class ElectronCommunicationManager {
    constructor() {
        this.connectionType = 'serial'; // Electron uses serial by default
        this.isConnected = false;
        this.isConnecting = false;
        this.currentPortPath = null;

        // Event callbacks
        this.onDataReceived = null;
        this.onConnectionChange = null;
        this.onError = null;

        // Setup event listeners from main process
        this.setupEventHandlers();
    }

    setupEventHandlers() {
        if (window.electronAPI) {
            // Listen for serial data from main process
            window.electronAPI.serial.onData((data) => {
                if (this.onDataReceived) {
                    this.onDataReceived(data);
                }
            });

            // Listen for errors
            window.electronAPI.serial.onError((error) => {
                if (this.onError) {
                    this.onError('Serial error: ' + error);
                }
            });

            // Listen for disconnection
            window.electronAPI.serial.onDisconnected(() => {
                this.isConnected = false;
                this.connectionType = null;
                if (this.onConnectionChange) {
                    this.onConnectionChange('disconnected');
                }
            });
        }
    }

    updateConnectionStatus(status, type = 'serial') {
        this.isConnected = status === 'connected';
        this.isConnecting = status === 'connecting';
        this.connectionType = status === 'connected' ? type : null;

        if (this.onConnectionChange) {
            this.onConnectionChange(status, type);
        }
    }

    async connectSerial(baudRate = 19200) {
        try {
            this.updateConnectionStatus('connecting', 'serial');

            // Get list of available ports
            const result = await window.electronAPI.serial.list();

            if (!result.success) {
                throw new Error(result.error);
            }

            if (result.ports.length === 0) {
                throw new Error('No serial ports found');
            }

            // Show port selection dialog
            const portPath = await this.showPortSelectionDialog(result.ports);

            if (!portPath) {
                throw new Error('No port selected');
            }

            // Connect to selected port
            const connectResult = await window.electronAPI.serial.connect(portPath, baudRate);

            if (!connectResult.success) {
                throw new Error(connectResult.error);
            }

            this.currentPortPath = portPath;
            this.updateConnectionStatus('connected', 'serial');

            return true;
        } catch (error) {
            this.updateConnectionStatus('disconnected');
            throw new Error('Serial connection failed: ' + error.message);
        }
    }

    async showPortSelectionDialog(ports) {
        // Create a simple dialog to select port
        return new Promise((resolve) => {
            const dialog = document.createElement('div');
            dialog.style.cssText = `
                position: fixed;
                top: 50%;
                left: 50%;
                transform: translate(-50%, -50%);
                background: white;
                padding: 20px;
                border-radius: 8px;
                box-shadow: 0 4px 6px rgba(0,0,0,0.3);
                z-index: 10000;
                min-width: 400px;
            `;

            const title = document.createElement('h3');
            title.textContent = 'Select Serial Port';
            title.style.marginTop = '0';
            dialog.appendChild(title);

            const select = document.createElement('select');
            select.style.cssText = 'width: 100%; padding: 8px; margin: 10px 0; font-size: 14px;';

            ports.forEach(port => {
                const option = document.createElement('option');
                option.value = port.path;
                option.textContent = `${port.path}${port.manufacturer ? ' - ' + port.manufacturer : ''}`;
                select.appendChild(option);
            });

            dialog.appendChild(select);

            const buttonContainer = document.createElement('div');
            buttonContainer.style.cssText = 'display: flex; gap: 10px; margin-top: 15px;';

            const connectBtn = document.createElement('button');
            connectBtn.textContent = 'Connect';
            connectBtn.className = 'button-primary';
            connectBtn.style.flex = '1';
            connectBtn.onclick = () => {
                const selectedPort = select.value;
                document.body.removeChild(overlay);
                resolve(selectedPort);
            };

            const cancelBtn = document.createElement('button');
            cancelBtn.textContent = 'Cancel';
            cancelBtn.className = 'button-secondary';
            cancelBtn.style.flex = '1';
            cancelBtn.onclick = () => {
                document.body.removeChild(overlay);
                resolve(null);
            };

            buttonContainer.appendChild(connectBtn);
            buttonContainer.appendChild(cancelBtn);
            dialog.appendChild(buttonContainer);

            const overlay = document.createElement('div');
            overlay.style.cssText = `
                position: fixed;
                top: 0;
                left: 0;
                right: 0;
                bottom: 0;
                background: rgba(0,0,0,0.5);
                z-index: 9999;
            `;
            overlay.appendChild(dialog);
            document.body.appendChild(overlay);
        });
    }

    async disconnect() {
        try {
            const result = await window.electronAPI.serial.disconnect();
            if (result.success) {
                this.updateConnectionStatus('disconnected');
                return true;
            }
            throw new Error(result.error);
        } catch (error) {
            throw new Error('Disconnection failed: ' + error.message);
        }
    }

    async sendData(data) {
        if (!this.isConnected) {
            throw new Error('No active connection');
        }

        try {
            const result = await window.electronAPI.serial.write(data);
            if (!result.success) {
                throw new Error(result.error);
            }
            return true;
        } catch (error) {
            throw new Error('Send failed: ' + error.message);
        }
    }

    getConnectionInfo() {
        return {
            isConnected: this.isConnected,
            isConnecting: this.isConnecting,
            type: this.connectionType,
            port: this.currentPortPath
        };
    }
}

// File System Adapter
class ElectronFileSystemAdapter {
    static async saveFile(filename, content) {
        if (!window.electronAPI) {
            throw new Error('Electron API not available');
        }

        const result = await window.electronAPI.fs.saveFile(filename, content);

        if (!result.success) {
            if (result.cancelled) {
                return null; // User cancelled
            }
            throw new Error(result.error);
        }

        return result.path;
    }

    static async openFile() {
        if (!window.electronAPI) {
            throw new Error('Electron API not available');
        }

        const result = await window.electronAPI.fs.openFile();

        if (!result.success) {
            if (result.cancelled) {
                return null; // User cancelled
            }
            throw new Error(result.error);
        }

        return {
            content: result.content,
            filename: result.filename
        };
    }
}

// Storage Adapter (uses localStorage through Electron's preload)
class ElectronStorageAdapter {
    static setItem(key, value) {
        if (window.electronAPI && window.electronAPI.storage) {
            window.electronAPI.storage.setItem(key, value);
        } else {
            localStorage.setItem(key, value);
        }
    }

    static getItem(key) {
        if (window.electronAPI && window.electronAPI.storage) {
            return window.electronAPI.storage.getItem(key);
        } else {
            return localStorage.getItem(key);
        }
    }

    static removeItem(key) {
        if (window.electronAPI && window.electronAPI.storage) {
            window.electronAPI.storage.removeItem(key);
        } else {
            localStorage.removeItem(key);
        }
    }
}

// Export for use in other scripts
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        ElectronCommunicationManager,
        ElectronFileSystemAdapter,
        ElectronStorageAdapter
    };
}
