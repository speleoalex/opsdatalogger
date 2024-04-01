/**
   FluxyLogger Datalogger - OpenSpeleo Project
   Author: Alessandro Vernassa (speleoalex@gmail.com)
   Copyright: 2021
   License: GNU General Public License
   Product link: https://techmakers.eu/

   This sketch is designed for the N.A.S.O. datalogger, available as a pre-assembled unit or kit.


  NOTE:
  Datalogger use 6 pin. Analog 4 and 5 for I2C.
  SD card use i digital pin 13, 12, 11, and 10.
  0=seriale rx
  1=seriale tx

*/

#define VERSION 2.41

#define BOUDRATE 19200  // 9600,57600,19200,115200
// Sensor presence configuration
#define BMP280_PRESENT 0    // Set to 1 if BMP280 sensor is present, 0 otherwise
#define HUMIDITY_PRESENT 0  // Set to 1 if a humidity sensor is present, 0 otherwise
#define DEBUGSENSOR 0       // Enable (1) or disable (0) sensor debugging
// Calibration and operation parameters
#define MINCONSECUTIVE_POSITIVE 20  // Minimum number of consecutive positive readings
#define MQ2_PREHEAT_TIME_S 30       // Preheat time in seconds for the MQ2 sensor
#define MINPPMPOSITIVE 10           // Minimum PPM for a positive reading

// Sensors
#define WINDSENSOR_PRESENT 0
#define MQ2SENSOR_PRESENT 1  // VOC MQ2 analogic

#define SGP40_PRESENT 0  // VOC SGP40 i2c
#define SGP30_PRESENT 0  // VOC SGP30 i2c
#define OLED_PRESENT 0   // OLED
#define LOGVCC 0
#define LCD_I2C_ENABLED 1  // LCD
#if defined(ARDUINO_UNOWIFIR4)
#define BLE_ENABLED 1
#else
#define BLE_ENABLED 0
#endif
// Pins
#define S0 A0    // AIR
#define LED_1 3  // Led 1 datalogger
#define LED_2 4  // Led 2 datalogger
// #define LED_1 8    // Led 1 datalogger
// #define LED_2 9    // Led 2 datalogger
#define CHIP_SD 10  // pin sd

#if WINDSENSOR_PRESENT
#define analogPinForRV A1  // change to pins you the analog pins are using
#define analogPinForTMP A2
#endif

// Buffer sizes
#define size_serialBuffer 40
#define MAX_ROWS_PER_FILE 10000  // max rows per file

// Log configs
#define LOG_INTERVAL_s 30       // log interval in seconds
#define SYNC_INTERVAL_ms 20000  // mills between calls to flush() - to write data to the card
#define NUM_READS 8
#define READS_DELAY 30
#define ZEROGAS_default 115
#define RL_VALUE 5.0  // Define the load resistance value in kilo ohms

// Libraries
#define USE_FAT_FILE_FLAG_CONTIGUOUS 0
#define USE_LONG_FILE_NAMES 0
#define USE_UTF8_LONG_NAMES 0
#define SDFAT_FILE_TYPE 1
#define CHECK_FLASH_PROGRAMMING 1  // May cause SD to sleep at high current.

#include <SdFatConfig.h>
#include <SdFat.h>
#include <Wire.h>
#include <RTClib.h>

#if SGP30_PRESENT
#include <SGP30.h>  // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#endif

#if OLED_PRESENT
#include <Tiny4kOLED.h>
uint8_t width = 128;
uint8_t height = 64;
#endif

#if LCD_I2C_ENABLED
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define BTN_1 A1
#endif
#if SGP40_PRESENT
#include "sensor_SGP40/SparkFun_SGP40_Arduino_Library.h"
#include "sensor_SGP40/sensirion_voc_algorithm.h"
#include "sensor_SGP40/SparkFun_SGP40_Arduino_Library.cpp"
#include "sensor_SGP40/sensirion_voc_algorithm.c"

SGP40 sgp40Sensor;  // create an object of the SGP40 class
#endif

#if BLE_ENABLED

#include <ArduinoBLE.h>
#define BLE_NAME "NASO_BT"

//#define BLE_UUID_SERVICE "0000ffe0-0000-1000-8000-00805f9b34fb"
//#define BLE_UUID_Characteristic "0000ffe1-0000-1000-8000-00805f9b34fb"

#define BLE_UUID_SERVICE "12345678-9878-9878-9878-123456789abc"
#define BLE_UUID_Characteristic "abcd1234-9878-9878-9878-abcdef123456"
#define BLE_BUFFERLEN 80

BLEService btMyService(BLE_UUID_SERVICE);  // create service: "Device Information"
// create direction control characteristic and allow remote device to read and write
BLEStringCharacteristic myCharacteristic(BLE_UUID_Characteristic, BLERead | BLEWrite | BLENotify, BLE_BUFFERLEN);
BLEDevice btController;
bool BLEconnected = false;

void initBluetouth() {
  if (!BLE.begin()) {
    Serial.println("Bluetooth® Low Energy module failed!");
  } else {
    Serial.println("Bluetooth® Low Energy module enabled");
  }
  BLE.setLocalName(BLE_NAME);
  BLE.setAdvertisedService(btMyService);
  // add the characteristics to the service
  btMyService.addCharacteristic(myCharacteristic);
  // add the service
  BLE.addService(btMyService);
  // start advertising
  BLE.advertise();
}

/**
 * @brief Manage Bluetouth
 *
 */
void gestBluetouth() {
  String bufferRead = "";
  if (!BLEconnected) {
    // listen for BLE peripherals to connect:
    // if a central is connected to peripheral:
    btController = BLE.central();
    if (btController) {
      SerialPrintln("BLE Connected to controller: ");
      // print the controller's MAC address:
      Serial.println(btController.address());
      BLEconnected = true;
    }
  } else {
    // while the controller is still connected to peripheral:
    if (btController.connected()) {
      /*if (myCharacteristic.written()) {
        bufferRead = myCharacteristic.value();
      }
      Serial.print(bufferRead);*/
    } else {
      BLEconnected = false;
      Serial.println("BLE disconnected");
    }
  }
}

String readBluetouth() {
  String bufferRead = "";
  if (!BLEconnected) {
    // listen for BLE peripherals to connect:
    // if a central is connected to peripheral:
    btController = BLE.central();
    if (btController) {
      Serial.println("BLE Connected to controller: ");
      // print the controller's MAC address:
      Serial.println(btController.address());
      BLEconnected = true;
    }
  } else {
    // while the controller is still connected to peripheral:
    if (btController.connected()) {
      if (myCharacteristic.written()) {
        bufferRead = myCharacteristic.value();
      }
      Serial.print(bufferRead);
    } else {
      BLEconnected = false;
      Serial.println("BLE disconnected");
    }
  }
  return bufferRead;
}

