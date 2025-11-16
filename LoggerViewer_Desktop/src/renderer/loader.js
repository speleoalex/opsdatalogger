// Loader script that adapts the original HTML for Electron

// Override file input handler for CSV files
function setupFileHandlers() {
    // Override the button click to use Electron dialog instead of file input
    const openButton = document.querySelector('button[onclick*="logfile"]');

    if (openButton) {
        // Remove the onclick attribute
        openButton.removeAttribute('onclick');

        // Add new click handler that uses Electron dialog
        openButton.addEventListener('click', async function(e) {
            e.preventDefault();
            e.stopPropagation();

            try {
                const result = await window.electronAPI.fs.openFile();

                if (!result) {
                    return; // User cancelled
                }

                // Simulate the original file handling
                if (typeof parseCSVData === 'function') {
                    parseCSVData(result.content);
                }
            } catch (error) {
                alert('Error opening file: ' + error.message);
            }
        });

        console.log('File handler setup complete');
    }

    // Keep the original handleFiles for any other uses
    const originalHandleFiles = window.handleFiles;
    if (originalHandleFiles) {
        window.handleFiles = originalHandleFiles;
    }
}

// Override download/save functions
function setupDownloadHandlers() {
    window.scaricaFile = async function(nomeFile, contents) {
        try {
            const result = await window.electronAPI.fs.saveFile(nomeFile, contents);

            if (result) {
                if (typeof ConsoleLog === 'function') {
                    ConsoleLog('File saved to: ' + result);
                }
                // Show success message in the output
                const outputDiv = document.getElementById('output');
                if (outputDiv) {
                    outputDiv.innerHTML += '\n<span class="ok">File saved successfully!</span>\n';
                }
            }
        } catch (error) {
            alert('Error saving file: ' + error.message);
        }
    };
}

// Override storage functions
function setupStorageHandlers() {
    if (typeof saveToSessionStorage === 'function') {
        const originalSave = saveToSessionStorage;
        window.saveToSessionStorage = function(key, value) {
            if (window.electronAPI && window.electronAPI.storage) {
                window.electronAPI.storage.setItem(key, value);
            } else {
                originalSave(key, value);
            }
        };
    }

    if (typeof readFromSessionStorage === 'function') {
        const originalRead = readFromSessionStorage;
        window.readFromSessionStorage = function(key) {
            if (window.electronAPI && window.electronAPI.storage) {
                return window.electronAPI.storage.getItem(key);
            } else {
                return originalRead(key);
            }
        };
    }
}

// Override window open for viewer - Switch to viewer tab instead
function setupWindowHandlers() {
    window.openViewerHtml = function() {
        if (window.electronAPI && window.electronAPI.window) {
            window.electronAPI.window.switchToViewer();
        } else {
            // Fallback - send message to parent
            if (window.parent !== window) {
                window.parent.postMessage({ action: 'switchTab', tab: 'viewer' }, '*');
            }
        }
    };

    // Add function to go back to manager
    window.goToManager = function() {
        if (window.electronAPI && window.electronAPI.window) {
            window.electronAPI.window.switchToManager();
        } else {
            // Fallback - send message to parent
            if (window.parent !== window) {
                window.parent.postMessage({ action: 'switchTab', tab: 'manager' }, '*');
            }
        }
    };
}

// Initialize when DOM is loaded
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        setupFileHandlers();
        setupDownloadHandlers();
        setupStorageHandlers();
        setupWindowHandlers();
    });
} else {
    setupFileHandlers();
    setupDownloadHandlers();
    setupStorageHandlers();
    setupWindowHandlers();
}
