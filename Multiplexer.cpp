#include "Multiplexer.h"

Multiplexer::Multiplexer(int _nBytes, uint8_t src, uint8_t dst, int maxSDUs, bool v){
    nBytes = _nBytes;
    buffer = new char[nBytes];
    srcAddr = src;
    dstAddr = dst;
    nSDUs = 0;
    maxNSDUs = maxSDUs;
    sizesSDUs = new uint16_t[maxNSDUs];
    dcsSDUs = new uint8_t[maxNSDUs];
    verbose = v;
}

Multiplexer::Multiplexer(char* _buffer, uint8_t _nSDUs, uint16_t* _sizesSDUs, uint8_t* _dcsSDUs, bool v){
    offset = 0;
    buffer = _buffer;
    nSDUs = _nSDUs;
    sizesSDUs = _sizesSDUs;
    dcsSDUs = _dcsSDUs;
    verbose = v; 
}

Multiplexer::~Multiplexer(){
    delete[] buffer;
    delete[] sizesSDUs;
    delete[] dcsSDUs;
}

int Multiplexer::currentBufferLength(){
    int n=0;
    for(int i=0;i<nSDUs;i++){
        n+=sizesSDUs[i];
    }
    return n;
}

//Returns the actual PDU length in bytes.
int Multiplexer::getNumberofBytes(){
    int n=0;
    //Header length:
    n = 2 + 2*nSDUs;
    n+=currentBufferLength();
    return n;
}

//Adds SDU to the multiplexing queue.
bool Multiplexer::addSDU(char* sdu, uint16_t size, uint8_t dc){
    if((size+2+getNumberofBytes())>nBytes){
        if(verbose) cout<<"[Multiplexer] Tried to multiplex SDU which size extrapolates nBytes."<<endl;
        return false;
    }
    if(nSDUs == maxNSDUs){
        if(verbose) cout<<"[Multiplexer] Tried to multiplex more SDUs than supported."<<endl;
        return false;
    }
    sizesSDUs[nSDUs] = size;
    dcsSDUs[nSDUs] = dc;
    int length = currentBufferLength();
    for(int i=0;i<size;i++)
        buffer[i+length] = sdu[i];
    nSDUs++;
    if(verbose) cout<<"[Multiplexer] Multiplexed "<<(int)nSDUs<<" SDUs into PDU."<<endl;
    return true;
}

//Returns the next SDU multiplexed. Returns 0 for EOF.
//Provisional: returns -1 for control.
ssize_t Multiplexer::getSDU(char* buf){
    if(offset == nSDUs){
        if(verbose) cout<<"[Multiplexer] End of demultiplexing."<<endl;
        return 0;
    }
    if(dcsSDUs[offset]==0){
        if(verbose) cout<<"[Multiplexer] Control SDU."<<endl;
        return -1;
    }
    int positionBuffer = 0;
    for(int i=0;i<offset;i++)
        positionBuffer+=sizesSDUs[i];
    for(int i=0;i<sizesSDUs[offset];i++)
        buf[i] = buffer[positionBuffer+i];
    if(verbose) cout<<"[Multiplexer] Demultiplexed SDU "<<offset+1<<endl;
    offset++;
    return sizesSDUs[offset-1];
}

//Returns the ProtocolPackage for the SDUs multiplexed.
//Requires clearing the buffer before using this multiplexer again.
ProtocolPackage* Multiplexer::getPDUPackage(){
    ProtocolPackage* pdu = new ProtocolPackage(srcAddr, dstAddr, nSDUs, sizesSDUs, dcsSDUs, buffer, verbose);
    return pdu;
}

//Clears the multiplexer buffer. Make sure the PDU is sent before clearing the buffer.
void Multiplexer::clearBuffer(){
    this->~Multiplexer();
    buffer = new char[nBytes];
    sizesSDUs = new uint16_t[maxNSDUs];
    dcsSDUs = new uint8_t[maxNSDUs];
    nSDUs=0;
}


