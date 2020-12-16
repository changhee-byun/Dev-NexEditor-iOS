/*
 *  SALBody_Sock.c
 *  nexSalBody
 *
 *  Created by Sunghyun Yoo on 08. 12. 26.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#include "SALBody_Sock.h"
#include "SALBody_Debug.h"
#include "SALBody_Task.h"
#include "SALBody_Time.h"			// JDKIM 2011/05/13
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>

/* SWSEO 2010/09/14 OpenSSL Headers and values */
#include "openssl/bio.h"
#include "openssl/err.h"
#include "openssl/ssl.h"

typedef struct
{
	SSL_CTX *ctx;
	SSL *ssl;

} SSLInfo;

typedef struct
{
	NEXSALSockHandle	hSock;

	int				isUseSSL;
	SSLInfo			infoSSL;

	int				isReceiving;				// JDKIM 2011/05/13
	int				isClosing;				// JDKIM 2011/05/13
	int				isConnecting;			// JDKIM 2011/05/20
} SYSSocketInfo;
#define __SINGLE_SSL_SOCK_

#define SOCKET_INVALID  (-1)
#define MAX_SYSSOCK		20

#ifndef __SINGLE_SSL_SOCK_	// SWSEO 2010/10/07
static SYSSocketInfo	g_infoSysSock[MAX_SYSSOCK];
#else
static SYSSocketInfo	g_infoSysSock;
#endif

static int				g_iSockCreateCount = 0;
/* SWSEO 2010/09/14 OpenSSL Headers and values end*/

//namespace android {

//static int g_socket_fd_max = 0;
static int g_iCloseMode = 0;

//void nexSALBody_Sock_Init()
//{
//	g_socket_fd_max = 0;
//}

#ifndef __SINGLE_SSL_SOCK_	// SWSEO 2010/10/07
/* SWSEO 2010/09/15 added SockInfo 1 */
static void SysInitSockInfo( void )
{
	if( !g_iSockCreateCount )
	{
		int nCnt = 0;
		memset( g_infoSysSock, 0x00, sizeof(SYSSocketInfo)*MAX_SYSSOCK );
		for(; nCnt < MAX_SYSSOCK; nCnt++)
		{
			g_infoSysSock[nCnt].hSock = SOCKET_INVALID;
		}
	}
}

static int SysGetSocketInfo( int hSock )
{
	int nCnt = 0;
	do
	{
		if( g_infoSysSock[nCnt].hSock == hSock )
		{
			return nCnt;
		}
		nCnt++;
	} while ( nCnt < MAX_SYSSOCK );

	return -1;
}
/* SWSEO 2010/09/15 added SockInfo 1 end*/
#else
static void SysInitSockInfo( void )
{
	//if( !g_iSockCreateCount )
	{
		memset( &g_infoSysSock, 0x00, sizeof(SYSSocketInfo) );
		g_infoSysSock.hSock = SOCKET_INVALID;
	}
}

