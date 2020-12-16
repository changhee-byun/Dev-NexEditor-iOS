#ifndef NexTheme_Math_h
#define NexTheme_Math_h


struct NXT_Vector4f {
    float       e[4];

    NXT_Vector4f(){

        e[0] = 0.0f;
        e[1] = 0.0f;
        e[2] = 0.0f;
        e[3] = 0.0f;
    }

    NXT_Vector4f(float* val){

        e[0] = val[0];
        e[1] = val[1];
        e[2] = val[2];
        e[3] = val[3];
    }
    NXT_Vector4f(float x, float y, float z, float w){

        e[0] = x;
        e[1] = y;
        e[2] = z;
        e[3] = w;
    }
};

struct NXT_Matrix4f{
    float       e[16];
    NXT_Matrix4f(){

        e[0] = 0.0f;
        e[1] = 0.0f;
        e[2] = 0.0f;
        e[3] = 0.0f;
        e[4] = 0.0f;
        e[5] = 0.0f;
        e[6] = 0.0f;
        e[7] = 0.0f;
        e[8] = 0.0f;
        e[9] = 0.0f;
        e[10] = 0.0f;
        e[11] = 0.0f;
        e[12] = 0.0f;
        e[13] = 0.0f;
        e[14] = 0.0f;
        e[15] = 0.0f;
    }
    NXT_Matrix4f(
                    float e00, float e01, float e02, float e03,
                    float e10, float e11, float e12, float e13,
                    float e20, float e21, float e22, float e23,
                    float e30, float e31, float e32, float e33){

        e[ 0] = e00;
        e[ 1] = e01;
        e[ 2] = e02;
        e[ 3] = e03;
        e[ 4] = e10;
        e[ 5] = e11;
        e[ 6] = e12;
        e[ 7] = e13;
        e[ 8] = e20;
        e[ 9] = e21;
        e[10] = e22;
        e[11] = e23;
        e[12] = e30;
        e[13] = e31;
        e[14] = e32;
        e[15] = e33;
    }
};

float NXT_Vector4f_Length( NXT_Vector4f v );
NXT_Matrix4f NXT_Matrix4f_Identity( void );
NXT_Matrix4f NXT_Matrix4f_Scale( float xscale, float yscale, float zscale );
NXT_Matrix4f NXT_Matrix4f_Rotate( NXT_Vector4f axis, float angle_radians );
NXT_Matrix4f NXT_Matrix4f_Translate( NXT_Vector4f v );
NXT_Matrix4f NXT_Matrix4f_Perspective( float fov_radians, float aspect, float z_near, float z_far );
NXT_Matrix4f NXT_Matrix4f_Ortho( float left, float right, float bottom, float top, float near, float far );
NXT_Matrix4f NXT_Matrix4f_MultMatrix( NXT_Matrix4f mat_a, NXT_Matrix4f mat_b );
NXT_Vector4f NXT_Matrix4f_MultVector( NXT_Matrix4f mat, NXT_Vector4f vec );
unsigned int NXT_NextHighestPowerOfTwo( unsigned int v );
NXT_Matrix4f NXT_Matrix4f_Frustum(double left, double right, double bottom, double top, double near, double far);
NXT_Vector4f NXT_Vector4f_Normalize( NXT_Vector4f v );
unsigned int NXT_Matrix4f_Compare(NXT_Matrix4f mat1, NXT_Matrix4f mat2);
NXT_Vector4f NXT_Vector4f_CrossProduct( NXT_Vector4f u, NXT_Vector4f v );
unsigned int NXT_Vector4f_Compare(NXT_Vector4f vec1, NXT_Vector4f vec2);
unsigned int NXT_Vector4f_ToARGBIntColor( NXT_Vector4f v );
NXT_Vector4f NXT_YUVtoRGB( NXT_Vector4f yuv );
NXT_Vector4f NXT_RGBtoUnbiasedYUV( NXT_Vector4f rgb );
NXT_Vector4f NXT_UnbiasedYUVtoRGB( NXT_Vector4f yuv );

void NXT_pMatrix4f_ScaleX( NXT_Matrix4f *pMatrix, float xscale );
void NXT_pMatrix4f_ScaleY( NXT_Matrix4f *pMatrix, float yscale );
void NXT_pMatrix4f_ScaleZ( NXT_Matrix4f *pMatrix, float zscale );
void NXT_pMatrix4f_TranslateX( NXT_Matrix4f *pMatrix, float xoffs );
void NXT_pMatrix4f_TranslateY( NXT_Matrix4f *pMatrix, float yoffs );
void NXT_pMatrix4f_TranslateZ( NXT_Matrix4f *pMatrix, float zoffs );

typedef struct NXT_RandSeed_ {
    int seed1;
    int seed2;
    NXT_RandSeed_():seed1(0), seed2(0){

    }
} NXT_RandSeed;

float NXT_FRandom( NXT_RandSeed* randSeed );
void NXT_SRandom( NXT_RandSeed* randSeed, int seed1, int seed2 );
int NXT_Random( NXT_RandSeed* randSeed );

NXT_Matrix4f NXT_YUV2RGBWithColorAdjust(float brightness,  // -1 ... +1
                                        float contrast,    // -1 ... +1
                                        float saturation,
                                        NXT_Vector4f tintColor,  // -1 ... +1
										unsigned int bJPEG_ColorConv);

NXT_Matrix4f NXT_ColorAdjustRGB(float brightness,  // -1 ... +1
                                float contrast,    // -1 ... +1
                                float saturation,
                                NXT_Vector4f tintColor);  // -1 ... +1
	
NXT_Matrix4f NXT_ColorAdjustYUV(float brightness,  // -1 ... +1
                                float contrast,    // -1 ... +1
                                float saturation,
                                NXT_Vector4f tintColor,  // -1 ... +1
								unsigned int bJPEG_ColorConv);

NXT_Matrix4f NXT_RGBToYUVWithColorAdjust(float brightness,  // -1 ... +1
                                         float contrast,    // -1 ... +1
                                         float saturation,
                                         NXT_Vector4f tintColor);  // -1 ... +1

    NXT_Matrix4f NXT_Matrix4f_Transpose( NXT_Matrix4f m );



#endif
