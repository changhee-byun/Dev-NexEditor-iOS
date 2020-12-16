/*******************************************************************************
 * File Name        : NEXVIDEOEDITOR_ThreadBase.h
 * Description      : 
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/


#ifndef __NEXVIDEOEDITOR_THREADBASE_H__
#define __NEXVIDEOEDITOR_THREADBASE_H__

#include "nexEvent.h"
#include "nexLock.h"
#include "nexMsgQueue.h"
#include <vector>
#include <map>
#include "NEXVIDEOEDITOR_MsgDef.h"
#include "NEXVIDEOEDITOR_Types.h"

#define UNKNMOW_TASK_NAME "NEXVIDEOEDITOR Noname task"

#define CNEXThreadBase_Exit			-1

using namespace std;

// The following MACRO can be used in C++ 11.
// Don't use them until C++ 11 stl library is available.
// From Eric.

#define NEXMSG_DECLARE_MESSAGE_MAP(Class) \
	static std::map<int,int(Class::*)(CNxMsgInfo*)> _mmap;

#define NEXMSG_IMPL_DISPATCHER(Class,FuncName) \
	inline int FuncName(CNxMsgInfo* pMsg) { \
	    std::map<int,int (Class::*)(CNxMsgInfo*)>::iterator it = _mmap.find(pMsg->m_nMsgType); \
    	if(it != _mmap.end()) { \
        	return (this->*(it->second))(pMsg); \
    	} \
		return MESSAGE_PROCESS_PASS; \
	}

#define NEXMSG_MAP_BEGIN(Class) \
	std::map<int,int (Class::*)(CNxMsgInfo*)> Class::_mmap = { \

#define NEXMSG_MAP_ENTRY(Key,Class,MemberFunc) \
	{ Key, &Class::MemberFunc },

#define NEXMSG_MAP_END() \
	};


/** \class CNEXThreadBase
    \brief CNEXThreadBase class
*/
class CNEXThreadBase : public CNxRef<INxRefObj>
{
protected:
	CNexMsgQueue		m_MsgQ;
	// CNexMsgQueue		m_FrameQ;
	PLAY_STATE			m_ePreThreadState;
	PLAY_STATE			m_eThreadState;
	unsigned int			m_uiCurrentTime;
	unsigned int			m_ThreadID;			///< ������ ID
	NEXSALTaskHandle		m_hThread;			///< ������ �ڵ�
	NXBOOL				m_bIsWorking;		///< �����尡 ���Ῡ�� ==> IsWorking
	NXBOOL				m_bIsActive;
	NXBOOL				m_bIsTaskPaused;
	NXBOOL				m_bIsTaskResumed;
	NXBOOL				m_isSemaState;		//0:Normal, 1:Waiting
	NEXSALSemaphoreHandle   m_hSema;
	NXBOOL					m_bThreadFuncEnd;

public:
	NEXSALTaskPriority  m_eTaskPriority;
	unsigned int        m_uTaskStackSize;
	NEXSALTaskOption    m_eTaskOption;
	NXUINT32			m_uLogCnt;

public:

	PLAY_STATE getState()
	{
		return m_eThreadState;
	}

	/**	\fn		CThreadBase( void )
		\brief	Thread ���� Ŭ���� ������
		\param	
		\return 
	*/
    CNEXThreadBase( void )
    {
		m_ePreThreadState	= PLAY_STATE_NONE;
		m_eThreadState		= PLAY_STATE_NONE;
		m_uiCurrentTime		= 0;
       	m_bIsWorking		= FALSE;
		m_hThread			= NEXSAL_INVALID_HANDLE;

		m_eTaskPriority		= NEXSAL_PRIORITY_NORMAL;
		// m_uTaskStackSize		= 32*1024;
		// for Android image load problem JIRA 3005 3006 3007 
		m_uTaskStackSize		= 1*1024*1024;
		m_eTaskOption		= NEXSAL_TASK_NO_OPTION;

		m_bIsActive			= FALSE;
		m_bIsTaskPaused		= FALSE;
		m_bIsTaskResumed	= FALSE;
		m_isSemaState		= FALSE;
		m_hSema				= NEXSAL_INVALID_HANDLE;
		m_bThreadFuncEnd	= FALSE;
		m_uLogCnt			= 0;
    }
    

