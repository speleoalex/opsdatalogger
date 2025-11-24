# FluxyLogger METEO and VOC Assembly Guide

Essential guide for assembling METEO (weather station) and VOC (air quality) configurations of the FluxyLogger.

---

## Index

- [METEO Configuration](#meteo-configuration)
- [VOC Configuration](#voc-configuration)
- [Functional Testing](#functional-testing)
- [Firmware Notes](#firmware-notes)

---

## METEO Configuration

Weather station with barometric sensor and anemometer.

### Components

**Base (same as NASO):**
1. Arduino UNO (or WiFi R4)
2. Data Logger Shield with RTC DS1307
3. MicroSD Card (FAT32)
4. CR1220 Battery
5. USB PowerBank
6. Enclosure box

**Specific Sensors:**
1. **BMP280** - Pressure and temperature sensor
2. **Anemometer** - Wind speed sensor (optional)

### BMP280 Wiring Diagram

![BMP280 Diagram](../wiring/connections_bmp280.png)

**BMP280 Connection Table (I2C):**

| BMP280 Pin | Shield Connection |
|------------|-------------------|
| VCC        | +3.3V             |
| GND        | GND               |
| SDA        | A4 (SDA)          |
| SCL        | A5 (SCL)          |

**IMPORTANT:** BMP280 operates at **3.3V**, do not connect to 5V!

### Anemometer Wiring Diagram

![Wind Sensor Diagram](../wiring/connections_wind.png)

**Anemometer Connection Table:**

| Anemometer Pin | Shield Connection |
|----------------|-------------------|
| VCC            | +5V               |
| GND            | GND               |
| OUT            | A1                |

### METEO Assembly

#### 1. Base Preparation

Same as NASO base:
- Insert CR1220 battery
- Mount shield on Arduino
- Insert microSD

#### 2. BMP280 Connection

**With Arduino powered off:**

- **VCC → +3.3V** (red wire) ⚠️ NOT 5V!
- **GND → GND** (black wire)
- **SDA → A4** (blue/green wire)
- **SCL → A5** (yellow/white wire)

**Verification:** BMP280 must be connected to 3.3V, not 5V to avoid damage

#### 3. Anemometer Connection (optional)

**With Arduino powered off:**

- **VCC → +5V** (red wire)
- **GND → GND** (black wire)
- **OUT → A1** (yellow wire)

#### 4. Positioning

- **Arduino+Shield:** in box, secured
- **BMP280:** inside box (measures atmospheric pressure)
- **Anemometer:** outside, exposed to wind
- **USB Cable:** sealed hole with silicone

### METEO Specifications

- **Power consumption:** ~200 mA
- **Sensors:** BMP280 (pressure, temperature), Anemometer (wind)
- **Protocol:** I2C for BMP280, Analog for anemometer
- **Acquisition interval:** Configurable (default 60s)

---

## VOC Configuration

Air quality monitoring with VOC (Volatile Organic Compounds) sensor.

### Components

**Base (same as NASO):**
1. Arduino UNO (or WiFi R4)
2. Data Logger Shield with RTC DS1307
3. MicroSD Card (FAT32)
4. CR1220 Battery
5. USB PowerBank
6. Transparent enclosure box (optional LCD)

**Specific Sensors:**
1. **SGP40** or **SGP30** - VOC sensor (air quality)
2. **LCD I2C Display** (optional, like NASO+LCD)

### SGP40 Wiring Diagram

![SGP40 Diagram](../wiring/connections_sgp40.png)

**SGP40 Connection Table (I2C):**

| SGP40 Pin | Shield Connection |
|-----------|-------------------|
| VCC       | +5V               |
| GND       | GND               |
| SDA       | A4 (SDA)          |
| SCL       | A5 (SCL)          |

### VOC Assembly

#### 1. Base Preparation

Same as NASO base:
- Insert CR1220 battery
- Mount shield on Arduino
- Insert microSD

#### 2. SGP40 Connection

**With Arduino powered off:**

- **VCC → +5V** (red wire)
- **GND → GND** (black wire)
- **SDA → A4** (blue/green wire)
- **SCL → A5** (yellow/white wire)

**Note:** SGP40 requires ~30 seconds preheat to stabilize

#### 3. LCD Display (optional)

If real-time display is desired:

**Connection like NASO+LCD:**
- VCC → +5V
- GND → GND
- SDA → A4 (shared with SGP40)
- SCL → A5 (shared with SGP40)

**I2C Multi-device:**
- SGP40 and LCD share the same I2C bus (A4/A5)
- Different addresses: SGP40 (0x59), LCD (0x27 or 0x3F)
- No conflict between devices

#### 4. Positioning

- **Arduino+Shield:** in box, secured
- **SGP40:** exposed to air, position near ventilation holes
- **LCD (if present):** on transparent box wall
- **USB Cable:** sealed hole

### VOC Specifications

- **Power consumption:** ~220 mA (without LCD), ~285 mA (with LCD)
- **Sensor:** SGP40 or SGP30 (VOC index)
- **Protocol:** I2C
- **Measured parameters:** VOC index, TVOC (ppb)
- **Acquisition interval:** Configurable (default 30s)

---

## Functional Testing

### Initial Test

1. **Serial Connection:**
   - USB → PC
   - Serial: 19200 baud
   - Monitor: Arduino IDE or webapp [https://applications.techmakers.it/datalogger/terminal.htm](https://applications.techmakers.it/datalogger/terminal.htm)

2. **Initialization Check:**
   - LEDs blink during preheat (~30 sec)
   - ADC values stabilize (10-150 for NASO)
   - No SD card/RTC errors

3. **Date/Time Configuration:**

```text
settime 2025-01-24 14:30:00
```

4. **NASO Calibration:**

```text
autocalib
```

Wait 30 seconds, then:

```text
setcalib
```

### Sensor Tests

**NASO (MQ-2):**

- Spray deodorant near sensor
- ADC/PPM values increase significantly
- LCD display (if present) updates values in real time

**METEO (BMP280):**

- Blow on sensor → temperature increases
- Stable pressure values (~1013 hPa at sea level)

**VOC (SGP40):**

- Wait 30 sec preheat
- VOC index baseline ~100
- Bring alcohol/deodorant close → VOC index increases

**Anemometer:**

- Anemometer still = minimum value
- Rotate manually = increasing values

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

### LCD Display Verification (NASO+LCD / VOC+LCD)

- [ ] Display shows firmware version at startup
- [ ] ADC/PPM/VOC values update in real time
- [ ] Correct time
- [ ] Backlight pushbutton works (press > 1 sec)
- [ ] Readable contrast

**If characters are illegible:** rotate blue trimmer on I2C module

---

## Firmware Notes

### Current Status (FluxyLogger v2.45)

✅ **Firmware includes complete METEO and VOC support**

The FluxyLogger firmware includes all necessary code for METEO and VOC, but sensors are disabled by default. To enable them:

**File: `FluxyLogger/FluxyLogger.ino`**

1. **METEO Configuration - Modify lines:**

```cpp
#define BMP280_PRESENT 1        // Line 24: 0 → 1
#define WINDSENSOR_PRESENT 1    // Line 33: 0 → 1 (optional)
```

2. **VOC Configuration - Modify lines:**

```cpp
#define SGP40_PRESENT 1         // Line 40: 0 → 1
// or
#define SGP30_PRESENT 1         // Line 41: 0 → 1 (alternative)
```

3. **Required libraries:**
   - **BMP280:** Already included (`farmerkeith_BMP280.h`)
   - **SGP40:** Already included (`SparkFun_SGP40_Arduino_Library.h`)
   - **SGP30:** `SGP30.h` from Arduino Library Manager

**Already implemented features:**
- ✅ Sensor initialization
- ✅ Data reading (temperature, pressure, wind, VOC)
- ✅ CSV saving with correct headers
- ✅ Automatic calibration (BMP280, SGP40)

**Calibration:**
- BMP280: automatic at boot
- SGP40: VOC baseline after ~12h continuous operation
- Anemometer: no calibration required

### I2C Multi-device Configuration

For VOC+LCD (both on I2C):

```text
Arduino A4 (SDA) ----+---- SGP40 SDA
                     |
                     +---- LCD SDA

Arduino A5 (SCL) ----+---- SGP40 SCL
                     |
                     +---- LCD SCL
```

Each device has unique address:
- SGP40: 0x59
- LCD: 0x27 or 0x3F
- BMP280 (METEO): 0x76 or 0x77

### BMP280 Initialization Example Code

```cpp
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // I2C

void setup() {
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found!");
    while (1);
  }

  // Configuration
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void loop() {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0; // hPa
  float altitude = bmp.readAltitude(1013.25); // m

  // Save to SD...
}
```

### SGP40 Initialization Example Code

```cpp
#include <Adafruit_SGP40.h>

Adafruit_SGP40 sgp;

void setup() {
  if (!sgp.begin()) {
    Serial.println("SGP40 not found!");
    while (1);
  }
}

void loop() {
  uint16_t raw_voc = sgp.measureRaw();
  int32_t voc_index = sgp.measureVocIndex();

  // Save to SD...
}
```

---

## CSV Data Format

### METEO

```csv
date Y-m-d H:M:S;temperature_C;pressure_hPa;altitude_m;wind_speed_ms
2025-01-24 12:30:00;15.2;1013.5;450;2.3
2025-01-24 12:31:00;15.3;1013.6;451;2.1
```

### VOC

```csv
date Y-m-d H:M:S;voc_index;tvoc_ppb;temperature_C;humidity_RH
2025-01-24 12:30:00;125;450;22.5;55
2025-01-24 12:30:30;130;475;22.6;54
```

---

## Troubleshooting

### BMP280 not detected

- Verify 3.3V connection (NOT 5V!)
- Check SDA/SCL on A4/A5
- Try alternative address: `bmp.begin(0x77)`
- Verify solder joints on sensor pins

### SGP40 not detected

- Verify 5V power supply
- Check SDA/SCL on A4/A5
- Wait for preheat (30 sec)
- Verify library installed correctly

### I2C multi-device conflict

- Run I2C Scanner to verify addresses
- Verify devices have different addresses
- Check SDA/SCL cables not too long (max 30cm)
- Add 4.7kΩ pull-up resistors on SDA/SCL if problems occur

### Anemometer incorrect values

- Verify 5V power supply
- Check OUT → A1 connection
- Calibrate: stop anemometer = minimum value
- Rotate anemometer = increasing values

---

## I2C Scanner Test

Before everything, verify sensors are detected:

```cpp
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(19200);
  Serial.println("I2C Scanner");
}

void loop() {
  for(byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if(Wire.endTransmission() == 0) {
      Serial.print("Device found at 0x");
      Serial.println(i, HEX);
    }
  }
  delay(5000);
}
```

**Expected addresses:**
- BMP280: 0x76 or 0x77
- SGP40: 0x59
- LCD: 0x27 or 0x3F

### BMP280 Test

1. Upload example code
2. Serial Monitor (19200 baud)
3. Verify temperature and pressure readings
4. Blow on sensor → temperature increases

### SGP40 Test

1. Upload example code
2. Wait 30 sec preheat
3. Verify VOC index baseline (~100)
4. Bring alcohol/deodorant close → VOC index increases

---

## Future Development

METEO and VOC configurations are in development. Contributions welcome:

**Roadmap:**
- [ ] Complete METEO firmware
- [ ] Complete VOC firmware
- [ ] Automatic sensor calibration
- [ ] Detailed user manuals
- [ ] Specific test procedures
- [ ] Field usage examples

**Contribute:**
- GitHub: [https://github.com/speleoalex/fluxylogger](https://github.com/speleoalex/fluxylogger)
- Telegram: [https://t.me/+u5CoELQNjC1iODZk](https://t.me/+u5CoELQNjC1iODZk)

---

## Related Documentation

**Assembly Guides:**

- [Guida Assemblaggio NASO (IT)](NASO_assembly_complete_guide_it.md)
- [NASO Assembly Guide (EN)](NASO_assembly_complete_guide_en.md)
- [Guida Assemblaggio METEO/VOC (IT)](METEO_VOC_assembly_guide_it.md)
- [METEO/VOC Assembly Guide (EN)](METEO_VOC_assembly_guide_en.md) - This document

**Other:**

- [Components List](../components.md)
- [BMP280 Datasheet](../datasheets/)
- [SGP40 Datasheet](../datasheets/sgp40.pdf)
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
**Status:** METEO and VOC in development
