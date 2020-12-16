/******************************************************************************
   __________________________________________________________________________
  |                                                                          |
  |                                                                          |
  |    	  Copyright�� 2002-2018 NexStreaming Corp. All rights reserved.      |
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



// File API���� Error�� �߻��� ��� Error ���� NxFFReader�� errvalFileAPI ������ �����ϱ� ���� �Ʒ� Define �� ���� - KJSIM 110927
#define NXFF_FILE_API_ERROR_CHECK

#define NXFF_READER_MAX_SUPPORT_TRACKS_PER_MEDIA			0x00000008	// �� Media �� ���� ������ Max Multitrack ��
#define NXFF_READER_MAX_SUPPORT_IFRAME_COUNT				20			// NXFF_EXTINFO_GET_IFRAME_INDEX ����Ͽ� �ִ� ������ �� �ִ� ����


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
#define NXFF_READER_OPTION_PARSE_MOOV						0x00008000	// 3gpp, mp4���� moov������ Parsing�ǵ��� ����
#define NXFF_READER_OPTION_RFC_ONEPASS						0x00010000	// uTube ���� RFC�� �̿��� ��� One Passing parseing �� �ϵ��� ����
#define NXFF_READER_OPTION_RFC_SKIP_ID3						0x00020000	// RFC�� �̿��� ��� MP3 �ļ����� ID3 v1.0�� �Ľ����� �ʵ��� ����
#define NXFF_READER_OPTION_ASF_RA_WITHOUT_TABLE				0x00040000	// ASF ���� Seek Table�� ������� �ʰ� Seek Seek table�� �ܺο� ���� ������ ����Ѵ�.
#define NXFF_READER_OPTION_PARSE_WITHOUT_TABLE				0x00080000	// ���� �� �ļ��鿡�� ������ �����̶� Flag�� ����� ��.
#define NXFF_READER_OPTION_PIFF_FORCE_MFRA_MODE				0x00100000	// PIFF���� MOOF�� �Ľ����� �ʰ� MFRA�� �̿��ؼ� Play�ϴ� mode RFC�� PIFF�� Play �Ҷ� ����ؾ� �Ѵ�.
#define NXFF_READER_OPTION_LOCAL_LIKE_PD					0x00200000	// Local �ε� ������ �÷��� �߿� ������ ���. PD �ϰ�쿡�� ����� Setting �Ǹ� �ȵȴ�. MP4 �� MP3 �� ����.
#define NXFF_READER_OPTION_DNLA_CTMODE						0x00400000	// DNLA Chunked Trandfer Mode ����
#define NXFF_READER_OPTION_MPEGAUDIO_XINGVBRI_ELIMINATION	0x00800000	// MPEG Audio (MP3) �� Read frame ��, XING Ȥ�� VBRI header frame �� �����ϵ��� �ϴ� �ɼ�.
#define NXFF_READER_OPTION_MAKE_SEEKBLOCK					0x01000000	// FLAC, OGG ���������� ���� ��ü������ Seek block �� ���鵵�� ��.(RASeek ������ ���̱� ���Ͽ� ���)

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
 * Comment : �Ʒ� ���ǵ��� OR �ɼ� �ִ�. ���� ��� NXFF_RA_RESTRICTION ���� 5�̸� 
 *			ex) NXFF_RA_RESTRICTION_LOOK_IFRAME | NXFF_RA_RESTRICTION_PERCENT_SEEK �̴�.
 *			NXFF_READER_RESTRICTION_(operation)
******************************************************************************/
#define	NXFF_RA_RESTRICTION_NONE					0x00000000	// Seek �� I Frame�� �׻� Return �Ѵ� �Ϲ� ���� ���
#define NXFF_RA_RESTRICTION_LOOK_IFRAME				0x00000001	// Seek �� I Frame�� Return ���� ���Ѵ�. MKV, OGM ���� Set �Ǹ� 
																// AVI ������ Seek Table�� ������ Reader API�� �̿��ؼ� Seek Table�� ������ Set �ȴ�.
																// MP4������ Seek Point ������ 2�� �����϶� Set �ȴ�.
#define NXFF_RA_RESTRICTION_ABNORMAL_TABLE			0x00000002	// Seek �� I Frame�� Return �� �� �� �ƴҼ��� �ִ� ���.
																// AVI ���� Seek table�� �������� ���� ��� Set �ȴ�.
