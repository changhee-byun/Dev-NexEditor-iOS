//
//  SALBody_File.h
//  nexSalBody
//
//  Created by Simon Kim on 1/26/15.
//
//

#ifndef nexSalBody_SALBody_File_h
#define nexSalBody_SALBody_File_h
#include "NexSAL_API.h"
#include "NexSAL_Internal.h"
#ifdef __cplusplus
extern "C" {
#endif
	
	
	NEXSALFileHandle nexSALBody_FileOpen( char * pFilename, NEXSALFileMode iMode );
	int nexSALBody_FileClose( NEXSALFileHandle hFile );
	int nexSALBody_FileRead( NEXSALFileHandle hFile, void *pBuf, unsigned int uiBytesToRead );
	int nexSALBody_FileWrite( NEXSALFileHandle hFile, void *pBuf, unsigned int uiBytesToWrite );
	NXINT32 nexSALBody_FileSeek( NEXSALFileHandle hFile, NXINT32 iOffset, NEXSALFileSeekOrigin iOrigin );
	NXINT64 nexSALBody_FileSeek64( NEXSALFileHandle hFile, NXINT64 iOffset, NEXSALFileSeekOrigin iOrigin );
	NXINT64 nexSALBody_FileSize(NEXSALFileHandle hFile);
	
	
#ifdef __cplusplus
}
#endif

#endif
