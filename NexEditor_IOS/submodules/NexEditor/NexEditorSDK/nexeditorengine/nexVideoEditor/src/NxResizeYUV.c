#include <stdlib.h>
#include "NxVScaler.h"

#define STATIC	extern

#define UNALIGNED_ACCESS

#if defined(__ARMCC_VERSION)
#define RVDS_ARM
#ifdef __TARGET_FEATURE_NEON
#define RVDS_NEON
#endif
#if defined(__TARGET_ARCH_6) || (__TARGET_ARCH_ARM >= 6)
#define RVDS_ARMV6A
#ifdef UNALIGNED_ACCESS
#define RVDS_ARMV6
#endif
#endif
#endif


#define XMERGE2(a, b)	a##b
#define MERGE2(a, b)	XMERGE2(a, b)
#define XMERGE3(a, b, c)	a##b##c
#define MERGE3(a, b, c)	XMERGE3(a, b, c)

#ifdef __ARMCC_VERSION
#define ARCH_POSTFIX	MERGE2(ARMV, __TARGET_ARCH_ARM)
#else
#define ARCH_POSTFIX	PC
//#define ARCH_POSTFIX
#endif

#define FUNC_ARM(a) MERGE2(a, ARCH_POSTFIX)


#define SEPARATE_HV

#if 0
typedef struct {
	unsigned int dWidth, dHeight, dPitch;
	unsigned int width, height, pitch;
	unsigned short *indexTableY;
	unsigned char *phaseTableY;
	unsigned char *posTableY;
	unsigned short *indexTableC;
	unsigned char *phaseTableC;
	unsigned char *posTableC;
} NxVScalerStruct;
#endif

#define LINEBUF_PIXELS	1280

#if 0
static void EnlargeComponent(unsigned char *dst, unsigned int dWidth, unsigned int dHeight, unsigned int dPitch, unsigned char *src, unsigned int width, unsigned int height, unsigned int pitch)
{
	unsigned int i, j;
	unsigned short indexTable[LINEBUF_PIXELS];
	unsigned char phaseTable[LINEBUF_PIXELS];
	unsigned char *dPtr, *sPtr;
	unsigned int pos;


	for (j = 0; j < dWidth; j++) {
		pos = j * 8 * (width - 1) / (dWidth - 1);
		indexTable[j] = pos >> 3;
		phaseTable[j] = pos & 0x7;
	}

	sPtr = src;
	dPtr = dst;
	for (i = 0; i < dHeight; i++) {
		unsigned int indexY, phaseY;
		pos = (i * 8 * (height - 1) / (dHeight - 1));
		indexY = pos >> 3;
		phaseY = pos & 7;

		sPtr = &src[indexY * pitch];
		for (j = 0; j < dWidth; j++) {
			unsigned int index, phase;
			unsigned int pixelA, pixelB, pixel;
			index = indexTable[j];
			phase = phaseTable[j];
			pixelA = sPtr[index] * (8 - phase) + sPtr[index+1] * phase;
			pixelB = sPtr[pitch+index] * (8 - phase) + sPtr[pitch+index+1] * phase;
			pixel = (pixelA * (8 - phaseY) + pixelB * phaseY) >> 6;
			dPtr[j] = pixel;
		}
		dPtr += dPitch;
	}

	//printf("hm...\n");
}

static void EnlargeYUV(unsigned char *dY, unsigned char *dU, unsigned char *dV,
				unsigned int dWidth, unsigned int dHeight, unsigned int dPitch,
				unsigned char *sY, unsigned char *sU, unsigned char *sV,
				unsigned int width, unsigned int height, unsigned int pitch)
{
	EnlargeComponent(dY, dWidth, dHeight, dPitch, sY, width, height, pitch);
	dWidth = (dWidth + 1) >> 1;
	dHeight = (dHeight + 1) >> 1;
	dPitch = (dPitch + 1) >> 1;
	width = (width + 1) >> 1;
	height = (height + 1) >> 1;
	pitch = (pitch + 1) >> 1;

	EnlargeComponent(dU, dWidth, dHeight, dPitch, sU, width, height, pitch);
	EnlargeComponent(dV, dWidth, dHeight, dPitch, sV, width, height, pitch);
}
#endif