#endif
//none
void SerialPrintln() {
  SerialPrint("\r\n");
}

//const char *
void SerialPrint(const char *data) {
  Serial.print(data);
#if BLE_ENABLED
  if (BLEconnected) {
    myCharacteristic.writeValue((const char *)data);
  }
#endif
}
void SerialPrintln(const char *data) {
  SerialPrint(data);
  SerialPrint("\r\n");
}
//char
void SerialPrint(char data) {
  Serial.print(data);
#if BLE_ENABLED
  if (BLEconnected) {
    myCharacteristic.writeValue((const char *)data);
    myCharacteristic.writeValue("\n");
  }
#endif
}
void SerialPrintln(char data) {
  SerialPrint(data);
  SerialPrint("\r\n");
}
#if defined(ARDUINO_UNOWIFIR4)
//const arduino::__FlashStringHelper *
void SerialPrint(const arduino::__FlashStringHelper *data) {
  Serial.print(data);
#if BLE_ENABLED
  if (BLEconnected) {
    myCharacteristic.writeValue(data);
    myCharacteristic.writeValue("\n");
  }
#endif
}
void SerialPrintln(const arduino::__FlashStringHelper *data) {
  SerialPrint(data);
  SerialPrint("\r\n");
}

#else
void SerialPrint(const __FlashStringHelper *data) {
  Serial.print(data);
}
void SerialPrintln(const __FlashStringHelper *data) {
  SerialPrint(data);
  SerialPrint("\r\n");
}
#endif







//int
void SerialPrint(int data, int format) {
  Serial.print(data, format);
#if BLE_ENABLED
  if (BLEconnected) {
    myCharacteristic.writeValue(String(data));
  }
#endif
}
void SerialPrintln(int data, int format) {
  SerialPrint(data, format);
  SerialPrint("\r\n");
}
//int
void SerialPrint(unsigned long data) {
  Serial.print(data);
#if BLE_ENABLED
  if (BLEconnected) {
    myCharacteristic.writeValue(String(data));
  }
#endif
}
void SerialPrintln(unsigned long data) {
  SerialPrint(data);
  SerialPrint("\r\n");
}
void SerialPrint(int data) {
  Serial.print(data);
#if BLE_ENABLED
  if (BLEconnected) {
    myCharacteristic.writeValue(String(data));
  }
#endif
}
void SerialPrintln(int data) {
  SerialPrint(data);
  SerialPrint("\r\n");
}
//float
void SerialPrint(float data) {
  Serial.print(data);
#if BLE_ENABLED
  if (BLEconnected) {
    myCharacteristic.writeValue(String(data));
  }
#endif
}
void SerialPrintln(float data) {
  SerialPrint(data);
  SerialPrint("\r\n");
}
//char *
void SerialPrint(char *data) {
  Serial.print(data);
#if BLE_ENABLED
  if (BLEconnected) {
    myCharacteristic.writeValue(data);
  }
#endif
}
void SerialPrintln(char *data) {
  SerialPrint(data);
  SerialPrint("\r\n");
}




#if BMP280_PRESENT
#include "sensor_BMP280/farmerkeith_BMP280.h"
bme280 bme0(0, DEBUGSENSOR);
bool BMP280Present = false;
#endif

// Start Global variables -->
bool plotterMode = false;  // Arduino plotter protocol
int gasDetected = 0;       // Gas detected
#if MQ2SENSOR_PRESENT
int zeroGasValue = ZEROGAS_default;  // Define the default value for zero gas calibration
#endif
#if SGP30_PRESENT
SGP30 SGP;
#endif
// strings used multiple times
char CONF_FILE[] = "CONFIG.INI";  // configuration file
const char textOk[] = "ok";
const char textFailed[] = "failed";
const char textNotFound[] = "unknown command";
// variables for the microSD
SdFat SD;
File logfile;
File settingFile;
int8_t logfileOpened = 0;
bool sdPresent = false;

// variables for realtime clock
bool rtcPresent = false;
RTC_DS1307 RTC;  // define the Real Time Clock object
DateTime now;

bool echoToSerial = true;
unsigned long log_interval_s = LOG_INTERVAL_s;  // log interval
unsigned long timeStartLog = 0;
unsigned long currentMillis = 0;
unsigned long timeTarget = 0;
unsigned long timeTargetFileWrite = 0;
float PPMGas = 0;             // Gas ppm
float PPMGasOld = 0;          // Gas ppm
unsigned int LogCounter = 0;  // counter
unsigned int dataToWrite = 0;
float rawSensorValue;
float rawSensorValueOld = 0;
int sensorReadingCounter = 0;

// status
bool failed = false;

char strDate[20];        //= "0000-00-00 00:00:00";
char strFileDate[24];    // = "YYYY-MM-DD_HH.mm.ss.txt";
char delimiter[] = ";";  // CSV delimiter
int sensorReading[NUM_READS];
char serialBuffer[size_serialBuffer];
// End Global variables --<
// Functions
// print ok of fail
bool printResult(bool isOk, bool ln = false) {
  if (isOk) {
    SerialPrint(textOk);
  } else {
    SerialPrint(textFailed);
  }
  if (ln) {
    SerialPrintln();
  }
  return isOk;
}

#if MQ2SENSOR_PRESENT
// Function to convert raw sensor value to parts per million (PPM) for MQ2 gas sensor
float MQ2_RawToPPM(float rawValue) {
  // If the zero calibration value is greater than or equal to the raw value, return 0 (no gas)
  if (zeroGasValue >= rawValue) {
    return 0;
  }

  // If the zero calibration value is less than or equal to 0, set it to 1 to avoid division by zero
  if (zeroGasValue <= 0) {
    zeroGasValue = 1;
  }

// Sensor characteristics for calibration
#define RAL 9.83                             // Ratio of load resistance to sensor resistance in clean air
  float LPGCurve[3] = { 2.3, 0.21, -0.47 };  // Calibration curve for LPG gas
  // Calculate the resistance of the sensor at zero gas concentration
  float ResZero = MQ2_calc_res(zeroGasValue) / RAL;
  // Calculate the current sensor resistance
  float ResCurrent = MQ2_calc_res(rawValue);
  // Convert the percentage to PPM
  return MQ2_PPM_gas(ResCurrent, ResZero, LPGCurve);
}

// Function to calculate the percentage of gas concentration
float MQ2_PPM_gas(float resCurrent, float resZero, float *pCurve) {
  // Calculate the ratio of current resistance to zero gas resistance
  float rs_ro_ratio = resCurrent / resZero;
  // Calculate and return the gas concentration using the calibration curve
  return pow(10, (((log10(rs_ro_ratio) - pCurve[1]) / pCurve[2]) + pCurve[0]));
}

// Function to calculate the sensor resistance based on the raw ADC value
float MQ2_calc_res(float rawAdc) {
  // Calculate and return the sensor resistance
  return (float)RL_VALUE * (1023.0 - rawAdc) / rawAdc;
}
#endif

