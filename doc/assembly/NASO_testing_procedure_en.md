# NASO Testing Procedure

Procedure for testing the NASO datalogger before delivery.

## Functional Test

1. **Serial Connection**
   - Connect NASO to PC USB port
   - Open serial terminal at 19200 baud (Arduino IDE or Serial USB Terminal)
   - Alternatively, use the [web app](https://applications.techmakers.it/datalogger/terminal.htm)

2. **Initialization Check**
   - Verify LED blinking during preheat phase
   - Wait for MQ-2 sensor stabilization (approximately 30 seconds)
   - ADC value should stabilize decreasing to around 10-150

3. **Configuration**
   - Set date and time with `settime` command
   - Execute `autocalib` command for automatic calibration
   - Alternatively, manually set zerogas value with `setconfig` (usually ~50)

4. **Sensor Verification**
   - Execute `plotter start` command for visualization mode
   - Verify operation with spray deodorant or by breathing on the sensor
   - ADC and PPM values should increase then decrease

5. **SD Write Test**
   - Verify that data is being saved to microSD
   - Check that no "failed" errors appear

## Troubleshooting

- **LEDs not blinking**: Check power supply and shield connection
- **Sensor not stabilizing**: Wait longer for preheat (up to 2-3 minutes)
- **SD error**: Verify microSD is inserted and formatted (FAT32)
- **Values not reacting**: Check MQ-2 sensor wiring
