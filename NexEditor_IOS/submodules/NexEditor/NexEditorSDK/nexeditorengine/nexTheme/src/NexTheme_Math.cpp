#include "NexTheme_Config.h"
#include "NexTheme_Math.h"
#include <math.h>

#define LOG_TAG "NexTheme_Math"

#define PI	 3.1415926535897932384626433832795
#define PI_OVER_180	 0.017453292519943295769236907684886
#define PI_OVER_360	 0.0087266462599716478846184538424431

unsigned int NXT_NextHighestPowerOfTwo( unsigned int v ) {
	// v--;
	// v |= v >> 1;
	// v |= v >> 2;
	// v |= v >> 4;
	// v |= v >> 8;
	// v |= v >> 16;
	// v++;
	return v;
}

NXT_Matrix4f NXT_Matrix4f_Identity( void ) {
    return NXT_Matrix4f(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
}

NXT_Matrix4f NXT_Matrix4f_Scale( float xscale, float yscale, float zscale ) {
    return NXT_Matrix4f(
            xscale, 0.0,    0.0,    0.0,
            0.0,    yscale, 0.0,    0.0,
            0.0,    0.0,    zscale, 0.0,
            0.0,    0.0,    0.0,    1.0
        );
}

float NXT_Vector4f_Length( NXT_Vector4f v ) {
    return sqrt( v.e[0] * v.e[0] +  v.e[1] * v.e[1] + v.e[2] * v.e[2] );
}

NXT_Matrix4f NXT_Matrix4f_Rotate( NXT_Vector4f axis, float angle_radians ) {
    float length = NXT_Vector4f_Length( axis );
    float ux = axis.e[0]/length;
    float uy = axis.e[1]/length;
    float uz = axis.e[2]/length;
    float cosa = cosf(angle_radians);
    float sina = sinf(angle_radians);
    //y LOGXV("angle_radians=%f length=%f, ux=%f, uy=%f, uz=%f, "
    //y     "cosa=%f, sina=%f, axis=[%f %f %f %f]",
    //y     angle_radians, length, ux, uy, uz, cosa, sina,
    //y     axis.e[0], axis.e[1], axis.e[2], axis.e[3]);
    
    return NXT_Matrix4f(
            cosa + ux*ux*(1-cosa),      ux*uy*(1-cosa) - uz*sina,       ux*uz*(1-cosa) + uy*sina,       0.0,
            uy*ux*(1-cosa)+uz*sina,     cosa + uy*uy*(1-cosa),          uy*uz*(1-cosa) - ux*sina,       0.0,
            uz*ux*(1-cosa)-uy*sina,     uz*uy*(1-cosa)+ux*sina,         cosa + uz*uz*(1-cosa),          0.0,
            0.0,                        0.0,                            0.0,                            1.0
        );
}

NXT_Matrix4f NXT_Matrix4f_Translate( NXT_Vector4f v ) {
    /*return (NXT_Matrix4f){
        {
            1.0,    0.0,    0.0,    0.0,
            0.0,    1.0,    0.0,    0.0,
            0.0,    0.0,    1.0,    0.0,
            v.e[0], v.e[1], v.e[2], 1.0
        }
    };*/
    return NXT_Matrix4f(
            1.0,    0.0,    0.0,    v.e[0],
            0.0,    1.0,    0.0,    v.e[1],
            0.0,    0.0,    1.0,    v.e[2],
            0.0,    0.0,    0.0,    1.0
        );
}

NXT_Matrix4f NXT_Matrix4f_Perspective( float fov_degrees, float aspect, float z_near, float z_far ) {
	float f = 1.0f / tanf((fov_degrees*PI_OVER_180)/2.0f);

    return NXT_Matrix4f(
            f/aspect,   0.0,        0.0,                            0.0,
            0.0,        f,          0.0,                            0.0,
            0.0,        0.0,        (z_far+z_near)/(z_near-z_far),  2*z_far*z_near/(z_near-z_far),
            0.0,        0.0,        -1.0,  0.0
        );
}

NXT_Matrix4f NXT_Matrix4f_Transpose( NXT_Matrix4f mx ) {
    float *m = mx.e;
    return NXT_Matrix4f(
            m[0], m[4], m[8], m[12],
            m[1], m[5], m[9], m[13],
            m[2], m[6], m[10], m[14],
            m[3], m[7], m[11], m[15]
        );
}

/*NXT_Matrix4f NXT_Matrix4f_Perspective(float fov, float aspect,
                       float znear, float zfar)
{
    //0.00872664626
    NXT_Matrix4f result;
    
    float xymax = znear * tan(fov * PI_OVER_360);
    float ymin = -xymax;
    float xmin = -xymax;
    
    float width = xymax - xmin;
    float height = xymax - ymin;
    
    float depth = zfar - znear;
    float q = -(zfar + znear) / depth;
    float qn = -2 * (zfar * znear) / depth;
    
    float w = 2 * znear / width;
    w = w / aspect;
    float h = 2 * znear / height;
    
    result.e[0]  = w;
    result.e[1]  = 0;
    result.e[2]  = 0;
    result.e[3]  = 0;
    
    result.e[4]  = 0;
    result.e[5]  = h;
    result.e[6]  = 0;
    result.e[7]  = 0;
    
    result.e[8]  = 0;
    result.e[9]  = 0;
    result.e[10] = q;
    result.e[11] = -1;
    
    result.e[12] = 0;
    result.e[13] = 0;
    result.e[14] = qn;
    result.e[15] = 0;
    
    return result;
}*/


static NXT_Matrix4f yuvrgb = NXT_Matrix4f(
                                1.164,  0.000,  1.596, -0.871,
                                1.164, -0.392, -0.813,  0.530,
                                1.164,  2.017,  0.000, -1.081,
                                0.000,  0.000,  0.000,  1.000);


/*static NXT_Matrix4f yuvrgb = {{ 1.000,  0.000,  1.371, -1.311,
                                1.000, -0.336, -0.698, -0.108,
                                1.000,  1.732,  0.000, -1.491,
                                0.000,  0.000,  0.000,  1.000 }};*/

/*static NXT_Matrix4f yuvrgb = {{ 1.000,  0.000,  1.402,-0.701,
                                1.000, -0.334, -0.714, 0.529,
                                1.000,  1.772,  0.000, -0.886,
                                0.000,  0.000,  0.000, 1.000 }};*/

static NXT_Matrix4f yuv_unbiased_rgb = NXT_Matrix4f( 
                                1.164,  0.000,  1.596,  0.000,
                                1.164, -0.392, -0.813,  0.000,
                                1.164,  2.017,  0.000,  0.000,
                                0.000,  0.000,  0.000,  1.000);

/*static NXT_Matrix4f yuv_unbiased_rgb = {{ 1.000,  0.000,  1.371,  0.000,
                                          1.000, -0.336, -0.698,  0.000,
                                          1.000,  1.732,  0.000,  0.000,
                                          0.000,  0.000,  0.000,  1.000 }};*/

/*static NXT_Matrix4f yuv_unbiased_rgb = {{ 1.000,  0.000,  1.402, 0.000,
                                          1.000, -0.334, -0.714, 0.000,
                                          1.000,  1.772,  0.000, 0.000,
                                          0.000,  0.000,  0.000, 1.000 }};*/


/*static NXT_Matrix4f rgbyuv = {{  0.299,  0.587,  0.114,  0.000,
                                -0.169, -0.331,  0.500,  0.000,
                                 0.500, -0.419, -0.081,  0.000,
                                 0.000,  0.000,  0.000,  1.000 }};*/

static NXT_Matrix4f rgbyuv = NXT_Matrix4f(
                                0.257,	0.504,	0.098,	0.000,
                                -0.148,	-0.291,	0.439,	0.000,
                                0.439,	-0.368,	-0.071,	0.000,
                                0.000,	0.000,	0.000,	1.000);

static NXT_Matrix4f rgb_biasedyuv = NXT_Matrix4f(  
                                        0.257,	0.504,	0.098,	0.0625,
                                        -0.148,	-0.291,	0.439,	0.500,
                                        0.439,	-0.368,	-0.071,	0.500,
                                        0.000,	0.000,	0.000,	1.000 );

static NXT_Matrix4f identity = NXT_Matrix4f(
                            1.000,	0.000,	0.000,	0.000,
							0.000,	1.000,	0.000,	0.000,
							0.000,	0.000,	1.000,	0.000,
							0.000,	0.000,	0.000,	1.000);

/*static NXT_Matrix4f rgb_biasedyuv = {{  0.300,  0.589,  0.111, -0.003,
                                       -0.169, -0.332,  0.502,  0.502,
                                        0.499, -0.420, -0.079,  0.502,
                                        0.000,  0.000,  0.000,  1.000 }};*/

/*static NXT_Matrix4f rgbyuv = {{ 0.300,  0.589,  0.111, -0.003,
    -0.169, -0.332,  0.502,  0.502,
    0.499, -0.420, -0.079,  0.502,
    0.000,  0.000,  0.000,  1.000 }};*/


NXT_Vector4f NXT_YUVtoRGB( NXT_Vector4f yuv ) {
    return NXT_Matrix4f_MultVector(yuvrgb,yuv);
}

NXT_Vector4f NXT_UnbiasedYUVtoRGB( NXT_Vector4f yuv ) {
    return NXT_Matrix4f_MultVector(yuv_unbiased_rgb,yuv);
}

NXT_Vector4f NXT_RGBtoUnbiasedYUV( NXT_Vector4f rgb ) {
    return NXT_Matrix4f_MultVector(rgbyuv,rgb);
}

NXT_Matrix4f NXT_YUV2RGBWithBrightnessContrast(
                                           float brightness,  // -1 ... +1
                                           float contrast)    // -1 ... +1
{
    NXT_Matrix4f brightnessAdjust;
    brightnessAdjust = NXT_Matrix4f_Translate(NXT_Vector4f(brightness, 0.0, 0.0, 0.0));
    
    NXT_Matrix4f contrastAdjust = NXT_Matrix4f_Translate(NXT_Vector4f(0.5, 0.5, 0.5, 0.0));
    contrastAdjust = NXT_Matrix4f_MultMatrix(NXT_Matrix4f_Scale( contrast+1, contrast+1, contrast+1),contrastAdjust);
    contrastAdjust = NXT_Matrix4f_MultMatrix(NXT_Matrix4f_Translate(NXT_Vector4f(-0.5, -0.5, -0.5, 0.0)), contrastAdjust);
    
    return NXT_Matrix4f_MultMatrix(contrastAdjust, NXT_Matrix4f_MultMatrix(brightnessAdjust, yuvrgb));
}

NXT_Matrix4f NXT_YUV2RGBWithColorAdjust(float brightness,  // -1 ... +1
                                        float contrast,    // -1 ... +1
                                        float saturation,
                                        NXT_Vector4f tintColor,
										unsigned int bJPEG_ColorConv)  // -1 ... +1
{
    // TODO
    NXT_Matrix4f unbiased_yuvToRGB;
    
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
    
	if( bJPEG_ColorConv ) {
		unbiased_yuvToRGB = NXT_Matrix4f(
			1.000,  0.000,  1.402, 0.000,
			1.000, -0.344, -0.714, 0.000,
			1.000,  1.772,  0.000, 0.000,
			0.000,  0.000,  0.000, 1.000
		);
	} else {
		unbiased_yuvToRGB = NXT_Matrix4f(
			1.164,  0.000,  1.596,  0.000,
			1.164, -0.392, -0.813,  0.000,
			1.164,  2.017,  0.000,  0.000,
			0.000,  0.000,  0.000,  1.000
		);
    }
	/*
	 
	 R = 1.164(Y - 16) + 1.596(V - 128)
	 G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
	 B = 1.164(Y - 16)                   + 2.018(U - 128)
	 
	 R = Y + 1.402*U
	 G = Y - 0.34414*V - 0.71414*U
	 B = Y + 1.772*V
	 */
	
    NXT_Matrix4f brightnessAdjust;
    brightnessAdjust = NXT_Matrix4f_Translate(NXT_Vector4f(brightness, 0.0, 0.0, 0.0));
    
    NXT_Matrix4f contrastAdjust = NXT_Matrix4f_Translate(NXT_Vector4f(0.5, 0.5, 0.5, 0.0));
    contrastAdjust = NXT_Matrix4f_MultMatrix(contrastAdjust,NXT_Matrix4f_Scale( contrast+1, contrast+1, contrast+1));
    contrastAdjust = NXT_Matrix4f_MultMatrix(contrastAdjust,NXT_Matrix4f_Translate(NXT_Vector4f(-0.5, -0.5, -0.5, 0.0)));
    
	NXT_Matrix4f bias;
	if( bJPEG_ColorConv ) {
		bias = NXT_Matrix4f_Translate(NXT_Vector4f(0.0, -0.5, -0.5, 0.0));
	} else {
		bias = NXT_Matrix4f_Translate(NXT_Vector4f(-0.0625, -0.5, -0.5, 0.0));
	}
    //NXT_Matrix4f satAdjust = NXT_Matrix4f_Translate((NXT_Vector4f){{ 0.0, -0.5, -0.5, 0.0 }});
    NXT_Matrix4f satAdjust = NXT_Matrix4f_Scale( 1, saturation+1, saturation+1);
    
    
    float r = tintColor.e[0];
    float g = tintColor.e[1];
    float b = tintColor.e[2];
    float total = r+g+b;
    const float r_weight = 0.241f;
    const float g_weight = 0.691f;
    const float b_weight = 0.068f;
    float rc = total==0?1:((r/total)*3.0);
    float gc = total==0?1:((g/total)*3.0);
    float bc = total==0?1:((b/total)*3.0);
    int i;
    
    NXT_Matrix4f tintAdjust;
    if( saturation < 0 ) {
        tintAdjust = NXT_Matrix4f(
                rc*r_weight, rc*g_weight, rc*b_weight, 0.0,             // Red      output
                gc*r_weight, gc*g_weight, gc*b_weight, 0.0,             // Green    output
                bc*r_weight, bc*g_weight, bc*b_weight, 0.0,             // Blue     output
                0.0,         0.0,         0.0,         1.0
                );
        
        NXT_Matrix4f identity = NXT_Matrix4f_Identity();
        for( i=0; i<16; i++ ) {
            tintAdjust.e[i] = (tintAdjust.e[i] * (1-(saturation+1))) + (identity.e[i] * (saturation+1));
        }
    } else {
        tintAdjust = NXT_Matrix4f_Identity();
    }

    
    
    
    NXT_Matrix4f sa_bias = NXT_Matrix4f_MultMatrix(satAdjust,bias);
    
    NXT_Matrix4f sa_bias_yuv = NXT_Matrix4f_MultMatrix(unbiased_yuvToRGB,sa_bias);
    
    return NXT_Matrix4f_MultMatrix(tintAdjust,NXT_Matrix4f_MultMatrix(NXT_Matrix4f_MultMatrix(sa_bias_yuv,brightnessAdjust),contrastAdjust));
}

NXT_Matrix4f NXT_ColorAdjustYUV(float brightness,  // -1 ... +1
                                float contrast,    // -1 ... +1
                                float saturation,
                                NXT_Vector4f tintColor,
								unsigned int bJPEG_ColorConv)  // -1 ... +1
{
	//    NXT_YUVtoRGB
    // NXT_RGBtoUnbiasedYUV
	
    return NXT_Matrix4f_MultMatrix(identity, NXT_YUV2RGBWithColorAdjust(brightness, contrast, saturation, tintColor,bJPEG_ColorConv));
}

NXT_Matrix4f NXT_ColorAdjustRGB(float brightness,  // -1 ... +1
                                float contrast,    // -1 ... +1
                                float saturation,
                                NXT_Vector4f tintColor)  // -1 ... +1
{

    return NXT_Matrix4f_MultMatrix(NXT_YUV2RGBWithColorAdjust(brightness, contrast, saturation, tintColor,0), rgb_biasedyuv);
    
}

NXT_Matrix4f NXT_RGBToYUVWithColorAdjust(float brightness,  // -1 ... +1
                                         float contrast,    // -1 ... +1
                                         float saturation,
                                         NXT_Vector4f tintColor)  // -1 ... +1
{
    NXT_Matrix4f unbiased_yuvToRGB = NXT_Matrix4f(
        1.164,  0.000,  1.596,  0.000,
        1.164, -0.392, -0.813,  0.000,
        1.164,  2.017,  0.000,  0.000,
        0.000,  0.000,  0.000,  1.000
        /*1.000,  0.000,  1.402, 0.000,
        1.000, -0.334, -0.714, 0.000,
        1.000,  1.772,  0.000, 0.000,
        0.000,  0.000,  0.000, 1.000*/
    );
    
    NXT_Matrix4f brightnessAdjust;
    brightnessAdjust = NXT_Matrix4f_Translate(NXT_Vector4f(brightness, 0.0, 0.0, 0.0));
    
    NXT_Matrix4f contrastAdjust = NXT_Matrix4f_Translate(NXT_Vector4f(0.5, 0.5, 0.5, 0.0));
    contrastAdjust = NXT_Matrix4f_MultMatrix(contrastAdjust,NXT_Matrix4f_Scale( contrast+1, contrast+1, contrast+1));
    contrastAdjust = NXT_Matrix4f_MultMatrix(contrastAdjust,NXT_Matrix4f_Translate(NXT_Vector4f(-0.5, -0.5, -0.5, 0.0)));
    
    //NXT_Matrix4f bias = NXT_Matrix4f_Translate((NXT_Vector4f){{ 0.0, -0.5, -0.5, 0.0 }});
    NXT_Matrix4f satAdjust = NXT_Matrix4f_Scale( 1, saturation+1, saturation+1);
    
    float r = tintColor.e[0];
    float g = tintColor.e[1];
    float b = tintColor.e[2];
    float total = r+g+b;
    const float r_weight = 0.241f;
    const float g_weight = 0.691f;
    const float b_weight = 0.068f;
    float rc = total==0?1:((r/total)*3.0);
    float gc = total==0?1:((g/total)*3.0);
    float bc = total==0?1:((b/total)*3.0);
    int i;
    
    NXT_Matrix4f tintAdjust;
    if( saturation < 0 ) {
        tintAdjust = NXT_Matrix4f(
                rc*r_weight, rc*g_weight, rc*b_weight, 0.0,             // Red      output
                gc*r_weight, gc*g_weight, gc*b_weight, 0.0,             // Green    output
                bc*r_weight, bc*g_weight, bc*b_weight, 0.0,             // Blue     output
                0.0,         0.0,         0.0,         1.0
                );
        
        NXT_Matrix4f identity = NXT_Matrix4f_Identity();
        for( i=0; i<16; i++ ) {
            tintAdjust.e[i] = (tintAdjust.e[i] * (1-(saturation+1))) + (identity.e[i] * (saturation+1));
        }
    } else {
        tintAdjust = NXT_Matrix4f_Identity();
    }
    
    //NXT_Matrix4f sa_bias = NXT_Matrix4f_MultMatrix(satAdjust,bias);
    NXT_Matrix4f sa_bias = NXT_Matrix4f_MultMatrix(satAdjust,rgbyuv);
    
    NXT_Matrix4f sa_bias_yuv = NXT_Matrix4f_MultMatrix(unbiased_yuvToRGB,sa_bias);
    
    return NXT_Matrix4f_MultMatrix(rgb_biasedyuv,NXT_Matrix4f_MultMatrix(tintAdjust,NXT_Matrix4f_MultMatrix(NXT_Matrix4f_MultMatrix(sa_bias_yuv,brightnessAdjust),contrastAdjust)));
}


NXT_Vector4f NXT_Matrix4f_MultVector( NXT_Matrix4f mat, NXT_Vector4f vec ) {
    float x = mat.e[0]*vec.e[0] + mat.e[1]*vec.e[1] + mat.e[ 2]*vec.e[2] + mat.e[3]*vec.e[3];
    float y = mat.e[4]*vec.e[0] + mat.e[5]*vec.e[1] + mat.e[ 6]*vec.e[2] + mat.e[7]*vec.e[3];
    float z = mat.e[8]*vec.e[0] + mat.e[9]*vec.e[1] + mat.e[10]*vec.e[2] + mat.e[11]*vec.e[3];
    float w = mat.e[12]*vec.e[0] + mat.e[13]*vec.e[1] + mat.e[14]*vec.e[2] + mat.e[15]*vec.e[3];
    
    return NXT_Vector4f(x,y,z,w);
}

NXT_Matrix4f NXT_Matrix4f_Ortho( float left, float right, float bottom, float top, float near, float far ) {
	float r_l = right - left;
	float t_b = top - bottom;
	float f_n = far - near;
	float tx = - (right + left) / (right - left);
	float ty = - (top + bottom) / (top - bottom);
	float tz = - (far + near) / (far - near);
    
    return NXT_Matrix4f(
            2.0f/r_l,   0.0,        0.0,         tx,
            0.0,        2.0f/t_b,   0.0,         ty,
            0.0,        0.0,        -2.0f/f_n,   tz,
            0.0,        0.0,        0.0,         1.0
        );
}

NXT_Matrix4f NXT_Matrix4f_MultMatrix( NXT_Matrix4f mat_a, NXT_Matrix4f mat_b ) {
    float *a = mat_b.e;
    float *b = mat_a.e;
    return NXT_Matrix4f(
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
        );
}

NXT_Matrix4f NXT_Matrix4f_Frustum(double left, double right, double bottom, double top, double near, double far) {
    
    float   a = (right+left)/(right-left);
    float   b = (top+bottom)/(top-bottom);
    float   c = (far+near)/(far-near);
    float   d = (2*far*near)/(far-near);
    
    return NXT_Matrix4f(
            (2*near)/(right-left),  0.0,                    0.0,    0.0,
            0.0,                    (2*near)/(top-bottom),  0.0,    0.0,
            a,                       b,                     c,      d,
            0.0,                    0.0,                    -1,     0.0
        );
}

NXT_Vector4f NXT_Vector4f_Normalize( NXT_Vector4f v ) {

	float vlen = sqrtf(v.e[0] * v.e[0] + v.e[1] * v.e[1] + v.e[2] * v.e[2]);
    
    return NXT_Vector4f(
    	v.e[0] / vlen,
    	v.e[1] / vlen,
    	v.e[2] / vlen,
        0.0f
    );

}

NXT_Vector4f NXT_Vector4f_CrossProduct( NXT_Vector4f p, NXT_Vector4f q ) {
	float Px = p.e[0];
	float Py = p.e[1];
	float Pz = p.e[2];
	float Qx = q.e[0];
	float Qy = q.e[1];
	float Qz = q.e[2];
	return NXT_Vector4f(
    	Py*Qz - Pz*Qy,
        Pz*Qx - Px*Qz,
        Px*Qy - Py*Qx,
        0.0f
    );
//	return (NXT_Vector4f){{
//    	u.e[1]*v.e[2] - u.e[2]*v.e[1],
//        u.e[2]*v.e[0] - u.e[0]*v.e[2],
//        u.e[0]*v.e[1] - u.e[1]*v.e[0]
//    }};
}

unsigned int NXT_Matrix4f_Compare(NXT_Matrix4f mat1, NXT_Matrix4f mat2) {
    return(mat1.e[0]==mat2.e[0] &&
           mat1.e[1]==mat2.e[1] &&
           mat1.e[2]==mat2.e[2] &&
           mat1.e[3]==mat2.e[3] &&
           mat1.e[4]==mat2.e[4] &&
           mat1.e[5]==mat2.e[5] &&
           mat1.e[6]==mat2.e[6] &&
           mat1.e[7]==mat2.e[7] &&
           mat1.e[8]==mat2.e[8] &&
           mat1.e[9]==mat2.e[9] &&
           mat1.e[10]==mat2.e[10] &&
           mat1.e[11]==mat2.e[11] &&
           mat1.e[12]==mat2.e[12] &&
           mat1.e[13]==mat2.e[13] &&
           mat1.e[14]==mat2.e[14] &&
           mat1.e[15]==mat2.e[15]);
}

unsigned int NXT_Vector4f_Compare(NXT_Vector4f vec1, NXT_Vector4f vec2) {
    return(vec1.e[0]==vec2.e[0] &&
           vec1.e[1]==vec2.e[1] &&
           vec1.e[2]==vec2.e[2] &&
           vec1.e[3]==vec2.e[3]);
}

static inline unsigned int ftoi255( float f ) {
    if( f<0.0 )
        return 0;
    if( f>1.0 )
        return 255;
    return (int)(f*255.0);
}

unsigned int NXT_Vector4f_ToARGBIntColor( NXT_Vector4f v ) {
    return (ftoi255(v.e[3])<<24) | (ftoi255(v.e[0])<<16) | (ftoi255(v.e[1])<<8) | ftoi255(v.e[2]);
}

void NXT_pMatrix4f_ScaleX( NXT_Matrix4f *pMatrix, float xscale ) {
    *pMatrix = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Scale(xscale,1,1), *pMatrix);
}