// Print firmware version
void printVersion() {
  SerialPrint(F("FluxyLogger "));
#if MQ2SENSOR_PRESENT
  SerialPrint(F("NASO "));
#endif

  SerialPrintln((float)VERSION);
  SerialPrint(F("Build time: "));
  SerialPrint(F(__DATE__));
  SerialPrint(F(" "));
  SerialPrintln(F(__TIME__));
}

// Function to list files on the SD card
void listFiles() {
  // size log files: 2023-11-22_20.18.33.txt
  char printBuffer[25];
  SdFile file;
  SdFile dir;
  if (!dir.open("/", O_RDONLY)) {
    return;
  }
  SerialPrintln("Files:");
  // Open the first file in the directory
  dir.rewind();
  // Loop through all files in the directory
  while (file.openNext(&dir, O_RDONLY)) {
    // Store the file name in a buffer
    file.getName(printBuffer, sizeof(printBuffer));
    // Check if the entry is not a directory
    if (!file.isDir() && strlen(printBuffer) > 0) {
      // Print the file name
      SerialPrint(printBuffer);
      SerialPrint("\t");
      // Print the file size
      SerialPrintln((int)file.fileSize());
    }
    file.close();
  }
  SerialPrintln();
  dir.close();
}

// Function to handle file download requests
void downloadFile() {
  // Prompt user to type a filename or 'e' to exit
  SerialPrintln(F("Type Filename or e to exit"));
  char data;
  // Infinite loop to handle user input
  while (1) {
    // Read user input from Serial
    readSerial(true);
    // If user types 'e', exit the function
    if (strcmp(serialBuffer, "e") == 0) {
      return;
    }

    // Check if the filename entered is longer than 4 characters
    if (strlen(serialBuffer) > 4) {
      // Attempt to open the file
      File file = SD.open(serialBuffer);
      if (file) {
        // Notify start of transmission
        SerialPrintln(F("Start transmission:"));
        // Read and transmit file contents
        while (file.available()) {
#if BLE_ENABLED
          data = file.read();
          if (BLEconnected) {
            myCharacteristic.writeValue(String(data));
          } else {
            Serial.write(file.read());
          }

#else
          Serial.write(file.read());

#endif
        }

        // Notify end of transmission and filename
        SerialPrint(F("End transmission:"));
        SerialPrintln(serialBuffer);
        file.close();
        return;
      } else {
        // Notify if file opening failed
        SerialPrintln(F("error opening file."));
        return;
      }
    }
  }
}
#if defined(ARDUINO_UNOWIFIR4)
float mapTo_0_1023(float value) {
  return value * 1023.0 / 4095;
}
#endif
// Function to read an analog value from a pin with filtering
float DL_analogReadAndFilter(int analogPin) {
// Array to store sensorReading
#if defined(ARDUINO_UNOWIFIR4)
  analogReadResolution(12);  //  0- 4095
#endif
  int sum = 0;
  int minValue = 1023.0;  // Initialize minValue with the highest possible analog value
  int maxValue = 0.0;     // Initialize maxValue with the lowest possible analog value

  // Loop to read the analog value multiple times
  for (int i = 0; i < NUM_READS; i++) {
    delay(READS_DELAY);  // Delay between sensorReading for stability
    sensorReading[i] = analogRead(analogPin);
#if defined(ARDUINO_UNOWIFIR4)
    sensorReading[i] = mapTo_0_1023(sensorReading[i]);  // Read the value from the analog pin
#endif
    // Find the minimum value among the sensorReading
    if (sensorReading[i] < minValue) {
      minValue = sensorReading[i];
    }
    // Find the maximum value among the sensorReading
    if (sensorReading[i] > maxValue) {
      maxValue = sensorReading[i];
    }
    sum += sensorReading[i];  // Sum up all the sensorReading
  }

  // Remove the minimum and maximum value from the sum for filtering
  sum = sum - minValue - maxValue;

  // Return the average value excluding the min and max values
  return (float)sum / (NUM_READS - 2);
}

void LOGPRINT(char *str) {
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  if (echoToSerial) {
    SerialPrint(str);
  }
}

void LOGPRINT(const char *str) {
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  if (echoToSerial) {
    SerialPrint(str);
  }
}

void LOGPRINT(const __FlashStringHelper *str) {
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  if (echoToSerial) {
    SerialPrint(str);
  }
}

void LOGPRINTLN(const __FlashStringHelper *str) {
  if (logfileOpened > 0) {
    failed = logfile.println(str) ? false : true;
    dataToWrite++;
  }
  if (echoToSerial) {
    SerialPrintln(str);
  }
}

void LOGPRINTLN(char *str) {
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  if (echoToSerial) {
    SerialPrintln(str);
  }
}

void LOGPRINTLN(const char *str) {
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  if (echoToSerial) {
    SerialPrintln(str);
  }
}

void LOGPRINT(float val, uint8_t precision = 2) {
  if (isnan(val)) {
    val = 0;
  }
  if (val < 0.0) {
    if (logfileOpened > 0)
      logfile.print('-');
    if (echoToSerial)
      SerialPrint('-');
    val = -val;
  }
  val = round(val * pow(10, precision));
  val = val / pow(10, precision);
  if (echoToSerial) {
    SerialPrint(int(val));  // prints the int part
  }
  if (logfileOpened > 0) {
    failed = logfile.print(int(val), DEC) ? 0 : 1;
    dataToWrite++;
  }
  if (precision > 0) {
    unsigned long frac;
    unsigned long mult = 1;
    byte padding = precision - 1;
    while (precision--)
      mult *= 10;
    if (val >= 0)
      frac = (val - int(val)) * mult;
    else
      frac = (int(val) - val) * mult;

    if (frac > 0) {
      if (logfileOpened > 0)
        logfile.print('.');
      if (echoToSerial)
        SerialPrint('.');
      unsigned long frac1 = frac;
      while (frac1 /= 10)
        padding--;
      while (padding--) {
        if (logfileOpened > 0)
          logfile.print('0');
        if (echoToSerial)
          SerialPrint('0');
      }
      if (logfileOpened > 0) {
        failed = logfile.print(frac, DEC) ? 0 : 1;
      }
      if (echoToSerial)
        SerialPrint(frac, DEC);
    }
  }
}

void LOGPRINT(unsigned long str) {
  if (logfileOpened > 0) {
    logfile.print(str);
    dataToWrite++;
  }
  if (echoToSerial) {
    SerialPrint(str);
  }
}

/**
 *
 */
int InputIntFromSerial(unsigned int defaultValue = 0) {
  SerialFlushAndClear();
  readSerial(true);
  if (strlen(serialBuffer) > 0) {
    /*
    SerialPrint("read:");
    SerialPrintln(serialBuffer);
    SerialPrint("atoi:");
    SerialPrintln(atoi(serialBuffer));
    */
    return atoi(serialBuffer);
  } else {
    return defaultValue;
  }
}

