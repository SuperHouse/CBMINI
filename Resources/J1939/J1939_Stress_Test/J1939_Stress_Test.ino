// ------------------------------------------------------------------------
//  J1939 Stress Test
// ------------------------------------------------------------------------
//
// IMPOPRTANT: Depending on the CAN shield used for this programming sample,
//             please make sure you set the proper CS pin in module can.cpp.
//
//  This Arduino program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.

#include <stdlib.h>
#include <SPI.h>

#include "mcp_can.h"
#include "can_ext.h"

// Simulation variables ---------------------------------------------------
// ------------------------------------------------------------------------
unsigned char msgVehiclePosition[8] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38}; 
unsigned char msgAmbientConditions[8] = {0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31};

/* Timer structure
** --------------------------------------------------------------------- */
struct j1939Timer
{
  int nCount;
  bool bStart;
  bool bExpired;
};

struct j1939Timer pTimerVehiclePosition;
struct j1939Timer pTimerAmbientConditions;

// ------------------------------------------------------------------------
//  SYSTEM: Setup routine runs on power-up or reset
// ------------------------------------------------------------------------
void setup()
{
  // Set the serial interface baud rate
  Serial.begin(115200);
  
  // Initialize the CAN controller
  if (canInitialize(CAN_250KBPS) == CAN_OK)  // Baud rates defined in mcp_can_dfs.h
    Serial.print("CAN Init OK.\n\r\n\r");
  else
    Serial.print("CAN Init Failed.\n\r");
    
  // Initialize the timers
  j1939TimerReset(&pTimerVehiclePosition);
  pTimerVehiclePosition.nCount = 5000;
  pTimerVehiclePosition.bStart = true;
  
  j1939TimerReset(&pTimerAmbientConditions);
  pTimerAmbientConditions.nCount = 1000;
  pTimerAmbientConditions.bStart = true;
   
}// end setup

// ------------------------------------------------------------------------
// Main Loop - Arduino Entry Point
// ------------------------------------------------------------------------
void loop() 
{
  // Establish the timer base
  j1939TimerControl();
  
  // Send out PGN 65629 - Ambient Conditions
  if(pTimerAmbientConditions.bExpired == true)
  {
    j1939TimerReset(&pTimerAmbientConditions);               // Reset the timer 
    j1939Transmit(65269, 6, 0x30, msgAmbientConditions, 8);  // Transmit the message
    pTimerAmbientConditions.nCount = 1;                   // Restart the timer
    pTimerAmbientConditions.bStart = true;
    
  }// end if

  // Send out PGN 65627 - Vehicle Position
  if(pTimerVehiclePosition.bExpired == true)
  {
    j1939TimerReset(&pTimerVehiclePosition);                  // Reset the timer 
    j1939Transmit(65267, 6, 0x20, msgVehiclePosition, 8);  // Transmit the message
    pTimerVehiclePosition.nCount = 1;                      // Restart the timer
    pTimerVehiclePosition.bStart = true;
    
  }// end if

}// end loop

/* ------------------------------------------------------------------------
** J1939 Timer Control
** --------------------------------------------------------------------- */
void j1939TimerControl(void)
{
  delay(1);

  if(pTimerVehiclePosition.bStart == true && pTimerVehiclePosition.bExpired == false)
  {
    if(--pTimerVehiclePosition.nCount == 0)
      pTimerVehiclePosition.bExpired = true;
  }

  if(pTimerAmbientConditions.bStart == true && pTimerAmbientConditions.bExpired == false)
    if(--pTimerAmbientConditions.nCount == 0)
      pTimerAmbientConditions.bExpired = true;

}/* end TimerControl */

/* ------------------------------------------------------------------------
** J1939 Timer Reset
** --------------------------------------------------------------------- */
void j1939TimerReset(struct j1939Timer* pTimer)
{
  pTimer->nCount = 0;
  pTimer->bStart = false;
  pTimer->bExpired = false;

}/* end TimerReset */


