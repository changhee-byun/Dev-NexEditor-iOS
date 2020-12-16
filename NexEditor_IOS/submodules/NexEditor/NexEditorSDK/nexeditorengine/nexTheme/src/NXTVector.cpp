//
//  NXTVector.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/29/14.
//
//

#include "NXTVector.h"

float NXTVector::out_of_bounds = 0.0;

static inline unsigned int ftoi255( float f ) {
    if( f<0.0 )
        return 0;
    if( f>1.0 )
        return 255;
    return (int)(f*255.0);
}

unsigned int NXTVector::to_argb_int_color() {
    return  (ftoi255(elements.e[3])<<24) |
    (ftoi255(elements.e[0])<<16) |
    (ftoi255(elements.e[1])<<8) |
    ftoi255(elements.e[2]);
}

NXTVector cross_product( const NXTVector& p, const NXTVector& q ) {
	float Px = p[0];
	float Py = p[1];
	float Pz = p[2];
	float Qx = q[0];
	float Qy = q[1];
	float Qz = q[2];
	return NXTVector(Py*Qz - Pz*Qy,
                     Pz*Qx - Px*Qz,
                     Px*Qy - Py*Qx);
}