static int SysGetSocketInfo( int hSock )
{
	if( g_infoSysSock.hSock == hSock )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
#endif // SWSEO 2010/10/07

void nexSALBody_Sock_SetCloseMode( int iMode )
{
	g_iCloseMode = iMode;
}

NEXSALSockHandle nexSALBody_SockCreate( int iType )
{
	int sock_fd;

#ifndef __SINGLE_SSL_SOCK_	// SWSEO 2010/10/07
	/* SWSEO 2010/09/15 added SSL 1 */
	int iIndex;

#ifndef __SINGLE_SSL_SOCK_
	SysInitSockInfo();
	iIndex = SysGetSocketInfo( SOCKET_INVALID );
	if ( iIndex < 0 )
	{
		nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SysGetSockInfo is null, No Socket ",__LINE__);
		return -1;
	}
#endif	// __SINGLE_SSL_SOCK_

	if( iType & NEXSAL_SOCK_SSL )	 // SWSEO 2010/09/15
	{
		int result = 0;
		SSL_METHOD *method;
		SSL_CTX *ctx;

		result = SSL_library_init();

		if( result == 1 )
		{
			OpenSSL_add_ssl_algorithms();
			SSL_load_error_strings();
			method = (SSL_METHOD *)TLSv1_client_method();
			if( method == NULL)
			{
				nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] TLSv1_client_method() fail :0x%x\n",__LINE__,method);
			}
			ctx = SSL_CTX_new(method);
			if( ctx == NULL )
			{
				nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SSL_CTX_new() fail :0x%x\n",__LINE__,ctx);
			}

			iType &= 0x1;
#ifndef __SINGLE_SSL_SOCK_	
			g_infoSysSock[iIndex].infoSSL.ctx = ctx;
			g_infoSysSock[iIndex].isUseSSL = 1;
			g_infoSysSock[iIndex].isReceiving = 0;				// JDKIM 2011/05/13
			g_infoSysSock[iIndex].isClosing = 0;				// JDKIM 2011/05/13
			g_infoSysSock[iIndex].infoSSL.ssl = 0;			// JDKIM 2011/05/20
#else
			g_infoSysSock.infoSSL.ctx = ctx;
			g_infoSysSock.isUseSSL = 1;
			g_infoSysSock.isReceiving = 0;				// JDKIM 2011/05/13
			g_infoSysSock.isClosing = 0;				// JDKIM 2011/05/13
			g_infoSysSock.infoSSL.ssl = 0;			// JDKIM 2011/05/20			
#endif	// __SINGLE_SSL_SOCK_
		}
		else
		{
			nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SSL_library_init fail :%d\n",__LINE__,result);
		}
	}
	/* SWSEO 2010/09/15 added SSL 1 end */
#else
	int iCheckSockInfo;

	SysInitSockInfo();
	iCheckSockInfo = SysGetSocketInfo( SOCKET_INVALID );
	if ( iCheckSockInfo < 0 )
	{
		nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SysGetSockInfo is null, No Socket ",__LINE__);
		return -1;
	}
	if( iType & NEXSAL_SOCK_SSL )	 // SWSEO 2010/09/15
	{
		int result = 0;
		SSL_METHOD *method;
		SSL_CTX *ctx;

		result = SSL_library_init();

		if( result == 1 )
		{
			OpenSSL_add_ssl_algorithms();
			SSL_load_error_strings();
			method = (SSL_METHOD *)TLSv1_client_method();
			if( method == NULL)
			{
				nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] TLSv1_client_method() fail :0x%x\n",__LINE__,method);
			}
			ctx = SSL_CTX_new(method);
			if( ctx == NULL )
			{
				nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SSL_CTX_new() fail :0x%x\n",__LINE__,ctx);
			}

			iType &= 0x1;
			g_infoSysSock.infoSSL.ctx = ctx;
			g_infoSysSock.isUseSSL = 1;
			g_infoSysSock.isReceiving = 0;				// JDKIM 2011/05/13
			g_infoSysSock.isClosing = 0;				// JDKIM 2011/05/13
		}
		else
		{
			nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SSL_library_init fail :%d\n",__LINE__,result);
		}
	}
#endif // SWSEO 2010/10/07

#if 0	// SWSEO 2010/09/15 for Test. 1 is original code.
	if ( iType == NEXSAL_SOCK_STREAM )
	{
		sock_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	}
	else
	{
		sock_fd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	}
#else
	if ( iType == NEXSAL_SOCK_STREAM )
	{
		iType = SOCK_STREAM;
	}
	else
	{
		iType = SOCK_DGRAM;
	}
	sock_fd = socket( AF_INET, iType, IPPROTO_IP);

	{
		int set  = 1024000; // 1MBytes
		int size = sizeof(set);
		setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&set, size);
	}
#endif // test end.
	
//	g_socket_fd_max = ( ( g_socket_fd_max > sock_fd ) ? g_socket_fd_max : sock_fd );
	
	//nexSALBody_DebugPrintf("SocketCreate:%d\n",sock_fd);

#ifndef __SINGLE_SSL_SOCK_
	/* SWSEO 2010/09/15 added SSL 2 */
	g_infoSysSock[iIndex].hSock = sock_fd;
	g_iSockCreateCount++;
	/* SWSEO 2010/09/15 added SSL 2 end */
