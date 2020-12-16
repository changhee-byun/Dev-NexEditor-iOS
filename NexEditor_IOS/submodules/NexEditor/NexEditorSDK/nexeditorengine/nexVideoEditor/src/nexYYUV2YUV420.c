// #define RVDS_NEON

#include "nexYYUV2YUV420.h"
#ifdef RVDS_NEON
#include <arm_neon.h>
#endif


int nexYYUVtoYUV420(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* pUV)
{
	int				x, y;

	unsigned char*	pNewY		= pY + (iWidth * iHeight ) - iWidth;
	unsigned char*	pNewUV		= pUV + (iWidth * iHeight / 2) - iWidth;

	unsigned char* 	pTempBuf = pSrc;
		
#ifdef RVDS_NEON	
	uint8x8_t val;
	uint16x8_t low;
	uint8x16_t val1, val2;
	uint16x4_t val3, val4;	
	uint16x4x2_t uvVal;
		
	int iMultiWidth			= iWidth * 2;
	int iHalfWidth			= iWidth /2;
	unsigned char* pYTop		= pY + (iWidth * iHeight) - iWidth;
	unsigned char* pYBottom	= pY + (iWidth * iHeight / 2) - iWidth;
	
	unsigned char* pU			= pUV + (iWidth * iHeight / 2) -iWidth;
	unsigned char* pV			= pUV + (iWidth * iHeight / 2) -iWidth + 1;
	

	unsigned char* pYT		= pSrc;
	unsigned char* pYB		= pSrc + 1;
	unsigned char* pTmpUV	= pSrc;
	
	for(  y = 0; y < iHeight/2; y++)
	{
		for(x=0; x<iWidth>>3; x++) {
		
			val1 = vld1q_u8(pTempBuf); 
			val2 = vld1q_u8(pTempBuf + 16);
			val3 = vmovn_u32(vreinterpretq_u32_u8(val1));			
			val4 = vmovn_u32(vreinterpretq_u32_u8(val2));
			low = vcombine_u16(val3, val4);
			val = vmovn_u16(low);
			vst1_u8(pYTop, val); pYTop += 8;
						
			val3 = vmovn_u32(vshrq_n_u32(vreinterpretq_u32_u8(val1), 8));			
			val4 = vmovn_u32(vshrq_n_u32(vreinterpretq_u32_u8(val2), 8));
			low = vcombine_u16(val3, val4);
			val = vmovn_u16(low);
			vst1_u8(pYBottom, val); pYBottom += 8;

			pTempBuf += 32;
			
			//UV
			val1 = vld1q_u8(pTmpUV); 
			val2 = vld1q_u8(pTmpUV+iHalfWidth);
			val3 = vmovn_u32(vshrq_n_u32(vreinterpretq_u32_u8(val1), 16));			
			val4 = vmovn_u32(vshrq_n_u32(vreinterpretq_u32_u8(val2), 16));
			low = vcombine_u16(val3, val4);
			val = vmovn_u16(low);
			low = vmovl_u8(val);
			uvVal = vzip_u16(vget_low_u16(low), vget_high_u16(low));
			low = vcombine_u16(uvVal.val[0], uvVal.val[1]);
			val = vmovn_u16(low);
			vst1_u8(pU, val); pU += 8;
			
			pTmpUV += 16;
		}
		
		pTmpUV += iHalfWidth;
		pYTop -= iMultiWidth;
		pYBottom -= iMultiWidth;
		pU -= iMultiWidth;
	}
#else	
	{
		int x, y;
		int iMultiWidth			= iWidth * 2;
		int iHalfWidth			= iWidth /2;
		unsigned char* pYTop		= pY + (iWidth * iHeight) - iWidth;
		unsigned char* pYBottom	= pY + (iWidth * iHeight / 2) - iWidth;
		
		unsigned char* pU			= pUV + (iWidth * iHeight / 4) -iHalfWidth;
		unsigned char* pV			= pU + (iWidth * iHeight / 4);
		

		unsigned char* pYT		= pSrc;
		unsigned char* pYB		= pSrc + 1;
		unsigned char* pTmpUV	= pSrc + 2;
		
		for(y = 0; y < iHeight / 2; y++)
		{
			for(x = 0; x < iWidth; x++)
			{
				*pYTop = *pYT;
				pYTop++;
				pYT += 4;
				
				*pYBottom = *pYB;
				pYBottom++;
				pYB += 4;

				if( x < iHalfWidth)
				{
					*pU = *pTmpUV;
					pU++;
				}
				else
				{
					*pV = *pTmpUV;
					pV++;
				}
				pTmpUV += 4;
			}
			pYTop -= iMultiWidth;
			pYBottom -= iMultiWidth;

			pU -= iWidth;
			pV -= iWidth;
		}
	}

	return 0;
#endif
}