	/**	\fn		~CThreadBase( void )
		\brief	Thread ���� Ŭ���� �Ҹ���, Thread�� �Ҹ���Ų��.
		\param	
		\return 
	*/
    virtual ~CNEXThreadBase( void )
    {
		if(m_bIsWorking)
		{
			End(500);
		}
		if( m_hThread )
		{
			nexSAL_TaskDelete(m_hThread);
			m_hThread = NEXSAL_INVALID_HANDLE;
		}

		if( m_hSema != NEXSAL_INVALID_HANDLE )
		{
			nexSAL_SemaphoreDelete(m_hSema);
			m_hSema = NEXSAL_INVALID_HANDLE;
		}
    }


	/**	\fn		const char* Name()
		\brief	Thread �̸��� ��ȯ�Ѵ�.
		\param	
		\return 
	*/
	virtual const char* Name()
	{	return UNKNMOW_TASK_NAME;
	}

	/**	\fn		NXBOOL Begin( void )
		\brief	Thread�� �����Ѵ�.
		\param	
		\return 
	*/
    virtual NXBOOL Begin( void ) // Thread����
    {
    		m_bIsWorking = TRUE;
		m_bIsActive = TRUE;	

		m_hSema = nexSAL_SemaphoreCreate(0, 1);
		if (m_hSema == NEXSAL_INVALID_HANDLE)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Thread %d] Video Task Create Semaphore Error!\n", __LINE__);
			return FALSE;
		}
		m_isSemaState = FALSE;	//Not Waiting

		m_hThread = nexSAL_TaskCreate(	(char*)Name(),
										(NEXSALTaskFunc)ThreadProc, 
										(void*)this, 
										m_eTaskPriority, 
										m_uTaskStackSize, 
										m_eTaskOption);

		if( m_hThread == NEXSAL_INVALID_HANDLE )
			return FALSE;
		return TRUE;
    }

	/**	\fn		NXBOOL Begin(char* pThreadName, NEXSALTaskPriority ePriority = NEXSAL_PRIORITY_NORMAL, unsigned int uiStackSize = 16*1024, NEXSALTaskOption eThreadOption = NEXSAL_TASK_NO_OPTION)
		\brief	Thread�� �����Ѵ�.
		\param	
		\return 
	*/
    virtual NXBOOL Begin( char* pThreadName, NEXSALTaskPriority ePriority = NEXSAL_PRIORITY_NORMAL, unsigned int uiStackSize = 16*1024, NEXSALTaskOption eThreadOption = NEXSAL_TASK_NO_OPTION )
    {
		m_eTaskPriority		= ePriority;
		m_uTaskStackSize		= uiStackSize;
		m_eTaskOption		= eThreadOption;

    		m_bIsWorking = TRUE;
		m_bIsActive = TRUE;				

		m_hSema = nexSAL_SemaphoreCreate(0, 1);
		if (m_hSema == NEXSAL_INVALID_HANDLE)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Thread %d] Video Task Create Semaphore Error!\n", __LINE__);
			return FALSE;
		}
		m_isSemaState = FALSE;	//Not Waiting

		m_hThread = nexSAL_TaskCreate(	pThreadName, 
										(NEXSALTaskFunc)ThreadProc, 
										(void*)this, 
										ePriority, 
										uiStackSize, 
										eThreadOption);

		if( m_hThread == NEXSAL_INVALID_HANDLE )
			return FALSE;
		return TRUE;
    }

	/**	\fn		void End( DWORD dwTimeout )
		\brief	Thread�� ������Ų��.
		\param	
		\return 
	*/
    virtual void End( unsigned int uiTimeout ) // Thread �۾� ����
    {
		if( m_bIsWorking == FALSE ) return;
		if( m_hThread == NEXSAL_INVALID_HANDLE ) return;

		CNxMsgInfo* pMsg = new CNxMsgInfo;

		pMsg->m_nMsgType = CNEXThreadBase_Exit;
		m_MsgQ.PushCommand( pMsg );
		SAFE_RELEASE(pMsg);

		if( nexSAL_TaskWait(m_hThread) != 0 )
		{
			// nexSAL_TaskTerminate(m_hThread);
		}

		if( m_hSema != NEXSAL_INVALID_HANDLE )
		{
			nexSAL_SemaphoreDelete(m_hSema);
			m_hSema = NEXSAL_INVALID_HANDLE;
		}
		
		nexSAL_TaskDelete(m_hThread);
		m_hThread = NEXSAL_INVALID_HANDLE;
		m_bIsWorking = FALSE;
    }

	virtual unsigned int SetSuspend()
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d]\n", Name(), __FUNCTION__, __LINE__);

		if (m_hThread && !m_bThreadFuncEnd)
		{
			m_bIsTaskPaused = FALSE;
			m_bIsActive = FALSE;
		}

		return TRUE;
	}

	virtual unsigned int Suspend()
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d]\n", Name(), __FUNCTION__, __LINE__);

		if (m_hThread && !m_bThreadFuncEnd)
		{
			if (m_isSemaState)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d] Already task waiting... return TRUE\n", Name(), __FUNCTION__, __LINE__);
				return TRUE;
			}

			while (m_bIsTaskPaused == FALSE && m_eThreadState != PLAY_STATE_IDLE)
			{
				nexSAL_TaskSleep(10);
			}
		}

		return TRUE;
	}

	virtual unsigned int SetActivate()
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d]\n", Name(), __FUNCTION__, __LINE__);

		if (m_hThread && !m_bThreadFuncEnd)
		{
			m_bIsTaskResumed = FALSE;
			m_bIsActive = TRUE;

			if( m_hSema != NEXSAL_INVALID_HANDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d] Task Semaphore Release!\n", Name(), __FUNCTION__, __LINE__);
				nexSAL_SemaphoreRelease(m_hSema);
				m_isSemaState = FALSE;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d] Task PauseMode Semaphore is NULL!!!\n", Name(), __FUNCTION__, __LINE__);
			}
		}

		return TRUE;
	}

	virtual unsigned int Activate()
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d]\n", Name(), __FUNCTION__, __LINE__);

		if (m_hThread && !m_bThreadFuncEnd)
		{
			while (m_bIsTaskResumed == FALSE && m_bIsWorking == TRUE)
			{
				nexSAL_TaskSleep(10);
			}
		}

		return TRUE;
	}

	/**	\fn		unsigned int GetThreadID()
		\brief	Thread ID�� ��ȯ�Ѵ�.
		\param	
		\return 
	*/
    unsigned int GetThreadID() { return m_ThreadID; }


	/**	\fn		BOOL IsWorking()
		\brief	Thread�� ������������ �˷��ش�.
		\param	
		\return 
	*/
    NXBOOL IsWorking()
    {
        return m_bIsWorking;
    }

    virtual unsigned int getCurrentTime()
    {
        return m_uiCurrentTime;
    }

