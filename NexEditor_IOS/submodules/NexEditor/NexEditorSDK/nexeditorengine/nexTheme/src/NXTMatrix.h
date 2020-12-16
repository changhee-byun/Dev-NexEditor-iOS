//
//  NXTMatrix.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/29/14.
//
//

#ifndef __NexVideoEditor__NXTMatrix__
#define __NexVideoEditor__NXTMatrix__

#include <iostream>
#include <memory>
#include <math.h>
#include "NXTVector.h"

#define PI	 3.1415926535897932384626433832795
#define PI_OVER_180	 0.017453292519943295769236907684886
#define PI_OVER_360	 0.0087266462599716478846184538424431

class NXTMatrix {
    
private:
    struct element_wrapper {
        float e[16];
    };
    static float out_of_bounds;

public:
    NXTMatrix() {
        elements = (element_wrapper){1,0,0,0,
                                     0,1,0,0,
                                     0,0,1,0,
                                     0,0,0,1};
    }
    NXTMatrix( const NXTMatrix& ) = default;
    NXTMatrix& operator=( const NXTMatrix& ) = default;
    
    float& operator[](int index) {
        if( index < 0 || index > 15 ) {
            out_of_bounds = 0.0;
            return out_of_bounds;
        } else {
            return elements.e[index];
        }
    }
    
    float operator[] (int index) const {
        if( index < 0 || index > 15 ) {
            out_of_bounds = 0.0;
            return out_of_bounds;
        } else {
            return elements.e[index];
        }
    }
    
    bool operator==( const NXTMatrix& other ) {
        return(elements.e[0]==other.elements.e[0] &&
               elements.e[1]==other.elements.e[1] &&
               elements.e[2]==other.elements.e[2] &&
               elements.e[3]==other.elements.e[3] &&
               elements.e[4]==other.elements.e[4] &&
               elements.e[5]==other.elements.e[5] &&
               elements.e[6]==other.elements.e[6] &&
               elements.e[7]==other.elements.e[7] &&
               elements.e[8]==other.elements.e[8] &&
               elements.e[9]==other.elements.e[9] &&
               elements.e[10]==other.elements.e[10] &&
               elements.e[11]==other.elements.e[11] &&
               elements.e[12]==other.elements.e[12] &&
               elements.e[13]==other.elements.e[13] &&
               elements.e[14]==other.elements.e[14] &&
               elements.e[15]==other.elements.e[15]);
    }
    
    bool operator!=( const NXTMatrix& other ) {
        return !(*this == other);
    }

    NXTMatrix& operator*=( const float other ) {
        for( float& e: elements.e )
            e *= other;
        return *this;
    }
    
    NXTMatrix& operator+=( const float other ) {
        for( float& e: elements.e )
            e += other;
        return *this;
    }
    
    NXTMatrix& operator/=( const float other ) {
        for( float& e: elements.e )
            e /= other;
        return *this;
    }
    
    NXTMatrix& operator-=( const float other ) {
        for( float& e: elements.e )
            e -= other;
        return *this;
    }
    
    NXTMatrix operator*( const NXTMatrix& other ) {
        float *a = elements.e;
        const float *b = other.elements.e;
        return (element_wrapper)
            {
                a[0] * b[0]  + a[4] * b[1]  + a[8] * b[2]   + a[12] * b[3],
                a[1] * b[0]  + a[5] * b[1]  + a[9] * b[2]   + a[13] * b[3],
                a[2] * b[0]  + a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3],
                a[3] * b[0]  + a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3],
                
