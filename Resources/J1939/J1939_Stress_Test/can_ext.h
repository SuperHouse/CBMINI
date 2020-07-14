#ifndef CAN_EXT_H
#define CAN_EXT_H

extern int canInitialize(int nBaudRate);
extern byte canTransmit(long lID, unsigned char* pData, int nDataLen);
byte j1939Transmit(long lPGN, int nPriority, int nAddress, byte* nData, int nDataLen);

#endif
