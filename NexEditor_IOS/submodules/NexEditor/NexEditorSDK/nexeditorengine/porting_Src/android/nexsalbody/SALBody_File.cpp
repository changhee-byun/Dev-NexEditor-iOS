#define LOG_TAG "NEXEDITOR"
//#include <utils/Log.h>

#include "SALBody_File.h"
#include "SALBody_Debug.h"
#include "SALBody_Mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

//namespace android {

#define NEXFT_SHAEDFD 0x10
#define NEXFT_WRITEFD 0x20
int g_FileResourceCount = 0;

typedef struct {
	int m_FD;
	NXINT64		m_StartOffset;
	NXINT64		m_Length;
	NXINT64		m_CurOffset;
	int m_FileType;
	unsigned int m_UserData;
}NexADFileHandle;	

#define  MAX_OUTPUTFD 16
static int s_outPutFDList[MAX_OUTPUTFD];
static int s_init_outPutFDList;

static int s_appAssetFD = -1;

int regAssetFD(int fd)
{
	unregAssetFD(-1);
	s_appAssetFD = dup(fd);
	return s_appAssetFD;
}

int unregAssetFD(int fd)
{
	if( s_appAssetFD >= 0 ){
		close(s_appAssetFD);
	}
	s_appAssetFD = -1;
	return 0;
}

int regOutPutFD(int fd)
{
    if( s_init_outPutFDList == 0 )
    {
        for(int i = 0 ; i < MAX_OUTPUTFD ; i++ )
        {
            s_outPutFDList[i] = -1;
        }
        s_init_outPutFDList = 1;
    }

    for(int i = 0 ; i < MAX_OUTPUTFD ; i++ )
    {
        if( s_outPutFDList[i] == -1 )
        {
            s_outPutFDList[i] = dup(fd);
            return s_outPutFDList[i];
        }
    }
    return -1;
}

int unregOutPutFD(int fd )
{
    if( s_init_outPutFDList == 0 )
    {
        return -1;
    }
    
    for(int i = 0 ; i < MAX_OUTPUTFD ; i++ )
    {
        if( s_outPutFDList[i] == fd )
        {
            close(fd);
            s_outPutFDList[i] = -1;
            return 0;
        }
    }
    return -1;
}

NEXSALFileHandle nexSALBody_FileOpen( char * pFilename, NEXSALFileMode iMode )
{
	int fd, newfd;
	long long offset = 0;
	long long len = 0;
	int flags = 0;
	int filetype = 0;
	unsigned int userdata = 0;
	NexADFileHandle * pHandle;
	
	if ( iMode & NEXSAL_FILE_CREATE )
	{
		flags |= O_CREAT;
		flags |= O_TRUNC;
	}		
	if ( ( iMode & NEXSAL_FILE_READ ) && !(iMode & NEXSAL_FILE_WRITE) )
	{
		flags |= O_RDONLY;
	}
	else if ( !( iMode & NEXSAL_FILE_READ ) && (iMode & NEXSAL_FILE_WRITE) )
	{
		flags |= O_WRONLY;
	}
	else
	{
		flags |= O_RDWR;
	}
	
	if (sscanf(pFilename, "sharedfd://%d:%lld:%lld", &fd, &offset, &len) == 3)
	{
		newfd = dup(fd);
		fd = newfd;
		filetype |= NEXFT_SHAEDFD;
//		filetype |= NEXFT_WRITEFD;

		if( len == 0x7ffffffffffffffL )
		{
			nexSALBody_DebugPrintf("[nexSALBody_FileOpen 451] length is -1...");
					
			struct stat sb;
			int ret = fstat(fd, &sb);
			len = sb.st_size;
		}
	}
	else if(sscanf(pFilename, "writefd://%d", &fd) == 1  )
	{
	    nexSALBody_DebugPrintf("[nexSALBody_FileOpen 451] write mode...");
   		newfd = dup(fd);
		fd = newfd;
	}
	else if( sscanf(pFilename, "nexasset://%d:%lld:%lld",&fd,&offset, &len) == 3)
	{
		if( s_appAssetFD < 0 ){
			fd = -1;
			nexSALBody_DebugPrintf("[nexSALBody_FileOpen ]%s: not open appAssetFD.");
		}else{
			fd = dup(s_appAssetFD);
			filetype |= NEXFT_SHAEDFD;
			if( len == 0x7ffffffffffffffL )
			{
				nexSALBody_DebugPrintf("[nexSALBody_FileOpen ] length is -1...");
					
				struct stat sb;
				int ret = fstat(fd, &sb);
				len = sb.st_size;
			}
		}
	}	
	else
	{
		umask(000);
		fd = open( pFilename, flags, 0666 );
	}
	
	if( fd == -1 )
	{
	    nexSALBody_DebugPrintf("nexSALBody_FileOpen(%s) open fail! flags(%x), errno(%d)",pFilename,flags,errno);
		return 0;
	}

	pHandle = (NexADFileHandle*)malloc(sizeof(NexADFileHandle) );
	if( pHandle == NULL )
	{
		nexSALBody_DebugPrintf("nexSALBody_FileOpen(%s) MemAlloc fail!",pFilename);
		close(fd);
		return 0;
	}
	
	g_FileResourceCount++;
	
	//lseek64(fd, offset , SEEK_SET);
	pHandle->m_FD = fd;
	pHandle->m_StartOffset = offset;
	pHandle->m_CurOffset = 0;
	pHandle->m_Length = len;
	pHandle->m_FileType = filetype;
	pHandle->m_UserData = userdata;

	return (NEXSALFileHandle)pHandle;
}

