# Procedura di Test NASO

Procedura per testare il datalogger NASO prima della consegna.

## Test Funzionale

1. **Collegamento Seriale**
   - Collegare il NASO alla porta USB del PC
   - Aprire terminale seriale a 19200 baud (Arduino IDE o Serial USB Terminal)
   - Oppure utilizzare la [webapp](https://applications.techmakers.it/datalogger/terminal.htm)

2. **Verifica Inizializzazione**
   - Verificare lampeggio dei LED durante fase di preriscaldamento
   - Attendere stabilizzazione del sensore MQ-2 (circa 30 secondi)
   - Il valore ADC deve stabilizzarsi scendendo intorno a 10-150

3. **Configurazione**
   - Impostare data e ora con comando `settime`
   - Eseguire comando `autocalib` per calibrazione automatica
   - In alternativa, impostare manualmente il valore zerogas con `setconfig` (solitamente ~50)

4. **Verifica Sensore**
   - Eseguire comando `plotter start` per modalità visualizzazione
   - Verificare funzionamento con deodorante spray o alitando sul sensore
   - I valori ADC e PPM devono alzarsi e poi ridiscendere

5. **Test Scrittura SD**
   - Verificare che i dati vengano salvati su microSD
   - Controllare che non compaiano errori "failed"

## Risoluzione Problemi

- **LED non lampeggiano**: Verificare alimentazione e connessione shield
- **Sensore non si stabilizza**: Attendere più tempo preriscaldamento (fino a 2-3 minuti)
- **Errore SD**: Verificare che la microSD sia inserita e formattata (FAT32)
- **Valori non reagiscono**: Verificare collegamento sensore MQ-2