void NXT_pMatrix4f_ScaleY( NXT_Matrix4f *pMatrix, float yscale ) {
    *pMatrix = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Scale(1,yscale,1), *pMatrix);
}

void NXT_pMatrix4f_ScaleZ( NXT_Matrix4f *pMatrix, float zscale ) {
    *pMatrix = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Scale(1,1,zscale), *pMatrix);
}

void NXT_pMatrix4f_TranslateX( NXT_Matrix4f *pMatrix, float xoffs ) {
    *pMatrix = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Translate(NXT_Vector4f(
        xoffs, 0, 0, 0
    )), *pMatrix);
}

void NXT_pMatrix4f_TranslateY( NXT_Matrix4f *pMatrix, float yoffs ) {
    *pMatrix = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Translate(NXT_Vector4f(
        0, yoffs, 0, 0
    )), *pMatrix);
}

void NXT_pMatrix4f_TranslateZ( NXT_Matrix4f *pMatrix, float zoffs ) {
    *pMatrix = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Translate(NXT_Vector4f(
        0, 0, zoffs, 0
    )), *pMatrix);
}

/********** Dual-Phase Linear Congruential Random Number Generator *************
 *
 * This is based on public-domain code from www.eternallyconfuzzled.com.
 * (According to notices on that site, all code there is public domain
 * unless otherwise noted).  The specific URL for this code is:
 *
 *  http://www.eternallyconfuzzled.com/tuts/algorithms/jsw_tut_rand.aspx
 *
 * Modified 2012-03-27 by Matthew Feinberg to store the seed in a non-static
 * way (so this can be used as a thread-safe random number generator).
 *
 *******************************************************************************/

