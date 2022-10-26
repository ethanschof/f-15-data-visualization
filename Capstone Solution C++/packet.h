//
// Created by C23Ethan.Schofield on 10/7/2022.
//

#ifndef F_15_DATA_VISUALIZATION_PACKET_H
#define F_15_DATA_VISUALIZATION_PACKET_H

#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;
#include "ChanSpecData.h"
#include "Messages.h"

#define PACKET_SYNC_LENGTH 16
#define CHAN_ID_LENGTH 16
#define PACKET_LENGTH_LENGTH 32
#define DATA_LENGTH 32
#define DATA_TYPE_VERSION_LENGTH 8
#define SEQ_NUM_LENGTH 8
#define PACKET_FLAGS_LENGTH 8
#define DATA_TYPE_BIT_LENGTH 8
#define RELATIVE_TIME_COUNTER_LENGTH 48
#define HEADER_CHECKSUM_LENGTH 16

class Packet
{
public:

    int msgCount;
    unsigned char *data;
    unsigned long channelID;
    unsigned long packetLength;
    unsigned long dataLength;
    unsigned long dataTypeVer;
    unsigned long seqNum;
    unsigned long packetFlags;
    unsigned long dataType;
    unsigned char *relativeTimeCounter;
    unsigned long headerCheckSum;
    ChanSpecData chanSpec;
    Messages messages;
    uint16_t wordCnt;
    uint16_t* pauData;

    Packet();

    Packet(unsigned char *myPacketsData, unsigned long chanID, unsigned long packLength, unsigned long datLen,
           unsigned long datatypever, unsigned long sequenceNumber, unsigned long packetflags,
           unsigned long myDataType, unsigned char *timeCounter, unsigned long checkSum);

    Packet(unsigned long chanID, unsigned long packLength, unsigned long datLen,
           unsigned long datatypever, unsigned long sequenceNumber, unsigned long packetflags,
           unsigned long myDataType, unsigned char *timeCounter, unsigned long checkSum,
           unsigned long msgCount, ChanSpecData channelData, Messages packMessages);

    unsigned char* swapEndian(unsigned char* bytes, int numBytes);

    int bitsToInt(unsigned char* bits, int numBits);

    static unsigned long bytesToLong(unsigned char* bytes, int numBytes);

    static float bytesToFloat(unsigned char* bytes, int numBytes);

    unsigned long long bytesToLongLong(unsigned char* bytes, int numBytes);

    unsigned char *bitManipulator(unsigned char* data, unsigned long numBits, long *fSize);

    unsigned char *bitManipulator(unsigned char* data, unsigned long numBits);

    bool isPrintable();

    void interpretData(bool print);

};


#endif //F_15_DATA_VISUALIZATION_PACKET_H