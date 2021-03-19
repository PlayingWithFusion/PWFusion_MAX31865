/***************************************************************************
* File Name: PwFusion_MAX31865.cpp
* Processor/Platform: Arduino Uno R3 (tested)
* Development Environment: Arduino 1.0.5
*
* Designed for use with with Playing With Fusion MAX31865 Resistance
* Temperature Device (RTD) breakout board: SEN-30201 (PT100 or PT1000)
*   ---> http://playingwithfusion.com/productview.php?pdid=25
*   ---> http://playingwithfusion.com/productview.php?pdid=26
*
* Copyright � 2014 Playing With Fusion, Inc.
* SOFTWARE LICENSE AGREEMENT: This code is released under the MIT License.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
* **************************************************************************
* REVISION HISTORY:
* Author		    Date		    Comments
* J. Steinlage  2014Jan09	  Original version
* J. Steinlage  2014Aug07   Change type of rtd_res to uint16_t to address sign issue
* J. Leonard    2021Mar16   Updated API to "feel" consistent with other PwFusion
*                           libraries.  Now features true object-oriented API.
*
* Playing With Fusion, Inc. invests time and resources developing open-source
* code. Please support Playing With Fusion and continued open-source
* development by buying products from Playing With Fusion!
*
* **************************************************************************
* ADDITIONAL NOTES:
* This file contains functions to initialize and run an Arduino Uno R3 in
* order to communicate with a MAX31865 single channel Resistance Thermal
* Device (RTD) breakout board. Funcionality is as described below:
*   - Read MAX31865 registers from Playing With Fusion SEN-30201
*       (both standard types, PT100 and PT1000)
*   - Unpack register read into relevant variables
***************************************************************************/

#include "PwFusion_MAX31865.h"


void MAX31865::begin(int8_t cs, uint8_t numWires, uint16_t sensorType, SPIClass &spiPort)
{
  // immediately pull CS pin high to avoid conflicts on SPI bus
  digitalWrite(cs, HIGH);

  // Function to initialize thermocouple channel, load private variables
  _cs = cs;
  _numWires = numWires;
  _sensorType = sensorType;
  _spiPort = &spiPort;

  writeConfig();
}


void MAX31865::writeConfig()
{
  uint8_t regCfg;

  // Write config to IC
  // bit 7: Vbias -> 1 (ON)
  // bit 6: conversion mode -> 1 (AUTO)
  // bit 5: 1-shot -> 0 (off)
  // bit 4: 3-wire select -> 0 (2/4 wire config)
  // bit 3-2: fault detection cycle -> 0 (none)
  // bit 1: fault status clear -> 1 (clear any fault)
  // bit 0: 50/60 Hz filter select -> 0 (60 Hz)
  regCfg = 0xC2 | ((_numWires==3) ? 0x10 : 0x00);

  // Write config to MAX31865 chip
  writeByte(REG_CFG, regCfg);

  writeWord(REG_HFT, regHft);
  writeWord(REG_LFT, regLft);
}


void MAX31865::sample()
{
  regRtdRes = readWord(REG_RESISTANCE) >> 1;  // Register 1 & 2, RTD MSB/LSB

  regStatus = readByte(REG_STATUS); // Register 7, Status
  
  // re-write config if no valid read or a fault present
  // keep in mind some faults re-set immediately (HFT/LFT)
  if((0 == regRtdRes) || (0 != regStatus))  
  {
     writeConfig();
  }
}


float MAX31865::getResistance()
{
  return (float)((uint32_t)regRtdRes * (uint32_t)_sensorType) / 32767.0f;
}


float MAX31865::getTemperature()
{
  return ((float)regRtdRes / 32.0f) - 256.0f;
}


uint8_t MAX31865::getStatus()
{
  return regStatus;
}


void MAX31865::setHighFaultTemperature(float value)
{
  // Convert degrees C to ADC counts
  regHft = (uint16_t)((value + 256.0f) *32.0f) << 1;
  writeWord(REG_HFT, regHft);

}


void MAX31865::setLowFaultTemperature(float value)
{
  // Convert degrees C to ADC counts
  regLft = (uint16_t)((value + 256.0f) *32.0f) << 1;
  writeWord(REG_LFT, regLft);
}


void MAX31865::writeByte(Max31865_Reg reg, uint8_t value)
{
  _spiPort->setClockDivider(SPI_CLOCK_DIV16);   // SPI speed to SPI_CLOCK_DIV16 (1MHz)
  _spiPort->setDataMode(SPI_MODE3);             // MAX31865 works in MODE1 or MODE3

  // take the chip select low to select the device:
  digitalWrite(_cs, LOW);

  _spiPort->transfer(reg | 0x80);
  _spiPort->transfer(value);

  // take the chip select high to de-select, finish config write
  digitalWrite(_cs, HIGH);  
}

void MAX31865::writeWord(Max31865_Reg reg, uint16_t value)
{
  _spiPort->setClockDivider(SPI_CLOCK_DIV16);   // SPI speed to SPI_CLOCK_DIV16 (1MHz)
  _spiPort->setDataMode(SPI_MODE3);             // MAX31865 works in MODE1 or MODE3

  // take the chip select low to select the device:
  digitalWrite(_cs, LOW);

  _spiPort->transfer(reg | 0x80);
  _spiPort->transfer(value >> 8);
  _spiPort->transfer(value & 0xFF);

  // take the chip select high to de-select, finish config write
  digitalWrite(_cs, HIGH);  
}

uint8_t MAX31865::readByte(Max31865_Reg reg)
{
  uint8_t result;

  _spiPort->setClockDivider(SPI_CLOCK_DIV16);   // SPI speed to SPI_CLOCK_DIV16 (1MHz)
  _spiPort->setDataMode(SPI_MODE3);             // MAX31865 works in MODE1 or MODE3

  // take the chip select low to select the device:
  digitalWrite(_cs, LOW);

  _spiPort->transfer(reg);
  result = _spiPort->transfer(0);

  // take the chip select high to de-select, finish config write
  digitalWrite(_cs, HIGH);  

  return result;
}


uint16_t MAX31865::readWord(Max31865_Reg reg)
{
  uint16_t result;

  _spiPort->setClockDivider(SPI_CLOCK_DIV16);   // SPI speed to SPI_CLOCK_DIV16 (1MHz)
  _spiPort->setDataMode(SPI_MODE3);             // MAX31865 works in MODE1 or MODE3

  // take the chip select low to select the device:
  digitalWrite(_cs, LOW);

  _spiPort->transfer(reg);
  result = (uint16_t)_spiPort->transfer(0) << 8;
  result |= (uint16_t)_spiPort->transfer(0);

  // take the chip select high to de-select, finish config write
  digitalWrite(_cs, HIGH);  

  return result;
}
