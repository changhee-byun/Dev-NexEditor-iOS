/******************************************************************************
* File Name   :	nexEvent.h
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
#ifndef __NEXEVENT_H__
#define __NEXEVENT_H__
//---------------------------------------------------------------------------

#include "NexSAL_Internal.h"

/**	\class CNexEvent
	\brief CNexEvent class.
*/
class CNexEvent
{
public:

	/**	\fn		CNexEvent( BOOL bManualReset = FALSE, BOOL bInitialState = FALSE )
		\brief	CNexEvent Class의 생성자, 이벤트핸들을 생성한다.
		\param	
		\return 
	*/
    CNexEvent( NXBOOL bManualReset = FALSE, NXBOOL bInitialState = FALSE )
    {
		NEXSALEventReset eReset = bManualReset ? NEXSAL_EVENT_MANUAL : NEXSAL_EVENT_AUTO;
		NEXSALEventInitial eInit = bInitialState ? NEXSAL_EVENT_SET : NEXSAL_EVENT_UNSET;

		m_hEvent = nexSAL_EventCreate(eReset, eInit);
    }


	/**	\fn		~CNexEvent()
		\brief	생성된 이벤트핸들을 해제한다.
		\param	
		\return 
	*/
    ~CNexEvent()
    {
		if( m_hEvent )
			nexSAL_EventDelete(m_hEvent);
    }


	/**	\fn		BOOL Set()
		\brief	이벤트를 발생시킨다.
		\param	
		\return 
	*/
    inline NXBOOL Set()
    {
		if( m_hEvent && nexSAL_EventSet(m_hEvent) == 0)
			return TRUE;
		return FALSE;
	}

	/**	\fn		BOOL Reset()
		\brief	이벤트를 초기화한다.
		\param	
		\return 
	*/
    inline NXBOOL Reset()
    {
		if( m_hEvent && nexSAL_EventClear(m_hEvent) == 0)
			return TRUE;
		return FALSE;
	}


	/**	\fn		BOOL WaitEvent( DWORD dwMSec )
		\brief	Param으로 넘어온 시간만큼(ms) 이벤트가 발생할때까지 대기한다.
		\param	
		\return 
	*/
    NXBOOL WaitEvent( unsigned int uiMSec )
    {
		if( m_hEvent && nexSAL_EventWait(m_hEvent, uiMSec) == 0)
			return TRUE;
		return FALSE;
    }
    
    NXBOOL isValid()
    {
        NXBOOL result = TRUE;
        if (!m_hEvent) result = FALSE;
        return result;
    }

#if 0
	/**	\fn		operator HANDLE() const
		\brief	HANDLE() Operator연산자
		\param	
		\return 
	*/
    operator HANDLE() const
    {   return m_hEvent;
    }
#endif

protected:
    NEXSALEventHandle m_hEvent;
};

#endif // __NEXEVENT_H__
