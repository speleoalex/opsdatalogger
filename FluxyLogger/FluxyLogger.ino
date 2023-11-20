/**
   FluxyLogger Datalogger - OpenSpeleo Project
   Author: Alessandro Vernassa (speleoalex@gmail.com)
   Copyright: 2021
   License: GNU General Public License
   Product link: https://techmakers.eu/

   This sketch is designed for the N.A.S.O. datalogger, available as a pre-assembled unit or kit.
*/

// Sensor presence configuration
#define BMP280_PRESENT 0     // Set to 1 if BMP280 sensor is present, 0 otherwise
#define HUMIDITY_PRESENT 0   // Set to 1 if a humidity sensor is present, 0 otherwise
#define DEBUGSENSOR 0        // Enable (1) or disable (0) sensor debugging
#define WINDSENSOR_PRESENT 0 // Set to 1 if a wind sensor is present, 0 otherwise

// Calibration and operation parameters
#define MINCONSECUTIVE_POSITIVE 20 // Minimum number of consecutive positive readings
#define MQ2_PREHEAT_TIME_S 30      // Preheat time in seconds for the MQ2 sensor
#define MINPPMPOSITIVE 10          // Minimum PPM for a positive reading

#define WINDSENSOR_PRESENT 0
#define MQ2SENSOR_PRESENT 1 // VOC MQ2 analogic
#define GAS_PPM 1
#define SGP40_PRESENT 0 // VOC SGP40 i2c
#define SGP30_PRESENT 0 // VOC SGP30 i2c
#define OLED_PRESENT 0  // OLED
#define size_serialBuffer 40
#define LOGVCC 0

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
#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#endif

#if OLED_PRESENT
#include <Tiny4kOLED.h>
uint8_t width = 128;
uint8_t height = 64;
#endif

int Detected = false;
char serialBuffer[size_serialBuffer];

#if SGP40_PRESENT
#include "SparkFun_SGP40_Arduino_Library.h"
SGP40 sgp40Sensor; // create an object of the SGP40 class
#endif

#if BMP280_PRESENT
#include "farmerkeith_BMP280.h"
bme280 bme0(0, DEBUGSENSOR);
#endif

bool plottermode = false;

#define ZEROGAS_default 115
#if MQ2SENSOR_PRESENT
int zerogasValue = ZEROGAS_default;
#define RL_VALOR 5.0
float MQ2_RawToPPM(int RawValue)
{
  if (zerogasValue >= RawValue)
  {
    return 0;
  }
  if (zerogasValue <= 0)
  {
    zerogasValue = 1;
  }
  float RAL(9.83);
  float LPGCurve[3] = {2.3, 0.21, -0.47};
  float ResZero = MQ2_calc_res(zerogasValue) / RAL;
  float ResCurrent = MQ2_calc_res(RawValue);
  float Perc = MQ2_Perc_gas(ResCurrent, ResZero, LPGCurve);
  return 1000 * Perc;
}

