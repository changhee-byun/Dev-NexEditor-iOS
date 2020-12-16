/******************************************************************************
   __________________________________________________________________________
  |                                                                          |
  |                                                                          |
  |    	  Copyrightⓒ 2002-2018 NexStreaming Corp. All rights reserved.      |
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
*		File Name			: NxFFReaderAPI.h
*		Author				: NexStreaming MiddleWare Team (mw@nexstreaming.com)
*		Description			: Header for the NxFFReader 
*		Revision History	: 
*		Comment				: 
*		Version				: 2.61.01.NEXEDITORSDK_BRANCH_OFFICIAL (2019.02.14)
******************************************************************************/

#ifndef _NXFFREADER_API_H_
#define _NXFFREADER_API_H_

#include "NexCommonDef.h"
#include "NexMediaDef.h"
#include "NexTypeDef.h"
#include "NexFFErrorDef.h"
#include "NexSAL_Internal.h"
#include "NexSAL_API.h"


#include "NxFFDRM.h"
#include "NxFFCodecID.h"
#include "NxFFMNOSpec.h"


#define NXFFREADER_VERSION_MAJOR		2
#define NXFFREADER_VERSION_MINOR		61
#define NXFFREADER_VERSION_PATCH		3
#define	NXFFREADER_VERSION_BRANCH		"NEXEDITORSDK_BRANCH_OFFICIAL"

#define	NXFFREADER_COMPATIBILITY_NUM	17

#if defined(NXFFREADERDLL_EXPORTS)
#		define _NXFFREADERDLL_EXPORTS 	__declspec( dllexport )
#else
#		define _NXFFREADERDLL_EXPORTS
#endif

