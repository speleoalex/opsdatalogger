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

#define VERSION 3.01
#define VERSION_STR "3.01"

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
#define TGS2610 1            // Figaro sensor

#define SGP40_PRESENT 0  // VOC SGP40 i2c
#define SGP30_PRESENT 0  // VOC SGP30 i2c
#define OLED_PRESENT 0   // OLED
#define LOGVCC 0
#define LCD_I2C_ENABLED 0  // LCD

// Pins
#define S0 A0    // AIR
#define LED_1 3  // Led 1 datalogger
#define LED_2 4  // Led 2 datalogger
// #define LED_1 8    // Led 1 datalogger
// #define LED_2 9    // Led 2 datalogger
#define CHIP_SD 10  // pin sd

#if defined(ARDUINO_UNOWIFIR4)
  #define BLE_ENABLED 1
  #define ADC_MAX   16383
  #define ADC_MAX_F 16383.0
  #define ADC_RESOLUTION 14
#else
  #define BLE_ENABLED 0
  #define ADC_MAX   4095
  #define ADC_MAX_F 4095.0
  #define ADC_RESOLUTION 12 
#endif

#if WINDSENSOR_PRESENT
#define analogPinForRV A1  // change to pins you the analog pins are using
#define analogPinForTMP A2
#endif

// Buffer sizes
#define MAX_ROWS_PER_FILE 10000  // max rows per file =====> WHY ???

// Log configs
#define LOG_INTERVAL_s  10 // 30       // default log interval in seconds
#define MIN_INTERVAL_S  10
#define SYNC_INTERVAL_ms 20000  // millis between calls to flush() - to write data to the SD card
#define NUM_READS 16
#define READS_DELAY 1
#define ZEROGAS_MQ2  200 // 115
#define ZEROGAS_TGS 1050 // 115
#define MIN_ZEROGAS   10
#define RL_VALUE 5.0  // Define the load resistance value in kilo ohms
#define READING_INTERVAL 10000 // msec between readings

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

// char buffer 
char msg[256];

// =======================================================
// BLUETOOTH

#if BLE_ENABLED
  #include <ArduinoBLE.h>
  
  //#define BLE_UUID_SERVICE "0000ffe0-0000-1000-8000-00805f9b34fb"
  //#define BLE_UUID_Characteristic "0000ffe1-0000-1000-8000-00805f9b34fb"
  
  // #define SINGLE_CHRT 1
  
  // #define BLE_UUID_SERVICE "12345678-9878-9878-9878-123456789abc"
  #define BLE_UUID_SERVICE "19b10000-e8f2-537e-4f6c-d104768a1214"
  
  #ifdef SINGLE_CHRT
    // #define BLE_UUID_CHRT  "abcd1234-9878-9878-9878-abcdef123456"
    #define BLE_UUID_CHRT  "19b10000-e8f2-537e-4f6c-d104768a1215"
  #else
    // #define BLE_UUID_WRITE_CHRT "abcd1234-9878-9878-9878-abcdef123457"
    // #define BLE_UUID_READ_CHRT  "abcd1234-9878-9878-9878-abcdef123456"
    #define BLE_UUID_READ_CHRT "19b10000-e8f2-537e-4f6c-d104768a1215"
    #define BLE_UUID_WRITE_CHRT  "19b10000-e8f2-537e-4f6c-d104768a1216"
  #endif

  #define BLE_BUFFERLEN 80 // Bluetooth MTU : must be more than SERIAL_BUFFER_SIZE
  
  BLEService mBtService( BLE_UUID_SERVICE );  // create service: "Device Information"
  // create direction control characteristic and allow remote device to read and write
  #ifdef SINGLE_CHRT
    BLEStringCharacteristic mWriteChrt( BLE_UUID_CHRT, BLERead | BLEWrite | BLENotify, BLE_BUFFERLEN);
    // BLEStringCharacteristic mWriteChrt( BLE_UUID_CHRT, BLERead | BLEWrite, BLE_BUFFERLEN);
    BLEStringCharacteristic mReadChrt = mWriteChrt;
  #else
    // BLEStringCharacteristic mReadChrt( BLE_UUID_READ_CHRT, BLERead | BLEWrite | BLENotify, BLE_BUFFERLEN);
    // BLEStringCharacteristic mWriteChrt( BLE_UUID_WRITE_CHRT, BLERead | BLEWrite | BLENotify, BLE_BUFFERLEN);
    BLEStringCharacteristic mReadChrt( BLE_UUID_READ_CHRT, BLERead | BLEWrite | BLENotify, BLE_BUFFERLEN);
    BLEStringCharacteristic mWriteChrt( BLE_UUID_WRITE_CHRT, BLERead | BLEWrite | BLENotify, BLE_BUFFERLEN);
  #endif
  
  BLEDevice btController;
  bool BLEconnected = false;
  char BLE_NAME[] = "NASO-000000"; // filled with last three hex digit of mac (was NASO_BT)
  
  /**
   * @brief initialize the Bluetooth service
   */
  void initBluetooth()
  {
    if ( ! BLE.begin() ) {
      Serial.println( F("Bluetooth® Low Energy module failed!") );
    } else {
      Serial.println( F("Bluetooth® Low Energy module enabled") );
    }
    
    String address = BLE.address();
    for ( int k=9, j=5; k<17; ++ k ) {
      if ( ((k+1)%3) != 0 ) BLE_NAME[j++] = address[k];
    }
    Serial.println( BLE_NAME );
    
    BLE.setDeviceName( BLE_NAME );
    BLE.setLocalName( BLE_NAME );
    BLE.setAdvertisedService( mBtService );
    // add the characteristics to the service
    // byte myValue[2] = {01, 00}; 
    // BLEDescriptor enable_notifications( "2902", myValue, 2 );
    // mReadChrt.addDescriptor(enable_notifications);
    mBtService.addCharacteristic( mWriteChrt );
    #ifndef SINGLE_CHRT
      mBtService.addCharacteristic( mReadChrt );
    #endif
    BLE.addService( mBtService ); // add the service
    BLE.advertise();              // start advertising
  }
  
  /** @brief Bluetooth connection management
   */
  void handleBluetooth() 
  {
    if ( ! BLEconnected ) { // if a central is connected to peripheral: check if the controller is connected
      btController = BLE.central();
      if ( btController ) {
        // print the controller's MAC address:
        sprintf( msg, "BLE Connected to controller: %s", btController.address() );
        Serial.println( msg );
        BLEconnected = true;
      }
    } else {
      if ( btController.connected() ) { // while the controller is still connected to peripheral:
        // Serial.prinln("Still connected")
        /*
        if (mReadChrt.written()) {
          bufferRead = mReadChrt.value();
          Serial.print("Read <");
          Serial.print(bufferRead);
          Serial.print(">\r\n");
        }
        */
      } else {
        BLEconnected = false;
        Serial.println( F("BLE disconnected") );
      }
    }
  }
  
  /** @brief read a message from the Bluetooth
   */ 
  String readBluetooth( ) 
  {
    String bufferRead = "";
    if ( ! BLEconnected ) { // if a central is connected to peripheral: listen for BLE peripherals to connect:
      btController = BLE.central();
      if ( btController ) {
        // print the controller's MAC address:
        sprintf( msg, "BLE Connected to controller: %s", btController.address().c_str() );
        Serial.println( msg );
        BLEconnected = true;
      }
    } else {
      // while the controller is still connected to peripheral:
      if ( btController.connected() ) {
        if ( mReadChrt.written() ) {
          bufferRead = mReadChrt.value();
        }
        // Serial.print( bufferRead.length );
      } else {
        BLEconnected = false;
        Serial.println( F("BLE disconnected") );
      }
    }
    return bufferRead;
  }