float MQ2_Perc_gas(float ResCurrent, float ResZero, float *pcurve)
{
  float rs_ro_ratio = ResCurrent / ResZero;
  return (pow(10, (((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0])));
}
float MQ2_calc_res(float raw_adc)
{
  return (((float)RL_VALOR * (1023.0 - raw_adc) / raw_adc));
}
#endif

#if WINDSENSOR_PRESENT
#define analogPinForRV A1 // change to pins you the analog pins are using
#define analogPinForTMP A2

#endif

/*
  Datalogger use 6 pin. Analog 4 and 5 for I2C.
  SD card use i digital pin 13, 12, 11, and 10.
  0=seriale rx
  1=seriale tx
*/

#define S0 A0 // AIR

#define LED_1 3 // Led 1 datalogger
#define LED_2 4 // Led 2 datalogger

#define MAX_ROWS_PER_FILE 10000 // max rows per file
#define LOG_INTERVAL_s 15       // log interval in seconds

#define CHIP_SD 10             // pin sd
#define SYNC_INTERVAL_ms 20000 // mills between calls to flush() - to write data to the card

#define NUM_READS 25
#define READS_DELAY 20

#define RXLED 17
#define TXLED 30

//-------------globals--------------------->
const char CONF_FILE [] =  "CONFIG.INI"; // configuration file
const char ok[]  = "ok";

File logfile;
File settingFile;
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

char strDate[20] ;//= "0000-00-00 00:00:00";
char strFileDate[24];// = "YYYY-MM-DD_HH.mm.ss.txt";
SdFat SD;

unsigned long PPMGas;

//-------------globals---------------------<

File root;

void listFiles()
{
  root = SD.open("/");
  Serial.println("Files:");
  File entry = root.openNextFile();
  while (entry)
  {
    
    entry.getName(printBuffer, sizeof(printBuffer));
    if (!entry.isDirectory())
    {
      Serial.print(printBuffer);
      Serial.print('\t');
      Serial.println((unsigned long)entry.size());
    }
    entry = root.openNextFile();
  }
}
void downloadFile()
{
  Serial.println(F("Type Filename or e to exit"));
  while (1)
  {
    readSerial(true);
    if (strcmp(serialBuffer, "e") == 0)
    {
      return;
    }
    if (serialBuffer[0] == 'r' && serialBuffer[1] == 'm' && serialBuffer[2] == ' ')
    {
      // Rimuove "rm " dalla stringa per ottenere il nome del file
      int i;
      for (i = 0; i < size_serialBuffer - 3; i++)
      {
        serialBuffer[i] = serialBuffer[i + 3];
      }
      serialBuffer[i] = "\0";
      Serial.print(F("del "));
      Serial.println(serialBuffer);
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
    if (strlen(serialBuffer) > 4)
    {
      File file = SD.open(serialBuffer);
      if (file)
      {
        Serial.println(F("Start trasmission:"));
        while (file.available())
        {
          Serial.write(file.read());
        }
        Serial.print(F("End trasmission:"));
        Serial.println(serialBuffer);
        file.close();
        return;
      }
      else
      {
        Serial.println(F("error opening file."));
        return;
      }
    }
  }
}

float DL_analogReadAndFilter(int analogPin)
{
  int readings[NUM_READS];
  int sum = 0;
  int minValue = 1024;
  int maxValue = 0;
  // Leggi il pin NUM_READS volte
  for (int i = 0; i < NUM_READS; i++)
  {
    delay(READS_DELAY);
    readings[i] = analogRead(analogPin);
    // find min value
    if (readings[i] < minValue)
    {
      minValue = readings[i];
    }
    if (readings[i] > maxValue)
    {
      maxValue = readings[i];
    }

    sum += readings[i];
  }
  // remove min e max value
  sum = sum - minValue - maxValue;
  return (float)sum / (NUM_READS - 2);
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
uint8_t InputIntFromSerial(uint8_t defaultValue = 0)
{
  readSerial(true);
  if (strlen(serialBuffer) > 0)
  {
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
    //DateTime now;
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
  //  LOGPRINT(F("\"count\"\t\"date Y-m-d m:s\""));
  LOGPRINT(F("\"date Y-m-d m:s\""));

#if MQ2SENSOR_PRESENT
  LOGPRINT(F("\t\"gas adc\""));
  LOGPRINT(F("\t\"LPG PPM\""));
#endif

#if BMP280_PRESENT
  if (BMP280Present)
  {
#if HUMIDITY_PRESENT
    LOGPRINT(F("\t\"temp\"\t\"pressure\"\t\"humidity\""));
#else
    LOGPRINT(F("\t\"temp\"\t\"pressure\""));
#endif
  }
#endif
#if WINDSENSOR_PRESENT
  LOGPRINT(F("\t\"wind\""));
  LOGPRINT(F("\t\"WindTemp\""));
#endif
#if SGP40_PRESENT
  LOGPRINT(F("\t\"VOCIndex \""));
  LOGPRINT(F("\t\"VOCRaw \""));
#endif

#if LOGVCC
  LOGPRINT(F("\t\"VCC\""));
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
#define MAX_KEY_LENGTH 10
#define MAX_VALUE_LENGTH 10
char lineBuffer[MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 2]; // Buffer for the line

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
static int CONF_getConfValueInt(char *filename, char *key, int defaultValue = 0) {
  File myFile;
  char character;
  String description = "";
  String value = "";
  boolean valid = true;
  int ret = defaultValue;
  if (!SD.exists(filename)) {
    return defaultValue;
  }
  myFile = SD.open(filename);
  while (myFile.available()) {
    description = "";
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
      description.concat(character);
      character = myFile.read();
      if (!myFile.available()) {
        myFile.close();
        return defaultValue;
      }
    } while (CONF_is_valid_char(character));
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
      if (description == key) {
        //-------elimino gli spazi------->
        do {
          character = myFile.read();
          if (!myFile.available()) {
            myFile.close();
            return defaultValue;
          }
        } while (character == ' ');
        //-------elimino gli spazi-------<
        value = "";
        valid = true;
        while (character != '\n' && character != '\r') {
          // Serial.println(character);
          if (isdigit(character)) {
            value.concat(character);
          } else if (character != '\n' && character != '\r') {
            // Use of invalid values
            valid = false;
          }
          character = myFile.read();
        };
        if (valid) {
          // Convert string to array of chars
          char charBuf[value.length() + 1];
          value.toCharArray(charBuf, value.length() + 1);
          // Convert chars to integer
          ret = atoi(charBuf);
          myFile.close();
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
#endif
bool CreateINIConf(int NewInterval_s, int NewzerogasValue = 0)
{
  SD.remove(CONF_FILE);
  settingFile = SD.open(CONF_FILE, FILE_WRITE);
  if (settingFile)
  {
    // create new file config.ini------>
    Serial.print(F("new file "));
    Serial.println(CONF_FILE);
    settingFile.print(F("log_interval_s="));
    settingFile.println(NewInterval_s);
#if MQ2SENSOR_PRESENT
    settingFile.print(F("zerogas="));
    settingFile.println(NewzerogasValue);
#endif
    settingFile.flush();
    settingFile.close();
    // create new file config.ini------<
  }
  else
  {
    Serial.print(F("error create "));
    Serial.println(CONF_FILE);
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
    if (!SD.exists(CONF_FILE))
    {
      Serial.print(CONF_FILE);
      Serial.println(F(" not exist"));
#if MQ2SENSOR_PRESENT
      CreateINIConf(LOG_INTERVAL_s, ZEROGAS_default);
#else
      CreateINIConf(LOG_INTERVAL_s);
#endif
    }
    else
    {
      Serial.print(CONF_FILE);
      Serial.println(F(" exist"));
    }
    // read from CONFIG.INI------->
    log_interval_s = CONF_getConfValueInt(CONF_FILE, "log_interval_s", LOG_INTERVAL_s);
#if MQ2SENSOR_PRESENT
    zerogasValue = CONF_getConfValueInt(CONF_FILE, "zerogas", ZEROGAS_default);
    Serial.print(F("zerogas="));
    Serial.println(zerogasValue);
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
void setup()
{
  PPMGas = 0;
  //uint8_t n = 0;
  int timeout = 5;
  //--init serial------------------------------>
  Serial.begin(115200); // 19200 boud
  //--init serial------------------------------<
  sdPresent = false;
  rtcPresent = false;
  logfileOpened = false;
  Serial.println(F("Aerologger NASO"));
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
  Serial.print(F("SD initialization:"));
  if (!SD.begin(CHIP_SD))
  {
    sdPresent = false;
    Serial.println(F("failed"));
    failed = true;
  }
  else
  {
    Serial.println(ok);
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
 **/
static int readSerial(bool wait)
{
  static bool ready = false;
  static int cnt = 0;
  do
  {
    while (Serial.available() > 0)
    {
      if (ready)
      {
        ready = false;
        return 1;
      }
      char c = Serial.read();
      if (c == '\n' || c == '\r' || (c >= 20 && c <= 126))
      {
        if (cnt == 0 && (c == '\n' || c == '\r'))
        {
          continue;
        }
        else
        {
          serialBuffer[cnt] = c;
          if (c == '\n' || c == '\r' || cnt == (size_serialBuffer - 1))
          {
            serialBuffer[cnt] = '\0';
            cnt = 0;
            ready = true;
          }
          else
          {
            cnt++;
          }
        }
      }
    }
  } while (wait);
  return 0;
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
/**

*/
static bool isValidNumber(char *str)
{
  for (byte i = 0; i < strlen(str); i++)
  {
    if (str[i] == '\0')
    {
      break;
    }
    if (!(isDigit(str[i]) || str[i] == '.'))
      return false;
  }
  return true;
}
/**

*/
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
    Serial.println(ok);
    DL_closeLogFile();
    logfileOpened = -1;
  }
  if (strcmp(command, "log start") == 0)
  {
    Serial.println(ok);
    DL_closeLogFile();
    logfileOpened = 0;
    DL_openLogFile();
  }

  if (strcmp(command, "echo stop") == 0)
  {
    echoToSerial = false;
    Serial.println(ok);
  }
  if (strcmp(command, "echo start") == 0)
  {
    echoToSerial = true;
    Serial.println(ok);
  }
  if (strcmp(command, "plotter on") == 0)
  {
    DL_closeLogFile();
    logfileOpened = -1;
    echoToSerial = false;
    plottermode = true;
    Serial.println(ok);
    return;
  }
  if (strcmp(command, "plotter off") == 0)
  {
    echoToSerial = true;
    plottermode = false;
    Serial.println(ok);
    return;
  }

#if MQ2SENSOR_PRESENT
  if (strcmp(command, "autocalib") == 0)
  {
    Mq2Calibration();
    return;
  }
#endif
  if (strcmp(command, "logs") == 0)
  {
    DL_closeLogFile();
    logfileOpened = -1;
    File root = SD.open("/");
    Serial.flush();
    listFiles();
    downloadFile();
    return;
  }

  if (strcmp(command, "reset") == 0)
  {
    DL_closeLogFile();
    Serial.println(ok);
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
      zerogasValue = S0Sensor + 1;
      Serial.println(F("completed"));
      CreateINIConf(log_interval_s, zerogasValue);
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
  if (interval <= 0)
  {
    interval = log_interval_s;
  }
#if MQ2SENSOR_PRESENT
  zerogas = zerogasValue;
  Serial.print(F("\nZerogas:"));
  Serial.print('(');
  Serial.print(zerogasValue);
  Serial.print(')');
  zerogas = InputIntFromSerial(zerogasValue);
  if (zerogas <= 0 || zerogas > 1023)
  {
    zerogas = zerogasValue;
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
/**

*/
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
void reset()
{
  asm volatile("  jmp 0");
}
/**


*/
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
float ppm;
int raw;
/**
   Loop.
*/
void loop()
{
  static unsigned long LedTimer = 0;
  static bool LogReady = false;
  static unsigned long TimeTargetRead = 0;
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
  float RV_Wind_ADunits;       // RV output from wind sensor
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

  if (!logfileOpened)
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
    if (TimeCurrent > TimeTargetRead + 1000)
    {
      raw = analogRead(S0);
      ppm = MQ2_RawToPPM(raw);
      TimeTargetRead = TimeCurrent;
      if (plottermode)
      {
        Serial.print(F("raw="));
        Serial.print(raw);
        Serial.print(F(",ppm="));
        Serial.println(ppm);
      }
      manageBlinkingByPPM(ppm);
    }

    if (TimeCurrent >= TimeTarget)
    {
      TimeTarget = TimeCurrent + (log_interval_s * 1000);
      if (LogCounter == 0 || (LogCounter >= MAX_ROWS_PER_FILE && (LogCounter % MAX_ROWS_PER_FILE == 0)))
      {
        DL_openLogFile();
      }
      //-----------sensors------------------------------------------------------->
      S0Sensor = DL_analogReadAndFilter(S0);

#if OLED_PRESENT
      oled.clear();
#endif

#if LOGVCC
      vcc = DL_readVcc();
#endif
      //-----------sensors-------------------------------------------------------<
      LogCounter++;
      // LOGPRINT(String(LogCounter));
      // LOGPRINT(F("\t"));
      LOGPRINT(F("\""));
      LOGPRINT(DL_strNow());
      LOGPRINT(F("\""));
#if MQ2SENSOR_PRESENT
      LOGPRINT(F("\t"));
      LOGPRINT(S0Sensor);

#if OLED_PRESENT
      oled.setCursor(0, 0);
      oled.print(S0Sensor);
#endif

      LOGPRINT(F("\t"));
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
        LOGPRINT(F("\t"));
        LOGPRINT(temperature);
        LOGPRINT(F("\t"));
        LOGPRINT(pressure, 0);
#if HUMIDITY_PRESENT
        humidity = bme0.readHumidity();
        LOGPRINT(F("\t"));
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
      LOGPRINT(F("\t"));
      float Wind_mts = WindSpeed_MPH * 0.44704;
      LOGPRINT(Wind_mts, 5); // m/s
      LOGPRINT(F("\t"));
      LOGPRINT((TempCtimes100 / 100)); // m/s
#endif

#if SGP40_PRESENT
      LOGPRINT(F("\t"));
      LOGPRINT(sgp40Sensor.getVOCindex(), 2);
      LOGPRINT(F("\t"));
      LOGPRINT(sgp40Sensor.getRaw(), 2);

#endif

#if LOGVCC

      LOGPRINT(F("\t"));
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