/**

*/
#if LOGVCC

long DL_readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif
  delay(2);             // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);  // Start conversion
  while (bit_is_set(ADCSRA, ADSC))
    ;                   // measuring
  uint8_t low = ADCL;   // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH;  // unlocks both
  long result = (high << 8) | low;
  result = 1125300L / result;  // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result;               // Vcc in millivolts
}
#endif

/**

*/
static char *DL_strNow(void) {
  if (rtcPresent) {
    // DateTime now;
    now = RTC.now();
    sprintf(strDate, "%4d-%2d-%2d_%2d:%2d:%2d", now.year(), now.month(),
            now.day(), now.hour(), now.minute(), now.second());
    for (unsigned int i = 0; i < strlen(strDate) && i < 18; i++) {
      if (strDate[i] == ' ') {
        strDate[i] = '0';
      } else {
        if (strDate[i] == '_') {
          strDate[i] = ' ';
        }
      }
    }
  }
  return strDate;
}

/**

*/
static char *DL_strDateToFilename(void) {
  //  DateTime now;
  now = RTC.now();
  sprintf(strFileDate, "%4d-%2d-%2d_%2d.%2d.%2d.txt", now.year(), now.month(),
          now.day(), now.hour(), now.minute(), now.second());
  for (unsigned int i = 0; i < strlen(strFileDate) && i < 18; i++) {
    if (strFileDate[i] == ' ') {
      strFileDate[i] = '0';
    }
  }
  return strFileDate;
}

// Close log file
void DL_closeLogFile() {
  if (logfileOpened > 0) {
    logfile.flush();
    logfile.close();
    logfileOpened = 0;
  }
}

// Open log file
void DL_openLogFile() {
  if (logfileOpened < 0) {
    return;
  }
  if (sdPresent == true) {
    if (logfileOpened > 0) {
      logfile.flush();
      logfile.close();
      logfileOpened = 0;
      LogCounter = 1;
    }
    char filename[] = "xxxx-xx-xx-xx.xx.xx.txt";

    for (uint8_t i = 0; i < 9; i++) {
      sprintf(filename, DL_strDateToFilename());
      if (!SD.exists(filename)) {
        // only open a new file if it doesn't exist
        logfile = SD.open(filename, FILE_WRITE);
        if (logfile) {
          logfileOpened = 1;
        } else {
          logfileOpened = 0;
        }
        break;
      } else {
        delay(1000);
      }
    }

    if (!logfileOpened) {
      SerialPrint(F("Couldnt create:"));
      SerialPrintln(filename);
    } else {
      SerialPrint(F("Log to:"));
      SerialPrintln(filename);
    }
  } else {
    SerialPrintln(F("Log:serial"));
  }

  LOGPRINT(F("\"date Y-m-d m:s\""));

#if MQ2SENSOR_PRESENT
  LOGPRINT(delimiter);
  LOGPRINT(F("\"gas adc\""));
  LOGPRINT(delimiter);
  LOGPRINT(F("\"LPG PPM\""));
#endif

#if SGP30_PRESENT
  LOGPRINT(F("\"TVOC\""));
  LOGPRINT(delimiter);
  LOGPRINT(F("\"CO2\""));

#endif

#if BMP280_PRESENT
  if (BMP280Present) {
#if HUMIDITY_PRESENT

    LOGPRINT(delimiter);

    LOGPRINT(delimiter);
    LOGPRINT(F("\"temp\""));
    OGPRINT(delimiter);

    LOGPRINT(F("\"pressure\""));
    LOGPRINT(delimiter);

    LOGPRINT(F("\"humidity\""));
#else
    LOGPRINT(delimiter);

    LOGPRINT(F("\"temp\""));
    LOGPRINT(F(\"pressure\""));
    LOGPRINT(delimiter);
#endif
  }
#endif
#if WINDSENSOR_PRESENT
  LOGPRINT(delimiter);
  LOGPRINT(F("\"wind\""));
  LOGPRINT(delimiter);
  LOGPRINT(F("\"WindTemp\""));
#endif
#if SGP40_PRESENT
  LOGPRINT(delimiter);
  LOGPRINT(F("\"VOCIndex \""));
  LOGPRINT(delimiter);
  LOGPRINT(F("\"VOCRaw \""));
#endif

#if LOGVCC
  LOGPRINT(delimiter);
  LOGPRINT(F("\"VCC\""));
#endif
  LOGPRINTLN(F(" "));
}

/**

*/
bool CONF_is_valid_char(char character) {
  if (isalnum(character) || character == '_') {
    return true;
  }
  return false;
}

// log_interval_s
#define MAX_INI_KEY_LENGTH 20
#define MAX_INI_VALUE_LENGTH 5
char lineBuffer[MAX_INI_KEY_LENGTH + MAX_INI_VALUE_LENGTH + 2];  // Buffer for the line

int CONF_getConfValueInt(char *filename, char *key, int defaultValue = 0) {
  File myFile;
  char character;
  char description[MAX_INI_KEY_LENGTH + 2];  // Buffer for the line
  char value[MAX_INI_VALUE_LENGTH + 2];      // Buffer for the line
  bool valid = true;
  int ret = defaultValue;
  int i = 0;
  if (!SD.exists(filename)) {
    return defaultValue;
  }
  myFile = SD.open(filename);
  while (myFile.available()) {
    i = 0;
    description[0] = '\0';
    value[0] = '\0';
    character = myFile.read();
    // cerca una linea valida----->
    if (!CONF_is_valid_char(character)) {
      // Comment - ignore this line
      while (character != '\n' && myFile.available()) {
        character = myFile.read();
      };
      continue;
    }
    // cerca una linea valida-----<
    //----riempo la descrizione---->
    do {
      // SerialPrintln(character);
      if (i >= MAX_INI_KEY_LENGTH - 2) {
        myFile.close();
        return defaultValue;
      }
      description[i++] = character;
      character = myFile.read();
      if (!myFile.available()) {
        myFile.close();
        return defaultValue;
      }
    } while (CONF_is_valid_char(character));
    description[i] = '\0';
    // SerialPrintln(description);
    //----riempo la descrizione----<
    //-------elimino gli spazi------->
    if (character == ' ') {
      do {
        character = myFile.read();
        if (!myFile.available()) {
          myFile.close();
          return defaultValue;
        }
      } while (character == ' ');
    }
    //-------elimino gli spazi-------<
    if (character == '=') {
      if (strcmp(description, key) == 0) {
        //-------elimino gli spazi------->
        do {
          character = myFile.read();
          if (!myFile.available()) {
            myFile.close();
            return defaultValue;
          }
        } while (character == ' ');
        //-------elimino gli spazi-------<
        i = 0;
        value[0] = '\0';
        valid = true;
        while (character != '\n' && character != '\r') {
          if (i >= MAX_INI_VALUE_LENGTH - 2) {
            myFile.close();
            return defaultValue;
          }
          // SerialPrintln(character);
          if (isdigit(character)) {
            value[i++] = character;
          } else if (character != '\n' && character != '\r') {
            // Use of invalid values
            valid = false;
          }
          character = myFile.read();
        };
        value[i] = '\0';
        if (valid) {
          myFile.close();
          // Convert chars to integer
          ret = atoi(value);
          return ret;
        }
      }
    } else {
      while (character != '\n' && myFile.available()) {
        character = myFile.read();
      };
      continue;
    }
  }
  myFile.close();
  return ret;
}

