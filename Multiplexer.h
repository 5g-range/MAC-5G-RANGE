#pragma once
#include "MacAddressTable.h"
#include "ProtocolPackage.h"
#include <iostream>
using namespace std;

class ProtocolPackage;

class Multiplexer{
private:
    char* buffer;       //Buffer acumulates SDUs
    uint8_t srcAddr;    //Source MAC address
    uint8_t dstAddr;    //Destination MAC address
    uint8_t nSDUs;      //Number of SDUs multiplexed
    int maxNSDUs;   //Maximum number of SDUs multiplexed
    int offset;     //Offset for decoding
    uint16_t* sizesSDUs; //Sizes of each SDU multiplexed
    uint8_t* dcsSDUs;   //Data(1)/Control(0) flag
    bool verbose;  
    int currentBufferLength();     
public:
    Multiplexer(int _nBytes, uint8_t src, uint8_t dst, int maxSDUs, bool v);
    Multiplexer(char* _buffer, uint8_t _nSDUs, uint16_t* _sizesSDUs, uint8_t* dcs_SDUs, bool v);
    ~Multiplexer();
    int nBytes;         //Maximum number of bytes
    int getNumberofBytes();     
    bool addSDU(char* sdu, uint16_t size, uint8_t dc);
    ssize_t getSDU(char* buf);  
    ProtocolPackage* getPDUPackage();   
    void clearBuffer(); 
};