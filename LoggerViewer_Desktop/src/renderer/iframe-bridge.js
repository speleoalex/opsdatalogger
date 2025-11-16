// Iframe Bridge - Provides electronAPI to iframes via postMessage
// This script creates a fake electronAPI that communicates with the parent window

(function() {
    // Check if we're in an iframe
    const isInIframe = window.parent !== window;

    if (!isInIframe) {
        // Not in iframe, skip
        return;
    }

    // Generate unique request IDs
    let requestIdCounter = 0;
    function generateRequestId() {
        return 'req_' + (++requestIdCounter) + '_' + Date.now();
    }

    // Store pending requests
    const pendingRequests = {};

    // Listen for responses from parent
    window.addEventListener('message', (event) => {
        const data = event.data;
        if (!data || !data.requestId) return;

        const pending = pendingRequests[data.requestId];
        if (!pending) return;

        // Remove from pending
        delete pendingRequests[data.requestId];

        // Resolve or reject the promise
        if (data.error) {
            pending.reject(new Error(data.error));
        } else {
            pending.resolve(data.result);
        }
    });

    // Helper function to send request to parent and wait for response
    function sendRequestToParent(action, params = {}) {
        return new Promise((resolve, reject) => {
            const requestId = generateRequestId();

            // Store the promise callbacks
            pendingRequests[requestId] = { resolve, reject };

            // Send request to parent
            window.parent.postMessage({
                action: action,
                requestId: requestId,
                ...params
            }, '*');

            // Timeout after 30 seconds
            setTimeout(() => {
                if (pendingRequests[requestId]) {
                    delete pendingRequests[requestId];
                    reject(new Error('Request timeout'));
                }
            }, 30000);
        });
    }

    // Create fake electronAPI that bridges to parent
    window.electronAPI = {
        // Serial Communication
        serial: {
            list: () => sendRequestToParent('serial:list'),
            connect: (portPath, baudRate) => sendRequestToParent('serial:connect', { portPath, baudRate }),
            disconnect: () => sendRequestToParent('serial:disconnect'),
            write: (data) => sendRequestToParent('serial:write', { data }),
            isConnected: () => sendRequestToParent('serial:isConnected'),
            onData: (callback) => {
                // Forward serial data events from parent
                window.addEventListener('message', (event) => {
                    if (event.data && event.data.action === 'serial:data') {
                        callback(event.data.data);
                    }
                });
            },
            onError: (callback) => {
                window.addEventListener('message', (event) => {
                    if (event.data && event.data.action === 'serial:error') {
                        callback(event.data.error);
                    }
                });
            },
            onDisconnected: (callback) => {
                window.addEventListener('message', (event) => {
                    if (event.data && event.data.action === 'serial:disconnected') {
                        callback();
                    }
                });
            }
        },

        // File System Operations
        fs: {
            saveFile: (filename, content) => sendRequestToParent('fs:saveFile', { filename, content }),
            openFile: () => sendRequestToParent('fs:openFile')
        },

        // Window Management
        window: {
            switchToViewer: () => {
                window.parent.postMessage({ action: 'switchTab', tab: 'viewer' }, '*');
            },
            switchToManager: () => {
                window.parent.postMessage({ action: 'switchTab', tab: 'manager' }, '*');
            }
        },

        // Platform Info
        platform: 'linux', // Will be overridden by parent if needed

        // Storage (localStorage works fine in iframes)
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
    };

    console.log('Iframe bridge initialized');
})();
