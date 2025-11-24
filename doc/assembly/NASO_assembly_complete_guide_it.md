# Guida Completa di Assemblaggio FluxyLogger NASO

Guida per l'assemblaggio delle configurazioni NASO e NASO+LCD del FluxyLogger per tracciamenti aerei in grotta.

---

## Indice

- [Configurazione NASO Base](#configurazione-naso-base)
- [Configurazione NASO+LCD](#configurazione-nasolcd)
- [Caricamento Firmware](#caricamento-firmware)
- [Test Funzionale](#test-funzionale)
- [Risoluzione Problemi](#risoluzione-problemi)

---

## Configurazione NASO Base

### Componenti Necessari

1. Arduino UNO (o Arduino UNO WiFi R4)
2. Data Logger Shield con RTC DS1307
3. Sensore MQ-2 (gas combustibili)
4. MicroSD Card (formattata FAT32)
5. Batteria CR1220 per RTC
6. PowerBank USB (minimo 10000mAh)
7. Cavo USB tipo A-B
8. Box contenitore
9. Cavetti Dupont maschio-femmina

### Schema Collegamenti NASO Base

![Schema MQ-2 NASO](../wiring/connections_MQ2_NASO.png)

**Tabella Collegamenti Sensore MQ-2:**

| Pin MQ-2 | Collegamento Shield |
|----------|---------------------|
| VCC      | +5V                 |
| GND      | GND                 |
| AOUT     | A0                  |
| DOUT     | Non collegato       |

### Assemblaggio NASO Base

#### 1. Preparazione Shield

- Inserire batteria CR1220 nel Data Logger Shield
- Inserire microSD formattata (FAT32) nello slot

#### 2. Montaggio Shield su Arduino

- Allineare e inserire il Data Logger Shield su Arduino UNO
- Verificare che tutti i pin siano correttamente inseriti

#### 3. Collegamento Sensore MQ-2

**Con Arduino spento:**

- **VCC sensore → +5V shield** (cavo rosso)
- **GND sensore → GND shield** (cavo nero)
- **AOUT sensore → A0 shield** (cavo giallo/verde)

#### 4. Verifica Collegamenti

Prima di alimentare verificare:

- [ ] Nessun cortocircuito tra VCC e GND
- [ ] AOUT collegato ad A0
- [ ] MicroSD inserita
- [ ] Batteria RTC installata

#### 5. Posizionamento nel Box

1. **Arduino+Shield:** fissare nel box con spugna/gommapiuma
2. **Sensore MQ-2:** deve rimanere esposto all'aria esterna, farlo uscire dal box
3. **Cavo USB:** praticare foro e sigillare con silicone

**Consumo:** ~210 mA
**Autonomia:** ~48-72 ore con PowerBank 10000mAh

---

## Configurazione NASO+LCD

### Componenti Aggiuntivi

Oltre ai componenti base NASO:

1. Display LCD 16x2 I2C (con modulo I2C integrato)
2. Pulsante momentaneo (opzionale, per controllo retroilluminazione)
3. Resistenza 10kΩ (opzionale, pull-down per pulsante)
4. Box contenitore trasparente

### Schemi Collegamenti NASO+LCD

#### Schema Sensore MQ-2

![Schema MQ-2 NASO](../wiring/connections_MQ2_NASO.png)

#### Schema Display LCD I2C

![Schema LCD I2C](../wiring/connections_lcd.png)

*Versione alternativa:*

![Schema LCD I2C Alternativo](../wiring/connections_LCD_LCM1602_I2C_version.png)

### Tabelle Collegamenti NASO+LCD

**Sensore MQ-2:**

| Pin MQ-2 | Collegamento Shield |
|----------|---------------------|
| VCC      | +5V                 |
| GND      | GND                 |
| AOUT     | A0                  |

**Display LCD I2C:**

| Pin LCD I2C | Collegamento Shield |
|-------------|---------------------|
| VCC         | +5V                 |
| GND         | GND                 |
| SDA         | A4 (SDA)            |
| SCL         | A5 (SCL)            |

**Pulsante Retroilluminazione (opzionale):**

| Componente  | Collegamento               |
|-------------|----------------------------|
| Pin 1       | Digital Pin 8 (D8)         |
| Pin 2       | GND (tramite resistenza)   |
| Resistenza  | 10kΩ tra Pin 2 e GND       |

**Schema pulsante:**

```
D8 ----[Pulsante]---- GND
                 |
              [10kΩ]
                 |
                GND
```

**Nota:** Senza pulsante, la retroilluminazione rimane sempre accesa.

### Assemblaggio NASO+LCD

#### 1-2. Preparazione e Montaggio Shield

Come per NASO base (punti 1 e 2)

#### 3. Collegamento Sensore MQ-2

Come per NASO base (punto 3)

#### 4. Collegamento Display LCD I2C

**Con Arduino spento:**

- **VCC display → +5V shield** (cavo rosso)
- **GND display → GND shield** (cavo nero)
- **SDA display → A4 shield** (cavo blu/verde)
- **SCL display → A5 shield** (cavo giallo/bianco)

**Nota:** Indirizzo I2C tipicamente 0x27 o 0x3F (rilevato automaticamente dal firmware)

#### 5. Collegamento Pulsante (opzionale)

Se si desidera controllo manuale della retroilluminazione:

- Un terminale → D8
- Altro terminale → GND tramite resistenza 10kΩ

**Funzionamento:** Pressione > 1 sec accende/spegne retroilluminazione

**Senza pulsante:** La retroilluminazione rimane sempre accesa

#### 6. Verifica Collegamenti Completa

- [ ] Nessun cortocircuito VCC/GND
- [ ] Sensore: AOUT su A0
- [ ] LCD: SDA su A4, SCL su A5
- [ ] Pulsante su D8 con resistenza (se installato)
- [ ] MicroSD e batteria RTC installati

#### 7. Posizionamento nel Box

1. **Display:** montare sulla parete trasparente del box (biadesivo o viti)
2. **Arduino+Shield:** fissare sul fondo
3. **Sensore MQ-2:** deve rimanere esposto all'aria esterna, farlo uscire dal box
4. **Pulsante (se installato):** verificare accessibilità dall'esterno
5. **Cavi:** evitare tensioni sui collegamenti I2C

#### 8. Regolazione Contrasto Display

Se il display non è leggibile:

- Ruotare il trimmer blu sul modulo I2C con cacciavite piccolo
- Regolare fino a ottenere contrasto ottimale

### Utilizzo Display

**Durante acquisizione:**

```
ADC:0125 PPM:045
16:32 Det:0003
```

- **ADC:** Valore grezzo sensore (0-1023)
- **PPM:** Parti per milione
- **HH:MM:** Ora corrente
- **Det:** Rilevamenti positivi consecutivi

**Consumo:** ~275 mA (con LCD retroilluminato)
**Autonomia:** ~36-48 ore con PowerBank 10000mAh

---

## Caricamento Firmware

### Preparazione Arduino IDE

1. Installare Arduino IDE da [arduino.cc](https://www.arduino.cc/en/software)
2. Collegare Arduino al PC via USB
3. Selezionare: **Tools → Board → Arduino UNO**
4. Selezionare porta: **Tools → Port → COMx** (Windows) o **/dev/ttyACMx** (Linux)

### Librerie Necessarie

Installare via **Tools → Manage Libraries:**

**Per NASO base:**
- RTClib (by Adafruit)
- SD (built-in)

**Per NASO+LCD aggiungere:**
- LiquidCrystal_I2C (by Frank de Brabander)

### Caricamento Sketch

1. Scaricare firmware: [FluxyLogger.ino](https://github.com/speleoalex/opsdatalogger/blob/main/FluxyLogger/FluxyLogger.ino)
2. Aprire file `.ino` con Arduino IDE
3. **Per NASO+LCD:** verificare che nel codice sia abilitato `#define LCD_ENABLED`
4. Cliccare: **Sketch → Verify/Compile**
5. Cliccare: **Sketch → Upload**
6. Attendere: "Done uploading"

### Prima Configurazione

1. Aprire: **Tools → Serial Monitor**
2. Baud rate: **19200**
3. Seguire istruzioni per configurazione iniziale

Per dettagli: [Procedura Test NASO](NASO_testing_procedure_it.md)

---

## Test Funzionale

Eseguire test completo seguendo la procedura:

**[→ Procedura di Test NASO (IT)](NASO_testing_procedure_it.md)**

**[→ NASO Testing Procedure (EN)](NASO_testing_procedure_en.md)**

### Verifiche Principali

1. **Inizializzazione:** LED lampeggiano durante preriscaldamento (~30 sec)
2. **Sensore:** valori ADC si stabilizzano (10-150)
3. **Configurazione:** impostare data/ora con comando `settime`
4. **Calibrazione:** eseguire `autocalib`
5. **Test sensore:** spray deodorante → valori ADC/PPM aumentano
6. **Scrittura SD:** verificare salvataggio dati, no errori "failed"

### Verifiche Aggiuntive NASO+LCD

- [ ] Display mostra versione firmware all'avvio
- [ ] Valori ADC/PPM si aggiornano
- [ ] Ora visualizzata corretta
- [ ] Pulsante retroilluminazione funziona
- [ ] Contrasto ottimale

---

## Risoluzione Problemi

### Problemi Comuni (NASO Base e LCD)

**Shield non si inserisce:**
- Verificare pin non piegati
- Non forzare inserimento

**Sensore non rileva variazioni:**
- Verificare AOUT → A0
- Controllare VCC e GND
- Attendere preriscaldamento (30-60 sec)
- Verificare che sensore sia esposto all'aria

**MicroSD non rilevata:**
- Verificare formattazione FAT32
- Provare microSD diversa (max 32GB)
- Pulire contatti

**RTC non mantiene orario:**
- Sostituire batteria CR1220
- Verificare polarità (+ verso l'alto)

### Problemi Specifici LCD

**Display non si accende:**
- Verificare VCC e GND
- Regolare contrasto (trimmer blu)
- Testare indirizzo I2C (0x27 o 0x3F)

**Caratteri strani:**
- Regolare contrasto
- Verificare libreria LiquidCrystal_I2C installata

**Pulsante non funziona (se installato):**
- Verificare collegamento D8
- Controllare resistenza 10kΩ
- Tenere premuto > 1 secondo
- Nota: senza pulsante, retroilluminazione sempre accesa

**Display si blocca:**
- Verificare alimentazione stabile
- Cavi I2C non troppo lunghi (max 30cm)
- Evitare interferenze

---

## Manutenzione

### Prima di ogni utilizzo

- Verificare carica PowerBank
- Controllare collegamenti
- Pulire sensore con aria compressa (delicatamente)
- **LCD:** verificare leggibilità display

### Annuale

- Sostituire batteria RTC
- Verificare impermeabilizzazione
- Controllare usura cavetti
- Aggiornare firmware se disponibile

---

## Specifiche Tecniche

### NASO Base

- **Alimentazione:** 5V USB
- **Consumo:** ~210 mA
- **Autonomia:** 48-72h (PowerBank 10000mAh)
- **Intervallo acquisizione:** Configurabile (default 15s)
- **Temperatura operativa:** 0°C ~ 50°C

### NASO+LCD

- **Consumo totale:** ~275 mA (LCD retroilluminato)
- **Consumo LCD:** 20-30 mA (senza retro), 60-80 mA (con retro)
- **Autonomia:** 36-48h (PowerBank 10000mAh)
- **Display:** LCD 16x2, retroilluminato
- **Protocollo:** I2C (0x27 o 0x3F)

---

## Vantaggi Configurazione LCD

✅ Visualizzazione real-time senza PC
✅ Monitoraggio sul campo durante tracciamenti
✅ Verifica immediata dei rilevamenti
✅ Controllo stato acquisizione
✅ Feedback visivo per calibrazione

---

## Documentazione Correlata

- [Manuale Utente NASO (IT)](../../manuals/FluxyLogger-NASO-it.md)
- [Manuale Utente NASO (EN)](../../manuals/FluxyLogger-NASO-en.md)
- [Manuale Utente NASO+LCD (IT)](../../manuals/FluxyLogger-NASO_lcd-it.md)
- [Manuale Utente NASO+LCD (EN)](../../manuals/FluxyLogger-NASO_lcd-en.md)
- [Aggiornamento Firmware](../../manuals/UpdateFirmware_it.md)
- [Lista Componenti](../components.md)

---

## Sicurezza

⚠️ **ATTENZIONI:**

- Non alimentare durante il montaggio
- Verificare polarità collegamenti
- Il sensore MQ-2 si scalda durante funzionamento (normale)
- Non utilizzare in ambienti esplosivi durante test
- **LCD:** sensibile a scariche elettrostatiche
- Utilizzare PowerBank con protezioni

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
