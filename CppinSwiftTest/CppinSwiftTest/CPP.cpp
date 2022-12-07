//
//  CPP.cpp
//  CppinSwiftTest
//
//  Created by Natali Schofield on 12/6/22.
//

#include "CPP.hpp"
#include <string>
#include <iostream>


int CPP::getMyInteger(){
    return this->myInteger;
}


void CPP::hello_from_cpp(const string userName){
    cout << "Hello " << userName << " from C++";
}
