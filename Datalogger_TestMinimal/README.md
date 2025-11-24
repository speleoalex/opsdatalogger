# Datalogger Test Minimal

Firmware minimalista per testare lo shield Data Logger con RTC e scheda SD.

## Scopo

Questo firmware serve per verificare il corretto funzionamento dei componenti base:
- Arduino UNO
- Data Logger Shield
- RTC (Real Time Clock) DS1307
- Scheda microSD

## Utilizzo

1. Caricare il firmware su Arduino UNO
2. Inserire una scheda microSD formattata
3. Aprire il Serial Monitor (19200 baud)
4. Verificare l'output:
   - Inizializzazione SD
   - Rilevamento RTC
   - Scrittura file di test

## Firmware Completo

Per il firmware completo di FluxyLogger con supporto sensori, vedere:
- [FluxyLogger/FluxyLogger.ino](../FluxyLogger/FluxyLogger.ino)
