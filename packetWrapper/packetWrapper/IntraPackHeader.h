//
// Created by C23Ethan.Schofield on 10/25/2022.
//

#ifndef F_15_DATA_VISUALIZATION_INTRAPACKHEADER_H
#define F_15_DATA_VISUALIZATION_INTRAPACKHEADER_H


class IntraPackHeader {
public:
    unsigned char *timeStamp; ///< Reference time
    int BSWReserved1; ///< Reserved
    int BSWbusID;
    int BSWMessageError;
    int BSWRT_RT_Transfer;
    int BSWFormatError;
    int BSWTimeout;
    int BSWReserved2;
    int BSWWordCountError;
    int BSWSyncTypeError;
    int BSWInvalidWord;
    int BSWReserved3;
    unsigned long gapTime1;
    unsigned long gapTime2;
    unsigned long msgLen;

    IntraPackHeader ();

    IntraPackHeader(unsigned char *time, int res1, int busID, int RT, int mesERR, int formErr, int timeout, int res2,
                    int wordCountErr, int syncErr, int invalWord, int res3, unsigned long gap1, unsigned long gap2,
                    unsigned long meslen);

};


#endif //F_15_DATA_VISUALIZATION_INTRAPACKHEADER_H
