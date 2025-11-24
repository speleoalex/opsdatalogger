# Guida di Assemblaggio FluxyLogger NASO+LCD

Guida completa per l'assemblaggio della configurazione NASO+LCD del FluxyLogger con display I2C per visualizzazione in tempo reale.

## Componenti Necessari

### Componenti Base

1. **Arduino UNO** (o Arduino UNO WiFi R4)
2. **Data Logger Shield** con RTC DS1307
3. **Sensore MQ-2** (gas combustibili)
4. **Display LCD 16x2 I2C** (con modulo I2C integrato)
5. **Pulsante momentaneo** (per retroilluminazione LCD)
6. **Resistenza 10kΩ** (pull-down per pulsante)
7. **MicroSD Card** (formattata FAT32, minimo 1GB)
8. **Batteria CR1220** per RTC
9. **PowerBank USB** (consigliato: minimo 10000mAh)
10. **Cavo USB** (tipo A-B per Arduino UNO standard)
11. **Box contenitore** trasparente (per vedere il display)
12. **Cavetti Dupont** maschio-femmina per collegamenti

### Materiale Aggiuntivo

- Fascette fermacavi
- Spugna o gommapiuma per fissaggio interno
- Silicone per impermeabilizzazione
- Biadesivo per fissaggio display
- Cavetto per saldatura pulsante (opzionale)

## Schemi di Collegamento

### Schema Sensore MQ-2

![Schema MQ-2 NASO](../wiring/connections_MQ2_NASO.png)

### Schema Display LCD I2C

![Schema LCD I2C](../wiring/connections_lcd.png)

Oppure versione alternativa:

![Schema LCD I2C Alternativo](../wiring/connections_LCD_LCM1602_I2C_version.png)

## Tabelle Collegamenti

### Collegamento Sensore MQ-2

| Pin MQ-2 | Collegamento Data Logger Shield |
|----------|----------------------------------|
| VCC      | +5V                              |
| GND      | GND                              |
| AOUT     | A0 (Analog Input 0)              |
| DOUT     | Non collegato                    |

### Collegamento Display LCD I2C

| Pin LCD I2C | Collegamento Data Logger Shield |
|-------------|----------------------------------|
| VCC         | +5V                              |
| GND         | GND                              |
| SDA         | A4 (SDA)                         |
| SCL         | A5 (SCL)                         |

**Nota:** I pin SDA e SCL sono standard I2C su Arduino UNO e corrispondono ai pin analogici A4 e A5.

### Collegamento Pulsante Retroilluminazione

| Componente     | Collegamento                        |
|----------------|-------------------------------------|
| Pulsante Pin 1 | Digital Pin 8 (D8)                  |
| Pulsante Pin 2 | GND (tramite resistenza 10kΩ)       |
| Resistenza     | Tra Pin 2 del pulsante e GND        |

**Schema pulsante:**
```
D8 ----[Pulsante]---- GND
                 |
              [10kΩ]
                 |
                GND
```

## Procedura di Assemblaggio

### 1. Preparazione Data Logger Shield

1. Inserire la batteria CR1220 nel Data Logger Shield (lato componenti)
2. Verificare il selettore del RTC (se presente)
3. Inserire la microSD card formattata (FAT32) nello slot

### 2. Montaggio Shield su Arduino

1. Allineare i pin del Data Logger Shield con i connettori dell'Arduino UNO
2. Inserire delicatamente lo shield premendo uniformemente
3. Verificare che tutti i pin siano correttamente inseriti
4. Controllare che lo shield sia ben saldo

### 3. Collegamento Sensore MQ-2

**Attenzione:** Effettuare i collegamenti con Arduino spento.

1. Collegare i cavetti Dupont:
   - **VCC del sensore → +5V dello shield** (cavo rosso)
   - **GND del sensore → GND dello shield** (cavo nero)
   - **AOUT del sensore → A0 dello shield** (cavo giallo/verde)

2. Verificare la solidità dei collegamenti

### 4. Collegamento Display LCD I2C

