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

// Sensor presence configuration
#define BMP280_PRESENT 0   // Set to 1 if BMP280 sensor is present, 0 otherwise
#define HUMIDITY_PRESENT 0 // Set to 1 if a humidity sensor is present, 0 otherwise
#define DEBUGSENSOR 0      // Enable (1) or disable (0) sensor debugging
// Calibration and operation parameters
#define MINCONSECUTIVE_POSITIVE 20 // Minimum number of consecutive positive readings
#define MQ2_PREHEAT_TIME_S 30      // Preheat time in seconds for the MQ2 sensor
#define MINPPMPOSITIVE 10          // Minimum PPM for a positive reading

// Sensors
#define WINDSENSOR_PRESENT 0
#define MQ2SENSOR_PRESENT 1 // VOC MQ2 analogic
#define GAS_PPM 1
#define SGP40_PRESENT 0 // VOC SGP40 i2c
#define SGP30_PRESENT 0 // VOC SGP30 i2c
#define OLED_PRESENT 0  // OLED
#define LOGVCC 0

// Pins
#define S0 A0      // AIR
#define LED_1 3    // Led 1 datalogger
#define LED_2 4    // Led 2 datalogger
#define CHIP_SD 10 // pin sd
#define RXLED 17
#define TXLED 30
#if WINDSENSOR_PRESENT
#define analogPinForRV A1 // change to pins you the analog pins are using
#define analogPinForTMP A2
#endif

// Buffer sizes
#define size_serialBuffer 40
#define MAX_ROWS_PER_FILE 10000 // max rows per file
#define MAX_INI_KEY_LENGTH 10
#define MAX_INI_VALUE_LENGTH 10

// Log configs
#define LOG_INTERVAL_s 15      // log interval in seconds
#define SYNC_INTERVAL_ms 20000 // mills between calls to flush() - to write data to the card
#define NUM_READS 25
#define READS_DELAY 20
#define ZEROGAS_default 115
#define RL_VALUE 5.0 // Define the load resistance value in kilo ohms

// Libraries
#define USE_FAT_FILE_FLAG_CONTIGUOUS 0
#define USE_LONG_FILE_NAMES 0
#define USE_UTF8_LONG_NAMES 0
#define SDFAT_FILE_TYPE 1
#define CHECK_FLASH_PROGRAMMING 1 // May cause SD to sleep at high current.

#include <SdFatConfig.h>
#include <SdFat.h>
#include <Wire.h>
#include <RTClib.h>

#if SGP30_PRESENT
#include <SGP30.h> // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#endif

#if OLED_PRESENT
#include <Tiny4kOLED.h>
uint8_t width = 128;
uint8_t height = 64;
#endif

#if SGP40_PRESENT
#include "sensor_SGP40/SparkFun_SGP40_Arduino_Library.h"
SGP40 sgp40Sensor; // create an object of the SGP40 class
#endif

#if BMP280_PRESENT
#include "sensor_BMP280/farmerkeith_BMP280.h"
bme280 bme0(0, DEBUGSENSOR);
#endif

// Start Global variables -->
int Detected = false;
char serialBuffer[size_serialBuffer];
bool plottermode = false;
#if MQ2SENSOR_PRESENT
int zeroGasValue = ZEROGAS_default; // Define the default value for zero gas calibration
#endif
#if SGP30_PRESENT
SGP30 SGP;
#endif

char lineBuffer[MAX_INI_KEY_LENGTH + MAX_INI_VALUE_LENGTH + 2]; // Buffer for the line
const char CONF_FILE[] = "CONFIG.INI";                          // configuration file
const char textOk[] = "ok";
const char textFailed[] = "failed";


// bool ledInternal = false;
bool sdPresent = false;
bool rtcPresent = false;
int8_t logfileOpened = 0;
bool echoToSerial = true;
bool failed = false;
bool BMP280Present = false;
uint8_t log_interval_s = LOG_INTERVAL_s; // log interval

unsigned long TimeStartLog = 0;
unsigned long TimeCurrent = 0;
unsigned long TimeTarget = 0;
unsigned long TimeTargetFileWrite = 0;

int Led2TimeOn = 0;
int Led2TimeOff = 0;

unsigned long LogCounter = 0; // counter
unsigned long dataToWrite = 0;
RTC_DS1307 RTC; // define the Real Time Clock object
DateTime now;
char printBuffer[32];
char strDate[20];       //= "0000-00-00 00:00:00";
char strFileDate[24];   // = "YYYY-MM-DD_HH.mm.ss.txt";
unsigned long PPMGas;   // Gas ppm
char delimiter[] = ";"; // CSV delimiter
SdFat SD;
File logfile;
File settingFile;

int rawSensorValue;
int sensorReading[NUM_READS];
uint8_t sensorReadingCounter = 0;
// End Global variables --<

// Functions
// print ok of fail
bool printResult(bool isOk, bool ln = false)
{
  if (isOk)
  {
    Serial.print(textOk);
  }
  else
  {
    Serial.print(textFailed);
  }
  if (ln)
  {
    Serial.println();
  }
  return isOk;
}