#define NXFF_RA_RESTRICTION_PERCENT_SEEK			0x00000004	// Table ���� Seek�� �ϴ´� Seek Point�� Duration ������ ã�� ����
																// �� ��� Seek ��ġ�� ��Ȯ ���� ���� �� �ִ�.
#define NXFF_RA_RESTRICTION_SEEK_VIDEO_FIRST 		0x00000008	// A/V ��Ʈ���� ��� ���� ��� �׻� Video Seek�� ���� �� ��, Audio�� Seek �ؾ� ��.
#define NXFF_RA_RESTRICTION_NOT_ALLOW_SEEK_IN_DMP	0x00000010	// DMP �ϰ�� Seek�� ������� �ʴ� ���� ���� ���
#define NXFF_RA_RESTRICTION_AUDIO_UNREALIABLE		0x00000020	// Audio�� Seek ����� �Է� �ɼǿ� ���� ���� �ƴ϶� �뷫 ���� �� �� ���
																// MKV���� Setting �ȴ�. �� ��� Video �ð����� Audio�� Seek �ϸ� Seek ����� Video �� �ð��� �ȴ�.

#define	NXFF_OPTIONAL_SUPPORT_NONE					0x00000000	// �߰����� ����� ������ �� ���� ���
#define NXFF_OPTIONAL_SUPPORT_READ_OFFSET			0x00000001	// �� flag�� �����Ǿ� ������ NxFFR_GetReadOffset() �Լ��� ����
#define NXFF_OPTIONAL_SUPPORT_READFRAME_STOP		0x00000002	// �� flag�� �����Ǿ� ������ NxFFR_Frame() �Լ����� file read API�� Ư�� ��ȯ ���� ���� �����ϴ� ����� ����
#define NXFF_OPTIONAL_SUPPORT_IFRAMETABLE			0x00000004	// �� flag�� �����Ǿ� ������ IFrame Table�� GetExtInfo�� ���ؼ� ��� �� �� �ִ�.


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
	__NxFFReaderTimeSeek				m_TimeSeek;	// RFC�� �̿��Ͽ� DNLA�� Timebase seek�� �Ҷ��� ���
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
	
	//DRM���� Media crypto����
	NXUINT8		pKID[16];		// KID
	NXUINT32	nEncrypted;		//  0 clear 
	NXUINT32	nIVLen;			// initail vactor�� ����
	NXUINT8		pIV[16];		// Initial Vactor for DRM Smooth Streaming ������ ����
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
	// �ܺο��� Reader Inital ���� ������ �ʿ��� member
	NXUINT32		nDLNACTSupport;				// DLNA���� Chunked Transfer Mode�� �����ϱ� ���� Flag
												// �⺻ ���� (Total duration �� RASeek ���� �Ұ�)
												// NxFF_RASeekTimeBase() �Լ� ����.
	NXUINT32		nFlagOptions;				// NXFF_READER_OPTION_
	NXUINT64		nReaderInitSkipOffset;		// ���� �ʱ�ȭ�� Skip�� �� �ִ� byte offset 20091202
	NXUINT32		nAudioBaseCTS;				// Ư���� ���� ���� ������ init �� Audio�� Base CTS�� ���� �Ҽ� �ִ�.

	// �Ʒ� ���ʹ� �ܺο��� ������ �ʿ䰡 ����.
	NXBOOL			bAudioExist;
	NXBOOL			bVideoExist;
	NXBOOL			bTextExist;
	NXBOOL			bOtherExist;
	
	NxFFReaderFileAPI *pFileAPI;
	NXUINT8			*pMemFileptr;				// Only for Internal Use
	NXUINT32		nMemFileSize;				// Only for Internal Use

	NXUINT32		nSupportFF;
	NXUINT8			*pFilePath;					// Init �Լ����� ���ڷ� ���� FilePath - 110208 KJSIM
	NXUINT32		nFilePathLen;				// Init �Լ����� ���ڷ� ���� FilePathLength - 110208 KJSIM
												// Unicode Path ���� ���� ������
												// ���� �� ���� �������� ������ ����ó�� strlen �� �Լ��� ���
												// �� ���� �����Ǿ� ������ �� �� ��ŭ memcpy�ؾ� ��
	NXUINT8			*pIBuf;						// internal Use

	NEX_FILEFORMAT_TYPE	MediaFileFormatType;	// NXMP4FREADER_XX_FF ... 
	NEX_FILEFORMAT_TYPE	MediaFileFormatSubType;	// NXMP4FREADER_XX_FF ... ���� ���� File Format�� ��������� ǥ�������� �ٸ� �̸��� ����� �� ����

	NXUINT32		nOptionalSupport;			// Ư�� ���� �����̳� ��Ȳ�� ���� ���������� ������ �� �ִ� �߰� ��� flag.
	NXUINT32		nSeekRestriction;			// Seek ������� flag �̴�.
		
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
	NXUINT32		nVideoTrackCnt;				// Video Track�� ��
	NxFFRStreamInfo hVideoTrackInfo[NXFF_READER_MAX_SUPPORT_TRACKS_PER_MEDIA];
	NXDOUBLE		fVideoFramerates;

	// for audio	
	NXUINT32		nAudioTrackCnt;				// Audio Track�� ��
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
	NXUINT32		nLinkDatas[13];				// memset �κ� ����. �� ����� üũ �䱸��.
	NXUINT32		nMidiDataSize;
	
	// File API���� ��ȯ�� Error Value�� �����ϱ� ���� ���� - KJSIM 110922
	NXINT64			nFileAPIErrVal;
		
	// for PD
	NXUINT64		nFileSize;
	NXUINT64		nPDfilesize;
	NXVOID*			hPDfh;						// Probe�� ���� File Handle
	NXUINT32		nPDStartFlag;			// NXFF_READER_FLAG_PD_~
	NXUINT32		nPDRECBytes;

	// for PVPD only
	NXUINT32		nPDPartCnt;
	NXUINT32		*pPDByteOffset;
	NXUINT32		nPDRestSize;				// SHOUTcast���� �������� ������ Frame���� �������� �ʾ��� ��� ������ ������ ũ��

	// Interanl use
	// ID3 Length Parser
	NXVOID			*hID3;						// handle for ID3 Length Parser

	NXUINT32		nAPIsCount;
	NXVOID			*ppAPIs;
	NXVOID			*pAPI;
	NXVOID			*pEAPI;						// Extend API
	NXVOID			*pHM;						// internal use;
	NXVOID			*pSettingValue;				// Setting�� Extra Info
	NXVOID			*pRoot;						// Only For internal Use	
	NXVOID			*hSubtitle;
} NxFFReader, *pNxFFReader;


