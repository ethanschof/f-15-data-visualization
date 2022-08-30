#include <string>
using namespace std;
#include <1553helper.hpp>

class Packet
{
private:
	unsigned char *data;
    unsigned char *channelID;
    unsigned char *packetLength;
    unsigned char *dataLength;
    unsigned char *dataTypeVer;
    unsigned char *seqNum;
    unsigned char *packetFlags;
    unsigned char *dataType;
    unsigned char *relativeTimeCounter;
    unsigned char *headerCheckSum;


public:
    Packet(){
        this->data = nullptr;
        this->channelID = nullptr;
        this->packetLength = nullptr;
        this->dataLength = nullptr;
        this->dataTypeVer = nullptr;
        this->seqNum = nullptr;
        this->packetFlags = nullptr;
        this->dataType = nullptr;
        this->relativeTimeCounter = nullptr;
        this->headerCheckSum = nullptr;
    }
	Packet(unsigned char *myPacketsData, unsigned char *chanID, unsigned char *packLength, unsigned char *datLen,
           unsigned char *datatypever, unsigned char *sequenceNumber, unsigned char *packetflags,
           unsigned char *myDataType, unsigned char *timeCounter, unsigned char *checkSum)
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

	P1553(unsigned char *channelData, unsigned char *intraPacketHeader, unsigned char *commWord1,
          unsigned char *commWord2, unsigned char *statusWord1, unsigned char *statusWord2)
          : Packet(unsigned char *myPacketsData, unsigned char *chanID, unsigned char *packLength,
                   unsigned char *datLen, unsigned char *datatypever, unsigned char *sequenceNumber,
                   unsigned char *packetflags, unsigned char *myDataType, unsigned char *timeCounter,
                   unsigned char *checkSum)
	{
		// call function passing in the DataStream as a parameter that assigns values to msgCount bufferOffset dataLen

		// passing in bufferOffset or dataStream will assign something to ChanSpecData

		// call constructor for command words
	}
};
