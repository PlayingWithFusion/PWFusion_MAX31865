/***************************************************************************
* File Name: R3aktorRTDDumpMeasurements.ino
* Processor/Platform: R3actor Core Thermo RTD
* Development Environment: Arduino 2.3.2
*
* Designed for use with with Playing With Fusion MAX31865 R3actor 
* RTD breakout board: FDQ-30002
*
* Copyright © 2024 Playing With Fusion, Inc.
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
* Author		  Date	      Comments
* J.Simeone   2024-06-04  Initial release
*
* Playing With Fusion, Inc. invests time and resources developing open-source
* code. Please support Playing With Fusion and continued open-source
* development by buying products from Playing With Fusion!
* **************************************************************************
* ADDITIONAL NOTES:
* It's recommended to reference the datasheet for the MAX31865 as well as the
* MAX31865 Playing With Fusion library. 
*
* This example demonstrates how to: 
* - Read temperature and resistance from RTD channels
* - Read battery voltage from charge circuit
* - Log as CSV to SD card
***************************************************************************/

#include <SD.h>
#include <SPI.h>
#include <PwFusion_MAX31865.h>

// =========================================================================
//                                   DEFS
// =========================================================================

#define NUM_CHANNELS (4)

// Structure holding RTD channel data
typedef struct {
  float temp;
  float resistance;
  uint8_t fault;
} rtd_vals_t;

// =========================================================================
//                             EXAMPLE VARIABLES
// =========================================================================

static MAX31865 rtds[NUM_CHANNELS];
static rtd_vals_t vals[NUM_CHANNELS] = {};
static int cs[NUM_CHANNELS] = {2, 3, 4, 5};
static File csvFile;   // CSV file object
static String path;    // Name of CSV file

// =========================================================================
//                               USER VARIABLES
// =========================================================================

// Leave this alone if you have a PT100 RTD type board, but switch the 
// commented statements if yo uhave a PT1000 board. 
static int type = RTD_TYPE_PT100;
//static int type = RTD_TYPE_PT1000;

// Change these to change the behavior of the example
static uint8_t rtdWireTypes[NUM_CHANNELS] = {
  RTD_4_WIRE,
  RTD_3_WIRE,
  RTD_2_WIRE,
  RTD_2_WIRE,
};

// =========================================================================
//                                MAIN PROGRAM
// =========================================================================

void
setup()
{
  // Setup tasks (see below)
  setupSerial();
  
  SPI.begin();

  setupRTDs();
}

void
loop()
{
  for (size_t i = 0; i < NUM_CHANNELS; ++i){
    rtds[i].sample();

    dumpRTDVals(rtds[i]);
  }

  // Run every 1 second
  delay(1000);
}

// =========================================================================
//                              EXAMPLE TASKS
// =========================================================================

static void setupSerial() {
  // Setup Serial
  Serial.begin(115200);
  while (!Serial)
  {
    ; // Wait for serial to be set up
  }
}

static void setupRTDs() {
  for (size_t i = 0; i < NUM_CHANNELS; ++i){
    pinMode(cs[i], OUTPUT);
    // Configures, in order:
    // - Chip select pin for this particular MAX31865 on SPI bus
    // - RTD wire type is how many wires the user used to connect the RTD sensor
    // - Finally, the type of RTD should be the same accross all channels on the board
    //   check your board to check if it's a PT100 or a PT1000
    rtds[i].begin(cs[i], rtdWireTypes[i], type);
  }
}

// =========================================================================
//                             UTILITY FUNCTIONS
// =========================================================================

void dumpRTDVals(MAX31865 rtd) {

  uint8_t status = rtd.getStatus();

  // Print the current values to the serial port
  Serial.print(rtd.getResistance());
  Serial.print(F(" Ohms,   "));

  Serial.print(rtd.getTemperature());
  Serial.print(F(" C,   "));

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
