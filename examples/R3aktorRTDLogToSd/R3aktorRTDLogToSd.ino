/***************************************************************************
* File Name: R3aktorRTDLogToSd.ino
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
//                                     DEFS
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

  setupSDCard();
}

void
loop()
{
  // Periodic tasks (see below)
  updateRTDs();
  writeToSDCard();

  // Run every 2 seconds
  delay(2000);
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

static void setupSDCard() {
  // Setup SD Card
  Serial.print("Initializing SD card....");

  if (!SD.begin())
  {
    Serial.println("Failed");
    while (1);
  }
  else
  {
    Serial.println("Done");
  }

  Serial.println("Initializing log file...");

  path = getNextFileName();
  csvFile = SD.open(path, FILE_WRITE);

  if (!csvFile) {
    Serial.println("Failed to initialize CSV file, aborting");
    while (1);
  }
  
  Serial.print("Logging to: ");
  Serial.println(path);
  Serial.println("Done");

  Serial.println(getCSVHeader());
  csvFile.println(getCSVHeader());
  csvFile.close();
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

static void updateRTDs() {
  for (size_t i = 0; i < NUM_CHANNELS; ++i){

    float temp = 0; 
    float resistance = 0;
    bool fault = true;

    rtds[i].sample();

    if (rtds[i].getStatus() == 0){
      fault = false;
    }

    resistance = rtds[i].getResistance();
    temp = rtds[i].getTemperature();

    vals[i] = rtd_vals_t {
      .temp = temp,
      .resistance = resistance,
      .fault = fault,
    };

  }
}

static void writeToSDCard() {
  // Write data to SD card
  // TODO: remove sim
  csvFile = SD.open(path, FILE_WRITE);
  String data = getCSVData(vals[0], vals[1], vals[2], vals[3], getBatteryVoltage(), millis());
  csvFile.println(data);
  Serial.println(data);
  csvFile.close();
}

// =========================================================================
//                             UTILITY FUNCTIONS
// =========================================================================

/*
 * Count the number of files present on the SD card
 * to create a new file name for the R3actor Logger.
 * NOTE: SD library support 8.3 file names, meaning
 * with the naming scheme "log<number>.csv", this function
 * can support up to 99,999 different files.
 */
String
getNextFileName()
{
  int numFiles = 0;
 File root = SD.open("/");
  if (!root)
  {
    Serial.println("Failed to open root file...");
    return "";
  }

  File nextFile = root.openNextFile();

  while (nextFile)
  {
    numFiles++;
    nextFile = root.openNextFile();
  }

  return "rtd" + String(numFiles) + ".csv";
}

/*
 * Create the string representation of data for CSV file
 * 
 */
String
getCSVData(rtd_vals_t rtd0, rtd_vals_t rtd1, rtd_vals_t rtd2, rtd_vals_t rtd3, float battV, long runTime)
{
  return String(rtd0.temp) + "," + String(rtd0.resistance) + "," + String(rtd0.fault) 
  + "," + String(rtd1.temp) + "," + String(rtd1.resistance) + "," + String(rtd1.fault)
  + "," + String(rtd2.temp) + "," + String(rtd2.resistance) + "," + String(rtd2.fault) 
  + "," + String(rtd3.temp) + "," + String(rtd3.resistance) + "," + String(rtd3.fault) 
  + "," + battV + "," + runTime;
}

/*
 * Construct a CSV file header. The CSV file will be 
 * prepended with the model number of board currently being used
 */
String
getCSVHeader()
{
  return "MODEL NUMBER: FDQ-30002\nrtd0-t,rtd0-r,rtd0-fault,rtd1-t,rtd1-r,rtd1-fault,rtd2-t,rtd2-r,rtd2-fault,rtd3-t,rtd3-r,rtd3-fault,battV,ms-after-start";
}

/*
 * Get the battery voltage from the charge circuit onboard
 */
float
getBatteryVoltage()
{
  // two 499k ohm resistors create a 
  // voltage divier with a ratio of 1:2
  // 
  // This function assumes:
  // 10-bit wide, 1.65V Vref, which should be 
  // the case if running on a R3actor TC Logger
  return ((float)analogRead(ADC_BATTERY) * 3.3) / 1023.0; // 3.3 = 1.65 * 2
}