protected:                                 

	/**	\fn		int ProcessCommandMsg( CNxMsgInfo* pMsg )
		\brief	Param���� �Ѿ��� �޼����� ó���Ѵ�.
		\param	
		\return 
	*/
    virtual int ProcessCommandMsg( CNxMsgInfo* pMsg )
	{
		if( pMsg == NULL ) return MESSAGE_PROCESS_PASS;

		switch( pMsg->m_nMsgType )
		{
			case MESSAGE_STATE_CHANGE:
			{
				CNxMsgChangeState* pStateChangeMsg = (CNxMsgChangeState*)pMsg;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thread.cpp %d] StateChange(%s : %d %d)", __LINE__, Name(), m_eThreadState, pStateChangeMsg->m_currentState );
				if( m_eThreadState == pStateChangeMsg->m_currentState )
				{
					break;
				}
				m_ePreThreadState = m_eThreadState;
				m_eThreadState = pStateChangeMsg->m_currentState;
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}
			case MESSAGE_UPDATE_CURRENTIME:
			{
				CNxMsgUpdateCurrentTimeStamp* pCurTime = (CNxMsgUpdateCurrentTimeStamp*)pMsg;
				m_uiCurrentTime = pCurTime->m_currentTimeStamp;
				break;
			}
		}
		return MESSAGE_PROCESS_PASS;
	}

	/**	\fn		void ProcessIdleState()
		\brief	
		\return 
	*/
    virtual void ProcessIdleState()
	{
		nexSAL_TaskSleep(5);
	}

	/**	\fn		void ProcessPlayState()
		\brief	
		\return 
	*/
    virtual void ProcessPlayState()
	{
		nexSAL_TaskSleep(5);
	}

	/**	\fn		void ProcessRecordState()
		\brief	
		\return 
	*/
    virtual void ProcessRecordState()
	{
		nexSAL_TaskSleep(5);
	}

	/**	\fn		void ProcessPauseState()
		\brief	
		\return 
	*/
    virtual void ProcessPauseState()
	{
		nexSAL_TaskSleep(10);
	}

	/**	\fn		int OnThreadMain( void )
		\brief	�������� Thread�� ������ �����Ѵ�.
		\param	
		\return 
	*/
	virtual int OnThreadMain( void )
	{
		m_bIsWorking = TRUE;
		while ( TRUE )
		{
			CNxMsgInfo* pCmdMsg = NULL;
			if( m_eThreadState == PLAY_STATE_IDLE || m_eThreadState == PLAY_STATE_NONE ) {
				pCmdMsg = m_MsgQ.PopCommand(NEXSAL_INFINITE);
			}
			else {
				pCmdMsg = m_MsgQ.PopCommand(5);
			}

			if( pCmdMsg != NULL )
			{
				if ( pCmdMsg->m_nMsgType == CNEXThreadBase_Exit )
				{
					SAFE_RELEASE(pCmdMsg);
					break;
				}

				if( ProcessCommandMsg(pCmdMsg) == MESSAGE_PROCESS_OK )
					continue;
				SAFE_RELEASE(pCmdMsg);
			}

			switch(m_eThreadState)
			{
				case PLAY_STATE_IDLE:
				{
					m_bIsTaskResumed = FALSE;
					m_bIsTaskPaused = FALSE;
					ProcessIdleState();
					break;
				}
				case PLAY_STATE_RUN:
				{
					ProcessPlayState();
					break;
				}
				case PLAY_STATE_RECORD:
				{
					ProcessRecordState();
					break;
				}
				case PLAY_STATE_PAUSE:
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thread.cpp %d] PLAY_STATE_PAUSE(%s)", __LINE__, Name());					
					if(m_bIsTaskPaused == FALSE)
					{
						m_bIsTaskPaused = TRUE;
						m_bIsTaskResumed = FALSE;
					}

					if (m_bIsTaskPaused && m_hSema != NEXSAL_INVALID_HANDLE )
					{
						m_isSemaState = TRUE;
						nexSAL_SemaphoreWait(m_hSema, NEXSAL_INFINITE);
					}
					break;
				}
				case PLAY_STATE_RESUME:
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thread.cpp %d] PLAY_STATE_RESUME(%s), %d", __LINE__, Name(), m_bIsTaskResumed);					
					if (m_bIsTaskResumed == FALSE)
					{
						m_bIsTaskResumed = TRUE;
						m_bIsTaskPaused = FALSE;
						m_eThreadState = PLAY_STATE_RECORD;
					}							
					break;
				}
				
				default:
				{
					nexSAL_TaskSleep(5);
					break;
				}
			};
		}
		m_bIsWorking = FALSE;
		return 0;
	}

	/**	\fn		static unsigned ThreadProc( void* PtrToInstance )
		\brief	Thread Main Procedure
		\param	
		\return 
	*/
    static int ThreadProc( void* PtrToInstance )
    {
        unsigned int uRet = 0;

	CNEXThreadBase *pThread = reinterpret_cast<CNEXThreadBase*>( PtrToInstance );
	if( pThread )
		uRet = pThread->OnThreadMain();
	/*
	if( pThread->m_bIsWorking == FALSE )
		uRet = pThread->OnThreadMain();
       */
	return uRet;
    }
