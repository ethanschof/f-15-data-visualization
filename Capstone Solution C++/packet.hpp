#include <string>
using namespace std;
#include <C:/Users/C23Ethan.Schofield/Source/Repos/ethanschof/f-15-data-visualization/Capstone Solution C++/1553helper.hpp>

class Packet
{
private:
	string data;
	string type;

public:
	Packet(string inData)
	{
		this->data = inData;
	}

	void set_data(string myData)
	{
		this->data = myData;
	}

	void set_type(string newType)
	{
		this->type = newType;
	}

	string get_data() { return this->data; }
	string get_type() { return this->type; }
};

class P1553 : public Packet
{
private:
	int msgCount;
	uint32_t bufferOffset; // Offset into data buffer - also might not need, depends on how we handle buffers
	uint32_t dataLen; // may not need this
	ChanSpecData chanSpec; // uses ChanSpecData class from 1553helper.hpp
	IntraPackHeader psu1553Hdr; // uses IntraPackHeader class from 1553helper.hpp
	CommandWord CmdWord1; // uses CommandWord class from 1553helper.hpp
	CommandWord CmdWord2; // uses CommandWord class from 1553helper.hpp
	uint16_t* statWord1;
	uint16_t* statWord2;
	uint16_t wordCnt;
	uint16_t* pauData;

	P1553 ()
	{
		
	}

public:
};
