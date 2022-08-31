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

using namespace std;

/**
 * @brief Outputs the contents of a file as hex values
 * 
 * @param data the buffer data from the file
 * @param fileSize the size in bytes of the file
 * @param quickDump if 1, dump only the first 200 bytes
 */
void fileDump(unsigned char *data, long fileSize, int quickDump){
    //pause the program
    cout << "\n:::::BEGINNING FILE DUMP:::::\n";
    int stopper = fileSize;
    if(quickDump){
        stopper = 200;
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
unsigned char* LittleEndianToBigEndian(unsigned char* bytes, int numBytes){
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
            unsigned char *packetLength = bitManipulator(data, (long)PACKET_LENGTH_LENGTH, fSize);
            unsigned char *dataLength = bitManipulator(data, (long)DATA_LENGTH, fSize);
            unsigned char *dataTypeVer = bitManipulator(data, (long)DATA_TYPE_VERSION_LENGTH, fSize);
            unsigned char *seqNum = bitManipulator(data, (long)SEQ_NUM_LENGTH, fSize);
            unsigned char *packetFlags = bitManipulator(data, (long)PACKET_FLAGS_LENGTH, fSize);
            unsigned char *dataType = bitManipulator(data, (long)DATA_TYPE_BIT_LENGTH, fSize);
            unsigned char *relativeTimeCounter = bitManipulator(data, (long)RELATIVE_TIME_COUNTER_LENGTH, fSize);
            unsigned char *headerCheckSum = bitManipulator(data, (long)HEADER_CHECKSUM_LENGTH, fSize);

            // 0x19 SHOULD be a 1553 packet, need to double-check
            if (dataType[0] == 0x19){

            }
            // For all other packet types
            else {
                // determine how many bits left are in the packet
                long bitsLeft = (long)*packetLength - PACKET_SYNC_LENGTH - CHAN_ID_LENGTH - PACKET_LENGTH_LENGTH - DATA_LENGTH;

                unsigned char *restOfPacket = bitManipulator(data, bitsLeft, fSize);

                // Using emplace_back calls the packet constructor for us
                myPackets.emplace_back(restOfPacket, channelID, packetLength,
                                       dataLength, dataTypeVer,seqNum,
                                       packetFlags, dataType, relativeTimeCounter,
                                       headerCheckSum);
            }



        }

    }
    return myPackets;
}


int main(){
    cout << "==========================\n F15 packet data analyzer \n==========================\n\n";

    //get file to open
    char fileName[25];
    cout << "Enter the name of the ch10 file: ";
    cin >> fileName;
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
            unsigned char* tester2 = LittleEndianToBigEndian(tester, numBytes);
            for(int i = 0; i < numBytes; i++){
                printf("%2x ", tester2[i]);
            }
            cout << "\n\n";

            free(tester);
        }else{
            done = 1;
        }
    }

    free(dataBuffer);
    fclose(ptr);
    return 0;
}