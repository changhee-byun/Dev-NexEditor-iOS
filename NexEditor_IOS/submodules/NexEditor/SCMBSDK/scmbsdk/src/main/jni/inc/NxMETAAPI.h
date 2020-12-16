/******************************************************************************
   __________________________________________________________________________
  |                                                                          |
  |                                                                          |
  |    	  Copyrightⓒ 2002-2017 NexStreaming Corp. All rights reserved.      |
  |                        http://www.nexstreaming.com                       |
  |                                                                          |
  |__________________________________________________________________________|

*******************************************************************************
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
_______________________________________________________________________________
*
*		File Name			: NxMETAAPI.h
*		Author				: NexStreaming MiddleWare Team (mw@nexstreaming.com)
*		Description			: Header for the NxMETA 
*		Revision History	: 
*		Comment				: 
*		Version				: 0.1.0.O_FFICIAL_20180420
******************************************************************************/
#ifndef _NXMETA_API_H_
#define _NXMETA_API_H_

#include "NexSAL_com.h"
#include "NexSAL_Internal.h"
#include "NexCommonDef.h"
#include "NexMediaDef.h"
#include "NexTypeDef.h"
#include "NexFFErrorDef.h"

#define NXMETA_VERSION_MAJOR		0
#define NXMETA_VERSION_MINOR		1
#define NXMETA_VERSION_PATCH		1
#define	NXMETA_VERSION_BRANCH		"OFFICIAL_20180814"

#define	NXMETA_COMPATIBILITY_NUM	0