#ifdef __cplusplus
extern "C" {  
#endif



// File API에서 Error가 발생한 경우 Error 값을 NxFFReader의 errvalFileAPI 변수에 저장하기 위해 아래 Define 값 설정 - KJSIM 110927
#define NXFF_FILE_API_ERROR_CHECK

#define NXFF_READER_MAX_SUPPORT_TRACKS_PER_MEDIA			0x00000008	// 각 Media 별 지원 가능한 Max Multitrack 수
#define NXFF_READER_MAX_SUPPORT_IFRAME_COUNT				20			// NXFF_EXTINFO_GET_IFRAME_INDEX 사용하여 최대 가져올 수 있는 개수


/******************************************************************************
 * Name : Seek mode
 *	
 * Comment : NXFF_RA_MODE_(operation)
******************************************************************************/
#define NXFF_RA_MODE_SEEK							0
#define NXFF_RA_MODE_PREV							1
#define NXFF_RA_MODE_NEXT							2
#define NXFF_RA_MODE_CUR_PREV						11

/******************************************************************************
 * Name : NxFFReader option
 *	
 * Comment : NXFF_READER_OPTION_(operation)
******************************************************************************/
#define NXFF_READER_OPTION_PARSE_ALL_FRAGMENT				0x00000001
#define	NXFF_READER_OPTION_SUPPORT_RA						0x00001000
#define	NXFF_READER_OPTION_SUPPORT_RA_WITHOUT_MFRA			0x00002000
#define NXFF_READER_OPTION_SUPPORT_ADTSAAC_HEADER			0x00004000
#define NXFF_READER_OPTION_PARSE_MOOV						0x00008000	// 3gpp, mp4에서 moov까지만 Parsing되도록 설정
#define NXFF_READER_OPTION_RFC_ONEPASS						0x00010000	// uTube 에서 RFC를 이용할 경우 One Passing parseing 만 하도록 수정
#define NXFF_READER_OPTION_RFC_SKIP_ID3						0x00020000	// RFC를 이용할 경우 MP3 파서에서 ID3 v1.0을 파싱하지 않도록 설정
#define NXFF_READER_OPTION_ASF_RA_WITHOUT_TABLE				0x00040000	// ASF 에서 Seek Table을 사용하지 않고 Seek Seek table이 외부에 있을 때에는 사용한다.
#define NXFF_READER_OPTION_PARSE_WITHOUT_TABLE				0x00080000	// 추후 각 파서들에서 지원할 예정이라 Flag만 만들어 둠.
#define NXFF_READER_OPTION_PIFF_FORCE_MFRA_MODE				0x00100000	// PIFF에서 MOOF를 파싱하지 않고 MFRA를 이용해서 Play하는 mode RFC로 PIFF를 Play 할때 사용해야 한다.
#define NXFF_READER_OPTION_LOCAL_LIKE_PD					0x00200000	// Local 인데 파일이 플레이 중에 증가할 경우. PD 일경우에는 절대로 Setting 되면 안된다. MP4 및 MP3 만 지원.
#define NXFF_READER_OPTION_DNLA_CTMODE						0x00400000	// DNLA Chunked Trandfer Mode 지원
#define NXFF_READER_OPTION_MPEGAUDIO_XINGVBRI_ELIMINATION	0x00800000	// MPEG Audio (MP3) 의 Read frame 시, XING 혹은 VBRI header frame 을 제거하도록 하는 옵션.
#define NXFF_READER_OPTION_MAKE_SEEKBLOCK					0x01000000	// FLAC, OGG 컨텐츠에서 리더 자체적으로 Seek block 을 만들도록 함.(RASeek 오차를 줄이기 위하여 사용)

/******************************************************************************
 * Name : PDstartflag in NxFFReaderStruct
 *	
 * Comment : NXFF_READER_FLAG_(operation)
******************************************************************************/
#define	NXFF_READER_FLAG_PD_NONE					0x00
#define	NXFF_READER_FLAG_PD_GENERAL					0x01
#define	NXFF_READER_FLAG_PD_PV						0x02
#define	NXFF_READER_FLAG_PD_IMODE					0x03
#define NXFF_READER_FLAG_PD_NORMAL					0x04
#define NXFF_READER_FLAG_PD_SHOUTCAST				0x05
#define	NXFF_READER_FLAG_PD_CHECK_IMODE				0xFF

/******************************************************************************
 * Name : Restriction value
 *	
 * Comment : 아래 조건들은 OR 될수 있다. 예를 들어 NXFF_RA_RESTRICTION 값이 5이면 
 *			ex) NXFF_RA_RESTRICTION_LOOK_IFRAME | NXFF_RA_RESTRICTION_PERCENT_SEEK 이다.
 *			NXFF_READER_RESTRICTION_(operation)
******************************************************************************/
#define	NXFF_RA_RESTRICTION_NONE					0x00000000	// Seek 시 I Frame을 항상 Return 한다 일반 적인 경우
#define NXFF_RA_RESTRICTION_LOOK_IFRAME				0x00000001	// Seek 시 I Frame을 Return 하지 못한다. MKV, OGM 에서 Set 되며 
																// AVI 에서는 Seek Table이 없으나 Reader API를 이용해서 Seek Table을 만든경우 Set 된다.
																// MP4에서는 Seek Point 갯수가 2개 이하일때 Set 된다.
#define NXFF_RA_RESTRICTION_ABNORMAL_TABLE			0x00000002	// Seek 시 I Frame을 Return 할 수 도 아닐수도 있는 경우.
																// AVI 에서 Seek table이 완전하지 않을 경우 Set 된다.
#define NXFF_RA_RESTRICTION_PERCENT_SEEK			0x00000004	// Table 없이 Seek를 하는대 Seek Point를 Duration 비율로 찾아 낸다
																// 이 경우 Seek 위치는 정확 하지 않을 수 있다.
#define NXFF_RA_RESTRICTION_SEEK_VIDEO_FIRST 		0x00000008	// A/V 스트림이 모두 있을 경우 항상 Video Seek를 먼저 한 후, Audio를 Seek 해야 함.
#define NXFF_RA_RESTRICTION_NOT_ALLOW_SEEK_IN_DMP	0x00000010	// DMP 일경우 Seek를 허용하지 않는 것이 좋은 경우
#define NXFF_RA_RESTRICTION_AUDIO_UNREALIABLE		0x00000020	// Audio의 Seek 결과가 입력 옵션에 따른 것이 아니라 대략 적인 값 인 경우
																// MKV에서 Setting 된다. 이 경우 Video 시간으로 Audio를 Seek 하면 Seek 결과가 Video 앞 시간이 된다.

#define	NXFF_OPTIONAL_SUPPORT_NONE					0x00000000	// 추가적인 기능을 지원할 수 없는 경우
#define NXFF_OPTIONAL_SUPPORT_READ_OFFSET			0x00000001	// 이 flag가 설정되어 있으면 NxFFR_GetReadOffset() 함수를 지원
#define NXFF_OPTIONAL_SUPPORT_READFRAME_STOP		0x00000002	// 이 flag가 설정되어 있으면 NxFFR_Frame() 함수에서 file read API의 특정 반환 값을 통해 중지하는 기능을 지원
#define NXFF_OPTIONAL_SUPPORT_IFRAMETABLE			0x00000004	// 이 flag가 설정되어 있으면 IFrame Table을 GetExtInfo를 통해서 얻어 올 수 있다.


typedef enum {
	NXFF_MEDIA_TYPE_AUDIO					=	0x00000000,
	NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO		=	0x00000001,
	NXFF_MEDIA_TYPE_TEXT					=	0x00000002,
	NXFF_MEDIA_TYPE_BIFS					=	0x00000003,
	NXFF_MEDIA_TYPE_OD						=	0x00000004,
	NXFF_MEDIA_TYPE_PDCF_STKM				=	0x00000005,
	NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO	=	0x0000000b,
	NXFF_MEDIA_TYPE_SL						=   0x00000016,
	NXFF_MEDIA_TYPE_META					=	0x00000020,			// only for internal....
	NXFF_MEDIA_TYPE_ALL						=	0x6FFFFFFF,
	NXFF_MEDIA_TYPE_DUMMY					=	0x7FFFFFFF
}NXFF_MEDIA_TYPE;


typedef NXVOID *(* __NxFFReaderOpen)(NXCHAR* pFilename, NEXSALFileMode eMode);
typedef NXINT32	(* __NxFFReaderClose)(NXVOID* hFile);
typedef NXINT32	(* __NxFFReaderSeek)(NXVOID* hFile, NXINT32 iOffSet, NXUINT32 iFromWhere);
typedef NXINT64 (* __NxFFReaderSeek64)(NXVOID* hFile, NXINT64 iOffSet, NXUINT32 iFromWhere);
typedef NXSSIZE	(* __NxFFReaderRead)(NXVOID* hFile, NXINT8* pBuf, NXUSIZE dwSize);
typedef NXSSIZE	(* __NxFFReaderWrite)(NXVOID* hFile, NXINT8* pBuf, NXUSIZE dwSize);
typedef NXINT64 (* __NxFFReaderSize)(NXVOID* hFile );
typedef NXINT64 (*__NxFFReaderTimeSeek) (NXVOID* hFile, NXINT64 nTargetCTS, NXINT64 nEndCTS, NXINT64 *pRecCTS);

typedef struct NxFFReaderFileAPI
{
	__NxFFReaderOpen					m_Open;
	__NxFFReaderClose					m_Close;
	__NxFFReaderSeek					m_Seek;
	__NxFFReaderSeek64					m_Seek64;
	__NxFFReaderRead					m_Read;
	__NxFFReaderWrite					m_Write;
	__NxFFReaderSize					m_Size;
	__NxFFReaderTimeSeek				m_TimeSeek;	// RFC를 이용하여 DNLA의 Timebase seek를 할때만 등록
} NxFFReaderFileAPI, *pNxFFReaderFileAPI;

typedef struct NxFFReaderRBuffer{
    NXUINT8		*inbfr;
	NXUINT8		*inbfrstart;
	NXUINT32	total_bits;
	NXUINT32	incnt;
} NxFFReaderRBuffer;

typedef struct NxFFREADER_SUBSAMPLE {
	NXUINT32	uBytesOfClearData;		// number of leading unencrypted bytes in each subSample
	NXUINT32	uBytesOfEncryptedData;	// number of leading encrypted bytes in each subSample
} NxFFREADER_SUBSAMPLE;

typedef struct NxFFREADER_PSSHS {
	NXUINT8*	m_pSystemId;
	NXUINT32	m_dwSystemIdSize;
	NXUINT8*	m_pPsshBox;
	NXUINT32	m_dwPsshBoxSize;
} NxFFREADER_PSSHS;

/******************************************************************************
 * Struct Name : NxFFReaderWBuffer
 *	
 * Comment :	usually used to NxFFR_Readframe() NxFFReaderWBuffer.
******************************************************************************/
typedef struct NxFFReaderWBuffer{
    NXUINT8		*bfr;
	NXUINT8		*bfrstart;
	NXUINT8		outbfr;
	NXINT32		outcnt;
	NXUINT32	bytecnt;
	
	NXUINT32	nCTS;
	NXUINT32	nDTS;
	NXUINT32	nETS;			// End Time Stamp

	NXUINT32	nMaxSize;		// Added for checking frame length
	
	//DRM관련 Media crypto지원
	NXUINT8		pKID[16];		// KID
	NXUINT32	nEncrypted;		//  0 clear 
	NXUINT32	nIVLen;			// initail vactor의 길이
	NXUINT8		pIV[16];		// Initial Vactor for DRM Smooth Streaming 에서만 쓰임
	NXUINT32	uSubSampleMaxCount;
	NXUINT32	uSubSampleCount;
	NxFFREADER_SUBSAMPLE *pstSubSamples;
} NxFFReaderWBuffer;



typedef struct NxFFRStreamInfo {
	NEX_CODEC_TYPE		nOTI;
	NXUINT32			n4CC;
	NXUINT32			nTrackID;
	NXUINT32			nTrackNum;
	NXUINT32			nBitRate;
	NXUINT32			nDSI;
	NXUINT8				*pDSI;
}NxFFRStreamInfo, *pNxFFRStreamInfo;

typedef struct NxFFReaderStruct {
	// 외부에서 Reader Inital 전에 설정이 필요한 member
	NXUINT32		nDLNACTSupport;				// DLNA에서 Chunked Transfer Mode를 지원하기 위한 Flag
												// 기본 조건 (Total duration 과 RASeek 지원 불가)
												// NxFF_RASeekTimeBase() 함수 지원.
	NXUINT32		nFlagOptions;				// NXFF_READER_OPTION_
	NXUINT64		nReaderInitSkipOffset;		// 리더 초기화시 Skip할 수 있는 byte offset 20091202
	NXUINT32		nAudioBaseCTS;				// 특정한 파일 포맷 에서는 init 시 Audio의 Base CTS를 설정 할수 있다.

	// 아래 부터는 외부에서 설정할 필요가 없음.
	NXBOOL			bAudioExist;
	NXBOOL			bVideoExist;
	NXBOOL			bTextExist;
	NXBOOL			bOtherExist;
	
	NxFFReaderFileAPI *pFileAPI;
	NXUINT8			*pMemFileptr;				// Only for Internal Use
	NXUINT32		nMemFileSize;				// Only for Internal Use

	NXUINT32		nSupportFF;
	NXUINT8			*pFilePath;					// Init 함수에서 인자로 받은 FilePath - 110208 KJSIM
	NXUINT32		nFilePathLen;				// Init 함수에서 인자로 받은 FilePathLength - 110208 KJSIM
												// Unicode Path 등을 위해 설정됨
												// 만일 이 값이 설정되지 않으면 기존처럼 strlen 등 함수를 사용
												// 이 값이 설정되어 있으면 이 값 만큼 memcpy해야 함
	NXUINT8			*pIBuf;						// internal Use

	NEX_FILEFORMAT_TYPE	MediaFileFormatType;	// NXMP4FREADER_XX_FF ... 
	NEX_FILEFORMAT_TYPE	MediaFileFormatSubType;	// NXMP4FREADER_XX_FF ... 실제 같은 File Format을 사용하지만 표면적으로 다른 이름을 사용할 때 설정

	NXUINT32		nOptionalSupport;			// 특정 파일 포맷이나 상황에 따라 선택적으로 지원할 수 있는 추가 기능 flag.
	NXUINT32		nSeekRestriction;			// Seek 제약사항 flag 이다.
		
	// for DRM 
	NXFF_DRM_TYPE	DRMType;					// DRM Type
	NXVOID			*pDRMInfo;					// DRM Info. Refer NxFFFDRM.h

	//for MediaCrypto
	NXBOOL			bUseMeidaCrypto;
	NXVOID			*pstMeidaCryptoData;


	// for MNO Spec
	NXFF_MNO_TYPE	MNOType;					// SKDV or.... 
	NXVOID			*pMNOInfo;					// MNO Spec info


	// for video
	NXUINT32		nTSFlag;					// Is Exist Enhanced Layer Video? 
	NXUINT32		nVideoTrackCnt;				// Video Track의 수
	NxFFRStreamInfo hVideoTrackInfo[NXFF_READER_MAX_SUPPORT_TRACKS_PER_MEDIA];
	NXDOUBLE		fVideoFramerates;

	// for audio	
	NXUINT32		nAudioTrackCnt;				// Audio Track의 수
	NxFFRStreamInfo hAudioTrackInfo[NXFF_READER_MAX_SUPPORT_TRACKS_PER_MEDIA];

	NXUINT32		nFramesPerSamples;			// 0 = Unknowen it will be deplicate soon
	NXUINT32		nG711ChannelNum;			// it will be deplicate soon

	// for text
	NXUINT32		nTextTrackCnt;
	NxFFRStreamInfo hTextTrackInfo[NXFF_READER_MAX_SUPPORT_TRACKS_PER_MEDIA];
	NXUINT32		isTextTrackRemain;
	NXUINT32		nTextBufLen;
	NXUINT8			*pTextBuf;

	NXUINT8			*pLinkDatas[13];
	NXUINT32		nLinkDatas[13];				// memset 부분 있음. 값 변경시 체크 요구됨.
	NXUINT32		nMidiDataSize;
	
	// File API에서 반환한 Error Value를 저장하기 위한 변수 - KJSIM 110922
	NXINT64			nFileAPIErrVal;
		
	// for PD
	NXUINT64		nFileSize;
	NXUINT64		nPDfilesize;
	NXVOID*			hPDfh;						// Probe를 위한 File Handle
	NXUINT32		nPDStartFlag;			// NXFF_READER_FLAG_PD_~
	NXUINT32		nPDRECBytes;

	// for PVPD only
	NXUINT32		nPDPartCnt;
	NXUINT32		*pPDByteOffset;
	NXUINT32		nPDRestSize;				// SHOUTcast에서 마지막이 온전한 Frame으로 구성되지 않았을 경우 나머지 데이터 크기

	// Interanl use
	// ID3 Length Parser
	NXVOID			*hID3;						// handle for ID3 Length Parser

	NXUINT32		nAPIsCount;
	NXVOID			*ppAPIs;
	NXVOID			*pAPI;
	NXVOID			*pEAPI;						// Extend API
	NXVOID			*pHM;						// internal use;
	NXVOID			*pSettingValue;				// Setting된 Extra Info
	NXVOID			*pRoot;						// Only For internal Use	
	NXVOID			*hSubtitle;
} NxFFReader, *pNxFFReader;


// return values
#define RET_NXFF_READER_INIT_ONEPASS_FRAGMENT	0x0000000B


/******************************************************************************
 * Function Name : NxFFR_Create() 
 * NxFFReader creat 와 destroy 쌍.
 * Return Value Description
 *	NULL : error
 *	
 * Comment :	IN NXUINT32 nFlgSupportedFileType 변수는 FLAG_NXFF_READER_SUPPORT_XXX 값들의 조합
******************************************************************************/
_NXFFREADERDLL_EXPORTS pNxFFReader NxFFR_Create(IN NXUSIZE nMaxHeapSize, IN NXUINT32 nFlagSupportedFileType, IN NXUINT32 nParam, IN NXVOID *pParam);

/******************************************************************************
 * Function Name : NxFFR_Destroy() 
 * NxFFReader creat 와 destroy 쌍.
 * Return Value Description
 *	 - NEX_FF_RETURN type
 *
 * Comment :	
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_Destroy(IN pNxFFReader pRC);

/******************************************************************************
 * Function Name : NxFFR_SetSupportFF() 
 * Return Value Description : NEX_FF_RETURN type
 *
 * Comment :	Create 후에 Support File Format을 변경하는 경우 사용
 *				IN NXUINT32 nFlgSupportedFileType 변수는 FLAG_NXFF_READER_SUPPORT_XXX 값들의 조합
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_SetSupportFF(IN pNxFFReader pRC, IN NXUINT32 nFlgSupportedFileType);

/******************************************************************************
 * Function Name : NxFFR_PrintSupportFF() 
 * Return Value Description : NXVOID type
 *
 * Comment :	NxFFReader Library에서 지원하도록 설정된 FileFormat List를 Print 하는 함수 - KJSIM 100818
******************************************************************************/
_NXFFREADERDLL_EXPORTS NXVOID NxFFR_PrintSupportFF(IN pNxFFReader pRC);

_NXFFREADERDLL_EXPORTS const NXCHAR* NxFFR_GetVersionString();
_NXFFREADERDLL_EXPORTS const NXINT32 NxFFR_GetMajorVersion();
_NXFFREADERDLL_EXPORTS const NXINT32 NxFFR_GetMinorVersion();
_NXFFREADERDLL_EXPORTS const NXINT32 NxFFR_GetPatchVersion();
_NXFFREADERDLL_EXPORTS const NXCHAR* NxFFR_GetBranchVersion();
_NXFFREADERDLL_EXPORTS const NXCHAR* NxFFR_GetVersionInfo();

_NXFFREADERDLL_EXPORTS NXBOOL NxFFR_CheckSameVersion(NXINT32 a_nMajor, NXINT32 a_nMinor, NXINT32 a_nPatch, NXCHAR *a_strBranch);
_NXFFREADERDLL_EXPORTS NXBOOL NxFFR_CheckCompatibleVersion(NXINT32 a_nCompatibilityNum);

/******************************************************************************
 * Function Name : NxFFR_RegisteFileAPI() 
 * Return Value  : NEX_FF_RETURN tyep
 * 
 * Comment :	외부 API를 사용 해야 할 때에는 NxFFRegisteFileAPI를 호출. (단, NxFFR_RegisteSALFileAPI은 SAL을 사용할때 이용)
 *				SAL에 Seek64 가 포팅되어 있지 않은 경우
 *				반듯이 NxFFRegisteFileAPI로 File access 함수를 등록해주어야 하되 
 *				m_Seek64는 Sal의 일반 Seek 함수로 등록 해야 한다.
 *				NxFFR_UnRegisteFileAPI는 호출하지 않아도 되나 명시적으로 해주는 것이 좋다.
 *				또한 한번 설정한 후 Unregister없에 계속 다시 설정해도 되며, 한번 설정되면 계속 유지 된다.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_RegisteFileAPI(IN pNxFFReader pRC, IN pNxFFReaderFileAPI pFileApi);

/******************************************************************************
 * Function Name : NxFFR_UnRegisteFileAPI() 
 * Return Value  : NEX_FF_RETURN tyep
 * 
 * Comment :
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_UnRegisteFileAPI(IN pNxFFReader pRC);

/******************************************************************************
 * Function Name : NxFFR_RegisteSALFileAPI() 
 * Return Value  : NEX_FF_RETURN tyep
 * 
 * Comment : SAL 사용할 때 사용.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_RegisteSALFileAPI(IN pNxFFReader pRC);

/******************************************************************************
 * Function Name : NxFFR_Init() 
 * NxFFReader Init. 와 Close 쌍.
 * Return Value Description : NEX_FF_RETURN type
 *	
 * Comment :	
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_Init(IN pNxFFReader pRC, IN NXUINT8 *pFilePath, IN NXUINT32 nFilePathLen);

/******************************************************************************
 * Function Name : NxFFR_Close() 
 * NxFFReader Init. 와 Close 쌍.
 * Return Value Description : NEX_FF_RETURN type
 *	
 * Comment :	
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_Close(IN pNxFFReader pRC);


/******************************************************************************
 * Function Name : NxFFR_FastGetFileType() 과 NxFFR_FastGetFileTypeClose() 한쌍임.
 * Return Value Description : NEX_FF_RETURN type
 *	
 * _NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_FastGetFileType(pNxFFReader pRC, NXUINT8 *pFilePath, NXUINT32 nFilePathLen);
 * _NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_FastGetFileTypeClose(pNxFFReader pRC);
 * Comment : 외부에서 NxFFR_Init()  을 하지 않고 Container type(File type and Sub file type) 만을 가져올 수 있도록 함.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_FastGetFileType(IN pNxFFReader pRC, IN NXUINT8 *pFilePath, IN NXUINT32 nFilePathLen);
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_FastGetFileTypeClose(IN pNxFFReader pRC);


/******************************************************************************
 * Function Name : NxFFR_ReadFrame()
 *
 * Return Value Description : NEX_FF_RETURN type
 *	
 * Comment : MediaType (0) Audio (1) Video (2) Text  (3) BIFS  (4) OD  (11) Video(Enh)
 * 			 Audio (, , 1,0), Video(, , 1,1), Text(, ,1,1 or , ,1,0) 
 *			 Text를 위한 별도의 버퍼를 Audio, Video 처럼 생성해주어야 함.
 *			 NxFFReaderSubSample *subSamples을 기본하나 할당해주고 
 *			 NXUINT16	*nSubsamplecount 할당된 크기 만큼 설정
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_ReadFrame(IN pNxFFReader pRC, IN NXFF_MEDIA_TYPE MediaType, IN NXUINT8 bSLFlag, INOUT NxFFReaderWBuffer *pWBuf);

/******************************************************************************
 * Function Name : NxFFR_MediaTimeStamp()
 *
 * Return Value Description : NEX_FF_RETURN type
 *	
 * Comment : Media Type 에 따른 마지막 frame 의 DTS 및 CTS 값을 반환.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_MediaTimeStamp(IN pNxFFReader pRC, IN NXFF_MEDIA_TYPE MediaType, OUT NXUINT32 *pDTS, OUT NXUINT32 *pCTS);



#define	NXFF_READER_READ_ONE_3GPP_TIMED_TEXT_SAMPLE_OK				0x00000000
#define	NXFF_READER_READ_ONE_3GPP_TIMED_TEXT_SAMPLE_EOF				0x00000001
#define	NXFF_READER_READ_ONE_3GPP_TIMED_TEXT_SAMPLE_NO_INFO_INDEX	0x00010000

#if 0
int		NxFFR_Read3GPPTimedTextSample(pNxFFReader pRC, NxFFReaderWBuffer *Buf, NXUINT32 SLpacketheaderflag, NxFFReader3GPPTextSample *sample, NXUINT32 *dts, NXUINT32 *duration, NXUINT32 *index);
int 	NxFFR_Read3GPPTimedTextSampleEnd(pNxFFReader pRC, NxFFReader3GPPTextSample *sample);
#endif


/******************************************************************************
 * Function Name : NxFFR_RASeek()
 * 
 * Return Value Description : NEX_FF_RETURN
 *
 * Comment : IN param and OUT param unit is msec.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_RASeek(IN pNxFFReader pRC, IN NXFF_MEDIA_TYPE MediaType, IN NXUINT32 mode, IN NXUINT32 UserTime, OUT NXUINT32 *pRetTime);

/******************************************************************************
 * Function Name : NxFFR_RASeekAll()
 *
 * Return Value Description : NEX_FF_RETURN
 *
 * Comment : AVI, ASF 에서만 지원 한다.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_RASeekAll(IN pNxFFReader pRC, IN NXUINT32 UserTime, OUT NXUINT32 *pRetTime);

/******************************************************************************
 * Function Name : NxFF_RASeekTimeBase()
 * DNLA 에서 RFC 를 이용한 Timebase Seek 함수.
 * Return Value Description : NEX_FF_RETURN
 *
 * Comment :	- MP4(Audio/Video), MP3, AAC(ADTS), MPEG-TS, ASF container 에서만 지원.
				- unit is msec.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_RASeekTimeBase(IN pNxFFReader pRC, IN NXUINT32 nTCTS, IN NXUINT32 nECTS, OUT NXUINT32 *pVideoTime, OUT NXUINT32 *pAudioTime);

/******************************************************************************
 * Function Name : NxFFR_IsPlayable() 
 * 
 * Return Value Description
 *	TRUE  : can play
 *	FALSE : can not play
 *
 * Comment :	Init 된 Content 가 Play 가 가능한지 여부 판다.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NXBOOL NxFFR_IsPlayable(IN pNxFFReader pRC);

/******************************************************************************
 * Function Name : NxFFR_SeekableCheck() 
 * 
 * Return Value Description
 *	eNEX_FF_RET_SUCCESS			: Seekable
 *	eNEX_FF_RET_UNSUPPORT		: not seekable
 *  eNEX_FF_RET_POLICY_DECISION : 
 *	eNEX_FF_RET_INVALID_PARAM	: Input param error
 *
 * Comment :	
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_SeekableCheck(IN pNxFFReader pRC);

/******************************************************************************
 * Function Name : NxFFR_GetSeekableRange() 
 * 
 * Return Value Description
 *
 * Comment :	
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_GetSeekableRange(IN pNxFFReader pRC, OUT NXINT64 *pSeekAbleStartDuration, OUT NXINT64 *pSeekAbleEndDuration);

/******************************************************************************
 * Function Name : NxFFR_SeekValidCheck() 
 * 
 * Return Value Description
 *
 * Comment : 정상적인 Seek가 가능한지 Check하는 함수
 *			이 함수 호출 후에는 프레임 위치가 0으로 이동할 수 있음
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_SeekValidCheck(IN pNxFFReader pRC, OUT NXBOOL bRetCheck);

/******************************************************************************
 * Function Name : NxFFR_MakeSeekTable() 
 * 
 * Return Value Description
 *
 * Comment : Init 된 Content 가 Seek 가 불 가능 하다면, 가능하게 Seek Table 을 만드는 함수
 *			 nRetRate 는 얼마나 자주 Return 할 것인가... (몇 % 만든후에 Return 할 것인가...)
 *			1 ~ 100 사이에 값
 *			nSeekDuration 는 Seek 간의 시간 간격(ms)
 *			리턴이 0이면 좀더 만들어야 하고, 1 이면 끝났다.
 *			-는 에러... 에러가 나도 play 는 가능하다.. .Seek 가 안될뿐...
 *			AVI와 FLV에서 사용가능 하다.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NXINT32 NxFFR_MakeSeekTable(IN pNxFFReader pRC, IN NXINT32 nRetRate, IN NXINT32 nSeekDuration);

/******************************************************************************
 * Function Name : NxFFR_GetPlayableRange() 
 * 
 * Return Value Description : NEX_FF_RETURN type
 *
 * Comment : 
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_GetPlayableRange(IN pNxFFReader pRC, OUT NXINT64 *pPlayAbleStartDuration, OUT NXINT64 *pPlayAbleEndDuration);

/******************************************************************************
 * Function Name : NxFFR_GetBitrate() 
 * 
 * Return Value Description
 *	
 * Comment : 함수 반환 값이 Bitrate 임. 
 *			반환 값이 0 이면 bitrate 확인이 불가능한 경우임.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NXUINT32 NxFFR_GetBitrate(IN pNxFFReader pRC, IN NXFF_MEDIA_TYPE Mediatype);

/******************************************************************************
 * Function Name : NxFFR_GetMediaDuration() 
 * 
 * Return Value Description
 *
 * Comment : 
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_GetMediaDuration(IN pNxFFReader pRC, IN NXFF_MEDIA_TYPE MediaType, OUT NXUINT32 *puDuration);

/******************************************************************************
 * Function Name : NxFFR_SetBytesOffset() 
 * 
 * Return Value Description
 *	0 : SUCCESS
 *	1 : end address pointer is less than start address pointer
 *	2 : end address pointer is same as or greater than total file size
 *	3 : downloaded parts count is more than the allocated array
 *	4 : received bytes is more than the file size
 *
 * Comment : for PV PD(Progressive download)
******************************************************************************/
_NXFFREADERDLL_EXPORTS NXUINT32 NxFFR_SetBytesOffset(IN pNxFFReader pRC, IN NXUINT64 totalfilesize, IN NXUINT64 startptr, IN NXUINT64 endptr);

/******************************************************************************
 * Function Name : NxFFR_PDReset() 
 * 
 * Return Value Description
 *
 * Comment : 
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_PDReset(IN pNxFFReader pRC);

/******************************************************************************
 * Function Name : NxFFR_PDStartCheckAndFileSizeUpdate() 
 * 
 * Return Value Description
 *	0 : PD can start or already starts
 *	1 : PD can't Start
 *	2 : even though the application is for i-mode (pRC->startflag == 0xFF), input file is not an i-mode file
 *	-1: Input fIle open error
 *	minus: FS error
 *
 * Comment :	Standard progressive download & i-mode
******************************************************************************/
_NXFFREADERDLL_EXPORTS NXINT32 NxFFR_PDStartCheckAndFileSizeUpdate(IN pNxFFReader pRC, IN NXINT8 *pFilePath, IN NXUINT64 nTotalFileSize, IN NXUINT64 nCurrentFileSize);

/******************************************************************************
 * Function Name : NxFFR_PDClose() 
 * 
 * Return Value Description
 *
 * Comment : 
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_PDClose(IN pNxFFReader pRC);

typedef struct {
	NXUINT32	unDisplayImageWidth;	// Aspect ratio 를 위하여 사용.(tkhd box)
	NXUINT32	unDisplayImageHeight;	// Aspect ratio 를 위하여 사용.(tkhd box)
}nxFFRDisplayImageSize, *pNxFFRDisplayImageSize;

typedef struct {
	NXUINT32	nCnt;
	struct {
		NXUINT32 nLen;
		NXUINT32 nTimeStamp;
	}pFinfo[20];
}nxFFRIFrameIdx, *pNxFFRIFrameIdx;

typedef struct {
	pNxFFReaderFileAPI	pFileAPI;
	NXUINT32			nFileName;
	NXUINT8				*pFileName;
}NxFFReaderForcePDModeParam, *pNxFFReaderForcePDModeParam;

typedef struct{
	// for Video
	NXUINT32 nVOTI;
	NXUINT32 nVTimeScale;
	
	NXUINT32 nVDSI;
	NXUINT8 *pVDSI;
	
	// for Audio
	NXUINT32 nAOTI;
	NXUINT32 nATimeScale;
	NXUINT32 nADSI;
	NXUINT8 *pADSI;

	// for text
	// not yet
	NXUINT32 nTOTI;	// Not in use yet...
	NXUINT32 nTTimeScale;
	NXUINT32 nADefalutSampleDuration;
	NXUINT32 nVDefalutSampleDuration;
	NXUINT32 nTDefalutSampleDuration;
}NxFFRMP4MFInfo, *pNxFFRMP4MFInfo;

typedef struct
{
	NXUINT32		nIndex;

	NXUINT8			Reference_Type;				// set to 1 ; the reference is to a segment index('sidx'), otherwise ; the refernce is to media content.
	NXUINT32		Referenced_Size;			// the distance in bytes from the first byte of the referneced item to the first byte of the next referenced item.
	NXUINT32		Subsegment_duration;		// the sum of the this fields in that box is total duration, unit(millisecond)
	NXUINT8			Starts_with_SAP;			// indicates whether the referenced subsegments start with a SAP.
	NXUINT8			SAP_Type;					// indicates a SAP type as specified in Annex I.
	NXUINT32		SAP_delta_time;				// indicates T_SAP of the first SAP/
	NXUINT64		Start_Offset;
	NXUINT64		Start_Time;					// time unit(millisecond)
} S_REF_SIDX;


typedef struct 
{
	NXUINT32	nEntry_count;			// Entry conunter
	NXUINT64	*pSegmentDurationTable;	// segment_duration in ms unit
	NXINT64		*pMediaTimeTable;		// media_time in ms unit
	NXUINT16	*pMediaRateTable;		// media_rate_integer
	NXUINT8		bInited;				// internal Use Only
} S_ELST;

typedef enum
{
	// NEX_FF_RETURN NxFFR_GetExtInfo(IN pNxFFReader pRC, IN NxFFReaderExtInfoType tpInfoType, IN NXINT64 nOption, OUT NXVOID *pOutput, OUT NXVOID** pResult)
	// pOutput의 Data type은 NXINT64 또는 NXDOUBLE이다.
	// GET Info
	NXFF_EXTINFO_GET_VIDEO_FPS = 0x00000001,	// pOutput : Video FPS(NXDOUBLE)
	NXFF_EXTINFO_GET_VIDEO_FRAME_COUNT,			// pOutput : Frame Counts, pRestu : NULL, Video의 Frame 수를 가져올때 사용
	NXFF_EXTINFO_GET_VIDEO_ORIENTATION,			// pOutput : Video의 Orientation 정보. representation of an angle. (0 deg., 90 deg., 180 deg., 270 deg.)
	NXFF_EXTINFO_GET_SSTREAMING_TFRFINFO,		// pOutput : SmoothStreaming에서 TFRF정보를 위로 올려 줄 때
	NXFF_EXTINFO_GET_REST_DATA_SIZE,			// pOutput : Remain Size, SHOUTcast에서 마지막이 온전한 Frame으로 구성되지 않았을 경우 나머지 데이터 크기 반환
	NXFF_EXTONFO_GET_EXTRA_HEADER,				// pResult : Extra Header data, NxFFR_GetEXTRAHeaders 함수 대신 사용, pNxFFAPEEXTRADATAHEADER 혹은 pNxFFOFFEXTRADATAHEADER 참고
	NXFF_EXTINFO_GET_IFRAME_INDEX,				// iFrame의 Size와 CTS를 얻어 온다. 이때 nOption에 시작 TimeStamp를 넣어주면 그 이후 20개의 IFrame을 얻어 온다.

	NXFF_EXTINFO_GET_META_ID3,					// Content내에 ID3를 얻어 올때 사용한다. nOption에는 SLFlag의 유무, pOutput에는 결과를 Return 하고 (ReadOneFrame과 같은 결과), pResult에는 NxFFReaderWBuffer *를 넣어주어야 한다.
	NXFF_EXTINFO_GET_ASF_HeaderDuration,		// pOutput : ASF Total duration, ASF 컨텐츠에서 Header 에 있는 정보만을 가지고 Total duration 을 가져오도록 함.
	NXFF_EXTINFO_GET_HLSINIT_INFO,				// TS에서 Init 정보를 얻어온다. NXFF_EXTINFO_SET_HLSINIT_INFO 에서 사용 된다. pOutput에는 buffer size, pResult는 buffer가 나온다
	NXFF_EXTINFO_GET_VIDEO_LANGUAGE_CODE,		// pOutput : Language code, MP4 컨텐이너에서 MDHD box 에 있는 Language 정보를 가져오는 함수. 현재 MP4, MKV 만 지원.
	NXFF_EXTINFO_GET_AUDIO_LANGUAGE_CODE,		// pOutput : Language code, MP4 컨텐이너에서 MDHD box 에 있는 Language 정보를 가져오는 함수. 현재 MP4, MKV 만 지원.// 
	NXFF_EXTINFO_GET_TEXT_LANGUAGE_CODE,		// pOutput : Language code, MP4 컨텐이너에서 MDHD box 에 있는 Language 정보를 가져오는 함수. 현재 MP4, MKV 만 지원.
	NXFF_EXTINFO_GET_MPEG_DASH_VIDEO_TIMESCALE,	// pOutput : Time scale, DASH 에서 Media type 에 따른 Time scale 값을 얻어 온다.
	NXFF_EXTINFO_GET_MPEG_DASH_AUDIO_TIMESCALE,	// pOutput : Time scale, DASH 에서 Media type 에 따른 Time scale 값을 얻어 온다.
	NXFF_EXTINFO_GET_MPEG_DASH_TEXT_TIMESCALE,	// pOutput : Time scale, DASH 에서 Media type 에 따른 Time scale 값을 얻어 온다.
	NXFF_EXTINFO_GET_TS_OFFSET,					// pOutput : Offset, pResult : NULL, TS 컨텐츠에서 Time Stamp 에 대한 Offset 값을 얻어 온다.
	NXFF_EXTINFO_GET_INDEX_TABLE_INFO,			// Container 의 Index table 에 대한 유무 확인 flag. (현재는 AVI 만 적용)
	NxFF_EXTINFO_GET_TS_OFFSET_COMPARE,			// protocol 이 넘겨준 특정 Offset 과 NXFFReader 가지고 있는 Offset 을 비교를 하여, P_offset > N_offset 이면 TRUE, P_offset <= N_offset 면 FALSE.
	NxFF_EXTINFO_GET_MPEG_TS_FIRST_TIMESTAMP_VIDEO_DTS,	// pOutput : First packet DTS, MPEG-TS Init. 시 Set Video/Audio 시 첫 번째 Packet 의 Time Stamp 값을 저장하고 해당 값을 리턴해줌.
	NxFF_EXTINFO_GET_MPEG_TS_FIRST_TIMESTAMP_VIDEO_PTS,	// pOutput : First packet PTS, MPEG-TS Init. 시 Set Video/Audio 시 첫 번째 Packet 의 Time Stamp 값을 저장하고 해당 값을 리턴해줌.
	NxFF_EXTINFO_GET_MPEG_TS_FIRST_TIMESTAMP_AUDIO_DTS,	// pOutput : First packet DTS, MPEG-TS Init. 시 Set Video/Audio 시 첫 번째 Packet 의 Time Stamp 값을 저장하고 해당 값을 리턴해줌.
	NxFF_EXTINFO_GET_MPEG_TS_FIRST_TIMESTAMP_AUDIO_PTS,	// pOutput : First packet PTS, MPEG-TS Init. 시 Set Video/Audio 시 첫 번째 Packet 의 Time Stamp 값을 저장하고 해당 값을 리턴해줌.
	NxFF_EXTINFO_GET_MP4_VIDEO_HANDLER_NAME,	// pOutput : name length, pResult : name data, nOption : track number, MP4 container 안에 있는 hdlr box 의 name 을 가져오는 함수
	NxFF_EXTINFO_GET_MP4_AUDIO_HANDLER_NAME,	// pOutput : name length, pResult : name data, nOption : track number
	NxFF_EXTINFO_GET_MP4_SUBT_HANDLER_NAME,		// pOutput : name length, pResult : name data, nOption : track number, CFF spec. handler tyep "_SUBT_"
	NXFF_EXTINFO_GET_AUDIO_BASE_CTS,			// pOutput : Audio Base CTS, 외부에서 설정된 Audio Base CTS값을 가져올 때 사용한다.
	NXFF_EXTINFO_GET_VIDEO_SYNC_SAMPLE_COUNT,	// pOutput : Sync sample (I-frame) count,   (MP4 만 지원)
	NXFF_EXTINFO_GET_READ_OFFSET,				// pOutput : Frame Offset, MP4 만 동작.
	NXFF_EXTINFO_GET_MEDIACRYPTO_PSSH,			// 설정된 MEDIACRYPTO_SYSTMEID에 해당하는 PSSH 전달
												// pOutput : size  pResult psshData NxFFreader내부에 있는 포인터 외부에서 값을 복사하여 사용

	NXFF_EXTINFO_GET_MEDIACRYPTO_TENC,			//Dash의 경우 init segment에서 가져옴 
												// Track Encryption Box ,Data pout Size(24byte)  point binary Data NxFFreader내부에 있는 포인터
												// pOutput : size  pResult psshData NxFFreader내부에 있는 포인터 외부에서 값을 복사하여 사용

	
	NXFF_EXTINFO_GET_360_METADATA_DATA,			//pOutput : Data Size(NXINT64), pResult : Data(NXCHAR), nOption : Track Number 360 METADATA DATA를 가져온다.

	NXFF_EXTINFO_GET_MPEG_DASH_VIDEO_DEFAULT_SAMPLE_DURATION,	// pOutput : Default sample duration, DASH 에서 Media type 에 따른 _DEFAULT_SAMPLE_DURATION 값을 얻어 온다.
	NXFF_EXTINFO_GET_MPEG_DASH_AUDIO_DEFAULT_SAMPLE_DURATION,
	NXFF_EXTINFO_GET_MPEG_DASH_TEXT_DEFAULT_SAMPLE_DURATION,
	
	NXFF_EXTINFO_GET_SIDX_COUNT,				//pOutput : SIDX Conts.	
	NXFF_EXTINFO_GET_SIDX,						//pOutput : SIDX referenced counts, pResult : Ref. SIDX

	NXFF_EXTINFO_GET_EDTSBOX,					// Only use MP4 container.
	NXFF_EXTINFO_GET_CHECK_EXIST_PTS,			// PTS of content exist check. - MP4 만 지원.
	NXFF_EXTINFO_GET_DISPLAYIMAGESIZE,			// nOption : Video의 트랙 번호, pResult : pNxFFRDisplayImageSize struct 
	NXFF_EXTINFO_GET_AUDIO_BIT_RATE_MODE,		// nOption : NULL, pOutput : 결과 체크(0 : CBR mode, 1 : VBR mode) // 현재 MP3(audio only) 콘텐츠만 지원.



	//NEX_FF_RETURN NxFFR_SetExtInfo(IN pNxFFReader pRC, IN NxFFReaderExtInfoType tpInfoType, IN NXINT64 nInput, INOUT NXVOID* pParam)	
	// SET Info
	NXFF_EXTINFO_SET_AUDIO_BASE_CTS = 0x40000001,	// pParam : NULL, nInput : CTS(NXUINT32)
	NXFF_EXTINFO_SET_ASF_EXTINDEX,					// nInput : File Path Len, pParam : File Path	ASF에서 index를 외부에서 file로 줄 때 사용, 해당 값을 더이상 사용하지 않을 경우 삭제.
	NXFF_EXTINFO_SET_SHOUTCAST_OGG_FILE,			// pParam : File Path
	NXFF_EXTINFO_SET_ASF_FORCEPERCENTSEEK,		// ASF에서강제로Percent Seek모드로동작하도록설정 - 20110829 lovehis
	NXFF_EXTINFO_SET_MP4_FORCEPDMODE,			// pParam : pPDParam, MP4 에서 Init후 PD형태로 Play 하게 동작 하도록 함. 이때 NxFFR_SetExtInfo 에서 nInput은 NXFF_READER_FLAG_PD_GENERAL 을 넣어야 하고,
												// pParam은 pNxFFReaderForcePDModeParam를 넣어줌
	//NXFF_EXTINFO_SET_MP4FRAGMENT_TIMESCALE,	// MP4 Fragment에서 TimeScale값을 설정.
	NXFF_EXTINFO_SET_MP4FRAGMENT_INFO,			// nInput : NXFF_MEDIA_TYPE, pParam : pNxFFRMP4MFInfo,												
												// pParam에 pNxFFRMP4MFInfo을 넣어 준다. Init후에 세팅해야 한다.
	NXFF_EXTINFO_SET_HLSINIT_INFO,				// nInput HLS Init data size(unsigned int), pParam : HLS Init Data(NXUINT8*) HLS에서 i-Frame Only track일때 Init 정보를 넣어 준다. Reader Init 전에 호출 해야 하며, Close 되면 지워진다.
	NXFF_EXTINFO_SET_TS_OFFSET,					// nInput : TS Offset(NXINT64), TS 컨텐츠에서 Time Stamp 에 대한 Offset 값을 외부에서 set 해준다.

	NXFF_EXTINFO_SET_SUBTITLE_FILE_NAME,		// pParam : Subtitle file path      외부 자막 파일 설정
	NXFF_EXTINFO_SET_SUBTITLE_MEMORY,			// pParam : Subtitle Memory address 외부 자막 메모리 설정
	NXFF_EXTINFO_SET_HLS_FRAMEDECRYPTOR,		// pParam : MEDIACRYPTO 정보, HLS에서 FRAME을 Decrypting를 설정 할 때. init 전에 해야 한다.

	NXFF_EXTINFO_SET_USE_MEDIACRYPTO,			// nInput : 0 or 1, MEDIACRYPTO에서 FRAME정보에 을 Decrypting를 설정 할 때. init 전에 해야 한다. 추가적으로 systemID를 전달
	
	NXFF_EXTINFO_SET_MEDIACRYPTO_PSSH,			//외부에서 PSSH설정  
												//nInput : size  pParam: psshdata point

	NXFF_EXTINFO_SET_MEDIACRYPTO_SYSTMEID,		// UUID만 우선 지원 DRM SYSTMEID 설정 
												//nInput : size  pParam: psshdata point

	NXFF_EXTINFO_SET_MEDIACRYPTO_TENC,			// Dash의 경우 init segment에서 가져온 데이타를 framesegment에 전달
												// NXFF_EXTINFO_GET_MEDIACRYPTO_TENC 와 쌍으로 구성
												// nInput : size(24byte)  pParam: Tenc data point
	
	NXFF_EXTINFO_SET_MEDIACRYPTO_KID,			// 16byte 외부에서 default_keyID 설정 
												//nInput : size(16byte)  pParam: KIDdata point
	
	NXFF_EXTINFO_SET_SIDX,						//pParam : SIDX data File Path
	NXFF_EXTINFO_SET_SIDX_OFFSET,				//nInput : SIDX Offset(dafault : 0), pParam	: NULL
	NXFF_EXTINFO_SET_NEEDSIDXBOX,				//pParam : NULL, nInput : 1 or 0, SIDX box 사용 여부 결정				
	
	NXFF_EXTINFO_SET_AUDIODURATIONCHECK,		//pParam : NULL, nInput : 1 or 0, Audio VBR 컨텐츠의 모든 Frame의 duration을 계산하도록 설정가능. (현재 MP3 만 지원.), NxFFR_Init후 호출

	NXFF_EXTINFO_SET_TIMESTAMP_CALC,			// Only use MP4 container. 

	NXFF_EXTINFO_SET_MEDIAFILETYPE,				// MediaFileFormatType 설정. (nInput : NEX_FILEFORMAT_TYPE 입력.)
	NXFF_EXTINFO_SET_MEDIAFILESUBTYPE,			// MediaFileFormatSubType 설정. (nInput : NEX_FILEFORMAT_TYPE 입력.)
	
	NXFF_EXTINFO_PADDING = 0x7FFFFFFF
} NxFFReaderExtInfoType;


/******************************************************************************
 * Function Name : NxFFR_GetExtInfo() 
 * 
 * Return Value Description : NEX_FF_RETURN
 *
 * Comment :	NxFFReaderExtInfoType을 정의 할 때 특별한 명시가 없으면 pOutput은 Data type은 signed 64bit 이다.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_GetExtInfo(IN pNxFFReader pRC, IN NxFFReaderExtInfoType tpInfoType, IN NXINT64 nOption, OUT NXVOID *pOutput, OUT NXVOID** pResult);

/******************************************************************************
 * Function Name : NxFFR_SetExtInfo() 
 * 
 * Return Value Description : NEX_FF_RETURN
 *
 * Comment :	NxFFR_SetExtInfo 함수는 ReaderInit전에 불러도 되지만 부른 후에는 꼭 ReaderClose를 해주어야 한다.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_SetExtInfo(IN pNxFFReader pRC, IN NxFFReaderExtInfoType tpInfoType, IN NXINT64 nInput, INOUT NXVOID* pParam);

/******************************************************************************
 * Function Name : NxFFR_GetWAVEFormatEX() 
 * 
 * Return Value Description
 *
 * Comment :Audio의 Wave format을 뽑기위한 함수, 추후에 MULTI track를 지원해야 한다.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_WAVEFORMATEX *NxFFR_GetWAVEFormatEX(IN pNxFFReader pRC, IN NXUINT32 nTrackNumber);


/******************************************************************************
 * Function Name : NxFFR_GetBITMAPINFOHEADER() 
 * 
 * Return Value Description
 *
 * Comment :
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_BITMAPINFOHEADER *NxFFR_GetBITMAPINFOHEADER(IN pNxFFReader pRC, IN NXUINT32 nTrackNumber);


//APE ExtraData전달을 위한 헤더 
typedef struct
{
    NXINT16		fileversion;
	NXUINT16	compressiontype;
    NXUINT32	blocksperframe;
    NXUINT32	finalframeblocks;
    NXUINT32	totalframes;
    NXUINT16	channels;
    NXUINT32	samplerate;
    NXUINT16	bps;
}NxFFAPEEXTRADATAHEADER, *pNxFFAPEEXTRADATAHEADER;

//OGG ExtraData전달을 위한 헤더 
typedef struct
{
	NXUINT32	HeaderSize;
	NXUINT8		*HeaderData;
}NxFFOGGEXTRADATAHEADER, *pNxFFOGGEXTRADATAHEADER;

// Media의 Track을 변경하는 API
// 변경후에는 Sync를 맞추기 위해 그 Media에 대한 Seek를 해야한다.
/******************************************************************************
 * Function Name : NxFFR_ChangeTrack()
 * Multi-Track contents 에 대한 Track change
 * Return Value Description
 *   Success (동일 트랙에 대해서)			: eNEX_FF_RET_SUCCESS
 *	 Success (서로 다른 트랙에 대해서)		: eNEX_FF_RET_HAS_NO_EFFECT
 *	 Error									: 이외 error value
 * 
 * Comment :	- Track change 가 success 이면, RASeek 를 수행해야함. 단, Success 1 인 경우만 RASeek 수행.
				- success return value 가 나눠진 이유는 track change 에 대한 시나리오의 구분이 필요하기 때문.
				- 동일 트랙으로 Track change 시도시 success 리턴. (외부에서는 동일트랙인지 다른 트랙인지 구분할 수 없음.)
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_ChangeTrack(IN pNxFFReader pRC, IN NXFF_MEDIA_TYPE MediaType, IN NXUINT32 nTrackNumber);

/******************************************************************************
 * Function Name : NxFFR_Reset() 
 * 
 * Return Value Description : NEX_FF_RETURN
 *
 * Comment :
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_Reset(IN pNxFFReader pRC);

/******************************************************************************
 * Function Name : NxFFR_GetUseMemory() 
 * 
 * Return Value Description : NXUSIZE
 *
 * Comment :
******************************************************************************/
_NXFFREADERDLL_EXPORTS NXUSIZE NxFFR_GetUseMemory(IN pNxFFReader pRC);


/******************************************************************************
 * Function Name : NxFFR_GetCodecID() 
 * 
 * Return Value Description : NEX_CODEC_TYPE
 *
 * Comment : 외부에서 4CC 값 및 Compression ID로 NxFFReader에 정의된 Codec ID를 구할 때 사용하는 함수
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_CODEC_TYPE NxFFR_GetCodecID(IN NEX_FILEFORMAT_TYPE tpFileFormat, IN NXFF_MEDIA_TYPE tpMediaType, IN NXUINT32 nCodec4CC);


#ifdef _FEATURE_SKDV_SUPPORT_
// for SK
_NXFFREADERDLL_EXPORTS NXINT32 NxFFR_SKMMidiWrite(pNxFFReader pRC, NXINT8 *pMidiFileName);

// for SKDV
#define NXFF_READER_TRACK_MOVE_OK									0x00000000
#define NXFF_READER_TRACK_MOVE_ERROR_INVALID_NxFFReaderStruct		0x00000001
#define NXFF_READER_TRACK_MOVE_ERROR_INVALID_INDEX					0x0000000A
#define NXFF_READER_TRACK_MOVE_ERROR_INVALID_MEDIA					0x0000000B
#define NXFF_READER_TRACK_MOVE_ERROR_INVALID_TRACKID_DETECTION		0x0000000C
int NxFFR_SKDVTrackMove(pNxFFReader pRC,  NXUINT32 UserTime, NXUINT32 TrackIndex, NXFF_MEDIA_TYPE MediaType);

#define NXFF_READER_CHAPTER_MOVE_OK									0x00000000
#define NXFF_READER_CHAPTER_MOVE_ERROR_INVALID_NxFFReaderStruct		0x00000001
#define NXFF_READER_CHAPTER_MOVE_ERROR_INVALID_FILE					0x00000002
#define NXFF_READER_CHAPTER_MOVE_ERROR_INVALID_INDEX				0x0000000A
int NxFFR_SKDVChapterMove(pNxFFReader pRC, NXUINT32 ChapterIndex);
#endif // _FEATURE_SKDV_SUPPORT_

#ifdef __cplusplus
}
#endif

#endif //_NXFFREADER_API_H_
