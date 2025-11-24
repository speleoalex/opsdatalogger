# FluxyLogger

![FluxyLogger](Naso.png)

**FluxyLogger** is an open source modular datalogger platform based on Arduino UNO for environmental monitoring, originally developed for cave air tracing studies.

## 🎯 Quick Start

**New here?** Start with the Platform Overview to understand available configurations:

- 📘 [**Platform Overview (English)**](FluxyLogger-Platform-Overview-en.md)
- 📘 [**Panoramica Piattaforma (Italiano)**](FluxyLogger-Platform-Overview-it.md)

## 📦 Available Configurations

FluxyLogger can be configured for different applications:

| Configuration | Sensors | Use Case | Documentation |
|---------------|---------|----------|---------------|
| 🌬️ **NASO** | MQ-2 | Air tracing in caves | [IT](manuals/FluxyLogger-NASO-it.md) \| [EN](manuals/FluxyLogger-NASO-en.md) |
| 📱 **NASO + LCD** | MQ-2 + Display | Air tracing with real-time display | [IT](manuals/FluxyLogger-NASO_lcd-it.md) \| [EN](manuals/FluxyLogger-NASO_lcd-en.md) |
| 🌤️ **METEO** | BMP280 + Wind | Weather station | *In preparation* |
| 🏭 **VOC** | SGP40/SGP30 | Air quality monitoring | *In preparation* |

**Purchase**: Assembled NASO units available at [TechMakers](https://techmakers.eu/products/new-cave-monitoring-n-a-s-o-datalogger-for-atmospheric-tracer-tracking-assembled)

## 🔧 Base Components

All configurations share:

- Arduino UNO (or WiFi R4)
- Data Logger Shield with RTC
- MicroSD Card
- USB PowerBank
- Enclosure box
- Connection wires

[Full component list](doc/components.md)

## 📖 Documentation

### User Guides

- [Platform Overview IT](FluxyLogger-Platform-Overview-it.md) - Panoramica completa delle configurazioni
- [Platform Overview EN](FluxyLogger-Platform-Overview-en.md) - Complete configuration overview
- [NASO Manual IT](manuals/FluxyLogger-NASO-it.md) - Manuale utente configurazione NASO
- [NASO Manual EN](manuals/FluxyLogger-NASO-en.md) - NASO configuration user manual
- [Update Firmware IT](manuals/UpdateFirmware_it.md) - Aggiornamento firmware
- [Project History IT](Progetto_Fluxylogger_NASO.md) - Storia del progetto

### Technical Documentation

- [Wiring Diagrams](doc/) - Connection schematics for all sensors
- [Source Code](FluxyLogger/FluxyLogger.ino) - Arduino firmware
- [CAD Files](drawings/) - 3D models and enclosure drawings

### Software Tools

- [LoggerViewer Web App](https://applications.techmakers.it/datalogger/loggermanager.htm) - Browser-based data viewer
- [LoggerViewer Desktop](LoggerViewer_Desktop/README.md) - Cross-platform desktop application
- [AI Assistant](https://www.sparkilla.com/application/NASO4CAVE) - Virtual assistant for log analysis

## 💾 Data Format

All configurations save data to microSD as CSV files with timestamps:

```csv
date Y-m-d m:s;gas adc;LPG PPM
2023-11-17 17:16:02;80;0
2023-11-17 17:17:02;125;45
```

Measurement interval configurable via CONFIG.INI or USB connection.

## 🌍 Community & Support

- **Telegram Group**: [https://t.me/+u5CoELQNjC1iODZk](https://t.me/+u5CoELQNjC1iODZk)
- **GitHub**: [https://github.com/speleoalex/opsdatalogger](https://github.com/speleoalex/opsdatalogger)
- **Issues**: Report bugs and request features on GitHub Issues

### Support the Project

If you find this project useful, consider supporting its development:

[![Donate with PayPal](paypal.png)](https://www.paypal.com/donate/?business=TKQWLKGENEP7L&no_recurring=0&item_name=Progetto+FluxyLogger+NASO&currency_code=EUR)

## 📜 License & Credits

- **License**: GNU General Public License
- **Author**: Alessandro Vernassa (speleoalex@gmail.com)
- **Website**: <https://techmakers.eu/>
- **Firmware Version**: 2.45
- **Started**: 2020

---

**Quick Links**: [Platform Overview](FluxyLogger-Platform-Overview-en.md) | [NASO Manual](manuals/FluxyLogger-NASO-en.md) | [Firmware Update](manuals/UpdateFirmware_it.md) | [Components](doc/components.md)
