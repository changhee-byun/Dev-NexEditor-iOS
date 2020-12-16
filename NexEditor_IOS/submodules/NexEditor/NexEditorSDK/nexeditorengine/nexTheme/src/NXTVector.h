//
//  NXTVector.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/29/14.
//
//

#ifndef __NexVideoEditor__NXTVector__
#define __NexVideoEditor__NXTVector__

#include <iostream>
#include <math.h>

class NXTVector {

public:
    NXTVector() = default;
    NXTVector( const NXTVector& ) = default;
    NXTVector& operator=( const NXTVector& ) = default;
    
    NXTVector( float e0, float e1, float e2, float e3 ) {
        elements = (element_wrapper){ e0, e1, e2, e3 };
    }
    
    NXTVector( float e0, float e1, float e2 ) {
        elements = (element_wrapper){ e0, e1, e2, 0 };
    }
    
    float& operator[](int index) {
        if( index < 0 || index > 3 ) {
            out_of_bounds = 0.0;
            return out_of_bounds;
        } else {
            return elements.e[index];
        }
    }
    
    float operator[](int index) const {
        if( index < 0 || index > 3 ) {
            out_of_bounds = 0.0;
            return out_of_bounds;
        } else {
            return elements.e[index];
        }
    }

    bool operator==( const NXTVector& other ) {
        return(elements.e[0]==other.elements.e[0] &&
               elements.e[1]==other.elements.e[1] &&
               elements.e[2]==other.elements.e[2] &&
               elements.e[3]==other.elements.e[3] );
    }
    
    bool operator!=( const NXTVector& other ) {
        return !(*this == other);
    }
    
    NXTVector operator* (float f) {
        return NXTVector(elements.e[0] * f,
                         elements.e[1] * f,
                         elements.e[2] * f );
    }
    
    NXTVector operator* (int n) {
        return NXTVector(elements.e[0] * static_cast<float>(n),
                         elements.e[1] * static_cast<float>(n),
                         elements.e[2] * static_cast<float>(n) );
    }
    
    float length() const {
        return sqrt(elements.e[0] * elements.e[0] +
                    elements.e[1] * elements.e[1] +
                    elements.e[2] * elements.e[2] );
    }
    
    float size() const {
        return 4;
    }
    
    void set( float e0, float e1, float e2, float e3 ) {
        elements = (element_wrapper){ e0, e1, e2, e3 };
    }
    
    void normalize() {
        float vlen = length();
        elements.e[0] /= vlen;
        elements.e[1] /= vlen;
        elements.e[2] /= vlen;
    }
    
    NXTVector normalized() {
        float vlen = length();
        return NXTVector(elements.e[0] / vlen,
                         elements.e[1] / vlen,
                         elements.e[2] / vlen );
    }
    
public:
    unsigned int to_argb_int_color();
    
private:
    struct element_wrapper {
        float e[4];
    };
    static float out_of_bounds;
    element_wrapper elements;
    NXTVector(const element_wrapper& w) {
        elements = w;
    }
};

NXTVector cross_product( const NXTVector& p, const NXTVector& q );

#endif /* defined(__NexVideoEditor__NXTVector__) */
