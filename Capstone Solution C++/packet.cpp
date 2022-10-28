#include "packet.h"

#include <utility>

Packet::Packet() {
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
    this->msgCount = 0;
}

Packet::Packet(unsigned char *myPacketsData, unsigned long chanID, unsigned long packLength, unsigned long datLen,
       unsigned long datatypever, unsigned long sequenceNumber, unsigned long packetflags,
       unsigned long myDataType, unsigned char *timeCounter, unsigned long checkSum):
    data(myPacketsData),
    channelID(chanID),
    packetLength(packLength),
    dataLength(datLen),
    dataTypeVer(datatypever),
    seqNum(sequenceNumber),
    packetFlags(packetflags),
    dataType(myDataType),
    relativeTimeCounter(timeCounter),
    headerCheckSum(checkSum),
    msgCount(0),
    chanSpec(ChanSpecData()),
    messages(Messages())
{
}

Packet::Packet(unsigned long chanID, unsigned long packLength, unsigned long datLen,
       unsigned long datatypever, unsigned long sequenceNumber, unsigned long packetflags,
       unsigned long myDataType, unsigned char *timeCounter, unsigned long checkSum,
       unsigned long msgCount, ChanSpecData channelData, Messages packMessages):
    channelID(chanID),
    packetLength(packLength),
    dataLength(datLen),
    dataTypeVer(datatypever),
    seqNum(sequenceNumber),
    packetFlags(packetflags),
    dataType(myDataType),
    relativeTimeCounter(timeCounter),
    headerCheckSum(checkSum),
    chanSpec(channelData),
    msgCount(msgCount),
    messages(std::move(packMessages)){
}

unsigned char* Packet::swapEndian(unsigned char* bytes, int numBytes){
    unsigned char* BEBytes = (unsigned char*)malloc(numBytes * sizeof(unsigned char));
    for(int i = 0; i < numBytes; i++){
        BEBytes[(numBytes-1)-i] = bytes[i];
        bytes[i] = BEBytes[(numBytes-1)-i];
    }
    return BEBytes;
}

int Packet::bitsToInt(unsigned char* bits, int numBits){
    int value = 0;
    for (int i = 1; i <= numBits; ++i) {
        value = value + (int)bits[i] * pow(2, numBits-1);
    }

    return value;
}

