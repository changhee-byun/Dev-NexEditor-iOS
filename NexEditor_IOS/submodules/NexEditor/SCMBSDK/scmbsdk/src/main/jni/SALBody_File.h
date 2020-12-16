#ifndef _SALBODY_H_INCLUDED_
#define _SALBODY_H_INCLUDED_

#include "NexSAL_Com.h"

//namespace android {

#ifdef __cplusplus
extern "C" {
#endif

int regAssetFD(int fd);
int unregAssetFD(int fd);
int regOutPutFD(int fd);
int unregOutPutFD(int fd );

NXVOID * nexSALBody_FileOpen( NXCHAR * pFilename, NEXSALFileMode iMode );
NXINT32 nexSALBody_FileClose( NXVOID* hFile );
NXSSIZE nexSALBody_FileRead( NXVOID* hFile, NXINT8* pBuf, NXUSIZE uiBytesToRead );
NXSSIZE nexSALBody_FileWrite( NXVOID* hFile, NXINT8*pBuf, NXUSIZE uiBytesToWrite );
NXINT32 nexSALBody_FileSeek( NXVOID* hFile, NXINT32 iOffset, NXUINT32 iOrigin );
NXINT64 nexSALBody_FileSeek64(NXVOID* hFile, NXINT64 iOffset, NXUINT32 iOrigin);
NXINT64 nexSALBody_FileSize(NXVOID* hFile);
int nexSALBody_FileRemove(char * pFilename);

#ifdef __cplusplus
}
#endif

//};

#endif //_SALBODY_H_INCLUDED_
