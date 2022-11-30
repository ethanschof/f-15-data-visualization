//
//  Packet_factory_wrapper.h
//  Wrapper
//
//  Created by Ethan Schofield on 11/23/22.
//

#import <Foundation/Foundation.h>
#import "Packet_factory.hpp"
#import "Packet.hpp"

@interface Packet_factory_wrapper : NSObject
{
    Packet packet;
    Packet_factory factory;
}
@end
