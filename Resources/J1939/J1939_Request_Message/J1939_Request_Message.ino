// ------------------------------------------------------------------------
//  J1939 Request Messages
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

// Simulation parameters --------------------------------------------------
// ------------------------------------------------------------------------
#define PGN_RequestMessage                      0x00EA00
#define PGN_EngineTemperature                   0x00FEEE
#define PGN_EngineTemperatureMSB                0x00
#define PGN_EngineTemperature2ND            	0xFE
#define PGN_EngineTemperatureLSB                0xEE

#define NODE_Request                            0x20
#define NODE_Response                           0x30

unsigned char msgEngineTemperature[8] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38}; 
unsigned char msgRequest[] = {PGN_EngineTemperatureLSB, PGN_EngineTemperature2ND, PGN_EngineTemperatureMSB};

// ------------------------------------------------------------------------
//  Check serial monitor for user input
// ------------------------------------------------------------------------
bool ReadSerialMonitorString(void)
{
  bool bRetCode = false;
  char sString[2];
  
  if(Serial.available() > 0)
  {
    Serial.setTimeout(1);
    Serial.readBytes(sString, 1);  
    bRetCode = true;  
  }
  
  return bRetCode;

}// end ReadSerialMonitorString

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
    
}// end setup

// ------------------------------------------------------------------------
// Main Loop - Arduino Entry Point
// ------------------------------------------------------------------------
void loop() 
{
  // Declarations
  byte nPriority;
  byte nSrcAddr;
  byte nDestAddr;
  byte nData[8];
  int nDataLen;
  long lPGN;
  
  char sString[80];
  
  // Check for received J1939 messages
  if(j1939Receive(&lPGN, &nPriority, &nSrcAddr, &nDestAddr, nData, &nDataLen) == 0)
  {
    // Display the received message on the serial monitor 
    sprintf(sString, "PGN: 0x%X Src: 0x%X Dest: 0x%X ", (int)lPGN, nSrcAddr, nDestAddr);
    Serial.print(sString);
    if(nDataLen == 0 )
      Serial.print("No Data.\n\r");
    else
    {
      Serial.print("Data: ");
      for(int nIndex = 0; nIndex < nDataLen; nIndex++)
      {          
        sprintf(sString, "0x%X ", nData[nIndex]);
        Serial.print(sString);
        
      }// end for
      Serial.print("\n\r");
      
    }// end else
    
    // Analyze the received PGN
    switch(lPGN)
    {
      case PGN_RequestMessage:
      
        if(nData[0] == PGN_EngineTemperatureLSB
        && nData[1] == PGN_EngineTemperature2ND
        && nData[2] == PGN_EngineTemperatureMSB)
        {
           // Request message for engine temperature was received
           j1939Transmit(PGN_EngineTemperature, 6, NODE_Response, NODE_Request, msgEngineTemperature, 8);  // Transmit the message
        }
      
        break;
        
      // Add more 'case' statements here...
      
    }// end switch
        
  }// end if
  
  // Check for user input per serial monitor
  // Send request message with user input
  if(ReadSerialMonitorString() == true)
    j1939Transmit(PGN_RequestMessage, 6, NODE_Request, NODE_Response, msgRequest, 3);

}// end loop



