/**
 * @file main.cpp
 * @author C1C Colin Seymour
 * @brief Allow the user to input a ch10 file and interpret the data into human readable information
 * @version 0.2
 * @date 2022-08-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//basic c++ functions
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>

#include "ch10.hpp"
#include "packet.hpp"

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
#define CHANNEL_SPECIFIC_DATA_LENGTH 32

using namespace std;

/**
 * @brief Outputs the contents of a file as hex values
 * 
 * @param data the buffer data from the file
 * @param fileSize the size in bytes of the file
 * @param quickDump if not 0, dump only the first 200 * quickDump bytes
 */
void fileDump(unsigned char *data, long fileSize, int quickDump){
    //pause the program
    cout << "\n:::::BEGINNING FILE DUMP:::::\n";
    int stopper = fileSize;
    if(quickDump){
        stopper = 200 * quickDump;
    }
    //test case 1: output hex
    //printf("      0:  ");
    for(int i = 0; i < stopper; i++){
        if(i%50 == 0){
            printf("\n%7d:  ", i);
        }
        printf("%2x ", data[i]);
    }
    cout << "\n\n";
}
/**
 * @brief quick dumps data and stops for testing
 * @param data
 * @param fSize
 */
void debug(unsigned char * data, long fSize){
    fileDump(data, fSize, 2);

    //search for next packet sync
    int i =0;
    while(data[i] != 0x25 && data[i+1] != 0xEB){
        i++;
        if(i >= fSize - 2){
            i = -1;
            break;
        }
    }
    cout << "\nThe next packet is " << i << " bytes away...\n";

    cout << "\nC++ is dumb, ascend to C [y/n]: ";
    char waste;
    cin >> waste;
}

/**
 * @brief takes in a file Buffer and returns the number of bits from the front requested and reallocates the file Buffer to no longer have those bits
 *          returns in BIG ENDIAN
 * 
 * @bug Will not decriment if bits do not result in a byte. Ex 2 function calls for 4 bits will result in the front byte being '00' and will need to be consumed
 * 
 * @param data the buffer data from the file
 * @param numBits the number of bits needed
 * @param fSize the number of bytes in the buffer
 * @return unsigned char* the bits at the top of the array of bytes
 */
unsigned char *bitManipulator(unsigned char* data, int numBits, long *fSize){
    int numBytes = numBits / 8;
    int bitShift = 0;

    if(numBits%8 != 0){
        //deal with truncation
        numBytes++;
        bitShift = 1;
    }

    unsigned char *desiredBits = (unsigned char *)malloc(numBytes * sizeof(unsigned char));
    for(int i = 0; i < numBytes; i++){
        desiredBits[i] = data[i];
    }

    if(bitShift){
        //for non-byte sized needs...
        desiredBits[numBytes-1] = desiredBits[numBytes-1] >> (8 - numBits%8);
        data[numBytes-1] = data[numBytes-1] << (numBits%8);
        numBytes--;
    }
    
    //remove used bytes
    for(int i = 0; i < *fSize; i++){
        data[i] = data[i+numBytes];
    }
    *fSize = *fSize - numBytes;
    return desiredBits;
}

/**
 * @brief converts array of unsigned char into a long value, must be in Little Endian
 *      --CONCEPTUAL--
 *      bitval = 2^[bit# + (byte# * 8)]
 *      totval = totval + bit*bitval
 * 
 * @param bytes the bytes and bits to be converted
 * @param numBytes the length of the array
 * @return long values contained in the array
 */
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

/**
 * @brief reverses position of bytes in the array
 * 
 * @param bytes the array of bytes
 * @param numBytes the length of the array
 * 
 * @return the array of bytes in Big Endian notation
 */
unsigned char* swapEndian(unsigned char* bytes, int numBytes){
    unsigned char* BEBytes = (unsigned char*)malloc(numBytes * sizeof(unsigned char));
    for(int i = 0; i < numBytes; i++){
        BEBytes[(numBytes-1)-i] = bytes[i];
        bytes[i] = BEBytes[(numBytes-1)-i];
    }
    return BEBytes;
}
/**
 *
 * @param data the buffer data from the file
 * @return an array of Packet objects containing the data from the file
 */
