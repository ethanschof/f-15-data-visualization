//
// Created by C23Ethan.Schofield on 10/7/2022.
//

#ifndef F_15_DATA_VISUALIZATION_TEST_STUFF_H
#define F_15_DATA_VISUALIZATION_TEST_STUFF_H

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <memory>

#include "packet.hpp"
#include "packet_factory.hpp"

using namespace std;


class test_stuff {

};

void testBitManipulate(unsigned char *dataBuffer, long fSize);

void testCommandWordInterpretation(const vector<unique_ptr<Packet>>& myPackets);

void debug(unsigned char * data, long fSize, int numPackets);

void fileDump(unsigned char *data, long fileSize, int quickDump);

int test();




#endif //F_15_DATA_VISUALIZATION_TEST_STUFF_H