#define M1 2147483647
#define M2 2147483399
#define A1 40015
#define A2 40692
#define Q1 ( M1 / A1 )
#define Q2 ( M2 / A2 )
#define R1 ( M1 % A1 )
#define R2 ( M2 % A2 )

void NXT_SRandom( NXT_RandSeed* randSeed, int seed1, int seed2 )
{
    randSeed->seed1 = seed1;
    randSeed->seed2 = seed2;
}

float NXT_FRandom( NXT_RandSeed* randSeed ) {
    int result = NXT_Random(randSeed);
    if( result < 0 )
        result = -result;
    result %= 0x7FFFFFFF;
    return (float)result / (float)0x7FFFFFFF;
}


int NXT_Random( NXT_RandSeed* randSeed )
{
    int result;
    
    randSeed->seed1 = A1 * ( randSeed->seed1 % Q1 ) - R1 * ( randSeed->seed1 / Q1 );
    randSeed->seed2 = A2 * ( randSeed->seed2 % Q2 ) - R2 * ( randSeed->seed2 / Q2 );
    
    if ( randSeed->seed1 <= 0 )
        randSeed->seed1 += M1;
    
    if ( randSeed->seed2 <= 0 )
        randSeed->seed2 += M2;
    
    result = randSeed->seed1 - randSeed->seed2;
    
    if ( result < 1 )
        result += M1 - 1;
    
    return result;
}