#else
	g_infoSysSock.hSock = sock_fd;
	g_iSockCreateCount++;
#endif

	if(sock_fd < 0 )
	{
		nexSALBody_DebugPrintf("SockeCreate Error:%d\n", errno);
	}
	return sock_fd;
}

int nexSALBody_SockClose( NEXSALSockHandle hSock )
{
#ifndef __SINGLE_SSL_SOCK_
	/* SWSEO 2010/09/15 added SSL 1 */
	int iIndex;
	if( hSock != -1)
	{
		iIndex = SysGetSocketInfo( hSock );

		if( iIndex >= 0 && g_infoSysSock[iIndex].isUseSSL )
		{
			g_infoSysSock[iIndex].isClosing = 1;			// JDKIM 2011/05/13

			if(g_infoSysSock[iIndex].infoSSL.ssl)		// JDKIM 2011/05/20
			{
				SSL_shutdown( g_infoSysSock[iIndex].infoSSL.ssl );
			}
			
			// JDKIM 2011/05/13
			unsigned int uiWaitStart = nexSALBody_GetTickCount();
			while(g_infoSysSock.isReceiving)
			{
				nexSALBody_TaskSleep( 10);

				if(nexSALBody_GetTickCount() - uiWaitStart > 1000)
				{
					nexSAL_DebugPrintf("[nexSALBody_SockClose %d] Wait receive function retun timeout.\n", __LINE__);
					break;
				}
			}

			if(g_infoSysSock.isReceiving == 0)
			{
				if(g_infoSysSock[iIndex].infoSSL.ssl)		// JDKIM 2011/05/20
				{
					SSL_free( g_infoSysSock[iIndex].infoSSL.ssl );
				}
				SSL_CTX_free( g_infoSysSock[iIndex].infoSSL.ctx );
			}
				// JDKIM : end
			
			g_infoSysSock[iIndex].isUseSSL = 0;
			g_infoSysSock[iIndex].isClosing = 0;			// JDKIM 2011/05/13
		}

	}
	/* SWSEO 2010/09/15 added SSL 1 end */
#else
	int iCheckSockInfo;

	//nexSAL_DebugPrintf("[nexSALBody_SockClose %d] 1. hSock:%d\n", __LINE__, hSock);
	
	if( hSock != -1)
	{
		iCheckSockInfo = SysGetSocketInfo( hSock );

		//nexSAL_DebugPrintf("[nexSALBody_SockClose %d] 2. hSock:%d iCheckSockInfo\n", __LINE__, hSock, iCheckSockInfo);

		if( (iCheckSockInfo == 0) && g_infoSysSock.isUseSSL )
		{
			g_infoSysSock.isClosing = 1;			// JDKIM 2011/05/13

			//nexSAL_DebugPrintf("[nexSALBody_SockClose %d] 3. SSL(%d)\n", __LINE__);

			if(g_infoSysSock.infoSSL.ssl)		// JDKIM 2011/05/20
			{
				SSL_shutdown( g_infoSysSock.infoSSL.ssl );
			}

			//nexSAL_DebugPrintf("[nexSALBody_SockClose %d] 4.\n", __LINE__);

			// JDKIM 2011/05/13
			unsigned int uiWaitStart = nexSALBody_GetTickCount();
			while(g_infoSysSock.isReceiving)
			{
				nexSALBody_TaskSleep( 10);

				if(nexSALBody_GetTickCount() - uiWaitStart > 1000)
				{
					nexSAL_DebugPrintf("[nexSALBody_SockClose %d] Wait receive function retun timeout.\n", __LINE__);
					break;
				}
			}

			uiWaitStart = nexSALBody_GetTickCount();
			while(g_infoSysSock.isConnecting)
			{
				nexSALBody_TaskSleep( 10);

				if(nexSALBody_GetTickCount() - uiWaitStart > 1000)
				{
					nexSAL_DebugPrintf("[nexSALBody_SockClose %d] Wait connect function retun timeout.\n", __LINE__);
					break;
				}
			}

			//nexSAL_DebugPrintf("[nexSALBody_SockClose %d] 5.\n", __LINE__);

			if(g_infoSysSock.isReceiving == 0)
			{
				if(g_infoSysSock.infoSSL.ssl && g_infoSysSock.isConnecting == 0)		// JDKIM 2011/05/20
				{
					SSL_free( g_infoSysSock.infoSSL.ssl );
				}
				SSL_CTX_free( g_infoSysSock.infoSSL.ctx );
			}
			// JDKIM : end

			//nexSAL_DebugPrintf("[nexSALBody_SockClose %d] 6.\n", __LINE__);
			
			g_infoSysSock.isUseSSL = 0;
			g_infoSysSock.isClosing = 0;			// JDKIM 2011/05/13
		}
	}
#endif

	close( hSock );

	/* SWSEO 2010/09/15 added SSL 2 */
	//nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] nexSALBody_SockClose - g_iSockCreateCount:%d\n",__LINE__,g_iSockCreateCount);
	if( g_iSockCreateCount )
	{
		g_iSockCreateCount--;
		//nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] nexSALBody_SockClose - g_iSockCreateCount:%d\n",__LINE__,g_iSockCreateCount);
	}
	/* SWSEO 2010/09/15 added SSL 2 end */

	return 0;
}

