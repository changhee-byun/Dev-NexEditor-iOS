#ifndef _NXCC_API_H_
#define _NXCC_API_H_

///////////////////////////////////////////////////////////////////////////////
// Macro
///////////////////////////////////////////////////////////////////////////////
#if defined(__arm) && defined(_ANDROID)

#define CLIP_BGR16				\
	__asm						\
	{							\
		ADD		R, Y, CR;		\
		CMP		R, 0xff;		\
		MVNHI	R, R, ASR 16;	\
		AND		R, R, 0xf8;		\
		ADD		G, Y, CG;		\
		CMP		G, 0xff;		\
		MVNHI	G, G, ASR 16;	\
		AND		G, G, 0xfc;		\
		ORR		R, G, R, LSL 5;	\
		ADD		G, Y, CB;		\
		CMP		G, 0xff;		\
		MVNHI	G, G, ASR 16;	\
		MOV		R, R, LSL 3;	\
		ORR		R, R, G, LSR 3;	\
	}

#else // #ifdef __arm

#define CONV_RGB16(R, G, B) (unsigned int) ((R & 0xf8) << 8 | (G & 0xfc) << 3 | (B >> 3))		// RGB565 mode
//#define CONV_RGB32(R, G, B) (unsigned int) (G << 24 | R << 16 | alpha<<8 | B)					// RGB888 mode
#define CONV_RGB32(R, G, B) (unsigned int) (R << 16 | G << 8 | B<<0 )							// RGB888 mode
//#define CONV_RGB16(B, G, R) (unsigned int) ((R & 0xf8) << 8 | (G & 0xfc) << 3 | (B >> 3))		// RGB565 mode
//#define CONV_RGB16(R, G, B) (unsigned int) ((R & 0xf8) << 7 | (G & 0xf8) << 2 | (B >> 3))		// RGB555 mode

/*#define CLIP_255(a,x)\
	a = x;\
	if (a > 255)\
		a = 255;\
	else if (a < 0)\
		a = 0;
*/

#define CLIP_255(a,x)\
	a = x;\
	if (a & 0xFFFFFF00 )\
	{\
		if (a & 0x80000000)\
			a ^= a;\
		else\
			a = 255;\
	}
#endif // __arm

extern "C" void NXCC_YUV420ToRGB565_EX( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, 
						unsigned int nSrcPitch, unsigned int nDstPitch );

extern "C" void NXCC_YUV420ToRGB888_EX( NXUINT32 *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, 
						unsigned int nSrcPitch, unsigned int nDstPitch );

extern "C" void NXCC_YUV420ToRGB888_EX_ROT180( NXUINT32 *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, 
						unsigned int nSrcPitch, unsigned int nDstPitch );

extern "C" void NXCC_YUV420ToRGB16_EX_ROT270( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, unsigned int pitch );

extern "C" void NXCC_YUV420ToRGB16_EX_DN2X2( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, unsigned int pitch );

#endif // _NXCC_API_H_
