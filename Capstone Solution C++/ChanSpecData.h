//
// Created by C23Ethan.Schofield on 10/25/2022.
//

#ifndef F_15_DATA_VISUALIZATION_CHANSPECDATA_H
#define F_15_DATA_VISUALIZATION_CHANSPECDATA_H


class ChanSpecData {
public:
    unsigned long msgCnt; ///< Message count
    unsigned char *reserved;
    int TTB; ///< Time tag bit
    ChanSpecData();

    ChanSpecData(unsigned long msgCnt, unsigned char *reserved, int TTB);

};


#endif //F_15_DATA_VISUALIZATION_CHANSPECDATA_H