static int g_isDomain = FALSE;
static char g_szDomain[256] = {0,};
static unsigned int g_inet_addr;

int _getaddress( struct sockaddr_in *psin, char *pAddr, unsigned short wPort )
{
	struct hostent *hp;

	bzero((char*)psin, sizeof(struct sockaddr_in));
	psin->sin_family = AF_INET;
	psin->sin_port = htons(wPort);
	//nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] nexSALBody_SockConnect 1 pAddr[%s], wPort[%d]\n", __LINE__, pAddr, wPort);

	if ( g_isDomain && strcmp(g_szDomain,pAddr) == 0 )
	{
		bcopy( &g_inet_addr, (char*)&psin->sin_addr, sizeof(g_inet_addr) );
		return 0;
	}
	//nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] nexSALBody_SockConnect 2 \n", __LINE__);
	g_isDomain = FALSE;
	
	if ( ( hp = gethostbyname( pAddr )) != NULL )
	{
		bcopy( hp->h_addr, (char *)&psin->sin_addr, hp->h_length );
		bcopy( hp->h_addr, &g_inet_addr, 4 );
		strcpy( g_szDomain, pAddr );
		g_isDomain = TRUE;
	}
	else
	{
		psin->sin_addr.s_addr = inet_addr( pAddr );
		if ( psin->sin_addr.s_addr == -1 )
			return -1;
	}
	//nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] nexSALBody_SockConnect 4\n", __LINE__);
	return 0;
}

//#include "SALBody_task.h"