// return values
#define RET_NXFF_READER_INIT_ONEPASS_FRAGMENT	0x0000000B


/******************************************************************************
 * Function Name : NxFFR_Create() 
 * NxFFReader creat �� destroy ��.
 * Return Value Description
 *	NULL : error
 *	
 * Comment :	IN NXUINT32 nFlgSupportedFileType ������ FLAG_NXFF_READER_SUPPORT_XXX ������ ����
******************************************************************************/
_NXFFREADERDLL_EXPORTS pNxFFReader NxFFR_Create(IN NXUSIZE nMaxHeapSize, IN NXUINT32 nFlagSupportedFileType, IN NXUINT32 nParam, IN NXVOID *pParam);

/******************************************************************************
 * Function Name : NxFFR_Destroy() 
 * NxFFReader creat �� destroy ��.
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
 * Comment :	Create �Ŀ� Support File Format�� �����ϴ� ��� ���
 *				IN NXUINT32 nFlgSupportedFileType ������ FLAG_NXFF_READER_SUPPORT_XXX ������ ����
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_SetSupportFF(IN pNxFFReader pRC, IN NXUINT32 nFlgSupportedFileType);

/******************************************************************************
 * Function Name : NxFFR_PrintSupportFF() 
 * Return Value Description : NXVOID type
 *
 * Comment :	NxFFReader Library���� �����ϵ��� ������ FileFormat List�� Print �ϴ� �Լ� - KJSIM 100818
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
 * Comment :	�ܺ� API�� ��� �ؾ� �� ������ NxFFRegisteFileAPI�� ȣ��. (��, NxFFR_RegisteSALFileAPI�� SAL�� ����Ҷ� �̿�)
 *				SAL�� Seek64 �� ���õǾ� ���� ���� ���
 *				�ݵ��� NxFFRegisteFileAPI�� File access �Լ��� ������־�� �ϵ� 
 *				m_Seek64�� Sal�� �Ϲ� Seek �Լ��� ��� �ؾ� �Ѵ�.
 *				NxFFR_UnRegisteFileAPI�� ȣ������ �ʾƵ� �ǳ� ��������� ���ִ� ���� ����.
 *				���� �ѹ� ������ �� Unregister���� ��� �ٽ� �����ص� �Ǹ�, �ѹ� �����Ǹ� ��� ���� �ȴ�.
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
 * Comment : SAL ����� �� ���.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_RegisteSALFileAPI(IN pNxFFReader pRC);

/******************************************************************************
 * Function Name : NxFFR_Init() 
 * NxFFReader Init. �� Close ��.
 * Return Value Description : NEX_FF_RETURN type
 *	
 * Comment :	
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_Init(IN pNxFFReader pRC, IN NXUINT8 *pFilePath, IN NXUINT32 nFilePathLen);

/******************************************************************************
 * Function Name : NxFFR_Close() 
 * NxFFReader Init. �� Close ��.
 * Return Value Description : NEX_FF_RETURN type
 *	
 * Comment :	
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_Close(IN pNxFFReader pRC);


/******************************************************************************
 * Function Name : NxFFR_FastGetFileType() �� NxFFR_FastGetFileTypeClose() �ѽ���.
 * Return Value Description : NEX_FF_RETURN type
 *	
 * _NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_FastGetFileType(pNxFFReader pRC, NXUINT8 *pFilePath, NXUINT32 nFilePathLen);
 * _NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_FastGetFileTypeClose(pNxFFReader pRC);
 * Comment : �ܺο��� NxFFR_Init()  �� ���� �ʰ� Container type(File type and Sub file type) ���� ������ �� �ֵ��� ��.
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
 *			 Text�� ���� ������ ���۸� Audio, Video ó�� �������־�� ��.
 *			 NxFFReaderSubSample *subSamples�� �⺻�ϳ� �Ҵ����ְ� 
 *			 NXUINT16	*nSubsamplecount �Ҵ�� ũ�� ��ŭ ����
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_ReadFrame(IN pNxFFReader pRC, IN NXFF_MEDIA_TYPE MediaType, IN NXUINT8 bSLFlag, INOUT NxFFReaderWBuffer *pWBuf);

/******************************************************************************
 * Function Name : NxFFR_MediaTimeStamp()
 *
 * Return Value Description : NEX_FF_RETURN type
 *	
 * Comment : Media Type �� ���� ������ frame �� DTS �� CTS ���� ��ȯ.
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
 * Comment : AVI, ASF ������ ���� �Ѵ�.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_RASeekAll(IN pNxFFReader pRC, IN NXUINT32 UserTime, OUT NXUINT32 *pRetTime);

/******************************************************************************
 * Function Name : NxFF_RASeekTimeBase()
 * DNLA ���� RFC �� �̿��� Timebase Seek �Լ�.
 * Return Value Description : NEX_FF_RETURN
 *
 * Comment :	- MP4(Audio/Video), MP3, AAC(ADTS), MPEG-TS, ASF container ������ ����.
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
 * Comment :	Init �� Content �� Play �� �������� ���� �Ǵ�.
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
 * Comment : �������� Seek�� �������� Check�ϴ� �Լ�
 *			�� �Լ� ȣ�� �Ŀ��� ������ ��ġ�� 0���� �̵��� �� ����
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_SeekValidCheck(IN pNxFFReader pRC, OUT NXBOOL bRetCheck);

/******************************************************************************
 * Function Name : NxFFR_MakeSeekTable() 
 * 
 * Return Value Description
 *
 * Comment : Init �� Content �� Seek �� �� ���� �ϴٸ�, �����ϰ� Seek Table �� ����� �Լ�
 *			 nRetRate �� �󸶳� ���� Return �� ���ΰ�... (�� % �����Ŀ� Return �� ���ΰ�...)
 *			1 ~ 100 ���̿� ��
 *			nSeekDuration �� Seek ���� �ð� ����(ms)
 *			������ 0�̸� ���� ������ �ϰ�, 1 �̸� ������.
 *			-�� ����... ������ ���� play �� �����ϴ�.. .Seek �� �ȵɻ�...
 *			AVI�� FLV���� ��밡�� �ϴ�.
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
 * Comment : �Լ� ��ȯ ���� Bitrate ��. 
 *			��ȯ ���� 0 �̸� bitrate Ȯ���� �Ұ����� �����.
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
	NXUINT32	unDisplayImageWidth;	// Aspect ratio �� ���Ͽ� ���.(tkhd box)
	NXUINT32	unDisplayImageHeight;	// Aspect ratio �� ���Ͽ� ���.(tkhd box)
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
	// pOutput�� Data type�� NXINT64 �Ǵ� NXDOUBLE�̴�.
	// GET Info
	NXFF_EXTINFO_GET_VIDEO_FPS = 0x00000001,	// pOutput : Video FPS(NXDOUBLE)
	NXFF_EXTINFO_GET_VIDEO_FRAME_COUNT,			// pOutput : Frame Counts, pRestu : NULL, Video�� Frame ���� �����ö� ���
	NXFF_EXTINFO_GET_VIDEO_ORIENTATION,			// pOutput : Video�� Orientation ����. representation of an angle. (0 deg., 90 deg., 180 deg., 270 deg.)
	NXFF_EXTINFO_GET_SSTREAMING_TFRFINFO,		// pOutput : SmoothStreaming���� TFRF������ ���� �÷� �� ��
	NXFF_EXTINFO_GET_REST_DATA_SIZE,			// pOutput : Remain Size, SHOUTcast���� �������� ������ Frame���� �������� �ʾ��� ��� ������ ������ ũ�� ��ȯ
	NXFF_EXTONFO_GET_EXTRA_HEADER,				// pResult : Extra Header data, NxFFR_GetEXTRAHeaders �Լ� ��� ���, pNxFFAPEEXTRADATAHEADER Ȥ�� pNxFFOFFEXTRADATAHEADER ����
	NXFF_EXTINFO_GET_IFRAME_INDEX,				// iFrame�� Size�� CTS�� ��� �´�. �̶� nOption�� ���� TimeStamp�� �־��ָ� �� ���� 20���� IFrame�� ��� �´�.

	NXFF_EXTINFO_GET_META_ID3,					// Content���� ID3�� ��� �ö� ����Ѵ�. nOption���� SLFlag�� ����, pOutput���� ����� Return �ϰ� (ReadOneFrame�� ���� ���), pResult���� NxFFReaderWBuffer *�� �־��־�� �Ѵ�.
	NXFF_EXTINFO_GET_ASF_HeaderDuration,		// pOutput : ASF Total duration, ASF ���������� Header �� �ִ� �������� ������ Total duration �� ���������� ��.
	NXFF_EXTINFO_GET_HLSINIT_INFO,				// TS���� Init ������ ���´�. NXFF_EXTINFO_SET_HLSINIT_INFO ���� ��� �ȴ�. pOutput���� buffer size, pResult�� buffer�� ���´�
	NXFF_EXTINFO_GET_VIDEO_LANGUAGE_CODE,		// pOutput : Language code, MP4 �����̳ʿ��� MDHD box �� �ִ� Language ������ �������� �Լ�. ���� MP4, MKV �� ����.
	NXFF_EXTINFO_GET_AUDIO_LANGUAGE_CODE,		// pOutput : Language code, MP4 �����̳ʿ��� MDHD box �� �ִ� Language ������ �������� �Լ�. ���� MP4, MKV �� ����.// 
	NXFF_EXTINFO_GET_TEXT_LANGUAGE_CODE,		// pOutput : Language code, MP4 �����̳ʿ��� MDHD box �� �ִ� Language ������ �������� �Լ�. ���� MP4, MKV �� ����.
	NXFF_EXTINFO_GET_MPEG_DASH_VIDEO_TIMESCALE,	// pOutput : Time scale, DASH ���� Media type �� ���� Time scale ���� ��� �´�.
	NXFF_EXTINFO_GET_MPEG_DASH_AUDIO_TIMESCALE,	// pOutput : Time scale, DASH ���� Media type �� ���� Time scale ���� ��� �´�.
	NXFF_EXTINFO_GET_MPEG_DASH_TEXT_TIMESCALE,	// pOutput : Time scale, DASH ���� Media type �� ���� Time scale ���� ��� �´�.
	NXFF_EXTINFO_GET_TS_OFFSET,					// pOutput : Offset, pResult : NULL, TS ���������� Time Stamp �� ���� Offset ���� ��� �´�.
	NXFF_EXTINFO_GET_INDEX_TABLE_INFO,			// Container �� Index table �� ���� ���� Ȯ�� flag. (����� AVI �� ����)
	NxFF_EXTINFO_GET_TS_OFFSET_COMPARE,			// protocol �� �Ѱ��� Ư�� Offset �� NXFFReader ������ �ִ� Offset �� �񱳸� �Ͽ�, P_offset > N_offset �̸� TRUE, P_offset <= N_offset �� FALSE.
	NxFF_EXTINFO_GET_MPEG_TS_FIRST_TIMESTAMP_VIDEO_DTS,	// pOutput : First packet DTS, MPEG-TS Init. �� Set Video/Audio �� ù ��° Packet �� Time Stamp ���� �����ϰ� �ش� ���� ��������.
	NxFF_EXTINFO_GET_MPEG_TS_FIRST_TIMESTAMP_VIDEO_PTS,	// pOutput : First packet PTS, MPEG-TS Init. �� Set Video/Audio �� ù ��° Packet �� Time Stamp ���� �����ϰ� �ش� ���� ��������.
	NxFF_EXTINFO_GET_MPEG_TS_FIRST_TIMESTAMP_AUDIO_DTS,	// pOutput : First packet DTS, MPEG-TS Init. �� Set Video/Audio �� ù ��° Packet �� Time Stamp ���� �����ϰ� �ش� ���� ��������.
	NxFF_EXTINFO_GET_MPEG_TS_FIRST_TIMESTAMP_AUDIO_PTS,	// pOutput : First packet PTS, MPEG-TS Init. �� Set Video/Audio �� ù ��° Packet �� Time Stamp ���� �����ϰ� �ش� ���� ��������.
	NxFF_EXTINFO_GET_MP4_VIDEO_HANDLER_NAME,	// pOutput : name length, pResult : name data, nOption : track number, MP4 container �ȿ� �ִ� hdlr box �� name �� �������� �Լ�
	NxFF_EXTINFO_GET_MP4_AUDIO_HANDLER_NAME,	// pOutput : name length, pResult : name data, nOption : track number
	NxFF_EXTINFO_GET_MP4_SUBT_HANDLER_NAME,		// pOutput : name length, pResult : name data, nOption : track number, CFF spec. handler tyep "_SUBT_"
	NXFF_EXTINFO_GET_AUDIO_BASE_CTS,			// pOutput : Audio Base CTS, �ܺο��� ������ Audio Base CTS���� ������ �� ����Ѵ�.
	NXFF_EXTINFO_GET_VIDEO_SYNC_SAMPLE_COUNT,	// pOutput : Sync sample (I-frame) count,   (MP4 �� ����)
	NXFF_EXTINFO_GET_READ_OFFSET,				// pOutput : Frame Offset, MP4 �� ����.
	NXFF_EXTINFO_GET_MEDIACRYPTO_PSSH,			// ������ MEDIACRYPTO_SYSTMEID�� �ش��ϴ� PSSH ����
												// pOutput : size  pResult psshData NxFFreader���ο� �ִ� ������ �ܺο��� ���� �����Ͽ� ���

	NXFF_EXTINFO_GET_MEDIACRYPTO_TENC,			//Dash�� ��� init segment���� ������ 
												// Track Encryption Box ,Data pout Size(24byte)  point binary Data NxFFreader���ο� �ִ� ������
												// pOutput : size  pResult psshData NxFFreader���ο� �ִ� ������ �ܺο��� ���� �����Ͽ� ���

	
	NXFF_EXTINFO_GET_360_METADATA_DATA,			//pOutput : Data Size(NXINT64), pResult : Data(NXCHAR), nOption : Track Number 360 METADATA DATA�� �����´�.

	NXFF_EXTINFO_GET_MPEG_DASH_VIDEO_DEFAULT_SAMPLE_DURATION,	// pOutput : Default sample duration, DASH ���� Media type �� ���� _DEFAULT_SAMPLE_DURATION ���� ��� �´�.
	NXFF_EXTINFO_GET_MPEG_DASH_AUDIO_DEFAULT_SAMPLE_DURATION,
	NXFF_EXTINFO_GET_MPEG_DASH_TEXT_DEFAULT_SAMPLE_DURATION,
	
	NXFF_EXTINFO_GET_SIDX_COUNT,				//pOutput : SIDX Conts.	
	NXFF_EXTINFO_GET_SIDX,						//pOutput : SIDX referenced counts, pResult : Ref. SIDX

	NXFF_EXTINFO_GET_EDTSBOX,					// Only use MP4 container.
	NXFF_EXTINFO_GET_CHECK_EXIST_PTS,			// PTS of content exist check. - MP4 �� ����.
	NXFF_EXTINFO_GET_DISPLAYIMAGESIZE,			// nOption : Video�� Ʈ�� ��ȣ, pResult : pNxFFRDisplayImageSize struct 
	NXFF_EXTINFO_GET_AUDIO_BIT_RATE_MODE,		// nOption : NULL, pOutput : ��� üũ(0 : CBR mode, 1 : VBR mode) // ���� MP3(audio only) �������� ����.



	//NEX_FF_RETURN NxFFR_SetExtInfo(IN pNxFFReader pRC, IN NxFFReaderExtInfoType tpInfoType, IN NXINT64 nInput, INOUT NXVOID* pParam)	
	// SET Info
	NXFF_EXTINFO_SET_AUDIO_BASE_CTS = 0x40000001,	// pParam : NULL, nInput : CTS(NXUINT32)
	NXFF_EXTINFO_SET_ASF_EXTINDEX,					// nInput : File Path Len, pParam : File Path	ASF���� index�� �ܺο��� file�� �� �� ���, �ش� ���� ���̻� ������� ���� ��� ����.
	NXFF_EXTINFO_SET_SHOUTCAST_OGG_FILE,			// pParam : File Path
	NXFF_EXTINFO_SET_ASF_FORCEPERCENTSEEK,		// ASF����������Percent Seek���ε����ϵ��ϼ��� - 20110829 lovehis
	NXFF_EXTINFO_SET_MP4_FORCEPDMODE,			// pParam : pPDParam, MP4 ���� Init�� PD���·� Play �ϰ� ���� �ϵ��� ��. �̶� NxFFR_SetExtInfo ���� nInput�� NXFF_READER_FLAG_PD_GENERAL �� �־�� �ϰ�,
												// pParam�� pNxFFReaderForcePDModeParam�� �־���
	//NXFF_EXTINFO_SET_MP4FRAGMENT_TIMESCALE,	// MP4 Fragment���� TimeScale���� ����.
	NXFF_EXTINFO_SET_MP4FRAGMENT_INFO,			// nInput : NXFF_MEDIA_TYPE, pParam : pNxFFRMP4MFInfo,												
												// pParam�� pNxFFRMP4MFInfo�� �־� �ش�. Init�Ŀ� �����ؾ� �Ѵ�.
	NXFF_EXTINFO_SET_HLSINIT_INFO,				// nInput HLS Init data size(unsigned int), pParam : HLS Init Data(NXUINT8*) HLS���� i-Frame Only track�϶� Init ������ �־� �ش�. Reader Init ���� ȣ�� �ؾ� �ϸ�, Close �Ǹ� ��������.
	NXFF_EXTINFO_SET_TS_OFFSET,					// nInput : TS Offset(NXINT64), TS ���������� Time Stamp �� ���� Offset ���� �ܺο��� set ���ش�.

	NXFF_EXTINFO_SET_SUBTITLE_FILE_NAME,		// pParam : Subtitle file path      �ܺ� �ڸ� ���� ����
	NXFF_EXTINFO_SET_SUBTITLE_MEMORY,			// pParam : Subtitle Memory address �ܺ� �ڸ� �޸� ����
	NXFF_EXTINFO_SET_HLS_FRAMEDECRYPTOR,		// pParam : MEDIACRYPTO ����, HLS���� FRAME�� Decrypting�� ���� �� ��. init ���� �ؾ� �Ѵ�.

	NXFF_EXTINFO_SET_USE_MEDIACRYPTO,			// nInput : 0 or 1, MEDIACRYPTO���� FRAME������ �� Decrypting�� ���� �� ��. init ���� �ؾ� �Ѵ�. �߰������� systemID�� ����
	
	NXFF_EXTINFO_SET_MEDIACRYPTO_PSSH,			//�ܺο��� PSSH����  
												//nInput : size  pParam: psshdata point

	NXFF_EXTINFO_SET_MEDIACRYPTO_SYSTMEID,		// UUID�� �켱 ���� DRM SYSTMEID ���� 
												//nInput : size  pParam: psshdata point

	NXFF_EXTINFO_SET_MEDIACRYPTO_TENC,			// Dash�� ��� init segment���� ������ ����Ÿ�� framesegment�� ����
												// NXFF_EXTINFO_GET_MEDIACRYPTO_TENC �� ������ ����
												// nInput : size(24byte)  pParam: Tenc data point
	
	NXFF_EXTINFO_SET_MEDIACRYPTO_KID,			// 16byte �ܺο��� default_keyID ���� 
												//nInput : size(16byte)  pParam: KIDdata point
	
	NXFF_EXTINFO_SET_SIDX,						//pParam : SIDX data File Path
	NXFF_EXTINFO_SET_SIDX_OFFSET,				//nInput : SIDX Offset(dafault : 0), pParam	: NULL
	NXFF_EXTINFO_SET_NEEDSIDXBOX,				//pParam : NULL, nInput : 1 or 0, SIDX box ��� ���� ����				
	
	NXFF_EXTINFO_SET_AUDIODURATIONCHECK,		//pParam : NULL, nInput : 1 or 0, Audio VBR �������� ��� Frame�� duration�� ����ϵ��� ��������. (���� MP3 �� ����.), NxFFR_Init�� ȣ��

	NXFF_EXTINFO_SET_TIMESTAMP_CALC,			// Only use MP4 container. 

	NXFF_EXTINFO_SET_MEDIAFILETYPE,				// MediaFileFormatType ����. (nInput : NEX_FILEFORMAT_TYPE �Է�.)
	NXFF_EXTINFO_SET_MEDIAFILESUBTYPE,			// MediaFileFormatSubType ����. (nInput : NEX_FILEFORMAT_TYPE �Է�.)
	
	NXFF_EXTINFO_PADDING = 0x7FFFFFFF
} NxFFReaderExtInfoType;


/******************************************************************************
 * Function Name : NxFFR_GetExtInfo() 
 * 
 * Return Value Description : NEX_FF_RETURN
 *
 * Comment :	NxFFReaderExtInfoType�� ���� �� �� Ư���� ��ð� ������ pOutput�� Data type�� signed 64bit �̴�.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_GetExtInfo(IN pNxFFReader pRC, IN NxFFReaderExtInfoType tpInfoType, IN NXINT64 nOption, OUT NXVOID *pOutput, OUT NXVOID** pResult);

/******************************************************************************
 * Function Name : NxFFR_SetExtInfo() 
 * 
 * Return Value Description : NEX_FF_RETURN
 *
 * Comment :	NxFFR_SetExtInfo �Լ��� ReaderInit���� �ҷ��� ������ �θ� �Ŀ��� �� ReaderClose�� ���־�� �Ѵ�.
******************************************************************************/
_NXFFREADERDLL_EXPORTS NEX_FF_RETURN NxFFR_SetExtInfo(IN pNxFFReader pRC, IN NxFFReaderExtInfoType tpInfoType, IN NXINT64 nInput, INOUT NXVOID* pParam);

