#include <string>
using namespace std;
#include "1553helper.hpp"

class Packet
{
private:
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


public:
    Packet(){
        this->data = nullptr;
        this->channelID = 0;
        this->packetLength = 0;
        this->dataLength = 0;
        this->dataTypeVer = 0;
        this->seqNum = 0;
        this->packetFlags = 0;
        this->dataType = 0;
        this->relativeTimeCounter = nullptr;
        this->headerCheckSum = 0;
    }
	Packet(unsigned char *myPacketsData, unsigned long chanID, unsigned long packLength, unsigned long datLen,
           unsigned long datatypever, unsigned long sequenceNumber, unsigned long packetflags,
           unsigned long myDataType, unsigned char *timeCounter, unsigned long checkSum)
	{
		this->data = myPacketsData;
        this->channelID = chanID;
        this->packetLength = packLength;
        this->dataLength = datLen;
        this->dataTypeVer = datatypever;
        this->seqNum = sequenceNumber;
        this->packetFlags = packetflags;
        this->dataType = myDataType;
        this->relativeTimeCounter = timeCounter;
        this->headerCheckSum = checkSum;
	}

};

class P1553 : public Packet
{
private:
	int msgCount;
	ChanSpecData chanSpec; // uses ChanSpecData class from 1553helper.hpp
	IntraPackHeader intraPHdr; // uses IntraPackHeader class from 1553helper.hpp
	CommandWord CmdWord1; // uses CommandWord class from 1553helper.hpp
	CommandWord CmdWord2; // uses CommandWord class from 1553helper.hpp
	uint16_t* statWord1;
	uint16_t* statWord2;
	uint16_t wordCnt;
	uint16_t* pauData;

public:
    P1553() : Packet() {
        int msgCount = 0;
        ChanSpecData chanSpec = ChanSpecData();
        IntraPackHeader intraPHdr = IntraPackHeader();
        CommandWord CmdWord1 = CommandWord();
        CommandWord CmdWord2 = CommandWord();
        uint16_t* statWord1 = 0;
        uint16_t* statWord2 = 0;
        uint16_t wordCnt = 0;
        uint16_t* pauData = 0;
    }

	P1553(unsigned char* channelData, unsigned char *intraPacketHeader, unsigned char *commWord1,
          unsigned char *commWord2, unsigned char *statusWord1, unsigned char *statusWord2)
          : Packet()
	{
		// call function passing in the DataStream as a parameter that assigns values to msgCount bufferOffset dataLen

		// passing in bufferOffset or dataStream will assign something to ChanSpecData

		// call constructor for command words

        //TEMPORARILY BANISHED TO THE SHADOW REALM
        /*unsigned char *myPacketsData, unsigned char *chanID, unsigned char *packLength,
                   unsigned char *datLen, unsigned char *datatypever, unsigned char *sequenceNumber,
                   unsigned char *packetflags, unsigned char *myDataType, unsigned char *timeCounter,
                   unsigned char *checkSum*/
	}
};
