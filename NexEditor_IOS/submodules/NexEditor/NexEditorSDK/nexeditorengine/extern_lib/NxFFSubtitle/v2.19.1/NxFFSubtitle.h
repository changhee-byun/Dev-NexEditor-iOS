/******************************************************************************
   __________________________________________________________________________
  |                                                                          |
  |                                                                          |
  |                         NexStreaming Corporation                         |
  |    	    Copyrightⓒ 2002-2013 All rights reserved to NexStreaming        |
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
*		File Name			: NxFFSubtitle.h
*		Author				: NexStreaming MiddleWare Team (mw@nexstreaming.com)
*		Description			:
*		Revision History	: 
*		Comment				: 
*		Version				: v2.19.1.OFFICIAL (2016.09.29)
******************************************************************************/

#ifndef _NxFFSUBTITLE_H_
#define _NxFFSUBTITLE_H_

#ifdef WIN32
	#pragma warning( disable:4013 )
#endif
#ifdef __cplusplus
extern "C" {  
#endif // __cplusplus

	#include "NexSAL_API.h"
	#include "NexSAL_Internal.h"
	#include "NexTypeDef.h"
	#include "NexMediaDef.h"
	#include "NexFFErrorDef.h"
	#include "nxXMLAPI.h"

	#define NxFFSubtitle_VERSION_MAJOR		2
	#define NxFFSubtitle_VERSION_MINOR		19
	#define NxFFSubtitle_VERSION_PATCH		1
	#define NxFFSubtitle_VERSION_BRANCH		"OFFICIAL"

	#define NxFFSubtitle_COMPATIBILITY_NUM	8

	typedef NXVOID* NxFFSubtitleHandle;

	#define NXFFSUBTITLE_BUFFER_MAX_SIZE			(5 * 1024 * 1024)			

	// Define Supported File Type
	#define NXFFSUBTITLE_SUPPORT_SMI
	#define NXFFSUBTITLE_SUPPORT_SRT
	#define NXFFSUBTITLE_SUPPORT_MICRODVD_SUB
	#define NXFFSUBTITLE_SUPPORT_LRC
	#define NXFFSUBTITLE_SUPPORT_SYLT
	#define NXFFSUBTITLE_SUPPORT_WEB_VTT
	#define NXFFSUBTITLE_SUPPORT_TIMED_TEXT_MARKUP_LNG

#if 0
	typedef	enum
	{
		NxFFSubtitle_ERROR_NONE				= 0,
		NxFFSubtitle_ERROR_INVALID_HANDLE	= -1,
		NxFFSubtitle_ERROR_FILE_IO			= -2,
		NxFFSubtitle_ERROR_FILE_EOF			= -3,
		NxFFSubtitle_ERROR_MEMORY			= -4,
		NxFFSubtitle_ERROR_API_INTERNEL		= -5,
		NxFFSubtitle_ERROR_PARAM			= -6,
		NxFFSubtitle_ERROR_CAPTION_NULL		= -7,
		NxFFSubtitle_ERROR_SET_TIME			= -8,
		NxFFSubtitle_ERROR_SORTING			= -9,
		NxFFSubtitle_ERROR_INVALID_SYNTAX	= -10,
		NxFFSubtitle_ERROR_PARSING_CONTINUE	= -11,	// Error는 맞지만 무시하고 계속 파싱해도 됨
		NxFFSubtitle_ERROR_HAS_MORE_SUBTITLE= -12,
		NxFFSubtitle_ERROR_UNICODE_ENCODING	= -13,
		NxFFSubtitle_ERROR_BUFFER_EOB		= -14,	// End of Buffer
		NxFFSubtitle_ERROR_BUFFER_OVERFLOW	= -15,	// Buffer is smaller than content

		NxFFSubtitle_ERROR_FOR_ALLIGN		= MAX_SIGNED32BIT
	} NxFFSubtitleRet;
#endif

	typedef enum
	{
		NXFFSUBTITLE_ID3TAG_VERSION_1_1_0	= 0x00000000,
		NXFFSUBTITLE_ID3TAG_VERSION_2_2_0	= 0x00000001,
		NXFFSUBTITLE_ID3TAG_VERSION_2_3_0	= 0x00000001 << 1,
		NXFFSUBTITLE_ID3TAG_VERSION_2_4_0	= 0x00000001 << 2
	} NxFFSubtitleID3TagVersion;

	typedef enum
	{
		NxFFSUBTITLE_ENCODING_UNKNOWN			= -1,
		NxFFSUBTITLE_ENCODING_ISO_8859_1		= 0,
		NxFFSUBTITLE_ENCODING_UNICODE			= 1, /* up to ID3v2.3.0 */
		NxFFSUBTITLE_ENCODING_UTF16withBOM		= 2,
		NxFFSUBTITLE_ENCODING_UTF16BEnoBOM		= 3,
		NxFFSUBTITLE_ENCODING_UTF8				= 4, /* up to ID3v2.4.0 */
		NxFFSUBTITLE_ENCODING_ASCII				= 5
	} NxFFSubtitleEncoding;

	///////////////////////////////////////////////////////////////////////////////
	// File API
	// 20140202 
	//typedef NXINT32 (* __NxFFSubtitle_FileOpen)( NXINT8* strFilename, NXINT32 eMode ); return value 수정
	typedef NXVOID* (* __NxFFSubtitle_FileOpen)( NXINT8* strFilename, NEXSALFileMode eMode );
	typedef NXINT32 (* __NxFFSubtitle_FileClose)( NXVOID* hFile );
	typedef NXINT32 (* __NxFFSubtitle_FileSeek)( NXVOID* hFile, NXINT32 iOffSet, NXUINT32 iFromWhere );
	typedef NXINT64	(* __NxFFSubtitle_FileSeek64)(NXVOID* hFile, NXINT64 iOffSet, NXUINT32 iFromWhere);
	typedef NXSSIZE (* __NxFFSubtitle_FileRead)( NXVOID* hFile, NXINT8* pBuf, NXUSIZE dwSize );
	typedef NXSSIZE (* __NxFFSubtitle_FileWrite)( NXVOID* hFile, NXINT8* pBuf, NXUSIZE dwSize );
	typedef NXINT64 (* __NxFFSubtitle_FileSize)( NXVOID* hFile );


	typedef struct NxFFSubtitleFileAPIStruct
	{
		__NxFFSubtitle_FileOpen		m_Open;
		__NxFFSubtitle_FileClose	m_Close;
		__NxFFSubtitle_FileSeek		m_Seek;
		__NxFFSubtitle_FileSeek64	m_Seek64;
		__NxFFSubtitle_FileRead		m_Read;
		__NxFFSubtitle_FileWrite	m_Write;
		__NxFFSubtitle_FileSize		m_Size;
	} NxFFSubtitleFileAPI, *NxFFSubtitleFileAPIPtr;
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// FFInterface(NxFFReader) API
	//==================================================================================
	// MEDIA TYPE. (Frame 을 읽을 수 있는 모든 Media Type)
	//==================================================================================
	typedef enum
	{
		NxFFSUBTITLE_MEDIATYPE_AUDIO					= 0x00000000,
		NxFFSUBTITLE_MEDIATYPE_BASEVIDEO				= 0x00000001,
		NxFFSUBTITLE_MEDIATYPE_TEXT						= 0x00000002,
		NxFFSUBTITLE_MEDIATYPE_ENHANCEDVIDEO			= 0x00000003,
		NxFFSUBTITLE_MEDIATYPE_MAX						= 0x00000004,
		NxFFSUBTITLE_MEDIATYPE_ALL						= 0x000000FF,
		NxFFSUBTITLE_MEDIATYPE_FOR4BYTEALIGN			= MAX_SIGNED32BIT
	} NxFFSubtitleMediaType;

	typedef enum
	{
		// GET Info
		NxFFSUBTITLE_EXTINFO_GET_VIDEO_FPS,
		NxFFSUBTITLE_EXTINFO_GET_VIDEO_FRAME_COUNT,
		NxFFSUBTITLE_EXTINFO_GET_VIDEO_ORIENTATION,		// Video의 Orientation 정보를 얻는다. 0: 0, 90: 1, 180: 2, 270: 3
		NxFFSUBTITLE_EXTINFO_GET_SSTREAMING_TFRFINFO,	// SmoothStreaming에서 TFRF정보를 위로 올려 줄 때
		NxFFSUBTITLE_EXTINFO_GET_REST_DATA_SIZE,		// SHOUTcast에서 마지막이 온전한 Frame으로 구성되지 않았을 경우 나머지 데이터 크기 반환
		NxFFSUBTITLE_EXTINFO_SET_CFF_IMAGESIZE_LIST,
		NxFFSUBTITLE_EXTINFO_GET_TEXT_FIRSTLANG,
		NxFFSUBTITLE_EXTINFO_GET_TEXT_NEXTLANG,
		NxFFSUBTITLE_EXTINFO_GET_TEXT_DEFAULTLANG,

		NxFFSUBTITLE_EXTINFO_PADDING = MAX_SIGNED32BIT
	} NxFFSubtitleExtInfoType;

	typedef struct
	{
		NXUINT8*					m_pFrame;
		NXUINT32					m_dwLen;
		NXUINT32					m_dwCts;
	} NxFFSubtitleFrameInfo, *NxFFSubtitleFrameInfoPtr;

	typedef NEX_FF_RETURN (*NxFFSUBTITLE_FFI_Open)		(NXUINT8*				pFile,						// File path or memory that contains file.
														 NXUINT32				dwFileSize,
														 NxFFSubtitleFileAPI*	pFileAPI,
														 NEX_CODEC_TYPE*		pFileType,
														 NXVOID**				ppFFI);

	typedef NEX_FF_RETURN (*NxFFSUBTITLE_FFI_Close)		(NXVOID*			pFFI);
	

	typedef NEX_FF_RETURN (*NxFFSUBTITLE_FFI_GetFrame)	(NXVOID*			pFFI,
															 NxFFSubtitleMediaType		eMediaType,
															 NxFFSubtitleFrameInfoPtr	pFrameInfo);

	typedef NEX_FF_RETURN (*NxFFSUBTITLE_FFI_GetExtInfo)			(NXVOID*			pFFI,
															 NxFFSubtitleExtInfoType tpInfoType,
															 NXINT64*			pOutput,
															 NXVOID**			pResult);
	typedef NEX_FF_RETURN (*NxFFSUBTITLE_FFI_SetExtInfo)			(NXVOID*			pFFI,
															 NxFFSubtitleExtInfoType tpInfoType,
															 NXINT64*			pInput,
															 NXVOID**			pResult);

	//------------------------------------------------------------
	// FILE FORMAT READER INTERFACE. (FFI)
	//------------------------------------------------------------
	typedef struct
	{
		NxFFSUBTITLE_FFI_Open									m_pOpen;
		NxFFSUBTITLE_FFI_Close									m_pClose;
		NxFFSUBTITLE_FFI_GetFrame								m_pGetFrame;
		NxFFSUBTITLE_FFI_GetExtInfo								m_pGetExtInfo;
		NxFFSUBTITLE_FFI_SetExtInfo								m_pSetExtInfo;
	} NxFFSubtitleFFInterface;
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Inner Structure
	typedef struct
	{
		NEX_CODEC_TYPE		DataType;

		NXUINT32					DataSize;
		NXUINT8					*Data;

#ifdef NXFFSUBTITLE_SUPPORT_SYLT
		NxFFSubtitleID3TagVersion	ID3TagVersion;
		NxFFSubtitleEncoding		Encoding;
		NXUINT8				TimeStampFormat;
		NXUINT8				ContentType;
#endif
	} NxFFSubtitleParam, *NxFFSubtitleParamPtr;
	///////////////////////////////////////////////////////////////////////////////

	typedef struct
	{
		NXINT32					StartTime;
		NXINT32					EndTime;
		NXINT32					NextStartTime;
		NxFFSubtitleEncoding	Encoding;
		NXUSIZE					SubtitleLength;
		NXINT8					*Subtitle;
		NXUINT32						CordinateSize;
		NXINT8					*Cordinate;

#ifdef NXFFSUBTITLE_SUPPORT_SMI
		NXINT8  *ClassID;	// nxSMI.h의 NXSMI_CLASS_ID_LEN에 정의된대로 4byte NXINT8 array
#endif

		NXUINT32	SeqNumber;
	} NxFFSubtitleSubtitle, *NxFFSubtitleSubtitlePtr;

	/** SMI **********************************************************************/
#ifdef NXFFSUBTITLE_SUPPORT_SMI
	typedef struct
	{
		NXUINT32	ClassSize;
		NXUINT8	*(*ClassNames);
	} NxFFSubtitleClassInfo, *NxFFSubtitleClassInfoPtr;
#endif
	/*****************************************************************************/


	/** NxFFSubtitle Essential Function ******************************************/
	NXBOOL NxFFSubtitle_CheckCompatibleVersion(NXINT32 a_nCompatibilityNum);
	NXBOOL NxFFSubtitle_CheckSameVersion(NXINT32 a_nMajor, NXINT32 a_nMinor, NXINT32 a_nPatch, NXCHAR *a_strBranch);
	const NXCHAR* NxFFSubtitle_GetVersionString();
	const NXINT32 NxFFSubtitle_GetMajorVersion();
	const NXINT32 NxFFSubtitle_GetMinorVersion();
	const NXINT32 NxFFSubtitle_GetPatchVersion();
	const NXCHAR* NxFFSubtitle_GetBranchVersion();
	const NXCHAR* NxFFSubtitle_GetVersionInfo();

//	NXINT8* NxFFSubtitle_GetLibraryVersion();

	NxFFSubtitleHandle NxFFSubtitle_Init( const NXINT8 *chFilePath, NXUINT32 nFilePathLen, NxFFSubtitleFileAPIPtr pFileAPI, NXVOID* pInterface );
	NEX_FF_RETURN NxFFSubtitle_Deinit( NxFFSubtitleHandle hHandle );

	NxFFSubtitleHandle NxFFSubtitle_InitMemory( NxFFSubtitleParamPtr pParam );

	NEX_FF_RETURN NxFFSubtitle_PrintFilePointer( NxFFSubtitleHandle hHandle );
	NEX_FF_RETURN NxFFSubtitle_Parsing( NxFFSubtitleHandle hHandle );
	NEX_CODEC_TYPE NxFFSubtitle_GetSubtitleType( NxFFSubtitleHandle hHandle );


	// 아래 API는 FFInterface(NxFFReader)의 GetExtInfo 정보를 호출하기 위한 함수
	// 즉, FFInterface의 GetExtInfo 정보가 필요 없는 대부분의 경우 사용할 필요 없음
	NXINT32 NxFFSubtitleGetExtInfo(NxFFSubtitleHandle hHandle, NxFFSubtitleExtInfoType tpInfoType, NXINT64 *pOutput, NXVOID** pResult);

	NXINT32 NxFFSubtitleSetExtInfo(NxFFSubtitleHandle hHandle, NxFFSubtitleExtInfoType tpInfoType, NXINT64 *pInput, NXVOID** pResult);
	/*****************************************************************************/

	/** NxFFSubtitle *************************************************************/
	NEX_FF_RETURN	NxFFSubtitle_Seek( NxFFSubtitleHandle hHandle, IN NXINT32 lTime );
	NEX_FF_RETURN	NxFFSubtitle_GetMinSyncTime( NxFFSubtitleHandle hHandle, OUT NXUINT32* pSyncTime);
	NEX_FF_RETURN	NxFFSubtitle_GetMaxSyncTime( NxFFSubtitleHandle hHandle, OUT NXUINT32* pSyncTime );
	NEX_FF_RETURN	NxFFSubtitle_GetSubtitleCount( NxFFSubtitleHandle hHandle, OUT NXUINT32* pCount);
	NEX_FF_RETURN	NxFFSubtitle_GetSubtitle( NxFFSubtitleHandle hHandle, NxFFSubtitleSubtitlePtr pSubtitle );
	/*****************************************************************************/

	/** SMI **********************************************************************/
#ifdef NXFFSUBTITLE_SUPPORT_SMI
	NEX_FF_RETURN	NxFFSubtitle_GetClassInfo( NxFFSubtitleHandle hHandle, NxFFSubtitleClassInfoPtr pClassInfo );
#endif
	/*****************************************************************************/

	/** TTML **********************************************************************/
#ifdef NXFFSUBTITLE_SUPPORT_TIMED_TEXT_MARKUP_LNG
	NXVOID NxFFSubtitle_SetImageSizeList( NxFFSubtitleHandle hHandle,NXINT8 *pData, NXINT32 *pImageSizeList );
#endif
	/*****************************************************************************/

	/** NxMicroDVD SUB ***********************************************************/
#ifdef NXFFSUBTITLE_SUPPORT_MICRODVD_SUB
	NXVOID NxFFSubtitle_SetFrameRate( NxFFSubtitleHandle hHandle, NXUINT32 unFrameRate );
#endif //NXFFSUBTITLE_SUPPORT_MICRODVD_SUB
	/*****************************************************************************/

	/** LRC **********************************************************************/
#ifdef NXFFSUBTITLE_SUPPORT_LRC
	NEX_FF_RETURN NxFFSubtitle_GetLRCInfo( NxFFSubtitleHandle hHandle, NXINT32 enumLrcTagID, NxFFSubtitleSubtitlePtr pSubtitle);
										// album 	= 2
										// artist	= 3
										// author	= 4
										// enterby	= 5
										// offset	= 6
										// remarks	= 7
										// title	= 8
										// version	= 9
										//	defined in NxLRCParser.h

	NXVOID NxFFSubtitle_ShiftTimeStampSet( NxFFSubtitleHandle hHandle );
	NXVOID NxFFSubtitle_ShiftTimeStampReset( NxFFSubtitleHandle hHandle );
#endif //NXFFSUBTITLE_SUPPORT_LRC
	/*****************************************************************************/

	/** SYLT *********************************************************************/
#ifdef NXFFSUBTITLE_SUPPORT_SYLT
	NEX_FF_RETURN NxFFSubtitle_GetSYLTInfo( NxFFSubtitleHandle hHandle, NXINT32 enumSYLTInfoID, NxFFSubtitleSubtitlePtr pSubtitle);
										//	NxSYLTInfo_ENCODING		= 1
										//	NxSYLTInfo_LANGUAGE		= 2
										//	NxSYLTInfo_CONTENTTYPE	= 3
										//	NxSYLTInfo_CONTENTDESC	= 4
										//  NxSYLTInfo_FRAMERATE	= 5

//NXVOID NxFFSubtitle_SetFrameRate( NxFFSubtitleHandle hHandle, NXUINT32 unFrameRate ) 조정 필요
// - NxFFSubtitleRet NxSYLTParser_SetFrameRate( NxFFSubtitlePtr hSubtitle, NXUINT32 uFrameRate ) case 추가
#endif //NXFFSUBTITLE_SUPPORT_SYLT
	/*****************************************************************************/

	
	NEX_FF_RETURN NxFFSubtitle_GetValue(NxFFSubtitleHandle hHandle, OUT NXINT64 *pOutput);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _NxFFSUBTITLE_H_
