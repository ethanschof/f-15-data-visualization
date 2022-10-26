//
// Created by C23Ethan.Schofield on 10/25/2022.
//

#include "IntraPackHeader.h"

IntraPackHeader::IntraPackHeader() {
    this->timeStamp = nullptr;
    this->BSWReserved1 = 0;
    this->BSWbusID = 0;
    this->BSWRT_RT_Transfer = 0;
    this->BSWMessageError = 0;
    this->BSWFormatError = 0;
    this->BSWTimeout = 0;
    this->BSWReserved2 = 0;
    this->BSWWordCountError = 0;
    this->BSWSyncTypeError = 0;
    this->BSWInvalidWord = 0;
    this->BSWReserved3 = 0;
    this->gapTime1 = 0;
    this->gapTime2 = 0;
    this->msgLen = 0;
}

IntraPackHeader::IntraPackHeader(unsigned char *time, int res1, int busID, int RT, int mesERR, int formErr, int timeout,
                                 int res2, int wordCountErr, int syncErr, int invalWord, int res3, unsigned long gap1,
                                 unsigned long gap2, unsigned long meslen):
                                 timeStamp(time),
                                 BSWReserved1(res1),
                                 BSWbusID(busID),
                                 BSWRT_RT_Transfer(RT),
                                 BSWMessageError(mesERR),
                                 BSWFormatError(formErr),
                                 BSWTimeout(timeout),
                                 BSWReserved2(res2),
                                 BSWWordCountError(wordCountErr),
                                 BSWSyncTypeError(syncErr),
                                 BSWInvalidWord(invalWord),
                                 BSWReserved3(res3),
                                 gapTime1(gap1),
                                 gapTime2(gap2),
                                 msgLen(meslen)
                                 {

                                 }