vector<Packet> createPackets(unsigned char* data, long* fSize){
    vector<Packet> myPackets;
    int done = 0;
    int packetsCreated = 0;

    while (!done){
        unsigned char *packetSync = bitManipulator(data, (long)PACKET_SYNC_LENGTH, fSize);

        // checks for packet sync
        if (packetSync[0] == 0x25 && packetSync[1] == 0xEB){
            // Get data from the packet header MUST STAY IN THIS ORDER
            unsigned char *channelID = bitManipulator(data, (long)CHAN_ID_LENGTH, fSize);
            channelID = swapEndian(channelID, 2);
            unsigned char *packetLength = bitManipulator(data, (long)PACKET_LENGTH_LENGTH, fSize);
            packetLength = swapEndian(packetLength, 4);
            unsigned char *dataLength = bitManipulator(data, (long)DATA_LENGTH, fSize);
            dataLength = swapEndian(dataLength, 4);
            unsigned char *dataTypeVer = bitManipulator(data, (long)DATA_TYPE_VERSION_LENGTH, fSize);
            unsigned char *seqNum = bitManipulator(data, (long)SEQ_NUM_LENGTH, fSize);
            unsigned char *packetFlags = bitManipulator(data, (long)PACKET_FLAGS_LENGTH, fSize);
            unsigned char *dataType = bitManipulator(data, (long)DATA_TYPE_BIT_LENGTH, fSize);
            unsigned char *relativeTimeCounter = bitManipulator(data, (long)RELATIVE_TIME_COUNTER_LENGTH, fSize);
            relativeTimeCounter = swapEndian(relativeTimeCounter, 6);
            unsigned char *headerCheckSum = bitManipulator(data, (long)HEADER_CHECKSUM_LENGTH, fSize);
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
                // We're going to get the channel specific data now
                unsigned char *mcChar = bitManipulator(data, 24, fSize);
                mcChar = swapEndian(mcChar, 3);
                unsigned long messageCount = bytesToLong(mcChar, 3);

                unsigned char *timeTagBitsChar = bitManipulator(data, (long)2, fSize);
                int timeTagBits = (int)timeTagBitsChar[0];
                free(timeTagBitsChar);

                unsigned char *chanSpecReservedChar = bitManipulator(data, 6, fSize);

                // eat the garbage
                bitManipulator(data, 8, fSize);

                ChanSpecData thisPacketsChanSpecificData(messageCount, chanSpecReservedChar, timeTagBits);

                Messages packetMessages;
                packetMessages.numMessages = messageCount;

                // Do this for as many messages are in the packet
                for (int i = 0; i < messageCount; ++i) {
                    // Get intrapacket time stamp
                    unsigned char *intraPacketTimeStamp = bitManipulator(data, 64, fSize);

                    unsigned char *blockStatusWord = bitManipulator(data, 16, fSize);
                    blockStatusWord = swapEndian(blockStatusWord, 2);

                    long *wordSize = nullptr;
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

                    unsigned char *gapTimesWord = bitManipulator(data, 16, fSize);
                    gapTimesWord = swapEndian(gapTimesWord, 2);

                    long *gapTimesSize = nullptr;
                    *gapTimesSize = 2;

                    unsigned char *gap1Char = bitManipulator(gapTimesWord, 8, gapTimesSize);
                    auto gap1 = (unsigned long)gap1Char[0];
                    free(gap1Char);

                    unsigned char *gap2Char = bitManipulator(gapTimesWord, 8, gapTimesSize);
                    auto gap2 = (unsigned long)gap2Char[0];
                    free(gap2Char);

                    unsigned char *msgLenChar = bitManipulator(data, 16, fSize);
                    auto msgLength = (unsigned long)msgLenChar[0];


                    IntraPackHeader thisMessagesHeader(intraPacketTimeStamp, reserved1, BusID, BSWmessageError, RT_to_RT,
                                                      BSWformatErr, BSWTimeOut, reserved2, BSWWordCountError, BSWSyncErr,
                                                      BSWWordErr, reserved3, gap1, gap2, msgLength);

                    // Get Command / Data words
                    unsigned char *commandWord1 = bitManipulator(data, 16, fSize);

                    // TODO this also could be where the data count goes off because I'm not sure when a second word is there
                    // Get the second word
                    unsigned char *secondWord2 = bitManipulator(data, 16, fSize);

                    Words thisMessageWords(commandWord1, secondWord2);

                    // Get the data
                    // TODO this might be where the data count gets off
                    int bitsLeftInMessage = (int)(msgLength * 8) - 32;
                    unsigned char *messageData = bitManipulator(data, bitsLeftInMessage, fSize);

                    packetMessages.addMessage(thisMessagesHeader, thisMessageWords, messageData);

                } // End of message loop

                myPackets.push_back(P1553(newChannelID, newPacketLength,
                                          newDataLength, newDataTypeVer, newSeqNum,
                                          newPacketFlags, newDataType, relativeTimeCounter,
                                          newCheckSum, messageCount, thisPacketsChanSpecificData, packetMessages));


            }
            // For all other packet types
            else {
                // determine how many bits left are in the packet
                // 192 is the total bits in the packet header
                long bitsLeft = ((long)*packetLength * 8) - 192;

                unsigned char *restOfPacket = bitManipulator(data, bitsLeft, fSize);


                // Using emplace_back calls the packet constructor for us
                myPackets.emplace_back(restOfPacket, newChannelID, newPacketLength,
                                       newDataLength, newDataTypeVer,newSeqNum,
                                       newPacketFlags, newDataType, relativeTimeCounter,
                                       newCheckSum);
            }



        }
        debug(data, *fSize);
        if (*fSize <= 0) {
            done = 1;
        }

    } // End of big loop
    return myPackets;
}

