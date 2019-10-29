# MAC-5G-RANGE
L2 layer for POC of 5G-RANGE project


# Instructions:

Compile with g++:

g++ *.cpp -lpthread -std=c++11 


Run with:

sudo ./a.out nEquipments ip1 port1 ... ipN portN MaxNBytes MacAddr [--v] [devname]

Where: 

nEquipments: Number of attached equipments (1 for UEs);

ipN: N-th IP address of destination L1 socket;

portN: port uset for N-th L1 socket connection;

MaxNBytes: Maximum number of bytes allowed in a single PDU;

MaxAddr: MAC5GR Address (0 for BS and 1, 2, ... for UEs);

--v: Verbosity [optional];

devname: Tun interface name [opcional].