int nexSALBody_SockConnect( NEXSALSockHandle hSock, char *pAddr, unsigned short wPort, unsigned int dwTimeout )
{
	int iRet;
	struct sockaddr_in sin;

	//nexSALBody_DebugPrintf("SockConnect 1\n");

	if ( _getaddress( &sin, pAddr, wPort ) != 0 )
		return -1;
	//nexSALBody_DebugPrintf("SockConnect 2\n");
/*	
	int nT1;
	struct timeval tv;
	int nSize = sizeof(tv);//0;//sizeof(nT1);
	iRet = getsockopt( hSock, SOL_SOCKET, SO_SNDTIMEO, &tv, &nSize);
	printf("getsockopt - SO_SNDTIMEO ret : %d, errno=%d, tv.tv_sec=%d, tv.tv_usec=%d, nSize=%d\n",iRet,errno,tv.tv_sec,tv.tv_usec,nSize);
	
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	iRet = setsockopt( hSock, SOL_SOCKET, SO_SNDTIMEO, &tv, nSize );
	printf("setsockopt iRet = %d, errno=%d\n", iRet, errno);

	iRet = getsockopt( hSock, SOL_SOCKET, SO_SNDTIMEO, &tv, &nSize);
	printf("getsockopt - SO_SNDTIMEO ret : %d, errno=%d, tv.tv_sec=%d, tv.tv_usec=%d, nSize=%d\n",iRet,errno,tv.tv_sec,tv.tv_usec,nSize);
////
	
	iRet = getsockopt( hSock, SOL_SOCKET, SO_RCVTIMEO, &tv, &nSize);
	printf("getsockopt - SO_SNDTIMEO ret : %d, errno=%d, tv.tv_sec=%d, tv.tv_usec=%d, nSize=%d\n",iRet,errno,tv.tv_sec,tv.tv_usec,nSize);
	
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	iRet = setsockopt( hSock, SOL_SOCKET, SO_RCVTIMEO, &tv, nSize );
	printf("setsockopt iRet = %d, errno=%d\n", iRet, errno);
	
	iRet = getsockopt( hSock, SOL_SOCKET, SO_RCVTIMEO, &tv, &nSize);
	printf("getsockopt - SO_SNDTIMEO ret : %d, errno=%d, tv.tv_sec=%d, tv.tv_usec=%d, nSize=%d\n",iRet,errno,tv.tv_sec,tv.tv_usec,nSize);
	
	
//	iRet = getsockopt( hSock, SOL_SOCKET, SO_SNDTIMEO, &nT1, sizeof(nT1));
//	printf("getsockopt - SO_SNDTIMEO ret : %d, errno=%d\n",iRet,errno);
	O_NONBLOCK
*/	
	//nexSALBody_DebugPrintf("dwTimeout is %d\n",dwTimeout);

	int value = fcntl(hSock, F_GETFL, 0);
	//nexSALBody_DebugPrintf("socket's fcntl value is %X\n", value);
	fcntl(hSock, F_SETFL, value|O_NONBLOCK );

	int nRemainedmSec = dwTimeout;
	
	for ( int i = 0 ; nRemainedmSec > 0 ; i++ )
	{
		iRet = connect( hSock, (struct sockaddr *)&sin, sizeof(sin) );
		//nexSALBody_DebugPrintf("SockConnect - connect ret : %d, errno=%d\n",iRet,errno);
		
		if ( iRet == 0 )
			break;
		if ( errno == EISCONN )
		{
			iRet = 0;
			break;
		}
		
		if ( g_iCloseMode )
		{
			iRet = -1;
			break;
		}
		
		if ( errno != EINPROGRESS && errno != EAGAIN && errno != EALREADY && errno != ETIMEDOUT )		// JDKIM 2010/11/18
			break;
		
		nexSALBody_TaskSleep( 100);//500 );		// JDKIM 2010/10/14
		nRemainedmSec -= 100;//500;				// JDKIM 2010/10/14
	}

	// JDKIM 2010/10/14
	if(iRet != 0)
	{
		nexSALBody_DebugPrintf("SockConnect - connect ret : %d, errno=%d(%s) Time(%d/%d)\n",iRet,errno, strerror( errno ), nRemainedmSec, dwTimeout);
		return iRet;
	}
	else
	{
		nexSALBody_DebugPrintf("SockConnect - connect ret : %d\n",iRet);
	}
	// JDKIM : end
	
#ifndef __SINGLE_SSL_SOCK_
	/* SWSEO 2010/09/15 added SSL 1 */

	fcntl(hSock, F_SETFL, value );

	int iIdx = SysGetSocketInfo( hSock );
	if( iIdx >= 0 && g_infoSysSock[iIdx].isUseSSL )
	{
		int iReturn;
		BIO *sbio;

		g_infoSysSock[iIdx].infoSSL.ssl = SSL_new(g_infoSysSock[iIdx].infoSSL.ctx);      /* create new SSL connection state */
		sbio = BIO_new_socket(hSock,BIO_NOCLOSE);
		SSL_set_bio(g_infoSysSock[iIdx].infoSSL.ssl,sbio,sbio);

		iReturn = SSL_connect(g_infoSysSock[iIdx].infoSSL.ssl);
		if( iReturn != 1)
		{
			nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SSL_connect failed : iReturn[%d]\n", __LINE__,iReturn);
		}
		else
		{
			iRet = 0;
			nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SSL_connect success : iReturn[%d] 1 is success \n", __LINE__,iReturn);
		}
	}
	/* SWSEO 2010/09/15 added SSL 1 end */
#else
	fcntl(hSock, F_SETFL, value );

	int iCheckSockInfo = SysGetSocketInfo( hSock );

	nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] iCheckSockInfo:%d\n", __LINE__, iCheckSockInfo);
	
	if( (iCheckSockInfo == 0) && g_infoSysSock.isUseSSL )
	{
		int iReturn;
		BIO *sbio;

		g_infoSysSock.infoSSL.ssl = SSL_new(g_infoSysSock.infoSSL.ctx);      /* create new SSL connection state */


		//nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] g_infoSysSock.infoSSL.ssl:0x%X\n", __LINE__, g_infoSysSock.infoSSL.ssl);		
		
		sbio = BIO_new_socket(hSock,BIO_NOCLOSE);

		//nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] sbio:0x%X\n", __LINE__, sbio);		
		
		SSL_set_bio(g_infoSysSock.infoSSL.ssl,sbio,sbio);

		//nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] after SSL_set_bio\n", __LINE__);		

		// JDKIM 2011/05/20
		if(g_infoSysSock.isClosing == 0)
		{
			g_infoSysSock.isConnecting = 1;
			iReturn = SSL_connect(g_infoSysSock.infoSSL.ssl);
			g_infoSysSock.isConnecting = 0;
		}
		else
		{
			iReturn = -1;
		}
		// JDKIM : end
		
		if( iReturn != 1)
		{
			nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SSL_connect failed : iReturn[%d]\n", __LINE__,iReturn);
		}
		else
		{
			iRet = 0;
			nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] SSL_connect success : iReturn[%d] 1 is success \n", __LINE__,iReturn);
		}
	}
