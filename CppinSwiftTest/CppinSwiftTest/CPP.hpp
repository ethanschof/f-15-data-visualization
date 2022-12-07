//
//  CPP.hpp
//  CppinSwiftTest
//
//  Created by Natali Schofield on 12/6/22.
//

#ifndef CPP_hpp
#define CPP_hpp

#include <stdio.h>
#include <string>
using namespace std;

class CPP {
    
public:
    int getMyInteger();
    void hello_from_cpp(const string userName);
    
private:
    int myInteger;
};

#endif /* CPP_hpp */