bool CreateINIConf(int NewInterval_s, int NewzeroGasValue = 0) {
  SD.remove(CONF_FILE);
  settingFile = SD.open(CONF_FILE, FILE_WRITE);
  SerialPrint(F("create "));
  SerialPrintln(CONF_FILE);
  if (settingFile) {
    // create new file config.ini------>
    settingFile.print(F("log_interval_s="));
    settingFile.println(NewInterval_s);
#if MQ2SENSOR_PRESENT
    settingFile.print(F("zerogas="));
    settingFile.println(NewzeroGasValue);
#endif
    settingFile.flush();
    settingFile.close();
    // create new file config.ini------<
    printResult(true, true);
  } else {
    printResult(false, true);
    return false;
  }
  return true;
}
// initialize CONFIG.INI
void DL_initConf() {
  if (sdPresent) {
    SerialPrint(CONF_FILE);
    SerialPrint(' ');
    if (!SD.exists(CONF_FILE)) {
      printResult(false, true);
#if MQ2SENSOR_PRESENT
      CreateINIConf(LOG_INTERVAL_s, ZEROGAS_default);
#else
      CreateINIConf(LOG_INTERVAL_s);
#endif
    } else {
      printResult(true, true);
    }
    // read from CONFIG.INI------->
    log_interval_s = CONF_getConfValueInt(CONF_FILE, (char *)"log_interval_s", LOG_INTERVAL_s);
#if MQ2SENSOR_PRESENT
    zeroGasValue = CONF_getConfValueInt(CONF_FILE, (char *)"zerogas", (int)ZEROGAS_default);
    SerialPrint(F("zerogas="));
    SerialPrintln(zeroGasValue);
#endif
    // read from CONFIG.INI-------<
    SerialPrint(F("Interval(s)="));
    SerialPrintln(log_interval_s);
  }
  if (log_interval_s == 0) {
    log_interval_s = LOG_INTERVAL_s;
    SerialPrint(F("Force to "));
    SerialPrintln(LOG_INTERVAL_s);
  }
}

// Function to read data from Serial
static int readSerial(bool wait) {
  static bool ready = false;  // Flag to indicate if a complete line is ready
  static int cnt = 0;         // Counter for the number of characters read
  do {
#if BLE_ENABLED
    if (BLEconnected) {
      String commandTmp = readBluetouth();
      if (commandTmp != "") {
        strcpy(serialBuffer, commandTmp.c_str());
        return 1;
      }
    }
#endif


    // Loop while there is data available on Serial
    while (Serial.available() > 0) {
      // Check if a complete line is ready
      if (ready) {
        ready = false;
        return 1;  // Return 1 to indicate a complete line is ready
      }

      // Read a character from Serial
      char c = Serial.read();
      // Check if the character is a newline, carriage return, or printable ASCII character
      if (c == '\n' || c == '\r' || (c >= 20 && c <= 126)) {
        // Store the character in the buffer
        serialBuffer[cnt] = c;
        // Check if the character is a newline, carriage return, or if the buffer is full
        if (c == '\n' || c == '\r' || c == '\n' || cnt == (size_serialBuffer - 1)) {
          // Null-terminate the string and reset the counter
          serialBuffer[cnt] = '\0';
          cnt = 0;
          ready = true;
        } else {
          // Increment the counter
          cnt++;
        }
      }
    }
  } while (wait);  // Continue if 'wait' is true
  return 0;        // Return 0 to indicate no complete line is ready
}

// switch device in slave mode
void switchCommandMode() {
  DL_closeLogFile();
  logfileOpened = -1;
  echoToSerial = false;
}

// parse command from serial
static int parse_serial_command() {
/*
#if BLE_ENABLED
  if (BLEconnected) {
    String commandTmp = readBluetouth();
    if (commandTmp != "") {
      execute_command((char *)commandTmp.c_str());
    }
  }
#endif
*/
  if (readSerial(false)) {
    execute_command(serialBuffer);
    return 1;
  }
  return 0;
}

void LCD_Print(int cursor) {
#if LCD_I2C_ENABLED
#endif
#if OLED_PRESENT
#endif
}
// switch logs
bool SwithLogs(bool logStart) {
  if (logStart) {
    DL_closeLogFile();
    logfileOpened = 0;
    DL_openLogFile();
  } else {
    DL_closeLogFile();
    logfileOpened = -1;
  }
  return true;
}


// manage commands
void execute_command(char *command) {
  unsigned int i;
  if (strlen(command) > 0) {
    switchCommandMode();
  } else {
    return;
  }
  if (strcmp(command, "?") == 0) {
    SerialPrintln();
    SerialPrintln(F("commands:"));
    SerialPrintln(F("v:firmware version"));
    SerialPrintln(F("logs:read data"));
    SerialPrintln(F("date:display device clock"));
    SerialPrintln(F("reset:reset device"));
    SerialPrintln(F("settime:set device clock"));
    SerialPrintln(F("setconfig:set config"));
    // SerialPrintln(F("echo start/stop: start/stop display values"));
    // SerialPrintln(F("log start/stop: start/stop log"));
    SerialPrintln(F("plotter start/stop: start/stop plotter mode"));
#if MQ2SENSOR_PRESENT
    SerialPrintln(F("autocalib:auto calibration"));
#endif
    SerialPrintln();

    return;
  }

  if (strcmp(command, "v") == 0) {
    SwithLogs(false);
    printVersion();
    return;
  }
  if (strcmp(command, "date") == 0) {
    SwithLogs(false);
    printDateTime();
    return;
  }
  if (strcmp(command, "plotter start") == 0) {
    echoToSerial = false;
    plotterMode = true;
    printResult(true, true);
    return;
  }
  if (strcmp(command, "plotter stop") == 0) {
    plotterMode = false;
    printResult(true, true);
    return;
  }

#if MQ2SENSOR_PRESENT
  if (strcmp(command, "autocalib") == 0) {
    Mq2Calibration();
    return;
  }
#endif
  if (strcmp(command, "ls") == 0) {
    listFiles();
    return;
  }
  if (strcmp(command, "logs") == 0) {
    // Serial.flush();
    listFiles();
    downloadFile();
    return;
  }
  if (strcmp(command, "reset") == 0) {
    DL_closeLogFile();
    printResult(true, true);
    delay(500);
    reset();  // reset arduino
    return;
  }
  if (strcmp(command, "settime") == 0) {
    SetDateTime();
    return;
  }
  if (strcmp(command, "setconfig") == 0) {
    SetConfig();
    return;
  }
  // If user types 'rm ' followed by a filename, delete that file
  if (command[0] == 'r' && command[1] == 'm' && command[2] == ' ') {
    // Remove "rm " from the string to get the filename
    for (i = 0; i < size_serialBuffer - 3; i++) {
      command[i] = command[i + 3];
    }
    command[i] = '\0';

    // Print delete command and filename
    SerialPrint(F("del "));
    SerialPrintln(command);

    // Delete the file and report status
    if (SD.remove(command)) {
      SerialPrintln(F("file deleted"));
    } else {
      SerialPrint(F("error remove "));
      SerialPrintln(command);
    }
    return;
  }

  SerialPrint(command);
  SerialPrint(' ');
  SerialPrintln(textNotFound);
}