/******************************************************************************
 * Function Name : NxFFR_GetWAVEFormatEX() 
 * 
 * Return Value Description
 *
 * Comment :Audio�� Wave format�� �̱����� �Լ�, ���Ŀ� MULTI track�� �����ؾ� �Ѵ�.
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


//APE ExtraData������ ���� ��� 
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

//OGG ExtraData������ ���� ��� 
typedef struct
{
	NXUINT32	HeaderSize;
	NXUINT8		*HeaderData;
}NxFFOGGEXTRADATAHEADER, *pNxFFOGGEXTRADATAHEADER;

// Media�� Track�� �����ϴ� API
// �����Ŀ��� Sync�� ���߱� ���� �� Media�� ���� Seek�� �ؾ��Ѵ�.
/******************************************************************************
 * Function Name : NxFFR_ChangeTrack()
 * Multi-Track contents �� ���� Track change
 * Return Value Description
 *   Success (���� Ʈ���� ���ؼ�)			: eNEX_FF_RET_SUCCESS
 *	 Success (���� �ٸ� Ʈ���� ���ؼ�)		: eNEX_FF_RET_HAS_NO_EFFECT
 *	 Error									: �̿� error value
 * 
 * Comment :	- Track change �� success �̸�, RASeek �� �����ؾ���. ��, Success 1 �� ��츸 RASeek ����.
				- success return value �� ������ ������ track change �� ���� �ó������� ������ �ʿ��ϱ� ����.
				- ���� Ʈ������ Track change �õ��� success ����. (�ܺο����� ����Ʈ������ �ٸ� Ʈ������ ������ �� ����.)
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
 * Comment : �ܺο��� 4CC �� �� Compression ID�� NxFFReader�� ���ǵ� Codec ID�� ���� �� ����ϴ� �Լ�
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
