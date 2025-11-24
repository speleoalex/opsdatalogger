# Update Firmware

## 1. Install Arduino IDE

- Visit the Arduino Website: Go to <arduino.cc> and navigate to the download section.
- Download the IDE: Select the Arduino IDE version compatible with your operating system (Windows, macOS, Linux).
- Installation: Follow the installation instructions specific to your operating system.

## 2. Prepare the Development Environment

- Connect Your Arduino UNO: Use a USB cable to connect the Arduino UNO board to your computer.
- Select the Board and Port: Open the Arduino IDE, go to "Tools" > "Board" and select "Arduino UNO". Then, under "Tools" > "Port", choose the port to which your board is connected.

## 3. Install Required Libraries

- Open the Library Manager: In the Arduino IDE, go to "Tools" > "Manage Libraries".
- Search and Install Libraries: Search for the libraries required by your program, such as those for RTC (Real Time Clock) and SD card. Select each library and click "Install".

Required libraries:

- RTClib
- SdFat

## 4. Upload the "FluxyLogger.ino" Program

- Get the Source Code: <https://github.com/speleoalex/opsdatalogger/blob/main/FluxyLogger/FluxyLogger.ino>
- Open the File in IDE: In the Arduino IDE, go to "File" > "Open" and select the "FluxyLogger.ino" file.
- Configure Sensors: Before uploading, edit the `#define` directives at the beginning of the file to match your hardware configuration (see [Platform Overview](../FluxyLogger-Platform-Overview-en.md)).
- Verify and Upload the Program: Click the "Verify" button to compile the code. If there are no errors, click "Upload" to transfer the program to your Arduino UNO board.

## Troubleshooting

**Upload fails:**

- Check that the correct board is selected (Arduino UNO)
- Verify the correct port is selected
- Ensure no other programs are using the serial port

**Compilation errors:**

- Make sure all required libraries are installed
- Check that you have the latest Arduino IDE version
- Verify the code hasn't been corrupted during download

## After Upload

1. Disconnect the Arduino from the computer
2. Insert a formatted microSD card
3. Connect to a USB powerbank
4. The device should start recording after the preheat phase

See the user manuals for your specific configuration:

- [NASO Manual](FluxyLogger-NASO-en.md)
- [NASO + LCD Manual](FluxyLogger-NASO_lcd-en.md)
