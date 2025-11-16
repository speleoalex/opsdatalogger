#!/bin/bash
# LoggerViewer Desktop - Linux Launch Script

cd "$(dirname "$0")"

echo "======================================"
echo "   LoggerViewer Desktop Launcher"
echo "======================================"
echo ""

# Check if node_modules exists
if [ ! -d "node_modules" ]; then
    echo "Installing dependencies..."
    npm install
    echo ""
fi

# Check for serial port permissions
if ! groups | grep -q dialout; then
    echo "WARNING: User is not in 'dialout' group."
    echo "You may not have permission to access serial ports."
    echo "To fix this, run: sudo usermod -a -G dialout $USER"
    echo "Then log out and log back in."
    echo ""
fi

echo "Starting LoggerViewer Desktop..."
echo ""

# Start the application
npm start