1. Identificare i pin sul modulo I2C del display:
   - VCC (alimentazione)
   - GND (massa)
   - SDA (dati I2C)
   - SCL (clock I2C)

2. Collegare i cavetti:
   - **VCC display → +5V dello shield** (cavo rosso)
   - **GND display → GND dello shield** (cavo nero)
   - **SDA display → A4 dello shield** (cavo blu/verde)
   - **SCL display → A5 dello shield** (cavo giallo/bianco)

3. Verificare i collegamenti

**Nota sull'indirizzo I2C:** Il display LCD I2C ha tipicamente indirizzo 0x27 o 0x3F. Il firmware FluxyLogger rileva automaticamente l'indirizzo.

### 5. Collegamento Pulsante Retroilluminazione

#### Opzione A - Pulsante Volante (senza saldature)

1. Utilizzare un pulsante con terminali per cavetti Dupont
2. Collegare:
   - Un terminale del pulsante → D8 dello shield
   - Altro terminale → GND tramite resistenza 10kΩ

#### Opzione B - Pulsante Montato su Box

1. Praticare foro nel box (diametro adatto al pulsante)
2. Montare il pulsante nel foro
3. Saldare cavetti ai terminali del pulsante
4. Collegare come descritto nell'opzione A
5. Proteggere le saldature con termorestringente

**Funzionamento:**
- Pressione breve (< 1 sec): Non fa nulla
- Pressione lunga (> 1 sec): Accende/spegne retroilluminazione LCD

### 6. Verifica Collegamenti Completa

Prima di alimentare, verificare:

- [ ] Nessun cortocircuito tra VCC e GND
- [ ] Sensore MQ-2: AOUT collegato ad A0
- [ ] Display LCD: SDA su A4, SCL su A5
- [ ] Pulsante collegato a D8 con resistenza di pull-down
- [ ] MicroSD inserita nello shield
- [ ] Batteria RTC installata
- [ ] Shield ben inserito su Arduino
- [ ] Nessun pin piegato o mal connesso

### 7. Test Indirizzo I2C (Opzionale ma Consigliato)

Prima del montaggio finale, testare l'indirizzo I2C del display:

1. Caricare uno sketch di test I2C Scanner
2. Aprire Serial Monitor (19200 baud)
3. Annotare l'indirizzo rilevato (es. 0x27 o 0x3F)
4. Il firmware FluxyLogger rileva automaticamente l'indirizzo

### 8. Posizionamento nel Box

#### Posizionamento Display

1. **Scelta posizione display:**
   - Montare il display sulla parete interna trasparente del box
   - Posizionare in modo che sia leggibile dall'esterno
   - Utilizzare biadesivo o viti (con distanziali) per fissaggio

2. **Fissaggio display:**
   - Pulire superficie con alcool
   - Applicare biadesivo a doppia faccia sul retro del display
   - Premere fermamente per 10 secondi
   - Verificare che il display non si muova

3. **Orientamento:**
   - Il display deve essere leggibile con box chiuso
   - Lasciare spazio per regolazione contrasto (trimmer blu sul modulo I2C)

#### Posizionamento Arduino e Sensore

1. **Arduino+Shield:**
   - Posizionare sul fondo del box
   - Fissare con spugna/gommapiuma per evitare movimenti
   - Mantenere accessibile lo slot microSD

2. **Sensore MQ-2:**
   - Posizionare vicino a fori di ventilazione
   - Non ostruire la griglia del sensore
   - Mantenere 2-3 cm di distanza dall'Arduino
   - Non coprire il sensore con altri componenti

3. **Pulsante retroilluminazione:**
   - Se montato su box: verificare accessibilità dall'esterno
   - Se volante: posizionare vicino al display per comodità

#### Gestione Cavi

1. Utilizzare fascette per organizzare i cavi
2. Evitare tensioni sui collegamenti I2C (SDA/SCL sono sensibili)
3. Tenere separati cavi di alimentazione e cavi dati
4. Lasciare lunghezza sufficiente per apertura box

### 9. Fori di Ventilazione

**IMPORTANTE:** Come per NASO base, il sensore necessita di ricambio d'aria.

