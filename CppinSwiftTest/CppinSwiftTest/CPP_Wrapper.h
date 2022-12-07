//
//  CPP_Wrapper.h
//  CppinSwiftTest
//
//  Created by Natali Schofield on 12/6/22.
//

#import <Foundation/Foundation.h>

@interface CPP_Wrapper : NSObject

- (void)hello_from_cpp_wrapped:(NSString *)userName;

-(int)get_my_cpp_integer;

@end
