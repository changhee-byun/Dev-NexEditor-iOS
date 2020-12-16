/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_IRef.h
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
#ifndef __NEXVIDEOEDITOR_IREF_H__
#define __NEXVIDEOEDITOR_IREF_H__
//---------------------------------------------------------------------------

class INxRefObj
{
public:
	virtual ~INxRefObj(){}
	/**	\fn		long AddRef()
		\brief	Reference를 증가시킨다.
		\param	
		\return 
	*/
	virtual int AddRef() = 0;

	/**	\fn		long Release()
		\brief	Reference를 감소시킨다.
		\param	
		\return 
	*/
	virtual int Release() = 0;

	/**	\fn		long GetRefCnt()
		\brief	Reference Count를 반환한다.
		\param	
		\return 
	*/
	virtual int GetRefCnt() = 0; //for debug
};

#endif	//__NEXVIDEOEDITOR_IREF_H__