#if MQ2SENSOR_PRESENT
// MQ2 sensor auto calibration
void Mq2Calibration() {
  int timeCount;
  int S0Sensor = 0;
  int S0Sensor_old = 0;
  int NumConsecutive = 0;
  SerialPrintln(F("Zerogas calibration"));
#if LCD_I2C_ENABLED
  lcd.setCursor(0, 0);
  lcd.print(F("Calibration"));
#endif

  while (1) {
    S0Sensor = (int)DL_analogReadAndFilter(S0);
    if (S0Sensor == S0Sensor_old) {
      NumConsecutive++;
    } else {
      S0Sensor_old = S0Sensor;
      NumConsecutive = 0;
    }
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    // blinking, delay and parse command
    for (timeCount = 0; timeCount < 100; timeCount++) {
      if (parse_serial_command()) {
        digitalWrite(LED_1, LOW);
        digitalWrite(LED_2, LOW);
        return;
      }
      delay(50);
    }
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    SerialPrint(F("Value:"));
    SerialPrintln(S0Sensor);
#if LCD_I2C_ENABLED
    lcd.setCursor(0, 10);
    lcd.print(S0Sensor);
#endif

    if (NumConsecutive > 10) {
      zeroGasValue = S0Sensor + 1;
      SerialPrintln(F("completed"));
      CreateINIConf((unsigned int)log_interval_s, zeroGasValue);
      return;
    }
  }
}

#endif

// manage config
void SetConfig() {
  unsigned int interval;
  unsigned int zerogas;
  DL_closeLogFile();
  delay(500);
  SerialPrint(F("\nInterval(s):"));
  SerialPrint('(');
  SerialPrint(log_interval_s);
  SerialPrint(')');
  interval = InputIntFromSerial((unsigned int)log_interval_s);
  SerialPrintln();
  if (interval <= 0) {
    interval = log_interval_s;
  }
#if MQ2SENSOR_PRESENT
  zerogas = zeroGasValue;
  SerialPrint(F("Zerogas:"));
  SerialPrint('(');
  SerialPrint(zeroGasValue);
  SerialPrint(')');
  zerogas = InputIntFromSerial((unsigned int)zeroGasValue);
  SerialPrintln();
  if (zerogas <= 0 || zerogas > 1023) {
    zerogas = zeroGasValue;
  }
#else
  zerogas = ZEROGAS_default;
#endif
  if (CreateINIConf(interval, zerogas)) {
    delay(500);
    reset();  // reset arduino
  } else {
    SerialPrintln(textFailed);
  }
}
// Set date and time
void SetDateTime() {
  DL_closeLogFile();
  delay(500);
  SerialFlushAndClear();
  int year, month, day, hours, minutes, seconds;
  SerialPrintln(F("\nClock:"));
  SerialPrint(F("Year:"));
  year = InputIntFromSerial();
  SerialPrintln(year);
  SerialPrint(F("Month:"));
  month = InputIntFromSerial();
  SerialPrintln(month);
  SerialPrint(F("Day:"));
  day = InputIntFromSerial();
  SerialPrintln(day);
  SerialPrint(F("Hours:"));
  hours = InputIntFromSerial();
  SerialPrintln(hours);
  SerialPrint(F("Minutes:"));
  minutes = InputIntFromSerial();
  SerialPrintln(minutes);
  SerialPrint(F("Seconds:"));
  seconds = InputIntFromSerial();
  SerialPrintln(seconds);
  RTC.adjust(DateTime(year, month, day, hours, minutes, seconds));
  delay(500);
  reset();
}

// Funzione di reset personalizzata
void reset() {
#ifdef ARDUINO_AVR_UNO
  asm volatile("  jmp 0");
#elif defined(ARDUINO_AVR_MEGA2560)
  asm volatile("  jmp 0");
#elif defined(ARDUINO_AVR_NANO)
  asm volatile("  jmp 0");
#else
  NVIC_SystemReset();
#endif
}
// Reset Arduino

//  asm volatile("  jmp 0");

// flush serial and clears the reading queue
void SerialFlushAndClear() {
  Serial.flush();
  while (Serial.available() > 0) {
    Serial.read();
  }
}
// manages the flashing time depending on the ppm
void manageBlinking(unsigned long timeOn, unsigned long timeOff) {
  if (failed) {
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    return;
  }
  static bool ledState = false;
  static unsigned long previousMillis = 0;
  if (timeOn <= 0 && timeOff <= 0) {
    // Se entrambi i tempi sono 0, spegni il LED e esci dalla funzione
    digitalWrite(LED_2, LOW);
    return;
  }
  // Ottieni il tempo attuale
  currentMillis = millis();
  if (ledState) {
    // Se il LED è acceso, controlla se è ora di spegnerlo
    if (currentMillis - previousMillis >= timeOn) {
      ledState = false;
      previousMillis = currentMillis;
      digitalWrite(LED_2, LOW);
    }
  } else {
    // Se il LED è spento, controlla se è ora di accenderlo
    if (currentMillis - previousMillis >= timeOff) {
      ledState = true;
      previousMillis = currentMillis;
      digitalWrite(LED_2, HIGH);
    }
  }
}
// print date and time to Serial
void printDateTime() {
  if (RTC.isrunning()) {
    SerialPrint(F("Device clock:"));
    SerialPrintln(DL_strNow());
  } else {
    SerialPrintln(textFailed);
  }
}
// manages the flashing time depending on the ppm
void manageBlinkingByPPM(float inputValue) {
  uint16_t MaxValue = 500;
  // Assicurati che il valore di input sia entro i limiti previsti
  if (inputValue > MaxValue || inputValue < 0) {
    inputValue = MaxValue;
  }
  inputValue = constrain(inputValue, 0, MaxValue);
  if (inputValue == 0) {
    manageBlinking(500, 30000);
    return;
  }
  unsigned long time = map(inputValue, 1, MaxValue, 2000, 100);
  // Calcola timeOn e timeOff come metà del tempo totale del ciclo
  unsigned long timeOn = time / 2;
  unsigned long timeOff = time / 2;
  manageBlinking(timeOn, timeOff);
}

