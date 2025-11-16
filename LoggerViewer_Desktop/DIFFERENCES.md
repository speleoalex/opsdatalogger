# Differenze tra Web e Desktop

## LoggerViewer Web vs Desktop

### Architettura

**Web Version (LoggerViewer/):**
- Eseguita in browser (Chrome/Chromium)
- Usa Web Serial API, WebUSB, Web Bluetooth
- Richiede flag speciali del browser
- PWA (Progressive Web App)
- ServiceWorker per offline

**Desktop Version (LoggerViewer_Desktop/):**
- Applicazione nativa Electron
- Usa Node.js SerialPort
- Nessun flag necessario
- Finestre native del sistema operativo
- File system nativo

### Comunicazione Seriale

**Web:**
```javascript
const port = await navigator.serial.requestPort();
await port.open({ baudRate: 19200 });
```

**Desktop:**
```javascript
const { SerialPort } = require('serialport');
const port = new SerialPort({
    path: '/dev/ttyUSB0',
    baudRate: 19200
});
```

### File System

**Web:**
- File picker del browser
- Download via blob URLs
- SessionStorage per dati temporanei

**Desktop:**
- Dialog nativi Electron
- Accesso diretto file system
- localStorage e file system Node.js

### Vantaggi Web

✅ Nessuna installazione richiesta
✅ Aggiornamenti automatici
✅ Multipiattaforma garantito
✅ Nessuna configurazione permessi

### Vantaggi Desktop

✅ Comunicazione seriale più affidabile
✅ Accesso completo al file system
✅ Nessuna dipendenza dal browser
✅ Prestazioni migliori
✅ Può funzionare completamente offline
✅ Integrazione sistema operativo
✅ Multi-finestra nativa

### Funzionalità Comuni

Entrambe le versioni supportano:
- Comunicazione con FluxyLogger
- Download e gestione log
- Visualizzazione dati con Chart.js
- Calibrazione sensori MQ-2
- Compensazione riscaldamento
- Esportazione CSV
- Real-time plotter
- Internazionalizzazione (IT/EN)

### Compatibilità

**Web - Browser Supportati:**
- Chrome 89+ (Linux, Windows, macOS)
- Chromium 89+
- Edge 89+ (solo Windows)
- Opera 75+

**Desktop - Sistema Operativo:**
- Linux (AppImage, .deb)
- Windows 10/11 (installer, portable)
- macOS 10.13+ (.dmg, .zip)

### Quando Usare Web

- Uso occasionale
- Non vuoi installare software
- Hai bisogno di Bluetooth (BLE)
- Preferisci aggiornamenti automatici
- Vuoi testare rapidamente

### Quando Usare Desktop

- Uso frequente/professionale
- Problemi con Web Serial API
- Vuoi prestazioni migliori
- Hai bisogno di operazioni batch
- Preferisci app nativa
- Vuoi integrazione sistema

## Migrazione Web → Desktop

### File di Configurazione
I file CSV e i dati possono essere trasferiti direttamente - entrambe le versioni usano lo stesso formato.

### SessionStorage → localStorage
La versione desktop usa localStorage anziché sessionStorage. I dati sono persistenti tra le sessioni.

### URL Parameters
La versione desktop non usa parametri URL. Le configurazioni sono salvate localmente.

## Sviluppo

### Web
```bash
cd LoggerViewer
./run-linux.sh  # Avvia Chrome con flag
```

### Desktop
```bash
cd LoggerViewer_Desktop
npm run dev     # Avvia Electron con DevTools
```

## Build

### Web
- Nessun build richiesto
- Deploy su web server
- Richiede HTTPS per alcune API

### Desktop
```bash
npm run build         # Build per OS corrente
npm run build:linux   # Linux specifico
npm run build:win     # Windows specifico
npm run build:mac     # macOS specifico
```

## Dimensioni

**Web:**
- HTML/CSS/JS: ~200 KB
- Dipendenze: Chart.js da CDN
- **Totale download**: ~200 KB

**Desktop:**
- Applicazione completa: ~150 MB
- Include Electron + Node.js + Chromium
- AppImage Linux: ~180 MB
- Installer Windows: ~90 MB (compresso)

## Performance

**Web:**
- Dipende dal browser
- Limitato da sandbox browser
- Gestione memoria del browser

**Desktop:**
- Controllo completo risorse
- Ottimizzazione nativa
- Gestione memoria dedicata

## Sicurezza

**Web:**
- Sandbox browser forte
- Permissions API rigoroso
- Limitato accesso file system

**Desktop:**
- Accesso completo sistema
- Richiede permessi utente
- Potenzialmente meno sandbox

## Conclusioni

**Usa Web se:**
- Vuoi semplicità
- Uso occasionale
- Non vuoi installare

**Usa Desktop se:**
- Lavoro professionale
- Uso frequente
- Vuoi massime prestazioni
- Hai problemi con browser APIs
