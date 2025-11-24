# Complete FluxyLogger NASO Assembly Guide

Guide for assembling NASO and NASO+LCD configurations of the FluxyLogger for airflow tracing in caves.

---

## Index

- [NASO Base Configuration](#naso-base-configuration)
- [NASO+LCD Configuration](#nasolcd-configuration)
- [Firmware Upload](#firmware-upload)
- [Functional Testing](#functional-testing)

---

## NASO Base Configuration

### Required Components

1. Arduino UNO (or Arduino UNO WiFi R4)
2. Data Logger Shield with RTC DS1307
3. MQ-2 Sensor (combustible gases)
4. MicroSD Card (FAT32 formatted)
5. CR1220 Battery for RTC
6. USB PowerBank (minimum 10000mAh)
7. USB Type A-B Cable
8. Enclosure box
9. Dupont male-female jumper wires

### NASO Base Wiring Diagram

![MQ-2 NASO Diagram](../wiring/connections_MQ2_NASO.png)

**MQ-2 Sensor Connection Table:**

| MQ-2 Pin | Shield Connection |
|----------|-------------------|
| VCC      | +5V               |
| GND      | GND               |
| AOUT     | A0                |
| DOUT     | Not connected     |

### NASO Base Assembly

#### 1. Shield Preparation

- Insert CR1220 battery into Data Logger Shield
- Insert formatted microSD (FAT32) into slot

#### 2. Shield Mounting on Arduino

- Align and insert Data Logger Shield onto Arduino UNO
- Verify all pins are correctly inserted

#### 3. MQ-2 Sensor Connection

**With Arduino powered off:**

- **Sensor VCC → Shield +5V** (red wire)
- **Sensor GND → Shield GND** (black wire)
- **Sensor AOUT → Shield A0** (yellow/green wire)

#### 4. Box Positioning

1. **Arduino+Shield:** secure in box with foam/sponge
2. **MQ-2 Sensor:** must remain exposed to outside air, extend it outside the box
3. **USB Cable:** drill hole and seal with silicone

**Power consumption:** ~210 mA
**Autonomy:** ~48-72 hours with 10000mAh PowerBank

---

## NASO+LCD Configuration

### Additional Components

Besides NASO base components:

1. LCD Display 16x2 I2C (with integrated I2C module)
2. Momentary pushbutton (optional, for backlight control)
3. Transparent enclosure box

### NASO+LCD Wiring Diagrams

#### MQ-2 Sensor Diagram

![MQ-2 NASO Diagram](../wiring/connections_MQ2_NASO.png)

#### LCD I2C Display Diagram

![LCD I2C Diagram](../wiring/connections_lcd.png)

*Alternative version:*

![LCD I2C Alternative Diagram](../wiring/connections_LCD_LCM1602_I2C_version.png)

### NASO+LCD Connection Tables

**MQ-2 Sensor:**

| MQ-2 Pin | Shield Connection |
|----------|-------------------|
| VCC      | +5V               |
| GND      | GND               |
| AOUT     | A0                |

**LCD I2C Display:**

| LCD I2C Pin | Shield Connection |
|-------------|-------------------|
| VCC         | +5V               |
| GND         | GND               |
| SDA         | A4 (SDA)          |
| SCL         | A5 (SCL)          |

**Backlight Pushbutton (optional):**

| Component | Connection         |
|-----------|--------------------|
| Pin 1     | +5V                |
| Pin 2     | Digital Pin 8 (D8) |

**Note:** Firmware activates internal pull-up on D8, no external resistor needed.

### NASO+LCD Assembly

#### 1-2. Preparation and Shield Mounting

Same as NASO base (steps 1 and 2)

#### 3. MQ-2 Sensor Connection

Same as NASO base (step 3)

#### 4. LCD I2C Display Connection

**With Arduino powered off:**

- **Display VCC → Shield +5V** (red wire)
- **Display GND → Shield GND** (black wire)
- **Display SDA → Shield A4** (blue/green wire)
- **Display SCL → Shield A5** (yellow/white wire)

**Note:** I2C address typically 0x27 or 0x3F (automatically detected by firmware)

#### 5. Pushbutton Connection (optional)

If manual backlight control is desired:

- One terminal → +5V
- Other terminal → D8

**Diagram:**

```text
+5V ----[Pushbutton]---- D8
```

**Technical note:** Firmware activates internal pull-up on D8, so no external resistor is needed.

**Operation:** Press > 1 sec turns backlight on/off

**Without pushbutton:** Backlight remains always on

#### 6. Complete Connection Verification

- [ ] No VCC/GND short circuit
- [ ] Sensor: AOUT on A0
- [ ] LCD: SDA on A4, SCL on A5
- [ ] Pushbutton: +5V on one terminal, D8 on the other (if installed)
- [ ] MicroSD and RTC battery installed

#### 7. Box Positioning

1. **Display:** mount on transparent box wall (double-sided tape or screws)
2. **Arduino+Shield:** secure on bottom
3. **MQ-2 Sensor:** must remain exposed to outside air, extend it outside the box
4. **Pushbutton (if installed):** verify accessibility from outside
5. **Cables:** avoid tension on I2C connections

#### 8. Display Contrast Adjustment

If display is not readable:

- Rotate blue trimmer on I2C module with small screwdriver
- Adjust until optimal contrast is achieved

### Display Usage

**During acquisition:**

```
ADC:0125 PPM:045
16:32 Det:0003
```

- **ADC:** Raw sensor value (0-1023)
- **PPM:** Parts per million
- **HH:MM:** Current time
- **Det:** Consecutive positive detections

**Power consumption:** ~275 mA (with backlit LCD)
**Autonomy:** ~36-48 hours with 10000mAh PowerBank

---

## Firmware Upload

### Arduino IDE Preparation

1. Install Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
2. Connect Arduino to PC via USB
3. Select: **Tools → Board → Arduino UNO**
4. Select port: **Tools → Port → COMx** (Windows) or **/dev/ttyACMx** (Linux)

### Required Libraries

Install via **Tools → Manage Libraries:**

**For NASO base:**
- RTClib (by Adafruit)
- SD (built-in)

**For NASO+LCD add:**
- LiquidCrystal_I2C (by Frank de Brabander)

### Sketch Upload

1. Download firmware: [FluxyLogger.ino](https://github.com/speleoalex/fluxylogger/blob/main/FluxyLogger/FluxyLogger.ino)
2. Open `.ino` file with Arduino IDE
3. **For NASO+LCD:** verify that `#define LCD_ENABLED` is enabled in code
4. Click: **Sketch → Verify/Compile**
5. Click: **Sketch → Upload**
6. Wait for: "Done uploading"

### Initial Configuration

1. Open: **Tools → Serial Monitor**
2. Baud rate: **19200**
3. Follow instructions for initial configuration

---

## Functional Testing

### Initial Test

**Serial Connection:**

- USB → PC
- Serial: 19200 baud
- Monitor: Arduino IDE or webapp [https://applications.techmakers.it/datalogger/terminal.htm](https://applications.techmakers.it/datalogger/terminal.htm)

**Initialization Check:**

- LEDs blink during preheat (~30 sec)
- ADC values stabilize (10-150)
- No SD card/RTC errors

**Date/Time Configuration:**

```text
settime 2025-01-24 14:30:00
```

**NASO Calibration:**

```text
autocalib
```

Wait 30 seconds, then:

```text
setcalib
```

### MQ-2 Sensor Test

- Spray deodorant near sensor
- ADC/PPM values increase significantly
- LCD display (if present) updates values in real time

### MicroSD Verification

**During acquisition:**

```text
test
```

Expected output: `OK writing to SD`

**Test when powered off:**

- Remove microSD
- Insert into PC card reader
- Verify CSV file with current date
- Open with text editor/Excel
- Verify columns and timestamps are correct

### LCD Display Verification (NASO+LCD)

- [ ] Display shows firmware version at startup
- [ ] ADC/PPM values update in real time
- [ ] Correct time displayed
- [ ] Backlight pushbutton works (press > 1 sec)
- [ ] Readable contrast

**If characters are illegible:** rotate blue trimmer on I2C module

---

## Related Documentation

**Assembly Guides:**

- [Guida Assemblaggio NASO (IT)](NASO_assembly_complete_guide_it.md)
- [NASO Assembly Guide (EN)](NASO_assembly_complete_guide_en.md) - This document
- [Guida Assemblaggio METEO/VOC (IT)](METEO_VOC_assembly_guide_it.md)
- [METEO/VOC Assembly Guide (EN)](METEO_VOC_assembly_guide_en.md)

**User Manuals:**

- [Manuale Utente NASO (IT)](../../manuals/FluxyLogger-NASO-it.md)
- [NASO User Manual (EN)](../../manuals/FluxyLogger-NASO-en.md)
- [Manuale Utente NASO+LCD (IT)](../../manuals/FluxyLogger-NASO_lcd-it.md)
- [NASO+LCD User Manual (EN)](../../manuals/FluxyLogger-NASO_lcd-en.md)

**Other:**

- [Aggiornamento Firmware (IT)](../../manuals/UpdateFirmware_it.md)
- [Firmware Update (EN)](../../manuals/UpdateFirmware_en.md)
- [Components List](../components.md)
- [Platform Overview (IT)](../../FluxyLogger-Platform-Overview-it.md)
- [Platform Overview (EN)](../../FluxyLogger-Platform-Overview-en.md)

---

## Support

- **Telegram:** [https://t.me/+u5CoELQNjC1iODZk](https://t.me/+u5CoELQNjC1iODZk)
- **GitHub:** [https://github.com/speleoalex/fluxylogger/issues](https://github.com/speleoalex/fluxylogger/issues)
- **Email:** speleoalex@gmail.com

---

**FluxyLogger Project**
**Version:** 1.0 - January 2025
**License:** GNU General Public License
**Author:** Alessandro Vernassa
