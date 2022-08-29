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
            free(tester);
        }else{
            done = 1;
        }
    }

    free(dataBuffer);
    fclose(ptr);

    system("pause");
    return 0;
}