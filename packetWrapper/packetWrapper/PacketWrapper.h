//
//  PacketWrapper.h
//  Wrapper
//
//  Created by Ethan Schofield on 11/23/22.
//

#import <Foundation/Foundation.h>
#import 

@interface PacketWrapper : NSObject

// Word 0x4035 attributes
bool airspeedValid;
float trueAirspeedValue;
bool angleAttackValid;
float angleAttackValue;
bool machNumValid;
float machNumValue;
float pitchAngleValue;
float rollAngleValue;
float rollRateValue;
float pitchRateValue;
float yawRateValue;
float rollAccelerationValue;
float pitchAccelerationValue;
float yawAccelerationValue;
float longitudinalAccelValue;
float lateralAccelValue;
float rollRateAFCSValue;
float lateralStickForceValue;
float longitudinalStickForceValue;
float rStabilatorDeflectionValue;
float lStabilatorDeflectionValue;
float normalAccelValue;
bool sideslipAngleINVALID;
float sideslipAngleValue;
float dynamicPressureValue;
@end
