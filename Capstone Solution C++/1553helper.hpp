#include <string>
using namespace std;

class ChanSpecData
{
public:
	uint32_t msgCnt : 24; ///< Message count
	uint32_t reserved : 6;
	uint32_t TTB : 2; ///< Time tag bit
	ChanSpecData()
	{
        msgCnt = 0;
        reserved = 0;
        TTB = 0;
		// call bit manipulate function to get the three attributes of this then assign them

	}
	ChanSpecData(uint32_t msgCnt, uint32_t reserved, uint32_t TTB)
	{
		this->msgCnt = msgCnt;
		this->reserved = reserved;
		this->TTB = TTB;
	}
};

class IntraPackHeader
{
public:
	uint8_t aubyIntPktTime[8]; ///< Reference time
	uint16_t Reserved1 : 3; ///< Reserved
	uint16_t bWordError : 1;
	uint16_t bSyncError : 1;
	uint16_t bWordCntError : 1;
	uint16_t Reserved2 : 3;
	uint16_t bRespTimeout : 1;
	uint16_t bFormatError : 1;
	uint16_t bRT2RT : 1;
	uint16_t bMsgError : 1;
	uint16_t iBusID : 1;
	uint16_t Reserved3 : 2;
	uint8_t uGapTime1;
	uint8_t uGapTime2;
	uint16_t uMsgLen;
	IntraPackHeader ()
	{
		
	}
};

class CommandWord
{
public:
};
