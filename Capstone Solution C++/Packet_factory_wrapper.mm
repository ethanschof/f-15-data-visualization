//
//  Packet_factory_wrapper.mm
//  Wrapper
//
//  Created by Aaron Schofield on 11/23/22.
//

#import <Foundation/Foundation.h>
#import "Packet_factory_wrapper.h"

@implementation Packet_factory_wrapper

-(Packet) getOnePacket
{
    Packet_factory factory;
    
    return factory.getOnePacket(<#unsigned char *data#>, <#long *fSize#>)
}

@end

