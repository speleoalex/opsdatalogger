# LoggerViewer Desktop - Guida Rapida

## Avvio Rapido

### Linux

```bash
./run.sh
```

Oppure:
```bash
npm start
```

### Permessi Porta Seriale (solo Linux)

Se non riesci a connetterti alle porte seriali:

```bash
sudo usermod -a -G dialout $USER
```

Poi **esci e rientra** per applicare le modifiche.

## Interfaccia a Tab

L'applicazione usa un **sistema a tab** in una singola finestra:

- **📡 Logger Manager**: Gestione dispositivo e download log
- **📊 Data Viewer**: Visualizzazione e analisi dati

### Navigazione
- **Click sui tab** in alto per cambiare vista
- **Ctrl+1**: Vai al Manager
- **Ctrl+2**: Vai al Viewer
- **Pulsanti**: "Go to Viewer" / "Logger Manager" nelle interfacce

## Primo Utilizzo

1. **Avvia l'applicazione** (tab Manager)
   - Clicca su "Connect to Serial/USB"
   - Seleziona la porta del tuo dispositivo FluxyLogger

2. **Scarica i Log** (tab Manager)
   - Clicca su "Read Logs"
   - Vedrai l'elenco dei file disponibili
   - Clicca sull'icona 📥 per scaricare
   - Clicca sull'icona 📈 per passare al Viewer automaticamente

3. **Visualizza i Dati** (tab Viewer)
   - Passa al tab "Data Viewer" (o clicca 📈 su un log)
   - Clicca "Open CSV File" se necessario
   - Configura la calibrazione se necessario
   - Usa "Export CSV" per salvare i dati elaborati

## Funzioni Principali

### Terminale
- Invia comandi diretti al dispositivo
- Visualizza le risposte in tempo reale
- Supporto per comandi FluxyLogger

### Comandi Disponibili
- `logs` - Elenca i file disponibili
- `settime` - Sincronizza ora e data
- `setconfig` - Configura intervallo di logging
- `autocalib` - Calibrazione zerogas
- `plotter start` - Avvia modalità real-time
- `plotter stop` - Ferma modalità real-time

### Visualizzatore Dati
- Apertura file CSV
- Calibrazione sensori MQ-2
- Compensazione riscaldamento
- Esportazione dati elaborati
- Grafici interattivi

## Build dell'Applicazione

### Crea Pacchetti Installabili

**AppImage (Linux):**
```bash
npm run build:linux
```

File generato: `dist/LoggerViewer-1.0.0.AppImage`

**Debian Package:**
```bash
npm run build:linux
```

File generato: `dist/loggerviewer-desktop_1.0.0_amd64.deb`

### Installazione del .deb

```bash
sudo dpkg -i dist/loggerviewer-desktop_1.0.0_amd64.deb
```

### Rimozione (se installato via .deb)

```bash
sudo apt remove loggerviewer-desktop
```

## Risoluzione Problemi

### L'app non si avvia

Verifica le dipendenze:
```bash
rm -rf node_modules package-lock.json
npm install
```

### Porta seriale non disponibile

1. Verifica che il dispositivo sia connesso:
   ```bash
   ls /dev/ttyUSB* /dev/ttyACM*
   ```

2. Controlla i permessi:
   ```bash
   groups
   ```
   Dovresti vedere "dialout" nella lista.

3. Se non c'è, aggiungi il gruppo:
   ```bash
   sudo usermod -a -G dialout $USER
   ```
   Poi **logout/login**.

### Errori di build

Pulisci e ricompila:
```bash
npm run build -- --clean
```

## Sviluppo

### Modalità Sviluppo (con DevTools)

```bash
npm run dev
```

### Struttura Progetto

```
LoggerViewer_Desktop/
├── main.js              # Processo principale Electron
├── preload.js           # Bridge IPC
├── package.json         # Configurazione progetto
└── src/
    ├── assets/          # Icone e risorse
    └── renderer/        # File interfaccia utente
        ├── loggermanager.html
        ├── loggerviewer.html
        ├── loggerviewer.js
        ├── electron-adapter.js
        └── loader.js
```

## Supporto

Per problemi o domande:
- GitHub: https://github.com/speleoalex/opsdatalogger
- Donazioni: https://www.paypal.com/donate/?business=TKQWLKGENEP7L

## Note Importanti

- **Chart.js**: Richiede connessione internet al primo avvio
- **SerialPort**: Comunicazione nativa Node.js, più affidabile delle Web APIs
- **Multi-finestra**: Il viewer può aprirsi in finestra separata
- **Compatibilità**: Protocollo seriale compatibile con firmware FluxyLogger

## Crediti

LoggerViewer Desktop è parte del progetto **FluxyLogger NASO**

Licenza: MIT
