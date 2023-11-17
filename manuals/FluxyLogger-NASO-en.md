# List of Components

![datalogger](datalogger.png)

# How to Start Recording

- Connect the device via the USB port to a power bank.
- The two LEDs on the datalogger will blink for a few seconds alternately.
- After this, data acquisition will start.
- The default acquisition time is 15 seconds.
- The data will be saved to files within the SD card. The file name corresponds to the date and time of the beginning of acquisition.
- In case of a read/write error on the SD card, LED1 and LED2 will remain steadily on.

# Tracer

As a tracer, you can use any spray deodorant based on propane or butane.
The detector is also sensitive to alcohol, methane, smoke.
The amount of tracer needed may vary depending on the air volumes.
For distances over a kilometer, it is recommended to use at least two 300ml cans of tracer.

# How to Modify the Date and Time

### Connection from a Smartphone:
You can set the date and time with any terminal program that connects to the PC's USB port or by connecting it to a smartphone using an adapter and an app, for example, "Serial USB Terminal" for Android devices.

### Connection from a PC:
The Arduino IDE program has a built-in terminal accessible from the Tools->Serial Monitor menu.
Set the USB port and the serial speed to 9600 baud.

### Setting the Date and Time:
Turn on the sensor by connecting it to the USB port and within 5 seconds send any character.

It will then sequentially ask for the Year, Month, Day, Hour, Minutes.

After setting the date, data acquisition will automatically start and will be shown in real-time.

With the device connected to the serial, you can reset the date and time at any moment by sending the "settime" command.

## Automatic Calibration
- Connect the datalogger to a PC or smartphone in a place where there is no tracer present.
- Send the "autocalib" command.

Lights L1 and L2 will blink in sync until calibration is complete.

*The device remains in calibration as long as it performs at least 20 consecutive readings. To stop calibration, turn off and restart the device.*

## Setting Acquisition Time and Calibration

You can modify the acquisition time or the "zerogas" calibration value.
"zerogas" represents the raw value of the sensor when there is no tracer in the air.

- Connect the datalogger to a PC or smartphone in a place where there is no tracer present.
- Send the "setconfig" command.
- Enter the interval in seconds or press enter to keep the old value.
- Enter the "zerogas" value or press enter to keep the old value.

## Downloading Data

- The data are written to files inside the SD card. The files are named after the start date of the log. They can be opened with any spreadsheet. Set the field separator as ";".
- You can download the data to a PC using Google Chrome or MS Edge through a web application.