#endif
	return iRet;
}

int nexSALBody_SockBind( NEXSALSockHandle hSock, unsigned short wPort )
{
	struct sockaddr_in sin;
	int iRet;
	
	//setsockopt( hSock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
	
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl( INADDR_ANY );
	sin.sin_port = htons( wPort );
	
	iRet = bind( hSock, (struct sockaddr*)&sin, sizeof(sin) );
	//printf("Setsockopt return is %d\n",setsockopt( hSock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)));
	if ( iRet != 0 )
		nexSALBody_DebugPrintf("bsd socket error - bind %d, %d(%s)\n", iRet, errno, strerror( errno ) );
	
	
	return iRet;
}

int nexSALBody_SockSelect( NEXSALFDSet* pRSet, NEXSALFDSet* pWSet, NEXSALFDSet* pESet, NEXSALTimeValue* pTV )
{
	int i, iRet, sfd_max = 0;
	fd_set rs;
	NEXSALFDSet fdsetTemp;
	struct timeval tv;
	
	//nexSALBody_DebugPrintf("sock_select begin1");
	
	tv.tv_sec = pTV->tv_sec;
	tv.tv_usec = pTV->tv_usec;
	
	FD_ZERO( &rs );
	
	for ( i = 0 ; i < pRSet->fd_count ; i++ )
	{
		FD_SET( pRSet->fd_array[i], &rs );
		sfd_max = ( ( sfd_max > pRSet->fd_array[i] ) ? sfd_max : pRSet->fd_array[i] );
	}

	//nexSALBody_DebugPrintf("sock_select begin2 : %d, %d", pRSet->fd_count, sfd_max );
	iRet = select( sfd_max+1, &rs, NULL, NULL, &tv );
	//nexSALBody_DebugPrintf("sock_select begin3 : %d", iRet);
	

	if ( iRet > 0 )
	{
		fdsetTemp.fd_count = pRSet->fd_count;
		for ( i = 0 ; i < pRSet->fd_count ; i++ )
		{
			fdsetTemp.fd_array[i] = pRSet->fd_array[i];
		}
		
		NEXSAL_FD_ZERO( pRSet );
		
		for ( i = 0 ; i < fdsetTemp.fd_count ; i++ )
		{
			if ( FD_ISSET( fdsetTemp.fd_array[i], &rs ) )
			{
				NEXSAL_FD_SET( fdsetTemp.fd_array[i], pRSet )
			}
		}
	}
	else
	{
		NEXSAL_FD_ZERO( pRSet );
	}

	//nexSALBody_DebugPrintf("sock_select end");

	return iRet;
}

