/***************************************************************************
* File Name: PwFusion_MAX31865.h
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
* Author		Date		Comments
* J. Steinlage		2014Jan09	Original version
* J. Leonard      2021Mar16 Updated API to "feel" consistent with other PwFusion
*                           libraries.  Now features true object-oriented API.
*
* Playing With Fusion, Inc. invests time and resources developing open-source
* code. Please support Playing With Fusion and continued open-source
* development by buying products from Playing With Fusion!
*
* **************************************************************************/

#ifndef PWFUSION_MAX31865_h
#define PWFUSION_MAX31865_h

#include <Arduino.h>
#include <stdlib.h>
#include <SPI.h>

#define RTD_2_WIRE        2
#define RTD_3_WIRE        3
#define RTD_4_WIRE        4

#define RTD_TYPE_PT100    400
#define RTD_TYPE_PT500    2000
#define RTD_TYPE_PT1000   4000

#define RTD_FAULT_TEMP_HIGH       0x80
#define RTD_FAULT_TEMP_LOW        0x40
#define RTD_FAULT_REFIN_HIGH      0x20
#define RTD_FAULT_REFIN_LOW_OPEN  0x10
#define RTD_FAULT_RTDIN_LOW_OPEN  0x08
#define RTD_FAULT_VOLTAGE_OOR     0x04


typedef enum Max31865_Reg_e {
  REG_CFG         = 0x00,
  REG_RESISTANCE  = 0x01,
  REG_HFT         = 0x03,
  REG_LFT         = 0x05,
  REG_STATUS      = 0x07
} Max31865_Reg;


class MAX31865
{
  public:
    void begin(int8_t cs, uint8_t numWires = RTD_3_WIRE, uint16_t sensorType = RTD_TYPE_PT100, SPIClass &spiPort = SPI);
    void sample();
    float getResistance();
    float getTemperature();
    uint8_t getStatus();
    void setLowFaultTemperature(float value);
    void setHighFaultTemperature(float value);

    void writeByte(Max31865_Reg reg, uint8_t value);
    void writeWord(Max31865_Reg reg, uint16_t value);
    uint8_t readByte(Max31865_Reg reg);
    uint16_t readWord(Max31865_Reg reg);
 
  private:
    void writeConfig();

    uint8_t _cs;
    SPIClass *_spiPort;
    uint8_t  _numWires;       // RTD type. 1 = PT100; 2 = PT1000
    uint16_t  _sensorType;    // # of wires. 2/4-wire: 0x00, 3-wire: 0x10

    uint16_t regRtdRes;       // RTD IC raw resistance register
    uint8_t  regStatus;       // RTD status - full status code
    uint16_t regHft = 65535;  // High fault threshold register
    uint16_t regLft = 0;      // Low fault threshold register
};

#endif // PWFUSION_MAX31865_h
