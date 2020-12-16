/******************************************************************************
* File Name        : nexSALClasses.h
* Description      : C++ classes using only nexSAL
* Revision History : Located at the bottom of this file
*******************************************************************************
*
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
*
*	Nextreaming Confidential Proprietary
*	Copyright (C) 2006~2011 Nextreaming Corporation
*	All rights are reserved by Nextreaming Corporation
*
******************************************************************************/

#ifndef _SAL_Classes_h_Included_
#define _SAL_Classes_h_Included_

#include "NexSAL_Internal.h"

#ifndef S_OK
#	define S_OK 0
#endif

#ifndef E_FAIL
#	define E_FAIL 1
#endif

#ifndef S_FALSE
#	define S_FALSE 2
#endif

class CSALFile
{
public:
	CSALFile() : m_hFile(NEXSAL_INVALID_HANDLE) {};

	static const int READ		= NEXSAL_FILE_READ;
	static const int WRITE		= NEXSAL_FILE_WRITE;
	static const int READWRITE	= NEXSAL_FILE_READWRITE;
	static const int CREATE		= NEXSAL_FILE_CREATE;

	static const int BEGIN		= NEXSAL_SEEK_BEGIN;
	static const int CUR		= NEXSAL_SEEK_CUR;
	static const int END		= NEXSAL_SEEK_END;

	NXBOOL Open( const char* pFN, int iMode )
	{
		m_hFile = nexSAL_FileOpen( (char*)pFN, (NEXSALFileMode)iMode );
		if ( m_hFile == NEXSAL_INVALID_HANDLE )
			return FALSE;

		return TRUE;
	};

	void Close()
	{
		if ( m_hFile != NEXSAL_INVALID_HANDLE )
			nexSAL_FileClose( m_hFile );

		m_hFile = NEXSAL_INVALID_HANDLE;
	};

	int Write( char* pData, int iLen )
	{
		if ( m_hFile == NEXSAL_INVALID_HANDLE )
			return -1;

		return nexSAL_FileWrite( m_hFile, pData, iLen );
	}

	int Read( char* pData, int iLen )
	{
		if ( m_hFile == NEXSAL_INVALID_HANDLE )
			return -1;

		return nexSAL_FileRead( m_hFile, pData, iLen );
	}

	int Seek( int iOffset, int iOrigin )
	{
		if ( m_hFile == NEXSAL_INVALID_HANDLE )
			return -1;

		return nexSAL_FileSeek( m_hFile, iOffset, (NEXSALFileSeekOrigin)iOrigin );
	}

private:

	NEXSALFileHandle m_hFile;
};

class CSALMutex
{
public:
	CSALMutex()
	{
		m_hMutex = nexSAL_MutexCreate();
	};

	~CSALMutex()
	{
		if ( m_hMutex != NEXSAL_INVALID_HANDLE )
		{
			nexSAL_MutexDelete( m_hMutex );
			m_hMutex = NEXSAL_INVALID_HANDLE;
		}
	}

	// lock or unlock the mutex
	int	Lock( int nTimeout = NEXSAL_INFINITE )
	{
		if ( m_hMutex != NEXSAL_INVALID_HANDLE )
			nexSAL_MutexLock( m_hMutex, nTimeout );
		return S_OK;
	}

	void Unlock()
	{
		if ( m_hMutex != NEXSAL_INVALID_HANDLE )
			nexSAL_MutexUnlock( m_hMutex );
	}

	NEXSALMutexHandle GetHandle()
	{
		return m_hMutex;
	}

	// Manages the mutex automatically. It'll be locked when 'Auto' is
	// constructed and released when 'Auto' goes out of scope.
	class Auto {
	public:
		inline Auto(CSALMutex& mutex) : m_pMutex(&mutex) { mutex.Lock(); }
		inline Auto(CSALMutex* mutex) : m_pMutex(mutex) { m_pMutex->Lock(); }
		inline ~Auto() { m_pMutex->Unlock(); }
	private:
		CSALMutex*  m_pMutex;
	};

private:

	NEXSALMutexHandle	m_hMutex;
};

class CSALSemaphore
{
public:

	CSALSemaphore( NEXSALSemaphoreHandle hSema = NULL, NXBOOL bAutoDelete = FALSE )
	{	
		m_bAutoDelete = bAutoDelete;
		m_hSema = hSema;
	};
	
	virtual ~CSALSemaphore()
	{	
		Delete();
	};

