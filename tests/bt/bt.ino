#include <ArduinoBLE.h>

// Definisce l'UUID del servizio e della caratteristica
const char* serviceUUID = "12345678-9878-9878-9878-123456789abc";
const char* characteristicUUID = "abcd1234-9878-9878-9878-abcdef123456";

BLEService myService(serviceUUID); // Crea un servizio BLE
BLEStringCharacteristic myCharacteristic(characteristicUUID, BLERead | BLEWrite | BLENotify, 20); // Consente di leggere, scrivere e notificare stringhe fino a 20 byte

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Inizializzazione BLE fallita!");
    while (1);
  }

  BLE.setLocalName("Arduino BLE");
  BLE.setAdvertisedService(myService); // Pubblicizza il servizio BLE
  myService.addCharacteristic(myCharacteristic); // Aggiunge la caratteristica al servizio
  BLE.addService(myService); // Aggiunge il servizio BLE

  BLE.advertise(); // Inizia a pubblicizzare il BLE

  Serial.println("Arduino in attesa di connessioni BLE...");
}

void loop() {
  BLEDevice central = BLE.central(); // Aspetta un dispositivo BLE da connettersi

  if (central) { // Se un dispositivo si è connesso...
    Serial.print("Connesso a ");
    Serial.println(central.address());

    // Invia la stringa "Hello" quando il dispositivo client si connette
    myCharacteristic.writeValue("Hello");

    while (central.connected()) { // Mentre il dispositivo è connesso...
      if (myCharacteristic.written()) { // Se è stata scritta una nuova stringa...
        // Ottiene la stringa scritta sulla caratteristica
        String newString = myCharacteristic.value();
        Serial.print("Stringa ricevuta: ");
        Serial.println(newString);

        // Qui puoi aggiungere codice per agire in base alla stringa ricevuta
        // Per esempio, inviare la stringa indietro o attivare funzionalità
      }
    }

    Serial.print("Disconnesso da ");
    Serial.println(central.address());
  }
}
