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

//namespace android {

//static int g_socket_fd_max = 0;
static int g_iCloseMode = 0;

//void nexSALBody_Sock_Init()
//{
//	g_socket_fd_max = 0;
//}

void nexSALBody_Sock_SetCloseMode( int iMode )
{
	g_iCloseMode = iMode;
}

NEXSALSockHandle nexSALBody_SockCreate( int iType )
{
	int sock_fd;
	if ( iType == NEXSAL_SOCK_STREAM )
	{
		sock_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	}
	else
	{
		sock_fd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	}
	
//	g_socket_fd_max = ( ( g_socket_fd_max > sock_fd ) ? g_socket_fd_max : sock_fd );
	
	nexSALBody_DebugPrintf("SocketCreate:%d\n",sock_fd);

	if(sock_fd < 0 )
	{
		nexSALBody_DebugPrintf("SockeCreate Error:%d\n", errno);
	}
	return sock_fd;
}

int nexSALBody_SockClose( NEXSALSockHandle hSock )
{
	close( hSock );
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

	if ( g_isDomain && strcmp(g_szDomain,pAddr) == 0 )
	{
		bcopy( &g_inet_addr, (char*)&psin->sin_addr, sizeof(g_inet_addr) );
		return 0;
	}
	
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
	
	return 0;
}

//#include "SALBody_task.h"


int nexSALBody_SockConnect( NEXSALSockHandle hSock, char *pAddr, unsigned short wPort, unsigned int dwTimeout )
{
	int iRet;
	struct sockaddr_in sin;

	nexSALBody_DebugPrintf("SockConnect 1\n");
	
	if ( _getaddress( &sin, pAddr, wPort ) != 0 )
		return -1;
	nexSALBody_DebugPrintf("SockConnect 2\n");
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
	nexSALBody_DebugPrintf("dwTimeout is %d\n",dwTimeout);
	
	int value = fcntl(hSock, F_GETFL, 0);
	nexSALBody_DebugPrintf("socket's fcntl value is %X\n", value);
	fcntl(hSock, F_SETFL, value|O_NONBLOCK );
	
	int nRemainedmSec = dwTimeout;
	
	for ( int i = 0 ; nRemainedmSec > 0 ; i++ )
	{
		iRet = connect( hSock, (struct sockaddr *)&sin, sizeof(sin) );
		nexSALBody_DebugPrintf("SockConnect - connect ret : %d, errno=%d\n",iRet,errno);
		
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
		
		if ( errno != EINPROGRESS && errno != EAGAIN && errno != EALREADY )
			break;
		
		nexSALBody_TaskSleep( 500 );
		nRemainedmSec -= 500;
	}
	
	fcntl(hSock, F_SETFL, value );
	
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
		nexSALBody_DebugPrintf("bsd socket error - bind %d, %d\n", iRet, errno );
	
	
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
	return send( hSock, pBuf, iLen, 0 );
}

int nexSALBody_SockRecvFrom( NEXSALSockHandle hSock, char *pBuf, int iLen, unsigned int *pdwAddr, unsigned short *pwPort, unsigned int dwTimeout )
{
	int ret;
	
	struct sockaddr_in sin;
	//struct sockaddr saddr;
	socklen_t len;

//	nexSALBody_DebugPrintf("sock_recvfrom begin");

	ret = recvfrom( hSock, pBuf, iLen, 0, (struct sockaddr*)&sin, &len );
	
	if ( pdwAddr )
		*pdwAddr = sin.sin_addr.s_addr;
	if ( pwPort )
		*pwPort = sin.sin_port;

//	nexSALBody_DebugPrintf("sock_recvfrom end");
	
	return ret;
}

int nexSALBody_SockRecv( NEXSALSockHandle hSock, char *pBuf, int iLen, unsigned int uTimeout )
{
	int iRet;
	int iReadBytes, i;
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
		iRet = select( hSock+1, &rs, NULL, NULL, &tv );
		if ( iRet > 0 )
		{
			iReadBytes = recv( hSock, pBuf, iLen, 0 );
			//nexSALBody_DebugPrintf("recv_iReadBytes:%d\n", iReadBytes);
			return iReadBytes;
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
	return 0;
}
//};