#ifdef __cplusplus
	extern "C" {  
#endif	//__cplusplus

typedef NXVOID *(* __NxMETAOpen)(NXCHAR* pFilename, NEXSALFileMode eMode);
typedef NXINT32	(* __NxMETAClose)(NXVOID* hFile);
typedef NXINT32	(* __NxMETASeek)(NXVOID* hFile, NXINT32 iOffSet, NXUINT32 iFromWhere);
typedef NXINT64 (* __NxMETASeek64)(NXVOID* hFile, NXINT64 iOffSet, NXUINT32 iFromWhere);
typedef NXSSIZE	(* __NxMETARead)(NXVOID* hFile, NXINT8* pBuf, NXUSIZE dwSize);
typedef NXSSIZE	(* __NxMETAWrite)(NXVOID* hFile, NXINT8* pBuf, NXUSIZE dwSize);
typedef NXINT64 (* __NxMETASize)(NXVOID* hFile );

typedef struct NxMETAFileAPI
{
	__NxMETAOpen		m_Open;
	__NxMETAClose		m_Close;
	__NxMETASeek		m_Seek;
	__NxMETASeek64		m_Seek64;
	__NxMETARead		m_Read;
	__NxMETAWrite		m_Write;
	__NxMETASize		m_Size;
} NxMETAFileAPI, *pNxMETAFileAPI;

PACKED(
	struct _SCMB_Entry
	{
		NXUINT32	uDivision;
		NXUINT64	uStart;
		NXUINT64	uEnd;
		NXUINT32	nExtLen;
		NXUINT8		*pExt;
	}
);
typedef struct _SCMB_Entry SCMB_Entry;

typedef struct
{
	NXUINT8		uVersion;
	NXUINT32	uEntryCount;
	NXUINT32	uDivScale;
	SCMB_Entry	*pEntrys;
} SCMB_Box;

typedef struct
{
	NXUINT32	nType;
	NXUINT32	nLength;
	NXUINT8		*pBuf;
}NXMT_Entry, *pNXMT_Entry;

typedef struct 
{
	NXUINT8		uVersion;
	NXUINT32	uEntryCount;
	pNXMT_Entry	pEntrys;
}NXMT_Box;

typedef struct _NxMETA
{
	NEXSALFileHandle	pFP;
	NxMETAFileAPI		*pFileAPI;

	NXUINT64			uFileSize;

	NXBOOL				bMetaExistFlag;
	NXUINT64			uMetaOffset;
	NXBOOL				bSCMBExistFlag;
	NXUINT64			uSCMBOffset;
	NXUINT32			uOriginSize4SCMB;
	NXMT_Box			nxmt;
	SCMB_Box			scmb;
} NxMETA, *pNxMETA;

typedef enum
{
	NXMETA_GET_ENTRY	= 1,
	NXMETA_ADD_ENTRY	= 1 << 1,
	NXMETA_DEL_ENTRY	= 1 << 2,
	NXMETA_MOD_ENTRY	= 1 << 3,
	
	NXMETA_PADDING		= 0x7FFFFFFF
} NxMETAInfoType;

const NXCHAR* NxMETA_GetVersionString();

/******************************************************************************
 * Function Name : NxMETASCEntry() 
 * 
 * Return Value Description : NEX_FF_RETURN
 * Comment : GET/ADD/DEL/MOD/UPDATE process for Entry data
******************************************************************************/
NEX_FF_RETURN	NxMETASCEntry(IN pNxMETA pMETA, IN NxMETAInfoType tpInfoType, IN NXINT32 nEntryCount, INOUT SCMB_Entry *pEntry);

// Function
NEX_FF_RETURN	NxMETARegisterFileAPI(IN pNxMETA pMETA, IN pNxMETAFileAPI pFileAPI);

/******************************************************************************
 * Function Name : NxMETACreate() 
 * 
 * Return Value Description : pNxSCMB or NULL
 * Comment : NxMETADestroy() 와 쌍.
******************************************************************************/
pNxMETA			NxMETACreate();

/******************************************************************************
 * Function Name : NxMETADestroy() 
 * 
 * Return Value Description : NEX_FF_RETURN type
 * Comment : NxMETACreate() 와 쌍.
******************************************************************************/
NEX_FF_RETURN	NxMETADestroy(IN pNxMETA pMETA);

/******************************************************************************
 * Function Name : NxMETAInit() 
 * 
 * Return Value Description : NEX_FF_RETURN type
 * Comment :	
******************************************************************************/
NEX_FF_RETURN	NxMETAInit(IN pNxMETA pMETA, IN NXUINT8 *pFileName);

/******************************************************************************
 * Function Name : NxMETAClose() 
 * 
 * Return Value Description : NEX_FF_RETURN type
 * Comment :	
******************************************************************************/
NEX_FF_RETURN	NxMETAClose(IN pNxMETA pMETA);


/******************************************************************************
 * Function Name : NxMETASCExistEntryCheck() 
 * 
 * Return Value Description : NEX_FF_RETURN type
 *							: nRet 값이 -1 -> scmb atom 없음
 *							: 0 -> scmb atom 존재하나 entry 없음
 *							: 1이상 -> scmb atom 존재하며 반환값은 entry count.
 * Comment :	
******************************************************************************/
NEX_FF_RETURN	NxMETASCExistEntryCheck(IN pNxMETA pMETA, INOUT NXINT32 *nRet);


/******************************************************************************
 * Function Name : NxMETASCUpdate() 
 * 
 * Return Value Description : NEX_FF_RETURN type
 * Comment : 변경된 scmb data 를 파일에 업데이트.
******************************************************************************/
NEX_FF_RETURN	NxMETASCUpdate(IN pNxMETA pMETA);



/******************************************************************************
 * Function Name : NxMETAExistEntryCheck() 
 * 
 * Return Value Description : NEX_FF_RETURN type
 *							: nRet 값이 -1 -> NXMT atom없음
 *							: 0 -> NXMT atom 존재하나 entry 없음
 *							: 1이상 -> NXMT atom 존재하며 반환값은 entry count.
 * Comment :	
******************************************************************************/
NEX_FF_RETURN	NxMETAExistEntryCheck(IN pNxMETA pMETA, INOUT NXINT32 *nRet);


/******************************************************************************
 * Function Name : NxMETAEntry() 
 * 
 * Return Value Description : NEX_FF_RETURN
 * Comment : GET/ADD/DEL/MOD/UPDATE process for Entry data
******************************************************************************/
NEX_FF_RETURN	NxMETAEntry(IN pNxMETA pMETA, IN NxMETAInfoType tpInfoType, IN NXINT32 nEntryCount, INOUT NXMT_Entry *pEntry);

/******************************************************************************
 * Function Name : NxMETAUpdate() 
 * 
 * Return Value Description : NEX_FF_RETURN type
 * Comment : 변경된 scmb data 를 파일에 업데이트.
******************************************************************************/
NEX_FF_RETURN	NxMETAUpdate(IN pNxMETA pMETA);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif	//_NXMETA_API_H_