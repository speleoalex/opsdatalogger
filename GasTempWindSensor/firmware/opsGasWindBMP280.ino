/**
 * @package Openspeleo datalogger
 * @author Alessandro Vernassa <speleoalex@gmail.com>
 * @copyright Copyright (c) 2021
 * @license http://opensource.org/licenses/gpl-license.php GNU General Public License
 */
#define BMP280_PRESENT 1
#define WINDSENSOR_PRESENT 0


#include <SdFat.h>
#include <MinimumSerial.h>
#include <SdFatConfig.h>
#include <Wire.h>
#include <RTClib.h>


#ifdef BMP280_PRESENT
#include "farmerkeith_BMP280.h"
bme280 bme0(0, 0);
#endif



#if WINDSENSOR_PRESENT
#define analogPinForRV    A1   // change to pins you the analog pins are using
#define analogPinForTMP   A2
/*
const float zeroWindAdjustment =  .2; // negative numbers yield smaller wind speeds and vice versa.
int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;    //RV output from wind sensor
float RV_Wind_Volts;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
float WindSpeed_MPH;
*/

#endif

/*
Il Datalogger utilizza 6 pin. Analog 4 e 5 sono per I2C. 
La scheda SD utilizza i pin digitali 13, 12, 11, e 10. 
I primi tre sono praticamente obbligatori. 
Se davvero hai bisogno del pin 10, e' possibile modificare il 
file di intestazione della libreria e cambiarla dal pin 10 al pin 
qualsiasi altro. Ma e' necessario avere il pin 10 come uscita, 
se e' usato come un ingresso, l'interfaccia SD non funzionera'. 
I sensori sono collegati su A0,A1,2,3
Led rosso:8
Led verde:9
0=seriale rx
1=seriale tx

 */

#define S0 A0					  //AIR


#define EXTERNAL_VOLTAGE_REF 0  		  //mettere a 1 se 3.2 e' collegato ad aref
#define INTERNAL_VOLTAGE_REF 0			  //set 1 for internal voltage
#define redLEDpin 8				  //pin red led
#define greenLEDpin 9				  //pin green led
#define MAX_ROWS_PER_FILE 10000			  //max rows per file
#define CONF_FILE "CONFIG.INI"  		  //configuration file
#define LOG_INTERVAL_ms 15000     		  //log interval in mmilliseconds

#define CHIP_SD 10				  //pin sd
#define SYNC_INTERVAL_ms 20000			  //mills between calls to flush() - to write data to the card


#define NUM_READS 100
#define siza_buffer 60

#define RXLED 17
#define TXLED 30
//--------------macro---------------------->
//#define LOGSTRING(str) if (logfileOpened){logfile.print(str);dataToWrite++;}//if(echoToSerial){Serial.print(str);}//if(lcdPresent){lcd.print(str);}
//#define LOGSTRINGLN(str) if (logfileOpened){logfile.println(str);dataToWrite++;}//if(echoToSerial){Serial.println(str);}//if(lcdPresent){lcd.println(str);}
//--------------macro----------------------<

//-------------globals--------------------->



static File logfile;
static File settingFile;
static bool sdPresent = false;
static bool rtcPresent = false;
static bool logfileOpened = false;
static bool echoToSerial = true;
static bool failed = false;
static bool BMP280Present = false;



static unsigned int log_interval_ms = LOG_INTERVAL_ms; //log interval

static unsigned long TimeCurrent = 0;
static unsigned long TimeTarget = 0;
static unsigned long TimeTargetFileWrite = 0;

static unsigned long LogCounter = 0; //counter
static unsigned long dataToWrite = 0;
static RTC_DS1307 RTC; // define the Real Time Clock object

char strDate[] = "0000-00-00 00:00:00";
char strFileDate[] = "YYYY-MM-DD_HH.mm.ss.csv";
SdFat SD;
//-------------globals---------------------<

void LOGSTRING(String str) {
    if (logfileOpened) {
        logfile.print(str);
        dataToWrite++;
    }
    if (echoToSerial) {
        Serial.print(str);
    }
}

void LOGSTRING(double str) {
    if (logfileOpened) {
        logfile.print(str);
        dataToWrite++;
    }
    if (echoToSerial) {
        Serial.print(str);
    }
}