#endif

// =========================================================
// SERIAL 

#define SERIAL_BUFFER_SIZE 40 
char serialBuffer[SERIAL_BUFFER_SIZE];

bool echoToSerial = false; // whether to print data string to serial/bluetooth

/** @brief print carriage-return + new-line to serial
 */
void SerialPrintln()
{
  Serial.print("\r\n");
}

/** @brief print a string to serial+bluetooth
 * @prama data  string to print
 */
void SerialPrint(const char *data) 
{
  Serial.print(data);
  #if BLE_ENABLED
    if ( BLEconnected ) {
      mWriteChrt.writeValue((const char *)data);
      // Serial.print( " BT-cc* " ); // DEBUG
    }
  #endif
}

/** @brief print a string to serial+bluetooth, followed by CR/NL on serial
 * @prama data  string to print
 */
void SerialPrintln(const char *data)
{
  SerialPrint(data);
  Serial.print("\r\n");
}

// unnecessary: char * is promoted const char *
// //char *
// void SerialPrint(char *data)
// {
//   Serial.print(data);
//   #if BLE_ENABLED
//     if (BLEconnected) {
//       mWriteChrt.writeValue(data);
//       Serial.print( " BT-c* " );
//     }
//   #endif
// }
// 
// void SerialPrintln(char *data)
// {
//   SerialPrint(data);
//   Serial.print("\r\n");
// }

/** @brief print a character to serial+bluetooth
 * @prama data  character to print
 */
void SerialPrint(char data)
{
  Serial.print(data);
  #if BLE_ENABLED
    if (BLEconnected) {
      mWriteChrt.writeValue((const char *)data);
      // mWriteChrt.writeValue("\n");
      // Serial.print(" BT-c "); // DEBUG
    }
  #endif
}

/** @brief print a character to serial+bluetooth, followed by CR/NL on serial
 * @prama data  character to print
 */
void SerialPrintln(char data)
{
  SerialPrint(data);
  Serial.print("\r\n");
}

/** @brief @brief print a string to serial+bluetooth
 * @param data  string to print
 */
//const arduino::__FlashStringHelper *
void SerialPrint(const arduino::__FlashStringHelper *data)
{
  Serial.print(data);
  #if BLE_ENABLED
    if (BLEconnected) {
      mWriteChrt.writeValue(data);
      // mWriteChrt.writeValue("\n");
      // Serial.print( " BT-s " ); // DEBUG
    }
  #endif
}

/** @brief @brief print a string to serial+bluetooth, followed by CR/NL on serial
 * @param data  string to print
 */
void SerialPrintln(const arduino::__FlashStringHelper *data)
{
  SerialPrint(data);
  Serial.print("\r\n");
}

/** @brief @brief print an integer to serial+bluetooth
 * @param data   integer to print
 * @param format print format (for serial)
 */
//int
void SerialPrint(int data, int format)
{
  Serial.print(data, format);
  #if BLE_ENABLED
    if (BLEconnected) {
      mWriteChrt.writeValue(String(data));
      // Serial.print( " BT-if " ); // DEBUG
    }
  #endif
}

/** @brief @brief print an integer to serial+bluetooth, followed by CR/NL on serial
 * @param data   integer to print
 * @param format print format (for serial)
 */
void SerialPrintln(int data, int format)
{
  SerialPrint(data, format);
  Serial.print("\r\n");
}

/** @brief @brief print a long integer to serial+bluetooth
 * @param data   long integer to print
 */
//int
void SerialPrint(unsigned long data)
{
  Serial.print(data);
  #if BLE_ENABLED
    if (BLEconnected) {
      mWriteChrt.writeValue(String(data));
      // Serial.print( " BT-ul " ); // DEBUG
    }
  #endif
}

/** @brief @brief print a long integer to serial+bluetooth, followed by CR/NL on serial
 * @param data   long integer to print
 */
void SerialPrintln(unsigned long data)
{
  SerialPrint(data);
  Serial.print("\r\n");
}

/** @brief @brief print an integer to serial+bluetooth
 * @param data   integer to print
 */
void SerialPrint(int data)
{
  Serial.print(data);
  #if BLE_ENABLED
    if (BLEconnected) {
      mWriteChrt.writeValue(String(data));
      Serial.print( " BT-i " );
    }
  #endif
}

/** @brief @brief print an integer to serial+bluetooth, followed by CR/NL on serial
 * @param data   integer to print
 */
void SerialPrintln(int data) {
  SerialPrint(data);
  Serial.print("\r\n");
}

/** @brief @brief print a float to serial+bluetooth
 * @param data   float to print
 */
//float
void SerialPrint(float data)
{
  Serial.print(data);
  #if BLE_ENABLED
    if (BLEconnected) {
      mWriteChrt.writeValue(String(data));
      Serial.print( " BT-f " );
    }
  #endif
}

/** @brief @brief print a float to serial+bluetooth, followed by CR/NL on serial
 * @param data   float to print
 */
void SerialPrintln(float data) {
  SerialPrint(data);
  Serial.print("\r\n");
}

