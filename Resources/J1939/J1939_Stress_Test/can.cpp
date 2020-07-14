// ------------------------------------------------------------------------
// J1939 CAN Connection
// ------------------------------------------------------------------------
#include "mcp_can.h"

#define CS_PIN                                 9     // Use pin 10 for Seeed Studio CAN Shield up to version 1.0
                                                     // Use pin 9 for Seeed Studion CAN shiled version 1.1 and higher
                                                     // Use pin 10 for SK Pang CAN shield
MCP_CAN CAN0(CS_PIN);

// ------------------------------------------------------------------------
// Initialize the CAN controller
// ------------------------------------------------------------------------
int canInitialize(int nBaudRate)
{
  // Initialize the CAN controller
  if(CAN0.begin(nBaudRate) == 0)
    return 0;
  else return 1;

}// end canInitialize

// ------------------------------------------------------------------------
// Transmit CAN message
// ------------------------------------------------------------------------
byte canTransmit(long lID, unsigned char* pData, int nDataLen)
{
  
  if(CAN0.sendMsgBuf(lID, CAN_EXTID, nDataLen, pData) == 0)
    return 0;
  else
    return 1;
  
}// end canTransmit

// ------------------------------------------------------------------------
// J1939 Transmit
// ------------------------------------------------------------------------
byte j1939Transmit(long lPGN, int nPriority, int nAddress, byte* nData, int nDataLen)
{
  // Declarations
  long lMessageID = ((long)nPriority << 26) + (lPGN << 8) + (long)nAddress;
  
  // Transmit the message
  return canTransmit(lMessageID, nData, nDataLen);
  
}// end j1939Transmit


