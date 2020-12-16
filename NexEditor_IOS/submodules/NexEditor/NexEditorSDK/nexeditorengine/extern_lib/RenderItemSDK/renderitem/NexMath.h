#ifndef NexMath_h
#define NexMath_h

typedef struct RITM_NXT_Vector4f_ {
    float       e[4];
} RITM_NXT_Vector4f;

typedef struct RITM_NXT_Matrix4f_ {
    float       e[16];
} RITM_NXT_Matrix4f;

float RITM_NXT_Vector4f_Length( RITM_NXT_Vector4f v );
RITM_NXT_Matrix4f RITM_NXT_Matrix4f_Identity( void );
RITM_NXT_Matrix4f RITM_NXT_Matrix4f_Scale( float xscale, float yscale, float zscale );
RITM_NXT_Matrix4f RITM_NXT_Matrix4f_Rotate( RITM_NXT_Vector4f axis, float angle_radians );
RITM_NXT_Matrix4f RITM_NXT_Matrix4f_Translate( RITM_NXT_Vector4f v );
RITM_NXT_Matrix4f RITM_NXT_Matrix4f_Perspective( float fov_radians, float aspect, float z_near, float z_far );
RITM_NXT_Matrix4f RITM_NXT_Matrix4f_Ortho( float left, float right, float bottom, float top, float near, float far );
RITM_NXT_Matrix4f RITM_NXT_Matrix4f_MultMatrix( RITM_NXT_Matrix4f mat_a, RITM_NXT_Matrix4f mat_b );
RITM_NXT_Vector4f RITM_NXT_Matrix4f_MultVector( RITM_NXT_Matrix4f mat, RITM_NXT_Vector4f vec );
unsigned int RITM_NXT_NextHighestPowerOfTwo( unsigned int v );
RITM_NXT_Matrix4f RITM_NXT_Matrix4f_Frustum(double left, double right, double bottom, double top, double near, double far);
RITM_NXT_Vector4f RITM_NXT_Vector4f_Normalize( RITM_NXT_Vector4f v );
unsigned int RITM_NXT_Matrix4f_Compare(RITM_NXT_Matrix4f mat1, RITM_NXT_Matrix4f mat2);
RITM_NXT_Vector4f RITM_NXT_Vector4f_CrossProduct( RITM_NXT_Vector4f u, RITM_NXT_Vector4f v );
unsigned int RITM_NXT_Vector4f_Compare(RITM_NXT_Vector4f vec1, RITM_NXT_Vector4f vec2);
unsigned int RITM_NXT_Vector4f_ToARGBIntColor( RITM_NXT_Vector4f v );
unsigned int RITM_NXT_Vector4f_ToARGBIntColor255( RITM_NXT_Vector4f v );
RITM_NXT_Vector4f RITM_NXT_YUVtoRGB( RITM_NXT_Vector4f yuv );
RITM_NXT_Vector4f RITM_NXT_RGBtoUnbiasedYUV( RITM_NXT_Vector4f rgb );
RITM_NXT_Vector4f RITM_NXT_UnbiasedYUVtoRGB( RITM_NXT_Vector4f yuv );

void RITM_NXT_pMatrix4f_ScaleX( RITM_NXT_Matrix4f *pMatrix, float xscale );
void RITM_NXT_pMatrix4f_ScaleY( RITM_NXT_Matrix4f *pMatrix, float yscale );
void RITM_NXT_pMatrix4f_ScaleZ( RITM_NXT_Matrix4f *pMatrix, float zscale );
void RITM_NXT_pMatrix4f_TranslateX( RITM_NXT_Matrix4f *pMatrix, float xoffs );
void RITM_NXT_pMatrix4f_TranslateY( RITM_NXT_Matrix4f *pMatrix, float yoffs );
void RITM_NXT_pMatrix4f_TranslateZ( RITM_NXT_Matrix4f *pMatrix, float zoffs );

typedef struct RITM_NXT_RandSeed_ {
    int seed1;
    int seed2;
} RITM_NXT_RandSeed;

float RITM_NXT_FRandom( RITM_NXT_RandSeed* randSeed );
void RITM_NXT_SRandom( RITM_NXT_RandSeed* randSeed, int seed1, int seed2 );
int RITM_NXT_Random( RITM_NXT_RandSeed* randSeed );

RITM_NXT_Matrix4f RITM_NXT_YUV2RGBWithColorAdjust(float brightness,  // -1 ... +1
                                        float contrast,    // -1 ... +1
                                        float saturation,
                                        RITM_NXT_Vector4f tintColor,  // -1 ... +1
										unsigned int bJPEG_ColorConv);

RITM_NXT_Matrix4f RITM_NXT_ColorAdjustRGB(float brightness,  // -1 ... +1
                                float contrast,    // -1 ... +1
                                float saturation,
                                RITM_NXT_Vector4f tintColor);  // -1 ... +1
	
RITM_NXT_Matrix4f RITM_NXT_ColorAdjustYUV(float brightness,  // -1 ... +1
                                float contrast,    // -1 ... +1
                                float saturation,
                                RITM_NXT_Vector4f tintColor,  // -1 ... +1
								unsigned int bJPEG_ColorConv);

RITM_NXT_Matrix4f RITM_NXT_RGBToYUVWithColorAdjust(float brightness,  // -1 ... +1
                                         float contrast,    // -1 ... +1
                                         float saturation,
                                         RITM_NXT_Vector4f tintColor);  // -1 ... +1

RITM_NXT_Matrix4f RITM_NXT_Matrix4f_Transpose( RITM_NXT_Matrix4f m );

float RITM_NXT_CubicBezierAtTime(float t, float p1x, float p1y, float p2x, float p2y, float duration);


#endif
