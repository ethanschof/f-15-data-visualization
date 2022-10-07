//
// Created by C23Ethan.Schofield on 10/7/2022.
//

#include "test_stuff.h"
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>

#include "packet.h"

using namespace std;

/**
 * @brief Outputs the contents of a file as hex values
 *
 * @param data the buffer data from the file
 * @param fileSize the size in bytes of the file
 * @param quickDump if not 0, dump only the first 200 * quickDump bytes
 */
void fileDump(unsigned char *data, long fileSize, int quickDump){
    cout << "\n:::::BEGINNING FILE DUMP:::::\n";
    int stopper = fileSize-byteIndex;
    if(quickDump){
        stopper = 200 * quickDump;
    }
    //test case 1: output hex
    for(int i = byteIndex; i < stopper+byteIndex; i++){
        if(i%50 == 0){
            printf("\n%7d:  ", i);
        }
        printf("%2x ", data[i]);
    }
    cout << "\n\n";
}

/**
 * @brief takes in an array and returns the number of bits from the front requested and reallocates the file Buffer to no longer have those bits
 *          returns in BIG ENDIAN
 *
 * @bug Will not decriment if bits do not result in a byte. Ex 2 function calls for 4 bits will result in the front byte being '00' and will need to be consumed
 *
 * @param data the buffer data from the file
 * @param numBits the number of bits needed
 * @param fSize the number of bytes in the buffer
 * @return unsigned char* the bits at the top of the array of bytes
 */
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

/**
 * @brief takes in the file Buffer ONLY and returns the number of bits from the front requested and reallocates the file Buffer to no longer have those bits
 *          returns in BIG ENDIAN
 *
 * @bug Will not decriment if bits do not result in a byte. Ex 2 function calls for 4 bits will result in the front byte being '00' and will need to be consumed
 *
 * @param data the buffer data from the file
 * @param numBits the number of bits needed
 * @param fSize the number of bytes in the buffer
 * @return unsigned char* the bits at the top of the array of bytes
 */

void debug(unsigned char * data, long fSize, int numPackets){

    //search for next packet sync
    int i =0;
    if(!numPackets){
        int Packetnum = 1;
        while(1){
            if(data[i] == 0x25 && data[i+1] == 0xEB){
                //cout << "Packet " << Packetnum <<" is " << i << " bytes away...\n";
                Packetnum++;
            }
            i++;
            if(i >= fSize - 2){
                i = -1;
                cout << "There are " << Packetnum << " packets in this file.\n";
                break;
            }
        }
    }else{
        fileDump(data, fSize, 2);
        int packetsFound = 0;
        while(numPackets > packetsFound){
            while(data[i] != 0x25 && data[i+1] != 0xEB){
                i++;
                if(i >= fSize - 2){
                    i = -1;
                    break;
                }
            }
            cout << "The next packet is " << i << " bytes away...\n";
            i++;
            packetsFound++;
        }
    }

    // This causes a seg fault
    // bitManipulator(data, i*8, &fSize);
    // system("pause");
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

void testCommandWordInterpretation(const vector<unique_ptr<Packet>>& myPackets){

    for (auto & packet : myPackets){

        if (packet->isPrintable()){
            packet->interpretData(true);
        }

    }
}

int test(){
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

    debug(dataBuffer, fSize, 0);
    string junk;
    cin >> junk;
    // Uncomment to test the bit manipulate function
    // testBitManipulate(dataBuffer, fSize);
    //bitManipulator(dataBuffer, (41336 * 8), &fSize);

    // Putting the packets into a data structure
    vector<unique_ptr<Packet>> myPackets;
    myPackets = Packet.(dataBuffer, &fSize, false);

    // this call tests the interpretation of command word data
    testCommandWordInterpretation(myPackets);


    free(dataBuffer);
    fclose(ptr);
    return 0;
}