int nexYYUVtoY2UV(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* pUV)
{
	int				x, y;

	unsigned char*	pNewY		= pY + (iWidth * iHeight ) - iWidth;
	unsigned char*	pNewUV		= pUV + (iWidth * iHeight / 2) - iWidth;

	unsigned char* 	pTempBuf = pSrc;
		
	{
		int x, y;
		int iMultiWidth			= iWidth * 2;
		int iHalfWidth			= iWidth /2;
		unsigned char* pYTop		= pY + (iWidth * iHeight) - iWidth;
		unsigned char* pYBottom	= pY + (iWidth * iHeight / 2) - iWidth;
		
		unsigned char* pU			= pUV + (iWidth * iHeight / 2) -iWidth;
		unsigned char* pV			= pUV + (iWidth * iHeight / 2) -iWidth + 1;
		

		unsigned char* pYT		= pSrc;
		unsigned char* pYB		= pSrc + 1;
		unsigned char* pTmpUV	= pSrc + 2;
		
		for(y = 0; y < iHeight / 2; y++)
		{
			for(x = 0; x < iWidth; x++)
			{
				*pYTop = *pYT;
				pYTop++;
				pYT += 4;
				
				*pYBottom = *pYB;
				pYBottom++;
				pYB += 4;

				if( x < iHalfWidth)
				{
					*pU = *pTmpUV;
					pU += 2;
				}
				else
				{
					*pV = *pTmpUV;
					pV += 2;
				}
				pTmpUV += 4;
			}
			pYTop -= iMultiWidth;
			pYBottom -= iMultiWidth;

			pU -= iMultiWidth;
			pV -= iMultiWidth;
		}
	}	
	return 0;
}

int nexYYUVtoY2VU(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* pUV)
{
	int				x, y;

	unsigned char*	pNewY		= pY + (iWidth * iHeight ) - iWidth;
	unsigned char*	pNewUV		= pUV + (iWidth * iHeight / 2) - iWidth;

	unsigned char* 	pTempBuf = pSrc;
		
	{
		int x, y;
		int iMultiWidth			= iWidth * 2;
		int iHalfWidth			= iWidth /2;
		unsigned char* pYTop		= pY + (iWidth * iHeight) - iWidth;
		unsigned char* pYBottom	= pY + (iWidth * iHeight / 2) - iWidth;
		
		unsigned char* pV			= pUV + (iWidth * iHeight / 2) -iWidth;
		unsigned char* pU			= pUV + (iWidth * iHeight / 2) -iWidth + 1;
		

		unsigned char* pYT		= pSrc;
		unsigned char* pYB		= pSrc + 1;
		unsigned char* pTmpUV	= pSrc + 2;
		
		for(y = 0; y < iHeight / 2; y++)
		{
			for(x = 0; x < iWidth; x++)
			{
				*pYTop = *pYT;
				pYTop++;
				pYT += 4;
				
				*pYBottom = *pYB;
				pYBottom++;
				pYB += 4;

				if( x < iHalfWidth)
				{
					*pU = *pTmpUV;
					pU += 2;
				}
				else
				{
					*pV = *pTmpUV;
					pV += 2;
				}
				pTmpUV += 4;
			}
			pYTop -= iMultiWidth;
			pYBottom -= iMultiWidth;

			pU -= iMultiWidth;
			pV -= iMultiWidth;
		}
	}	
	return 0;
}

int nexYYUVAtoYUV420(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* p_U, unsigned char* p_V)
{
	int				x, y;

	//unsigned char*	pNewY		= pY + (iWidth * iHeight ) - iWidth;
	//unsigned char*	pNewUV		= pUV + (iWidth * iHeight / 2) - iWidth;

	unsigned char* 	pTempBuf = pSrc;
		
	{
		int x, y;
		int iMultiWidth			= iWidth * 2;
		int iHalfWidth			= iWidth /2;
		unsigned char* pYTop		= pY + (iWidth * iHeight) - iWidth;
		unsigned char* pYBottom	= pY + (iWidth * iHeight / 2) - iWidth;
		
		unsigned char* pU			= p_U+ (iWidth * iHeight / 4) -iHalfWidth;
		unsigned char* pV			= p_V+ (iWidth * iHeight / 4) -iHalfWidth;

		//unsigned char* pV			= p_V+ (iWidth * iHeight / 2) -iWidth + 1;
		

		unsigned char* pYT		= pSrc;
		unsigned char* pYB		= pSrc + 1;
		unsigned char* pTmpUV	= pSrc + 2;
		
		for(y = 0; y < iHeight / 2; y++)
		{
			for(x = 0; x < iWidth; x++)
			{
				*pYTop = *pYT;
				pYTop++;
				pYT += 4;
				
				*pYBottom = *pYB;
				pYBottom++;
				pYB += 4;

				if( x < iHalfWidth)
				{
					*pU = *pTmpUV;
					pU++;
				}
				else
				{
					*pV = *pTmpUV;
					pV++;
				}
				pTmpUV += 4;
			}
			pYTop -= iMultiWidth;
			pYBottom -= iMultiWidth;

			pU -= iWidth;
			pV -= iWidth;
		}
	}	
	return 0;
}

int nexUVYYAtoYUV420(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* p_U, unsigned char* p_V)
{
	int				x, y;

	unsigned char* 	pTempBuf = pSrc;
		
	{
		int x, y;
		int iMultiWidth			= iWidth * 2;
		int iHalfWidth			= iWidth /2;
		unsigned char* pYTop		= pY + (iWidth * iHeight / 2);
		unsigned char* pYBottom	= pY;
		
		unsigned char* pU			= p_U;
		unsigned char* pV			= p_V;

		//unsigned char* pV			= p_V+ (iWidth * iHeight / 2) -iWidth + 1;
		

		unsigned char* pYT		= pSrc + 2;
		unsigned char* pYB		= pSrc + 1;
		unsigned char* pTmpUV	= pSrc;
		
		for(y = 0; y < iHeight / 2; y++)
		{
			for(x = 0; x < iWidth; x++)
			{
				*pYTop = *pYT;
				pYTop++;
				pYT += 4;
				
				*pYBottom = *pYB;
				pYBottom++;
				pYB += 4;

				if( x < iHalfWidth)
				{
					*pU = *pTmpUV;
					pU++;
				}
				else
				{
					*pV = *pTmpUV;
					pV++;
				}
				pTmpUV += 4;
			}
		}
	}	
	return 0;
}


