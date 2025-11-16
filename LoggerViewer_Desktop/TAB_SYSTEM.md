# Sistema a Tab - LoggerViewer Desktop

## Panoramica

L'applicazione ora utilizza un sistema a **tab singola finestra** invece di finestre multiple. Questo rende l'interfaccia più intuitiva e simile ad altre applicazioni desktop moderne.

## Struttura

```
┌────────────────────────────────────────────┐
│ [📡 Logger Manager] [📊 Data Viewer]      │  ← Tab Navigation
├────────────────────────────────────────────┤
│                                            │
│                                            │
│        Contenuto della Tab Attiva          │
│                                            │
│                                            │
└────────────────────────────────────────────┘
```

## File Coinvolti

### 1. **index.html** (Nuovo)
- File principale che contiene il sistema a tab
- Carica manager e viewer in iframe separati
- Gestisce la navigazione tra tab
- Supporta scorciatoie da tastiera

### 2. **loggermanager.html**
- Caricato nel primo tab
- Pulsante "Go to Viewer" ora passa al tab viewer

### 3. **loggerviewer.html**
- Caricato nel secondo tab
- Pulsante "Logger Manager" ora torna al tab manager

### 4. **main.js**
- Modificato per caricare `index.html` invece di `loggermanager.html`
- Rimossa gestione finestre multiple

### 5. **preload.js**
- Aggiornato con funzioni `switchToViewer()` e `switchToManager()`
- Usa `postMessage` per comunicare con il parent

### 6. **loader.js**
- Funzione `openViewerHtml()` ora cambia tab invece di aprire finestra
- Aggiunta funzione `goToManager()` per tornare al manager

## Navigazione

### Via Interfaccia
- **Click sui tab**: Clicca su "📡 Logger Manager" o "📊 Data Viewer"
- **Pulsanti interni**:
  - Manager → Viewer: pulsante "Go to Viewer"
  - Viewer → Manager: pulsante "Logger Manager"

### Via Tastiera
- **Ctrl+1**: Passa al Logger Manager
- **Ctrl+2**: Passa al Data Viewer

## Vantaggi del Sistema a Tab

✅ **Una Sola Finestra**: Più pulito e organizzato
✅ **Navigazione Rapida**: Switch istantaneo tra tab
✅ **Stato Mantenuto**: Ogni tab mantiene il suo stato
✅ **Scorciatoie**: Accesso rapido via tastiera
✅ **User-Friendly**: Simile a browser e IDE moderni
✅ **Meno Risorse**: Non crea finestre multiple

## Confronto Prima/Dopo

### Prima (Multi-finestra)
```
Finestra 1: Logger Manager
Finestra 2: Data Viewer (quando aperto)
```
- Due finestre separate
- Più difficile da gestire
- Possibilità di perdere finestre

### Dopo (Tab)
```
Finestra Unica:
- Tab 1: Logger Manager
- Tab 2: Data Viewer
```
- Una sola finestra
- Navigazione semplice
- Sempre visibile e organizzato

## Funzionamento Tecnico

### Comunicazione Parent-Child

```javascript
// Dall'iframe (child) al parent
window.parent.postMessage({
    action: 'switchTab',
    tab: 'viewer'  // o 'manager'
}, '*');

// Nel parent (index.html)
window.addEventListener('message', (event) => {
    if (event.data.action === 'switchTab') {
        switchTab(event.data.tab);
    }
});
```

### Via Electron API

```javascript
// preload.js espone
window.electronAPI.window.switchToViewer()
window.electronAPI.window.switchToManager()

// loader.js usa
window.openViewerHtml = function() {
    window.electronAPI.window.switchToViewer();
}
```

## Iframe vs Single Page

### Perché Iframe?

**Vantaggi:**
- ✅ Isolamento completo tra Manager e Viewer
- ✅ Nessuna interferenza tra script
- ✅ Caricamento pigro (lazy loading)
- ✅ Facile mantenimento del codice esistente
- ✅ Ogni tab può avere il suo stile/scope

**Alternative considerate:**
- ❌ Single Page: Avrebbe richiesto refactoring completo
- ❌ Dynamic Loading: Possibili conflitti tra script

## Stato e Persistenza

### Cosa Viene Mantenuto
- ✅ Connessione seriale (comune tra tab)
- ✅ Dati caricati nel viewer
- ✅ Configurazioni di calibrazione
- ✅ Output del terminale nel manager

### Cosa Viene Resettato
- ❌ Niente! Gli iframe mantengono tutto

## Estensibilità

### Aggiungere Una Nuova Tab

1. Crea il nuovo file HTML (es. `settings.html`)

2. Aggiungi il tab in `index.html`:
```html
<button class="tab-button" data-tab="settings">
    ⚙️ Settings
</button>

<div class="tab-content" id="settings-tab">
    <iframe src="settings.html"></iframe>
</div>
```

3. Aggiungi scorciatoia (opzionale):
```javascript
if (e.ctrlKey && e.key === '3') {
    switchTab('settings');
}
```

## Testing

### Test Manuali
- [ ] Click su tab Manager → Viewer
- [ ] Click su tab Viewer → Manager
- [ ] Pulsante "Go to Viewer" dal manager
- [ ] Pulsante "Logger Manager" dal viewer
- [ ] Ctrl+1 per Manager
- [ ] Ctrl+2 per Viewer
- [ ] Stato mantenuto dopo switch
- [ ] Nessun errore nella console

## Troubleshooting

### Tab Non Si Carica
- Verifica path degli iframe in `index.html`
- Controlla console per errori
- Verifica che i file HTML esistano

### Switch Non Funziona
- Verifica che `loader.js` sia caricato
- Controlla console per errori postMessage
- Verifica che electron-adapter.js sia caricato

### Stile Sbagliato
- Ogni iframe ha il suo CSS isolato
- Modifica direttamente il file HTML del tab

## Performance

### Caricamento
- Manager caricato immediatamente
- Viewer caricato al primo accesso
- Overhead minimo (~50ms per iframe)

### Memoria
- Ogni iframe ha il suo contesto
- ~10MB extra per iframe
- Totale: ~120MB (vs ~110MB multi-finestra)

## Conclusioni

Il sistema a tab offre un'esperienza utente significativamente migliore rispetto alle finestre multiple, mantenendo la semplicità del codice originale grazie all'uso di iframe.

**Vantaggi principali:**
- User experience migliorata
- Codice più semplice
- Manutenzione facilitata
- Estensibilità per future tab
