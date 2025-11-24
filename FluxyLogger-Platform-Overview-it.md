# FluxyLogger - Piattaforma Modulare

![FluxyLogger](Naso.png)

## Cos'è

**FluxyLogger** è una piattaforma **open source** basata su **Arduino UNO** per creare datalogger configurabili con diversi sensori.

## Componenti Base

Comuni a tutte le configurazioni:

- Arduino UNO (o WiFi R4)
- Data Logger Shield con RTC
- MicroSD Card
- PowerBank USB
- Box contenitore
- Cavi collegamento

## Sensori Disponibili

| Sensore | Modello | Utilizzo |
|---------|---------|----------|
| Gas GPL/Butano | MQ-2 | Tracciamenti aerei |
| Temperatura/Pressione | BMP280 | Meteo |
| Vento | Wind Sensor | Velocità vento |
| VOC | SGP40/SGP30 | Qualità aria |
| Display | LCD I2C 16x2 | Visualizzazione |

---

## Configurazioni

### 🌬️ NASO - Tracciamenti Aerei

Rileva gas traccianti (butano/propano) per studiare flussi d'aria in grotte.

- **Sensore**: MQ-2
- **Uso**: Verificare connessioni tra ingressi di cavità
- **Consumo**: ~275 mA (misurato sperimentalmente)
- **Autonomia**: ~36h (powerbank 10000mAh)

**Config firmware**:

```cpp
#define MQ2SENSOR_PRESENT 1
#define LCD_I2C_ENABLED 0
```

**Documentazione**: [IT](manuals/FluxyLogger-NASO-it.md) | [EN](manuals/FluxyLogger-NASO-en.md)
**Acquisto**: [Kit assemblato](https://techmakers.eu/products/new-cave-monitoring-n-a-s-o-datalogger-for-atmospheric-tracer-tracking-assembled)

---

### 📱 NASO + LCD

Come NASO ma con display che mostra valori real-time (ADC, PPM, rilevamenti).

- **Sensori**: MQ-2 + LCD 16x2
- **Consumo**: ~210 mA (calcolato)
- **Autonomia**: ~48h (powerbank 10000mAh)

**Config firmware**:

```cpp
#define MQ2SENSOR_PRESENT 1
#define LCD_I2C_ENABLED 1
```

**Documentazione**: [IT](manuals/FluxyLogger-NASO_lcd-it.md) | [EN](manuals/FluxyLogger-NASO_lcd-en.md)

---

### 🌤️ METEO - Stazione Meteo

Monitora temperatura, pressione, umidità, vento.

- **Sensori**: BMP280 + Wind Sensor
- **Uso**: Microclimatologia grotte
- **Consumo**: ~50 mA (calcolato)
- **Autonomia**: ~200h (powerbank 10000mAh)

**Config firmware**:

```cpp
#define BMP280_PRESENT 1
#define WINDSENSOR_PRESENT 1
```

**Documentazione**: *In preparazione*

---

### 🏭 VOC - Qualità Aria

Rileva Composti Organici Volatili (inquinanti, solventi).

- **Sensore**: SGP40 o SGP30
- **Uso**: Qualità aria indoor/grotte turistiche
- **Consumo**: ~50 mA (calcolato, SGP40)
- **Autonomia**: ~200h (powerbank 10000mAh)

**Config firmware**:

```cpp
#define SGP40_PRESENT 1
```

**Documentazione**: *In preparazione*

---

## Tabella Comparativa

| Caratteristica | NASO | NASO+LCD | METEO | VOC |
|----------------|------|----------|-------|-----|
| Tracciamenti | ✅ | ✅ | ❌ | ❌ |
| Meteo | ❌ | ❌ | ✅ | ❌ |
| Qualità aria | ❌ | ❌ | ❌ | ✅ |
| Display | ❌ | ✅ | Opz. | Opz. |
| Consumo | 275mA¹ | 210mA | 50mA | 50mA |
| Autonomia² | 36h | 48h | 200h | 200h |
| Documentazione | ✅ | ✅ | 📝 | 📝 |
| Kit disponibile | ✅ | ✅ | ❌ | ❌ |

¹ Misurato sperimentalmente; altri valori calcolati da datasheet
² Con powerbank 10000mAh

## Consumo Componenti

| Componente | Assorbimento | Note |
|------------|--------------|------|
| Arduino UNO | ~45 mA | Consumo base |
| Data Logger Shield + RTC | ~2 mA | DS1307 + componenti passivi |
| Scheda MicroSD | ~0.5 mA | Media (scrittura ogni 30s, duty cycle 0.67%) |
| **Sensore MQ-2** | **~160 mA** | Elemento riscaldante (consumatore principale) |
| LCD 16x2 I2C | ~3 mA | Senza retroilluminazione (attivabile con pulsante) |
| BMP280 | ~1 mA | Modalità normale |
| Wind Sensor | ~1 mA | Uscita analogica |
| SGP40 | ~3 mA | Media durante misurazione |
| SGP30 | ~12 mA | Media con duty cycle |

**Consumo base** (Arduino + Shield + SD): ~47.5 mA

---

## Configurazione Firmware

Modifica i valori in [`FluxyLogger.ino`](FluxyLogger/FluxyLogger.ino):

```cpp
#define MQ2SENSOR_PRESENT 1      // 0=off, 1=on
#define BMP280_PRESENT 0
#define WINDSENSOR_PRESENT 0
#define SGP40_PRESENT 0
#define LCD_I2C_ENABLED 1
```

Puoi combinare sensori (es. NASO + METEO) verificando consumo e pin disponibili.

---

## Gestione Dati

Tutti i dati sono salvati su microSD in formato CSV con timestamp.

**Visualizzazione**:

- **Web App**: [LoggerViewer](https://applications.techmakers.it/datalogger/loggermanager.htm) (Chrome, Edge)
- **Desktop App**: [LoggerViewer Desktop](LoggerViewer_Desktop/README.md) (Linux, Windows, macOS)
- **Foglio calcolo**: Excel, LibreOffice (separatore `;`)

**Formato CSV**:

```csv
date Y-m-d m:s;gas adc;LPG PPM
2023-11-17 17:16:02;80;0
2023-11-17 17:17:02;125;45
```

---

## Link Utili

**Documentazione**:

- [Aggiornamento Firmware](manuals/UpdateFirmware_it.md)
- [Lista Componenti](doc/components.md)
- [Schemi Elettrici](doc/)
- [Storia Progetto](Progetto_Fluxylogger_NASO.md)

**Comunità**:

- [Gruppo Telegram](https://t.me/+u5CoELQNjC1iODZk)
- [GitHub](https://github.com/speleoalex/opsdatalogger)
- [Assistente AI](https://www.sparkilla.com/application/NASO4CAVE)

**Supporto**:

[![Dona con PayPal](paypal.png)](https://www.paypal.com/donate/?business=TKQWLKGENEP7L&no_recurring=0&item_name=Progetto+FluxyLogger+NASO&currency_code=EUR)

---

**Licenza**: GNU GPL | **Autore**: Alessandro Vernassa | **Versione Firmware**: 2.45