void LOGSTRINGLN(String str) {
    if (logfileOpened) {
        logfile.print(str);
        dataToWrite++;
    }
    if (echoToSerial) {
        Serial.println(str);
    }

}
/**
 *
 */
long DL_readVcc() {
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
#else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif
    delay(2); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA, ADSC))
    ; // measuring
    uint8_t low = ADCL; // must read ADCL first - it then locks ADCH
    uint8_t high = ADCH; // unlocks both
    long result = (high << 8) | low;
    result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
    return result; // Vcc in millivolts
}

/**
 * 
 */
static void DL_fixStrFilename(char *str) {
    for (unsigned int i = 0; i < strlen(str) && i < 7; i++) {
        if (str[i] == ' ') {
            str[i] = '0';
        }
    }
}

/**
 *
 */
static char* DL_strNow(void) {

    if (rtcPresent) {
        DateTime  now;
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
 *
 */
static char* DL_strDateToFilename(void) {
    DateTime  now;
    now = RTC.now();
    sprintf(strFileDate, "%4d-%2d-%2d_%2d.%2d.%2d.csv", now.year(), now.month(),
            now.day(), now.hour(), now.minute(), now.second());
    for (unsigned int i = 0; i < strlen(strFileDate) && i < 18; i++) {
        if (strFileDate[i] == ' ') {
            strFileDate[i] = '0';
        }
    }
    return strFileDate;
}
/** 
 *  
 *  
 */
static void DL_closeLogFile() {
    if (logfileOpened) {
        logfile.flush();
        logfile.close();
        logfileOpened = false;
    }
}

/**
 *
 */
static void DL_openLogFile() {
    if (sdPresent == true) {
        if (logfileOpened) {
            logfile.flush();
            logfile.close();
            logfileOpened = false;
            LogCounter = 1;
        }
        char filename[] = "xxxx-xx-xx-xx.xx.xx.csv";

        sprintf(filename, DL_strDateToFilename());
        for (uint8_t i = 0; i < 9; i++) {
            if (!SD.exists(filename)) {
                // only open a new file if it doesn't exist
                logfile = SD.open(filename, FILE_WRITE);
                if (logfile) {
                    logfileOpened = true;
                } else {
                    logfileOpened = false;
                }
                break;
            } else {
                delay(1000);
                sprintf(filename, DL_strDateToFilename());
            }
        }

        if (!logfileOpened) {
            Serial.print(F("Couldnt create file:"));
            Serial.println(filename);
        } else {
            Serial.print(F("Log to:"));
            Serial.println(filename);
        }
    } else {
        Serial.println(F("Log:serial"));
    }
    LOGSTRING(F("\"count\"\t\"date Y-m-d m:s\"\t\"gas\""));
#if BMP280_PRESENT
    if (BMP280Present) {
        LOGSTRING(F("\t\"temp\"\t\"pressure\"\t\"humidity\""));
    }
#endif
#if WINDSENSOR_PRESENT
    LOGSTRING(F("\t\"wind\""));
    LOGSTRING(F("\t\"temp2\""));
#endif
    LOGSTRINGLN(F("\t\"VCC\""));
}

/**
 * 
 */
static bool CONF_is_valid_char(char character) {
    if (isalnum(character) || character == '_') {
        return true;
    }
    return false;
}

/**
 * 
 */
static int CONF_getConfValueInt(char *filename, char *key, int defaultValue = 0) {
    File myFile;
    char character;
    String description = "";
    String value = "";
    boolean valid = true;
    int ret = defaultValue;
    if (!SD.exists(filename)) {
        //Serial.println("file not exists");
        return defaultValue;
    }
    myFile = SD.open(filename);
    while (myFile.available()) {
        description = "";
        character = myFile.read();

        //cerca una linea valida----->
        if (!CONF_is_valid_char(character)) {
            // Comment - ignore this line
            while (character != '\n' && myFile.available()) {
                character = myFile.read();
            };
            continue;
        }
        //cerca una linea valida-----<
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
                    //Serial.println(character);
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

/**
 * Fa N letture, prende i 10 centrali e fa la media di quelli
 */
static float DL_analogReadAndFilter(uint8_t sensorpin) {
    // read multiple values and sort them to take the mode
    analogRead(sensorpin);
    delay(10);
    int sortedValues[NUM_READS];
    int value;
    int j;
    for (int i = 0; i < NUM_READS; i++) {
        value = analogRead(sensorpin);
        if (value < sortedValues[0] || i == 0) {
            j = 0; //insert at first position
        } else {
            for (j = 1; j < i; j++) {
                if (sortedValues[j - 1] <= value && sortedValues[j] >= value) {
                    // j is insert position
                    break;
                }
            }
        }
        for (int k = i; k > j; k--) {
            // move all values higher than current reading up one position
            sortedValues[k] = sortedValues[k - 1];
        }
        sortedValues[j] = value; //insert current reading
        delay(2);
    }
    //return scaled mode of 10 values
    float returnval = 0;
    for (int i = NUM_READS / 2 - 5; i < (NUM_READS / 2 + 5); i++) {
        returnval += sortedValues[i];
    }
    returnval = returnval / 10;
    return returnval;
}

/**
 *
 */
static void DL_initConf() {

    if (sdPresent) {
        if (!SD.exists(CONF_FILE)) {
            Serial.print(CONF_FILE);
            Serial.println(F(" not exist"));
            settingFile = SD.open(CONF_FILE, FILE_WRITE);
            if (settingFile) {
                //create new file config.ini------>
                Serial.print(F("new file"));
                Serial.println(CONF_FILE);
                settingFile.print(F("log_interval_ms="));
                settingFile.println(LOG_INTERVAL_ms);
                settingFile.flush();
                settingFile.close();
                //create new file config.ini------<
            } else {
                Serial.print(F("error to create "));
                Serial.println(CONF_FILE);
            }
        } else {
            Serial.print(CONF_FILE);
            Serial.println(F(" already exist"));
        }

        //read from CONFIG.INI------->
        log_interval_ms = CONF_getConfValueInt(CONF_FILE, "log_interval_ms", LOG_INTERVAL_ms);
        //read from CONFIG.INI-------<
        Serial.print(F("Interval(ms)="));
        Serial.println(log_interval_ms);

    }
    if (log_interval_ms == 0) {
        log_interval_ms = LOG_INTERVAL_ms;
        Serial.print(F("Force to "));
        Serial.println(LOG_INTERVAL_ms);
    }
}

/**
 * The setup function is called once at startup of the sketch
 */
void setup() {
    int n = 0;
    int timeout = 5;

    //--init serial------------------------------>
    Serial.begin(9600);
    //--init serial------------------------------<
    sdPresent = false;
    rtcPresent = false;
    logfileOpened = false;
    Serial.println(F("\nOpenspeleo Datalogger"));
    Serial.print(F("Build time: "));
    Serial.print(F(__DATE__));
    Serial.print(F(" "));
    Serial.println(F(__TIME__));
    // initialize the SD card ------------------->
    //Serial.println("Initializing SD");
    pinMode(CHIP_SD, OUTPUT);
    Serial.print(F("SD initialization "));
    if (!SD.begin(CHIP_SD)) {
        sdPresent = false;
        Serial.println(F("failed"));
        failed = true;
    } else {
        Serial.println(F("successful"));
        sdPresent = true;
    }
    SerialFlush();
    DL_initConf();
    // initialize the SD card -------------------<
    // connect to RTC --------------------------->
    Wire.begin();
    unsigned long unixtime;
    unixtime = DateTime(__DATE__, __TIME__).unixtime();
    if (RTC.begin()) {
        Serial.println(F("RTC present"));
        rtcPresent = true;
        if (!RTC.isrunning()) {
            Serial.println(F("Clock not running"));
            RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
        Serial.print(F("Device clock:"));
        Serial.println(DL_strNow());
        Serial.println(F("Type any character to set time:"));
        while (!Serial.available()) {
            Serial.print(timeout - n);
            Serial.print(' ');
            if (n >= timeout) {
                break;
            }
            delay(1000);
            n++;
        }
        if (n < timeout) {
            SetDateTime();
        } else {
            Serial.println();
        }
    } else {
        rtcPresent = false;
        Serial.println(F("RTC Failed"));
        failed = true;

    }
    // connect to RTC ---------------------------<

#if BMP280_PRESENT
    if (!bme0.begin()) {
        Serial.println(F("BMP280 sensor not present"));
        BMP280Present = false;
    } else {
        Serial.println(F("BMP280 sensor present"));
        BMP280Present = true;
    }
#endif





    delay(10);
    TimeCurrent = millis();
    TimeTarget = TimeCurrent + log_interval_ms;
    TimeTargetFileWrite = TimeCurrent + SYNC_INTERVAL_ms;
    //---faccio una lettura a vuoto per scaricare il condensatore -------->
    for (int i = 0; i < 10; i++) {
        DL_analogReadAndFilter(S0);
        delay(20);
    }
    //---faccio una lettura a vuoto per scaricare il condensatore --------<
    //-----------init analog input-----------------------------------------<
    TimeTarget = millis();

}
/**
 *
 */
int parse_serial_command() {
    char buffer[siza_buffer];
    static bool ready = false;
    static int cnt = 0;
    while (Serial.available() > 0) {
        if (ready) {
            ready = false;
            // Serial.println(buffer);
            execute_command(buffer);
            return 1;
        }
        char c = Serial.read();
        if (c == '\n' || c == '\r' || (c >= 20 && c <= 126)) {
            if (cnt == 0 && (c == '\n' || c == '\r')) {
                continue;
            } else {
                buffer[cnt] = c;
                if (c == '\n' || c == '\r' || cnt == (siza_buffer - 1)) {
                    buffer[cnt] = '\0';
                    cnt = 0;
                    ready = true;
                } else {
                    cnt++;
                }
            }
        }
    }
    return 0;
}
/**
 *
 */
bool isValidNumber(char *str) {
    boolean isNum = false;
    for (byte i = 0; i < strlen(str); i++) {
        if (str[i] == '\0') {
            break;
        }
        isNum = isDigit(str[i]) || str[i] == '.';
        if (!isNum) return false;
    }
    return isNum;
}
/**
 *
 */
void execute_command(char *command) {
    unsigned long cmd;
    long year;
    long month;
    long day;
    long h;
    long m;
    long s = 0;
    if (strcmp(command, "help") == 0) {
        Serial.println();
        Serial.println("commands:");
        Serial.println("reset: reset device");
        Serial.println("settime: set device clock");
        Serial.println("setinterval: set log interval");
        Serial.println();
        return;
    }
    if (strcmp(command, "reset") == 0) {
        asm volatile("  jmp 0");  // reset arduino
        return;
    }
    if (strcmp(command, "settime") == 0) {
        SetDateTime();
        return;
    }
    if (strcmp(command, "setinterval") == 0) {
        SetInterval();
        return;
    }
}
/**
 *
 */
void SetInterval() {

    unsigned int interval;
    interval = (unsigned int)(log_interval_ms / 1000);
    Serial.print(F("\ncurrent interval in seconds:"));
    Serial.println(interval);
    Serial.print(F("\nSet interval in seconds:"));
    interval = InputIntFromSerial();
    Serial.println();
    Serial.print(F("\nNew interval:"));
    Serial.println(interval);
    log_interval_ms = interval * 1000;
    Serial.print(F("\nNew interval (ms):"));
    Serial.println(log_interval_ms);
    if (log_interval_ms > 0) {
        DL_closeLogFile();
        SD.remove(CONF_FILE);
        settingFile = SD.open(CONF_FILE, FILE_WRITE);
        if (settingFile) {
            //create new file config.ini------>
            settingFile.print(F("log_interval_ms="));
            settingFile.println(log_interval_ms);
            settingFile.flush();
            settingFile.close();
            Serial.print(F("Updated "));
            Serial.println(F(CONF_FILE));
            //create new file config.ini------<
        } else {
            Serial.print(F("error to update "));
            Serial.println(F(CONF_FILE));
        }
        DL_openLogFile();
    } else {
        Serial.println(F("\nInvalid value"));
    }

}
/**
 *
 */
void SetDateTime() {
    int year, month, day, hours, minutes;
    Serial.println(F("\nClock configuration:"));
    Serial.print(F("Set year:"));
    year = InputIntFromSerial();
    Serial.println(F("Set month:"));
    month = InputIntFromSerial();
    Serial.println(F("Set day:"));
    day = InputIntFromSerial();
    Serial.println(F("Set hours:"));
    hours = InputIntFromSerial();
    Serial.println(F("Set minutes:"));
    minutes = InputIntFromSerial();
    RTC.adjust(DateTime(year, month, day, hours, minutes, 0));
    asm volatile("  jmp 0");
}
/** 
 *  
 *  
 */
void SerialFlush() {
    Serial.flush();
    while (Serial.available() > 0) {
        Serial.read();
    }
}
/**
 *
 */
int InputIntFromSerial() {
    String value;
    SerialFlush();
    while (1) {
        if (Serial.available()) {
            value = Serial.readStringUntil('\n');
            SerialFlush();
            return value.toInt();
        }
    }
}




/**
 * Loop principale.
 */
void loop() {
    parse_serial_command();
    double S0Sensor = 0.0;
    double humidity = 0;
    double pressure = 0;
    double temperature = 0;
    double temperatureWindSensor;
    long vcc = 0;
#if WINDSENSOR_PRESENT
    const float zeroWindAdjustment =  .2; // negative numbers yield smaller wind speeds and vice versa.
    int TMP_Therm_ADunits;  //temp termistor value from wind sensor
    float RV_Wind_ADunits;    //RV output from wind sensor
    float RV_Wind_Volts;
    int TempCtimes100;
    float zeroWind_ADunits;
    float zeroWind_volts;
    float WindSpeed_MPH;
#endif

    TimeCurrent = millis();
    if (TimeCurrent >= TimeTarget) {
        TimeTarget = TimeCurrent + log_interval_ms;
        if (LogCounter == 0 || (LogCounter >= MAX_ROWS_PER_FILE
                                && (LogCounter % MAX_ROWS_PER_FILE == 0))) {
            DL_openLogFile();
        }
        //-----------sensors------------------------------------------------------->
        S0Sensor = DL_analogReadAndFilter(S0);
        vcc = DL_readVcc();
        //-----------sensors-------------------------------------------------------<
        LogCounter++;
        LOGSTRING(String(LogCounter));
        LOGSTRING(F("\t"));
        LOGSTRING(F("\""));
        LOGSTRING(DL_strNow());
        LOGSTRING(F("\""));
        LOGSTRING(F("\t"));
        LOGSTRING(S0Sensor);
#if BMP280_PRESENT
        if (BMP280Present) {

            temperature = bme0.readTemperature();
            humidity = bme0.readHumidity();
            pressure = bme0.readPressure();



            LOGSTRING(F("\t"));
            LOGSTRING(temperature);
            LOGSTRING(F("\t"));
            LOGSTRING(pressure);
            LOGSTRING(F("\t"));
            LOGSTRING(humidity);
        }
#endif
#if WINDSENSOR_PRESENT
        TMP_Therm_ADunits = DL_analogReadAndFilter(analogPinForTMP);
        RV_Wind_ADunits = DL_analogReadAndFilter(analogPinForRV);
        RV_Wind_Volts = (RV_Wind_ADunits *  0.0048828125);
        // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
        // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
        TempCtimes100 = (0.005 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) - (16.862 * (float)TMP_Therm_ADunits) + 9075.4;
        zeroWind_ADunits = -0.0006 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39
        zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;
        // This from a regression from data in the form of
        // Vraw = V0 + b * WindSpeed ^ c
        // V0 is zero wind at a particular temperature
        // The constants b and c were determined by some Excel wrangling with the solver.
        WindSpeed_MPH =  pow(((RV_Wind_Volts - zeroWind_volts) / .2300), 2.7265);
        /*
         Serial.print("  TMP volts ");
         Serial.print(TMP_Therm_ADunits * 0.0048828125);
         Serial.print(" RV volts ");
         Serial.print((float)RV_Wind_Volts);
         Serial.print("\t  TempC*100 ");
         Serial.print(TempCtimes100 );
         Serial.print("   ZeroWind volts ");
         Serial.print(zeroWind_volts);
         Serial.print("   WindSpeed MPH ");
         Serial.println((float)WindSpeed_MPH);
         */
        LOGSTRING(F("\t"));
        LOGSTRING(WindSpeed_MPH * 0.44704); // m/s
        LOGSTRING(F("\t"));
        LOGSTRING((temperatureWindSensor / 100)); // m/s
#endif
        LOGSTRING(F("\t"));
        LOGSTRING(String(vcc));
        LOGSTRING(F("\n"));
    }
    if (dataToWrite != 0 && TimeCurrent >= TimeTargetFileWrite) {
        if (logfileOpened) {
            logfile.flush();
        }
        dataToWrite = 0;
        TimeTargetFileWrite = TimeCurrent + SYNC_INTERVAL_ms;
    }
    if (failed) {
        Serial.print(F("\b")); //this make rx led on but not print anything in serial monitor
        delay(500);
    }
}