#if MQ2SENSOR_PRESENT
// Function to convert raw sensor value to parts per million (PPM) for MQ2 gas sensor
float MQ2_RawToPPM(float rawValue)
{
  // If the zero calibration value is greater than or equal to the raw value, return 0 (no gas)
  if (zeroGasValue >= rawValue)
  {
    return 0;
  }

  // If the zero calibration value is less than or equal to 0, set it to 1 to avoid division by zero
  if (zeroGasValue <= 0)
  {
    zeroGasValue = 1;
  }

  // Sensor characteristics for calibration
  float RAL = 9.83;                       // Ratio of load resistance to sensor resistance in clean air
  float LPGCurve[3] = {2.3, 0.21, -0.47}; // Calibration curve for LPG gas

  // Calculate the resistance of the sensor at zero gas concentration
  float ResZero = MQ2_calc_res(zeroGasValue) / RAL;

  // Calculate the current sensor resistance
  float ResCurrent = MQ2_calc_res(rawValue);

  // Calculate the gas concentration percentage
  float Perc = MQ2_Perc_gas(ResCurrent, ResZero, LPGCurve);

  // Convert the percentage to PPM
  return 1000 * Perc;
}

// Function to calculate the percentage of gas concentration
float MQ2_Perc_gas(float resCurrent, float resZero, float *pCurve)
{
  // Calculate the ratio of current resistance to zero gas resistance
  float rs_ro_ratio = resCurrent / resZero;

  // Calculate and return the gas concentration using the calibration curve
  return (pow(10, (((log(rs_ro_ratio) - pCurve[1]) / pCurve[2]) + pCurve[0])));
}

// Function to calculate the sensor resistance based on the raw ADC value
float MQ2_calc_res(float rawAdc)
{
  // Calculate and return the sensor resistance
  return (((float)RL_VALUE * (1023.0 - rawAdc) / rawAdc));
}
#endif

void printDirectory(const char *dirname, int levels)
{
  SdFile file;
  SdFile dir;

  // Apri la directory corrente
  if (!dir.open(dirname))
  {
    Serial.print("Errore nell'apertura della directory ");
    Serial.println(dirname);
    return;
  }

  // Leggi ogni file/directory nella cartella
  while (file.openNext(&dir, O_RDONLY))
  {
    // Stampa il nome del file/directory
    file.printName(&Serial);
    Serial.println();
    file.close();
  }
  dir.close();
}

// Function to list files on the SD card
void listFiles()
{
  SdFile file;
  SdFile dir;
  if (!dir.open("/", O_RDONLY))
  {
    return;
  }
  Serial.println("Files:");
  // Open the first file in the directory
  dir.rewind();
  // Loop through all files in the directory
  while (file.openNext(&dir, O_RDONLY))
  {
    // Store the file name in a buffer
    file.getName(printBuffer, sizeof(printBuffer));
    // Check if the entry is not a directory
    if (!file.isDir())
    {
      // Print the file name
      Serial.print(printBuffer);
      Serial.print("\t");
      // Print the file size
      Serial.println((unsigned long)file.fileSize());
    }
    file.close();
  }
  dir.close();
}

// Function to handle file download requests
void downloadFile()
{
  // Prompt user to type a filename or 'e' to exit
  Serial.println(F("Type Filename or e to exit"));

  // Infinite loop to handle user input
  while (1)
  {
    // Read user input from Serial
    readSerial(true);

    // If user types 'e', exit the function
    if (strcmp(serialBuffer, "e") == 0)
    {
      return;
    }

    // If user types 'rm ' followed by a filename, delete that file
    if (serialBuffer[0] == 'r' && serialBuffer[1] == 'm' && serialBuffer[2] == ' ')
    {
      // Remove "rm " from the string to get the filename
      int i;
      for (i = 0; i < size_serialBuffer - 3; i++)
      {
        serialBuffer[i] = serialBuffer[i + 3];
      }
      serialBuffer[i] = '\0';

      // Print delete command and filename
      Serial.print(F("del "));
      Serial.println(serialBuffer);

      // Delete the file and report status
      if (SD.remove(serialBuffer))
      {
        Serial.println(F("file deleted"));
      }
      else
      {
        Serial.print(F("error remove "));
        Serial.println(serialBuffer);
      }
      return;
    }

    // Check if the filename entered is longer than 4 characters
    if (strlen(serialBuffer) > 4)
    {
      // Attempt to open the file
      File file = SD.open(serialBuffer);
      if (file)
      {
        // Notify start of transmission
        Serial.println(F("Start transmission:"));

        // Read and transmit file contents
        while (file.available())
        {
          Serial.write(file.read());
        }

        // Notify end of transmission and filename
        Serial.print(F("End transmission:"));
        Serial.println(serialBuffer);
        file.close();
        return;
      }
      else
      {
        // Notify if file opening failed
        Serial.println(F("error opening file."));
        return;
      }
    }
  }
}