public:


	/**	\fn		NXBOOL SendCommand(CNxMsgInfo* pMsg)
		\brief	Param���� �Ѿ��� �޼����� ������
		\param	
		\return 
	*/
	NXBOOL SendCommand(CNxMsgInfo* pMsg)
	{
		if( pMsg == NULL)
			return FALSE;
		// TRACE("SendCommand : %d\n",pMsg->m_nMsgType);
		if(m_eThreadState == PLAY_STATE_PAUSE && pMsg->m_nMsgType == MESSAGE_STATE_CHANGE && ((CNxMsgChangeState*)pMsg)->m_currentState == PLAY_STATE_IDLE)
		{
			SetActivate();
		}	
		m_MsgQ.PushCommand(pMsg);
		return TRUE;
	}


	/**	\fn		HRESULT SendSimpleCommand(MESSAGE_DEFINITION enMsg)
		\brief	�޼��� ���޽� �޼��� ID�� �ִ� ���쿡 �����ϰ� �����Ҽ� �ֵ��� �Ѵ�.
		\param	
		\return 
	*/
	NXBOOL SendSimpleCommand(NEXVIDEOEDITOR_MESSAGE eMsg)
	{
		CNxMsgInfo* pMsg = new CNxMsgInfo(eMsg);
		m_MsgQ.PushCommand(pMsg);
		SAFE_RELEASE(pMsg);
		return TRUE;
	}

	NXBOOL SendSimpleStateChangeCommand(PLAY_STATE eNewState)
	{
		CNxMsgChangeState* pMsg = new CNxMsgChangeState(eNewState);
		if(m_eThreadState == PLAY_STATE_PAUSE && pMsg->m_nMsgType == MESSAGE_STATE_CHANGE && pMsg->m_currentState == PLAY_STATE_IDLE)
		{
			SetActivate();
		}	
		m_MsgQ.PushCommand(pMsg);
		SAFE_RELEASE(pMsg);
		return TRUE;
	}	

	NXBOOL SendSimpleUpdateTimeCommand(unsigned int uiTime)
	{
		CNxMsgUpdateCurrentTimeStamp* pMsg = new CNxMsgUpdateCurrentTimeStamp(uiTime);
		m_MsgQ.PushCommand(pMsg);
		SAFE_RELEASE(pMsg);
		return TRUE;
	}	

	NXBOOL SendCommandWithClear(CNxMsgInfo* pMsg)
	{
		if( pMsg == NULL)
			return FALSE;
		m_MsgQ.ClearCommand();
		m_MsgQ.PushCommand(pMsg);
		return TRUE;
	}

	int GetMsgSize()
	{
		return m_MsgQ.GetMsgSize();
	}	

	NXBOOL SendPauseCommand(PLAY_STATE eNewState)
	{
		SetSuspend();
		CNxMsgChangeState* pMsg = new CNxMsgChangeState(eNewState);
		m_MsgQ.PushCommand(pMsg);
		SAFE_RELEASE(pMsg);
		Suspend();		
		return TRUE;
	}

	NXBOOL SendResumeCommand(PLAY_STATE eNewState)
	{
		CNxMsgChangeState* pMsg = new CNxMsgChangeState(eNewState);
		m_MsgQ.PushCommand(pMsg);
		SAFE_RELEASE(pMsg);
		SetActivate();
		Activate();		
		return TRUE;
	}
};

typedef std::vector<CNEXThreadBase*>	CNEXThreadVector;
typedef CNEXThreadVector::iterator		CNEXThreadVectorIter;

#endif // __NEXVIDEOEDITOR_THREADBASE_H__

/*------------------------------------------------------------------------------
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 cskb	2011/05/13	Draft.
 Eric	2017/04/06	Introduce Dispatch method.
 -----------------------------------------------------------------------------*/
