#ifndef _SALBODY_H_INCLUDED_
#define _SALBODY_H_INCLUDED_

#include "NexSAL_API.h"
#include "NexSAL_Internal.h"

//namespace android {

#ifdef __cplusplus
extern "C" {
#endif

int regAssetFD(int fd);
int unregAssetFD(int fd);
int regOutPutFD(int fd);
int unregOutPutFD(int fd );

NEXSALFileHandle nexSALBody_FileOpen( char * pFilename, NEXSALFileMode iMode );
int nexSALBody_FileClose( NEXSALFileHandle hFile );
int nexSALBody_FileRead( NEXSALFileHandle hFile, void *pBuf, unsigned int uiBytesToRead );
int nexSALBody_FileWrite( NEXSALFileHandle hFile, void *pBuf, unsigned int uiBytesToWrite );
int nexSALBody_FileSeek( NEXSALFileHandle hFile, int iOffset, NEXSALFileSeekOrigin iOrigin );
NXINT64 nexSALBody_FileSeek64(NEXSALFileHandle hFile, NXINT64 iOffset, NEXSALFileSeekOrigin iOrigin);
int nexSALBody_FileSize(NEXSALFileHandle hFile);
int nexSALBody_FileRemove(char * pFilename);

#ifdef __cplusplus
}
#endif

//};

#endif //_SALBODY_H_INCLUDED_
