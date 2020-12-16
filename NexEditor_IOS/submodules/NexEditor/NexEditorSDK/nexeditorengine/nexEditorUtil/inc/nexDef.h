/******************************************************************************
* File Name   :	NEXPLAYER_Def.h
* Description :	Header file including only Define directives for NexPlayer
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
#ifndef __NEXPLAYER_DEF_H__
#define __NEXPLAYER_DEF_H__
//---------------------------------------------------------------------------

#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)

#if defined(__APPLE__)

#include <atomic>
using namespace std;

#else

#include <stdatomic.h>

#endif//defined(__APPLE__)

#else

#if defined(__APPLE__)

#include <atomic>
using namespace std;

#else

#include <sys/atomics.h>

#endif//defined(__APPLE__)

#endif//defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)

//---------------------------------------------------------------------------
// Typedef
#ifndef NULL
#define NULL 0
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	\
	{	if ( x )\
		{	x->Release();\
			x = NULL;\
		}\
	}
#endif

#ifndef SAFE_ADDREF
#define SAFE_ADDREF(x)	\
	{	if ( x )\
		{	(x)->AddRef();\
		}\
	}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)	\
	{	if ( x )\
		{	delete (x);\
			x = NULL;\
		}\
	}
#endif

/**	\class CNxRef
	\brief CNxRef class.
*/
template<class T>
class CNxRef : public T
{
public:

	/**	\fn		CNxRef()
		\brief	CNxRef Class 생성자
		\param	
		\return 
	*/
    CNxRef()
    {
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__) || defined(__APPLE__)
		atomic_store(&m_lRef, 1);
#else
		m_lRef = 1;
#endif
	}

	/**	\fn		~CNxRef()
		\brief	CNxRef Class 소멸자
		\param	
		\return 
	*/
    virtual ~CNxRef()
    {}

	/**	\fn		long AddRef()
		\brief	레퍼런스 카운트를 증가시킨다.
		\param	
		\return 
	*/
	virtual int AddRef()
	{
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__) || defined(__APPLE__)
		return atomic_fetch_add(&m_lRef, 1);
#else
		return __atomic_inc((int *)&m_lRef) + 1;
#endif
	}

	/**	\fn		long Release()
		\brief	레퍼런스 카운트를 감소시키고 레퍼런스 카운트가 0이면 자신을 소멸시킨다.
		\param	
		\return 
	*/
	virtual int Release()
	{
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__) || defined(__APPLE__)
		int before_dec = atomic_fetch_sub(&m_lRef, 1);
		if (1 == before_dec)
		{
			delete this;
		}
		return before_dec - 1;
#else
		int before_dec = __atomic_dec((int *)&m_lRef);
		if (1 == before_dec)
		{
			delete this;
		}
		return before_dec - 1;
#endif
	}

	/**	\fn		long GetRefCnt()
		\brief	레퍼런스 카운트를 반환한다.
		\param	
		\return 
	*/
	virtual int GetRefCnt(){
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__) || defined(__APPLE__)
		return atomic_load(&m_lRef);
#else
		return m_lRef;
#endif
	};

private:

#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
    
#if defined(__APPLE__)

    atomic<int> m_lRef;

#else

	_Atomic(int) m_lRef;

#endif//defined(__APPLE__)
    
#else

#if defined(__APPLE__)
    
    atomic<int> m_lRef;
    
#else
    
	volatile int	m_lRef;

#endif//defined(__APPLE__)
    
#endif//defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)


};

#endif	//__NEXPLAYER_DEF_H__
