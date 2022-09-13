#include <string>
using namespace std;

class ChanSpecData
{
public:
	unsigned long msgCnt; ///< Message count
	unsigned char *reserved;
	int TTB; ///< Time tag bit
	ChanSpecData()
	{
        msgCnt = 0;
        reserved = nullptr;
        TTB = 0;
		// call bit manipulate function to get the three attributes of this then assign them

	}
	ChanSpecData(unsigned long msgCnt, unsigned char *reserved, int TTB)
	{
		this->msgCnt = msgCnt;
		this->reserved = reserved;
		this->TTB = TTB;
	}
};

class IntraPackHeader
{
public:
	unsigned char *timeStamp; ///< Reference time
	int BSWReserved1; ///< Reserved
    int BSWbusID;
    int BSWMessageError;
    int BSWRT_RT_Transfer;
    int BSWFormatError;
    int BSWTimeout;
    int BSWReserved2;
    int BSWWordCountError;
    int BSWSyncTypeError;
    int BSWInvalidWord;
    int BSWReserved3;
    unsigned long gapTime1;
    unsigned long gapTime2;
	unsigned long msgLen;
	IntraPackHeader ()
	{
        this->timeStamp = nullptr;
        this->BSWReserved1 = 0;
        this->BSWbusID = 0;
        this->BSWRT_RT_Transfer = 0;
        this->BSWMessageError = 0;
        this->BSWFormatError = 0;
        this->BSWTimeout = 0;
        this->BSWReserved2 = 0;
        this->BSWWordCountError = 0;
        this->BSWSyncTypeError = 0;
        this->BSWInvalidWord = 0;
        this->BSWReserved3 = 0;
        this->gapTime1 = 0;
        this->gapTime2 = 0;
        this->msgLen = 0;
	}
    IntraPackHeader(unsigned char *time, int res1, int busID, int RT, int mesERR, int formErr, int timeout, int res2,
                    int wordCountErr, int syncErr, int invalWord, int res3, unsigned long gap1, unsigned long gap2,
                    unsigned long meslen) {

        this->timeStamp = time;
        this->BSWReserved1 = res1;
        this->BSWbusID = busID;
        this->BSWRT_RT_Transfer = RT;
        this->BSWMessageError = mesERR;
        this->BSWFormatError = formErr;
        this->BSWTimeout = timeout;
        this->BSWReserved2 = res2;
        this->BSWWordCountError = wordCountErr;
        this->BSWSyncTypeError = syncErr;
        this->BSWInvalidWord = invalWord;
        this->BSWReserved3 = res3;
        this->gapTime1 = gap1;
        this->gapTime2 = gap2;
        this->msgLen = meslen;
    }
};

class Words
{
private :
    unsigned char *word1;
    unsigned char *word2;
public:
    Words(unsigned char* part1, unsigned char* part2){
    this->word1 = part1;
    this->word2 = part2;
}
};

class Messages {
public:
    vector <IntraPackHeader> intraHeaders;
    vector <Words> commWords;
    vector <unsigned char*> data;
    unsigned long numMessages;
    void addMessage(IntraPackHeader header, Words commandWords, unsigned char* messageData){
        this->intraHeaders.push_back(header);
        this->commWords.push_back(commandWords);
        this->data.push_back(messageData);
    }

};
