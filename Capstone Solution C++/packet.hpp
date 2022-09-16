#include <string>
#include <vector>
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
    Packet(unsigned long chanID, unsigned long packLength, unsigned long datLen,
           unsigned long datatypever, unsigned long sequenceNumber, unsigned long packetflags,
           unsigned long myDataType, unsigned char *timeCounter, unsigned long checkSum)
    {
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
	Messages messages; // uses Message class from 1553helper.hpp
	uint16_t wordCnt;
	uint16_t* pauData;

public:
    P1553() : Packet() {
        int msgCount = 0;
        ChanSpecData chanSpec = ChanSpecData();
        Messages messages;
    }

    P1553(unsigned char *myPacketsData, unsigned long chanID, unsigned long packLength, unsigned long datLen,
          unsigned long datatypever, unsigned long sequenceNumber, unsigned long packetflags,
          unsigned long myDataType, unsigned char *timeCounter, unsigned long checkSum)
          : Packet(myPacketsData, chanID, packLength, datLen, datatypever,
                   sequenceNumber, packetflags, myDataType, timeCounter, checkSum){
        int msgCount = 0;
        ChanSpecData chanSpec = ChanSpecData();
        Messages messages;
    }

	P1553(unsigned long chanID, unsigned long packLength, unsigned long datLen,
          unsigned long datatypever, unsigned long sequenceNumber, unsigned long packetflags,
          unsigned long myDataType, unsigned char *timeCounter, unsigned long checkSum,
          unsigned long msgCount, ChanSpecData channelData, Messages packMessages)
          : Packet(chanID, packLength, datLen, datatypever,
                   sequenceNumber, packetflags, myDataType, timeCounter, checkSum)
	{
		this->chanSpec = channelData;
        this->msgCount = msgCount;
        this->messages = packMessages;

	}

    unsigned char* swapEndian(unsigned char* bytes, int numBytes){
        unsigned char* BEBytes = (unsigned char*)malloc(numBytes * sizeof(unsigned char));
        for(int i = 0; i < numBytes; i++){
            BEBytes[(numBytes-1)-i] = bytes[i];
            bytes[i] = BEBytes[(numBytes-1)-i];
        }
        return BEBytes;
    }

    unsigned long bytesToLong(unsigned char* bytes, int numBytes){
        long totalVal = 0;

        if(numBytes <= 4){
            numBytes--;

            for(int i = numBytes; i >= 0; i--){
                //parse each byte (start LSB - highest array value)
                for(int j = 0; j < 8; j++){
                    //parse each bit
                    long bitVal = (long)pow(2, (((numBytes-i)*8)+j));

                    if(bytes[i]&(1 << j)){
                        totalVal = totalVal + bitVal;
                    }
                }
            }
        }else{
            cout << "ERROR: Too many Bytes to process; returning 0...\n";
        }
        return totalVal;
    }

    unsigned char *bitManipulator(unsigned char* data, unsigned long numBits, long *fSize){
        unsigned long numBytes = numBits / 8;
        unsigned long bitShift = 0;

        if(numBits%8 != 0){
            //deal with truncation
            numBytes++;
            bitShift = 1;
        }

        unsigned char *desiredBits = (unsigned char *)malloc(numBytes * sizeof(unsigned char));
        for(unsigned long i = 0; i < numBytes; i++){
            desiredBits[i] = data[i];
        }

        if(bitShift){
            //for non-byte sized needs...
            desiredBits[numBytes-1] = desiredBits[numBytes-1] >> (8 - numBits%8);
            data[numBytes-1] = data[numBytes-1] << (numBits%8);
            numBytes--;
        }

        //remove used bytes
        for(unsigned long i = 0; i < *fSize - numBytes; i++){
            data[i] = data[i+numBytes];
        }
        *fSize = *fSize - numBytes;
        return desiredBits;
    }

    void interpretData(bool print){

        // iterate through all messages in the packet
        for (int messageNumber = 0; messageNumber < this->messages.intraHeaders.size(); ++messageNumber) {
            // Get the command word for this message
            unsigned char *commandWord = this->messages.commWords.at(messageNumber).word1;

            unsigned char *data = this->messages.data.at(messageNumber);
            // need to get fSize for data
            int msgLength = this->messages.intraHeaders.at(messageNumber).msgLen;
            long bitsleft = (long)(msgLength * 8) - 32;

            long *fSize = &bitsleft;

            if (commandWord[0] == 0x40 && commandWord[1] == 0x35){
                // Word 1, True Airspeed 2 bytes w/ valid bit
                unsigned char* trueAirspeed = bitManipulator(data, 16, fSize);
                trueAirspeed = swapEndian(trueAirspeed, 2);

                // Get the first bit to see if the data is valid
                bool airspeedValid = trueAirspeed[0] >> 7;

                // Turn the first bit into a 0 so it doesn't effect our value
                trueAirspeed[0] = trueAirspeed[0]&0x7F;
                // Convert the characters to a usable long value
                unsigned long trueAirspeedValue = bytesToLong(trueAirspeed, 2);

                // Word 2, Angle of attack 2 bytes w/ valid bit
                unsigned char* angleAttack = bitManipulator(data, 16, fSize);
                angleAttack = swapEndian(angleAttack, 2);

                bool angleAttackValid = angleAttack[0] >> 7;

                angleAttack[0] = angleAttack[0]&0x7F;

                unsigned long angleAttackValue = bytesToLong(angleAttack, 2);

                // Word 3, Mach Number 2 bytes w/ valid bit
                unsigned char* machNum = bitManipulator(data, 16, fSize);
                machNum = swapEndian(machNum, 2);

                bool machNumValid = machNum[0] >> 7;

                machNum[0] = machNum[0]&0x7F;

                unsigned long machNumValue = bytesToLong(machNum, 2);

                // Word 4, Pitch angle 2 bytes
                unsigned char* pitchAngle = bitManipulator(data, 16, fSize);
                pitchAngle = swapEndian(pitchAngle, 2);

                unsigned long pitchAngleValue = bytesToLong(pitchAngle, 2);

                // Word 5 Roll Angle 2 bytes
                unsigned char* rollAngle = bitManipulator(data, 16, fSize);
                rollAngle = swapEndian(rollAngle, 2);

                unsigned long rollAngleValue = bytesToLong(rollAngle, 2);

                // Word 6 Roll Rate 2 bytes
                unsigned char* rollRate = bitManipulator(data, 16, fSize);
                rollRate = swapEndian(rollRate, 2);

                unsigned long rollRateValue = bytesToLong(rollRate, 2);

                // Word 7 Pitch Rate 2 bytes
                unsigned char* pitchRate = bitManipulator(data, 16, fSize);
                pitchRate = swapEndian(pitchRate, 2);

                unsigned long pitchRateValue = bytesToLong(pitchRate, 2);

                // Word 8 Yaw Rate 2 bytes
                unsigned char* yawRate = bitManipulator(data, 16, fSize);
                yawRate = swapEndian(yawRate, 2);

                unsigned long yawRateValue = bytesToLong(yawRate, 2);

                // Word 9 Roll Acceleration 2 bytes, bits 14-15 spare
                unsigned char* rollAcceleration = bitManipulator(data, 16, fSize);
                rollAcceleration = swapEndian(rollAcceleration, 2);

                // Ensure that bits 14-15 are zeroed out
                rollAcceleration[0] = rollAcceleration[0]&0x6F;

                unsigned long rollAccelerationValue = bytesToLong(rollAcceleration, 2);

                // Word 10 Pitch Acceleration 2 bytes, bits 12-15 spare
                unsigned char* pitchAcceleration = bitManipulator(data, 16, fSize);
                pitchAcceleration = swapEndian(pitchAcceleration, 2);

                // Ensure that bits 12-15 are zeroed out
                pitchAcceleration[0] = pitchAcceleration[0]&0x4F;

                unsigned long pitchAccelerationValue = bytesToLong(pitchAcceleration, 2);

                // Word 11 Yaw Acceleration 2 bytes, bits 12-15 spare
                unsigned char* yawAcceleration = bitManipulator(data, 16, fSize);
                yawAcceleration = swapEndian(yawAcceleration, 2);

                // Ensure that bits 12-15 are zeroed out
                yawAcceleration[0] = yawAcceleration[0]&0x4F;

                unsigned long yawAccelerationValue = bytesToLong(yawAcceleration, 2);

                // Word 12 Longitudinal Acceleration 2 bytes
                unsigned char* longitudinalAccel = bitManipulator(data, 16, fSize);
                longitudinalAccel = swapEndian(longitudinalAccel, 2);

                unsigned long longitudinalAccelValue = bytesToLong(longitudinalAccel, 2);

                // Word 13 Lateral Acceleration 2 bytes
                unsigned char* lateralAccel = bitManipulator(data, 16, fSize);
                lateralAccel = swapEndian(lateralAccel, 2);

                unsigned long lateralAccelValue = bytesToLong(lateralAccel, 2);

                // Word 14 Roll Rate AFCS 2 bytes
                unsigned char* rollRateAFCS = bitManipulator(data, 16, fSize);
                rollRateAFCS = swapEndian(rollRateAFCS, 2);

                unsigned long rollRateAFCSValue = bytesToLong(rollRateAFCS, 2);

                // Word 15 Lateral Stick Force 2 bytes
                unsigned char* lateralStickForce = bitManipulator(data, 16, fSize);
                lateralStickForce = swapEndian(lateralStickForce, 2);

                unsigned long lateralStickForceValue = bytesToLong(lateralStickForce, 2);

                // Word 16 Longitudinal Stick Force 2 bytes
                unsigned char* longitudinalStickForce = bitManipulator(data, 16, fSize);
                longitudinalStickForce = swapEndian(longitudinalStickForce, 2);

                unsigned long longitudinalStickForceValue = bytesToLong(longitudinalStickForce, 2);

                // Word 17 Right Stabilator Deflection 2 bytes
                unsigned char* rStabilatorDef = bitManipulator(data, 16, fSize);
                rStabilatorDef = swapEndian(rStabilatorDef, 2);

                unsigned long rStabilatorDeflectionValue = bytesToLong(rStabilatorDef, 2);

                // Word 18 Left Stabilator Deflection 2 bytes
                unsigned char* lStabilatorDef = bitManipulator(data, 16, fSize);
                lStabilatorDef = swapEndian(lStabilatorDef, 2);

                unsigned long lStabilatorDeflectionValue = bytesToLong(lStabilatorDef, 2);

                // Word 19 Normal Acceleration 2 bytes
                unsigned char* normalAccel = bitManipulator(data, 16, fSize);
                normalAccel = swapEndian(normalAccel, 2);

                unsigned long normalAccelValue = bytesToLong(normalAccel, 2);

                // Word 20 Angle of Sideslip 2 bytes w/ data is invalid byte
                unsigned char* sideslipAngle = bitManipulator(data, 16, fSize);
                sideslipAngle = swapEndian(sideslipAngle, 2);

                bool sideslipAngleINVALID = sideslipAngle[0] >> 7;

                sideslipAngle[0] = sideslipAngle[0]&0x7F;

                unsigned long sideslipAngleValue = bytesToLong(sideslipAngle, 2);

                // Word 21 Dynamic pressure 2 bytes
                unsigned char* dynamicPressure = bitManipulator(data, 16, fSize);
                dynamicPressure = swapEndian(dynamicPressure, 2);

                unsigned long dynamicPressureValue = bytesToLong(dynamicPressure, 2);

                // display the data we've processed
                if (print){
                    cout << "Message " << messageNumber << "\n\n";

                    if (airspeedValid){
                        cout << "True Airspeed: " << trueAirspeedValue << " knots\n";

                    } else {
                        cout << "True Airspeed data invalid\n";
                    }

                    if (angleAttackValid){
                        cout << "Angle of Attack: " << angleAttackValue << " BAMS\n";
                    } else {
                        cout << "Angle of Attack data is invalid\n";
                    }

                    if (machNumValid){
                        cout << "Mach Number: " << machNumValue << "\n";
                    } else {
                        cout << "Mach Number data invalid\n";
                    }

                    cout << "Pitch Angle: " << pitchAngleValue << " BAMS\n";
                    cout << "Roll Angle: " << rollAngleValue << " BAMS\n";
                    cout << "Roll Rate: " << rollRateValue << " Deg/s\n";
                    cout << "Pitch Rate: " << pitchRateValue << " Deg/s\n";
                    cout << "Yaw Rate: " << yawRateValue << " Deg/s\n";
                    cout << "Roll Acceleration: " << rollAccelerationValue << " Deg/s^2\n";
                    cout << "Pitch Acceleration: " << pitchAccelerationValue << " Deg/s^2\n";
                    cout << "Yaw Acceleration: " << yawAccelerationValue << " Deg/s^2\n";
                    cout << "Longitudinal Acceleration: " << longitudinalAccelValue << " ft/s^2\n";
                    cout << "Lateral Acceleration: " << lateralAccelValue << " ft/s^2\n";
                    cout << "Roll Rate AFCS: " << rollRateAFCSValue << " Deg/s\n";
                    cout << "Lateral Stick Force: " << lateralStickForceValue << " lbs\n";
                    cout << "Longitudinal Stick Force: " << longitudinalStickForceValue << " lbs\n";
                    cout << "Right Stabilator Deflection: " << rStabilatorDeflectionValue << " BAMS\n";
                    cout << "Left Stabilator Deflection: " << lStabilatorDeflectionValue << " BAMS\n";
                    cout << "Normal Acceleration: " << normalAccelValue << " G\n";

                    if (!sideslipAngleINVALID){
                        cout << "Angle of Sideslip: " << sideslipAngleValue << " BAMS\n";
                    } else {
                        cout << "Angle of Sideslip data invalid\n";
                    }

                    cout << "Dynamic Pressure: " << dynamicPressureValue << " lb/ft^2\n\n";

                } // end of printing for command word 0x4035


            } else if (commandWord[0] == 0x40 && commandWord[1] == 0x55){
                // Word 1 Left Engine Total Fuel FLow Lb/Hr 2 bytes
                unsigned char* leftEngineFuelFlow = bitManipulator(data, 16, fSize);
                leftEngineFuelFlow = swapEndian(leftEngineFuelFlow, 2);

                unsigned long leftEngineFuelFlowValue = bytesToLong(leftEngineFuelFlow, 2);

                // Word 2 Right Engine Total Fuel Flow lb/hr 2 bytes

                // Word 3

                // Word 4

                // Word 5

                // Word 6 Discrete word 04 2 bytes, 16 bool values
                unsigned char* discreteWord4 = bitManipulator(data, 16, fSize);
                discreteWord4 = swapEndian(discreteWord4, 2);

                long *wordSize = (long*)malloc(sizeof(long));
                *wordSize = 2;

                // Grab bit 15
                unsigned char* tmp =  bitManipulator(discreteWord4, 1, wordSize);
                bool radarLowSpeedCaution = tmp [0];

                // Grab bit 14
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarJamWarning = tmp [0];

                // Grab bit 13
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarNoTerrainWarning = tmp [0];

                // 12
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarLimitWarning = tmp [0];

                // 11
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarDiveAngleWarning = tmp[0];

                // 10
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarTurnAccelWarning = tmp[0];

                // 9
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarTurnRateWarning = tmp[0];

                // 8
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarRollAngleWarning = tmp[0];

                // After eating a 8 bits, you must delete it
                unsigned char* trash = bitManipulator(discreteWord4, 8, wordSize);

                // 7
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarRightTurnCaution = tmp[0];

                // 6
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarLeftTurnCaution = tmp[0];

                // 5
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarSetClearanceWarning = tmp[0];

                // 4
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarGLimitWarning = tmp[0];

                // 3
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarObstacleWarning = tmp[0];

                // 2
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool radarDataGood = tmp[0];

                // 1
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool afcsGCommandValid = tmp[0];

                // 0
                tmp = bitManipulator(discreteWord4, 1, wordSize);
                bool displayGCommandValid = tmp[0];

                // Eat garbage
                trash = bitManipulator(discreteWord4, 8, wordSize);
                free(trash);

                // Word 7 Stores Data Word 1 2 bytes
                unsigned char* storeWord1 = bitManipulator(data, 16, fSize);
                storeWord1 = swapEndian(storeWord1, 2);

                *wordSize = 2;

                // Bits 15 - 12 spare
                unsigned char* spare = bitManipulator(storeWord1, 4, wordSize);

                // Bit 11
                tmp = bitManipulator(storeWord1, 1, wordSize);
                bool station8PylonPresent = tmp[0];

                // Bit 10
                tmp = bitManipulator(storeWord1, 1, wordSize);
                bool station5PylonPresent = tmp[0];

                // bit 9

                // bit 8 spare bit to destroy

                // Eat the garbage byte

                // Bit 7-5 spare bits

                // bit 4

                // bit 3

                // bit 2

                // bit 1

                // bit 0

                // Word 8

                // Word 9

                // Word 10

                // Word 11

                // Word 12

                // Word 13

                // Word 14

                // Word 15

                // Word 16

                // Word 17

                // Word 18

                // Word 19

                // Word 20

                // Word 21

                // Display the processed data
                if (print){

                }

            } else if (commandWord[0] == 0x40 && commandWord[1] == 0x6B){

            } else if (commandWord[0] == 0x40 && commandWord[1] == 0x90){

            } else if (commandWord[0] == 0x40 && commandWord[1] == 0xB0){

            } else if (commandWord[0] == 0x40 && commandWord[1] == 0xD3){

            } else if (commandWord[0] == 0x40 && commandWord[1] == 0xE8){

            } else {
                // We cannot interpret any other command words
            }

        }



    }
};
