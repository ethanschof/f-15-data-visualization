//
// Created by C23Ethan.Schofield on 10/25/2022.
//

#include "ChanSpecData.h"

ChanSpecData::ChanSpecData() {
    this->msgCnt = 0;
    this->reserved = nullptr;
    this->TTB = 0;
    // call bit manipulate function to get the three attributes of this then assign them

}

ChanSpecData::ChanSpecData(unsigned long msgCnt, unsigned char *reserved, int TTB):
    msgCnt(msgCnt),
    reserved(reserved),
    TTB(TTB)
{
}