#if LCD_I2C_ENABLED
void GestLcd() {
  int val = 0;
  static int oldval = 0;
  static unsigned long ledTimer = 0;
  if (currentMillis > ledTimer + 1000) {
    val = digitalRead(BTN_1);
    if (oldval == val) {
      // SerialPrintln(val);
      if (val) {
        lcd.setBacklight(1);
        //        SerialPrintln("lcd on");
      } else {
        lcd.setBacklight(0);
        //        SerialPrintln("lcd off");
      }
    }
    oldval = val;
    ledTimer = currentMillis;
  }
}
#endif
// Setup
void setup() {
#if LCD_I2C_ENABLED
  lcd.init();
  lcd.clear();
  lcd.setBacklight(1);
  lcd.setCursor(0, 0);
  lcd.print(F("NASO "));
  lcd.print(VERSION);
  delay(1000);
  lcd.setBacklight(0);
#endif
  //--init serial------------------------------>
  Serial.begin(BOUDRATE);  //  boud
  //--init serial------------------------------<
  sdPresent = false;
  rtcPresent = false;
  logfileOpened = false;
  printVersion();
  // initialize the SD card ------------------->
  pinMode(CHIP_SD, OUTPUT);
  delay(200);
#if SGP30_PRESENT
  SerialPrint(F("init SGP30 "));
  if (!SGP.begin()) {
    SerialPrintln(F("failed"));
  } else {
    SerialPrintln(ok);
  }
#endif

  SerialPrint(F("init SD "));
  if (!SD.begin(CHIP_SD)) {
    sdPresent = false;
    printResult(false, true);
    failed = true;
  } else {
    printResult(true, true);
    sdPresent = true;
  }

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  if (failed) {
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
  }
  delay(200);
  DL_initConf();
  // initialize the SD card -------------------<
  SerialFlushAndClear();
  // connect to RTC --------------------------->
  Wire.begin();
  if (RTC.begin()) {
    SerialPrintln(F("RTC present"));
    rtcPresent = true;
    if (!RTC.isrunning()) {
      SerialPrintln(F("Clock not running"));
      RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    printDateTime();
  } else {
    rtcPresent = false;
    SerialPrintln(F("RTC Failed"));
    failed = true;
  }
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_1, LOW);

  // connect to RTC ---------------------------<

#if SGP40_PRESENT
  if (sgp40Sensor.begin() == true) {
    SerialPrintln(F("SGP40 detected"));
  }
#endif
#if BMP280_PRESENT
  if (!bme0.begin()) {
    SerialPrintln(F("\nBMP280 sensor not present"));
    BMP280Present = false;
  } else {
    SerialPrintln(F("\nBMP280 sensor present"));
    BMP280Present = true;
  }
#endif
  delay(10);
  currentMillis = millis();
  timeTarget = currentMillis + (log_interval_s * 1000);
  timeTargetFileWrite = currentMillis + SYNC_INTERVAL_ms;
  //-----------init analog input-----------------------------------------<
  // pinMode(LED_BUILTIN, OUTPUT);

  // return;

#if OLED_PRESENT

  oled.begin(width, height, sizeof(tiny4koled_init_128x64br), tiny4koled_init_128x64br);
  oled.clear();
  oled.setFont(FONT6X8);
  oled.on();
  // oled.setCursor(0, 0);
  // oled.println(F("Start log"));
#endif

#if LCD_I2C_ENABLED
  pinMode(BTN_1, INPUT);
  // turn on the backlight
  //  lcd.backlight();
  lcd.setCursor(0, 1);
  if (failed) {
    lcd.print(textFailed);
  } else {
    lcd.print("SD OK");
    lcd.setCursor(0, 0);
    lcd.print(DL_strNow());
  }
  // lcd.print(""+S0Sensor);
#endif
#if BLE_ENABLED
  initBluetouth();
#endif
  timeStartLog = millis() + (1000 * MQ2_PREHEAT_TIME_S);
}
bool On1Seconds = false;
bool On10Seconds = false;
void updateTimers() {
  static unsigned long last_on_10_Seconds = 0;
  static unsigned long last_on_1_Seconds = 0;
  On1Seconds = false;
  On10Seconds = false;
  //----- Time ----->
  currentMillis = millis();
  if (currentMillis - last_on_10_Seconds > 10000) {
    last_on_10_Seconds = currentMillis;
    On10Seconds = true;
  }
  if (currentMillis - last_on_1_Seconds > 1000) {
    last_on_1_Seconds = currentMillis;
    On1Seconds = true;
  }
  //----- Time ----->
}

/**
   Loop.
*/
void loop() {
  static float adcTotal = 0;
  static unsigned int adcCount = 0;

  static unsigned long ledTimer = 0;
  static bool readyToMeasure = false;
  static unsigned long timeTargetContinuousReading = 0;
  float S0Sensor = 0.0;

  currentMillis = millis();
  // commands:
  parse_serial_command();

  updateTimers();
  if (On1Seconds) {
#if BLE_ENABLED
    //    Serial.println("gestBluetouth");
    gestBluetouth();
#endif
  }

#if HUMIDITY_PRESENT
  float humidity = 0;
#endif
  float pressure = 0;
  float temperature = 0;
#if LOGVCC
  long vcc = 0;
#endif
#if WINDSENSOR_PRESENT
#define zeroWindAdjustment 0.2;  // negative numbers yield smaller wind speeds and vice versa.
  int TMP_Therm_ADunits;         // temp termistor value from wind sensor
  float RV_Wind_ADunits;         // RV output from wind sensor
  float RV_Wind_Volts;
  int TempCtimes100;
  float zeroWind_ADunits;
  float zeroWind_volts;
  float WindSpeed_MPH;
#endif

  if (gasDetected >= MINCONSECUTIVE_POSITIVE) {
    digitalWrite(LED_1, HIGH);
  } else {
    digitalWrite(LED_1, LOW);
  }
  if (currentMillis > timeStartLog || plotterMode || logfileOpened == -1) {
    readyToMeasure = true;
  } else {
    if (failed) {
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, HIGH);
    } else {
      analogRead(S0);
      delay(100);
      digitalWrite(LED_1, HIGH);
      delay(100);
      digitalWrite(LED_1, LOW);
      delay(100);
      digitalWrite(LED_1, HIGH);
      delay(100);
      digitalWrite(LED_1, LOW);
      delay(100);
      digitalWrite(LED_2, HIGH);
      delay(100);
      digitalWrite(LED_2, LOW);
      delay(100);
      digitalWrite(LED_2, HIGH);
      delay(100);
      digitalWrite(LED_2, LOW);
      if (echoToSerial) {
        SerialPrintln(F("Preheating"));
      }
#if LCD_I2C_ENABLED
      lcd.setCursor(0, 1);
      lcd.print(F("Preheating"));
#endif
    }
  }
  if (readyToMeasure) {

    if (logfileOpened <= 0) {
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, HIGH);
    }