NxVScalerStruct *FUNC_ARM(NxVScalerInit)(unsigned int dWidth, unsigned int dHeight, unsigned int dPitch,
				unsigned int width, unsigned int height, unsigned int pitch)
{
	NxVScalerStruct *nv;
	unsigned short *indexTableY, *indexTableC;
	unsigned char *phaseTableY, *phaseTableC, *avgBuffer, *posTableY, *posTableC;
	
	unsigned int	phSize, poSize;
	unsigned int j;

	nv = (NxVScalerStruct *)malloc(sizeof(NxVScalerStruct));

	if (nv == NULL)
		return NULL;

	nv->dWidth = dWidth;
	nv->dHeight = dHeight;
	nv->dPitch = dPitch;

	nv->width = width;
	nv->height = height;
	nv->pitch = pitch;

	phSize = (dWidth < width) ? 4 : dWidth;
	poSize = (dWidth < width) ? height : (((dHeight + 1) >> 1) << 2);

	indexTableY = nv->indexTableY = (unsigned short *)malloc(2 * dWidth);
	//phaseTableY = nv->phaseTableY = (unsigned char *)malloc(phSize);
	indexTableC = nv->indexTableC = (unsigned short *)malloc(((dWidth+1)>>1)<<1);
	//phaseTableC = nv->phaseTableC = (unsigned char *)malloc((phSize+1) >> 1);

	posTableY = nv->posTableY = (unsigned char *)malloc(poSize);
	posTableC = nv->posTableC = (unsigned char *)malloc((poSize+1)>>1);

	avgBuffer = nv->avgBuffer = (unsigned char *)malloc(width);


	if (indexTableY == NULL || indexTableC == NULL || posTableY == NULL || posTableC == NULL || avgBuffer == NULL) {
		if (indexTableY)
			free(indexTableY);

		if (indexTableC)
			free(indexTableC);

		if (avgBuffer)
			free(avgBuffer);
		free(nv);
		return NULL;
	}

	if (dWidth < width) {
		signed char *pTableY=(signed char *)posTableY, *pTableC=(signed char *)posTableC;
		unsigned int wIRatio, hIRatio, minIRatio;
		unsigned int m, n, cW, cH;
		unsigned int nW, nH;
		unsigned int i, k;
		int pos;
#ifdef SEPARATE_HV
		wIRatio = width / dWidth;
		hIRatio = height / dHeight;

		for (i = 0; i < 16; i++) {
			if ((1 << i) > wIRatio) {
				m = i - 1;
				break;
			}
		}
		for (i = 0; i < 16; i++) {
			if ((1 << i) > hIRatio) {
				n = i - 1;
				break;
			}
		}
		
		nv->m = m;
		nv->n = n;

		nv->cW = cW = 1 << m;
		nv->cH = cH = 1 << n;
		//r = (1 << (2 * n - 1)) - 1;
		nv->nW = nW = width >> m;
		nv->nH = nH = height >> n;
#else
		wIRatio = width / dWidth;
		hIRatio = height / dHeight;

		minIRatio = MIN(wIRatio, hIRatio);

		for (i = 0; i < 16; i++) {
			if ((1 << i) > minIRatio) {
				n = i - 1;
				break;
			}
		}
		m = n;

		
		nv->cW = cW = 1 << n;
		nv->cH = cH = 1 << n;
		//r = (1 << (2 * n - 1)) - 1;
		nv->nW = nW = width >> n;
		nv->nH = nH = height >> n;
#endif
		for (j = 0; j < dWidth; j++)
			indexTableY[j] = (2 * j * nW + (dWidth/2)) / dWidth;

		k = 0;
		for (i = 0; i < nH; i++) {
			pTableY[i] = pos = (2 * i * dHeight / nH) - 2 * k;
			if (pos >= 0)
				k++;
		}

		dWidth = (dWidth+1)>>1;
		dHeight = (dHeight+1)>>1;
		width = (width+1)>>1;
		nW >>= 1;
		//nH >>= 1;
		nH = (nH + 1) >> 1;
		for (j = 0; j < dWidth; j++)
			indexTableC[j] = (2 * j * nW + (dWidth/2)) / dWidth;
		
		k = 0;
		for (i = 0; i < nH; i++) {
			pTableC[i] = pos = (2 * i * dHeight / nH) - 2 * k;
			if (pos >= 0)
				k++;
		}

		nv->mode = 1;
	} else {
		unsigned short *pTableY=(unsigned short *)posTableY, *pTableC=(unsigned short *)posTableC;
		unsigned int i;

		for (j = 0; j < dWidth; j++) {
			unsigned int pos;
			pos = j * 8 * (width - 1) / (dWidth - 1);
			//indexTableY[j] = pos >> 3;
			//phaseTableY[j] = pos & 0x7;
			indexTableY[j] = pos;
		}

		for (i = 0; i < dHeight; i++) {
			pTableY[i] = (i * 8 * (height - 1) / (dHeight - 1));
		}

		dWidth = (dWidth+1)>>1;
		width = (width+1)>>1;
		
		dHeight = (dHeight+1)>>1;
		height = (height+1)>>1;

		for (j = 0; j < dWidth; j++) {
			unsigned int pos;
			pos = j * 8 * (width - 1) / (dWidth - 1);
			//indexTableC[j] = pos >> 3;
			//phaseTableC[j] = pos & 0x7;
			indexTableC[j] = pos;
		}

		for (i = 0; i < dHeight; i++) {
			pTableC[i] = (i * 8 * (height - 1) / (dHeight - 1));
		}

		nv->mode = 2;
	}



	return nv;
}

