//
// Created by C23Ethan.Schofield on 10/9/2022.
//

#ifndef F_15_DATA_VISUALIZATION_PACKET_FACTORY_H
#define F_15_DATA_VISUALIZATION_PACKET_FACTORY_H

#include "packet.h"
#include <vector>
#include <memory>
using namespace std;

extern long byteIndex;

vector<unique_ptr<Packet>> createPackets(unsigned char* data, long* fSize);

vector<unique_ptr<Packet>> createPacketsTestData(unsigned char* data, long* fSize);

unsigned char* swapEndian(unsigned char* bytes, int numBytes);

int bitsToInt(unsigned char* bits, int numBits);

unsigned long bytesToLong(unsigned char* bytes, int numBytes);

unsigned long long bytesToLongLong(unsigned char* bytes, int numBytes);

unsigned char *bitManipulator(unsigned char* data, unsigned long numBits, long *fSize);

unsigned char *bitManipulator(unsigned char* data, unsigned long numBits);

#endif //F_15_DATA_VISUALIZATION_PACKET_FACTORY_H
