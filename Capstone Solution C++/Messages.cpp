//
// Created by C23Ethan.Schofield on 10/25/2022.
//

#include "Messages.h"

Messages::Messages() {
    vector<IntraPackHeader> iph;
    vector <Words> w;
    vector <unsigned char*> d;

    this->intraHeaders = iph;
    this->commWords = w;
    this->data = d;
    this->numMessages = 999999;
}

void Messages::addMessage(IntraPackHeader header, Words commandWords, unsigned char *messageData) {
    this->intraHeaders.push_back(header);
    this->commWords.push_back(commandWords);
    this->data.push_back(messageData);
}
