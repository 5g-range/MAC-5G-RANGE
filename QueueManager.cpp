#include "QueueManager.h"

QueueManager::QueueManager(uint16_t nB, uint8_t _srcMac, MacAddressTable* _arp, int _maxSDUs, bool v){
    multiplexers = new Multiplexer*[MAX_BUFFERS];
    srcMac = _srcMac;
    dstMac = new uint8_t[MAX_BUFFERS];
    nBytes = new uint16_t[MAX_BUFFERS];
    maxNBytes = nB;
    nMultiplexers = 0;
    arp = _arp;
    maxSDUs = _maxSDUs;
    verbose = v;
    if(v) cout<<"[QueueManager] Created successfully."<<endl;
}

QueueManager::~QueueManager(){
    for(int i=0;i<nMultiplexers;i++)
        delete multiplexers[i];
    delete[] multiplexers;
    delete[] dstMac;
    delete[] nBytes; 
}

int QueueManager::addSdu(char* sdu, uint16_t n){
    int i;
    //First, see if there's a multiplexer ready
    uint8_t ipAddr[4];
    for(int i=0;i<4;i++)
        ipAddr[i] = (uint8_t) sdu[DST_OFFSET+i]; //Copying IP address
    uint8_t mac = arp->getMacAddress(ipAddr);
    for(i=0;i<nMultiplexers;i++)
        if(dstMac[i]==mac)
            break;

    if(i==nMultiplexers){   //Means there's no multiplexer for this address
        if(nMultiplexers>MAX_BUFFERS && verbose) cout<<"[QueueManager] Trying to create more buffers than supported."<<endl;
        multiplexers[i] = new Multiplexer(maxNBytes, srcMac, mac, maxSDUs, verbose);
        dstMac[i] = mac;
        nBytes[i] = 0;
        nMultiplexers++;
    }

    if((n + 2 + multiplexers[i]->getNumberofBytes())>maxNBytes){
        if(verbose) cout<<"[QueueManager] Number of bytes exceed buffer max length. Returning index."<<endl;
        return i;
    }

    if(multiplexers[i]->addSDU(sdu, n, 1)){
        nBytes[i]+=n;
        if(verbose) cout<<"[QueueManager] SDU added to queue!"<<endl;
        return -1;
    }
    return -2; 
}

ssize_t QueueManager::getPdu(char* buffer, int index){
    ssize_t size;
    if(index>=nMultiplexers){
        if(verbose) cout<<"[QueueManager] Could not get PDU: index out of bounds."<<endl;
        return -1;
    }
    if(nBytes[index] == 0){
        if(verbose) cout<<"[QueueManager] Could not get PDU: no Bytes to transfer."<<endl;
        return -1;
    }
    ProtocolPackage* pdu = multiplexers[index]->getPDUPackage();
    if(verbose) cout<<"[QueueManager] Inserting MAC Header and CRC."<<endl;
    pdu->insertMacHeader();
    size = pdu->getPduSize();
    memcpy(buffer, pdu->buffer, size);
    delete pdu;

    multiplexers[index]->clearBuffer();
    nBytes[index] = 0;

    return size;
}

bool QueueManager::emptyPdu(int index){
    if(index>=nMultiplexers) return true;
    return nBytes[index]==0;
}