int nexSALBody_FileClose( NEXSALFileHandle hFile )
{
	int rval = -1;
	NexADFileHandle *pHandle;

	pHandle = (NexADFileHandle*)hFile;
	
	rval = close(pHandle->m_FD);

	free(pHandle);
	
	g_FileResourceCount--;
	return rval;
}

int nexSALBody_FileRead( NEXSALFileHandle hFile, void *pBuf, unsigned int uiBytesToRead )
{
	NexADFileHandle *pHandle;
	long long remain;
	int nRead = 0 ;

	pHandle = (NexADFileHandle*)hFile;

	if( pHandle->m_FileType & NEXFT_SHAEDFD )
	{
		if (pHandle->m_CurOffset >= pHandle->m_Length)
			return 0;
	
		remain = pHandle->m_Length - pHandle->m_CurOffset;
	
		if( remain < (long long)uiBytesToRead )
			uiBytesToRead = (unsigned int)remain;
	
		int iReadCount = 0;
		do {
			if( iReadCount > 0 )
				usleep( 1000 );
			lseek64(pHandle->m_FD, pHandle->m_CurOffset + pHandle->m_StartOffset, SEEK_SET);
			nRead = read(pHandle->m_FD, pBuf, uiBytesToRead);
			remain = lseek64(pHandle->m_FD, 0, SEEK_CUR);
			iReadCount++;
		} while (  (pHandle->m_CurOffset + pHandle->m_StartOffset + nRead) != remain && iReadCount < 10 );

		if( iReadCount > 1 )
		{
			nexSALBody_DebugPrintf("nexSALBody_FileRead(): Seek is interrupted by other thread(s), RE-READ %d Times", iReadCount );
		}

		if (remain >= 0)
		{
			pHandle->m_CurOffset = remain - pHandle->m_StartOffset;
		}
	
		return nRead;
	}	
		
	return read(pHandle->m_FD, pBuf, uiBytesToRead);
}

int nexSALBody_FileWrite( NEXSALFileHandle hFile, void *pBuf, unsigned int uiBytesToWrite )
{
	NexADFileHandle *pHandle;
	int size = 0;
	
	pHandle = (NexADFileHandle*)hFile;
	
	if( pHandle->m_FileType & NEXFT_SHAEDFD )
	{
		//TODO: 
	}
			
	size = write( pHandle->m_FD, pBuf, uiBytesToWrite );

	if(size == -1)
	{
		nexSALBody_DebugPrintf("nexSALBody_FileWrite(): written size is different %d, %d", size, uiBytesToWrite );
		size = write( pHandle->m_FD, pBuf, uiBytesToWrite );
	}
	
	return size;
}

