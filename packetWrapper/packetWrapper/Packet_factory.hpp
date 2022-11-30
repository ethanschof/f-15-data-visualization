//
// Created by C23Ethan.Schofield on 10/9/2022.
//

#ifndef F_15_DATA_VISUALIZATION_PACKET_FACTORY_HPP
#define F_15_DATA_VISUALIZATION_PACKET_FACTORY_HPP

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

#include "Packet.hpp"
#include <vector>
#include <memory>
using namespace std;
extern long byteIndex;

class Packet_factory {
public:
    static vector<unique_ptr<Packet>> createPackets(unsigned char* data, long* fSize);

    static unsigned char* swapEndian(unsigned char* bytes, int numBytes);

    static int bitsToInt(unsigned char* bits, int numBits);

    static unsigned long bytesToLong(unsigned char* bytes, int numBytes);

    static unsigned long long bytesToLongLong(unsigned char* bytes, int numBytes);

    static unsigned char *bitManipulator(unsigned char* data, unsigned long numBits, long *fSize);

    static unsigned char *bitManipulator(unsigned char* data, unsigned long numBits);

    static Packet getOnePacket(unsigned char* data, long* fSize);
};



#endif //F_15_DATA_VISUALIZATION_PACKET_FACTORY_HPP
