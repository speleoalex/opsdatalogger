# Guida di Assemblaggio FluxyLogger NASO

Guida completa per l'assemblaggio della configurazione NASO del FluxyLogger per tracciamenti aerei in grotta.

## Componenti Necessari

### Componenti Base

1. **Arduino UNO** (o Arduino UNO WiFi R4)
2. **Data Logger Shield** con RTC DS1307
3. **Sensore MQ-2** (gas combustibili)
4. **MicroSD Card** (formattata FAT32, minimo 1GB)
5. **Batteria CR1220** per RTC
6. **PowerBank USB** (consigliato: minimo 10000mAh)
7. **Cavo USB** (tipo A-B per Arduino UNO standard)
8. **Box contenitore** impermeabile
9. **Cavetti Dupont** maschio-femmina per collegamenti

### Materiale Opzionale

- Fascette fermacavi
- Spugna o gommapiuma per fissaggio interno
- Silicone per impermeabilizzazione passacavi
- Etichette per identificazione

## Schema di Collegamento

![Schema MQ-2 NASO](../wiring/connections_MQ2_NASO.png)

### Tabella Collegamenti Sensore MQ-2

| Pin MQ-2 | Collegamento Data Logger Shield |
|----------|----------------------------------|
| VCC      | +5V                              |
| GND      | GND                              |
| AOUT     | A0 (Analog Input 0)              |
| DOUT     | Non collegato                    |

**IMPORTANTE:** Il sensore MQ-2 dispone di un trimmer per la regolazione della sensibilità. Per l'uso con NASO, questo trimmer non viene utilizzato in quanto la calibrazione avviene via software.

## Procedura di Assemblaggio

### 1. Preparazione Data Logger Shield

1. Inserire la batteria CR1220 nel Data Logger Shield (lato componenti)
2. Verificare che il selettore del RTC sia in posizione corretta (se presente)
3. Inserire la microSD card formattata (FAT32) nello slot

**Nota:** La microSD deve essere formattata prima del primo utilizzo. Utilizzare il formato FAT32.

### 2. Montaggio Shield su Arduino

1. Allineare i pin del Data Logger Shield con i connettori dell'Arduino UNO
2. Inserire delicatamente lo shield premendo uniformemente
3. Verificare che tutti i pin siano correttamente inseriti
4. Controllare che lo shield sia ben saldo (non deve muoversi)

### 3. Collegamento Sensore MQ-2

**Attenzione:** Effettuare i collegamenti con Arduino spento/scollegato dall'alimentazione.

1. Identificare i pin sul sensore MQ-2:
   - VCC (alimentazione positiva)
   - GND (massa)
   - AOUT (uscita analogica)
   - DOUT (uscita digitale - non usata)

2. Collegare i cavetti Dupont:
   - VCC del sensore → +5V dello shield
   - GND del sensore → GND dello shield
   - AOUT del sensore → A0 dello shield

3. Verificare la solidità dei collegamenti

**Suggerimento:** Utilizzare cavetti di colore diverso per facilitare l'identificazione:
- Rosso per VCC
- Nero per GND
- Giallo/Verde per AOUT

### 4. Verifica Collegamenti

Prima di alimentare, verificare:

- [ ] Nessun cortocircuito tra VCC e GND
- [ ] Pin AOUT collegato ad A0
- [ ] MicroSD inserita nello shield
- [ ] Batteria RTC installata
- [ ] Shield ben inserito su Arduino

### 5. Posizionamento nel Box

1. **Posizionare Arduino+Shield** nel box
   - Fissare con spugna/gommapiuma per evitare movimenti

2. **Posizionare il sensore MQ-2**
   - Il sensore deve rimanere esposto all'aria esterna
   - Farlo uscire dal box attraverso un'apertura
   - Non ostruire la griglia del sensore

3. **Passaggio cavo USB**
   - Praticare foro nel box per il cavo USB
   - Sigillare con silicone per impermeabilizzazione

### 6. Ventilazione Sensore

**IMPORTANTE:** Il sensore MQ-2 deve rimanere esposto all'aria esterna.

- Posizionare il sensore in modo che la griglia sia esposta fuori dal box
- Se necessario, praticare un foro per far uscire il sensore mantenendolo a contatto con l'aria ambiente

## Caricamento Firmware

### 1. Preparazione Arduino IDE

