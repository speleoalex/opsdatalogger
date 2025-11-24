# Guida Assemblaggio FluxyLogger METEO e VOC

Guida essenziale per l'assemblaggio delle configurazioni METEO (stazione meteo) e VOC (qualità aria) del FluxyLogger.

---

## Indice

- [Configurazione METEO](#configurazione-meteo)
- [Configurazione VOC](#configurazione-voc)
- [Note su Firmware](#note-su-firmware)

---

## Configurazione METEO

Stazione meteorologica con sensore barometrico e anemometro.

### Componenti

**Base (come NASO):**
1. Arduino UNO (o WiFi R4)
2. Data Logger Shield con RTC DS1307
3. MicroSD Card (FAT32)
4. Batteria CR1220
5. PowerBank USB
6. Box contenitore

**Sensori Specifici:**
1. **BMP280** - Sensore pressione e temperatura
2. **Anemometro** - Sensore velocità vento (opzionale)

### Schema Collegamenti BMP280

![Schema BMP280](../wiring/connections_bmp280.png)

**Tabella Collegamenti BMP280 (I2C):**

| Pin BMP280 | Collegamento Shield |
|------------|---------------------|
| VCC        | +3.3V               |
| GND        | GND                 |
| SDA        | A4 (SDA)            |
| SCL        | A5 (SCL)            |

**IMPORTANTE:** BMP280 funziona a **3.3V**, non collegare a 5V!

### Schema Collegamenti Anemometro

![Schema Wind Sensor](../wiring/connections_wind.png)

**Tabella Collegamenti Anemometro:**

| Pin Anemometro | Collegamento Shield |
|----------------|---------------------|
| VCC            | +5V                 |
| GND            | GND                 |
| OUT            | A1                  |

### Assemblaggio METEO

#### 1. Preparazione Base

Come per NASO base:
- Inserire batteria CR1220
- Montare shield su Arduino
- Inserire microSD

#### 2. Collegamento BMP280

**Con Arduino spento:**

- **VCC → +3.3V** (cavo rosso) ⚠️ NON 5V!
- **GND → GND** (cavo nero)
- **SDA → A4** (cavo blu/verde)
- **SCL → A5** (cavo giallo/bianco)

**Verifica:** BMP280 deve essere collegato a 3.3V, non 5V per evitare danni

#### 3. Collegamento Anemometro (opzionale)

**Con Arduino spento:**

- **VCC → +5V** (cavo rosso)
- **GND → GND** (cavo nero)
- **OUT → A1** (cavo giallo)

#### 4. Posizionamento

- **Arduino+Shield:** nel box, fissato
- **BMP280:** all'interno del box (misura pressione atmosferica)
- **Anemometro:** all'esterno, esposto al vento
- **Cavo USB:** foro sigillato con silicone

### Specifiche METEO

- **Consumo:** ~200 mA
- **Sensori:** BMP280 (pressione, temperatura), Anemometro (vento)
- **Protocollo:** I2C per BMP280, Analogico per anemometro
- **Intervallo acquisizione:** Configurabile (default 60s)

---

## Configurazione VOC

Monitoraggio qualità aria con sensore VOC (Volatile Organic Compounds).

### Componenti

**Base (come NASO):**
1. Arduino UNO (o WiFi R4)
2. Data Logger Shield con RTC DS1307
3. MicroSD Card (FAT32)
4. Batteria CR1220
5. PowerBank USB
6. Box contenitore trasparente (opzionale LCD)

**Sensori Specifici:**
1. **SGP40** o **SGP30** - Sensore VOC (qualità aria)
2. **Display LCD I2C** (opzionale, come NASO+LCD)

### Schema Collegamenti SGP40

![Schema SGP40](../wiring/connections_sgp40.png)

**Tabella Collegamenti SGP40 (I2C):**

| Pin SGP40 | Collegamento Shield |
|-----------|---------------------|
| VCC       | +5V                 |
| GND       | GND                 |
| SDA       | A4 (SDA)            |
| SCL       | A5 (SCL)            |

### Assemblaggio VOC

#### 1. Preparazione Base

Come per NASO base:
- Inserire batteria CR1220
- Montare shield su Arduino
- Inserire microSD

#### 2. Collegamento SGP40

**Con Arduino spento:**

- **VCC → +5V** (cavo rosso)
- **GND → GND** (cavo nero)
- **SDA → A4** (cavo blu/verde)
- **SCL → A5** (cavo giallo/bianco)

**Nota:** SGP40 richiede preriscaldamento di ~30 secondi per stabilizzarsi

#### 3. Display LCD (opzionale)

Se si desidera visualizzazione real-time:

**Collegamento come NASO+LCD:**
- VCC → +5V
- GND → GND
- SDA → A4 (condiviso con SGP40)
- SCL → A5 (condiviso con SGP40)

**I2C Multi-dispositivo:**
- SGP40 e LCD condividono lo stesso bus I2C (A4/A5)
- Indirizzi diversi: SGP40 (0x59), LCD (0x27 o 0x3F)
- Nessun conflitto tra dispositivi

#### 4. Posizionamento

- **Arduino+Shield:** nel box, fissato
- **SGP40:** esposto all'aria, posizionarlo vicino a fori di ventilazione
- **LCD (se presente):** su parete trasparente del box
- **Cavo USB:** foro sigillato

### Specifiche VOC

- **Consumo:** ~220 mA (senza LCD), ~285 mA (con LCD)
- **Sensore:** SGP40 o SGP30 (VOC index)
- **Protocollo:** I2C
- **Parametri misurati:** VOC index, TVOC (ppb)
- **Intervallo acquisizione:** Configurabile (default 30s)

---

## Note su Firmware

### Stato Attuale (FluxyLogger v2.45)

✅ **Il firmware include già il supporto completo per METEO e VOC**

Il firmware FluxyLogger include tutto il codice necessario per METEO e VOC, ma i sensori sono disabilitati di default. Per attivarli:

**File: `FluxyLogger/FluxyLogger.ino`**

1. **Configurazione METEO - Modificare le linee:**

```cpp
#define BMP280_PRESENT 1        // Linea 24: 0 → 1
#define WINDSENSOR_PRESENT 1    // Linea 33: 0 → 1 (opzionale)
```

2. **Configurazione VOC - Modificare le linee:**

```cpp
#define SGP40_PRESENT 1         // Linea 40: 0 → 1
// oppure
#define SGP30_PRESENT 1         // Linea 41: 0 → 1 (alternativo)
```

3. **Librerie necessarie:**
   - **BMP280:** Già inclusa (`farmerkeith_BMP280.h`)
   - **SGP40:** Già inclusa (`SparkFun_SGP40_Arduino_Library.h`)
   - **SGP30:** `SGP30.h` da Arduino Library Manager

**Funzionalità già implementate:**
- ✅ Inizializzazione sensori
- ✅ Lettura dati (temperatura, pressione, vento, VOC)
- ✅ Salvataggio CSV con header corretti
- ✅ Calibrazione automatica (BMP280, SGP40)

**Calibrazione:**
- BMP280: automatica al boot
- SGP40: baseline VOC dopo ~12h funzionamento continuo
- Anemometro: nessuna calibrazione richiesta

### Configurazione I2C Multi-dispositivo

Per VOC+LCD (entrambi su I2C):

```text
Arduino A4 (SDA) ----+---- SGP40 SDA
                     |
                     +---- LCD SDA

Arduino A5 (SCL) ----+---- SGP40 SCL
                     |
                     +---- LCD SCL
```

Ogni dispositivo ha indirizzo univoco:
- SGP40: 0x59
- LCD: 0x27 o 0x3F
- BMP280 (METEO): 0x76 o 0x77

### Esempio Codice Inizializzazione BMP280

```cpp
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // I2C

void setup() {
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found!");
    while (1);
  }

  // Configurazione
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

  // Salva su SD...
}
```

### Esempio Codice Inizializzazione SGP40

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

  // Salva su SD...
}
```

---

## Formato Dati CSV

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

## Verifica Funzionamento

### Test I2C Scanner

Prima di tutto, verificare che i sensori siano rilevati:

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

**Indirizzi attesi:**
- BMP280: 0x76 o 0x77
- SGP40: 0x59
- LCD: 0x27 o 0x3F

### Test BMP280

1. Caricare codice di esempio
2. Serial Monitor (19200 baud)
3. Verificare letture temperatura e pressione
4. Soffiare sul sensore → temperatura aumenta

### Test SGP40

1. Caricare codice di esempio
2. Attendere 30 sec preriscaldamento
3. Verificare VOC index baseline (~100)
4. Avvicinare alcool/deodorante → VOC index aumenta

---

## Test Funzionale

### Test Iniziale

1. **Collegamento Seriale:**
   - USB → PC
   - Seriale: 19200 baud
   - Monitor: Arduino IDE o webapp [https://applications.techmakers.it/datalogger/terminal.htm](https://applications.techmakers.it/datalogger/terminal.htm)

2. **Verifica Inizializzazione:**
   - LED lampeggiano durante preriscaldamento (~30 sec)
   - Valori ADC si stabilizzano (10-150 per NASO)
   - Nessun errore SD card/RTC

3. **Configurazione Data/Ora:**

```text
settime 2025-01-24 14:30:00
```

4. **Calibrazione NASO:**

```text
autocalib
```

Attendere 30 secondi, poi:

```text
setcalib
```

### Test Sensori

**NASO (MQ-2):**

- Spray deodorante vicino sensore
- Valori ADC/PPM aumentano significativamente
- Display LCD (se presente) aggiorna valori in tempo reale

**METEO (BMP280):**

- Soffiare sul sensore → temperatura aumenta
- Valori pressione stabili (~1013 hPa al livello del mare)

**VOC (SGP40):**

- Attendere 30 sec preriscaldamento
- VOC index baseline ~100
- Avvicinare alcool/deodorante → VOC index aumenta

**Anemometro:**

- Anemometro fermo = valore minimo
- Ruotare manualmente = valori crescenti

### Verifica MicroSD

**Durante acquisizione:**

```text
test
```

Output atteso: `OK writing to SD`

**Test da spento:**

- Rimuovere microSD
- Inserire in lettore PC
- Verificare file CSV con data corrente
- Aprire con editor testo/Excel
- Colonne e timestamp corretti

### Verifica Display LCD (NASO+LCD / VOC+LCD)

- [ ] Display mostra versione firmware all'avvio
- [ ] Valori ADC/PPM/VOC si aggiornano in tempo reale
- [ ] Ora corretta
- [ ] Pulsante retroilluminazione funziona (pressione > 1 sec)
- [ ] Contrasto leggibile

**Se caratteri illeggibili:** ruotare trimmer blu sul modulo I2C

---

## Risoluzione Problemi

### BMP280 non rilevato

- Verificare collegamento 3.3V (NON 5V!)
- Controllare SDA/SCL su A4/A5
- Provare indirizzo alternativo: `bmp.begin(0x77)`
- Verificare saldature sui pin del sensore

### SGP40 non rilevato

- Verificare alimentazione 5V
- Controllare SDA/SCL su A4/A5
- Attendere preriscaldamento (30 sec)
- Verificare libreria installata correttamente

### Conflitto I2C multi-dispositivo

- Eseguire I2C Scanner per verificare indirizzi
- Verificare che dispositivi abbiano indirizzi diversi
- Controllare cavi SDA/SCL non troppo lunghi (max 30cm)
- Aggiungere resistenze pull-up 4.7kΩ su SDA/SCL se problemi

### Anemometro valori errati

- Verificare alimentazione 5V
- Controllare collegamento OUT → A1
- Calibrare: fermare anemometro = valore minimo
- Ruotare anemometro = valori crescenti

---

## Sviluppo Futuro

Le configurazioni METEO e VOC sono in fase di sviluppo. Contributi benvenuti:

**Roadmap:**
- [ ] Firmware completo METEO
- [ ] Firmware completo VOC
- [ ] Calibrazione automatica sensori
- [ ] Manuali utente dettagliati
- [ ] Procedure di test specifiche
- [ ] Esempi di utilizzo sul campo

**Contribuisci:**
- GitHub: [https://github.com/speleoalex/opsdatalogger](https://github.com/speleoalex/opsdatalogger)
- Telegram: [https://t.me/+u5CoELQNjC1iODZk](https://t.me/+u5CoELQNjC1iODZk)

---

## Documentazione Correlata

**Guide di Assemblaggio:**

- [Guida Assemblaggio NASO (IT)](NASO_assembly_complete_guide_it.md)
- [NASO Assembly Guide (EN)](NASO_assembly_complete_guide_en.md)
- [Guida Assemblaggio METEO/VOC (IT)](METEO_VOC_assembly_guide_it.md) - Questo documento
- [METEO/VOC Assembly Guide (EN)](METEO_VOC_assembly_guide_en.md)

**Altro:**

- [Lista Componenti](../components.md)
- [Datasheet BMP280](../datasheets/)
- [Datasheet SGP40](../datasheets/sgp40.pdf)
- [Platform Overview (IT)](../../FluxyLogger-Platform-Overview-it.md)
- [Platform Overview (EN)](../../FluxyLogger-Platform-Overview-en.md)

---

## Supporto

- **Telegram:** [https://t.me/+u5CoELQNjC1iODZk](https://t.me/+u5CoELQNjC1iODZk)
- **GitHub:** [https://github.com/speleoalex/opsdatalogger/issues](https://github.com/speleoalex/opsdatalogger/issues)
- **Email:** speleoalex@gmail.com

---

**Progetto FluxyLogger**
**Versione:** 1.0 - Gennaio 2025
**Licenza:** GNU General Public License
**Autore:** Alessandro Vernassa
**Stato:** METEO e VOC in sviluppo