// Function to read an analog value from a pin with filtering
float DL_FilterSensorReading(void)
{
  // Array to store sensorReading

  int sum = 0.0;
  int minValue = 1023; // Initialize minValue with the highest possible analog value
  int maxValue = 0.0;  // Initialize maxValue with the lowest possible analog value

  // Loop to read the analog value multiple times
  for (int i = 0; i < NUM_READS; i++)
  {
    delay(READS_DELAY); // Delay between sensorReading for stability
    // Find the minimum value among the sensorReading
    if (sensorReading[i] < minValue)
    {
      minValue = sensorReading[i];
    }
    // Find the maximum value among the sensorReading
    if (sensorReading[i] > maxValue)
    {
      maxValue = sensorReading[i];
    }

    sum += sensorReading[i]; // Sum up all the sensorReading
  }

  // Remove the minimum and maximum value from the sum for filtering
  sum = sum - minValue - maxValue;

  // Return the average value excluding the min and max values
  return (float)sum / (NUM_READS - 2);
}

// Function to read an analog value from a pin with filtering
float DL_analogReadAndFilter(int analogPin)
{
  // Array to store sensorReading

  int sum = 0;
  int minValue = 1023; // Initialize minValue with the highest possible analog value
  int maxValue = 0;    // Initialize maxValue with the lowest possible analog value

  // Loop to read the analog value multiple times
  for (int i = 0; i < NUM_READS; i++)
  {
    delay(READS_DELAY);                       // Delay between sensorReading for stability
    sensorReading[i] = analogRead(analogPin); // Read the value from the analog pin

    // Find the minimum value among the sensorReading
    if (sensorReading[i] < minValue)
    {
      minValue = sensorReading[i];
    }

    // Find the maximum value among the sensorReading
    if (sensorReading[i] > maxValue)
    {
      maxValue = sensorReading[i];
    }

    sum += sensorReading[i]; // Sum up all the sensorReading
  }

  // Remove the minimum and maximum value from the sum for filtering
  sum = sum - minValue - maxValue;

  // Return the average value excluding the min and max values
  return (float)sum / (NUM_READS - 2);
}

void LOGPRINT(char *str)
{
  if (logfileOpened > 0)
  {
    logfile.print(str);
    dataToWrite++;
  }
  if (echoToSerial)
  {
    Serial.print(str);
  }
}

void LOGPRINT(const __FlashStringHelper *str)
{
  if (logfileOpened > 0)
  {
    logfile.print(str);
    dataToWrite++;
  }
  if (echoToSerial)
  {
    Serial.print(str);
  }
}

void LOGPRINTLN(const __FlashStringHelper *str)
{
  if (logfileOpened > 0)
  {
    logfile.println(str);
    dataToWrite++;
  }
  if (echoToSerial)
  {
    Serial.println(str);
  }
}

void LOGPRINT(float val, uint8_t precision = 2)
{
  if (isnan(val))
  {
    val = 0;
  }
  if (val < 0.0)
  {
    if (logfileOpened > 0)
      logfile.print('-');
    if (echoToSerial)
      Serial.print('-');
    val = -val;
  }
  val = round(val * pow(10, precision));
  val = val / pow(10, precision);
  if (echoToSerial)
  {
    Serial.print(int(val)); // prints the int part
  }
  if (logfileOpened > 0)
    logfile.print(int(val), DEC);
  if (precision > 0)
  {
    unsigned long frac;
    unsigned long mult = 1;
    byte padding = precision - 1;
    while (precision--)
      mult *= 10;
    if (val >= 0)
      frac = (val - int(val)) * mult;
    else
      frac = (int(val) - val) * mult;

    if (frac > 0)
    {
      if (logfileOpened > 0)
        logfile.print('.');
      if (echoToSerial)
        Serial.print('.');
      unsigned long frac1 = frac;
      while (frac1 /= 10)
        padding--;
      while (padding--)
      {
        if (logfileOpened > 0)
          logfile.print('0');
        if (echoToSerial)
          Serial.print('0');
      }
      if (logfileOpened > 0)
        logfile.print(frac, DEC);
      if (echoToSerial)
        Serial.print(frac, DEC);
    }
  }
}

void LOGPRINT(unsigned long str)
{
  if (logfileOpened > 0)
  {
    logfile.print(str);
    dataToWrite++;
  }
  if (echoToSerial)
  {
    Serial.print(str);
  }
}

/**
 *
 */
int InputIntFromSerial(uint8_t defaultValue = 0)
{
  SerialFlush();
  readSerial(true);
  if (strlen(serialBuffer) > 0)
  {
    /*
    Serial.print("read:");
    Serial.println(serialBuffer);
    Serial.print("atoi:");
    Serial.println(atoi(serialBuffer));
    */
    return atoi(serialBuffer);
  }
  else
  {
    return defaultValue;
  }
}

/**

*/
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
  delay(2);            // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC))
    ;                  // measuring
  uint8_t low = ADCL;  // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  long result = (high << 8) | low;
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result;              // Vcc in millivolts
}
#endif

/**

*/
static char *DL_strNow(void)
{
  if (rtcPresent)
  {
    // DateTime now;
    now = RTC.now();
    sprintf(strDate, "%4d-%2d-%2d_%2d:%2d:%2d", now.year(), now.month(),
            now.day(), now.hour(), now.minute(), now.second());
    for (unsigned int i = 0; i < strlen(strDate) && i < 18; i++)
    {
      if (strDate[i] == ' ')
      {
        strDate[i] = '0';
      }
      else
      {
        if (strDate[i] == '_')
        {
          strDate[i] = ' ';
        }
      }
    }
  }
  return strDate;
}