#if MQ2SENSOR_PRESENT
    if (currentMillis > timeTargetContinuousReading + 1000) {
      timeTargetContinuousReading = currentMillis;
      rawSensorValue = DL_analogReadAndFilter(S0);
      adcTotal += rawSensorValue;
      adcCount++;
      PPMGas = MQ2_RawToPPM(rawSensorValue);
      if (plotterMode) {
        SerialPrint(F("raw:"));
        SerialPrint(rawSensorValue);
        SerialPrint(F(",ppm:"));
        SerialPrintln(PPMGas);
      }
#if LCD_I2C_ENABLED
      if (gasDetected >= MINCONSECUTIVE_POSITIVE) {
        lcd.setCursor(14, 0);  // col row
        lcd.print(gasDetected);
      }
      if (rawSensorValue != rawSensorValueOld || PPMGas != PPMGasOld) {
        lcd.clear();
        rawSensorValueOld = rawSensorValue;
        PPMGasOld = PPMGas;
      }
      lcd.setCursor(0, 0);
      lcd.print("ADC: ");
      lcd.print(rawSensorValue);
      lcd.setCursor(0, 1);
      lcd.print("PPM: ");
      lcd.print(PPMGas);
      if (logfileOpened <= 0) {
        lcd.setCursor(14, 1);
        lcd.print('E');
      }

#endif
      manageBlinkingByPPM(PPMGas);
    }
#endif

    if (currentMillis >= timeTarget) {
      timeTarget = currentMillis + log_interval_s * 1000;
      if (LogCounter == 0 || (LogCounter >= MAX_ROWS_PER_FILE && (LogCounter % MAX_ROWS_PER_FILE == 0))) {
        DL_openLogFile();
      }
      //-----------sensors------------------------------------------------------->
      if (adcCount > 0) {
        S0Sensor = adcTotal / adcCount;
        adcCount = 0;
        adcTotal = 0;
      } else {
        S0Sensor = DL_analogReadAndFilter(S0);
      }

#if OLED_PRESENT
      oled.clear();
#endif

#if LOGVCC
      vcc = DL_readVcc();
#endif
      //-----------sensors-------------------------------------------------------<
      LogCounter++;
      LOGPRINT(F("\""));
      LOGPRINT(DL_strNow());
      LOGPRINT(F("\""));
#if MQ2SENSOR_PRESENT
      LOGPRINT(delimiter);
      LOGPRINT(S0Sensor);

#if OLED_PRESENT
      oled.setCursor(0, 0);
      oled.print(S0Sensor);
#endif

      LOGPRINT(delimiter);
      PPMGas = MQ2_RawToPPM(S0Sensor);
      LOGPRINT(PPMGas);
      if (PPMGas >= MINPPMPOSITIVE) {
        // if (gasDetected < MINCONSECUTIVE_POSITIVE) {
        gasDetected++;
        //}
      } else {
        if (gasDetected < MINCONSECUTIVE_POSITIVE) {
          gasDetected = 0;
        }
      }

#if OLED_PRESENT
      oled.setCursor(0, 10);
      oled.print(MQ2_RawToPPM(S0Sensor), 0);
#endif
#if OLED_PRESENT

#endif

#endif

#if BMP280_PRESENT
      if (BMP280Present) {
        temperature = bme0.readTemperature();
        pressure = bme0.readPressure();
        LOGPRINT(delimiter);
        LOGPRINT(temperature);
        LOGPRINT(delimiter);
        LOGPRINT(pressure, 0);
#if HUMIDITY_PRESENT
        humidity = bme0.readHumidity();
        LOGPRINT(delimiter);
        LOGPRINT(humidity);
#endif
      }
#endif
#if WINDSENSOR_PRESENT
      TMP_Therm_ADunits = DL_analogReadAndFilter(analogPinForTMP);
      RV_Wind_ADunits = DL_analogReadAndFilter(analogPinForRV);
      RV_Wind_Volts = (RV_Wind_ADunits * 0.0048828125);
      // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
      // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
      TempCtimes100 = (0.005 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) - (16.862 * (float)TMP_Therm_ADunits) + 9075.4;
      zeroWind_ADunits = -0.0006 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39
      zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;
      // This from a regression from data in the form of
      // Vraw = V0 + b * WindSpeed ^ c
      // V0 is zero wind at a particular temperature
      // The constants b and c were determined by some Excel wrangling with the solver.
      WindSpeed_MPH = pow(((RV_Wind_Volts - zeroWind_volts) / 0.2300), 2.7265);
      /*
        SerialPrint(F("  TMP volts "));
        SerialPrint(TMP_Therm_ADunits * 0.0048828125);
        SerialPrint(F(" RV volts "));
        SerialPrint((float)RV_Wind_Volts);
        SerialPrint(F("\t  TempC*100 "));
        SerialPrint(TempCtimes100 );
        SerialPrint(F("   ZeroWind volts "));
        SerialPrint(zeroWind_volts);
        SerialPrint(F("   WindSpeed MPH "));
        SerialPrintln((float)WindSpeed_MPH);
      */
      LOGPRINT(delimiter);
      float Wind_mts = WindSpeed_MPH * 0.44704;
      LOGPRINT(Wind_mts, 5);  // m/s
      LOGPRINT(delimiter);
      LOGPRINT((TempCtimes100 / 100));  // m/s
#endif

#if SGP40_PRESENT
      LOGPRINT(delimiter);
      LOGPRINT(sgp40Sensor.getVOCindex(), 2);
      LOGPRINT(delimiter);
      LOGPRINT(sgp40Sensor.getRaw(), 2);

#endif

#if SGP30_PRESENT
      SGP.measure(false);  // returns false if no measurement is made
      LOGPRINT(delimiter);
      LOGPRINT((float)SGP.getTVOC());
      LOGPRINT(delimiter);
      SerialPrint((float)SGP.getCO2());
#endif

#if LOGVCC

      LOGPRINT(delimiter);
      LOGPRINT(String(vcc));
#endif
      LOGPRINT(F("\n"));
    }
    if (dataToWrite != 0 && currentMillis >= timeTargetFileWrite) {
      if (logfileOpened) {
        logfile.flush();
        // SerialPrintln("flush");
      }
      dataToWrite = 0;
      timeTargetFileWrite = currentMillis + SYNC_INTERVAL_ms;
    }
  }
#if LCD_I2C_ENABLED
  GestLcd();
#endif
}