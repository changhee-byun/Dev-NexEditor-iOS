#ifndef _NEXNV12TILETONV12_H_
#define _NEXNV12TILETONV12_H_

#ifdef __cplusplus
extern "C" {
#endif

void convertNV12TiledToNV12(int width, int height, int wpitch, int hpitch, void *srcBits, void *dstBits);

#ifdef __cplusplus
}
#endif

#endif // _NEXNV12TILETONV12_H_