1. Praticare 4-6 fori (diametro 5-8 mm):
   - 2-3 fori parte inferiore (ingresso aria)
   - 2-3 fori parte superiore (uscita aria)
   - Posizionare fori lontano dal display per evitare condensa

2. Applicare retina anti-insetto (opzionale)

### 10. Impermeabilizzazione

1. Passaggio cavo USB: sigillare con silicone
2. Attorno al pulsante (se montato): applicare guarnizione o silicone
3. Verificare chiusura ermetica del box

## Caricamento Firmware

### 1. Preparazione Arduino IDE

1. Installare Arduino IDE da [arduino.cc](https://www.arduino.cc/en/software)
2. Collegare Arduino al PC tramite USB
3. Selezionare: **Tools → Board → Arduino UNO**
4. Selezionare porta: **Tools → Port → COMx / /dev/ttyACMx**

### 2. Installazione Librerie

Installare tramite **Tools → Manage Libraries**:

- **RTClib** (by Adafruit) - per RTC DS1307
- **SD** (built-in) - per microSD
- **LiquidCrystal_I2C** (by Frank de Brabander) - per display LCD I2C

### 3. Caricamento Firmware

1. Scaricare firmware FluxyLogger:
   - [https://github.com/speleoalex/opsdatalogger/blob/main/FluxyLogger/FluxyLogger.ino](https://github.com/speleoalex/opsdatalogger/blob/main/FluxyLogger/FluxyLogger.ino)

2. Aprire file `.ino` con Arduino IDE

3. **IMPORTANTE:** Verificare nel codice che la modalità LCD sia abilitata
   - Cercare: `#define LCD_ENABLED`
   - Deve essere presente e non commentato

4. Verificare: **Sketch → Verify/Compile**

5. Caricare: **Sketch → Upload**

6. Attendere: "Done uploading"

### 4. Prima Configurazione

1. Aprire Serial Monitor: **Tools → Serial Monitor**
2. Baud rate: **19200**
3. Il display dovrebbe mostrare:
   - Prima riga: Versione firmware
   - Seconda riga: "Preheating..."

4. Seguire procedura configurazione: [NASO_testing_procedure_it.md](NASO_testing_procedure_it.md)

### 5. Regolazione Contrasto Display

Se il display non è leggibile:

1. Individuare il trimmer blu sul modulo I2C (piccola vite di regolazione)
2. Ruotare delicatamente con cacciavite piccolo
3. Regolare fino a ottenere contrasto ottimale
4. Il display deve mostrare caratteri nitidi

## Utilizzo Display

### Informazioni Visualizzate

**Durante preriscaldamento:**
```
FluxyLogger v2.XX
Preheating...
```

**Durante acquisizione normale:**
```
ADC:0125 PPM:045
16:32 Det:0003
```

Dove:
- **ADC:** Valore grezzo sensore (0-1023)
- **PPM:** Parti per milione calcolate
- **HH:MM:** Ora corrente
- **Det:** Numero rilevamenti positivi consecutivi

**In caso di errore SD:**
```
ADC:0125 PPM:045
SD Write FAILED!
```

### Pulsante Retroilluminazione

- **Pressione > 1 secondo:** Accende/spegne retroilluminazione
- **Pressione < 1 secondo:** Nessuna azione (per evitare attivazioni accidentali)
- Retroilluminazione si spegne automaticamente dopo 30 secondi di inattività (configurabile nel firmware)

## Test Funzionale

Eseguire test completo seguendo:

**[→ Procedura di Test NASO](NASO_testing_procedure_it.md)**

Verifiche aggiuntive per versione LCD:

- [ ] Display visualizza versione firmware all'avvio
- [ ] Valori ADC e PPM si aggiornano correttamente
- [ ] Ora visualizzata è corretta
- [ ] Pulsante retroilluminazione funziona
- [ ] Contrasto display è ottimale
- [ ] Non ci sono caratteri strani o missing

## Risoluzione Problemi

### Display non si accende

**Soluzione:**
- Verificare alimentazione VCC e GND
- Controllare contrasto (trimmer blu)
- Verificare indirizzo I2C (provare 0x27 o 0x3F)
- Testare con I2C Scanner

### Display mostra caratteri strani

**Soluzione:**
- Regolare contrasto con trimmer
- Verificare libreria LiquidCrystal_I2C installata
- Controllare indirizzo I2C nel codice

### Pulsante non funziona

**Soluzione:**
- Verificare collegamento D8
- Controllare resistenza pull-down 10kΩ
- Tenere premuto per > 1 secondo
- Verificare nel codice che il pin button sia definito correttamente

### Display si blocca durante funzionamento

**Soluzione:**
- Verificare alimentazione stabile (PowerBank adeguato)
- Controllare che cavi I2C non siano troppo lunghi
- Verificare assenza interferenze elettriche
- Accorciare cavi SDA/SCL se troppo lunghi (max 30cm consigliato)

### Valori sul display non corrispondono al Serial Monitor

**Soluzione:**
- Verificare che entrambi usino lo stesso codice firmware
- Controllare che il baud rate sia 19200
- Problema di visualizzazione: aggiornare libreria LCD

## Manutenzione

### Prima di ogni utilizzo

- Verificare leggibilità display
- Controllare carica PowerBank
- Pulire display con panno morbido
- Verificare funzionamento pulsante
- Controllare ventilazione sensore

### Periodica

- Regolare contrasto se necessario
- Pulire sensore MQ-2 con aria compressa
- Verificare tenuta impermeabilizzazione
- Controllare integrità collegamenti I2C

### Annuale

- Sostituire batteria RTC
- Verificare ossidazione connettori
- Controllare usura pulsante
- Aggiornare firmware se disponibile

## Specifiche Tecniche

- **Alimentazione:** 5V via USB
- **Consumo totale:** ~275 mA (con LCD retroilluminato)
- **Consumo LCD:** ~20-30 mA (senza retroilluminazione)
- **Consumo LCD:** ~60-80 mA (con retroilluminazione)
- **Autonomia:** ~36-48 ore con PowerBank 10000mAh (uso normale)
- **Display:** LCD 16x2 caratteri, retroilluminato
- **Protocollo comunicazione:** I2C (indirizzo 0x27 o 0x3F)
- **Temperatura operativa:** 0°C ~ 50°C
- **Tempo preriscaldamento:** 30-60 secondi

## Vantaggi Configurazione LCD

✅ **Visualizzazione real-time** senza PC
✅ **Monitoraggio sul campo** durante tracciamenti
✅ **Verifica immediata** dei rilevamenti
✅ **Controllo stato** acquisizione
✅ **Feedback visivo** per calibrazione

## Documentazione Correlata

- [Manuale Utente NASO+LCD (IT)](../../manuals/FluxyLogger-NASO_lcd-it.md)
- [Manuale Utente NASO+LCD (EN)](../../manuals/FluxyLogger-NASO_lcd-en.md)
- [Guida Assemblaggio NASO Base](NASO_assembly_guide_it.md)
- [Procedura Test NASO](NASO_testing_procedure_it.md)
- [Aggiornamento Firmware](../../manuals/UpdateFirmware_it.md)
- [Lista Componenti](../components.md)

## Sicurezza

⚠️ **ATTENZIONI:**

- Non alimentare durante il montaggio
- Verificare polarità collegamenti
- Display I2C sensibile a scariche elettrostatiche
- Evitare cortocircuiti sui pin I2C
- Il sensore MQ-2 si scalda (normale)
- Non utilizzare in ambienti esplosivi durante test
- Pulsante: verificare isolamento da componenti elettrici

## Supporto

Per problemi o domande:

- **Telegram Group:** [https://t.me/+u5CoELQNjC1iODZk](https://t.me/+u5CoELQNjC1iODZk)
- **GitHub Issues:** [https://github.com/speleoalex/opsdatalogger/issues](https://github.com/speleoalex/opsdatalogger/issues)
- **Email:** speleoalex@gmail.com

---

**Guida creata per il progetto FluxyLogger**
**Versione:** 1.0 - Gennaio 2025
**Licenza:** GNU General Public License
