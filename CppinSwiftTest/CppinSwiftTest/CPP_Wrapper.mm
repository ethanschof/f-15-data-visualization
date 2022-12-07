//
//  CPP_Wrapper.m
//  CppinSwiftTest
//
//  Created by Natali Schofield on 12/6/22.
//

#import <Foundation/Foundation.h>
#import "CPP_Wrapper.h"
#include "CPP.hpp"

@implementation CPP_Wrapper

-(void) hello_from_cpp_wrapped:(NSString *)userName {
    CPP cpp;
    
    cpp.hello_from_cpp([userName cStringUsingEncoding:NSUTF8StringEncoding]);
}

-(int) get_my_cpp_integer {
    CPP cpp;
    
    int myInteger = cpp.getMyInteger();
    
    return myInteger;
}


@end