                a[0] * b[4]  + a[4] * b[5]  + a[8] * b[6]   + a[12] * b[7],
                a[1] * b[4]  + a[5] * b[5]  + a[9] * b[6]   + a[13] * b[7],
                a[2] * b[4]  + a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7],
                a[3] * b[4]  + a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7],
                
                a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10]  + a[12] * b[11],
                a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10]  + a[13] * b[11],
                a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11],
                a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11],
                
                a[0] * b[12] + a[4] * b[13] + a[8] * b[14]  + a[12] * b[15],
                a[1] * b[12] + a[5] * b[13] + a[9] * b[14]  + a[13] * b[15],
                a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15],
                a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15]
            };
    }
    
    NXTMatrix& operator*=( const NXTMatrix& other ) {
        
        float *a = elements.e;
        const float *b = other.elements.e;
        elements = (element_wrapper)
        {
            a[0] * b[0]  + a[4] * b[1]  + a[8] * b[2]   + a[12] * b[3],
            a[1] * b[0]  + a[5] * b[1]  + a[9] * b[2]   + a[13] * b[3],
            a[2] * b[0]  + a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3],
            a[3] * b[0]  + a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3],
            
            a[0] * b[4]  + a[4] * b[5]  + a[8] * b[6]   + a[12] * b[7],
            a[1] * b[4]  + a[5] * b[5]  + a[9] * b[6]   + a[13] * b[7],
            a[2] * b[4]  + a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7],
            a[3] * b[4]  + a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7],
            
            a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10]  + a[12] * b[11],
            a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10]  + a[13] * b[11],
            a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11],
            a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11],
            
            a[0] * b[12] + a[4] * b[13] + a[8] * b[14]  + a[12] * b[15],
            a[1] * b[12] + a[5] * b[13] + a[9] * b[14]  + a[13] * b[15],
            a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15],
            a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15]
        };

        return *this;
    }
    
    NXTVector operator*( const NXTVector& v ) const {
        const float *m = elements.e;
        return NXTVector(m[0]*v[0] + m[4]*v[1] + m[ 8]*v[2] + m[12]*v[3],
                         m[1]*v[0] + m[5]*v[1] + m[ 9]*v[2] + m[13]*v[3],
                         m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14]*v[3],
                         m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15]*v[3]);
    }
    
    void setIdentity() {
        elements = (element_wrapper)
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
    }
    
    void set(float e0,  float e1,  float e2,  float e3,
             float e4,  float e5,  float e6,  float e7,
             float e8,  float e9,  float e10, float e11,
             float e12, float e13, float e14, float e15)
    {
        elements = (element_wrapper)
        {
            e0,  e1,  e2,  e3,
            e4,  e5,  e6,  e7,
            e8,  e9,  e10, e11,
            e12, e13, e14, e15
        };
    }
    
    static NXTMatrix scale( float x, float y, float z ) {
        return (element_wrapper)
        {
            x,0,0,0,
            0,y,0,0,
            0,0,z,0,
            0,0,0,1
        };
    }
    
    static NXTMatrix scale( float factor ) {
        return (element_wrapper)
        {
            factor,0,0,0,
            0,factor,0,0,
            0,0,factor,0,
            0,0,0,1
        };
    }
    
    static NXTMatrix translate( const NXTVector& v ) {
        return (element_wrapper)
        {
            1,0,0,v[0],
            0,1,0,v[1],
            0,0,1,v[2],
            0,0,0,1
        };
    }

    static NXTMatrix translate( float x, float y, float z ) {
        return (element_wrapper)
        {
            1,0,0,x,
            0,1,0,y,
            0,0,1,z,
            0,0,0,1
        };
    }
    
    static NXTMatrix translate( float x, float y ) {
        return (element_wrapper)
        {
            1,0,0,x,
            0,1,0,y,
            0,0,1,0,
            0,0,0,1
        };
    }
    
    static NXTMatrix biased_yuv_to_rgb() {
        return (element_wrapper)
        {
            1.164,  0.000,  1.596, -0.871,
            1.164, -0.392, -0.813,  0.530,
            1.164,  2.017,  0.000, -1.081,
            0.000,  0.000,  0.000,  1.000
        };
    }
    
    static NXTMatrix rgb_to_yuv() {
        return (element_wrapper)
        {
            0.257,	0.504,	0.098,	0.000,
            -0.148,	-0.291,	0.439,	0.000,
            0.439,	-0.368,	-0.071,	0.000,
            0.000,	0.000,	0.000,	1.000
        };
    }
    
    static NXTMatrix yuv_to_rgb() {
        return (element_wrapper)
        {
            1.164,  0.000,  1.596,  0.000,
            1.164, -0.392, -0.813,  0.000,
            1.164,  2.017,  0.000,  0.000,
            0.000,  0.000,  0.000,  1.000
        };
    }
    
    static NXTMatrix jpeg_yuv_to_rgb() {
        return (element_wrapper)
        {
            1.000,  0.000,  1.402, 0.000,
			1.000, -0.344, -0.714, 0.000,
			1.000,  1.772,  0.000, 0.000,
			0.000,  0.000,  0.000, 1.000
        };
    }
    
    static NXTMatrix rgb_to_biased_yuv() {
        return (element_wrapper)
        {
            0.257,	0.504,	0.098,	0.0625,
            -0.148,	-0.291,	0.439,	0.500,
            0.439,	-0.368,	-0.071,	0.500,
            0.000,	0.000,	0.000,	1.000
        };
    }
    
    static NXTMatrix color_adjust(float brightness,  // -1 ... +1
                                  float contrast,    // -1 ... +1
                                  float saturation,  // -1 ... +1
                                  const NXTVector tintColor = NXTVector())
    {
        if( brightness < -1.0 ) {
            brightness = -1.0;
        } else if( brightness > 1.0 ) {
            brightness = 1.0;
        }
        
        if( contrast < -1.0 ) {
            contrast = -1.0;
        } else if( contrast > 1.0 ) {
            contrast = 1.0;
        }
        
        
        NXTMatrix brightnessAdjust;
        brightnessAdjust = NXTMatrix::translate( brightness, 0.0, 0.0 );
        
        NXTMatrix contrastAdjust = NXTMatrix::translate( 0.5, 0.5, 0.5 );
        contrastAdjust *= NXTMatrix::scale( contrast+1 );
        contrastAdjust *= NXTMatrix::translate( -0.5, -0.5, -0.5 );
        
        NXTMatrix satAdjust = NXTMatrix::scale( 1, saturation+1, saturation+1);

        NXTMatrix bias = NXTMatrix::translate(0.0, -0.5, -0.5);
        
        float r = tintColor[0];
        float g = tintColor[1];
        float b = tintColor[2];
        float total = r+g+b;
        const float r_weight = 0.241f;
        const float g_weight = 0.691f;
        const float b_weight = 0.068f;
        float rc = total==0?1:((r/total)*3.0);
        float gc = total==0?1:((g/total)*3.0);
        float bc = total==0?1:((b/total)*3.0);
        int i;
        
        NXTMatrix tintAdjust;
        if( saturation < 0 ) {
            tintAdjust.set(
                    rc*r_weight, rc*g_weight, rc*b_weight, 0.0,             // Red      output
                    gc*r_weight, gc*g_weight, gc*b_weight, 0.0,             // Green    output
                    bc*r_weight, bc*g_weight, bc*b_weight, 0.0,             // Blue     output
                    0.0,         0.0,         0.0,         1.0
            );
            
            static const NXTMatrix identity = NXTMatrix::identity();
            for( i=0; i<16; i++ ) {
                tintAdjust[i] = (tintAdjust[i] * (1-(saturation+1))) + (identity[i] * (saturation+1));
            }
        }
        
        return (tintAdjust*(((NXTMatrix::yuv_to_rgb()*(satAdjust*bias))*brightnessAdjust)*contrastAdjust)) * NXTMatrix::rgb_to_biased_yuv();

        
    }
    
    static NXTMatrix identity() {
        return (element_wrapper)
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
    }
    
    static NXTMatrix perspective( float fov_degrees, float aspect, float z_near, float z_far ) {
        float f = 1.0f / tanf((fov_degrees*PI_OVER_180)/2.0f);
        return (element_wrapper)
        {
            f/aspect,   0.0,        0.0,                            0.0,
            0.0,        f,          0.0,                            0.0,
            0.0,        0.0,        (z_far+z_near)/(z_near-z_far),  2*z_far*z_near/(z_near-z_far),
            0.0,        0.0,        -1.0,  0.0
        };
    }
    
    static NXTMatrix ortho( float left, float right, float bottom, float top, float near, float far ) {
        float r_l = right - left;
        float t_b = top - bottom;
        float f_n = far - near;
        float tx = - (right + left) / (right - left);
        float ty = - (top + bottom) / (top - bottom);
        float tz = - (far + near) / (far - near);
        return (element_wrapper)
        {
            2.0f/r_l,   0.0,        0.0,        0.0,
            0.0,        2.0f/t_b,   0.0,        0.0,
            0.0,        0.0,        -2.0f/f_n,  0.0,
            tx,         ty,         tz,         1.0
        };
    }
    
    static NXTMatrix frustum( float left, float right, float bottom, float top, float near, float far ) {
        float   a = (right+left)/(right-left);
        float   b = (top+bottom)/(top-bottom);
        float   c = (far+near)/(far-near);
        float   d = (2*far*near)/(far-near);
        return (element_wrapper)
        {
            (2*near)/(right-left),  0.0,                    0.0,    0.0,
            0.0,                    (2*near)/(top-bottom),  0.0,    0.0,
            a,                       b,                     c,      d,
            0.0,                    0.0,                    -1,     0.0
        };
    }

    
    static NXTMatrix rotate( const NXTVector& axis, float angle_radians ) {
        float length = axis.length();
        float ux = axis[0]/length;
        float uy = axis[1]/length;
        float uz = axis[2]/length;
        float cosa = cosf(angle_radians);
        float sina = sinf(angle_radians);
        
        return (element_wrapper)
        {
            cosa + ux*ux*(1-cosa),  ux*uy*(1-cosa) - uz*sina,   ux*uz*(1-cosa) + uy*sina,   0.0,
            uy*ux*(1-cosa)+uz*sina, cosa + uy*uy*(1-cosa),      uy*uz*(1-cosa) - ux*sina,   0.0,
            uz*ux*(1-cosa)-uy*sina, uz*uy*(1-cosa)+ux*sina,     cosa + uz*uz*(1-cosa),      0.0,
            0.0,                    0.0,                        0.0,                        1.0
        };
    }
    
    
private:
    element_wrapper elements;
    NXTMatrix(const element_wrapper& w) {
        elements = w;
    }
};

#endif /* defined(__NexVideoEditor__NXTMatrix__) */
