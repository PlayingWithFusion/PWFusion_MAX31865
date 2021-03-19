/***************************************************************************
* File Name: max31865_example.ino
* Processor/Platform: Playing With Fusion R3aktor M0 (tested)
* Development Environment: Arduino 1.8.13
*
* Designed for use with with Playing With Fusion MAX31865 Resistance
* Temperature Device (RTD) breakout board: SEN-30202 (PT100 or PT1000)
*   ---> http://playingwithfusion.com/productview.php?pdid=25
*   ---> http://playingwithfusion.com/productview.php?pdid=26
*
* Copyright © 2021 Playing With Fusion, Inc.
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
* Author                        Date            Comments
* J. Leonard                    2021Mar19       Original version
*
* Playing With Fusion, Inc. invests time and resources developing open-source
* code. Please support Playing With Fusion and continued open-source 
* development by buying products from Playing With Fusion!
*
* **************************************************************************
* ADDITIONAL NOTES:
* This file configures then runs a program on PwFusion R3aktor to read a
* MAX31865 RTD-to-digital converter breakout board and print results to
* a serial port. Communication is via SPI built-in library.
*    - Configure Arduino Uno or PwFusion R3aktor
*    - Configure and read resistances and statuses from MAX31865 IC 
*      - Write config registers (MAX31865 starts up in a low-power state)
*      - RTD resistance register
*      - High and low status thresholds 
*      - Fault statuses
*    - Write formatted information to serial port
*  Circuit:
*    Arduino Uno   Arduino Mega   R3aktor M0      -->  SEN-30201
*    CS:   pin  9  CS:   pin  9   CS: pin D9      -->  CS, CH0
*    MOSI: pin 11  MOSI: pin 51   MOSI: pin MOSI  -->  SDI (must not be changed for hardware SPI)
*    MISO: pin 12  MISO: pin 50   MISO: pin MISO  -->  SDO (must not be changed for hardware SPI)
*    SCK:  pin 13  SCK:  pin 52   SCK:  pin SCK   -->  SCLK (must not be changed for hardware SPI)
*    GND           GND            GND             -->  GND
*    5V            5V             3.3V            -->  Vin (supply with same voltage as Arduino I/O, 5V or 3.3V)
***************************************************************************/

// the sensor communicates using SPI, so include the hardware SPI library:
#include <SPI.h>
// include Playing With Fusion MAX31865 library
#include <PwFusion_MAX31865.h> 

// CS pin used for the connection with the sensor
// other connections are controlled by the SPI library)
const int CS_PIN = 9;

// Create instance of MAX31865 class
MAX31865 rtd0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Boot"));

  // setup for the the SPI library:
  SPI.begin();
  
  // initalize the chip select pin
  pinMode(CS_PIN, OUTPUT);

  // configure rtd sensor
  rtd0.begin(CS_PIN, RTD_4_WIRE, RTD_TYPE_PT100);
  rtd0.setLowFaultTemperature(30);  // Set the low fault threshold to 30 degrees C
  rtd0.setHighFaultTemperature(70); // Set the high fault threshold to 70 degrees C

  Serial.println(F("MAX31865 Configured"));
  
  // give the sensor time to set up
  delay(100);
}


void loop() 
{
  // Get the latest temperature and status values from the MAX31865
  rtd0.sample();
  
  // Print the current values to the serial port
  Serial.print(rtd0.getResistance());
  Serial.print(F(" Ohms,   "));

  Serial.print(rtd0.getTemperature());
  Serial.print(F(" C,   "));

  // Print the Status bitmask
  PrintRTDStatus(rtd0.getStatus());

  // 500ms delay... can be much faster
  delay(500);    
}


void PrintRTDStatus(uint8_t status)
{
  // status will be 0 if no faults are active
  if (status == 0)
  {
    Serial.print(F("OK"));
  }
  else 
  {
    // status is a bitmask, so multiple faults may be active at the same time

    // The RTD temperature is above the threshold set by setHighFaultTemperature()
    if (status & RTD_FAULT_TEMP_HIGH)
    {
      Serial.print(F("RTD High Threshold Met, "));
    }

    // The RTD temperature is below the threshold set by setHLowFaultTemperature()
    if (status & RTD_FAULT_TEMP_LOW)
    {
      Serial.print(F("RTD Low Threshold Met, "));
    }

    // The RefIn- is > 0.85 x Vbias
    if (status & RTD_FAULT_REFIN_HIGH)
    {
      Serial.print(F("REFin- > 0.85 x Vbias, "));
    }

    // The RefIn- or RtdIn- pin is < 0.85 x Vbia
    if (status & (RTD_FAULT_REFIN_LOW_OPEN | RTD_FAULT_RTDIN_LOW_OPEN))
    {
      Serial.print(F("FORCE- open, "));
    }

    // The measured voltage at the RTD sense pins is too high or two low
    if (status & RTD_FAULT_VOLTAGE_OOR)
    {
      Serial.print(F("Voltage out of range fault, "));
    }
  }

  Serial.println();
}