#if 1
STATIC void NxVScalerEnlargeComponent(NxVScalerStruct *nv, unsigned char *dst, unsigned char *src, int bChroma)
{
	unsigned int i, j;
	unsigned short *indexTable;
	unsigned short *posTable;
	unsigned char *dPtr, *sPtr;
	unsigned int pos;


	unsigned int width, height, pitch, dWidth, dHeight, dPitch;
	unsigned int d_diff;

	width = nv->width;
	height = nv->height;
	pitch = nv->pitch;
	dWidth = nv->dWidth;
	dHeight = nv->dHeight;
	dPitch = nv->dPitch;

	if (bChroma) {
		dWidth = (dWidth + 1) >> 1;
		dHeight = (dHeight + 1) >> 1;
		dPitch = (dPitch + 1) >> 1;
		width = (width + 1) >> 1;
		height = (height + 1) >> 1;
		pitch = (pitch + 1) >> 1;

		indexTable = nv->indexTableC;
		//phaseTable = nv->phaseTableC;
		posTable = (unsigned short *)nv->posTableC;
	} else {
		indexTable = nv->indexTableY;
		//phaseTable = nv->phaseTableY;
		posTable = (unsigned short *)nv->posTableY;
	}

	d_diff = dPitch - dWidth;
	sPtr = src;
	dPtr = dst;
	for (i = 0; i < dHeight; i++) {
		unsigned short *ip = indexTable;
		unsigned int indexY, phaseY;
		unsigned int p1;
		//pos = (i * 8 * (height - 1) / (dHeight - 1));
		pos = posTable[i];
		indexY = pos >> 3;
		phaseY = pos & 7;
		p1 = (phaseY << 16) | (8 - phaseY);

		sPtr = &src[indexY * pitch];
		//for (j = 0; j < dWidth; j++) {
		for (j = dWidth; j; j--) {
			unsigned int index, phase;
			unsigned int pixelA, pixelB, pixel;
			//pos = indexTable[j];
			pos = *ip++;
			index = pos >> 3;
			phase = pos & 7;
			//index = indexTable[j];
			//phase = phaseTable[j];
#ifdef RVDS_ARMV6
			{
				unsigned int a0, a1, p0;
				p0 = (phase << 16) | (8 - phase);
				a0 = (sPtr[index+1] << 16) | sPtr[index];
				a1 = (sPtr[pitch+index+1] << 16) | sPtr[pitch+index];
				
				__asm {
					SMUAD	pixelA, a0, p0
					SMUAD	pixelB, a1, p0
				}
				pixelA = pixelA | (pixelB << 16);
				__asm {
					SMUAD	pixel, pixelA, p1
				}
				pixel = pixel >> 6;
			}
#else
			pixelA = sPtr[index] * (8 - phase) + sPtr[index+1] * phase;
			pixelB = sPtr[pitch+index] * (8 - phase) + sPtr[pitch+index+1] * phase;
			pixel = (pixelA * (8 - phaseY) + pixelB * phaseY) >> 6;
#endif
			
			*dPtr++ = pixel;
		}
		//dPtr += dPitch;
		dPtr += d_diff;
	}
}
#else
STATIC void NxVScalerEnlargeComponent(NxVScalerStruct *nv, unsigned char *dst, unsigned char *src, int bChroma)
{
	unsigned int i, j;
	unsigned short *indexTable;
	unsigned short *posTable;
	unsigned char *dPtr, *sPtr;
	unsigned int pos;


	unsigned int width, height, pitch, dWidth, dHeight, dPitch;
	unsigned int d_diff;

	width = nv->width;
	height = nv->height;
	pitch = nv->pitch;
	dWidth = nv->dWidth;
	dHeight = nv->dHeight;
	dPitch = nv->dPitch;

	if (bChroma) {
		dWidth = (dWidth + 1) >> 1;
		dHeight = (dHeight + 1) >> 1;
		dPitch = (dPitch + 1) >> 1;
		width = (width + 1) >> 1;
		height = (height + 1) >> 1;
		pitch = (pitch + 1) >> 1;

		indexTable = nv->indexTableC;
		//phaseTable = nv->phaseTableC;
		posTable = (unsigned short *)nv->posTableC;
	} else {
		indexTable = nv->indexTableY;
		//phaseTable = nv->phaseTableY;
		posTable = (unsigned short *)nv->posTableY;
	}

	d_diff = dPitch - dWidth;
	sPtr = src;
	dPtr = dst;
	for (i = 0; i < dHeight; i++) {
		unsigned short *ip = indexTable;
		unsigned int indexY, phaseY;
		unsigned int p1;
		//pos = (i * 8 * (height - 1) / (dHeight - 1));
		pos = posTable[i];
		indexY = pos >> 3;
		phaseY = pos & 7;
		p1 = (phaseY << 16) | (8 - phaseY);

		sPtr = &src[indexY * pitch];
		//for (j = 0; j < dWidth; j++) {
		for (j = dWidth; j; j--) {
			unsigned int index, phase;
			unsigned int pixelA, pixelB, pixel;
			//pos = indexTable[j];
			pos = *ip++;
			index = pos >> 3;
			phase = pos & 7;
			//index = indexTable[j];
			//phase = phaseTable[j];
#ifdef RVDS_ARMV6
			{
				unsigned int a0, a1, p0;
				p0 = (phase << 16) | (8 - phase);
				a0 = (sPtr[index+1] << 16) | sPtr[index];
				a1 = (sPtr[pitch+index+1] << 16) | sPtr[pitch+index];
				
				__asm {
					SMUAD	pixelA, a0, p0
					SMUAD	pixelB, a1, p0
				}
				pixelA = pixelA | (pixelB << 16);
				__asm {
					SMUAD	pixel, pixelA, p1
				}
				pixel = pixel >> 6;
			}
#else
			pixelA = sPtr[index] * (8 - phase) + sPtr[index+1] * phase;
			pixelB = sPtr[pitch+index] * (8 - phase) + sPtr[pitch+index+1] * phase;
			pixel = (pixelA * (8 - phaseY) + pixelB * phaseY) >> 6;
#endif
			
			*dPtr++ = pixel;
		}
		//dPtr += dPitch;
		dPtr += d_diff;
	}
}
#endif

