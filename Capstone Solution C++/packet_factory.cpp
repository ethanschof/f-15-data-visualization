//
// Created by C23Ethan.Schofield on 10/9/2022.
//
#include "packet_factory.hpp"
#include "packet.hpp"
#include <vector>
#include <memory>
using namespace std;

long byteIndex = 0;

unsigned char* swapEndian(unsigned char* bytes, int numBytes){
    unsigned char* BEBytes = (unsigned char*)malloc(numBytes * sizeof(unsigned char));
    for(int i = 0; i < numBytes; i++){
        BEBytes[(numBytes-1)-i] = bytes[i];
        //bytes[i] = BEBytes[(numBytes-1)-i];
    }
    free(bytes);
    return BEBytes;
}

int bitsToInt(unsigned char* bits, int numBits){
    int value = 0;
    for (int i = 1; i <= numBits; ++i) {
        value = value + (int)bits[i] * pow(2, numBits-1);
    }

    return value;
}

unsigned long bytesToLong(unsigned char* bytes, int numBytes){
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

unsigned long long bytesToLongLong(unsigned char* bytes, int numBytes){
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

unsigned char * bitManipulator(unsigned char* data, unsigned long numBits, long *fSize){
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

unsigned char * bitManipulator(unsigned char* data, unsigned long numBits){
    unsigned long numBytes = numBits / 8;
    unsigned long bitShift = 0;

    if(numBits%8 != 0){
        //deal with truncation
        numBytes++;
        bitShift = 1;
    }

    unsigned char *desiredBits = (unsigned char *)malloc(numBytes * sizeof(unsigned char));
    for(unsigned long i = byteIndex; i < (byteIndex+numBytes); i++){
        desiredBits[i-byteIndex] = data[i];
    }

    if(bitShift){
        //for non-byte sized needs...
        desiredBits[numBytes-1] = desiredBits[numBytes-1] >> (8 - numBits%8);
        data[numBytes-1] = data[numBytes-1] << (numBits%8);
        numBytes--;
    }

    //remove used bytes
    /*for(unsigned long i = 0; i < *fSize - numBytes; i++){
        data[i] = data[i+numBytes];
    }*/
    byteIndex = byteIndex + numBytes; //this working
    //*fSize = *fSize - numBytes;
    return desiredBits;
}

vector<unique_ptr<Packet>> createPackets(unsigned char* data, long* fSize){
    vector<unique_ptr < Packet>> myPackets;
    int done = 0;
    int packetsCreated = 0;
    int num1553 = 0;

    while (packetsCreated < 10000){

        unsigned char *packetSync = bitManipulator(data, (long)PACKET_SYNC_LENGTH);

        // checks for packet sync
        if (packetSync[0] == 0x25 && packetSync[1] == 0xEB){

            // Get data from the packet header MUST STAY IN THIS ORDER
            unsigned char *channelID = bitManipulator(data, (long)CHAN_ID_LENGTH);
            channelID = swapEndian(channelID, 2);
            unsigned char *packetLength = bitManipulator(data, (long)PACKET_LENGTH_LENGTH);
            packetLength = swapEndian(packetLength, 4);
            unsigned char *dataLength = bitManipulator(data, (long)DATA_LENGTH);
            dataLength = swapEndian(dataLength, 4);
            unsigned char *dataTypeVer = bitManipulator(data, (long)DATA_TYPE_VERSION_LENGTH);
            unsigned char *seqNum = bitManipulator(data, (long)SEQ_NUM_LENGTH);
            unsigned char *packetFlags = bitManipulator(data, (long)PACKET_FLAGS_LENGTH);
            unsigned char *dataType = bitManipulator(data, (long)DATA_TYPE_BIT_LENGTH);
            unsigned char *relativeTimeCounter = bitManipulator(data, (long)RELATIVE_TIME_COUNTER_LENGTH);
            relativeTimeCounter = swapEndian(relativeTimeCounter, 6);
            unsigned char *headerCheckSum = bitManipulator(data, (long)HEADER_CHECKSUM_LENGTH);
            headerCheckSum = swapEndian(headerCheckSum, 2);

            // Change into long values
            unsigned long newChannelID = bytesToLong(channelID, CHAN_ID_LENGTH/8);
            unsigned long newPacketLength = bytesToLong(packetLength, PACKET_LENGTH_LENGTH/8);
            unsigned long newDataLength = bytesToLong(dataLength, DATA_LENGTH/8);
            unsigned long newDataTypeVer = bytesToLong(dataTypeVer, DATA_TYPE_VERSION_LENGTH/8);
            unsigned long newSeqNum = bytesToLong(seqNum, SEQ_NUM_LENGTH/8);
            unsigned long newPacketFlags = bytesToLong(packetFlags, PACKET_FLAGS_LENGTH/8);
            unsigned long newDataType = bytesToLong(dataType, DATA_TYPE_BIT_LENGTH/8);
            unsigned long newCheckSum = bytesToLong(headerCheckSum, HEADER_CHECKSUM_LENGTH/8);

            // 0x19 is a 1553 packet version format 1
            if (dataType[0] == 0x19){
                num1553++;

                // We're going to get the channel specific data now
                unsigned char *mcChar = bitManipulator(data, 24);
                mcChar = swapEndian(mcChar, 3);
                unsigned long messageCount = bytesToLong(mcChar, 3);

                unsigned char *timeTagBitsChar = bitManipulator(data, (long)2);
                int timeTagBits = (int)timeTagBitsChar[0];
                free(timeTagBitsChar);

                unsigned char *chanSpecReservedChar = bitManipulator(data, 6);

                // eat the garbage
                bitManipulator(data, 8, fSize);

                ChanSpecData thisPacketsChanSpecificData(messageCount, chanSpecReservedChar, timeTagBits);

                Messages packetMessages;
                packetMessages.numMessages = messageCount;

                // Do this for as many messages are in the packet
                for (int i = 0; i < messageCount; ++i) {
                    // Get intrapacket time stamp
                    unsigned char *intraPacketTimeStamp = bitManipulator(data, 64);

                    unsigned char *blockStatusWord = bitManipulator(data, 16);
                    blockStatusWord = swapEndian(blockStatusWord, 2);

                    long *wordSize = (long*)malloc(sizeof(long));
                    *wordSize = 2;

                    unsigned char *reserved1Char = bitManipulator(blockStatusWord, 2, wordSize);
                    int reserved1 = (int)reserved1Char[0];
                    free(reserved1Char);

                    unsigned char *bidChar = bitManipulator(blockStatusWord, 1, wordSize);
                    int BusID = (int)bidChar[0];
                    free(bidChar);

                    unsigned char *meChar = bitManipulator(blockStatusWord, 1, wordSize);
                    int BSWmessageError = (int)meChar[0];
                    free(meChar);

                    unsigned char *rtchar = bitManipulator(blockStatusWord, 1, wordSize);
                    int RT_to_RT = (int)rtchar[0];
                    free(rtchar);

                    unsigned char *formErrChar = bitManipulator(blockStatusWord, 1, wordSize);
                    int BSWformatErr = (int)formErrChar[0];
                    free(formErrChar);

                    unsigned char *respTO = bitManipulator(blockStatusWord, 1, wordSize);
                    int BSWTimeOut = (int)respTO[0];
                    free(respTO);

                    unsigned char *resCharpt1 = bitManipulator(blockStatusWord, 1, wordSize);
                    int reservPT1 = (int)resCharpt1[0];
                    free(resCharpt1);

                    // Eat the garbage byte
                    bitManipulator(blockStatusWord, 8, wordSize);

                    unsigned char *reservedCharpart2 = bitManipulator(blockStatusWord, 2, wordSize);
                    int reserved2 = (int)reservedCharpart2[0] + (reservPT1 << 2);
                    free(reservedCharpart2);

                    unsigned char *WEchar = bitManipulator(blockStatusWord, 1, wordSize);
                    int BSWWordCountError = (int)WEchar[0];
                    free(WEchar);

                    unsigned char *seChar = bitManipulator(blockStatusWord, 1, wordSize);
                    int BSWSyncErr = (int)seChar[0];
                    free(seChar);

                    unsigned char *weChar = bitManipulator(blockStatusWord, 1, wordSize);
                    int BSWWordErr = (int)weChar[0];
                    free(weChar);

                    unsigned char *resChar3 = bitManipulator(blockStatusWord, 3, wordSize);
                    int reserved3 = (int)resChar3[0];
                    free(resChar3);

                    unsigned char *gapTimesWord = bitManipulator(data, 16);
                    gapTimesWord = swapEndian(gapTimesWord, 2);

                    long *gapTimesSize = (long*)malloc(sizeof(long));
                    *gapTimesSize = 2;

                    unsigned char *gap1Char = bitManipulator(gapTimesWord, 8, gapTimesSize);
                    auto gap1 = (unsigned long)gap1Char[0];
                    free(gap1Char);

                    unsigned char *gap2Char = bitManipulator(gapTimesWord, 8, gapTimesSize);
                    auto gap2 = (unsigned long)gap2Char[0];
                    free(gap2Char);

                    unsigned char *msgLenChar = bitManipulator(data, 16);
                    auto msgLength = (unsigned long)msgLenChar[0];


                    IntraPackHeader thisMessagesHeader(intraPacketTimeStamp, reserved1, BusID, BSWmessageError, RT_to_RT,
                                                       BSWformatErr, BSWTimeOut, reserved2, BSWWordCountError, BSWSyncErr,
                                                       BSWWordErr, reserved3, gap1, gap2, msgLength);


                    // Get Command / Data words
                    unsigned char *commandWord1 = bitManipulator(data, 16);
                    commandWord1 = swapEndian(commandWord1, 2);

                    //printf("%ud:  %2x %2x\n", commandWord1, commandWord1[0], commandWord1[1]);
                    //string junk;
                    //cin >> junk;

                    // Get the second word
                    unsigned char *secondWord2 = bitManipulator(data, 16);

                    //Words thisMessageWords(commandWord1, commandWord1);
                    Words thisMessageWords(commandWord1, secondWord2);

                    // Get the data
                    int bitsLeftInMessage = (int)(msgLength * 8) - 32;
                    unsigned char *messageData = bitManipulator(data, bitsLeftInMessage);

                    packetMessages.addMessage(thisMessagesHeader, thisMessageWords, messageData);

                } // End of message loop

                myPackets.emplace_back(new Packet{newChannelID, newPacketLength,
                                                  newDataLength, newDataTypeVer, newSeqNum,
                                                  newPacketFlags, newDataType, relativeTimeCounter,
                                                  newCheckSum, messageCount, thisPacketsChanSpecificData, packetMessages});


            }
                // For all other packet types
            else {
                // determine how many bits left are in the packet
                // 192 is the total bits in the packet header
                unsigned long bitsLeft = (newPacketLength * 8) - 192;

                unsigned char *restOfPacket = bitManipulator(data, bitsLeft);

                // Using emplace_back calls the packet constructor for us
                myPackets.emplace_back(new Packet{restOfPacket, newChannelID, newPacketLength,
                                                  newDataLength, newDataTypeVer, newSeqNum,
                                                  newPacketFlags, newDataType, relativeTimeCounter,
                                                  newCheckSum});
            }



        }
        if (*fSize <= 0) {
            done = 1;
        }
        packetsCreated++;
    } // End of big loop

//    for (auto & packet : myPackets){
//        //        printf("%2x ", data[i]);
//        int numWords = packet->messages.commWords.size();
//        for (int i = 0; i < numWords; ++i) {
//            unsigned char* command = packet->messages.commWords[i].word1;
//            printf("%ud:  %2x %2x\n", command, command[0], command[1]);
//        }
//
//    }

    return myPackets;
}