int nexSALBody_SockSendTo( NEXSALSockHandle hSock, char *pBuf, int iLen, char *pAddr, unsigned short wPort )
{
	struct sockaddr_in sin;
	
	if ( _getaddress( &sin, pAddr, wPort ) != 0 )
		return -1;
	
	return sendto( hSock, pBuf, iLen, 0, (struct sockaddr*)&sin, sizeof(sin) );
}

int nexSALBody_SockSend( NEXSALSockHandle hSock, char *pBuf, int iLen )
{
#if 0 // original SWSEO 2010/09/15
	return send( hSock, pBuf, iLen, 0 );
#else

#ifndef __SINGLE_SSL_SOCK_
	/* SWSEO 2010/09/15 added SSL 1 */
	int iSend;
	int iIndex = SysGetSocketInfo ( hSock );

	if( iIndex >= 0 && g_infoSysSock[iIndex].isUseSSL )
	{
		iSend = SSL_write( g_infoSysSock[iIndex].infoSSL.ssl, pBuf, iLen);
	}
	else
	{
		iSend = send(hSock, pBuf, iLen, 0);
	}

	if (!(iSend > 0))
	{
		nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] NetSend failed - SetSSL(%d)\n",__LINE__, g_infoSysSock[iIndex].isUseSSL);
		return	-1;
	}

	return	iSend;
	/* SWSEO 2010/09/15 added SSL 1 end */
#else
	int iSend;
	int iCheckSockInfo = SysGetSocketInfo ( hSock );

	if( (iCheckSockInfo == 0) && g_infoSysSock.isUseSSL )
	{
		iSend = SSL_write( g_infoSysSock.infoSSL.ssl, pBuf, iLen);
	}
	else
	{
		iSend = send(hSock, pBuf, iLen, 0);
	}

	if (!(iSend > 0))
	{
		nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] NetSend failed - SetSSL(%d)\n",__LINE__, g_infoSysSock.isUseSSL);
		return	-1;
	}

	return	iSend;
#endif // __SINGLE_SSL_SOCK_
#endif
}

int nexSALBody_SockRecvFrom( NEXSALSockHandle hSock, char *pBuf, int iLen, unsigned int *pdwAddr, unsigned short *pwPort, unsigned int dwTimeout )
{
	int ret;
	
	struct sockaddr_in sin;
	//struct sockaddr saddr;
	socklen_t len = sizeof(sin);

	//nexSALBody_DebugPrintf("sock_recvfrom begin");

	ret = recvfrom( hSock, pBuf, iLen, 0, (struct sockaddr*)&sin, &len );
	
	if ( pdwAddr )
		*pdwAddr = sin.sin_addr.s_addr;
	if ( pwPort )
		*pwPort = sin.sin_port;

	//nexSALBody_DebugPrintf("sock_recvfrom end");
	
	return ret;
}

