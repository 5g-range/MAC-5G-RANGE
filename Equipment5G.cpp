#include "Equipment5G.h"

Equipment5G::Equipment5G(int nEquipments, uint16_t _nB, const char* _devNameTun, MacAddressTable* _arp, uint8_t _macAddr, CoreL1* _l1, bool v){
    attachedEquipments = nEquipments;
    nB = _nB;
    tunIf = new TunInterface(_devNameTun, v);
    verbose = v;
    arp = _arp;
    macAddr = _macAddr;
    l1 = _l1;
    if(!(tunIf->allocTunInterface())){
        if(verbose) cout << "[Equipment5G] Error allocating tun interface." << endl;
        exit(1);
    }
}

Equipment5G::~Equipment5G(){
    delete tunIf;
}

void Equipment5G::encoding(){
    char buf[MAXLINE], buf2[MAXLINE];
    int val;
    QueueManager* q1 = new QueueManager(nB, macAddr, arp, MAXSDUS, verbose);

    //Communication infinite loop
    while(1){
        //Clear buffer
        bzero(buf,sizeof(buf));

        //Receive a packet a packet from TUN
        ssize_t nread = tunIf->readTunInterface(buf, MAXLINE);
        if(nread==0)    //EOF
            break;

        //Check ipv4
        if((buf[0]>>4)&15 != 4){
            cout<<"[Equipment5G] Dropped non-ipv4 packet."<<endl;
            continue;
        }

        //Check broadcast
        if((buf[DST_OFFSET]==255)&&(buf[DST_OFFSET+1]==255)&&(buf[DST_OFFSET+2]==255)&&(buf[DST_OFFSET+3]==255)){
            cout<<"[Equipment5G] Dropped broadcast packet."<<endl;
            continue;
        }

        //Check multicast
        if((buf[DST_OFFSET]>=224)&&(buf[DST_OFFSET]<=239)){
            cout<<"[Equipment5G] Dropped multicast packet."<<endl;
            continue;
        }
        
        val = q1->addSdu(buf, nread);

        if(val == -1) continue;

        if(val == -2){
            if(verbose) cout<<"[Equipment5G] Error!" <<endl;
            break;
        }

        bzero(buf2, sizeof(buf2));

        ssize_t nread2 = q1->getPdu(buf2, val);

        for(int i=0;i<attachedEquipments;i++)
            l1->sendPdu(buf2, nread2, (l1->getPorts())[i]);
        
        q1->addSdu(buf,nread);
    }
    delete q1;
}

void Equipment5G::decoding(uint16_t port){
    char buf[MAXLINE];
    ProtocolPackage* pdu;
    Multiplexer* mux;

    //Communication infinite loop
    while(1){
        //Clear buffer
        bzero(buf,sizeof(buf));

        //Read packet from UE01 Socket
        int r = l1->receivePdu(buf, MAXLINE, port);
        if(r==-1 && verbose){ 
            cout<<"[Equipment5G] Error reading from socket."<<endl;
            break;
        }
        if(r==0) break;

        if(verbose) cout<<"[Equipment5G] Decoding "<<port<<": in progress..."<<endl;

        pdu = new ProtocolPackage(buf, r, verbose);

        if(!(pdu->crcPackageChecking())){
            if(verbose) cout<<"[Equipment5G] Drop package: CRC error."<<endl;
            delete pdu;
            continue;       //Drop packet
        }

        pdu->removeMacHeader();

        if(pdu->getDstMac()!=macAddr){
            if(verbose) cout<<"[Equipment5G] Drop package: Wrong destination."<<endl;
            delete pdu;
            continue;       //Drop packet
        }

        mux = pdu->getMultiplexedSDUs();
        while((r = mux->getSDU(buf))>0){
            if(verbose) cout<<"[Equipment5G] Received from socket. Forwarding to TUN."<<endl;
            tunIf->writeTunInterface(buf, r);
        }
        delete mux;
        delete pdu;
    }
}