/** #brief read an integer from the serial line
 * @param defaultValue  default value returned if the read fails
 * @return the read value, or the default
 */
int InputIntFromSerial(unsigned int defaultValue = 0)
{
  SerialFlushAndClear();
  readSerial( true );
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

/** @brief Function to read data from Serial
 * @param wait  whether to spin-block until someting arrives
 * @return 1 if something has been read, 0 otherwise
 * @note the read string is writted in the serialBuffer
 */
static int readSerial( bool wait )
{
  static bool ready = false;  // Flag to indicate if a complete line is ready
  static int cnt = 0;         // Counter for the number of characters read
  do {
    #if BLE_ENABLED
      if ( BLEconnected ) {
        String bufferRead = readBluetooth();
        if ( bufferRead != "" ) {
          // Serial.print("BT command <"); // DEBUG
          // // Serial.print( bufferRead );
          // for ( int k = 0; k < bufferRead.length(); ++k ) Serial.print( (byte)bufferRead[k], HEX );
          // Serial.print(">\r\n");
          strcpy( serialBuffer, bufferRead.c_str() );
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
        if (c == '\n' || c == '\r' || c == '\n' || cnt == (SERIAL_BUFFER_SIZE - 1)) {
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

// ================================================================
// SENSORS

int gasDetected = 0;       // Gas detected - for LED and LCD
float PPMGas = 0;          // Gas ppm
float PPMGasOld = 0;       // Gas ppm

float rawSensorValue;
float rawSensorValueOld = 0;
int sensorReadingCounter = 0;
int sensorReading[NUM_READS];


#if SGP40_PRESENT
  #include "sensor_SGP40/SparkFun_SGP40_Arduino_Library.h"
  #include "sensor_SGP40/sensirion_voc_algorithm.h"
  #include "sensor_SGP40/SparkFun_SGP40_Arduino_Library.cpp"
  #include "sensor_SGP40/sensirion_voc_algorithm.c"
  SGP40 sgp40Sensor;  // create an object of the SGP40 class
#endif

#if BMP280_PRESENT
  #include "sensor_BMP280/farmerkeith_BMP280.h"
  bme280 bme0(0, DEBUGSENSOR);
  bool BMP280Present = false;
#endif


#if SGP30_PRESENT
  #include <SGP30.h>  // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
  SGP30 SGP;
#endif

#if MQ2SENSOR_PRESENT

  #ifdef TGS2610
    // #define RL_VALUE 4.7 // K_ohm
    // #define Rair_Ro 1.09
    int zeroGasValue = ZEROGAS_TGS;  // Define the default value for zero gas calibration

    /** @brief compute output voltage from the ADC value
     * @param adc   ADC value
     * @return input voltage
     */
    float Vout( float adc )
    {
      return (adc < 108)? 0 : (adc - 108)/3513.0;
    }

    /** @brief compute the ration of the sensor resistance in gas (Rs) to that in air (Ra)
     * @param vout  input voltage
     * @retrun ratio of sensor resistance in gas to that in air
     */
    float Rs_Ra( float vout )
    {
      return 0.066667 * ( 4.48 / vout - 1.0);
    }

    /** @brief compute the log (base 10) of PPM from the ratio of the sensor resistance
     * @param rsra  ration of sensor resistance
     * @return log base 10 of the PPM
     */
    float LogPPM( float rsra ) // log base 10 
    {
       float log_rsra = log10(rsra);
       return 1.01 - 1.794 * log_rsra;
    }

    /** @brief compute the PPM from the raw ADC value
     * @param rawValue   raw ADC value
     * @return PPM
     */
    float MQ2_RawToPPM( float rawValue )
    {
      if ( rawValue < zeroGasValue ) rawValue = zeroGasValue;
      float vout = Vout( rawValue );
      float RsRa = Rs_Ra( vout );
      // Serial.print("Rs/Ra ");
      // Serial.println( RsRa );
      float logPPM =  LogPPM( RsRa);
      float ppm = exp( log(10) * logPPM );
      return ( ppm < 11 )? 0 : ppm - 11;
    }
  #else
    int zeroGasValue = ZEROGAS_MQ2;  // Define the default value for zero gas calibration
    #define RAL_MQ2 9.83                           // Ratio of load resistance to sensor resistance in clean air
    
    /** @brief Function to calculate the sensor resistance based on the raw ADC value
     * @param adc   ADC value
     * @return sensor resistance
     */
    float MQ2_calc_res(float rawAdc)
    {
      // Calculate and return the sensor resistance
      return (float)RL_VALUE * (ADC_MAX_F - rawAdc) / rawAdc;
    }

    /** @brief Function to calculate the percentage of gas concentration
     * @param resCurrent sensor resistance
     * @param resZero    zero-value resistance
     * @param pCurve     parameters of the sensor curve
     * @return PPM
     */
    float MQ2_PPM_gas(float resCurrent, float resZero, float *pCurve)
    {
      // Calculate the ratio of current resistance to zero gas resistance
      float rs_ro_ratio = resCurrent / resZero;
      // Calculate and return the gas concentration using the calibration curve
      return pow(10, (((log10(rs_ro_ratio) - pCurve[1]) / pCurve[2]) + pCurve[0]));
    }
    
    /** @brief Function to convert raw sensor value to parts per million (PPM) for MQ2 gas sensor
     * @param adc   ADC value
     * @return PPM
     */
    float MQ2_RawToPPM(float rawValue)
    {
      // If the zero calibration value is greater than or equal to the raw value, return 0 (no gas)
      if (zeroGasValue >= rawValue) {
        return 0;
      }
      // If the zero calibration value is less than or equal to 0, set it to 1 to avoid division by zero
      if (zeroGasValue <= 0) {
        zeroGasValue = 1;
      }
    
      // Calculate the resistance of the sensor at zero gas concentration
      float ResZero = MQ2_calc_res(zeroGasValue) / RAL_MQ2;
      // Calculate the current sensor resistance
      float ResCurrent = MQ2_calc_res(rawValue);

      // Sensor characteristics for calibration
      float LPGCurve_MQ2[3] = { 2.3, 0.21, -0.47 };  // Calibration curve for LPG gas
        // Convert the percentage to PPM
      return MQ2_PPM_gas(ResCurrent, ResZero, LPGCurve_MQ2);
    }
  #endif
#endif

#if defined(ARDUINO_UNOWIFIR4)
  float mapTo_0_1023(float value)
  {
    return value * ADC_MAX_F / 4095;
  }
#endif

/** @brief Function to read an analog value from a pin with filtering
 * @param analogPin    analog pin
 * @return ???
 */
float DL_analogReadAndFilter(int analogPin)
{
  // Array to store sensorReading
  #if defined( ARDUINO_UNOWIFIR4 )
    analogReadResolution( ADC_RESOLUTION );  //  was 12: 0- 4095
  #endif
  int sum = 0;
  int minValue = ADC_MAX_F;  // Initialize minValue with the highest possible analog value
  int maxValue = 0.0;        // Initialize maxValue with the lowest possible analog value

  // Loop to read the analog value multiple times
  for (int i = 0; i < NUM_READS; i++) {
    delay(READS_DELAY);  // Delay between sensorReading for stability
    sensorReading[i] = analogRead(analogPin);
    // SerialPrint( i );
    // SerialPrint(F(": "));
    // SerialPrintln( sensorReading[i] );
    #if defined(ARDUINO_UNOWIFIR4)
      // sensorReading[i] = mapTo_0_1023(sensorReading[i]);  // Read the value from the analog pin
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
  float ret = (float)sum / (NUM_READS - 2);
  // if ( echo ) {
  //   SerialPrintln( ret );
  // }
  return ret;
}

#if LOGVCC
  long DL_readVcc()
  {
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
    uint8_t low  = ADCL;  // must read ADCL first - it then locks ADCH
    uint8_t high = ADCH;  // unlocks both
    long result = (high << 8) | low;
    result = 1125300L / result;  // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
    return result;               // Vcc in millivolts
  }
#endif


// ========================================================
// RESULT

// status
bool failed = false;

char strDate[20];        //= "0000-00-00 00:00:00";
char strFileDate[24];    // = "YYYY-MM-DD_HH.mm.ss.txt";
char delimiter[] = ";";  // CSV delimiter

const char textOk[] = "ok";
const char textFailed[] = "failed";
const char textNotFound[] = "unknown command";

// End Global variables --<
// Functions
// print ok of fail
bool printResult(bool isOk, bool ln = false)
{
  if (isOk) {
    Serial.print(textOk);
  } else {
    Serial.print(textFailed);
  }
  if (ln) {
    Serial.println();
  }
  return isOk;
}

// ========================================================
// VERSION

int8_t logfileOpened = 0;  // -1 slave/command mode, 0 closed, 1 open
File logfile;
char logfileName[32];

// Print firmware version
void printVersion()
{
  #if MQ2SENSOR_PRESENT
    #if BLE_ENABLED
      sprintf( msg, "FluxyLogger %s v. %s", BLE_NAME, VERSION_STR );
    #else
      sprintf( msg, "FluxyLogger NASO v. %s", VERSION_STR );
    #endif
  #endif
  SerialPrintln( msg );
  sprintf( msg , "Build time: %s %s", __DATE__, __TIME__ );
  SerialPrintln( msg );
  if ( logfileOpened < 0 ) {
    sprintf( msg, "Mode: slave");
  } else if ( logfileOpened == 0 ) {
    sprintf( msg, "Mode: master, No log-file");
  } else {
    sprintf( msg, "Mode: master, log-file %s", logfileName );
  }
  SerialPrintln( msg );
}

// ========================================================
// FILES and SDCARD

SdFat SDfs;
File settingFile;
bool sdPresent = false;


unsigned long log_interval_s = LOG_INTERVAL_s;  // log interval
unsigned int LogCounter = 0;  // counter
unsigned int dataToWrite = 0;

/** @brief Function to list files on the SD card on serial/bluetooth
 */
void listFiles()
{
  Serial.println( F("List Files") );
  // size log files: 2023-11-22_20.18.33.txt
  char printBuffer[25];
  SdFile file;
  SdFile dir;
  if ( ! dir.open("/", O_RDONLY) ) {
    return;
  }
  SerialPrintln( F("Files:") );
  // Open the first file in the directory
  dir.rewind();
  // Loop through all files in the directory
  while ( file.openNext( &dir, O_RDONLY ) ) {
    // Store the file name in a buffer
    file.getName( printBuffer, sizeof(printBuffer) );
    // Check if the entry is not a directory
    if ( ! file.isDir() && strlen(printBuffer) > 0 ) {
      // Print the file name
      sprintf( msg, "%s %d", printBuffer, file.fileSize() );
      SerialPrintln( msg );
      // SerialPrint(printBuffer);
      // SerialPrint("\t");
      // Print the file size
      // SerialPrintln((int)file.fileSize());
    }
    file.close();
  }
  SerialPrintln();
  dir.close();
}

/** @brief Function to handle file download requests - file is written to serial/buetooth
 */
void downloadFile()
{
  Serial.println(F("Download File"));
  // Prompt user to type a filename or 'e' to exit
  SerialPrintln(F("Type Filename or e to exit"));
  char data;
  // Infinite loop to handle user input
  while (1) {
    // Read user input from Serial
    readSerial( true );
    // If user types 'e', exit the function
    if (strcmp( serialBuffer, "e" ) == 0) {
      return;
    }

    // Check if the filename entered is longer than 4 characters
    if (strlen(serialBuffer) > 4) {
      // Attempt to open the file
      Serial.print("open file ");
      Serial.println( serialBuffer );
      File file = SDfs.open(serialBuffer);
      if (file) {
        // Notify start of transmission
        sprintf( msg, "Start transmission: %s", serialBuffer );
        SerialPrintln( msg );
        // Read and transmit file contents
        String line = "";
        while (file.available()) {
          #if BLE_ENABLED
            data = file.read();
            if ( data == '\n' ) {
              if (BLEconnected) {
                mWriteChrt.writeValue(String(line));
              } else {
                Serial.println(line );
              }
              line = "";
            } else {
              line += (char)data;
            }
          #else
            Serial.write(file.read());
          #endif
        }

        // Notify end of transmission and filename
        sprintf( msg, "End transmission: %s", serialBuffer );
        SerialPrintln( msg );
        file.close();
        return;
      } else {
        // Notify if file opening failed
        sprintf( msg, "Error opening file %s", serialBuffer );
        SerialPrintln( msg );
        return;
      }
    }
  }
}

// ========================================================
// LOGS

void ekoToSerial( const char * str )
{
  if ( echoToSerial ) {
    Serial.print(str);
  // } else {
  //   Serial.print(str);
  }
}

void ekoToSerial( const __FlashStringHelper *str )
{
  if ( echoToSerial ) {
    Serial.print(str);
  // } else {
  //   Serial.print(str);
  }
}

void LOGPRINT_NOBT(char *str)
{
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  ekoToSerial(str );
}

void LOGPRINT_NOBT(const char *str)
{
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  ekoToSerial( str );
}

void LOGPRINT_NOBT(const __FlashStringHelper *str)
{
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  ekoToSerial( str );
}

void LOGPRINT( char * str )
{
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  ekoToSerial( str );
}

void LOGPRINT( const char *str )
{
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  ekoToSerial( str );
}

void LOGPRINT(const __FlashStringHelper *str)
{
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  ekoToSerial( str );
}

/*
void LOGPRINTLN(const __FlashStringHelper *str)
{
  if (logfileOpened > 0) {
    failed = logfile.println(str) ? false : true;
    dataToWrite++;
  }
  ekoToSerial( str );
}

void LOGPRINTLN(char *str)
{
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  ekoToSerial(str);
}
*/

void LOGPRINTLN_NOBT(const char *str)
{
  if (logfileOpened > 0) {
    failed = logfile.print(str) ? false : true;
    dataToWrite++;
  }
  ekoToSerial( str );
}

void LOGPRINT(float val, uint8_t precision = 2)
{
  if (isnan(val)) {
    val = 0;
  }
  if (logfileOpened > 0) {
    char format[8];
    sprintf( format, "%%.%df", precision );
    sprintf( msg, format, val );
    logfile.print( msg );
  }
  ekoToSerial( msg );
}

void LOGPRINT(unsigned long str)
{
  if ( logfileOpened > 0 ) {
    logfile.print(str);
    dataToWrite++;
  }
  if (echoToSerial) {
    Serial.print(str);
  // } else {
  //   Serial.print(str);
  }
}

/** @brief Close the log file
 */
void DL_closeLogFile() 
{
  Serial.println( F("Close LOG file") );
  if ( logfileOpened > 0 ) {
    logfile.flush();
    logfile.close();
    logfileOpened = 0;
  }
}

/** @brief Open a log file, if the log file is not already opened
 */
void DL_openLogFile() 
{ 
  if ( logfileOpened < 0 ) {
    Serial.println( F("Open log-file: -1") );
    return;
  }
  if ( sdPresent == true ) {
    if ( logfileOpened > 0 ) {
      Serial.println( F("Reopen log-file") );
      logfile.flush();
      logfile.close();
      logfileOpened = 0;
      LogCounter = 1;
    }
    char filename[] = "xxxx-xx-xx-xx.xx.xx.txt";
    for (uint8_t i = 0; i < 9; i++) {
      sprintf(filename, DL_strDateToFilename());
      if ( ! SDfs.exists(filename) ) {
        // only open a new file if it doesn't exist
        logfile = SDfs.open(filename, FILE_WRITE);
        if (logfile) {
          strcpy( logfileName, filename );
          logfileOpened = 1;
        } else {
          logfileOpened = -1; // failure to open log-file : do not retry
        }
        sprintf( msg, "Open new log-file %s -> %d", filename, logfileOpened );
        Serial.println( msg );
        break;
      } else {
        delay(1000);
      }
    }
    if ( logfileOpened < 0 ) {
      sprintf(msg, "FAIL create %s", filename );
    } else if ( logfileOpened == 0 ) {
      sprintf(msg, "Not opened %s", filename );
    } else {
      sprintf(msg, "Log to %s", filename );
    }
    SerialPrintln( msg );
  } else {
    Serial.println(F("No SD card - Log to serial"));
  }
  LOGPRINT_NOBT(F("\"date Y-m-d m:s\""));
  #if MQ2SENSOR_PRESENT
    LOGPRINT_NOBT(delimiter);
    LOGPRINT_NOBT(F("\"gas adc\""));
    LOGPRINT_NOBT(delimiter);
    LOGPRINT_NOBT(F("\"LPG PPM\""));
  #endif
  #if SGP30_PRESENT
    LOGPRINT_NOBT(F("\"TVOC\""));
    LOGPRINT_NOBT(delimiter);
    LOGPRINT_NOBT(F("\"CO2\""));
  #endif
  #if BMP280_PRESENT
    if (BMP280Present) {
      #if HUMIDITY_PRESENT
        LOGPRINT_NOBT(delimiter);
        LOGPRINT_NOBT(F("\"temp\""));
        LOGPRINT_NOBT(delimiter);
        LOGPRINT_NOBT(F("\"pressure\""));
        LOGPRINT_NOBT(delimiter);
        LOGPRINT_NOBT(F("\"humidity\""));
      #else
        LOGPRINT_NOBT(delimiter);
        LOGPRINT_NOBT(F("\"temp\""));
        LOGPRINT_NOBT(F(\"pressure\""));
        LOGPRINT_NOBT(delimiter);
      #endif
    }
  #endif
  #if WINDSENSOR_PRESENT
    LOGPRINT_NOBT(delimiter);
    LOGPRINT_NOBT(F("\"wind\""));
    LOGPRINT_NOBT(delimiter);
    LOGPRINT_NOBT(F("\"WindTemp\""));
  #endif
  #if SGP40_PRESENT
    LOGPRINT_NOBT(delimiter);
    LOGPRINT_NOBT(F("\"VOCIndex \""));
    LOGPRINT_NOBT(delimiter);
    LOGPRINT_NOBT(F("\"VOCRaw \""));
  #endif
  #if LOGVCC
    LOGPRINT_NOBT(delimiter);
    LOGPRINT_NOBT(F("\"VCC\""));
  #endif
  LOGPRINTLN_NOBT(F(" "));
}


/** @brief switch logs: close the log file and, if required, open a new one
 */
void restartLogs( )
{
  sprintf( msg, "Switch logs - start new log" );
  Serial.println( msg );
  if ( logfileOpened > 0 ) DL_closeLogFile();
  logfileOpened = 0;
  DL_openLogFile();
}

// ===================================================
// CLOCK

unsigned long timeStartLog  = 0;
unsigned long currentMillis = 0;
unsigned long timeTarget    = 0;
unsigned long timeFileWrite = 0;

// variables for realtime clock
bool rtcPresent = false;
RTC_DS1307 RTC;  // define the Real Time Clock object
DateTime now;

/** @brief compose the date-time string
 * @return the date-time string of now
 */
static char *DL_strNow(void)
{
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

/** @brief compose a filename with the date-time string
 * @return the filename
 */
static char *DL_strDateToFilename(void)
{
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

// ===================================================================
// CONFIG

char CONF_FILE[] = "CONFIG.INI";  // configuration file

/** @brief check if a character is valid for a configuration string
 * @param   character character
 * @return true if the character is valid
 */
bool CONF_is_valid_char(char character)
{
  if (isalnum(character) || character == '_') {
    return true;
  }
  return false;
}

// log_interval_s
#define MAX_INI_KEY_LENGTH 20
#define MAX_INI_VALUE_LENGTH 5
char lineBuffer[MAX_INI_KEY_LENGTH + MAX_INI_VALUE_LENGTH + 2];  // Buffer for the line


/** @brief get a configuration value
 * @param filename configuration file (on the SD card)
 * @param key      configuration key
 * @param defaultValue default value
 * @return the value of the key, or the default value (if file or key are not found)
 */
int CONF_getConfValueInt(char *filename, char *key, int defaultValue = 0) 
{
  File myFile;
  char character;
  char description[MAX_INI_KEY_LENGTH + 2];  // Buffer for the line
  char value[MAX_INI_VALUE_LENGTH + 2];      // Buffer for the line
  bool valid = true;
  int ret = defaultValue;
  int i = 0;
  if ( ! SDfs.exists(filename)) {
    return defaultValue;
  }
  myFile = SDfs.open(filename);
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

/** @brief create a configuration file
 * @param NewInterval_s  log interval
 * @param NewZeroGasValue zero-gas value
 * @return true if success
 */
bool CreateINIConf(int NewInterval_s, int NewZeroGasValue = 0)
{
  if ( NewInterval_s < MIN_INTERVAL_S ) NewInterval_s   = log_interval_s;
  if ( NewZeroGasValue < MIN_ZEROGAS  ) NewZeroGasValue = zeroGasValue;

  SDfs.remove(CONF_FILE);
  settingFile = SDfs.open(CONF_FILE, FILE_WRITE);
  SerialPrint(F("create "));
  SerialPrintln(CONF_FILE);
  if (settingFile) {
    // create new file config.ini------>
    sprintf( msg, "Config log-interval = %d", NewInterval_s );
    Serial.println( msg );
    settingFile.print(F("log_interval_s="));
    settingFile.println(NewInterval_s);
#if MQ2SENSOR_PRESENT
    sprintf( msg, "Config zero-gas = %d", NewZeroGasValue );
    Serial.println( msg );
    settingFile.print(F("zerogas="));
    settingFile.println(NewZeroGasValue);
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

/** @brief initialize CONFIG.INI
 */
void DL_initConf()
{
  if ( sdPresent ) {
    SerialPrint( CONF_FILE );
    SerialPrint( ' ' );
    if ( ! SDfs.exists(CONF_FILE) ) {
      printResult(false, true);
      #if MQ2SENSOR_PRESENT
        CreateINIConf( LOG_INTERVAL_s, ZEROGAS_MQ2 );
      #else
        CreateINIConf( LOG_INTERVAL_s, ZEROGAS_TGS );
      #endif
    } else {
      printResult( true, true );
    }
    // read from CONFIG.INI------->
    #if MQ2SENSOR_PRESENT
      #ifndef TGS2610 
        zeroGasValue = CONF_getConfValueInt( CONF_FILE, (char *)"zerogas", (int)ZEROGAS_MQ2 );
      #else 
        zeroGasValue = CONF_getConfValueInt( CONF_FILE, (char *)"zerogas", (int)ZEROGAS_TGS );
      #endif
      sprintf( msg, "Zero-gas value = %.1f", zeroGasValue );
      SerialPrintln( msg );
    #endif
    log_interval_s = CONF_getConfValueInt( CONF_FILE, (char *)"log_interval_s", LOG_INTERVAL_s );
    sprintf(msg, "Log Interval = %d s", (int)log_interval_s );
    SerialPrintln( msg );
  } else {
    Serial.println( F("No SD card - cannot init config") );
  }
  if ( log_interval_s == 0 ) {
    log_interval_s = LOG_INTERVAL_s;
    sprintf( msg, "Force to %d s", (int)LOG_INTERVAL_s );
    SerialPrintln( msg );
  }
}

// ===========================================================
// LCD

void LCD_Print(int cursor)
{
  #if LCD_I2C_ENABLED
  #endif
  #if OLED_PRESENT
  #endif
}

// =======================================================
// COMMAND

bool plotterMode = false;  // Arduino plotter protocol

// switch device in slave mode
void switchCommandMode()
{
  if ( logfileOpened >= 0 ) {
    Serial.println( F("Switch to slave mode") );
    if ( logfileOpened > 0 ) DL_closeLogFile();
    logfileOpened = -1; 
    echoToSerial = false;
  } else {
    logfileOpened = 0;
    echoToSerial = true;
    DL_openLogFile();
  }
}

// manage commands
void execute_command(char *command)
{
  unsigned int i;
  // if (strlen(command) > 0) {
  //   switchCommandMode();
  // } else {
  //   return;
  // }
  if (strncmp(command, "?", 1) == 0) {
    SerialPrintln();
    SerialPrintln(F("commands:"));
    SerialPrintln(F("v: firmware version"));
    SerialPrintln(F("logs:read data"));
    SerialPrintln(F("date: display device clock"));
    SerialPrintln(F("reset: reset device"));
    SerialPrintln(F("settime: set device clock"));
    SerialPrintln(F("setconfig: set config"));
    SerialPrintln(F("restart: start a new log-file"));
    SerialPrintln(F("switch: switch master/slave mode"));
    // SerialPrintln(F("echo start/stop: start/stop display values"));
    // SerialPrintln(F("log start/stop: start/stop log"));
    SerialPrintln(F("plotter start/stop: start/stop plotter mode"));
#if MQ2SENSOR_PRESENT
    SerialPrintln(F("autocalib: perform auto calibration"));
#endif
    SerialPrintln();

    return;
  }

  if (strncmp(command, "v", 1) == 0) {
    printVersion();
    return;
  }
  if (strncmp(command, "date", 4) == 0) {
    printDateTime();
    return;
  }
  if (strncmp(command, "switch", 4) == 0) {
    switchCommandMode();
    return;
  }
  if (strncmp(command, "restart", 4) == 0) {
    restartLogs();
    return;
  }
  if (strncmp(command, "plotter start", 13) == 0) {
    echoToSerial = false;
    plotterMode = true;
    printResult(true, true);
    return;
  }
  if (strncmp(command, "plotter stop", 12) == 0) {
    plotterMode = false;
    printResult(true, true);
    return;
  }
  #if MQ2SENSOR_PRESENT
    if (strncmp(command, "autocalib", 9) == 0) {
      Mq2Calibration();
      return;
    }
  #endif
  if (strncmp(command, "ls", 2) == 0) {
    listFiles();
    return;
  }
  if (strncmp(command, "logs", 4) == 0) {
    // Serial.flush();
    listFiles();
    downloadFile();
    return;
  }
  if (strncmp(command, "reset", 5) == 0) {
    DL_closeLogFile();
    printResult(true, true);
    delay(500);
    reset();  // reset arduino
    return;
  }
  if (strncmp(command, "settime", 7) == 0) {
    SetDateTime();
    return;
  }
  if (strncmp(command, "setconfig", 9) == 0) {
    SetConfig();
    return;
  }
  // If user types 'rm ' followed by a filename, delete that file
  if (command[0] == 'r' && command[1] == 'm' && command[2] == ' ') {
    // Remove "rm " from the string to get the filename
    for (i = 0; i < SERIAL_BUFFER_SIZE - 3; i++) {
      command[i] = command[i + 3];
    }
    command[i] = '\0';
    // Delete the file and report status
    if ( logfileOpened > 0 && strcmp( command, logfileName ) == 0 ) {
      sprintf( msg, "Cannot delete log-file %s", command );
    } else {
      if ( SDfs.remove(command) ) {
        sprintf( msg, "deleted %s", command );
      } else {
        sprintf( msg, "FAIL delete %s", command );
      }
    }
    SerialPrintln( msg );
    return;
  }

  SerialPrint(command);
  SerialPrint(' ');
  SerialPrintln(textNotFound);
}

// parse command from serial
static int parse_serial_command()
{
  // #if BLE_ENABLED
  //   if (BLEconnected) {
  //     String commandTmp = readBluetouth();
  //     if (commandTmp != "") {
  //       execute_command((char *)commandTmp.c_str());
  //     }
  //   }
  // #endif
  if ( readSerial(false) ) {
    int k1 = strlen(serialBuffer);
    if ( k1 == 0 ) return 0;
    int k = 0;
    while ( k < k1 && isspace(serialBuffer[k]) ) ++k;
    if ( k == k1 ) return 0;
    int j = 0;
    for ( ; k < k1; ++j, ++k) serialBuffer[j] = serialBuffer[k];
    serialBuffer[j] = 0;
    sprintf( msg, "Exec command <%s>", serialBuffer );
    Serial.println( msg );
    execute_command( serialBuffer );
    return 1;
  }
  return 0;
}

#define DELTA_CALIB 3
#define NUM_CALIB 10

#if MQ2SENSOR_PRESENT
  // MQ2 sensor auto calibration
  void Mq2Calibration() 
  {
    bool echo = echoToSerial;
    echoToSerial = true;
    int timeCount;
    int S0Sensor = 0;
    int S0Sensor_old = 0;
    int NumConsecutive = 0;
    sprintf(msg, "Zero-gas calibration (with %d data)", NUM_CALIB );
    SerialPrintln( msg );
    #if LCD_I2C_ENABLED
      lcd.setCursor(0, 0);
      lcd.print(F("Calibration"));
    #endif
    float aveS0Sensor;
    while (1) {
      S0Sensor = (int)DL_analogReadAndFilter(S0);
      if ( abs( S0Sensor - S0Sensor_old ) < DELTA_CALIB ) {
        aveS0Sensor += S0Sensor; 
        NumConsecutive++;
      } else {
        S0Sensor_old = S0Sensor; // restart counting
        aveS0Sensor  = S0Sensor;
        NumConsecutive = 1;
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
      sprintf( msg, "Value %d: %d", NumConsecutive, S0Sensor);
      SerialPrintln( msg );
      // SerialPrint(F("Value:"));
      // SerialPrintln(S0Sensor);
      #if LCD_I2C_ENABLED
        lcd.setCursor(0, 10);
        lcd.print(S0Sensor);
      #endif
      if ( NumConsecutive >= NUM_CALIB ) {
        zeroGasValue = aveS0Sensor / NumConsecutive + 1;
        sprintf( msg, "Completed: zero-gas %d", zeroGasValue );
        SerialPrintln( msg );
        CreateINIConf((unsigned int)log_interval_s, zeroGasValue);
        sprintf( msg, "Log interval = %d s", log_interval_s );
        Serial.println( msg );
        return;
      }
    }
    echoToSerial = echo;
  }
#endif

// manage config
void SetConfig() 
{
  Serial.println( F("Set Config") );
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
    if (zerogas <= 0 || zerogas > ADC_MAX) {
      zerogas = zeroGasValue;
    }
  #else
    zerogas = ZEROGAS_MQ2;
  #endif
  if (CreateINIConf(interval, zerogas)) {
    delay(500);
    reset();  // reset arduino
  } else {
    SerialPrintln(textFailed);
  }
}

// Set date and time
void SetDateTime() 
{
  Serial.println( F("Set Date And Time") );
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
void reset() 
{
  Serial.println( F("Reset") );
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
void SerialFlushAndClear()
{
  Serial.flush();
  while (Serial.available() > 0) {
    Serial.read();
  }
}
// manages the flashing time depending on the ppm
void manageBlinking(unsigned long timeOn, unsigned long timeOff)
{
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
void printDateTime()
{
  if (RTC.isrunning()) {
    SerialPrint(F("Device clock:"));
    SerialPrintln(DL_strNow());
  } else {
    SerialPrintln(F("FAIL print date and time"));
  }
}

// manages the flashing time depending on the ppm
void manageBlinkingByPPM(float inputValue)
{
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
  void GestLcd() 
  {
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

// =========================================================
// SETUP

void setup() 
{
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
  Serial.println( F("Setup") );
  //--init serial------------------------------<
  sdPresent = false;
  rtcPresent = false;
  logfileOpened = 0; // -1 slave; 0 closed; 1 open
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
  if ( ! SDfs.begin(CHIP_SD) ) {
    sdPresent = false;
    Serial.println( F("FAIL SD card") );
    // printResult(false, true);
    failed = true;
  } else {
    Serial.println( F("Init SD card") );
    // printResult(true, true);
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
  
  //-----------init analog input-----------------------------------------<
  // pinMode(LED_BUILTIN, OUTPUT);

  // return;

  #if OLED_PRESENT
    oled.begin(width, height, sizeof(tiny4koled_init_128x64br), tiny4koled_init_128x64br);
    oled.clear();
    oled.setFont(FONT6X8);
    oled.on();
    // oled.setCursor(0, 0);
    // oled.println( F("Start log") );
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
    initBluetooth();
  #endif

  currentMillis = millis();
  timeTarget    = currentMillis + (log_interval_s * 1000);
  timeFileWrite = currentMillis + SYNC_INTERVAL_ms;
  timeStartLog  = currentMillis + (1000 * MQ2_PREHEAT_TIME_S);
}

bool On1Seconds = false;
bool On10Seconds = false;

void updateTimers() 
{
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

// =========================================================
// LOOP

void loop()
{
  static float adcTotal = 0;
  static unsigned int adcCount = 0;

  static unsigned long ledTimer = 0;
  static bool readyToMeasure = false;
  static unsigned long lastMillis = 0; // last time of reading
  float S0Sensor = 0.0;

  currentMillis = millis();
  // commands:
  parse_serial_command();

  updateTimers();
  if (On1Seconds) {
    #if BLE_ENABLED
      //    Serial.println( F("handle Bluetooth") );
      handleBluetooth();
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
  if ( ! readyToMeasure ) {
    if ( currentMillis > timeStartLog || plotterMode || logfileOpened == -1 ) {
      Serial.println( F("Ready to measure") );
      readyToMeasure = true;
    }
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
        Serial.println( F("Preheating") );
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
      if ( currentMillis > lastMillis + READING_INTERVAL ) {
        lastMillis = currentMillis;
        rawSensorValue = DL_analogReadAndFilter(S0);
        adcTotal += rawSensorValue;
        adcCount++;  
        PPMGas = MQ2_RawToPPM(rawSensorValue);
        if (plotterMode) {
          sprintf( msg, "raw: %.2f ppm: %.2f", rawSensorValue, PPMGas);
          SerialPrintln( msg );
          // SerialPrint(F("raw:"));
          // SerialPrint(rawSensorValue);
          // SerialPrint(F(",ppm:"));
          // SerialPrintln(PPMGas);
        }
        #if LCD_I2C_ENABLED
          if ( gasDetected >= MINCONSECUTIVE_POSITIVE ) {
            lcd.setCursor(14, 0);  // col row
            lcd.print( gasDetected );
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

    if ( currentMillis < timeTarget ) {
      sprintf( msg, "current time %d target %d", currentMillis, timeTarget );
    } else {
      timeTarget = currentMillis + log_interval_s * 1000;
      
      if ( logfileOpened == 0 || (LogCounter % MAX_ROWS_PER_FILE) == 0 ) {
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
      // sprintf( msg, "Log %d: %d %.2f", LogCounter, currentMillis, S0Sensor);
      // Serial.println( msg );

      #if OLED_PRESENT
        oled.clear();
      #endif

      #if LOGVCC
        vcc = DL_readVcc();
      #endif
      //-----------sensors-------------------------------------------------------<
      LogCounter++;
      LOGPRINT_NOBT(F("\""));
      LOGPRINT(DL_strNow());
      LOGPRINT_NOBT(F("\""));
      #if MQ2SENSOR_PRESENT
        LOGPRINT_NOBT(delimiter);
        LOGPRINT(S0Sensor);
        #if OLED_PRESENT
          oled.setCursor(0, 0);
          oled.print(S0Sensor);
        #endif
        LOGPRINT_NOBT(delimiter);
        PPMGas = MQ2_RawToPPM(S0Sensor);
        LOGPRINT(PPMGas);
        if (PPMGas >= MINPPMPOSITIVE) {
          // if ( gasDetected < MINCONSECUTIVE_POSITIVE ) {
          gasDetected++;
          //}
        } else {
          if ( gasDetected < MINCONSECUTIVE_POSITIVE ) {
            gasDetected = 0;
          }
        }
        #if OLED_PRESENT
          oled.setCursor(0, 10);
          oled.print(MQ2_RawToPPM(S0Sensor), 0);
        #endif
      #endif
      #if BMP280_PRESENT
        if (BMP280Present) {
          temperature = bme0.readTemperature();
          pressure = bme0.readPressure();
          LOGPRINT_NOBT(delimiter);
          LOGPRINT(temperature);
          LOGPRINT_NOBT(delimiter);
          LOGPRINT(pressure, 0);
          #if HUMIDITY_PRESENT
            humidity = bme0.readHumidity();
            LOGPRINT_NOBT(delimiter);
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
        LOGPRINT_NOBT(delimiter);
        float Wind_mts = WindSpeed_MPH * 0.44704;
        LOGPRINT(Wind_mts, 5);  // m/s
        LOGPRINT_NOBT(delimiter);
        LOGPRINT((TempCtimes100 / 100));  // m/s
      #endif
      #if SGP40_PRESENT
        LOGPRINT_NOBT(delimiter);
        LOGPRINT(sgp40Sensor.getVOCindex(), 2);
        LOGPRINT_NOBT(delimiter);
        LOGPRINT(sgp40Sensor.getRaw(), 2);
      #endif
      #if SGP30_PRESENT
        SGP.measure(false);  // returns false if no measurement is made
        LOGPRINT_NOBT(delimiter);
        LOGPRINT((float)SGP.getTVOC());
        LOGPRINT_NOBT(delimiter);
        SerialPrint((float)SGP.getCO2());
      #endif
      #if LOGVCC
        LOGPRINT_NOBT(delimiter);
        LOGPRINT(String(vcc));
      #endif
      LOGPRINT_NOBT(F("\n"));
    }
    if ( dataToWrite != 0 && currentMillis >= timeFileWrite ) {
      if ( logfileOpened > 0 ) {
        // Serial.println( "Flush log-file" );
        logfile.flush();
        // SerialPrintln("flush");
      }
      dataToWrite = 0;
      timeFileWrite = currentMillis + SYNC_INTERVAL_ms;
    }
  }
  #if LCD_I2C_ENABLED
    GestLcd();
  #endif
}
