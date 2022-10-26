//
// Created by C23Ethan.Schofield on 10/25/2022.
//

#ifndef F_15_DATA_VISUALIZATION_MESSAGES_H
#define F_15_DATA_VISUALIZATION_MESSAGES_H

#include "Words.h"
#include "IntraPackHeader.h"
#include <vector>
using namespace std;

class Messages {
public:
    vector <IntraPackHeader> intraHeaders;
    vector <Words> commWords;
    vector <unsigned char*> data;
    unsigned long numMessages;

    Messages();
    void addMessage(IntraPackHeader header, Words commandWords, unsigned char* messageData);

};


#endif //F_15_DATA_VISUALIZATION_MESSAGES_H
