# FluxyLogger - Modular Platform

![FluxyLogger](Naso.png)

## What is it

**FluxyLogger** is an **open source** platform based on **Arduino UNO** to create configurable dataloggers with different sensors.

## Base Components

Common to all configurations:

- Arduino UNO (or WiFi R4)
- Data Logger Shield with RTC
- MicroSD Card
- USB PowerBank
- Enclosure box
- Connection wires

## Available Sensors

| Sensor | Model | Usage |
|--------|-------|-------|
| LPG/Butane Gas | MQ-2 | Air tracing |
| Temperature/Pressure | BMP280 | Weather |
| Wind | Wind Sensor | Wind speed |
| VOC | SGP40/SGP30 | Air quality |
| Display | LCD I2C 16x2 | Visualization |

---

## Configurations

### 🌬️ NASO - Air Tracing

Detects tracer gases (butane/propane) to study air flows in caves.

- **Sensor**: MQ-2
- **Use**: Verify connections between cave entrances
- **Consumption**: ~275 mA (measured experimentally)
- **Battery life**: ~36h (10000mAh powerbank)

**Firmware config**:

```cpp
#define MQ2SENSOR_PRESENT 1
#define LCD_I2C_ENABLED 0
```

**Documentation**: [IT](manuals/FluxyLogger-NASO-it.md) | [EN](manuals/FluxyLogger-NASO-en.md)
**Purchase**: [Assembled kit](https://techmakers.eu/products/new-cave-monitoring-n-a-s-o-datalogger-for-atmospheric-tracer-tracking-assembled)

---

### 📱 NASO + LCD

Like NASO but with display showing real-time values (ADC, PPM, detections).

- **Sensors**: MQ-2 + LCD 16x2
- **Consumption**: ~210 mA (calculated)
- **Battery life**: ~48h (10000mAh powerbank)

**Firmware config**:

```cpp
#define MQ2SENSOR_PRESENT 1
#define LCD_I2C_ENABLED 1
```

**Documentation**: [IT](manuals/FluxyLogger-NASO_lcd-it.md) | [EN](manuals/FluxyLogger-NASO_lcd-en.md)

---

### 🌤️ METEO - Weather Station

Monitors temperature, pressure, humidity, wind.

- **Sensors**: BMP280 + Wind Sensor
- **Use**: Cave microclimatology
- **Consumption**: ~50 mA (calculated)
- **Battery life**: ~200h (10000mAh powerbank)

**Firmware config**:

```cpp
#define BMP280_PRESENT 1
#define WINDSENSOR_PRESENT 1
```

**Documentation**: *In preparation*

---

### 🏭 VOC - Air Quality

Detects Volatile Organic Compounds (pollutants, solvents).

- **Sensor**: SGP40 or SGP30
- **Use**: Indoor/tourist cave air quality
- **Consumption**: ~50 mA (calculated, SGP40)
- **Battery life**: ~200h (10000mAh powerbank)

**Firmware config**:

```cpp
#define SGP40_PRESENT 1
```

**Documentation**: *In preparation*

---

## Comparison Table

| Feature | NASO | NASO+LCD | METEO | VOC |
|---------|------|----------|-------|-----|
| Air tracing | ✅ | ✅ | ❌ | ❌ |
| Weather | ❌ | ❌ | ✅ | ❌ |
| Air quality | ❌ | ❌ | ❌ | ✅ |
| Display | ❌ | ✅ | Opt. | Opt. |
| Consumption | 275mA¹ | 210mA | 50mA | 50mA |
| Battery life² | 36h | 48h | 200h | 200h |
| Documentation | ✅ | ✅ | 📝 | 📝 |
| Kit available | ✅ | ✅ | ❌ | ❌ |

¹ Measured experimentally; other values calculated from datasheets
² With 10000mAh powerbank

## Component Power Consumption

| Component | Current Draw | Notes |
|-----------|--------------|-------|
| Arduino UNO | ~45 mA | Base consumption |
| Data Logger Shield + RTC | ~2 mA | DS1307 + passive components |
| MicroSD Card | ~0.5 mA | Average (write every 30s, duty cycle 0.67%) |
| **MQ-2 Gas Sensor** | **~160 mA** | Heating element (main consumer) |
| LCD 16x2 I2C | ~3 mA | Without backlight (button activated) |
| BMP280 | ~1 mA | Normal mode |
| Wind Sensor | ~1 mA | Analog output |
| SGP40 | ~3 mA | Average during measurement |
| SGP30 | ~12 mA | Average with duty cycle |

**Base consumption** (Arduino + Shield + SD): ~47.5 mA

---

## Firmware Configuration

Edit values in [`FluxyLogger.ino`](FluxyLogger/FluxyLogger.ino):

```cpp
#define MQ2SENSOR_PRESENT 1      // 0=off, 1=on
#define BMP280_PRESENT 0
#define WINDSENSOR_PRESENT 0
#define SGP40_PRESENT 0
#define LCD_I2C_ENABLED 1
```

You can combine sensors (e.g. NASO + METEO) checking consumption and available pins.

---

## Data Management

All data is saved to microSD in CSV format with timestamp.

**Visualization**:

- **Web App**: [LoggerViewer](https://applications.techmakers.it/datalogger/loggermanager.htm) (Chrome, Edge)
- **Desktop App**: [LoggerViewer Desktop](LoggerViewer_Desktop/README.md) (Linux, Windows, macOS)
- **Spreadsheet**: Excel, LibreOffice (separator `;`)

**CSV Format**:

```csv
date Y-m-d m:s;gas adc;LPG PPM
2023-11-17 17:16:02;80;0
2023-11-17 17:17:02;125;45
```

---

## Useful Links

**Documentation**:

- [Firmware Update](manuals/UpdateFirmware_it.md)
- [Components List](doc/components.md)
- [Wiring Diagrams](doc/)
- [Project History](Progetto_Fluxylogger_NASO.md)

**Community**:

- [Telegram Group](https://t.me/+u5CoELQNjC1iODZk)
- [GitHub](https://github.com/speleoalex/fluxylogger)
- [AI Assistant](https://www.sparkilla.com/application/NASO4CAVE)

**Support**:

[![Donate with PayPal](paypal.png)](https://www.paypal.com/donate/?business=TKQWLKGENEP7L&no_recurring=0&item_name=Progetto+FluxyLogger+NASO&currency_code=EUR)

---

**License**: GNU GPL | **Author**: Alessandro Vernassa | **Firmware Version**: 2.45
