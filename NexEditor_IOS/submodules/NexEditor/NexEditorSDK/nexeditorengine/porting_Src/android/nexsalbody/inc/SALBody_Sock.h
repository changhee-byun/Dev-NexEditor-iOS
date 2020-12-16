/*
 *  SALBody_Sock.h
 *  nexSalBody
 *
 *  Created by Sunghyun Yoo on 08. 12. 26.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _SALBODY_SCOK_HEADER_
#define _SALBODY_SCOK_HEADER_


#include "NexSAL_API.h"
#include "NexSAL_Internal.h"

//namespace android {

#ifdef __cplusplus
extern "C" {
#endif

	NEXSALSockHandle nexSALBody_SockCreate( int iType );
	int nexSALBody_SockClose( NEXSALSockHandle hSock );
	int nexSALBody_SockConnect( NEXSALSockHandle hSock, char *pAddr, unsigned short wPort, unsigned int dwTimeout );
	int nexSALBody_SockBind( NEXSALSockHandle hSock, unsigned short wPort );
	int nexSALBody_SockSelect( NEXSALFDSet* pRSet, NEXSALFDSet* pWSet, NEXSALFDSet* pESet, NEXSALTimeValue* pTV );
	int nexSALBody_SockSendTo( NEXSALSockHandle hSock, char *pBuf, int iLen, char *pAddr, unsigned short wPort );
	int nexSALBody_SockSend( NEXSALSockHandle hSock, char *pBuf, int iLen );
	int nexSALBody_SockRecvFrom( NEXSALSockHandle hSock, char *pBuf, int iLen, unsigned int *pdwAddr, unsigned short *pwPort, unsigned int dwTimeout );
	int nexSALBody_SockRecv( NEXSALSockHandle hSock, char *pBuf, int iLen, unsigned int uTimeout );
	int nexSALBody_SockMultiGroup(NEXSALSockHandle hSock, char *multiGroup, int bDrop );

//	void nexSALBody_Sock_Init();
	void nexSALBody_Sock_SetCloseMode( int iMode );

#ifdef __cplusplus
}
#endif

//};

#endif
