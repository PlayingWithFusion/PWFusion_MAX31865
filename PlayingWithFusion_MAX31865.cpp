/***************************************************************************
* File Name: PlayingWithFusion_MAX31865.cpp
* Processor/Platform: Arduino Uno R3 (tested)
* Development Environment: Arduino 1.0.5
*
* Designed for use with with Playing With Fusion MAX31865 Resistance
* Temperature Device (RTD) breakout board: SEN-30201 (PT100 or PT1000)
*   ---> http://playingwithfusion.com/productview.php?pdid=25
*   ---> http://playingwithfusion.com/productview.php?pdid=26
*
* Copyright © 2014 Playing With Fusion, Inc.
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
* Author		    Date		Comments
* J. Steinlage		2014Jan09	Original version
*                   2014Aug07   Change type of rtd_res to uint16_t to address sign issue
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

#include "PlayingWithFusion_MAX31865.h"

PWFusion_MAX31865_RTD::PWFusion_MAX31865_RTD(int8_t CSx)
{
  // Function to initialize thermocouple channel, load private variables
  _cs = CSx;

  // immediately pull CS pin high to avoid conflicts on SPI bus
  digitalWrite(_cs, HIGH);
}

// function to configure MAX31865 configuration register
void PWFusion_MAX31865_RTD::MAX31865_config(struct var_max31865 *rtd_ptr, uint8_t RTD_TYPEx) 
{
  // handle RTD wiring
  uint8_t rtd_wiring_conf;
  if(RTD_USE_STORED == RTD_TYPEx) // use stored value (used for re-config fcn below)
  {
	rtd_wiring_conf = rtd_ptr->RTD_config;
  }
  else // else the value is new, store the value in the RTD structure
  {
	rtd_ptr->RTD_config = RTD_TYPEx;
	rtd_wiring_conf = RTD_TYPEx;
  }

  // take the chip select low to select the device:
  digitalWrite(_cs, LOW);

  // Write config to MAX31865 chip
  SPI.transfer(0x80);    // Send config register location to chip
                            // 0x8x to specify 'write register value' 
                            // 0xx0 to specify 'configuration register'

  uint8_t conf_byte = 0xC2;
  conf_byte |= rtd_wiring_conf;
  SPI.transfer(conf_byte);    // Write config to IC
                            // bit 7: Vbias -> 1 (ON)
                            // bit 6: conversion mode -> 1 (AUTO)
                            // bit 5: 1-shot -> 0 (off)
                            // bit 4: 3-wire select -> 0 (2/4 wire config)
                            // bit 3-2: fault detection cycle -> 0 (none)
                            // bit 1: fault status clear -> 1 (clear any fault)
                            // bit 0: 50/60 Hz filter select -> 0 (60 Hz)
  
  // take the chip select high to de-select, finish config write
  digitalWrite(_cs, HIGH);


  // code if you need to configure High and Low fault threshold registers (4 total registers)
//  // take the chip select low to select the device:
//  digitalWrite(_cs, LOW);
//  SPI.transfer(0x83);  // write cmd, start at HFT MSB reg (0x83)
//  SPI.transfer(0xFF);  // write cmd, start at HFT MSB reg (0x83)
//  SPI.transfer(0xFF);  // write cmd, start at HFT MSB reg (0x83)
//  SPI.transfer(0x00);  // write cmd, start at HFT MSB reg (0x83)
//  SPI.transfer(0x00);  // write cmd, start at HFT MSB reg (0x83)
//  // take the chip select high to de-select, finish config write
//  digitalWrite(_cs, HIGH);
}


void PWFusion_MAX31865_RTD::MAX31865_full_read(struct var_max31865 *rtd_ptr)
{
  // Function to unpack and store MAX31865 data
  uint16_t _temp_u16, _rtd_res;
  uint32_t _temp_u32;

  digitalWrite(_cs, LOW);			// must set CS low to start operation

  // Write command telling IC that we want to 'read' and start at register 0
  SPI.transfer(0x00);				// plan to start read at the config register

  // read registers in order:
	// configuration
	// RTD MSBs
	// RTD LSBs
	// High Fault Threshold MSB
	// High Fault Threshold LSB
	// Low Fault Threshold MSB
	// Low Fault Threshold LSB
	// Fault Status

  rtd_ptr->conf_reg = SPI.transfer(0x00);	// read 1st 8 bits

  _rtd_res = SPI.transfer(0x00);		    // read 2nd 8 bits
  _rtd_res <<= 8;				       		// shift data 8 bits left
  _rtd_res |= SPI.transfer(0x00);  			// read 3rd 8 bits
  rtd_ptr->rtd_res_raw = _rtd_res >> 1;		// store data after 1-bit right shift

  _temp_u16 = SPI.transfer(0x00);			// read 4th 8 bits
  _temp_u16 <<= 8;							// shift data 8 bits left
  _temp_u16 |= SPI.transfer(0x00);			// read 5th 8 bits
  rtd_ptr->HFT_val = _temp_u16 >> 1;		// store data after 1-bit right shift

  _temp_u16 = SPI.transfer(0x00);			// read 6th 8 bits
  _temp_u16 <<= 8;							// shift data 8 bits left
  _temp_u16 |= SPI.transfer(0x00);			// read 7th 8 bits
  rtd_ptr->LFT_val = _temp_u16;				// store data after 1-bit right shift

  rtd_ptr->status = SPI.transfer(0x00);		// read 8th 8 bits

  digitalWrite(_cs, HIGH);					// set CS high to finish read
  
  // re-write config if no valid read or a fault present
  // keep in mind some faults re-set immediately (HFT/LFT)
  if((0 == _rtd_res) || (0 != rtd_ptr->status))  
  {
     MAX31865_config(rtd_ptr, RTD_USE_STORED);	// call config function
  }
}