/**

*/
static char *DL_strDateToFilename(void)
{
  //  DateTime now;
  now = RTC.now();
  sprintf(strFileDate, "%4d-%2d-%2d_%2d.%2d.%2d.txt", now.year(), now.month(),
          now.day(), now.hour(), now.minute(), now.second());
  for (unsigned int i = 0; i < strlen(strFileDate) && i < 18; i++)
  {
    if (strFileDate[i] == ' ')
    {
      strFileDate[i] = '0';
    }
  }
  return strFileDate;
}
/**


*/
void DL_closeLogFile()
{
  if (logfileOpened > 0)
  {
    logfile.flush();
    logfile.close();
    logfileOpened = 0;
  }
}

/**

*/
void DL_openLogFile()
{
  if (logfileOpened < 0)
  {
    return;
  }
  if (sdPresent == true)
  {
    if (logfileOpened > 0)
    {
      logfile.flush();
      logfile.close();
      logfileOpened = 0;
      LogCounter = 1;
    }
    char filename[] = "xxxx-xx-xx-xx.xx.xx.txt";
    sprintf(filename, DL_strDateToFilename());
    for (uint8_t i = 0; i < 9; i++)
    {
      if (!SD.exists(filename))
      {
        // only open a new file if it doesn't exist
        logfile = SD.open(filename, FILE_WRITE);
        if (logfile)
        {
          logfileOpened = 1;
        }
        else
        {
          logfileOpened = 0;
        }
        break;
      }
      else
      {
        delay(1000);
        sprintf(filename, DL_strDateToFilename());
      }
    }

    if (!logfileOpened)
    {
      Serial.print(F("Couldnt create:"));
      Serial.println(filename);
    }
    else
    {
      Serial.print(F("Log to:"));
      Serial.println(filename);
    }
  }
  else
  {
    Serial.println(F("Log:serial"));
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
  if (BMP280Present)
  {
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
static bool CONF_is_valid_char(char character)
{
  if (isalnum(character) || character == '_')
  {
    return true;
  }
  return false;
}

#if 1
/**
 * Get an integer configuration value from a file.
 *
 * @param filename The name of the configuration file.
 * @param key The key for which the value is to be fetched.
 * @param defaultValue The default value to return if the key is not found.
 * @return The integer value associated with the key, or the default value.
 */
static uint8_t CONF_getConfValueInt(const char *filename, const char *key, uint8_t defaultValue = 0)
{
  File myFile;
  uint8_t ret = defaultValue;
  if (!SD.exists(filename))
  {
    return defaultValue;
  }

  myFile = SD.open(filename);
  if (!myFile)
  {
    return defaultValue;
  }

  while (myFile.available())
  {
    // Read a line into the buffer
    size_t len = myFile.readBytesUntil('\n', lineBuffer, sizeof(lineBuffer) - 1);
    lineBuffer[len] = '\0'; // Null-terminate the string
    // Ignore empty lines or lines that do not contain '='
    if (len == 0 || strchr(lineBuffer, '=') == NULL)
    {
      continue;
    }

    // Split the line into key and value
    char *token = strtok(lineBuffer, "=");
    if (token != NULL && strcmp(token, key) == 0)
    {
      token = strtok(NULL, "=");
      if (token != NULL)
      {
        // Check if the value is a valid integer
        char *endptr;
        long val = strtol(token, &endptr, 10);
        if (*endptr == '\0' || *endptr == '\n' || *endptr == '\r')
        {
          ret = (int)val;
          break;
        }
      }
    }
  }
  myFile.close();
  return ret;
}
#else
static int CONF_getConfValueInt(char *filename, char *key, int defaultValue = 0)
{
  File myFile;
  char character;
  String description = "";
  String value = "";
  boolean valid = true;
  int ret = defaultValue;
  if (!SD.exists(filename))
  {
    return defaultValue;
  }
  myFile = SD.open(filename);
  while (myFile.available())
  {
    description = "";
    character = myFile.read();
    // cerca una linea valida----->
    if (!CONF_is_valid_char(character))
    {
      // Comment - ignore this line
      while (character != '\n' && myFile.available())
      {
        character = myFile.read();
      };
      continue;
    }
    // cerca una linea valida-----<
    //----riempo la descrizione---->
    do
    {
      description.concat(character);
      character = myFile.read();
      if (!myFile.available())
      {
        myFile.close();
        return defaultValue;
      }
    } while (CONF_is_valid_char(character));
    //----riempo la descrizione----<
    //-------elimino gli spazi------->
    if (character == ' ')
    {
      do
      {
        character = myFile.read();
        if (!myFile.available())
        {
          myFile.close();
          return defaultValue;
        }
      } while (character == ' ');
    }
    //-------elimino gli spazi-------<
    if (character == '=')
    {
      if (description == key)
      {
        //-------elimino gli spazi------->
        do
        {
          character = myFile.read();
          if (!myFile.available())
          {
            myFile.close();
            return defaultValue;
          }
        } while (character == ' ');
        //-------elimino gli spazi-------<
        value = "";
        valid = true;
        while (character != '\n' && character != '\r')
        {
          // Serial.println(character);
          if (isdigit(character))
          {
            value.concat(character);
          }
          else if (character != '\n' && character != '\r')
          {
            // Use of invalid values
            valid = false;
          }
          character = myFile.read();
        };
        if (valid)
        {
          // Convert string to array of chars
          char charBuf[value.length() + 1];
          value.toCharArray(charBuf, value.length() + 1);
          // Convert chars to integer
          ret = atoi(charBuf);
          myFile.close();
          return ret;
        }
      }
    }
    else
    {
      while (character != '\n' && myFile.available())
      {
        character = myFile.read();
      };
      continue;
    }
  }
  myFile.close();
  return ret;
}
#endif
bool CreateINIConf(int NewInterval_s, int NewzeroGasValue = 0)
{
  SD.remove(CONF_FILE);
  settingFile = SD.open(CONF_FILE, FILE_WRITE);
  Serial.print(F("create "));
  Serial.println(CONF_FILE);
  if (settingFile)
  {
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
  }
  else
  {
    printResult(false, true);
    return false;
  }
  return true;
}

/**

*/
void DL_initConf()
{

  if (sdPresent)
  {
    Serial.print(CONF_FILE);
    Serial.print(' ');
    if (!SD.exists(CONF_FILE))
    {
      printResult(false, true);
#if MQ2SENSOR_PRESENT
      CreateINIConf(LOG_INTERVAL_s, ZEROGAS_default);
#else
      CreateINIConf(LOG_INTERVAL_s);
#endif
    }
    else
    {
      printResult(true, true);
    }
    // read from CONFIG.INI------->
    log_interval_s = CONF_getConfValueInt(CONF_FILE, "log_interval_s", LOG_INTERVAL_s);
#if MQ2SENSOR_PRESENT
    zeroGasValue = CONF_getConfValueInt(CONF_FILE, "zerogas", ZEROGAS_default);
    Serial.print(F("zerogas="));
    Serial.println(zeroGasValue);
#endif
    // read from CONFIG.INI-------<
    Serial.print(F("Interval(s)="));
    Serial.println(log_interval_s);
  }
  if (log_interval_s == 0)
  {
    log_interval_s = LOG_INTERVAL_s;
    Serial.print(F("Force to "));
    Serial.println(LOG_INTERVAL_s);
  }
}

/**
 **/
// Function to read data from Serial
static int readSerial(bool wait)
{
  static bool ready = false; // Flag to indicate if a complete line is ready
  static int cnt = 0;        // Counter for the number of characters read
  do
  {
    // Loop while there is data available on Serial
    while (Serial.available() > 0)
    {
      // Check if a complete line is ready
      if (ready)
      {
        ready = false;
        return 1; // Return 1 to indicate a complete line is ready
      }

      // Read a character from Serial
      char c = Serial.read();
      // Check if the character is a newline, carriage return, or printable ASCII character
      if (c == '\n' || c == '\r' || (c >= 20 && c <= 126))
      {
        // Store the character in the buffer
        serialBuffer[cnt] = c;
        // Check if the character is a newline, carriage return, or if the buffer is full
        if (c == '\n' || c == '\r' || c == '\n' || cnt == (size_serialBuffer - 1))
        {
          // Null-terminate the string and reset the counter
          serialBuffer[cnt] = '\0';
          cnt = 0;
          ready = true;
        }
        else
        {
          // Increment the counter
          cnt++;
        }
      }
    }
  } while (wait); // Continue if 'wait' is true

  return 0; // Return 0 to indicate no complete line is ready
}

/**

*/
static int parse_serial_command()
{
  if (readSerial(false))
  {
    execute_command(serialBuffer);
    return 1;
  }
  return 0;
}
// switch logs
bool SwithLogs(bool LogStart)
{
  if (LogStart)
  {
    DL_closeLogFile();
    logfileOpened = 0;
    DL_openLogFile();
  }
  else
  {
    DL_closeLogFile();
    logfileOpened = -1;
  }
  return true;
}

void execute_command(char *command)
{

  if (strcmp(command, "help") == 0)
  {
    Serial.println();
    Serial.println(F("commands:"));
    Serial.println(F("logs:read data"));
    Serial.println(F("reset:reset device"));
    Serial.println(F("settime:set device clock"));
    Serial.println(F("setconfig:set config"));
    Serial.println(F("echo start/stop: start/stop display values"));
    Serial.println(F("log start/stop: start/stop log"));
    Serial.println(F("plotter start/stop: start/stop plotter mode"));
#if MQ2SENSOR_PRESENT
    Serial.println(F("autocalib:auto calibration MQ2 sensor"));
#endif
    Serial.println();

    return;
  }

  if (strcmp(command, "log stop") == 0)
  {
    printResult(SwithLogs(false), true);
  }
  if (strcmp(command, "log start") == 0)
  {
    printResult(SwithLogs(true), true);
  }

  if (strcmp(command, "echo stop") == 0)
  {
    echoToSerial = false;
    printResult(true, true);
  }
  if (strcmp(command, "echo start") == 0)
  {
    echoToSerial = true;
    printResult(true, true);
  }
  if (strcmp(command, "plotter start") == 0)
  {
    DL_closeLogFile();
    logfileOpened = -1;
    echoToSerial = false;
    plottermode = true;
    printResult(true, true);
    return;
  }
  if (strcmp(command, "plotter stop") == 0)
  {
    echoToSerial = true;
    plottermode = false;
    printResult(true, true);
    return;
  }

#if MQ2SENSOR_PRESENT
  if (strcmp(command, "autocalib") == 0)
  {
    Mq2Calibration();
    return;
  }
#endif
  if (strcmp(command, "ls") == 0)
  {
    DL_closeLogFile();
    listFiles();
  }
  if (strcmp(command, "logs") == 0)
  {
    DL_closeLogFile();
    logfileOpened = -1;
    Serial.flush();
    listFiles();
    downloadFile();
    return;
  }

  if (strcmp(command, "reset") == 0)
  {
    DL_closeLogFile();
    printResult(true, true);
    reset(); // reset arduino
    return;
  }
  if (strcmp(command, "settime") == 0)
  {
    SetDateTime();
    return;
  }
  if (strcmp(command, "setconfig") == 0)
  {
    SetConfig();
    return;
  }
}

#if MQ2SENSOR_PRESENT
void Mq2Calibration()
{
  int timeCount;
  int S0Sensor = 0;
  int S0Sensor_old = 0;
  int NumConsecutive = 0;
  Serial.print(F("Zerogas calibration"));
  while (1)
  {
    S0Sensor = (int)DL_analogReadAndFilter(S0);
    if (S0Sensor == S0Sensor_old)
    {
      NumConsecutive++;
    }
    else
    {
      S0Sensor_old = S0Sensor;
      NumConsecutive = 0;
    }

    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    // blinking and parse command
    for (timeCount = 0; timeCount < 100; timeCount++)
    {
      if (parse_serial_command())
      {
        digitalWrite(LED_1, LOW);
        digitalWrite(LED_2, LOW);
        return;
      }
      delay(50);
    }
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    Serial.print(F("Value:"));
    Serial.println(S0Sensor);
    if (NumConsecutive > 10)
    {
      zeroGasValue = S0Sensor + 1;
      Serial.println(F("completed"));
      CreateINIConf(log_interval_s, zeroGasValue);
      return;
    }
  }
}

#endif

/**

*/
void SetConfig()
{
  DL_closeLogFile();
  delay(500);
  SerialFlush();
  int interval, zerogas;
  Serial.print(F("\nInterval(s):"));
  Serial.print('(');
  Serial.print(log_interval_s);
  Serial.print(')');
  interval = InputIntFromSerial(log_interval_s);
  Serial.println();
  if (interval <= 0)
  {
    interval = log_interval_s;
  }
#if MQ2SENSOR_PRESENT
  zerogas = zeroGasValue;
  Serial.print(F("Zerogas:"));
  Serial.print('(');
  Serial.print(zeroGasValue);
  Serial.print(')');
  zerogas = InputIntFromSerial(zeroGasValue);
  Serial.println();
  if (zerogas <= 0 || zerogas > 1023)
  {
    zerogas = zeroGasValue;
  }
#else
  zerogas = ZEROGAS_default;
#endif
  if (interval > 0)
  {
    if (CreateINIConf(interval, zerogas))
    {
      delay(500);
      reset(); // reset arduino
    }
  }
  else
  {
    Serial.println(F("\nInvalid values"));
  }
}
// Set date and time
void SetDateTime()
{
  DL_closeLogFile();
  delay(500);
  SerialFlush();
  int year, month, day, hours, minutes;
  Serial.println(F("\nClock:"));
  Serial.print(F("Year:"));
  year = InputIntFromSerial();
  Serial.println(year);
  Serial.print(F("Month:"));
  month = InputIntFromSerial();
  Serial.println(month);
  Serial.print(F("Day:"));
  day = InputIntFromSerial();
  Serial.println(day);
  Serial.print(F("Hours:"));
  hours = InputIntFromSerial();
  Serial.println(hours);
  Serial.print(F("Minutes:"));
  minutes = InputIntFromSerial();
  Serial.println(minutes);
  RTC.adjust(DateTime(year, month, day, hours, minutes, 0));
  delay(500);
  reset();
}
// Reset Arduino
void reset()
{
  asm volatile("  jmp 0");
}

void SerialFlush()
{
  Serial.flush();
  while (Serial.available() > 0)
  {
    Serial.read();
  }
}

void manageBlinking(unsigned long timeOn, unsigned long timeOff)
{
  if (failed)
  {
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    return;
  }

  static bool ledState = false;
  static unsigned long previousMillis = 0;
  if (timeOn <= 0 && timeOff <= 0)
  {
    // Se entrambi i tempi sono 0, spegni il LED e esci dalla funzione
    digitalWrite(LED_2, LOW);
    return;
  }
  // Ottieni il tempo attuale
  unsigned long currentMillis = millis();

  if (ledState)
  {
    // Se il LED è acceso, controlla se è ora di spegnerlo
    if (currentMillis - previousMillis >= timeOn)
    {
      ledState = false;
      previousMillis = currentMillis;
      digitalWrite(LED_2, LOW);
    }
  }
  else
  {
    // Se il LED è spento, controlla se è ora di accenderlo
    if (currentMillis - previousMillis >= timeOff)
    {
      ledState = true;
      previousMillis = currentMillis;
      digitalWrite(LED_2, HIGH);
    }
  }
}

void manageBlinkingByPPM(int inputValue)
{
  uint16_t MaxValue = 500;
  // Assicurati che il valore di input sia entro i limiti previsti
  if (inputValue > MaxValue || inputValue < 0)
  {
    inputValue = MaxValue;
  }
  inputValue = constrain(inputValue, 0, MaxValue);
  if (inputValue == 0)
  {
    manageBlinking(0, 0);
    return;
  }
  unsigned long time = map(inputValue, 1, MaxValue, 2000, 100);
  // Calcola timeOn e timeOff come metà del tempo totale del ciclo
  unsigned long timeOn = time / 2;
  unsigned long timeOff = time / 2;
  manageBlinking(timeOn, timeOff);
}

// Setup
void setup()
{
  PPMGas = 0;
  // uint8_t n = 0;
  int timeout = 5;
  //--init serial------------------------------>
  Serial.begin(115200); // 19200 boud
  //--init serial------------------------------<
  sdPresent = false;
  rtcPresent = false;
  logfileOpened = false;
  Serial.print(F("FluxyLogger"));
#if MQ2SENSOR_PRESENT
  Serial.print(F(" NASO"));
#endif
  Serial.println();
  Serial.print(F("Build time: "));
  Serial.print(F(__DATE__));
  Serial.print(F(" "));
  Serial.println(F(__TIME__));
  // initialize the SD card ------------------->
  pinMode(CHIP_SD, OUTPUT);
  // pinMode(S0_S, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
//  digitalWrite(S0_S, HIGH);
#if SGP30_PRESENT
  Serial.print(F("init SGP30 "));
  if (!SGP.begin())
  {
    Serial.println(F("failed"));
  }
  else
  {
    Serial.println(ok);
  }
#endif

  Serial.print(F("init SD "));
  if (!SD.begin(CHIP_SD))
  {
    sdPresent = false;
    printResult(false, true);
    failed = true;
  }
  else
  {
    printResult(true, true);
    sdPresent = true;
  }
  if (failed)
  {
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
  }

  DL_initConf();
  // initialize the SD card -------------------<
  SerialFlush();
  // connect to RTC --------------------------->
  Wire.begin();
  unsigned long unixtime;
  unixtime = DateTime(__DATE__, __TIME__).unixtime();
  if (RTC.begin())
  {
    Serial.println(F("RTC present"));
    rtcPresent = true;
    if (!RTC.isrunning())
    {
      Serial.println(F("Clock not running"));
      RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    Serial.print(F("Device clock:"));
    Serial.print(DL_strNow());
    Serial.println(F(" type 'settime' to change"));
  }
  else
  {
    rtcPresent = false;
    Serial.println(F("RTC Failed"));
    failed = true;
  }
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_1, LOW);

  // connect to RTC ---------------------------<

#if SGP40_PRESENT
  if (sgp40Sensor.begin() == true)
  {
    Serial.println(F("SGP40 detected"));
  }
#endif
#if BMP280_PRESENT
  if (!bme0.begin())
  {
    Serial.println(F("\nBMP280 sensor not present"));
    BMP280Present = false;
  }
  else
  {
    Serial.println(F("\nBMP280 sensor present"));
    BMP280Present = true;
  }
#endif
  delay(10);
  TimeCurrent = millis();
  TimeTarget = TimeCurrent + (log_interval_s * 1000);
  TimeTargetFileWrite = TimeCurrent + SYNC_INTERVAL_ms;
  //---faccio una lettura a vuoto per scaricare il condensatore -------->
  for (int i = 0; i < 10; i++)
  {
    DL_analogReadAndFilter(S0);
    delay(20);
  }
  //---faccio una lettura a vuoto per scaricare il condensatore --------<
  //-----------init analog input-----------------------------------------<
  pinMode(LED_BUILTIN, OUTPUT);

#if OLED_PRESENT

  oled.begin(width, height, sizeof(tiny4koled_init_128x64br), tiny4koled_init_128x64br);
  oled.clear();
  oled.setFont(FONT6X8);
  oled.on();
  // oled.setCursor(0, 0);
  // oled.println(F("Start log"));
#endif
  TimeStartLog = millis() + (1000 * MQ2_PREHEAT_TIME_S);
}

/**
   Loop.
*/
void loop()
{
  static unsigned long LedTimer = 0;
  static bool LogReady = false;
  static unsigned long TimeTargetContinuousReading = 0;
  float S0Sensor = 0.0;

  TimeCurrent = millis();

  // commands:
  parse_serial_command();
#if HUMIDITY_PRESENT
  float humidity = 0;
#endif
  float pressure = 0;
  float temperature = 0;
#if LOGVCC
  long vcc = 0;
#endif
#if WINDSENSOR_PRESENT
#define zeroWindAdjustment 0.2; // negative numbers yield smaller wind speeds and vice versa.
  int TMP_Therm_ADunits;        // temp termistor value from wind sensor
  float RV_Wind_ADunits;        // RV output from wind sensor
  float RV_Wind_Volts;
  int TempCtimes100;
  float zeroWind_ADunits;
  float zeroWind_volts;
  float WindSpeed_MPH;
#endif

  if (Detected >= MINCONSECUTIVE_POSITIVE)
  {
    digitalWrite(LED_1, HIGH);
  }
  else
  {
    digitalWrite(LED_1, LOW);
  }
  if (TimeCurrent > TimeStartLog || plottermode || logfileOpened == -1)
  {
    LogReady = true;
  }
  else
  {
    if (failed)
    {
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, HIGH);
    }
    else
    {
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
      if (echoToSerial)
      {
        Serial.println(F("Prehead"));
      }
    }
  }

  if (logfileOpened <= 0)
  {
    if (TimeCurrent > LedTimer + 1000)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
      LedTimer = TimeCurrent;
    }
    if (TimeCurrent > LedTimer + 500)
    {
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  if (LogReady)
  {

    if (logfileOpened <= 0)
    {
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, HIGH);
    }

    if (TimeCurrent > TimeTargetContinuousReading + 1000)
    {
      rawSensorValue = analogRead(S0);
      if (sensorReadingCounter >= NUM_READS)
      {
        sensorReadingCounter = 0;
      }
      sensorReading[sensorReadingCounter++] = rawSensorValue;
      PPMGas = round(MQ2_RawToPPM(rawSensorValue));
      TimeTargetContinuousReading = TimeCurrent;
      if (plottermode)
      {
        Serial.print(F("raw:"));
        Serial.print(rawSensorValue);
        Serial.print(F(",ppm:"));
        Serial.println(PPMGas);
      }
      manageBlinkingByPPM(PPMGas);
    }

    if (TimeCurrent >= TimeTarget)
    {
      TimeTarget = TimeCurrent + (log_interval_s * 1000);
      if (LogCounter == 0 || (LogCounter >= MAX_ROWS_PER_FILE && (LogCounter % MAX_ROWS_PER_FILE == 0)))
      {
        DL_openLogFile();
      }
      //-----------sensors------------------------------------------------------->
      S0Sensor = DL_FilterSensorReading();

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
      if (PPMGas > MINPPMPOSITIVE)
      {
        if (Detected < MINCONSECUTIVE_POSITIVE)
        {
          Detected++;
        }
      }
      else
      {
        if (Detected < MINCONSECUTIVE_POSITIVE)
        {
          Detected = 0;
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
      if (BMP280Present)
      {
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
      zeroWind_ADunits = -0.0006 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172; //  13.0C  553  482.39
      zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;
      // This from a regression from data in the form of
      // Vraw = V0 + b * WindSpeed ^ c
      // V0 is zero wind at a particular temperature
      // The constants b and c were determined by some Excel wrangling with the solver.
      WindSpeed_MPH = pow(((RV_Wind_Volts - zeroWind_volts) / 0.2300), 2.7265);
      /*
        Serial.print(F("  TMP volts "));
        Serial.print(TMP_Therm_ADunits * 0.0048828125);
        Serial.print(F(" RV volts "));
        Serial.print((float)RV_Wind_Volts);
        Serial.print(F("\t  TempC*100 "));
        Serial.print(TempCtimes100 );
        Serial.print(F("   ZeroWind volts "));
        Serial.print(zeroWind_volts);
        Serial.print(F("   WindSpeed MPH "));
        Serial.println((float)WindSpeed_MPH);
      */
      LOGPRINT(delimiter);
      float Wind_mts = WindSpeed_MPH * 0.44704;
      LOGPRINT(Wind_mts, 5); // m/s
      LOGPRINT(delimiter);
      LOGPRINT((TempCtimes100 / 100)); // m/s
#endif

#if SGP40_PRESENT
      LOGPRINT(delimiter);
      LOGPRINT(sgp40Sensor.getVOCindex(), 2);
      LOGPRINT(delimiter);
      LOGPRINT(sgp40Sensor.getRaw(), 2);

#endif

#if SGP30_PRESENT
      SGP.measure(false); // returns false if no measurement is made
      LOGPRINT(delimiter);
      LOGPRINT((float)SGP.getTVOC());
      LOGPRINT(delimiter);
      Serial.print((float)SGP.getCO2());
#endif

#if LOGVCC

      LOGPRINT(delimiter);
      LOGPRINT(String(vcc));
#endif
      LOGPRINT(F("\n"));
    }
    if (dataToWrite != 0 && TimeCurrent >= TimeTargetFileWrite)
    {
      if (logfileOpened)
      {
        logfile.flush();
      }
      dataToWrite = 0;
      TimeTargetFileWrite = TimeCurrent + SYNC_INTERVAL_ms;
    }
  }
}