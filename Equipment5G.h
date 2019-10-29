#pragma once
#include <iostream> //std::cout
#include <future>   //std::async, std::future
#include <chrono>   //std::chrono::milisseconds

#include "TunInterface.h"
#include "MacAddressTable.h"
#include "CoreL1.h"
#include "ProtocolPackage.h"
#include "Multiplexer.h"
#include "QueueManager.h"

using namespace std;

#define MAXSDUS 20
#define MAXLINE 2048
#define SRC_OFFSET 12
#define DST_OFFSET 16

class Equipment5G{
private:
    int attachedEquipments;
    uint16_t nB;
    uint8_t macAddr;
    TunInterface* tunIf;
    bool verbose;
    MacAddressTable* arp;
    CoreL1* l1;
public:
    Equipment5G(int nEquipments, uint16_t _nB, const char* _devNameTun, MacAddressTable* _arp, uint8_t _macAddr, CoreL1* _l1, bool v);
    ~Equipment5G();
    void encoding();
    void decoding(uint16_t port);
};