#if 0
STATIC void NxVScalerReduceComponent(NxVScalerStruct *nv, unsigned char *dst, unsigned char *src, int bChroma)
{
	unsigned int i, j, ii, jj, k;
	unsigned int c, nW, nH;
	unsigned char *s_ptr, *ptr;
	unsigned int R;
	//unsigned char buffer[LINEBUF_PIXELS * 2];
	unsigned char *avgBuffer = nv->avgBuffer;
	unsigned short *indexTable;
	int pos, index, phase;
	unsigned char *dPtr, *bPtr, *cBuffer;
	unsigned int width, pitch, dWidth, dHeight, dPitch;
	signed char *posTable;
	//signed char posTable[512];

	
	width = nv->width;
	pitch = nv->pitch;

	dWidth = nv->dWidth;
	dHeight = nv->dHeight;
	dPitch = nv->dPitch;
	nW = nv->nW;
	nH = nv->nH;



	if (bChroma) {
		dWidth = (dWidth + 1) >> 1;
		dHeight = (dHeight + 1) >> 1;
		dPitch = (dPitch + 1) >> 1;
		width = (width + 1) >> 1;
		
		pitch = (pitch + 1) >> 1;

		nW = (nW + 1) >> 1;
		nH = (nH + 1) >> 1;
		indexTable = nv->indexTableC;
		posTable = (signed char *)(nv->posTableC);
	} else {
		indexTable = nv->indexTableY;
		posTable = (signed char *)(nv->posTableY);
	}

#if 1
	for (i = nH; i; i--, src += pitch) {
		pos = *posTable++;
#else
	for (i = 0; i < nH; i++, src += pitch) {
		pos = posTable[i];
#endif
		if (pos < 0) {
			//src += pitch;
			continue;
		}

		else if (pos) {
#ifdef RVDS_ARMV6
//#if 0
			unsigned long *lsp0 = (unsigned long *)src;
			unsigned long *lsp1 = (unsigned long *)(src + pitch);
			unsigned long *ldp = (unsigned long *)avgBuffer;
			unsigned int j;

			for (j = (nW>>2);j;j--) {
				unsigned long d0, s0, s1;
				s0 = *lsp0++;
				s1 = *lsp1++;
				__asm {
					UHADD8	d0, s0, s1;
				}
				*ldp++ = d0;
			}
#else
			for (j = 0; j < nW; j++) {
				avgBuffer[j] = (src[j] + src[pitch + j]) >> 1;
			}
#endif
			bPtr = avgBuffer;
		} else {
			bPtr = src;
		}

		//src += pitch;
#if 1
		dPtr = dst;
		{
			unsigned short *iPtr = indexTable;
			for (j = dWidth; j; j--) {
				unsigned int p, d;
				//index = (2 * j * nW + (dWidth/2)) / dWidth;
				index = *iPtr++;
				phase = index & 1;
				p = index >> 1;
				if (phase) {
					d = (bPtr[p] + bPtr[p+1]) >> 1;
				} else {
					d = bPtr[p];
				}
				*dPtr++ = d;
				//dst[i] = d;
			}
		}
#else
		dPtr = dst;
		for (j = 0; j < dWidth; j++) {
			unsigned int p, d;
			//index = (2 * j * nW + (dWidth/2)) / dWidth;
			index = indexTable[j];
			phase = index & 1;
			p = index >> 1;
			if (phase) {
				d = (bPtr[p] + bPtr[p+1]) >> 1;
			} else {
				d = bPtr[p];
			}
			*dPtr++ = d;
			//dst[i] = d;
		}
#endif
		dst += dPitch;
	}
}
#else

STATIC void NxVScalerReduceComponent(NxVScalerStruct *nv, unsigned char *dst, unsigned char *src, int bChroma)
{
	unsigned int i, j, ii, jj, k;
	unsigned int c, nW, nH;
	unsigned char *s_ptr, *ptr;
	unsigned int R;
	unsigned char buffer[LINEBUF_PIXELS * 2];
	unsigned char *avgBuffer = nv->avgBuffer;
	unsigned short *indexTable = (bChroma) ? nv->indexTableC : nv->indexTableY;
	unsigned int wIRatio, hIRatio, minIRatio;
	unsigned int m, n, cW, cH;
	int pos, index, phase;
	unsigned char *dPtr, *bPtr, *cBuffer;
	unsigned int width, pitch, dWidth, dHeight, dPitch;
	unsigned int d_diff;

	
	width = nv->width;
	pitch = nv->pitch;

	dWidth = nv->dWidth;
	dHeight = nv->dHeight;
	dPitch = nv->dPitch;
	nW = nv->nW;
	nH = nv->nH;
	cW = nv->cW;
	cH = nv->cH;
	m = nv->m;
	n = nv->n;

	if (bChroma) {
		dWidth = (dWidth + 1) >> 1;
		dHeight = (dHeight + 1) >> 1;
		dPitch = (dPitch + 1) >> 1;
		width = (width + 1) >> 1;
		
		pitch = (pitch + 1) >> 1;

		nW = (nW + 1) >> 1;
		nH = (nH + 1) >> 1;
		indexTable = nv->indexTableC;
	} else {
		indexTable = nv->indexTableY;
	}



	d_diff = dPitch - dWidth;

	k = 0;
	for (i = 0; i < nH; i++) {
		s_ptr = src;
		cBuffer = &buffer[(i & 1) ? LINEBUF_PIXELS : 0];
		dPtr = cBuffer;
		for (j = 0; j < nW; j++) {
			R = 0;
			ptr = s_ptr;
			s_ptr += cW;
			for (ii = 0; ii < cH; ii++) {
				for (jj = 0; jj < cW; jj++) {
					R += *ptr++;
				}
				ptr += pitch - cW;
			}
#ifdef SEPARATE_HV
			R = R >> (m + n);
#else
			R = R >> (n << 1);
#endif			
			*dPtr++ = R;
		}
		src += pitch * cH;

		// 
		pos = (2 * i * dHeight / nH) - 2 * k;
		if (pos < 0)
			continue;
		k++;

		if (pos) {
			for (j = 0; j < nW; j++) {
				avgBuffer[j] = (buffer[j] + buffer[LINEBUF_PIXELS + j]) >> 1;
			}
			bPtr = avgBuffer;
		} else {
			bPtr = cBuffer;
		}


		for (j = 0; j < dWidth; j++) {
			//index = (2 * j * nW + (dWidth/2)) / dWidth;
			index = indexTable[j];
			phase = index & 1;
			pos = index >> 1;
			if (phase) {
				*dst++ = (bPtr[pos] + bPtr[pos+1]) >> 1;
			} else {
				*dst++ = bPtr[pos];
			}
		}
		dst += d_diff;
	}
}
#endif

void FUNC_ARM(NxVScalerClose)(NxVScalerStruct *nv)
{
	free(nv->indexTableY);
	free(nv->posTableY);
	free(nv->indexTableC);
	free(nv->posTableC);
	free(nv->avgBuffer);
	free(nv);
}

void FUNC_ARM(NxVScalerEnlargeYUV)(NxVScalerStruct *nv, unsigned char *dY, unsigned char *dU, unsigned char *dV,
				unsigned char *sY, unsigned char *sU, unsigned char *sV)
{
	NxVScalerEnlargeComponent(nv, dY, sY, 0);
	NxVScalerEnlargeComponent(nv, dU, sU, 1);
	NxVScalerEnlargeComponent(nv, dV, sV, 1);
}

void FUNC_ARM(NxVScalerResizeYUV)(NxVScalerStruct *nv, unsigned char *dY, unsigned char *dU, unsigned char *dV,
				unsigned char *sY, unsigned char *sU, unsigned char *sV)
{
	if (nv->mode == 2) {
		NxVScalerEnlargeComponent(nv, dY, sY, 0);
		NxVScalerEnlargeComponent(nv, dU, sU, 1);
		NxVScalerEnlargeComponent(nv, dV, sV, 1);
	} else {
		NxVScalerReduceComponent(nv, dY, sY, 0);
		NxVScalerReduceComponent(nv, dU, sU, 1);
		NxVScalerReduceComponent(nv, dV, sV, 1);
	}
}
