/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_MsgQueue.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef __NEXVIDEOEDITOR_MSGQUEUE_H__
#define __NEXVIDEOEDITOR_MSGQUEUE_H__
//---------------------------------------------------------------------------

#include <queue>
#include "nexDef.h"
#include "nexEvent.h"
#include "nexLock.h"
#include "NEXVIDEOEDITOR_MsgInfo.h"

using namespace std;

/**	\class CNexMsgQueue
	\brief CNexMsgQueue class.
*/
class CNexMsgQueue
{
public:


	/**	\fn		CNexMsgQueue()
		\brief	CNexMsgQueue Class생성자
		\param	
		\return 
	*/
	CNexMsgQueue() : m_Event( FALSE, FALSE )
	{
	}

	/**	\fn		~CNexMsgQueue()
	\brief	CNexMsgQueue Class소멸자
	\param	
	\return 
	*/
	virtual ~CNexMsgQueue()
	{

		ClearCommand();
	}

	int GetMsgSize()
	{
		return m_Msg.size();
	}

	/**	\fn		NXBOOL ClearCommand( CNxMsgInfo* pMsg )
		\brief	메세지 큐에 메세지를 clear한다.
		\param	
		\return 
	*/
	NXBOOL ClearCommand( )
	{
		CAutoLock lock( m_Lock );
		while(m_Msg.empty() == FALSE )
		{
			CNxMsgInfo* pMsg = m_Msg.front();
			m_Msg.pop();
			if( pMsg == NULL ) break;
				SAFE_RELEASE(pMsg);
		}
		return TRUE;
	}


	/**	\fn		NXBOOL PushCommand( CNxMsgInfo* pMsg )
		\brief	메세지 큐에 메세지를 push한다.
		\param	
		\return 
	*/
	NXBOOL PushCommand( CNxMsgInfo* pMsg )
	{
		// if( pMsg == NULL ) return FALSE;
		CAutoLock lock( m_Lock );
		SAFE_ADDREF(pMsg);
		m_Msg.push( pMsg );
		m_Event.Set();
		return TRUE;
	}


	/**	\fn		CNxMsgInfo* PopCommand( DWORD dwTimeout )
		\brief	메세지 큐에서 메세지를 pop한다.
		\param	
		\return 
	*/
	CNxMsgInfo* PopCommand( unsigned int uiTimeout )
	{
		m_Lock.Lock();
		NXBOOL bRet = m_Msg.empty();
		m_Lock.Unlock();

		if( bRet )
			bRet = m_Event.WaitEvent(uiTimeout);

		CAutoLock lock( m_Lock );
		if ( m_Msg.empty() )
		{
			m_Event.Reset();
			return NULL;
		}

		CNxMsgInfo* pMsg = m_Msg.front();
		m_Msg.pop();
		if ( m_Msg.empty() )
		{
			m_Event.Reset();
		}
		return pMsg;
	}

	protected:
	queue<CNxMsgInfo*> m_Msg;

	CNexLock	m_Lock;
	CNexEvent	m_Event;
};

#endif // __NEXVIDEOEDITOR_MSGQUEUE_H__