1. Scaricare e installare Arduino IDE da [arduino.cc](https://www.arduino.cc/en/software)
2. Collegare Arduino al PC tramite USB
3. Selezionare la scheda corretta: **Tools → Board → Arduino UNO**
4. Selezionare la porta COM corretta: **Tools → Port → COMx** (Windows) o **/dev/ttyACMx** (Linux)

### 2. Installazione Librerie

Installare le seguenti librerie tramite **Tools → Manage Libraries**:

- **RTClib** (by Adafruit) - per RTC DS1307
- **SD** (built-in) - per gestione microSD

### 3. Caricamento Sketch

1. Scaricare il firmware da:
   - [https://github.com/speleoalex/opsdatalogger/blob/main/FluxyLogger/FluxyLogger.ino](https://github.com/speleoalex/opsdatalogger/blob/main/FluxyLogger/FluxyLogger.ino)

2. Aprire il file `.ino` con Arduino IDE

3. Verificare il codice: **Sketch → Verify/Compile**

4. Caricare su Arduino: **Sketch → Upload**

5. Attendere il messaggio: "Done uploading"

### 4. Prima Configurazione

1. Aprire il Serial Monitor: **Tools → Serial Monitor**
2. Impostare baud rate: **19200**
3. Seguire le istruzioni per configurazione iniziale

Per dettagli sulla configurazione, vedere: [NASO_testing_procedure_it.md](NASO_testing_procedure_it.md)

## Test Funzionale

Dopo l'assemblaggio, eseguire il test funzionale seguendo la procedura completa:

**[→ Procedura di Test NASO](NASO_testing_procedure_it.md)**

## Risoluzione Problemi di Assemblaggio

### Problema: Shield non si inserisce su Arduino

**Soluzione:**
- Verificare che i pin non siano piegati
- Allineare correttamente lo shield prima di premere
- Non forzare l'inserimento

### Problema: Sensore non rileva variazioni

**Soluzione:**
- Verificare collegamento AOUT → A0
- Controllare alimentazione VCC e GND
- Attendere tempo di preriscaldamento (30-60 secondi)
- Verificare ventilazione del sensore

### Problema: MicroSD non rilevata

**Soluzione:**
- Verificare formattazione FAT32
- Provare con microSD diversa (max 32GB)
- Verificare inserimento corretto nello slot
- Pulire contatti microSD con panno morbido

### Problema: RTC non mantiene orario

**Soluzione:**
- Sostituire batteria CR1220
- Verificare polarità batteria (+ verso l'alto)
- Verificare saldature contatti batteria sullo shield

## Manutenzione

### Periodica (prima di ogni utilizzo)

- Verificare carica PowerBank
- Controllare integrità collegamenti
- Pulire sensore MQ-2 con aria compressa (delicatamente)
- Verificare spazio disponibile su microSD

### Annuale

- Sostituire batteria RTC
- Verificare impermeabilizzazione box
- Controllare usura cavetti Dupont
- Aggiornare firmware se disponibili nuove versioni

## Specifiche Tecniche

- **Alimentazione:** 5V via USB (da PowerBank)
- **Consumo:** ~210 mA (senza LCD)
- **Autonomia:** ~48-72 ore con PowerBank 10000mAh
- **Intervallo acquisizione:** Configurabile (default 15s)
- **Temperatura operativa:** 0°C ~ 50°C
- **Tempo preriscaldamento sensore:** 30-60 secondi

## Documentazione Correlata

- [Manuale Utente NASO (IT)](../../manuals/FluxyLogger-NASO-it.md)
- [Manuale Utente NASO (EN)](../../manuals/FluxyLogger-NASO-en.md)
- [Procedura Test NASO (IT)](NASO_testing_procedure_it.md)
- [Procedura Test NASO (EN)](NASO_testing_procedure_en.md)
- [Aggiornamento Firmware](../../manuals/UpdateFirmware_it.md)
- [Lista Componenti](../components.md)

## Sicurezza

⚠️ **ATTENZIONI:**

- Non alimentare il circuito durante il montaggio
- Verificare polarità prima di collegare l'alimentazione
- Non utilizzare in ambienti con presenza di gas esplosivi durante i test
- Il sensore MQ-2 si scalda durante il funzionamento (normale)
- Non toccare il sensore durante il funzionamento
- Utilizzare solo PowerBank con protezioni da cortocircuito

## Supporto

Per problemi o domande:

- **Telegram Group:** [https://t.me/+u5CoELQNjC1iODZk](https://t.me/+u5CoELQNjC1iODZk)
- **GitHub Issues:** [https://github.com/speleoalex/opsdatalogger/issues](https://github.com/speleoalex/opsdatalogger/issues)
- **Email:** speleoalex@gmail.com

---

**Guida creata per il progetto FluxyLogger**
**Versione:** 1.0 - Gennaio 2025
**Licenza:** GNU General Public License
