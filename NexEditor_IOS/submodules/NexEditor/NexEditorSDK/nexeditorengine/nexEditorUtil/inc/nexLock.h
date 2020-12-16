/******************************************************************************
* File Name   :	nexLock.h
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
#ifndef __NEXLOCK_H__
#define __NEXLOCK_H__
//---------------------------------------------------------------------------

#include "NexSAL_Internal.h"

/**	\class CNexLock
	\brief CNexLock class.
*/
class CNexLock
{
public:

	/**	\fn		CNexLock()
		\brief	CNexLock Class�� ������
		\param	
		\return 
	*/
    CNexLock()
    {
		m_hMutex = nexSAL_MutexCreate();
    }


	/**	\fn		~CNexLock()
		\brief	CNexLock Class�� �Ҹ���
		\param	
		\return 
	*/
    ~CNexLock()
    {
		if( m_hMutex )
			nexSAL_MutexDelete(m_hMutex);
    }


	/**	\fn		void Lock()
		\brief	�ڿ��� ���û���� ���� ���Ͽ� Lock�� �����Ѵ�.
		\param	
		\return 
	*/
    void Lock()
    {
		nexSAL_MutexLock(m_hMutex, NEXSAL_INFINITE);
    }


	/**	\fn		void Unlock()
		\brief	�����ϰ� �ִ� �ڿ��� �����ش�.
		\param	
		\return 
	*/
    void Unlock()
    {
		nexSAL_MutexUnlock(m_hMutex);
    }

protected:
	NEXSALMutexHandle m_hMutex;
};

/**	\class CAutoLock
	\brief CAutoLock class.
*/
class CAutoLock
{
public:

	/**	\fn		CAutoLock( CNexLock& lock )
		\brief	CAutoLock Class������, Param���� ���� ���۷����� Lock�� �����Ѵ�.
		\param	
		\return 
	*/
    CAutoLock( CNexLock& lock )
    {
        m_pLock = &lock;
        m_pLock->Lock();
    }

	/**	\fn		~CAutoLock()
		\brief	�����ڿ��� ���� CNexLock ���۷����� UnLock�� �����Ѵ�.
		\param	
		\return 
	*/
    ~CAutoLock()
    {
        if ( m_pLock )
            m_pLock->Unlock();
    }

	/**	\fn	    void Lock()
		\brief	Lock�� �����Ѵ�.
		\param	
		\return 
	*/
    void Lock()
	{
        if ( m_pLock )
            m_pLock->Lock();
    }

	/**	\fn		void Unlock()
		\brief	Unlock�� �����Ѵ�.
		\param	
		\return 
	*/
    void Unlock()
    {
        if ( m_pLock )
            m_pLock->Unlock();
    }

protected:
    CNexLock* m_pLock;
};

#endif // __NEXLOCK_H__