void testBitManipulate(unsigned char *dataBuffer, long fSize){
    //SANITY CHECKER #1
    //ask user if they would like to dump the current data in 'dataBuffer'
    cout << "Would you like to dump the data to the terminal? [y/n] ";
    char shouldDump;
    cin >> shouldDump;
    if(shouldDump == 'y'){
        cout << "Would you like to preform a quick dump? [y/n] ";
        cin >> shouldDump;
        int quickDump = 0;
        if(shouldDump == 'y'){
            quickDump = 1;
        }
        fileDump(dataBuffer, fSize, quickDump);
    }

    //SANITY CHECKER #2
    //get a number of the bits to output for the user
    int done = 0;
    while(!done){
        char checkBits;
        cout << "Would you like to get some bits off out of the Buffer? [y/n] ";
        cin >> checkBits;
        if(checkBits == 'y'){
            cout << "How many bits? ";
            int input;
            cin >> input;
            unsigned char *tester = bitManipulator(dataBuffer, input, &fSize);
            //output desired bits
            int numBytes = input/8;
            if(input%8){numBytes++;}

            cout << "DESIRED BITS:  ";
            for(int i = 0; i < numBytes; i++){
                printf("%2x ", tester[i]);
            }
            cout << "\n";

            //convert to long SANITY CHECK #3
            cout << "DECIMAL VALUE:  " << bytesToLong(tester, numBytes) << "\n";

            //convert to Big Endian Notation SANITY CHECK #4
            cout << "BIG ENDIAN:  ";
            unsigned char* tester2 = swapEndian(tester, numBytes);
            for(int i = 0; i < numBytes; i++){
                printf("%2x ", tester2[i]);
            }
            cout << "\n\n";

            free(tester);
        }else{
            done = 1;
        }
    }
}

int main(){
    cout << "==========================\n F15 packet data analyzer \n==========================\n\n";

    //get file to open
    char fileName[25];
    cout << "Enter the name of the ch10 file: ";
    cin >> fileName;
    //char* fileName = "myChap10.ch10";
    FILE *ptr;
    ptr = fopen(fileName, "rb");

    if(ptr == NULL){
        cout << "Failed to open file.\n";
        return 1;
    }

    //find the size of the file
    fseek(ptr, 0, SEEK_END);
    long fSize = ftell(ptr); 
    cout << "file size (in bytes) : " << fSize << "\n";

    //ignore ch10 header info
    fseek(ptr, 0, SEEK_SET);

    //transfer binary into a buffer
    unsigned char *dataBuffer = (unsigned char*)malloc(fSize * sizeof(unsigned char));
    for(int i = 0; i < fSize; i++){
        dataBuffer[i] = (unsigned char)fgetc(ptr);
    }

    debug(dataBuffer, fSize);

    // Uncomment to test the bit manipulate function
    // testBitManipulate(dataBuffer, fSize);

    // Putting the packets into a data structure
    vector<Packet> myPackets;
    myPackets = createPackets(dataBuffer, &fSize);


    free(dataBuffer);
    fclose(ptr);
    return 0;
}