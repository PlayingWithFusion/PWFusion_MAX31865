/***************************************************************************
* File Name: PlayingWithFusion_MAX31865.h
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
* Author		Date		Comments
* J. Steinlage		2014Jan09	Original version
*
* Playing With Fusion, Inc. invests time and resources developing open-source
* code. Please support Playing With Fusion and continued open-source
* development by buying products from Playing With Fusion!
*
* **************************************************************************/

#ifndef PWF_MAX31865_h
#define PWF_MAX31865_h

#include "Arduino.h"			// use "WProgram.h" for IDE <1.0
#include "avr/pgmspace.h"
#include "util/delay.h"
#include "stdlib.h"
#include "SPI.h"
#include "PlayingWithFusion_MAX31865_STRUCT.h"

#define		RTD_2_WIRE		0x00
#define		RTD_3_WIRE		0x10
#define		RTD_4_WIRE		0x00
#define		RTD_USE_STORED	0x99

class PWFusion_MAX31865_RTD
{
 public:
  PWFusion_MAX31865_RTD(int8_t CSx);
  void MAX31865_config(struct var_max31865 *rtd_ptr, uint8_t RTD_TYPEx);
  void MAX31865_full_read(struct var_max31865 *rtd_ptr);
  
 private:
  int8_t _cs;
};

#endif