int nexSALBody_FileSeek( NEXSALFileHandle hFile, int iOffset, NEXSALFileSeekOrigin iOrigin )
{
	NexADFileHandle *pHandle;
	int uPos = 0;
	int whence;

	pHandle = (NexADFileHandle*)hFile;
	

	if( pHandle->m_FileType & NEXFT_SHAEDFD )
	{	
		uPos = (int)pHandle->m_CurOffset;
		if(iOrigin == NEXSAL_SEEK_BEGIN)
		{
			uPos = iOffset;
		}
		else if(iOrigin == NEXSAL_SEEK_CUR)
		{
			uPos = uPos + iOffset;
		}
		else if(iOrigin == NEXSAL_SEEK_END)
		{
			if( pHandle->m_Length == -1 )
				uPos = (int)iOffset; // TODO : new end pos
			else	
				uPos = (int)pHandle->m_Length + iOffset;
		}
		
		if( uPos < 0 )
			uPos = 0;
		
		if( uPos > pHandle->m_Length )
			uPos = (int)pHandle->m_Length;
		
		pHandle->m_CurOffset = uPos;
		return uPos;
	}		

	if(iOrigin == NEXSAL_SEEK_BEGIN)
	{
			whence = SEEK_SET;
	}
	else if(iOrigin == NEXSAL_SEEK_CUR)
	{
			whence = SEEK_CUR;
	}
	else if(iOrigin == NEXSAL_SEEK_END)
	{
		whence = SEEK_END;
	}

	return lseek(pHandle->m_FD, iOffset, whence);
}

NXINT64 nexSALBody_FileSeek64(NEXSALFileHandle hFile, NXINT64 iOffset, NEXSALFileSeekOrigin iOrigin)
{
	NexADFileHandle *pHandle;
	long long uPos = 0;
	int whence;

	pHandle = (NexADFileHandle*)hFile;


	if( pHandle->m_FileType & NEXFT_SHAEDFD )
	{	
		uPos = pHandle->m_CurOffset;
		if(iOrigin == NEXSAL_SEEK_BEGIN)
		{
			uPos = iOffset;
		}
		else if(iOrigin == NEXSAL_SEEK_CUR)
		{
			uPos = uPos + iOffset;
		}
		else if(iOrigin == NEXSAL_SEEK_END)
		{
			if( pHandle->m_Length == -1 )
				uPos = iOffset; // TODO : new end pos
			else	
				uPos = pHandle->m_Length + iOffset;
		}
		
		if( uPos < 0 )
			uPos = 0;
		
		if( uPos > pHandle->m_Length )
			uPos = pHandle->m_Length;
		
		pHandle->m_CurOffset = uPos;
		return uPos;
	}		
	
	if(iOrigin == NEXSAL_SEEK_BEGIN)
	{
			whence = SEEK_SET;
	}
	else if(iOrigin == NEXSAL_SEEK_CUR)
	{
			whence = SEEK_CUR;
	}
	else if(iOrigin == NEXSAL_SEEK_END)
	{
		whence = SEEK_END;
	}


	return lseek64(pHandle->m_FD, iOffset, whence);
}	

int nexSALBody_FileSize(NEXSALFileHandle hFile)
{
	NexADFileHandle *pHandle;
	NXINT64 nCurrentPos;
	NXINT64 nSize;	
	
	pHandle = (NexADFileHandle*)hFile;	
	if( pHandle->m_FileType & NEXFT_SHAEDFD ){
		return pHandle->m_Length;
	}
	nCurrentPos = nexSALBody_FileSeek64(hFile, 0, NEXSAL_SEEK_CUR);
	nSize = nexSALBody_FileSeek64(hFile, 0, NEXSAL_SEEK_END);
	nexSALBody_FileSeek64(hFile, nCurrentPos, NEXSAL_SEEK_BEGIN);

	return nSize;
}

int nexSALBody_FileRemove(char * pFilename)
{
	NEXSALFileHandle hSrc;


	if (unlink(pFilename) < 0) {
		nexSALBody_DebugPrintf((char*)"[SALBody_File.cpp %d] FileRemove error, %s", __LINE__, pFilename);	
		return -1;
	} else {
		//nexSALBody_DebugPrintf("in SALBody_FileRemove ok, %s", pFilename);
		return 0;
	}
}

//};
