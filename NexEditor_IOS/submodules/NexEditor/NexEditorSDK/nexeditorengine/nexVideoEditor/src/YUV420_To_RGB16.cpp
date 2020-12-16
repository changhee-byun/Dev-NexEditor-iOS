/*****************************************************************************************
*
*       File Name   :	YUV420_To_RGB16.c
*		Module		:	Nextreaming YUV420 to RGB16 color conversion
*       Description :	
*
******************************************************************************************
                         Nextreaming Confidential Proprietary
                     Copyright (C) 2002 Nextreaming Corporation
                 All rights are reserved by Nextreaming Corporation

Revision History:
Author              Date			Version		Description of Changes
------------------------------------------------------------------------------------------
Video team		2006/08/06			1.0.0		Draft
----------------------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
// Configuration
///////////////////////////////////////////////////////////////////////////////
#include "NexSAL_Com.h"
#include "yuv420_to_rgb16_ex.h"

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

// Output display size is constant
#define DISPLAY_WIDTH			320//240
#define DISPLAY_HEIGHT			240//320
#define DISPLAY_PITCH			DISPLAY_WIDTH
#define DISPLAY_OFFSET_DN2		( DISPLAY_PITCH - ( DISPLAY_WIDTH >> 1 ) )
#define DISPLAY_OFFSET2			( ( DISPLAY_PITCH << 1 ) - DISPLAY_WIDTH )

#define DISPLAY_WIDTH_ROT		320
#define DISPLAY_HEIGHT_ROT		240
#define DISPLAY_PITCH_ROT		DISPLAY_HEIGHT_ROT

//#define BT709

///////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////

//
// NXCC_YUV420ToRGB16_EX
//
// Assume :
//			- Output display size is constant
//			- Input size is even, value range is video range
//			- Input format is YUV planar, Luma and chResultoma pitch are same
//			- BT.709 conversion
//			- Output RGB format is BGR565
//
#if defined(__arm) && defined(_ANDROID)

void NXCC_YUV420ToRGB16_EX( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, 
						unsigned int pitch, unsigned int dstPitch )
{
	int x, pitch2, pitch_org;

//	width = ( width > DISPLAY_WIDTH ) ? DISPLAY_WIDTH : width;
//	height = ( height > DISPLAY_HEIGHT ) ? DISPLAY_HEIGHT : height;
	pitch2 = ( pitch << 1 ) - width;
	pitch_org = pitch;
	pitch -= width;

	for( ; height; height -= 2 ) 	
	{
		for( x = 0; x < width; x += 2 ) 
		{
			int CR,CB,CG;
			int	Y;
			unsigned int R, G;
			{
#ifdef __TARGET_FEATURE_DSPMUL

				int PCr, PCb;
				PCr = ((int)(*VP++) - 128);
				PCb = ((int)(*UP++) - 128);

				CR = (401 * PCr) >> 8;
				CG =  - (120 * PCr + 48 * PCb) >> 8;
				CB = (PCr + 476 * PCb) >> 8;

#else // #ifdef __TARGET_FEATURE_DSPMUL

				CR = ((int)(*VP++) - 128);
				CB = ((int)(*UP++) - 128);

				__asm
				{
					// CG
					SUB	R, CR, CR, LSL 4;
					SUB R, R, CB, LSL 1;
					SUB R, R, CB, LSL 2;
					MOV CG, R, ASR 5;

					// CB
					RSB R, CB, CB, LSL 3;
					ADD R, R, R, LSL 4;
					ADD R, CR, R, LSL 2;
					MOV CB, R, ASR 8

					// CR
					ADD R, CR, CR, LSL 3;
					ADD R, CR, R, LSL 4;
					ADD CR, CR, R, ASR 8;
				}
#endif // #ifdef __TARGET_FEATURE_DSPMUL
			}

			Y = *YP;
			CLIP_BGR16;
			*pDest = R;

			Y = YP[pitch_org];
			CLIP_BGR16;
			pDest[DISPLAY_PITCH] = R;
			YP++;
			pDest++;

			Y = *YP;
			CLIP_BGR16;
			*pDest = R;

			Y = YP[pitch_org];
			CLIP_BGR16;
			pDest[DISPLAY_PITCH] = R;
			YP++;
			pDest++;
		}

		YP += pitch2;
		UP += pitch >> 1;
		VP += pitch >> 1;

		//pDest += DISPLAY_OFFSET2;
		pDest += dstPitch;
	}
}

#elif defined(__IWMMXT__)

void NXCC_YUV420ToRGB16_EX( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, unsigned int pitch )
{
	int x, pitch_y, pitch_uv;
	unsigned char *YP_pitch = YP + pitch;
	unsigned short *pDest_pitch = pDest + DISPLAY_PITCH;
	int display_pitch = (DISPLAY_PITCH << 1) - width;

	width = ( width > DISPLAY_WIDTH ) ? DISPLAY_WIDTH : width;
	height = ( height > DISPLAY_HEIGHT ) ? DISPLAY_HEIGHT : height;
	pitch_y = ( pitch << 1 ) - width;
	pitch_uv = (pitch - width) >> 1;

	asm volatile(
		"tbcsth		wR10, %0		\n\t"
		"tbcsth		wR11, %1		\n\t"
		"tbcsth		wR12, %2		\n\t"
		"tbcsth		wR13, %3		\n\t"
		"tbcsth		wR14, %4		\n\t"
		"tbcsth		wR15, %5		\n\t"
		:
		: "r" (145), "r" (-120), "r" (-48), "r" (220), "r" (8), "r" (128)
		);
	asm volatile(
		"tmcr	wCGR0, %0		\n\t"
		"tmcr	wCGR1, %1		\n\t"
		"tmcr	wCGR2, %2		\n\t"
		"tmcr	wCGR3, %3		\n\t"
		:
		: "r" (2), "r" (3), "r" (5), "r" (11)
		);
	for( ; height; height -= 2 )
	{
		for( x = 0; x < width; x += 8 )
		{
			asm volatile(
				"wldrw	wR0, [%0]		\n\t"	// u3 u2 u1 u0
				"wldrw	wR1, [%1]		\n\t"	// v3 v2 v1 v0

				"wunpckelub	wR0, wR0	\n\t"	// PCb
				"wunpckelub	wR1, wR1	\n\t"	// PCr

				"wsubh		wR0, wR0, wR15	\n\t"	// PCb - 128
				"wsubh		wR1, wR1, wR15	\n\t"	// PCr - 128

				// wR2 = CR, wR3 = CG, wR4 = CB
				"wmulsl		wR2, wR1, wR10	\n\t"	// 145 * PCr
				"wsrah		wR2, wR2, wR14	\n\t"	// (145 * PCr) >> 8
				"waddh		wR2, wR2, wR1	\n\t"	// PCr + (145 * PCr) >> 8

				"wmulsl		wR3, wR1, wR11	\n\t"	// -120 * PCr
				"wmulsl		wR4, wR0, wR12	\n\t"	// -48 * PCb
				"waddh		wR3, wR3, wR4	\n\t"	// -(120 * PCr + 48 * pCb)
				"wsrah		wR3, wR3, wR14	\n\t"	// -(120 * PCr + 48 * pCb) >> 8

				"wmulsl		wR4, wR0, wR13	\n\t"	// 220 * PCb
				"waddh		wR4, wR4, wR1	\n\t"	// PCr + 220 * PCb
				"wsrah		wR4, wR4, wR14	\n\t"	// (PCr + 220 * PCb) >> 8
				"waddh		wR4, wR4, wR0	\n\t"	// (PCr + 220 * PCb) >> 8 + PCb
				:
				: "r" (UP), "r" (VP)
				);
			UP += 4;
			VP += 4;

			asm volatile(
				"wldrd		wR0, [%0]		\n\t"	// y7 y6 y5 y4 y3 y2 y1 y0
				"wsrlh		wR1, wR0, wR14	\n\t"	// y7 y5 y3 y1
				"wsllh		wR0, wR0, wR14	\n\t"	// y6 0 y4 0 y2 0 y0 0
				"wsrlh		wR0, wR0, wR14	\n\t"	// y6 y4 y2 y0

				"waddh		wR7, wR0, wR2	\n\t"	// Y + CR
				"waddh		wR8, wR0, wR3	\n\t"	// Y + CG
				"waddh		wR9, wR0, wR4	\n\t"	// Y + CB
				"wpackhus	wR7, wR7, wR7	\n\t"
				"wpackhus	wR8, wR8, wR8	\n\t"
				"wpackhus	wR9, wR9, wR9	\n\t"
				"wunpckelub	wR7, wR7		\n\t"
				"wunpckelub	wR8, wR8		\n\t"
				"wunpckelub	wR9, wR9		\n\t"

				"wsrlhg		wR7, wR7, wCGR1	\n\t"	// R >> 3
				"wsrlhg		wR8, wR8, wCGR0	\n\t"	// G >> 2
				"wsrlhg		wR9, wR9, wCGR1	\n\t"	// B >> 3
				"wsllhg		wR7, wR7, wCGR3	\n\t"	// R << 11
				"wsllhg		wR8, wR8, wCGR2	\n\t"	// G << 5
				"wor		wR9, wR9, wR8	\n\t"	// (G << 5) | B
				"wor		wR9, wR9, wR7	\n\t"	// (R << 11) | (G << 5) | B

				"waddh		wR0, wR1, wR2	\n\t"	// Y + CR
				"waddh		wR7, wR1, wR3	\n\t"	// Y + CG
				"waddh		wR8, wR1, wR4	\n\t"	// Y + CB
				"wpackhus	wR0, wR0, wR0	\n\t"
				"wpackhus	wR7, wR7, wR7	\n\t"
				"wpackhus	wR8, wR8, wR8	\n\t"
				"wunpckelub	wR0, wR0		\n\t"
				"wunpckelub	wR7, wR7		\n\t"
				"wunpckelub	wR8, wR8		\n\t"

				"wsrlhg		wR0, wR0, wCGR1	\n\t"	// R >> 3
				"wsrlhg		wR7, wR7, wCGR0	\n\t"	// G >> 2
				"wsrlhg		wR8, wR8, wCGR1	\n\t"	// B >> 3
				"wsllhg		wR0, wR0, wCGR3	\n\t"	// R << 11
				"wsllhg		wR7, wR7, wCGR2	\n\t"	// G << 5
				"wor		wR8, wR8, wR7	\n\t"	// (G << 5) | B
				"wor		wR8, wR8, wR0	\n\t"	// (R << 11) | (G << 5) | B

				"wunpckilh	wR0, wR9, wR8	\n\t"	// RGB3 RGB2 RGB1 RGB0
				"wunpckihh	wR1, wR9, wR8	\n\t"	// RGB7 RGB6 RGB5 RGB4
				"wstrd		wR0, [%1]		\n\t"
				"wstrd		wR1, [%1, #8]	\n\t"
				:
				: "r" (YP), "r" (pDest)
				);
			YP += 8;
			pDest += 8;

			asm volatile(
				"wldrd		wR0, [%0]		\n\t"	// y7 y6 y5 y4 y3 y2 y1 y0
				"wsrlh		wR1, wR0, wR14	\n\t"	// y7 y5 y3 y1
				"wsllh		wR0, wR0, wR14	\n\t"	// y6 0 y4 0 y2 0 y0 0
				"wsrlh		wR0, wR0, wR14	\n\t"	// y6 y4 y2 y0

				"waddh		wR7, wR0, wR2	\n\t"	// Y + CR
				"waddh		wR8, wR0, wR3	\n\t"	// Y + CG
				"waddh		wR9, wR0, wR4	\n\t"	// Y + CB
				"wpackhus	wR7, wR7, wR7	\n\t"
				"wpackhus	wR8, wR8, wR8	\n\t"
				"wpackhus	wR9, wR9, wR9	\n\t"
				"wunpckelub	wR7, wR7		\n\t"
				"wunpckelub	wR8, wR8		\n\t"
				"wunpckelub	wR9, wR9		\n\t"

				"wsrlhg		wR7, wR7, wCGR1	\n\t"	// R >> 3
				"wsrlhg		wR8, wR8, wCGR0	\n\t"	// G >> 2
				"wsrlhg		wR9, wR9, wCGR1	\n\t"	// B >> 3
				"wsllhg		wR7, wR7, wCGR3	\n\t"	// R << 11
				"wsllhg		wR8, wR8, wCGR2	\n\t"	// G << 5
				"wor		wR9, wR9, wR8	\n\t"	// (G << 5) | B
				"wor		wR9, wR9, wR7	\n\t"	// (R << 11) | (G << 5) | B

				"waddh		wR0, wR1, wR2	\n\t"	// Y + CR
				"waddh		wR7, wR1, wR3	\n\t"	// Y + CG
				"waddh		wR8, wR1, wR4	\n\t"	// Y + CB
				"wpackhus	wR0, wR0, wR0	\n\t"
				"wpackhus	wR7, wR7, wR7	\n\t"
				"wpackhus	wR8, wR8, wR8	\n\t"
				"wunpckelub	wR0, wR0		\n\t"
				"wunpckelub	wR7, wR7		\n\t"
				"wunpckelub	wR8, wR8		\n\t"

				"wsrlhg		wR0, wR0, wCGR1	\n\t"	// R >> 3
				"wsrlhg		wR7, wR7, wCGR0	\n\t"	// G >> 2
				"wsrlhg		wR8, wR8, wCGR1	\n\t"	// B >> 3
				"wsllhg		wR0, wR0, wCGR3	\n\t"	// R << 11
				"wsllhg		wR7, wR7, wCGR2	\n\t"	// G << 5
				"wor		wR8, wR8, wR7	\n\t"	// (G << 5) | B
				"wor		wR8, wR8, wR0	\n\t"	// (R << 11) | (G << 5) | B

				"wunpckilh	wR0, wR9, wR8	\n\t"	// RGB3 RGB2 RGB1 RGB0
				"wunpckihh	wR1, wR9, wR8	\n\t"	// RGB7 RGB6 RGB5 RGB4
				"wstrd		wR0, [%1]		\n\t"
				"wstrd		wR1, [%1, #8]	\n\t"
				:
				: "r" (YP_pitch), "r" (pDest_pitch)
				);
			YP_pitch += 8;
			pDest_pitch += 8;
		}

		YP += pitch_y;
		YP_pitch += pitch_y;
		UP += pitch_uv;
		VP += pitch_uv;

		pDest += display_pitch;
		pDest_pitch += display_pitch;
	}
}

#else // #ifdef __arm

long Cr_r_tab[] = {
	-273,-271,-269,-267,-264,-262,-260,-258,
	-256,-254,-252,-249,-247,-245,-243,-241,
	-239,-237,-235,-232,-230,-228,-226,-224,
	-222,-220,-217,-215,-213,-211,-209,-207,
	-205,-202,-200,-198,-196,-194,-192,-190,
	-188,-185,-183,-181,-179,-177,-175,-173,
	-170,-168,-166,-164,-162,-160,-158,-155,
	-153,-151,-149,-147,-145,-143,-141,-138,
	-136,-134,-132,-130,-128,-126,-123,-121,
	-119,-117,-115,-113,-111,-108,-106,-104,
	-102,-100, -98, -96, -94, -91, -89, -87,
	-85, -83, -81, -79, -76, -74, -72, -70,
	-68, -66, -64, -61, -59, -57, -55, -53,
	-51, -49, -47, -44, -42, -40, -38, -36,
	-34, -32, -29, -27, -25, -23, -21, -19,
	-17, -14, -12, -10,  -8,  -6,  -4,  -2,
	0,   2,   4,   6,   8,  10,  12,  14,
	17,  19,  21,  23,  25,  27,  29,  32,
	34,  36,  38,  40,  42,  44,  47,  49,
	51,  53,  55,  57,  59,  61,  64,  66,
	68,  70,  72,  74,  76,  79,  81,  83,
	85,  87,  89,  91,  94,  96,  98, 100,
	102, 104, 106, 108, 111, 113, 115, 117,
	119, 121, 123, 126, 128, 130, 132, 134,
	136, 138, 141, 143, 145, 147, 149, 151,
	153, 155, 158, 160, 162, 164, 166, 168,
	170, 173, 175, 177, 179, 181, 183, 185,
	188, 190, 192, 194, 196, 198, 200, 202,
	205, 207, 209, 211, 213, 215, 217, 220,
	222, 224, 226, 228, 230, 232, 235, 237,
	239, 241, 243, 245, 247, 249, 252, 254,
	256, 258, 260, 262, 264, 267, 269, 271,
};
long Cr_g_tab[] = {
	59,  59,  58,  58,  58,  57,  57,  56,
	56,  55,  55,  54,  54,  53,  53,  52,
	52,  51,  51,  51,  50,  50,  49,  49,
	48,  48,  47,  47,  46,  46,  45,  45,
	44,  44,  43,  43,  43,  42,  42,  41,
	41,  40,  40,  39,  39,  38,  38,  37,
	37,  36,  36,  36,  35,  35,  34,  34,
	33,  33,  32,  32,  31,  31,  30,  30,
	29,  29,  29,  28,  28,  27,  27,  26,
	26,  25,  25,  24,  24,  23,  23,  22,
	22,  21,  21,  21,  20,  20,  19,  19,
	18,  18,  17,  17,  16,  16,  15,  15,
	14,  14,  14,  13,  13,  12,  12,  11,
	11,  10,  10,   9,   9,   8,   8,   7,
	7,   7,   6,   6,   5,   5,   4,   4,
	3,   3,   2,   2,   1,   1,   0,   0,
	0,   0,   0,  -1,  -1,  -2,  -2,  -3,
	-3,  -4,  -4,  -5,  -5,  -6,  -6,  -7,
	-7,  -7,  -8,  -8,  -9,  -9, -10, -10,
	-11, -11, -12, -12, -13, -13, -14, -14,
	-14, -15, -15, -16, -16, -17, -17, -18,
	-18, -19, -19, -20, -20, -21, -21, -21,
	-22, -22, -23, -23, -24, -24, -25, -25,
	-26, -26, -27, -27, -28, -28, -29, -29,
	-29, -30, -30, -31, -31, -32, -32, -33,
	-33, -34, -34, -35, -35, -36, -36, -36,
	-37, -37, -38, -38, -39, -39, -40, -40,
	-41, -41, -42, -42, -43, -43, -43, -44,
	-44, -45, -45, -46, -46, -47, -47, -48,
	-48, -49, -49, -50, -50, -51, -51, -51,
	-52, -52, -53, -53, -54, -54, -55, -55,
	-56, -56, -57, -57, -58, -58, -58, -59,
};
long Cb_g_tab[] = {
	23,  23,  23,  23,  23,  22,  22,  22,
	22,  22,  22,  21,  21,  21,  21,  21,
	20,  20,  20,  20,  20,  19,  19,  19,
	19,  19,  19,  18,  18,  18,  18,  18,
	17,  17,  17,  17,  17,  16,  16,  16,
	16,  16,  16,  15,  15,  15,  15,  15,
	14,  14,  14,  14,  14,  14,  13,  13,
	13,  13,  13,  12,  12,  12,  12,  12,
	11,  11,  11,  11,  11,  11,  10,  10,
	10,  10,  10,   9,   9,   9,   9,   9,
	8,   8,   8,   8,   8,   8,   7,   7,
	7,   7,   7,   6,   6,   6,   6,   6,
	5,   5,   5,   5,   5,   5,   4,   4,
	4,   4,   4,   3,   3,   3,   3,   3,
	2,   2,   2,   2,   2,   2,   1,   1,
	1,   1,   1,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,  -1,  -1,
	-1,  -1,  -1,  -2,  -2,  -2,  -2,  -2,
	-2,  -3,  -3,  -3,  -3,  -3,  -4,  -4,
	-4,  -4,  -4,  -5,  -5,  -5,  -5,  -5,
	-5,  -6,  -6,  -6,  -6,  -6,  -7,  -7,
	-7,  -7,  -7,  -8,  -8,  -8,  -8,  -8,
	-8,  -9,  -9,  -9,  -9,  -9, -10, -10,
	-10, -10, -10, -11, -11, -11, -11, -11,
	-11, -12, -12, -12, -12, -12, -13, -13,
	-13, -13, -13, -14, -14, -14, -14, -14,
	-14, -15, -15, -15, -15, -15, -16, -16,
	-16, -16, -16, -16, -17, -17, -17, -17,
	-17, -18, -18, -18, -18, -18, -19, -19,
	-19, -19, -19, -19, -20, -20, -20, -20,
	-20, -21, -21, -21, -21, -21, -22, -22,
	-22, -22, -22, -22, -23, -23, -23, -23,
};
long Cb_b_tab[] = {
	-237,-235,-233,-231,-229,-227,-226,-224,
	-222,-220,-218,-216,-214,-213,-211,-209,
	-207,-205,-203,-202,-200,-198,-196,-194,
	-192,-190,-189,-187,-185,-183,-181,-179,
	-177,-176,-174,-172,-170,-168,-166,-164,
	-163,-161,-159,-157,-155,-153,-151,-150,
	-148,-146,-144,-142,-140,-139,-137,-135,
	-133,-131,-129,-127,-126,-124,-122,-120,
	-118,-116,-114,-113,-111,-109,-107,-105,
	-103,-101,-100, -98, -96, -94, -92, -90,
	-88, -87, -85, -83, -81, -79, -77, -75,
	-74, -72, -70, -68, -66, -64, -63, -61,
	-59, -57, -55, -53, -51, -50, -48, -46,
	-44, -42, -40, -38, -37, -35, -33, -31,
	-29, -27, -25, -24, -22, -20, -18, -16,
	-14, -12, -11,  -9,  -7,  -5,  -3,  -1,
	0,   1,   3,   5,   7,   9,  11,  12,
	14,  16,  18,  20,  22,  24,  25,  27,
	29,  31,  33,  35,  37,  38,  40,  42,
	44,  46,  48,  50,  51,  53,  55,  57,
	59,  61,  63,  64,  66,  68,  70,  72,
	74,  75,  77,  79,  81,  83,  85,  87,
	88,  90,  92,  94,  96,  98, 100, 101,
	103, 105, 107, 109, 111, 113, 114, 116,
	118, 120, 122, 124, 126, 127, 129, 131,
	133, 135, 137, 139, 140, 142, 144, 146,
	148, 150, 151, 153, 155, 157, 159, 161,
	163, 164, 166, 168, 170, 172, 174, 176,
	177, 179, 181, 183, 185, 187, 189, 190,
	192, 194, 196, 198, 200, 202, 203, 205,
	207, 209, 211, 213, 214, 216, 218, 220,
	222, 224, 226, 227, 229, 231, 233, 235,
};

#include "stdio.h"

void NXCC_YUV420ToRGB565_EX( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, 
						unsigned int nSrcPitch, unsigned int nDstPitch )
{
	int x;//, pitch2, pitch_org;
	int Y_gap, UV_gap, RGB_gap;

	height = (height % 2) != 0 ? height - 1 : height;

	Y_gap = ( nSrcPitch << 1 ) - width;
	UV_gap = (nSrcPitch-width)>>1;
	RGB_gap = ( nDstPitch << 1 ) - width;

	for( ; height; height -= 2 ) 	
	{
		for( x = 0; x < (int)width; x += 2 ) 
		{
			register int CR,CB,CG;
			int	Y;
			register int R, G, B;

			{
				int PCr, PCb;
				PCr = ((int)(*VP++) - 128);
				PCb = ((int)(*UP++) - 128);
#ifdef BT709
				CR = (401 * PCr) >> 8;
				CG =  - (120 * PCr + 48 * PCb) >> 8;
				CB = (476 * PCb) >> 8;
#else
				CR = (359 * PCr) >> 8;
				CG =  - (183 * PCr + 88 * PCb) >> 8;
				CB = (454 * PCb) >> 8;
#endif
			}
#if 0
			CR = Cr_r_tab[*VP];
			CG = Cr_g_tab[*VP] + Cb_g_tab[*UP];
			CB = Cb_b_tab[*UP];
			VP++;
			UP++;
#endif
			Y = YP[0];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );


			pDest[0] = CONV_RGB16(R, G, B);

			Y = YP[nSrcPitch];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );


			pDest[nDstPitch] = CONV_RGB16(R, G, B);

			YP++;
			pDest++;

			Y = YP[0];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			pDest[0] = CONV_RGB16(R, G, B);

			Y = YP[nSrcPitch];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			pDest[nDstPitch] = CONV_RGB16(R, G, B);
			//pDest[nDstPitch] = CONV_RGB16(B, G, R);

			YP++;
			pDest++;
		}

		YP += Y_gap;
		UP += UV_gap;
		VP += UV_gap;

		pDest += RGB_gap ;
	}
}

void NXCC_YUV420ToRGB888_EX( NXUINT32 *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, 
						unsigned int nSrcPitch, unsigned int nDstPitch )
{
	int x;
	int Y_gap, UV_gap, RGB_gap;

	height = (height % 2) != 0 ? height - 1 : height;

	Y_gap = ( nSrcPitch << 1 ) - width;
	UV_gap = (nSrcPitch-width)>>1;
	RGB_gap = ( nDstPitch << 1 ) - width;

	for( ; height; height -= 2 ) 	
	{
		for( x = 0; x < (int)width; x += 2 ) 
		{
			register int CR,CB,CG;
			int	Y;
			register int R, G, B;
			{
				int PCr, PCb;
				PCr = ((int)(*VP++) - 128);
				PCb = ((int)(*UP++) - 128);
#ifdef BT709
				CR = (401 * PCr) >> 8;
				CG =  - (120 * PCr + 48 * PCb) >> 8;
				CB = (476 * PCb) >> 8;
#else
				CR = (359 * PCr) >> 8;
				CG =  - (183 * PCr + 88 * PCb) >> 8;
				CB = (454 * PCb) >> 8;
#endif
			}
#if 0
			CR = Cr_r_tab[*VP];
			CG = Cr_g_tab[*VP] + Cb_g_tab[*UP];
			CB = Cb_b_tab[*UP];
			VP++;
			UP++;
#endif

			Y = YP[0];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			pDest[0] = CONV_RGB32(R, G, B);

			Y = YP[nSrcPitch];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			pDest[nDstPitch] = CONV_RGB32(R, G, B);

			YP++;
			pDest++;

			Y = YP[0];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			pDest[0] = CONV_RGB32(R, G, B);

			Y = YP[nSrcPitch];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			pDest[nDstPitch] = CONV_RGB32(R, G, B);

			YP++;
			pDest++;
		}

		YP += Y_gap;
		UP += UV_gap;
		VP += UV_gap;

		pDest += RGB_gap ;
	}
}

void NXCC_YUV420ToRGB888_EX_ROT180( NXUINT32 *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, 
						unsigned int nSrcPitch, unsigned int nDstPitch )
{
	int x;
	int Y_gap, UV_gap, RGB_gap;

	height = (height % 2) != 0 ? height - 1 : height;

	Y_gap = ( nSrcPitch << 1 ) - width;
	UV_gap = (nSrcPitch-width)>>1;
	RGB_gap = ( nDstPitch << 1 ) + width;

	pDest = pDest + (width * (height-1));

	for( ; height; height -= 2 ) 	
	{
		for( x = 0; x < (int)width; x += 2 ) 
		{
			register int CR,CB,CG;
			int	Y;
			register int R, G, B;
			{
				int PCr, PCb;
				PCr = ((int)(*VP++) - 128);
				PCb = ((int)(*UP++) - 128);
#ifdef BT709
				CR = (401 * PCr) >> 8;
				CG =  - (120 * PCr + 48 * PCb) >> 8;
				CB = (476 * PCb) >> 8;
#else
				CR = (359 * PCr) >> 8;
				CG =  - (183 * PCr + 88 * PCb) >> 8;
				CB = (454 * PCb) >> 8;
#endif
			}

			Y = YP[0];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			pDest[0] = CONV_RGB32(R, G, B);

			Y = YP[nSrcPitch];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			*(pDest-nDstPitch) = CONV_RGB32(R, G, B);

			YP++;
			pDest++;

			Y = YP[0];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			pDest[0] = CONV_RGB32(R, G, B);

			Y = YP[nSrcPitch];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );

			*(pDest-nDstPitch) = CONV_RGB32(R, G, B);

			YP++;
			pDest++;
		}

		YP += Y_gap;
		UP += UV_gap;
		VP += UV_gap;

		pDest -= RGB_gap;
	}
}

#endif // #ifdef __arm

//
// NXCC_YUV420ToRGB16_ROT270
//
// Assume :
//			- Output display size is constant
//			- Input size is even, value range is video range
//			- Input format is YUV planar, Luma and chResultoma pitch are same
//			- BT.709 conversion
//			- Rotate 270 degree
//			- Output RGB format is BGR565
//
#if defined(__arm) && defined(_ANDROID)

void NXCC_YUV420ToRGB16_ROT270( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, unsigned int pitch )
{
	int x, pitch2, pitch_org;
	unsigned short *pOut;

	width = ( width > DISPLAY_WIDTH_ROT ) ? DISPLAY_WIDTH_ROT : width;
	height = ( height > DISPLAY_HEIGHT_ROT ) ? DISPLAY_HEIGHT_ROT : height;
	pitch2 = ( pitch << 1 ) - width;
	pitch_org = pitch;
	pitch -= width;

	for( pDest += height - 2; height; height -= 2, pDest -= 2 ) 	
	{
		for( x = 0, pOut = pDest; x < width; x += 2 ) 
		{
			int CR,CB,CG;
			int	Y;
			unsigned int R, G;
			{
#ifdef __TARGET_FEATURE_DSPMUL

				int PCr, PCb;
				PCr = ((int)(*VP++) - 128);
				PCb = ((int)(*UP++) - 128);

				CR = (401 * PCr) >> 8;
				CG =  - (120 * PCr + 48 * PCb) >> 8;
				CB = (PCr + 476 * PCb) >> 8;

#else // #ifdef __TARGET_FEATURE_DSPMUL

				CR = ((int)(*VP++) - 128);
				CB = ((int)(*UP++) - 128);

				__asm
				{
					// CG
					SUB	R, CR, CR, LSL 4;
					SUB R, R, CB, LSL 1;
					SUB R, R, CB, LSL 2;
					MOV CG, R, ASR 5;

					// CB
					RSB R, CB, CB, LSL 3;
					ADD R, R, R, LSL 4;
					ADD R, CR, R, LSL 2;
					MOV CB, R, ASR 8

					// CR
					ADD R, CR, CR, LSL 3;
					ADD R, CR, R, LSL 4;
					ADD CR, CR, R, ASR 8;
				}
#endif // #ifdef __TARGET_FEATURE_DSPMUL
			}

			Y = *YP;
			CLIP_BGR16;
			pOut[1] = R;

			Y = YP[pitch_org];
			CLIP_BGR16;
			pOut[0] = R;
			YP++;
			pOut += DISPLAY_PITCH_ROT;

			Y = *YP;
			CLIP_BGR16;
			pOut[1] = R;

			Y = YP[pitch_org];
			CLIP_BGR16;
			pOut[0] = R;
			YP++;
			pOut += DISPLAY_PITCH_ROT;
		}

		YP += pitch2;
		UP += pitch >> 1;
		VP += pitch >> 1;
	}
}

#elif defined(__IWMMXT__)

void NXCC_YUV420ToRGB16_ROT270( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, unsigned int pitch )
{
	int x, pitch2, pitch_org;
	unsigned short *pOut;
	unsigned char *YP_pitch = YP + pitch;

	width = ( width > DISPLAY_WIDTH_ROT ) ? DISPLAY_WIDTH_ROT : width;
	height = ( height > DISPLAY_HEIGHT_ROT ) ? DISPLAY_HEIGHT_ROT : height;
	pitch2 = ( pitch << 1 ) - width;
	pitch_org = pitch;
	pitch -= width;

	asm volatile(
		"tbcsth		wR10, %0		\n\t"
		"tbcsth		wR11, %1		\n\t"
		"tbcsth		wR12, %2		\n\t"
		"tbcsth		wR13, %3		\n\t"
		"tbcsth		wR14, %4		\n\t"
		"tbcsth		wR15, %5		\n\t"
		:
		: "r" (145), "r" (-120), "r" (-48), "r" (220), "r" (8), "r" (128)
		);
	asm volatile(
		"tmcr	wCGR0, %0		\n\t"
		"tmcr	wCGR1, %1		\n\t"
		"tmcr	wCGR2, %2		\n\t"
		"tmcr	wCGR3, %3		\n\t"
		:
		: "r" (2), "r" (3), "r" (5), "r" (11)
		);
	for( pDest += height - 2; height; height -= 2, pDest -= 2 ) 	
	{
		for( x = 0, pOut = pDest; x < width; x += 4 ) 
		{
			asm volatile(
				"wldrh	wR0, [%0]		\n\t"	// 0 0 0 0 0 0 u1 u0
				"wldrh	wR1, [%1]		\n\t"	// 0 0 0 0 0 0 v1 v0

				"wunpckelub	wR0, wR0	\n\t"	// 0 0 PCb1 PCb0
				"wunpckelub	wR1, wR1	\n\t"	// 0 0 PCr1 PCr0

				"wunpckilh	wR0, wR0, wR0	\n\t"	// PCb1 PCb1 PCb0 PCb0
				"wunpckilh	wR1, wR1, wR1	\n\t"	// PCr1 PCr1 PCr0 PCr0

				"wsubh		wR0, wR0, wR15	\n\t"	// PCb - 128
				"wsubh		wR1, wR1, wR15	\n\t"	// PCr - 128

				// wR2 = CR, wR3 = CG, wR4 = CB
				"wmulsl		wR2, wR1, wR10	\n\t"	// 145 * PCr
				"wsrah		wR2, wR2, wR14	\n\t"	// (145 * PCr) >> 8
				"waddh		wR2, wR2, wR1	\n\t"	// PCr + (145 * PCr) >> 8

				"wmulsl		wR3, wR1, wR11	\n\t"	// -120 * PCr
				"wmulsl		wR4, wR0, wR12	\n\t"	// -48 * PCb
				"waddh		wR3, wR3, wR4	\n\t"	// -(120 * PCr + 48 * pCb)
				"wsrah		wR3, wR3, wR14	\n\t"	// -(120 * PCr + 48 * pCb) >> 8

				"wmulsl		wR4, wR0, wR13	\n\t"	// 220 * PCb
				"waddh		wR4, wR4, wR1	\n\t"	// PCr + 220 * PCb
				"wsrah		wR4, wR4, wR14	\n\t"	// (PCr + 220 * PCb) >> 8
				"waddh		wR4, wR4, wR0	\n\t"	// (PCr + 220 * PCb) >> 8 + PCb
				:
				: "r" (UP), "r" (VP)
				);
			UP += 2;
			VP += 2;

			// first line
			asm volatile(
				"wldrw		wR0, [%0]		\n\t"	// 0 0 0 0 y3 y2 y1 y0
				"wunpckelub	wR0, wR0		\n\t"	// y3 y2 y1 y0

				"waddh		wR7, wR0, wR2	\n\t"	// Y + CR
				"waddh		wR8, wR0, wR3	\n\t"	// Y + CG
				"waddh		wR9, wR0, wR4	\n\t"	// Y + CB
				"wpackhus	wR7, wR7, wR7	\n\t"
				"wpackhus	wR8, wR8, wR8	\n\t"
				"wpackhus	wR9, wR9, wR9	\n\t"
				"wunpckelub	wR7, wR7		\n\t"
				"wunpckelub	wR8, wR8		\n\t"
				"wunpckelub	wR9, wR9		\n\t"

				"wsrlhg		wR7, wR7, wCGR1	\n\t"	// R >> 3
				"wsrlhg		wR8, wR8, wCGR0	\n\t"	// G >> 2
				"wsrlhg		wR9, wR9, wCGR1	\n\t"	// B >> 3
				"wsllhg		wR7, wR7, wCGR3	\n\t"	// R << 11
				"wsllhg		wR8, wR8, wCGR2	\n\t"	// G << 5
				"wor		wR0, wR7, wR8	\n\t"	// (R << 11) | (G << 5)
				"wor		wR0, wR0, wR9	\n\t"	// (R << 11) | (G << 5) | B
													// RGB3_F RGB2_F RGB1_F RGB0_F
				:
				: "r" (YP)
				);
			YP += 4;

			// second line
			asm volatile(
				"wldrw		wR1, [%0]		\n\t"	// 0 0 0 0 y3 y2 y1 y0
				"wunpckelub	wR1, wR1		\n\t"	// y3 y2 y1 y0

				"waddh		wR7, wR1, wR2	\n\t"	// Y + CR
				"waddh		wR8, wR1, wR3	\n\t"	// Y + CG
				"waddh		wR9, wR1, wR4	\n\t"	// Y + CB
				"wpackhus	wR7, wR7, wR7	\n\t"
				"wpackhus	wR8, wR8, wR8	\n\t"
				"wpackhus	wR9, wR9, wR9	\n\t"
				"wunpckelub	wR7, wR7		\n\t"
				"wunpckelub	wR8, wR8		\n\t"
				"wunpckelub	wR9, wR9		\n\t"

				"wsrlhg		wR7, wR7, wCGR1	\n\t"	// R >> 3
				"wsrlhg		wR8, wR8, wCGR0	\n\t"	// G >> 2
				"wsrlhg		wR9, wR9, wCGR1	\n\t"	// B >> 3
				"wsllhg		wR7, wR7, wCGR3	\n\t"	// R << 11
				"wsllhg		wR8, wR8, wCGR2	\n\t"	// G << 5
				"wor		wR1, wR7, wR8	\n\t"	// (R << 11) | (G << 5)
				"wor		wR1, wR1, wR9	\n\t"	// (R << 11) | (G << 5) | B
													// RGB3_S RGB2_S RGB1_S RGB0_S
				:
				: "r" (YP_pitch)
				);
			YP_pitch += 4;

			asm volatile(
				"wunpckilh	wR2, wR1, wR0	\n\t"	// RGB1_F RGB1_S RGB0_F RGB0_S
				"wstrw		wR2, [%0]		\n\t"
				:
				: "r" (pOut)
				);
			pOut += DISPLAY_PITCH_ROT;
			asm volatile(
				"wshufh		wR2, wR2, #0x4E	\n\t"
				"wstrw		wR2, [%0]		\n\t"
				:
				: "r" (pOut)
				);
			pOut += DISPLAY_PITCH_ROT;
			asm volatile(
				"wunpckihh	wR2, wR1, wR0	\n\t"	// RGB3_F RGB3_S RGB2_F RGB2_S
				"wstrw		wR2, [%0]		\n\t"
				:
				: "r" (pOut)
				);
			pOut += DISPLAY_PITCH_ROT;
			asm volatile(
				"wshufh		wR2, wR2, #0x4E	\n\t"
				"wstrw		wR2, [%0]		\n\t"
				:
				: "r" (pOut)
				);
			pOut += DISPLAY_PITCH_ROT;
		}

		YP += pitch2;
		YP_pitch += pitch2;
		UP += pitch >> 1;
		VP += pitch >> 1;
	}
}

#else // #ifdef __arm

void NXCC_YUV420ToRGB16_ROT270( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, unsigned int pitch )
{
	int x, pitch2, pitch_org;
	unsigned short *pOut;

	width = ( width > DISPLAY_WIDTH_ROT ) ? DISPLAY_WIDTH_ROT : width;
	height = ( height > DISPLAY_HEIGHT_ROT ) ? DISPLAY_HEIGHT_ROT : height;
	pitch2 = ( pitch << 1 ) - width;
	pitch_org = pitch;
	pitch -= width;

	for( pDest += height - 2; height; height -= 2, pDest -= 2 ) 	
	{
		for( x = 0, pOut = pDest; x < (int)width; x += 2 ) 
		{
			int CR,CB,CG;
			int	Y;
			int R, G, B;
			{
				int PCr, PCb;
				PCr = ((int)(*VP++) - 128);
				PCb = ((int)(*UP++) - 128);

				CR = (401 * PCr) >> 8;
				CG =  - (120 * PCr + 48 * PCb) >> 8;
				CB = (PCr + 476 * PCb) >> 8;
			}

			Y = YP[pitch_org];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );
			pOut[0] = CONV_RGB16(R, G, B);

			Y = *YP++;
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );
			pOut[1] = CONV_RGB16(R, G, B);
			pOut += DISPLAY_PITCH_ROT;

			Y = YP[pitch_org];
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );
			pOut[0] = CONV_RGB16(R, G, B);

			Y = *YP++;
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );
			pOut[1] = CONV_RGB16(R, G, B);
			pOut += DISPLAY_PITCH_ROT;
		}

		YP += pitch2;
		UP += pitch >> 1;
		VP += pitch >> 1;
	}
}

#endif // #ifdef __arm

//
// NXCC_YUV420ToRGB16_DN2X2
//
// Assume :
//			- Output display size is constant
//			- Input size is even, value range is video range
//			- Input format is YUV planar, Luma and chResultoma pitch are same
//			- BT.709 conversion
//			- Output RGB format is BGR565
//
#if defined(__arm) && defined(_ANDROID)

void NXCC_YUV420ToRGB16_DN2X2( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, unsigned int pitch )
{
	int x, pitch2, pitch_org, pitch_out;

	width = ( width > DISPLAY_WIDTH << 1 ) ? ( DISPLAY_WIDTH << 1 ) : width;
	height = ( height > DISPLAY_HEIGHT << 1 ) ? ( DISPLAY_HEIGHT << 1 ) : height;
	pitch2 = ( pitch << 1 ) - width;
	pitch_org = pitch;
	pitch -= width;
	pitch_out = DISPLAY_PITCH - ( width >> 1 );

	for( ; height; height -= 2 ) 	
	{
		for( x = 0; x < width; x += 2 ) 
		{
			int CR,CB,CG;
			int	Y;
			unsigned int R, G;
			{
#ifdef __TARGET_FEATURE_DSPMUL

				int PCr, PCb;
				PCr = ((int)(*VP++) - 128);
				PCb = ((int)(*UP++) - 128);

				CR = (401 * PCr) >> 8;
				CG =  - (120 * PCr + 48 * PCb) >> 8;
				CB = (PCr + 476 * PCb) >> 8;

#else // #ifdef __TARGET_FEATURE_DSPMUL

				CR = ((int)(*VP++) - 128);
				CB = ((int)(*UP++) - 128);

				__asm
				{
					// CG
					SUB	R, CR, CR, LSL 4;
					SUB R, R, CB, LSL 1;
					SUB R, R, CB, LSL 2;
					MOV CG, R, ASR 5;

					// CB
					RSB R, CB, CB, LSL 3;
					ADD R, R, R, LSL 4;
					ADD R, CR, R, LSL 2;
					MOV CB, R, ASR 8

					// CR
					ADD R, CR, CR, LSL 3;
					ADD R, CR, R, LSL 4;
					ADD CR, CR, R, ASR 8;
				}
#endif // #ifdef __TARGET_FEATURE_DSPMUL
			}

			Y = YP[pitch_org];
			Y += *YP++;
			Y += YP[pitch_org];
			Y += *YP++;
			Y >>= 2;
			CLIP_BGR16;
			*pDest++ = R;
		}

		YP += pitch2;
		UP += pitch >> 1;
		VP += pitch >> 1;

		pDest += pitch_out;
	}
}

#elif defined(__IWMMXT__)

void NXCC_YUV420ToRGB16_DN2X2( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, unsigned int pitch )
{
	int x, pitch_y, pitch_uv, pitch_out;
	unsigned char *YP_pitch = YP + pitch;

	width = ( width > DISPLAY_WIDTH << 1 ) ? DISPLAY_WIDTH << 1 : width;
	height = ( height > DISPLAY_HEIGHT << 1 ) ? DISPLAY_HEIGHT << 1 : height;
	pitch_y = ( pitch << 1 ) - width;
	pitch_uv = (pitch - width) >> 1;
	pitch_out = DISPLAY_PITCH - (width >> 1);

	asm volatile(
		"tbcsth		wR10, %0		\n\t"
		"tbcsth		wR11, %1		\n\t"
		"tbcsth		wR12, %2		\n\t"
		"tbcsth		wR13, %3		\n\t"
		"tbcsth		wR14, %4		\n\t"
		"tbcsth		wR15, %5		\n\t"
		:
		: "r" (145), "r" (-120), "r" (-48), "r" (220), "r" (8), "r" (128)
		);
	asm volatile(
		"tmcr	wCGR0, %0		\n\t"
		"tmcr	wCGR1, %1		\n\t"
		"tmcr	wCGR2, %2		\n\t"
		"tmcr	wCGR3, %3		\n\t"
		:
		: "r" (2), "r" (3), "r" (5), "r" (11)
		);
	for( ; height; height -= 2 ) 	
	{
		for( x = 0; x < width; x += 8 ) 
		{
			asm volatile(
				"wldrw	wR0, [%0]		\n\t"	// u3 u2 u1 u0
				"wldrw	wR1, [%1]		\n\t"	// v3 v2 v1 v0

				"wunpckelub	wR0, wR0	\n\t"	// PCb
				"wunpckelub	wR1, wR1	\n\t"	// PCr

				"wsubh		wR0, wR0, wR15	\n\t"	// PCb - 128
				"wsubh		wR1, wR1, wR15	\n\t"	// PCr - 128

				// wR2 = CR, wR3 = CG, wR4 = CB
				"wmulsl		wR2, wR1, wR10	\n\t"	// 145 * PCr
				"wsrah		wR2, wR2, wR14	\n\t"	// (145 * PCr) >> 8
				"waddh		wR2, wR2, wR1	\n\t"	// PCr + (145 * PCr) >> 8

				"wmulsl		wR3, wR1, wR11	\n\t"	// -120 * PCr
				"wmulsl		wR4, wR0, wR12	\n\t"	// -48 * PCb
				"waddh		wR3, wR3, wR4	\n\t"	// -(120 * PCr + 48 * pCb)
				"wsrah		wR3, wR3, wR14	\n\t"	// -(120 * PCr + 48 * pCb) >> 8

				"wmulsl		wR4, wR0, wR13	\n\t"	// 220 * PCb
				"waddh		wR4, wR4, wR1	\n\t"	// PCr + 220 * PCb
				"wsrah		wR4, wR4, wR14	\n\t"	// (PCr + 220 * PCb) >> 8
				"waddh		wR4, wR4, wR0	\n\t"	// (PCr + 220 * PCb) >> 8 + PCb
				:
				: "r" (UP), "r" (VP)
				);
			UP += 4;
			VP += 4;

			asm volatile(
				"wldrd	wR0, [%0]		\n\t"	// y7 y6 y5 y4 y3 y2 y1 y0
				"wldrd	wR1, [%1]		\n\t"	// y7 y6 y5 y4 y3 y2 y1 y0

				"wsrlh	wR5, wR0, wR14	\n\t"	// y7 y5 y3 y1
				"wsllh	wR6, wR0, wR14	\n\t"
				"wsrlh	wR0, wR6, wR14	\n\t"	// y6 y4 y2 y0
				"wsrlh	wR6, wR1, wR14	\n\t"	// y7 y5 y3 y1
				"wsllh	wR7, wR1, wR14	\n\t"
				"wsrlh	wR1, wR7, wR14	\n\t"	// y6 y4 y2 y0

				"waddh	wR0, wR0, wR1	\n\t"
				"waddh	wR0, wR0, wR5	\n\t"
				"waddh	wR0, wR0, wR6	\n\t"
				"wsrlhg	wR0, wR0, wCGR0	\n\t"
				:
				: "r" (YP), "r" (YP_pitch)
				);
			YP += 8;
			YP_pitch += 8;

			asm volatile(
				"waddh		wR7, wR0, wR2	\n\t"	// Y + CR
				"waddh		wR8, wR0, wR3	\n\t"	// Y + CG
				"waddh		wR9, wR0, wR4	\n\t"	// Y + CB
				"wpackhus	wR7, wR7, wR7	\n\t"
				"wpackhus	wR8, wR8, wR8	\n\t"
				"wpackhus	wR9, wR9, wR9	\n\t"
				"wunpckelub	wR7, wR7		\n\t"
				"wunpckelub	wR8, wR8		\n\t"
				"wunpckelub	wR9, wR9		\n\t"

				"wsrlhg		wR7, wR7, wCGR1	\n\t"	// R >> 3
				"wsrlhg		wR8, wR8, wCGR0	\n\t"	// G >> 2
				"wsrlhg		wR9, wR9, wCGR1	\n\t"	// B >> 3
				"wsllhg		wR7, wR7, wCGR3	\n\t"	// R << 11
				"wsllhg		wR8, wR8, wCGR2	\n\t"	// G << 5
				"wor		wR0, wR7, wR8	\n\t"	// (R << 11) | (G << 5)
				"wor		wR0, wR0, wR9	\n\t"	// (R << 11) | (G << 5) | B

				"wstrd		wR0, [%0]		\n\t"
				:
				: "r" (pDest)
				);
			pDest += 4;
		}

		YP += pitch_y;
		YP_pitch += pitch_y;
		UP += pitch_uv;
		VP += pitch_uv;

		pDest += pitch_out;
	}
}

#else // #ifdef __arm

void NXCC_YUV420ToRGB16_DN2X2( unsigned short *pDest, 
						unsigned char *YP, unsigned char *UP, unsigned char *VP, 
						unsigned int width, unsigned int height, unsigned int pitch )
{
	int x, pitch2, pitch_org, pitch_out;

	width = ( width > DISPLAY_WIDTH << 1 ) ? DISPLAY_WIDTH << 1 : width;
	height = ( height > DISPLAY_HEIGHT << 1 ) ? DISPLAY_HEIGHT << 1 : height;
	pitch2 = ( pitch << 1 ) - width;
	pitch_org = pitch;
	pitch -= width;
	pitch_out = DISPLAY_PITCH - ( width >> 1 );

	for( ; height; height -= 2 ) 	
	{
		for( x = 0; x < (int)width; x += 2 ) 
		{
			int CR,CB,CG;
			int	Y;
			unsigned int R, G, B;
			{
				int PCr, PCb;
				PCr = ((int)(*VP++) - 128);
				PCb = ((int)(*UP++) - 128);

				CR = (401 * PCr) >> 8;
				CG =  - (120 * PCr + 48 * PCb) >> 8;
				CB = (PCr + 476 * PCb) >> 8;
			}

			Y = YP[pitch_org];
			Y += *YP++;
			Y += YP[pitch_org];
			Y += *YP++;
			Y >>= 2;
			CLIP_255(R, Y + CR );
			CLIP_255(G, Y + CG );
			CLIP_255(B, Y + CB );
			*pDest++ = CONV_RGB16(R, G, B);
		}

		YP += pitch2;
		UP += pitch >> 1;
		VP += pitch >> 1;

		pDest += pitch_out;
	}
}

#endif // #ifdef __arm
