//
//  Packet_factory_wrapper.mm
//  Wrapper
//
//  Created by Ethan Schofield on 11/23/22.
//

#import <Foundation/Foundation.h>
#import "Packet_factory_wrapper.h"
#import "Packet_factory.hpp"

@implementation Packet_factory_wrapper

// Do i even need a init??
//- (id)init {
//  self = [super init];
//  if (self) {
//    factory = new Packet_factory();
//  }
//  return self;
//}

-(Packet) getOnePacket:(unsigned char*)data fileSize:(long*) fSize
{
    Packet_factory factory;
    
    Packet myPacket;
    
    myPacket = factory.getOnePacket(data, fSize);
    
    return myPacket;
}

@end