	int Create(int iInitCount,int iMaxCount)
	{
		Delete();
		m_hSema = nexSAL_SemaphoreCreate( iInitCount, iMaxCount );

		if ( !m_hSema )
			return E_FAIL;

		m_bAutoDelete = TRUE;
		return S_OK;
	}

	int	Delete()
	{
		if ( m_hSema && m_bAutoDelete )
			nexSAL_SemaphoreDelete(m_hSema);
		m_hSema = (NEXSALSemaphoreHandle)NULL;
		return S_OK;
	}

	int Wait(int nTimeout=NEXSAL_INFINITE)
	{
		int nRet = nexSAL_SemaphoreWait(m_hSema,nTimeout);
		
		if ( nRet == eNEXSAL_RET_NO_ERROR )
			return S_OK;
		else if ( nRet == eNEXSAL_RET_TIMEOUT )
			return S_FALSE;
		
		return E_FAIL;
	}

	int Release()
	{
		nexSAL_SemaphoreRelease(m_hSema);
		return S_OK;
	}

	NEXSALSemaphoreHandle	GetHandle()
	{
		return m_hSema;
	}

private:
	NXBOOL					m_bAutoDelete;
	NEXSALSemaphoreHandle	m_hSema;
};


class CSALTask
{
public:
	static void Sleep( int iMiliSec )
	{
		nexSAL_TaskSleep( iMiliSec );
	}
	
	typedef enum Priority
	{
		LOWEST			= NEXSAL_PRIORITY_LOWEST,
		LOW				= NEXSAL_PRIORITY_LOW,
		BELOW_NORMAL	= NEXSAL_PRIORITY_BELOW_NORMAL,
		NORMAL			= NEXSAL_PRIORITY_NORMAL,
		ABOVE_NORMAL	= NEXSAL_PRIORITY_ABOVE_NORMAL,
		HIGH			= NEXSAL_PRIORITY_HIGH,
		URGENT			= NEXSAL_PRIORITY_URGENT,
		HIGHEST			= NEXSAL_PRIORITY_HIGHEST

	} Priority;

	typedef enum TaskOption
	{
		NO_OPTION	= NEXSAL_TASK_NO_OPTION,
		USE_DSP		= NEXSAL_TASK_USE_DSP,
		USE_FILE	= NEXSAL_TASK_USE_FILE

	} TaskOption;

	typedef enum StateCondition
	{
		CONDITION_NONE,
		CONDITION_ACTIVE,
		CONDITION_SUSPENDED,
		CONDITION_FORCE_TERMINATING

	} StateCondition;

	typedef enum State
	{
		NONE,
		ACTIVE,
		SUSPENDED,
		TERMINATED

	} State;

	// Create a Task object, but doesn't create or start the associated
	// thread. See the run() method.
	CSALTask();
	virtual ~CSALTask();
	
	virtual int Begin	( const char* pTaskName = 0
						, Priority ePriority = NORMAL
						, unsigned int uStackSize = 0
						, TaskOption eTaskOpt = NO_OPTION
						, State eStartState = ACTIVE );

	// blocks until task is suspended unless called by task itself
	virtual int Suspend();
	// blocks until task is activated unless called by task itself
	virtual int Activate();

	virtual int End();

	NEXSALTaskHandle GetHandle() { return m_hTask; };
	
protected:
	virtual int SetStateCondition( StateCondition eCondition );
	virtual StateCondition GetStateCondition();
	virtual int ResetStateCondition();
	virtual int Run();
	virtual int RunActiveLoop() { return 0; };
	
	virtual void OnSuspended() {};
	virtual void OnActivated() {};
	virtual void OnTerminated() {};

private:
	State GetState();
	void SetState( State eS );
	
	static int _threadRunLoop( void* user );

	StateCondition m_eStateCondition;
	State m_eState;
	NEXSALTaskHandle m_hTask;
	CSALMutex m_hMutex;
};

#endif

/*=============================================================================
                                Revision History
===============================================================================
    Author         Date          API Version
===============================================================================
    ysh            2011/05/28    1.0
-------------------------------------------------------------------------------
 Draft
===============================================================================
    benson         2012/03/12    1.1
-------------------------------------------------------------------------------
 CSALTask changes:
  added ResetStateCondition;
  added OnTerminated;
  changed visibility of StateCondition related functions to protected;
  changed visibility of State related functions to private;
  prevented Activate and Suspend from blocking if called by the task;
  added more lock usage
 Changed Revision History format
=============================================================================*/