int nexSALBody_SockRecv( NEXSALSockHandle hSock, char *pBuf, int iLen, unsigned int uTimeout )
{
	int iRet;
	int iReadBytes, i, iIndex;
	int iCheckSockInfo;
	fd_set rs;
	struct timeval tv;
	//nexSALBody_DebugPrintf("sockrecv_timeout:%u\n", uTimeout);
	tv.tv_sec = uTimeout/1000;
	tv.tv_usec = (uTimeout%1000)*1000;

	if ( hSock == -1 )
		return -1;
	
	FD_ZERO( &rs );
	FD_SET( hSock, &rs );
	
	//for ( iReadBytes = 0, i = 0 ; i < 5 && iReadBytes == 0 ; i++ )
	{
		//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] Select before. Sock(0x%X)\n", hSock);
		iRet = select( hSock+1, &rs, NULL, NULL, &tv );
		//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] Select after. Sock(0x%X), iRet(%d)\n", hSock, iRet);
		if ( iRet > 0 )
		{
			
#ifndef __SINGLE_SSL_SOCK_
			/* SWSEO 2010/09/16 added SSL */
			iIndex = SysGetSocketInfo( hSock );

			//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] SysGetSocketInfo after. Sock(0x%X), iIndex(%d)\n", hSock, iIndex);
			
			if ( iIndex >= 0 && g_infoSysSock[iIndex].isUseSSL )
			{
				// JDKIM 2011/05/13
				if(g_infoSysSock.isClosing == 0)
				{
					g_infoSysSock.isReceiving = 1;			
					iReadBytes = SSL_read(g_infoSysSock[iIndex].infoSSL.ssl, pBuf, iLen);
					g_infoSysSock.isReceiving = 0;				
				}
				else
				{
				 	iReadBytes = -1;
				}
				// JDKIM : end

				//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] SSL_read after. Sock(0x%X), iReadBytes(%d)\n", hSock, iReadBytes);
			}
			else
			{
			/* SWSEO 2010/09/16 added SSL ended */
				iReadBytes = recv( hSock, pBuf, iLen, 0 );
				//nexSALBody_DebugPrintf("recv_iReadBytes:%d\n", iReadBytes);
				//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] recv after. Sock(0x%X), iReadBytes(%d)\n", hSock, iReadBytes);
			}
			//return iReadBytes; // SWSEO 2010/09/16
#else
			iCheckSockInfo = SysGetSocketInfo( hSock );

			//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] SysGetSocketInfo after. Sock(0x%X), iIndex(%d), g_infoSysSock(0x%X)\n", hSock, iCheckSockInfo, g_infoSysSock);
			//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] Is SSL(%d)\n", g_infoSysSock.isUseSSL);
			
			if ( (iCheckSockInfo == 0) && g_infoSysSock.isUseSSL )
			{
				//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] SSL_read before. Sock(0x%X), iReadBytes(%d)\n", hSock);

				// JDKIM 2011/05/13
				if(g_infoSysSock.isClosing == 0)
				{
					g_infoSysSock.isReceiving = 1;				
					iReadBytes = SSL_read(g_infoSysSock.infoSSL.ssl, pBuf, iLen);
					g_infoSysSock.isReceiving = 0;				
				}
				else
				{
				 	iReadBytes = -1;
				}
				// JDKIM : end
				
				//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] SSL_read after. Sock(0x%X), iReadBytes(%d)\n", hSock, iReadBytes);
			}
			else
			{
				//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] recv before. Sock(0x%X)\n", hSock);

				iReadBytes = recv( hSock, pBuf, iLen, 0 );

				//nexSALBody_DebugPrintf("[nexSALBody_SockRecv] recv after. Sock(0x%X), iReadBytes(%d)\n", hSock, iReadBytes);
			}
#endif
			if ( iReadBytes != 0 )
				return iReadBytes;
		}
		else if ( iRet == 0 )			
			return -2;
		else
			return -1;
	}	
}
int nexSALBody_SockMultiGroup(NEXSALSockHandle hSock, char *multiGroup, int bDrop )
{
#if 0	// SWSEO 2010/09/15 for Test. 1 is Original code.
	return 0;
#else
	struct ip_mreq mreq;
	int    multiProp;

	if (bDrop)
	{
		multiProp = IP_DROP_MEMBERSHIP;
	}
	else
	{
		multiProp = IP_ADD_MEMBERSHIP;
	}

	mreq.imr_multiaddr.s_addr = inet_addr(multiGroup);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(hSock, IPPROTO_IP, multiProp, (char*)&mreq, sizeof(mreq)) != 0)
	{
		nexSALBody_DebugPrintf("[SALBody_Sock_SSL.cpp %d] setsockopt error!\n", __LINE__);
		return 0;
	}

	return 1;
#endif
}
//};
