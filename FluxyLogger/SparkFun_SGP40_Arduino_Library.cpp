/*
  This is a library written for the SPG40
  By Paul Clark @ SparkFun Electronics, December 5th, 2020


  https://github.com/sparkfun/SparkFun_SGP40_Arduino_Library

  Development environment specifics:
  Arduino IDE 1.8.13

  SparkFun labored with love to create this code. Feel like supporting open
  source hardware? Buy a board from SparkFun!
  https://www.sparkfun.com/products/17729


  VOC Index Algorithm provided by Sensirion:
  https://github.com/Sensirion/embedded-sgp


  CRC lookup table from Bastian Molkenthin  http://www.sunshine2k.de/coding/javascript/crc/crc_js.html

  Copyright (c) 2015 Bastian Molkenthin

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/


#include "SparkFun_SGP40_Arduino_Library.h"


//Constructor
SGP40::SGP40()
{
}

//Start I2C communication using specified port
//Returns true if successful or false if no sensor detected
bool SGP40::begin(TwoWire &wirePort)
{
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  VocAlgorithm_init(&vocAlgorithmParameters); //Initialize the VOC parameters

  SGP40ERR result = measureTest();
  return (result == SGP40_SUCCESS);
}

//Calling this function with nothing sets the debug port to Serial
//You can also call it with other streams like Serial1, SerialUSB, etc.
void SGP40::enableDebugging(Stream &debugPort)
{
	_debugPort = &debugPort;
	_printDebug = true;
}

//Sensor runs on chip self test
//Returns SUCCESS (0) if successful or other error code if unsuccessful
SGP40ERR SGP40::measureTest(void)
{
  _i2cPort->beginTransmission(_SGP40Address);
  _i2cPort->write(sgp40_measure_test, 2); //Perform a measurement test
  uint8_t i2cResult = _i2cPort->endTransmission();

  if (i2cResult != 0)
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("measureTest: endTransmission returned: "));
      //_debugPort->println(i2cResult);
    }
    return SGP40_ERR_I2C_ERROR;
  }

  //Hang out while measurement is taken. v1.1 of the datasheet says 320ms
  delay(320);

  //Comes back in 3 bytes, data(MSB) / data(LSB) / Checksum
  uint8_t toRead = _i2cPort->requestFrom(_SGP40Address, (uint8_t)3);
  if (toRead != 3)
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("measureTest: requestFrom returned: "));
      //_debugPort->println(toRead);
    }
    return SGP40_ERR_I2C_ERROR; //Error out
  }

  uint16_t results = ((uint16_t)_i2cPort->read()) << 8; //store MSB in results
  results |= _i2cPort->read(); //store LSB in results

  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(results))
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("measureTest: checksum failed! Expected: 0x"));
      //_debugPort->print(_CRC8(results), HEX);
      //_debugPort->print(F(" Received: 0x"));
      //_debugPort->println(checkSum, HEX);
    }
    return SGP40_ERR_BAD_CRC; //checksum failed
  }

  if (results != sgp40_measure_test_pass)
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("measureTest: unexpected test results! Received: 0x"));
      //_debugPort->println(results, HEX);
    }
    return SGP40_MEASURE_TEST_FAIL; //self test results incorrect
  }

  return SGP40_SUCCESS;
}

//Perform a soft reset
//Returns SUCCESS (0) if successful
SGP40ERR SGP40::softReset(void)
{
  _i2cPort->beginTransmission(_SGP40Address);
  _i2cPort->write(sgp40_soft_reset, 2); //Perform a soft reset
  uint8_t i2cResult = _i2cPort->endTransmission();

  if (i2cResult != 0)
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("softReset: endTransmission returned: "));
      //_debugPort->println(i2cResult);
    }
    return SGP40_ERR_I2C_ERROR;
  }

  return SGP40_SUCCESS;
}

//Turn the heater off
//Returns SUCCESS (0) if successful
SGP40ERR SGP40::heaterOff(void)
{
  _i2cPort->beginTransmission(_SGP40Address);
  _i2cPort->write(sgp40_heater_off, 2); //Turn the heater off
  uint8_t i2cResult = _i2cPort->endTransmission();

  delay(1); // From datasheet

  if (i2cResult != 0)
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("heaterOff: endTransmission returned: "));
      //_debugPort->println(i2cResult);
    }
    return SGP40_ERR_I2C_ERROR;
  }

  return SGP40_SUCCESS;
}

//Measure Raw Signal
//Returns SUCCESS (0) if successful
//The raw signal is returned in SRAW_ticks
//The user can provide Relative Humidity and Temperature parameters if desired:
//RH_ticks and T_ticks are 16 bit
//RH_ticks = %RH * 65535 / 100
//T_ticks = (DegC + 45) * 65535 / 175
//%RH: Minimum=0 (RH_ticks = 0x0000)  Maximum=100 (RH_ticks = 0xFFFF)
//DegC: Minimum=-45 (T_ticks = 0x0000)  Minimum=130 (T_ticks = 0xFFFF)
//Default %RH = 50 (RH_ticks = 0x8000)
//Default DegC = 25 (T_ticks = 0x6666)
//See the SGP40 datasheet for more details
SGP40ERR SGP40::measureRaw(uint16_t *SRAW_ticks, float RH, float T)
{
  uint16_t RH_ticks, T_ticks;

  // Check RH and T are within bounds (probably redundant, but may prevent unexpected wrap-around errors)
  if (RH < 0)
  {
    if (_printDebug == true)
      //_debugPort->println(F("measureRaw: RH too low! Correcting..."));
    RH = 0;
  }
  if (RH > 100)
  {
    if (_printDebug == true)
      //_debugPort->println(F("measureRaw: RH too high! Correcting..."));
    RH = 100;
  }
  if (T < -45)
  {
    if (_printDebug == true)
      //_debugPort->println(F("measureRaw: T too low! Correcting..."));
    T = -45;
  }
  if (T > 130)
  {
    if (_printDebug == true)
      //_debugPort->println(F("measureRaw: T too high! Correcting..."));
    T = 130;
  }

  RH_ticks = (uint16_t)(RH * 65535 / 100); // Convert RH from %RH to ticks
  T_ticks = (uint16_t)((T + 45) * 65535 / 175); // Convert T from DegC to ticks
  if (_printDebug == true)
  {
    //_debugPort->print(F("measureRaw: RH_ticks: 0x"));
    //_debugPort->print(RH_ticks, HEX);
    //_debugPort->print(F(" T_ticks: 0x"));
    //_debugPort->println(T_ticks, HEX);
  }

  _i2cPort->beginTransmission(_SGP40Address);
  _i2cPort->write(sgp40_measure_raw, 2); //Perform a raw measurement

  _i2cPort->write((uint8_t)(RH_ticks >> 8)); //Write the RH_ticks MSB
  _i2cPort->write((uint8_t)(RH_ticks & 0xFF)); //Write the RH_ticks LSB
  _i2cPort->write(_CRC8(RH_ticks)); //Write the CRC

  _i2cPort->write((uint8_t)(T_ticks >> 8)); //Write the T_ticks MSB
  _i2cPort->write((uint8_t)(T_ticks & 0xFF)); //Write the T_ticks LSB
  _i2cPort->write(_CRC8(T_ticks)); //Write the CRC

  uint8_t i2cResult = _i2cPort->endTransmission();

  if (i2cResult != 0)
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("measureRaw: endTransmission returned: "));
      //_debugPort->println(i2cResult);
    }
    return SGP40_ERR_I2C_ERROR;
  }

  //Hang out while measurement is taken. datasheet says 30ms
  delay(30);

  //Comes back in 3 bytes, data(MSB) / data(LSB) / Checksum
  uint8_t toRead = _i2cPort->requestFrom(_SGP40Address, (uint8_t)3);
  if (toRead != 3)
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("measureRaw: requestFrom returned: "));
      //_debugPort->println(toRead);
    }
    return SGP40_ERR_I2C_ERROR; //Error out
  }

  uint16_t results = ((uint16_t)_i2cPort->read()) << 8; //store MSB in results
  results |= _i2cPort->read(); //store LSB in results

  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(results))
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("measureRaw: checksum failed! Expected: 0x"));
      //_debugPort->print(_CRC8(results), HEX);
      //_debugPort->print(F(" Received: 0x"));
      //_debugPort->println(checkSum, HEX);
    }
    return SGP40_ERR_BAD_CRC; //checksum failed
  }

  *SRAW_ticks = results; //Pass the results

  return SGP40_SUCCESS;
}

//Get VOC Index
//Returns -100 if an error occurs
//The user can provide Relative Humidity and Temperature parameters if desired
int32_t SGP40::getVOCindex(float RH, float T)
{
  int32_t vocIndex;
  uint16_t SRAW_ticks;

  SGP40ERR result = measureRaw(&SRAW_ticks, RH, T);

  if (result != SGP40_SUCCESS)
  {
    if (_printDebug == true)
    {
      //_debugPort->print(F("getVOCindex: measureRaw returned error: "));
      //_debugPort->println(result);
    }
    return -100; //fail...
  }
  //return SRAW_ticks;
  
  VocAlgorithm_process(&vocAlgorithmParameters, SRAW_ticks, &vocIndex);

  return vocIndex;
  
}

uint32_t SGP40::getRaw()
{
  int32_t vocIndex;
  uint16_t SRAW_ticks;
  SGP40ERR result = measureRaw(&SRAW_ticks, 0, 0);
  if (result != SGP40_SUCCESS)
  {
    return -100; //fail...
  }
  return SRAW_ticks;  
}

#ifndef SGP40_LOOKUP_TABLE
//Given an array and a number of bytes, this calculate CRC8 for those bytes
//CRC is only calc'd on the data portion (two bytes) of the four bytes being sent
//From: http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
//Tested with: http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
//x^8+x^5+x^4+1 = 0x31
uint8_t SGP40::_CRC8(uint16_t data)
{
  uint8_t crc = 0xFF; //Init with 0xFF

  crc ^= (data >> 8); // XOR-in the first input byte

  for (uint8_t i = 0 ; i < 8 ; i++)
  {
    if ((crc & 0x80) != 0)
      crc = (uint8_t)((crc << 1) ^ 0x31);
    else
      crc <<= 1;
  }

  crc ^= (uint8_t)data; // XOR-in the last input byte

  for (uint8_t i = 0 ; i < 8 ; i++)
  {
    if ((crc & 0x80) != 0)
      crc = (uint8_t)((crc << 1) ^ 0x31);
    else
      crc <<= 1;
  }

  return crc; //No output reflection
}
#else
//Generates CRC8 for SGP40 from lookup table
uint8_t SGP40::_CRC8(uint16_t data)
{
  uint8_t crc = 0xFF; //inital value
  crc ^= (uint8_t)(data >> 8); //start with MSB
  crc = _CRC8LookupTable[crc >> 4][crc & 0xF]; //look up table [MSnibble][LSnibble]
  crc ^= (uint8_t)data; //use LSB
  crc = _CRC8LookupTable[crc >> 4][crc & 0xF]; //look up table [MSnibble][LSnibble]
  return crc;
}
#endif