unsigned long Packet::bytesToLong(unsigned char* bytes, int numBytes){
    unsigned long totalVal = 0;

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

float Packet::bytesToFloat(unsigned char* bytes, int numBytes) {
    unsigned int* totalVal = (unsigned int*)malloc(sizeof(int));
    float *retVal = (float*)totalVal;

    if (numBytes <= 4) {
        *totalVal = (int)bytes[0];
        for (int i = 1; i < numBytes; i++) {
            //parse each byte (start MSB -> the lowest array value)
            *totalVal = (*totalVal << 8) | (unsigned int)bytes[i];
        }
    } else {
        cout << "ERROR: Too many Bytes to process; returning 0...\n";
    }
    return *retVal;
}

unsigned long long Packet::bytesToLongLong(unsigned char* bytes, int numBytes){
    unsigned long long totalVal = 0;

    if(numBytes <= 8){
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

unsigned char * Packet::bitManipulator(unsigned char* data, unsigned long numBits, long *fSize){
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

//unsigned char * Packet::bitManipulator(unsigned char* data, unsigned long numBits){
//    unsigned long numBytes = numBits / 8;
//    unsigned long bitShift = 0;
//
//    if(numBits%8 != 0){
//        //deal with truncation
//        numBytes++;
//        bitShift = 1;
//    }
//
//    unsigned char *desiredBits = (unsigned char *)malloc(numBytes * sizeof(unsigned char));
//    for(unsigned long i = byteIndex; i < (byteIndex+numBytes); i++){
//        desiredBits[i-byteIndex] = data[i];
//    }
//
//    if(bitShift){
//        //for non-byte sized needs...
//        desiredBits[numBytes-1] = desiredBits[numBytes-1] >> (8 - numBits%8);
//        data[numBytes-1] = data[numBytes-1] << (numBits%8);
//        numBytes--;
//    }
//
//    //remove used bytes
//    /*for(unsigned long i = 0; i < *fSize - numBytes; i++){
//        data[i] = data[i+numBytes];
//    }*/
//    byteIndex = byteIndex + numBytes; //this working
//    //*fSize = *fSize - numBytes;
//    return desiredBits;
//}

bool Packet::isPrintable(){
    if (this->msgCount == 0){
        return false;
    }

    int numMessages = this->messages.intraHeaders.size();
    for (int messageNumber = 0; messageNumber < numMessages; ++messageNumber){
        unsigned char *commandWord = this->messages.commWords.at(messageNumber).word1;

        if (commandWord[0] == 0x40 && commandWord[1] == 0x35){
            return true;
        }
    }
    return false;
}

void Packet::interpretData(bool print) {
    long *wordSize = (long *) malloc(sizeof(long));
    int numMessages = this->messages.intraHeaders.size();

    // iterate through all messages in the packet
    for (int messageNumber = 0; messageNumber < numMessages; ++messageNumber) {
        // Get the command word for this message
        unsigned char *commandWord = this->messages.commWords.at(messageNumber).word1;

        unsigned char *data = this->messages.data.at(messageNumber);
        // need to get fSize for data
        int msgLength = this->messages.intraHeaders.at(messageNumber).msgLen;
        long bitsleft = (long) (msgLength * 8) - 32;

        long *fSize = &bitsleft;

        if (commandWord[0] == 0x40 && commandWord[1] == 0x35) {
            // Word 1, True Airspeed 2 bytes w/ valid bit
            unsigned char *trueAirspeed = bitManipulator(data, 16, fSize);
            //trueAirspeed = swapEndian(trueAirspeed, 2);

            // Get the first bit to see if the data is valid
            bool airspeedValid = trueAirspeed[0] >> 7;

            // Turn the first bit into a 0 so it doesn't effect our value
            trueAirspeed[0] = trueAirspeed[0] & 0x7F;
            // Convert the characters to a usable long value
            unsigned long trueAirspeedValue = bytesToLong(trueAirspeed, 2);

            // Word 2, Angle of attack 2 bytes w/ valid bit
            unsigned char *angleAttack = bitManipulator(data, 16, fSize);
            //angleAttack = swapEndian(angleAttack, 2);

            bool angleAttackValid = angleAttack[0] >> 7;

            angleAttack[0] = angleAttack[0] & 0x7F;

            unsigned long angleAttackValue = bytesToLong(angleAttack, 2);

            // Word 3, Mach Number 2 bytes w/ valid bit
            unsigned char *machNum = bitManipulator(data, 16, fSize);
            //machNum = swapEndian(machNum, 2);

            bool machNumValid = machNum[0] >> 7;

            machNum[0] = machNum[0] & 0x7F;

            float machNumValue = bytesToFloat(machNum, 2);
            //unsigned long machNumValue = bytesToLong(machNum, 2);

            // Word 4, Pitch angle 2 bytes
            unsigned char *pitchAngle = bitManipulator(data, 16, fSize);
            //pitchAngle = swapEndian(pitchAngle, 2);

            unsigned long pitchAngleValue = bytesToLong(pitchAngle, 2);

            // Word 5 Roll Angle 2 bytes
            unsigned char *rollAngle = bitManipulator(data, 16, fSize);
            //rollAngle = swapEndian(rollAngle, 2);

            unsigned long rollAngleValue = bytesToLong(rollAngle, 2);

            // Word 6 Roll Rate 2 bytes
            unsigned char *rollRate = bitManipulator(data, 16, fSize);
            //rollRate = swapEndian(rollRate, 2);

            unsigned long rollRateValue = bytesToLong(rollRate, 2);

            // Word 7 Pitch Rate 2 bytes
            unsigned char *pitchRate = bitManipulator(data, 16, fSize);
            //pitchRate = swapEndian(pitchRate, 2);

            unsigned long pitchRateValue = bytesToLong(pitchRate, 2);

            // Word 8 Yaw Rate 2 bytes
            unsigned char *yawRate = bitManipulator(data, 16, fSize);
            //yawRate = swapEndian(yawRate, 2);

            unsigned long yawRateValue = bytesToLong(yawRate, 2);

            // Word 9 Roll Acceleration 2 bytes, bits 14-15 spare
            unsigned char *rollAcceleration = bitManipulator(data, 16, fSize);
            //rollAcceleration = swapEndian(rollAcceleration, 2);

            // Ensure that bits 14-15 are zeroed out
            rollAcceleration[0] = rollAcceleration[0] & 0x6F;

            unsigned long rollAccelerationValue = bytesToLong(rollAcceleration, 2);

            // Word 10 Pitch Acceleration 2 bytes, bits 12-15 spare
            unsigned char *pitchAcceleration = bitManipulator(data, 16, fSize);
            //pitchAcceleration = swapEndian(pitchAcceleration, 2);

            // Ensure that bits 12-15 are zeroed out
            pitchAcceleration[0] = pitchAcceleration[0] & 0x4F;

            unsigned long pitchAccelerationValue = bytesToLong(pitchAcceleration, 2);

            // Word 11 Yaw Acceleration 2 bytes, bits 12-15 spare
            unsigned char *yawAcceleration = bitManipulator(data, 16, fSize);
            //yawAcceleration = swapEndian(yawAcceleration, 2);

            // Ensure that bits 12-15 are zeroed out
            yawAcceleration[0] = yawAcceleration[0] & 0x4F;

            unsigned long yawAccelerationValue = bytesToLong(yawAcceleration, 2);

            // Word 12 Longitudinal Acceleration 2 bytes
            unsigned char *longitudinalAccel = bitManipulator(data, 16, fSize);
            //longitudinalAccel = swapEndian(longitudinalAccel, 2);

            unsigned long longitudinalAccelValue = bytesToLong(longitudinalAccel, 2);

            // Word 13 Lateral Acceleration 2 bytes
            unsigned char *lateralAccel = bitManipulator(data, 16, fSize);
            //lateralAccel = swapEndian(lateralAccel, 2);

            unsigned long lateralAccelValue = bytesToLong(lateralAccel, 2);

            // Word 14 Roll Rate AFCS 2 bytes
            unsigned char *rollRateAFCS = bitManipulator(data, 16, fSize);
            //rollRateAFCS = swapEndian(rollRateAFCS, 2);

            unsigned long rollRateAFCSValue = bytesToLong(rollRateAFCS, 2);

            // Word 15 Lateral Stick Force 2 bytes
            unsigned char *lateralStickForce = bitManipulator(data, 16, fSize);
            //lateralStickForce = swapEndian(lateralStickForce, 2);

            unsigned long lateralStickForceValue = bytesToLong(lateralStickForce, 2);

            // Word 16 Longitudinal Stick Force 2 bytes
            unsigned char *longitudinalStickForce = bitManipulator(data, 16, fSize);
            //longitudinalStickForce = swapEndian(longitudinalStickForce, 2);

            unsigned long longitudinalStickForceValue = bytesToLong(longitudinalStickForce, 2);

            // Word 17 Right Stabilator Deflection 2 bytes
            unsigned char *rStabilatorDef = bitManipulator(data, 16, fSize);
            //rStabilatorDef = swapEndian(rStabilatorDef, 2);

            unsigned long rStabilatorDeflectionValue = bytesToLong(rStabilatorDef, 2);

            // Word 18 Left Stabilator Deflection 2 bytes
            unsigned char *lStabilatorDef = bitManipulator(data, 16, fSize);
            //lStabilatorDef = swapEndian(lStabilatorDef, 2);

            unsigned long lStabilatorDeflectionValue = bytesToLong(lStabilatorDef, 2);

            // Word 19 Normal Acceleration 2 bytes
            unsigned char *normalAccel = bitManipulator(data, 16, fSize);
            //normalAccel = swapEndian(normalAccel, 2);

            unsigned long normalAccelValue = bytesToLong(normalAccel, 2);

            // Word 20 Angle of Sideslip 2 bytes w/ data is invalid byte
            unsigned char *sideslipAngle = bitManipulator(data, 16, fSize);
            //sideslipAngle = swapEndian(sideslipAngle, 2);

            bool sideslipAngleINVALID = sideslipAngle[0] >> 7;

            sideslipAngle[0] = sideslipAngle[0] & 0x7F;

            unsigned long sideslipAngleValue = bytesToLong(sideslipAngle, 2);

            // Word 21 Dynamic pressure 2 bytes
            unsigned char *dynamicPressure = bitManipulator(data, 16, fSize);
            //dynamicPressure = swapEndian(dynamicPressure, 2);

            unsigned long dynamicPressureValue = bytesToLong(dynamicPressure, 2);

            // display the data we've processed
            if (print) {
                cout << "Message " << messageNumber << "\n\n";

                if (airspeedValid) {
                    cout << "True Airspeed: " << trueAirspeedValue << " knots\n";

                } else {
                    cout << "True Airspeed data invalid\n";
                }

                if (angleAttackValid) {
                    cout << "Angle of Attack: " << angleAttackValue << " BAMS\n";
                } else {
                    cout << "Angle of Attack data is invalid\n";
                }

                if (machNumValid) {
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

                if (!sideslipAngleINVALID) {
                    cout << "Angle of Sideslip: " << sideslipAngleValue << " BAMS\n";
                } else {
                    cout << "Angle of Sideslip data invalid\n";
                }

                cout << "Dynamic Pressure: " << dynamicPressureValue << " lb/ft^2\n\n";

            } // end of printing for command word 0x4035


        }
        else if (commandWord[0] == 0x40 && commandWord[1] == 0x55) {
            // Word 1 Left Engine Total Fuel FLow Lb/Hr 2 bytes
            unsigned char *leftEngineFuelFlow = bitManipulator(data, 16, fSize);
            leftEngineFuelFlow = swapEndian(leftEngineFuelFlow, 2);

            unsigned long leftEngineFuelFlowValue = bytesToLong(leftEngineFuelFlow, 2);

            // Word 2 Right Engine Total Fuel Flow lb/hr 2 bytes
            unsigned char *rightEngineFuelFlow = bitManipulator(data, 16, fSize);
            rightEngineFuelFlow = swapEndian(rightEngineFuelFlow, 2);

            unsigned long rightEngineFuelFlowValue = bytesToLong(rightEngineFuelFlow, 2);

            // Word 3 Radar Altitude Ft 2 bytes
            unsigned char *radarAltitude = bitManipulator(data, 16, fSize);
            radarAltitude = swapEndian(radarAltitude, 2);

            unsigned long radarAltitudeValue = bytesToLong(radarAltitude, 2);

            // Word 4 Display TF G Command G 2 bytes
            unsigned char *displayTFGCommand = bitManipulator(data, 16, fSize);
            displayTFGCommand = swapEndian(displayTFGCommand, 2);

            unsigned long displayTFGCommandValue = bytesToLong(displayTFGCommand, 2);

            // Word 5 AFCS TF G Command G 2 bytes
            unsigned char *AFCSTFGCommand = bitManipulator(data, 16, fSize);
            AFCSTFGCommand = swapEndian(AFCSTFGCommand, 2);

            unsigned long AFCSTFGCommandValue = bytesToLong(AFCSTFGCommand, 2);

            // Word 6 Discrete word 04 2 bytes, 16 bool values
            unsigned char *discreteWord4 = bitManipulator(data, 16, fSize);
            discreteWord4 = swapEndian(discreteWord4, 2);

            *wordSize = 2;

            // Grab bit 15
            unsigned char *tmp = bitManipulator(discreteWord4, 1, wordSize);
            bool radarLowSpeedCaution = tmp[0];

            // Grab bit 14
            tmp = bitManipulator(discreteWord4, 1, wordSize);
            bool radarJamWarning = tmp[0];

            // Grab bit 13
            tmp = bitManipulator(discreteWord4, 1, wordSize);
            bool radarNoTerrainWarning = tmp[0];

            // 12
            tmp = bitManipulator(discreteWord4, 1, wordSize);
            bool radarLimitWarning = tmp[0];

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

            // After eating 8 bits, you must delete it
            unsigned char *trash = bitManipulator(discreteWord4, 8, wordSize);

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
            unsigned char *storeWord1 = bitManipulator(data, 16, fSize);
            storeWord1 = swapEndian(storeWord1, 2);

            *wordSize = 2;

            // Bits 15 - 12 spare
            unsigned char *spare = bitManipulator(storeWord1, 4, wordSize);

            // Bit 11
            tmp = bitManipulator(storeWord1, 1, wordSize);
            bool station8PylonPresent = tmp[0];

            // Bit 10
            tmp = bitManipulator(storeWord1, 1, wordSize);
            bool station5PylonPresent = tmp[0];

            // bit 9
            tmp = bitManipulator(storeWord1, 1, wordSize);
            bool station2PylonPresent = tmp[0];

            // bit 8 spare bit to destroy

            // Eat the garbage byte
            trash = bitManipulator(storeWord1, 8, wordSize);
            free(trash);

            // Bit 7-5 spare bits
            spare = bitManipulator(storeWord1, 3, wordSize);
            // bit 4
            tmp = bitManipulator(storeWord1, 1, wordSize);
            bool AGMERPresentStation8 = tmp[0];

            // bit 3
            tmp = bitManipulator(storeWord1, 1, wordSize);
            bool AGMERPresentRC = tmp[0];

            // bit 2
            tmp = bitManipulator(storeWord1, 1, wordSize);
            bool AGMERPresentStation5 = tmp[0];

            // bit 1
            tmp = bitManipulator(storeWord1, 1, wordSize);
            bool AGMERPresentLC = tmp[0];

            // bit 0
            tmp = bitManipulator(storeWord1, 1, wordSize);
            bool AGMERPresentStation2 = tmp[0];

            // Word 8
            unsigned char *storeWord2 = bitManipulator(data, 16, fSize);
            storeWord2 = swapEndian(storeWord2, 2);

            *wordSize = 2;

            // bit 15
            tmp = bitManipulator(storeWord2, 1, wordSize);
            bool station8FuelTankPresent = tmp[0];

            // bit 14
            tmp = bitManipulator(storeWord2, 1, wordSize);
            bool station5FuelTankPresent = tmp[0];

            // bit 13
            tmp = bitManipulator(storeWord2, 1, wordSize);
            bool station2FuelTankPresent = tmp[0];

            // bit 12-10 spare bits
            bitManipulator(storeWord2, 3, wordSize);

            // bit 9-8
            unsigned char *aaWeaponIDStation7 = bitManipulator(storeWord2, 2, wordSize);

            // clear
            bitManipulator(storeWord2, 8, wordSize);

            // bit 7-6
            unsigned char *aaWeaponIDStation6 = bitManipulator(storeWord2, 2, wordSize);

            // bit 5-4
            unsigned char *aaWeaponIDStation4 = bitManipulator(storeWord2, 2, wordSize);

            // bit 3-2
            unsigned char *aaWeaponIDStation3 = bitManipulator(storeWord2, 2, wordSize);

            // bits 1-0 spare bits

            // Word 9
            unsigned char *storeWord3 = bitManipulator(data, 16, fSize);
            storeWord3 = swapEndian(storeWord3, 2);

            *wordSize = 2;

            // bit 15-8 spare bits
            bitManipulator(storeWord3, 8, wordSize);

            // bit 7
            tmp = bitManipulator(storeWord3, 1, wordSize);
            bool missileStorePresent7 = tmp[0];

            // bit 6
            tmp = bitManipulator(storeWord3, 1, wordSize);
            bool missileStorePresent6 = tmp[0];

            // bit 5
            tmp = bitManipulator(storeWord3, 1, wordSize);
            bool missileStorePresent4 = tmp[0];

            // bit 4
            tmp = bitManipulator(storeWord3, 1, wordSize);
            bool missileStorePresent3 = tmp[0];

            // bit 3
            tmp = bitManipulator(storeWord3, 1, wordSize);
            bool missileStorePresen8B = tmp[0];

            // bit 2
            tmp = bitManipulator(storeWord3, 1, wordSize);
            bool missileStorePresent8A = tmp[0];

            // bit 1
            tmp = bitManipulator(storeWord3, 1, wordSize);
            bool missileStorePresent2A = tmp[0];

            // bit 0
            tmp = bitManipulator(storeWord3, 1, wordSize);
            bool missileStorePresent2B = tmp[0];

            // Word 10
            unsigned char *storeWord4 = bitManipulator(data, 16, fSize);
            storeWord4 = swapEndian(storeWord4, 2);

            *wordSize = 2;

            // bit 15 spare bit
            bitManipulator(storeWord4, 1, wordSize);

            // bit 14-12
            tmp = bitManipulator(storeWord4, 3, wordSize);
            int agWeaponCountStationRC = bitsToInt(tmp, 3);

            // bit 11-9
            tmp = bitManipulator(storeWord4, 3, wordSize);
            int agWeaponCountStationLC = bitsToInt(tmp, 3);

            // bit 6-8
            tmp = bitManipulator(storeWord4, 1, wordSize);
            bool msbOn = tmp[0];

            // clear
            bitManipulator(storeWord4, 8, wordSize);

            tmp = bitManipulator(storeWord4, 2, wordSize);
            int value = bitsToInt(tmp, 2);

            // Most significant bit was the 4s place so we add 4 to this if it was a 1
            if (msbOn) {
                value = value + 4;
            }

            int agWeaponCountStation8 = value;

            // bit 5-3
            tmp = bitManipulator(storeWord4, 3, wordSize);
            int agWeaponCountStation5 = bitsToInt(tmp, 3);

            // bit 2-0
            tmp = bitManipulator(storeWord4, 3, wordSize);
            int agWeaponCountStation2 = bitsToInt(tmp, 3);

            // Word 11
            unsigned char *storeWord5 = bitManipulator(data, 16, fSize);
            storeWord5 = swapEndian(storeWord5, 2);

            *wordSize = 2;

            // bit 15-14 spare bits

            // bit 13-7

            // bit 6-0

            // Word 12
            unsigned char *storeWord6 = bitManipulator(data, 16, fSize);
            storeWord6 = swapEndian(storeWord6, 2);

            *wordSize = 2;

            // bit 15-14

            // bit 13-7

            // bit 6-0

            // Word 13
            unsigned char *storeWord7 = bitManipulator(data, 16, fSize);
            storeWord7 = swapEndian(storeWord7, 2);

            *wordSize = 2;

            // bit 15-14 spare bits

            // bit 13

            // bit 12

            // bit 11

            // bit 10

            // bit 9-7 spare bits

            // bit 6-0

            // Word 14 Gun Rounds Remaining

            // bit 15-8 spare bits

            // bit 7-4

            // bit 3-0

            // Word 15
            unsigned char *storeWord8 = bitManipulator(data, 16, fSize);
            storeWord8 = swapEndian(storeWord8, 2);

            *wordSize = 2;

            // bit 15-12 spare bits

            // bit 11-9

            // bit 4-6

            // bit 5-3

            // bit 2-0

            // Word 16
            unsigned char *storeWord9 = bitManipulator(data, 16, fSize);
            storeWord9 = swapEndian(storeWord9, 2);

            *wordSize = 2;

            // bit 15-6 spare bits

            // bit 5

            // bit 4

            // bit 3-0 spare bits

            // Word 17 Total Fuel Weight lb 2 bytes
            unsigned char *totalFuelWeight = bitManipulator(data, 16, fSize);
            totalFuelWeight = swapEndian(totalFuelWeight, 2);

            unsigned long totalFuelWeightValue = bytesToLong(totalFuelWeight, 2);

            // Word 18

            // bit 15-12

            // bit 11-8

            // bit 7-4

            // bit 3-0

            // Word 19

            // bit 15-12

            // bit 11-8

            // bit 7-4

            // bit 3-0

            // Word 20

            // bit 15-12

            // bit 11-8

            // bit 7-4

            // bit 3-0

            // Word 21

            // bit 15

            // bit 14

            // bit 13

            // bit 12

            // bit 11-8

            // bit 7-4

            // bit 3-0


        }
        else if (commandWord[0] == 0x40 && commandWord[1] == 0x6B) {
            unsigned char *word5 = bitManipulator(data, 16, fSize);
            word5 = swapEndian(word5, 2);

            *wordSize = 2;

            bool cautionAltitude = bitManipulator(word5, 1, wordSize);
            bitManipulator(word5, 1, wordSize);
            bool cautionRightBleedAir = bitManipulator(word5, 1, fSize);
            bool cautionLeftBleedAir = bitManipulator(word5, 1, fSize);
            bitManipulator(word5, 4, wordSize);

            // clear byte 1
            bitManipulator(word5, 8, wordSize);

            // trash 4 bits
            bitManipulator(word5, 4, wordSize);

            bool cautionCASyaw = bitManipulator(word5, 1, wordSize);
            bool cautionCASroll = bitManipulator(word5, 1, wordSize);
            bool cautionCASpitch = bitManipulator(word5, 1, wordSize);


            // Word 2 2 bytes
            unsigned char *word6 = bitManipulator(data, 16, fSize);
            word6 = swapEndian(word6, 2);

            *wordSize = 2;

            bool cautionHydraulicPressure = bitManipulator(word6, 1, wordSize);
            bool cautionBoostSystemMalfunction = bitManipulator(word6, 1, wordSize);
            bool cautionRightEngineController = bitManipulator(word6, 1, wordSize);
            bool cautionLeftEngineController = bitManipulator(word6, 1, wordSize);
            bitManipulator(word6, 4, wordSize);

            // clear byte
            bitManipulator(word6, 8, wordSize);

            // spare bit
            bitManipulator(word6, 1, wordSize);

            bool cautionEmergencyBoostOn = bitManipulator(word6, 1, wordSize);
            bool cautionRollRatio = bitManipulator(word6, 1, wordSize);
            bitManipulator(word6, 4, wordSize);
            bool cautionPitchRatio = bitManipulator(word6, 1, wordSize);

            // Word 3
            unsigned char *word7 = bitManipulator(data, 16, fSize);
            word7 = swapEndian(word7, 2);

            *wordSize = 2;

            // spare 15-7
            bitManipulator(word7, 8, wordSize);
            bitManipulator(word7, 1, wordSize);

            bool cautionHydraulicPressurePC2B = bitManipulator(word7, 1, wordSize);
            bool cautionHydraulicPressurePC1B = bitManipulator(word7, 1, wordSize);
            bool cautionHydraulicPressureUTILB = bitManipulator(word7, 1, wordSize);
            bool cautionHydraulicPressurePC1A = bitManipulator(word7, 1, wordSize);

            // Word 4 TFR Set Clearance Status
            unsigned char *TFRsetClearanceStatus = bitManipulator(data, 16, fSize);
            TFRsetClearanceStatus = swapEndian(TFRsetClearanceStatus, 2);

            // Word 5
            unsigned char *pressureRatio = bitManipulator(data, 16, fSize);
            pressureRatio = swapEndian(pressureRatio, 2);

            // Get the first bit to see if the data is valid
            bool pressureRatioValid = pressureRatio[0] >> 7;

            // Turn the first bit into a 0 so it doesn't effect our value
            pressureRatio[0] = pressureRatio[0] & 0x7F;
            // Convert the characters to a usable long value
            unsigned long trueAirspeedValue = bytesToLong(pressureRatio, 2);

            // Word 6
            unsigned char *barCorrectedPressureAlt = bitManipulator(data, 16, fSize);
            barCorrectedPressureAlt = swapEndian(barCorrectedPressureAlt, 2);

            // Unit ft
            unsigned long barCorrectedPressureAltValue = bytesToLong(barCorrectedPressureAlt, 2);

            // Word 7
            unsigned char *trueHeading = bitManipulator(data, 16, fSize);
            trueHeading = swapEndian(trueHeading, 2);

            unsigned long trueHeadingValue = bytesToLong(trueHeading, 2);

            // Word 8
            unsigned char *verticalVelocity = bitManipulator(data, 16, fSize);
            verticalVelocity = swapEndian(verticalVelocity, 2);

            unsigned long verticalVelocityValue = bytesToLong(verticalVelocity, 2);

            // Word 9
            unsigned char *word1 = bitManipulator(data, 16, fSize);
            word1 = swapEndian(word1, 2);

            *wordSize = 2;

            // 15-6 spare
            bitManipulator(word1, 8, wordSize);
            bitManipulator(word1, 2, wordSize);

            bool landingGearHandleUp = bitManipulator(word1, 1, wordSize);

            // Word 10
            unsigned char *word2 = bitManipulator(data, 16, fSize);
            word2 = swapEndian(word2, 2);

            *wordSize = 2;

            // 15-10 spare
            bitManipulator(word2, 6, wordSize);
            bool autoTFconnect = bitManipulator(word2, 1, wordSize);
            bool manualTFconnect = bitManipulator(word2, 1, wordSize);

            // 7-0 are spare nothing else needed

            // word 11
            unsigned char *word3 = bitManipulator(data, 16, fSize);
            word3 = swapEndian(word3, 2);

            *wordSize = 2;

            // 15-14 spare
            bitManipulator(word3, 2, wordSize);

            bool highLowAirspeedIndicatorFlashed = bitManipulator(word3, 1, wordSize);

            // 12-11 spare
            bitManipulator(word3, 2, wordSize);

            bool aftControlStickOverrideIndicated = bitManipulator(word3, 1, wordSize);

            bool AFCSDetectedTFfailed = bitManipulator(word3, 1, wordSize);

            // 8-6 spare
            bitManipulator(word3, 1, wordSize);

            bitManipulator(word3, 8, wordSize);

            bitManipulator(word3, 2, wordSize);

            bool lowAltitudeMonitorFailed = bitManipulator(word3, 1, wordSize);

            bool modeBFlyUpIndicated = bitManipulator(word3, 1, wordSize);

            bool modeAFlyUpIndicated = bitManipulator(word3, 1, wordSize);

            // 2-1 spare
            bitManipulator(word3, 2, wordSize);

            bool TFBitInhibited = bitManipulator(word3, 1, wordSize);

        }
        else if (commandWord[0] == 0x40 && commandWord[1] == 0x90) {

            // Word 1 Left Engine Fan Turbing Inlet Temp Deg C* 2 bytes
            unsigned char *leftEngineFanTemp = bitManipulator(data, 16, fSize);
            leftEngineFanTemp = swapEndian(leftEngineFanTemp, 2);

            unsigned long leftEngineFanTempValue = bytesToLong(leftEngineFanTemp, 2);

            // Word 2 Left Engine Main Oil Pressure Deg PSI 2 bytes
            unsigned char *leftEngineOilPressure = bitManipulator(data, 16, fSize);
            leftEngineOilPressure = swapEndian(leftEngineOilPressure, 2);

            unsigned long leftEngineOilPressureValue = bytesToLong(leftEngineOilPressure, 2);

            // Word 3 Left Engine Nozzle Position Percent 2 bytes
            unsigned char *leftEngineNozzlePosition = bitManipulator(data, 16, fSize);
            leftEngineNozzlePosition = swapEndian(leftEngineNozzlePosition, 2);

            unsigned long leftEngineNozzlePositionValue = bytesToLong(leftEngineNozzlePosition, 2);

            // Word 4 Left Engine Core Speed RPM 2 bytes
            unsigned char *leftEngineCoreSpeed = bitManipulator(data, 16, fSize);
            leftEngineCoreSpeed = swapEndian(leftEngineCoreSpeed, 2);

            unsigned long leftEngineCoreSpeedValue = bytesToLong(leftEngineCoreSpeed, 2);

            // Word 5 Left Engine Fan Speed RPM 2 bytes
            unsigned char *leftEngineFanSpeed = bitManipulator(data, 16, fSize);
            leftEngineFanSpeed = swapEndian(leftEngineFanSpeed, 2);

            unsigned long leftEngineFanSpeedValue = bytesToLong(leftEngineFanSpeed, 2);

            // Word 6 Left Engine Total Temp Deg C* 2 bytes
            unsigned char *leftEngineTotalTemp = bitManipulator(data, 16, fSize);
            leftEngineTotalTemp = swapEndian(leftEngineTotalTemp, 2);

            unsigned long leftEngineTotalTempValue = bytesToLong(leftEngineTotalTemp, 2);

            // Word 7 Left Engine Power Level Angle Deg 2 bytes
            unsigned char *leftEnginePowerLevelAngle = bitManipulator(data, 16, fSize);
            leftEnginePowerLevelAngle = swapEndian(leftEnginePowerLevelAngle, 2);

            unsigned long leftEnginePowerLevelAngleValue = bytesToLong(leftEnginePowerLevelAngle, 2);

            // Word 8 Right Engine Fan Turbing Inlet Temp Deg C* 2 bytes
            unsigned char *rightEngineFanTemp = bitManipulator(data, 16, fSize);
            rightEngineFanTemp = swapEndian(rightEngineFanTemp, 2);

            unsigned long rightEngineFanTempValue = bytesToLong(rightEngineFanTemp, 2);

            // Word 9 Right Engine Main Oil Pressure PSI 2 bytes
            unsigned char *rightEngineOilPressure = bitManipulator(data, 16, fSize);
            rightEngineOilPressure = swapEndian(rightEngineOilPressure, 2);

            unsigned long rightEngineOilPressureValue = bytesToLong(rightEngineOilPressure, 2);

            // Word 10 Right Engine Nozzle Position Percent 2 bytes
            unsigned char *rightEngineNozzlePosition = bitManipulator(data, 16, fSize);
            rightEngineNozzlePosition = swapEndian(rightEngineNozzlePosition, 2);

            unsigned long rightEngineNozzlePositionValue = bytesToLong(rightEngineNozzlePosition, 2);

            // Word 11 Right Engine Core Speed RPM 2 bytes
            unsigned char *rightEngineCoreSpeed = bitManipulator(data, 16, fSize);
            rightEngineCoreSpeed = swapEndian(rightEngineCoreSpeed, 2);

            unsigned long rightEngineCoreSpeedValue = bytesToLong(rightEngineCoreSpeed, 2);

            // Word 12 Right Engine Fan Spead RPM 2 bytes
            unsigned char *rightEngineFanSpeed = bitManipulator(data, 16, fSize);
            rightEngineFanSpeed = swapEndian(rightEngineFanSpeed, 2);

            unsigned long rightEngineFanSpeedValue = bytesToLong(rightEngineFanSpeed, 2);

            // Word 13 Right Engine Total Temp Deg C* 2 bytes
            unsigned char *rightEngineTotalTemp = bitManipulator(data, 16, fSize);
            rightEngineTotalTemp = swapEndian(rightEngineTotalTemp, 2);

            unsigned long rightEngineTotalTempValue = bytesToLong(rightEngineTotalTemp, 2);

            // Word 14 Right Engine Power Lever Angle Deg 2 bytes
            unsigned char *rightEnginePowerLevelAngle = bitManipulator(data, 16, fSize);
            rightEnginePowerLevelAngle = swapEndian(rightEnginePowerLevelAngle, 2);

            unsigned long rightEnginePowerLevelAngleValue = bytesToLong(rightEnginePowerLevelAngle, 2);

            // Word 15 Left Engine Validity Word 1 2 bytes
            unsigned char *leftEngineValidityWord1 = bitManipulator(data, 16, fSize);
            leftEngineValidityWord1 = swapEndian(leftEngineValidityWord1, 2);

            unsigned long leftEngineValidityWord1Value = bytesToLong(leftEngineValidityWord1, 2);

            *wordSize = 2;

            // bit 15
            unsigned char *tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftMachNum = tmp[0];

            // bit 14
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftDuctFuelFlow = tmp[0];

            // bit 13
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftCoreFuelFlow = tmp[0];

            // bit 12
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftGasFuelFlow = tmp[0];

            // bit 11
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftPowerLeverAngle = tmp[0];

            // bit 10
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftRCVVPosition = tmp[0];

            // bit 9
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftCIVVPosition = tmp[0];

            // bit 8
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftBurnerPressure = tmp[0];

            // Clear byte 1
            bitManipulator(leftEngineValidityWord1, 8, wordSize);

            // bit 7
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftDischargePressure = tmp[0];

            // bit 6
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftTotalPressure = tmp[0];

            // bit 5
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftTotalTemp = tmp[0];

            // bit 4
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftFanSpeed = tmp[0];

            // bit 3
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftCoreSpeed = tmp[0];

            // bit 2
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftNozzlePosition = tmp[0];

            // bit 1
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftMainOilPressure = tmp[0];

            // bit 0
            tmp = bitManipulator(leftEngineValidityWord1, 1, wordSize);
            bool validLeftFanTurbineInletTemp = tmp[0];

            // Word 16 Right Engine Validity Word 1 2 bytes
            unsigned char *rightEngineValidityWord1 = bitManipulator(data, 16, fSize);
            rightEngineValidityWord1 = swapEndian(rightEngineValidityWord1, 2);

            unsigned long rightEngineValidityWord1Value = bytesToLong(rightEngineValidityWord1, 2);

            *wordSize = 2;

            // bit 15
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightMachNum = tmp[0];

            // bit 14
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightDuctFuelFlow = tmp[0];

            // bit 13
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightCoreFuelFlow = tmp[0];

            // bit 12
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightGasFuelFlow = tmp[0];

            // bit 11
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightPowerLeverAngle = tmp[0];

            // bit 10
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightRCVVPosition = tmp[0];

            // bit 9
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightCIVVPosition = tmp[0];

            // bit 8
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightBurnerPressure = tmp[0];

            // Clear byte
            bitManipulator(rightEngineValidityWord1, 8, wordSize);

            // bit 7
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightDischargePressure = tmp[0];

            // bit 6
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightTotalPressure = tmp[0];

            // bit 5
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightTotalTemp = tmp[0];

            // bit 4
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightFanSpeed = tmp[0];

            // bit 3
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightCoreSpeed = tmp[0];

            // bit 2
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightNozzlePosition = tmp[0];

            // bit 1
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightMainOilPressure = tmp[0];

            // bit 0
            tmp = bitManipulator(rightEngineValidityWord1, 1, wordSize);
            bool validRightFanTurbineInletTemp = tmp[0];

        } else if (commandWord[0] == 0x40 && commandWord[1] == 0xB0) {
            // Word 1 Discrete Word 8 2 bytes
            unsigned char *discreteWord8 = bitManipulator(data, 16, fSize);
            discreteWord8 = swapEndian(discreteWord8, 2);

            *wordSize = 2;

            // bits 15-1 spare
            bitManipulator(discreteWord8, 8, wordSize);
            bitManipulator(discreteWord8, 7, wordSize);

            // bit 0 Weight Off Wheels
            unsigned char *tmp = bitManipulator(discreteWord8, 1, wordSize);
            bool weightOffWheels = tmp[0];

            // Word 2 Overload Warning System Status 2 bytes
            unsigned char *overloadWarningSystemStatus = bitManipulator(data, 16, fSize);
            overloadWarningSystemStatus = swapEndian(overloadWarningSystemStatus, 2);

            *wordSize = 2;

            // bit 15
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool ADCSpikeTrueAOA = tmp[0];
            // bit 14
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool ADCSpikePressureRatio = tmp[0];
            // bit 13
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool ADCSpikeMachNum = tmp[0];
            // bit 12
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool ADCSpikeBaroCorrectedPressAlt = tmp[0];
            // bit 11
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool ADCSpikeNormAcceleration = tmp[0];
            // bit 10
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool ADCDataInvalid = tmp[0];
            // bit 9
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool cumVoiceTimerTimedOut = tmp[0];
            // bit 8
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool MPDPCommInvalid = tmp[0];

            // clear byte 1
            bitManipulator(overloadWarningSystemStatus, 8, wordSize);

            // bit 7
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool SGPNotOperative = tmp[0];
            // bit 6
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool GPNotOperative = tmp[0];
            // bit 5
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool AFCSCommORDataInvalid = tmp[0];
            // bit 4
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool SGPBCommInvalid = tmp[0];
            // bit 3
            tmp = bitManipulator(overloadWarningSystemStatus, 1, wordSize);
            bool CAUFailure = tmp[0];
            // bits 2-0 spare
            bitManipulator(overloadWarningSystemStatus, 3, wordSize);

            // Word 3 Discrete Word 9 2 bytes
            unsigned char *discreteWord9 = bitManipulator(data, 16, fSize);
            discreteWord9 = swapEndian(discreteWord9, 2);

            *wordSize = 2;

            // bits 15-6 spare
            bitManipulator(discreteWord9, 8, wordSize);
            bitManipulator(discreteWord9, 2, wordSize);

            // bit 5 Solid 900Hz Tone
            tmp = bitManipulator(discreteWord9, 1, wordSize);
            bool solid900Tone = tmp[0];

            // bit 4 AOA Doublet Tone
            tmp = bitManipulator(discreteWord9, 1, wordSize);
            bool AOADoubletTone = tmp[0];

            // bit 3 Intermittent 4Hz Tone
            tmp = bitManipulator(discreteWord9, 1, wordSize);
            bool intermittent4Tone = tmp[0];

            // bit 2 Intermittent 10Hz Tone
            tmp = bitManipulator(discreteWord9, 1, wordSize);
            bool intermittent10Tone = tmp[0];

            // bit 1 Overload Warning System Voice Msg
            tmp = bitManipulator(discreteWord9, 1, wordSize);
            bool overloadWarningSystemVMsg = tmp[0];

            // bit 0 ASP Latch 72 Set
            tmp = bitManipulator(discreteWord9, 1, wordSize);
            bool ASPLatch72 = tmp[0];

            // Word 4 Forward Fuselage Warning Ratio 2 bytes
            unsigned char *fwdFuselageWarningRatio = bitManipulator(data, 16, fSize);
            fwdFuselageWarningRatio = swapEndian(fwdFuselageWarningRatio, 2);

            unsigned long fwdFuselageWarningRatioValue = bytesToLong(fwdFuselageWarningRatio, 2);

            // Word 5 Wing Warning Ratio 2 bytes
            unsigned char *wingWarningRatio = bitManipulator(data, 16, fSize);
            wingWarningRatio = swapEndian(wingWarningRatio, 2);

            unsigned long wingWarningRatioValue = bytesToLong(wingWarningRatio, 2);

            // Word 6 Left Horizontal Tail Warning Ratio 2 bytes
            unsigned char *lHorizontalTailWarningRatio = bitManipulator(data, 16, fSize);
            lHorizontalTailWarningRatio = swapEndian(lHorizontalTailWarningRatio, 2);

            unsigned long lHorizontalTailWarningRatioValue = bytesToLong(lHorizontalTailWarningRatio, 2);

            // Word 7 Right Horizontal Tail Warning Ratio 2 bytes
            unsigned char *rHorizontalTailWarningRatio = bitManipulator(data, 16, fSize);
            rHorizontalTailWarningRatio = swapEndian(rHorizontalTailWarningRatio, 2);

            unsigned long rHorizontalTailWarningRatioValue = bytesToLong(rHorizontalTailWarningRatio, 2);

            // Word 8 Pylon Warning Ratio 2 bytes
            unsigned char *pylonWarningRatio = bitManipulator(data, 16, fSize);
            pylonWarningRatio = swapEndian(pylonWarningRatio, 2);

            unsigned long pylonWarningRatioValue = bytesToLong(pylonWarningRatio, 2);

            // Word 9 Mass Items Warning Ratio 2 bytes
            unsigned char *massItemsWarningRatio = bitManipulator(data, 16, fSize);
            massItemsWarningRatio = swapEndian(massItemsWarningRatio, 2);

            unsigned long massItemsWarningRatioValue = bytesToLong(massItemsWarningRatio, 2);

            // Word 10 CFT Warning Ratio 2 bytes
            unsigned char *CFTWarningRatio = bitManipulator(data, 16, fSize);
            CFTWarningRatio = swapEndian(CFTWarningRatio, 2);

            unsigned long CFTWarningRatioValue = bytesToLong(CFTWarningRatio, 2);

            // Word 11 Forward Fuselage NZ Allowable 2 bytes
            unsigned char *fwdFuselageNZAllowable = bitManipulator(data, 16, fSize);
            fwdFuselageNZAllowable = swapEndian(fwdFuselageNZAllowable, 2);

            unsigned long fwdFuselageNZAllowableValue = bytesToLong(fwdFuselageNZAllowable, 2);

            // Word 12 Wing NZ Allowable 2 bytes
            unsigned char *wingNZAllowable = bitManipulator(data, 16, fSize);
            wingNZAllowable = swapEndian(wingNZAllowable, 2);

            unsigned long wingNZAllowableValue = bytesToLong(wingNZAllowable, 2);

            // Word 13 Pylon NZ Allowable 2 bytes
            unsigned char *pylonNZAllowable = bitManipulator(data, 16, fSize);
            pylonNZAllowable = swapEndian(pylonNZAllowable, 2);

            unsigned long pylonNZAllowableValue = bytesToLong(pylonNZAllowable, 2);

            // Word 14 CFT NZ Allowable 2 bytes
            unsigned char *CFTNZAllowable = bitManipulator(data, 16, fSize);
            CFTNZAllowable = swapEndian(CFTNZAllowable, 2);

            unsigned long CFTNZAllowableValue = bytesToLong(CFTNZAllowable, 2);

            // Word 15 Time 2 bytes
            unsigned char *time = bitManipulator(data, 16, fSize);
            time = swapEndian(time, 2);

            *wordSize = 2;

            // bits spare 15-12
            bitManipulator(time, 4, wordSize);

            // bits 11-6 Minutes
            tmp = bitManipulator(time, 4, wordSize);

            // conduct a modified bits to int because of the bits being split across the bytes
            int value = 0;
            for (int i = 1; i <= 4; ++i) {
                value = value + (int) tmp[i] * pow(2, 6 - i);
            }

            // Clear byte
            bitManipulator(time, 8, wordSize);

            // grab bits 7 and 6
            tmp = bitManipulator(time, 2, wordSize);

            for (int i = 0; i < 2; ++i) {
                value = value + (int) tmp[i] * pow(2, 1 - i);
            }

            int minutes = value;


            // bits 5-0 Hours
            tmp = bitManipulator(time, 6, wordSize);
            int hours = bitsToInt(tmp, 6);

            // Word 16 Seconds 2 bytes with Valid Bit
            unsigned char *secondsChar = bitManipulator(data, 16, fSize);
            secondsChar = swapEndian(secondsChar, 2);

            *wordSize = 2;

            // bits 31 time is valid
            tmp = bitManipulator(secondsChar, 1, wordSize);
            bool timeValid = tmp[0];

            // bits 30-22 spare
            bitManipulator(secondsChar, 7, wordSize);
            // clear byte
            bitManipulator(secondsChar, 8, wordSize);
            bitManipulator(secondsChar, 2, wordSize);

            // bits 21-16 secondsChar
            tmp = bitManipulator(secondsChar, 6, wordSize);
            int seconds = bitsToInt(tmp, 6);

        } else if (commandWord[0] == 0x40 && commandWord[1] == 0xD3) {
            // Word 1 discreteWord10  2 bytes
            unsigned char *discreteWord10 = bitManipulator(data, 16, fSize);
            discreteWord10 = swapEndian(discreteWord10, 2);

            *wordSize = 2;

            // Bits 15-1 spare
            bitManipulator(discreteWord10, 8, wordSize);
            bitManipulator(discreteWord10, 7, wordSize);

            // Bit 0 is value
            bool IPEEngineInstalled = bitManipulator(discreteWord10, 1, wordSize);

            // Word 2 discreteWord11  2 bytes
            unsigned char *discreteWord11 = bitManipulator(data, 16, fSize);
            discreteWord11 = swapEndian(discreteWord11, 2);

            *wordSize = 2;

            // Bits 15-1 spare
            bitManipulator(discreteWord11, 8, wordSize);
            bitManipulator(discreteWord11, 7, wordSize);

            // Bit 0
            bool CFTSInstalled = bitManipulator(discreteWord11, 1, wordSize);

            // Word 3 Overload Warning System Status 2  2 bytes
            unsigned char *overloadWarning = bitManipulator(data, 16, fSize);
            overloadWarning = swapEndian(overloadWarning, 2);

            *wordSize = 2;
            // bit 15
            unsigned char *tmp = bitManipulator(overloadWarning, 1, wordSize);
            bool excessFuel = tmp[0];
            // bit 14
            tmp = bitManipulator(overloadWarning, 1, wordSize);
            bool PACSCommInvalid = tmp[0];
            // bit 13
            tmp = bitManipulator(overloadWarning, 1, wordSize);
            bool invalidCFTSAboard = tmp[0];
            // bit 12
            tmp = bitManipulator(overloadWarning, 1, wordSize);
            bool AGCountGT1 = tmp[0];
            // bit 11
            tmp = bitManipulator(overloadWarning, 1, wordSize);
            bool invalidArmamentAGCountNoID = tmp[0];
            // bit 10
            tmp = bitManipulator(overloadWarning, 1, wordSize);
            bool TEWSIDOnAnyStationBut5 = tmp[0];

            // bit 9-0 Reserved
            // So we don't have to do anything

            // Word 4 Aircraft Gross Weight Lb  2 bytes
            unsigned char *ACWeight = bitManipulator(data, 16, fSize);
            ACWeight = swapEndian(ACWeight, 2);

            unsigned long ACWeightValue = bytesToLong(ACWeight, 2);

            // Word 5 Left CFT Filtered Fuel Weight Lb  2 bytes
            unsigned char *leftFuelWeight = bitManipulator(data, 16, fSize);
            leftFuelWeight = swapEndian(leftFuelWeight, 2);

            unsigned long leftFuelWeightValue = bytesToLong(leftFuelWeight, 2);

            // Word 6 Right CFT Filtered Fuel Weight Lb  2 bytes
            unsigned char *rightFuelWeight = bitManipulator(data, 16, fSize);
            rightFuelWeight = swapEndian(rightFuelWeight, 2);

            unsigned long rightFuelWeightValue = bytesToLong(rightFuelWeight, 2);

            // Word 7 Total Filtered Fuel Weight Lb  2 bytes
            unsigned char *totalFuelWeight = bitManipulator(data, 16, fSize);
            totalFuelWeight = swapEndian(totalFuelWeight, 2);

            unsigned long totalFuelWeightValue = bytesToLong(totalFuelWeight, 2);

            // Word 8 Date 6 bytes
            unsigned char *date = bitManipulator(data, 48, fSize);
            date = swapEndian(date, 6);

            *wordSize = 6;

            // bits 47 - 40 are spare
            bitManipulator(date, 8, wordSize);

            // 39-36
            tmp = bitManipulator(date, 4, wordSize);
            int dayOnesDigit = bitsToInt(tmp, 4);

            // 35-32
            tmp = bitManipulator(date, 4, wordSize);
            int dayTensDigit = bitsToInt(tmp, 4);

            // clear byte 5 from bitmanipulator
            bitManipulator(date, 8, wordSize);

            // Byte 4 is spare
            bitManipulator(data, 8, wordSize);

            // 23-20
            tmp = bitManipulator(date, 4, wordSize);
            int monthOnesDigit = bitsToInt(tmp, 4);

            // 19-16
            tmp = bitManipulator(date, 4, wordSize);
            int monthTensDigit = bitsToInt(tmp, 4);

            // clear byte 3 from bitmanipulator
            bitManipulator(date, 8, wordSize);

            // Byte 2 spare
            bitManipulator(date, 8, wordSize);

            // 7-4
            tmp = bitManipulator(date, 4, wordSize);
            int yearOnesDigit = bitsToInt(tmp, 4);

            // 3-0
            tmp = bitManipulator(date, 4, wordSize);
            int yearTensDigit = bitsToInt(tmp, 4);

            // Word 11 Mission Type Code 2 bytes
            unsigned char *missionTypeCode = bitManipulator(data, 16, fSize);
            missionTypeCode = swapEndian(missionTypeCode, 2);

            *wordSize = 2;
            // First byte is spare
            bitManipulator(missionTypeCode, 8, wordSize);

            //bit 7-4
            tmp = bitManipulator(missionTypeCode, 4, wordSize);
            int missionCodeDigit7 = bitsToInt(tmp, 4);

            //bit 3-0
            tmp = bitManipulator(missionTypeCode, 4, wordSize);
            int missionCodeDigit1 = bitsToInt(tmp, 4);

            // Word 12 Aircraft Serial Number 6 bytes
            unsigned char *ACSerialNum = bitManipulator(data, 48, fSize);
            ACSerialNum = swapEndian(ACSerialNum, 2);

            *wordSize = 6;

            // 47 -40 spare
            bitManipulator(ACSerialNum, 8, wordSize);

            // 39-36
            tmp = bitManipulator(ACSerialNum, 4, wordSize);
            int acNumDigit6 = bitsToInt(tmp, 4);

            // 35-32
            tmp = bitManipulator(ACSerialNum, 4, wordSize);
            int acNumDigit5 = bitsToInt(tmp, 4);

            // Clear byte 5
            bitManipulator(ACSerialNum, 8, wordSize);

            // Byte 4 spare
            bitManipulator(ACSerialNum, 8, wordSize);

            // 23-20
            tmp = bitManipulator(ACSerialNum, 4, wordSize);
            int acNumDigit4 = bitsToInt(tmp, 4);

            // 19-16
            tmp = bitManipulator(ACSerialNum, 4, wordSize);
            int acNumDigit2 = bitsToInt(tmp, 4);

            // Clear byte 3
            bitManipulator(ACSerialNum, 8, wordSize);

            // Byte 2 spare
            bitManipulator(ACSerialNum, 8, wordSize);

            // 7-4
            tmp = bitManipulator(ACSerialNum, 4, wordSize);
            int acNumDigit3 = bitsToInt(tmp, 4);

            // 3 - 0
            tmp = bitManipulator(ACSerialNum, 4, wordSize);
            int acNumDigit1 = bitsToInt(tmp, 4);

            // Word 15 Wing ID 4 bytes
            unsigned char *wingID = bitManipulator(data, 32, fSize);
            wingID = swapEndian(wingID, 2);

            *wordSize = 4;

            // Byte 4 spare
            bitManipulator(wingID, 8, wordSize);

            // bit 23-20
            tmp = bitManipulator(wingID, 4, wordSize);
            int wingNumDigit4 = bitsToInt(tmp, 4);
            // bit 19-16
            tmp = bitManipulator(wingID, 4, wordSize);
            int wingNumDigit2 = bitsToInt(tmp, 4);

            // Clear byte 3
            bitManipulator(wingID, 8, wordSize);

            // Byte 2 spare
            bitManipulator(wingID, 8, wordSize);

            // bit 7-4
            tmp = bitManipulator(wingID, 4, wordSize);
            int wingNumDigit3 = bitsToInt(tmp, 4);

            // bit 3-0
            tmp = bitManipulator(wingID, 4, wordSize);
            int wingNumDigit1 = bitsToInt(tmp, 4);

            // Word 17 SFDR IBIT Control
            unsigned char *IBITControl = bitManipulator(data, 16, fSize);
            IBITControl = swapEndian(IBITControl, 2);

            *wordSize = 2;

            // bit 15-1 spare
            bitManipulator(IBITControl, 8, wordSize);
            bitManipulator(IBITControl, 7, wordSize);

            // bit 0
            tmp = bitManipulator(IBITControl, 1, wordSize);
            bool SFDRIBITControl = tmp[0];

            // Word 18 Station 2 Total Weight Lb 2 bytes
            unsigned char *station2Weight = bitManipulator(data, 16, fSize);
            station2Weight = swapEndian(station2Weight, 2);

            unsigned long station2WeightValue = bytesToLong(station2Weight, 2);

            // Word 19 Station 8 Total Weight Lb 2 bytes
            unsigned char *station8Weight = bitManipulator(data, 16, fSize);
            station8Weight = swapEndian(station8Weight, 2);

            unsigned long station8WeightValue = bytesToLong(station8Weight, 2);

        } else if (commandWord[0] == 0x40 && commandWord[1] == 0xE8) {
            // Word 1 GCWS Status Discretes  2 bytes
            unsigned char *GCWSStatusDiscretes = bitManipulator(data, 16, fSize);
            GCWSStatusDiscretes = swapEndian(GCWSStatusDiscretes, 2);

            *wordSize = 2;

            //bit 15
            unsigned char *tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete16 = tmp[0];

            //bit 14
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete15 = tmp[0];

            //bit 13
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete14 = tmp[0];

            //bit 12
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete13 = tmp[0];

            //bit 11
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete12 = tmp[0];

            //bit 10
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete11 = tmp[0];

            //bit 9
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete10 = tmp[0];

            //bit 8
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete9 = tmp[0];

            // Clear the first byte
            bitManipulator(GCWSStatusDiscretes, 8, wordSize);

            //bit 7
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete8 = tmp[0];

            //bit 6
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete7 = tmp[0];

            //bit 5
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete6 = tmp[0];

            //bit 4
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete5 = tmp[0];

            //bit 3
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete4 = tmp[0];

            //bit 2
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete3 = tmp[0];

            //bit 1
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete2 = tmp[0];

            //bit 0
            tmp = bitManipulator(GCWSStatusDiscretes, 1, wordSize);
            bool GCWSStatusDiscrete1 = tmp[0];

            // Word 2 GCWS Validity Discretes  2 bytes
            unsigned char *GCWSValidityDiscretes = bitManipulator(data, 16, fSize);
            GCWSValidityDiscretes = swapEndian(GCWSValidityDiscretes, 2);

            *wordSize = 2;

            //bit 15
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete16 = tmp[0];

            //bit 14
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete15 = tmp[0];

            //bit 13
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete14 = tmp[0];

            //bit 12
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete13 = tmp[0];

            //bit 11
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete12 = tmp[0];

            //bit 10
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete11 = tmp[0];

            //bit 9
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete10 = tmp[0];

            //bit 8
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete9 = tmp[0];

            // Clear first byte
            bitManipulator(GCWSValidityDiscretes, 8, wordSize);

            //bit 7
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete8 = tmp[0];

            //bit 6
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete7 = tmp[0];

            //bit 5
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete6 = tmp[0];

            //bit 4
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete5 = tmp[0];

            //bit 3
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete4 = tmp[0];

            //bit 2
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete3 = tmp[0];

            //bit 1
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete2 = tmp[0];

            //bit 0
            tmp = bitManipulator(GCWSValidityDiscretes, 1, wordSize);
            bool GCWSValidityDiscrete1 = tmp[0];

            // Word 3 GCWS Data Reasonable Discretes  2 bytes
            unsigned char *GCWSReasonableDiscretes = bitManipulator(data, 16, fSize);
            GCWSReasonableDiscretes = swapEndian(GCWSReasonableDiscretes, 2);

            *wordSize = 2;

            //bit 15
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete16 = tmp[0];

            //bit 14
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete15 = tmp[0];

            //bit 13
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete14 = tmp[0];

            //bit 12
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete13 = tmp[0];

            //bit 11
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete12 = tmp[0];

            //bit 10
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete11 = tmp[0];

            //bit 9
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete10 = tmp[0];

            //bit 8
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete9 = tmp[0];

            // Trash the first byte
            bitManipulator(GCWSReasonableDiscretes, 8, wordSize);

            //bit 7
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete8 = tmp[0];

            //bit 6
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete7 = tmp[0];

            //bit 5
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete6 = tmp[0];

            //bit 4
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete5 = tmp[0];

            //bit 3
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete4 = tmp[0];

            //bit 2
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete3 = tmp[0];

            //bit 1
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete2 = tmp[0];

            //bit 0
            tmp = bitManipulator(GCWSReasonableDiscretes, 1, wordSize);
            bool GCWSReasonableDiscrete1 = tmp[0];

            // Word 4 GCWS HD feet 2 bytes
            unsigned char *GCWSHD = bitManipulator(data, 16, fSize);
            GCWSHD = swapEndian(GCWSHD, 2);

            unsigned long GCWSHDValue = bytesToLong(GCWSHD, 2);

            // Word 5 GCWS HM feet 2 bytes
            unsigned char *GCWSHM = bitManipulator(data, 16, fSize);
            GCWSHM = swapEndian(GCWSHM, 2);

            unsigned long GCWSHMValue = bytesToLong(GCWSHM, 2);

            // Word 6 GCWS HT feet 2 bytes
            unsigned char *GCWSHT = bitManipulator(data, 16, fSize);
            GCWSHT = swapEndian(GCWSHT, 2);

            unsigned long GCWSHTValue = bytesToLong(GCWSHT, 2);

            // Word 7 GCWS HDB feet 2 bytes
            unsigned char *GCWSHDB = bitManipulator(data, 16, fSize);
            GCWSHDB = swapEndian(GCWSHDB, 2);

            unsigned long GCWSHDBValue = bytesToLong(GCWSHDB, 2);

            // Word 8 GCWS NZ Maximum 2 bytes
            unsigned char *GCWSNZ = bitManipulator(data, 16, fSize);
            GCWSNZ = swapEndian(GCWSNZ, 2);

            unsigned long GCWSNZValue = bytesToLong(GCWSNZ, 2);

        } else {
            // We cannot interpret any other command words
        }

    }

}
