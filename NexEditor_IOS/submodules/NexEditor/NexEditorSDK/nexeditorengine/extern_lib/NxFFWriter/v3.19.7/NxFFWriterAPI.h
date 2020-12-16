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

File		:	NxFFWriterAPI.h

Description	:	This Software Module was developed by Nextreaming
Authors		: 	Andrew Kang		(lovehis@nextreaming.com)
				Jeong Heon KIM	(junghuni@nextreaming.com)
History		:
Version		:	3.19.6.OFFICIAL (2017.08.03)

******************************************************************************/
#ifndef __NxFFWriterAPI_H__ 
#define __NxFFWriterAPI_H__    


#include "NexMediaDef.h"
#include "NexTypeDef.h"
#include "NexSAL_Internal.h"
#include "NexSAL_API.h"
#include "NxFFDRM.h"

#define NXFFWRITER_VERSION_MAJOR			3
#define NXFFWRITER_VERSION_MINOR			19
#define NXFFWRITER_VERSION_PATCH			7
#define NXFFWRITER_VERSION_BRANCH			"OFFICIAL"
#define NXFFWRITER_COMPATIBILITY_NUM		8

typedef enum
{
	// GET Info
	NXFFW_EXTINFO_GET_TS_LAST_SEGMENT_INFO = 0x0000001,

	// SET Info
	NXFFW_EXTINFO_SET_TS_SEGMENT_INTERVAL = 0x40000001,
	NXFFW_EXTINFO_SET_TS_SEGMENT_START_INDEX,			// Segment 파일 시작 파일명 설정하는데 default 값은 0 부터 시작하나 변경 가능함.
	NXFFW_EXTINFO_SET_TS_SEGMENT_IFRAME_ALLIGN,			// Segment 시작이 I-Frame 인 경우 설정
	NXFFW_EXTINFO_SET_TS_SEGMENT_SETLASTSEGMENT,		// 더 이상 Segment 를 하지 않는 경우 설정
	NXFFW_EXTINFO_SET_360_METADATA_DATA,				// MP4 360 VR Metadata 추가(video track)
	
	NXFFW_EXTINFO_SET_LAST_FRAME_DURATION,				// 마지막 Frame의 duration 을 넣을 수 있는 API

	NXFFW_EXTINFO_SET_ID3_DATA,

	NXFFW_EXTINFO_PADDING = MAX_SIGNED32BIT
} NxFFWriterExtInfoType;

// KDDI Extension - Digital Rights Management
#define	NXMP4FWRITER_KDDIEXT_DRM_NO_PLAYBACK_RESTRICTION	0x00	/*!< No playback restriction */
#define	NXMP4FWRITER_KDDIEXT_DRM_EFFECTIVE_DATE_ENABLED		0x01	/*!< Playback restriction by effective date is enabled */
#define	NXMP4FWRITER_KDDIEXT_DRM_EXPIRATION_DATE_ENABLED	0x02	/*!< Playback restriction by expiration date is enabled */
#define	NXMP4FWRITER_KDDIEXT_DRM_PLAYBACK_COUNT_ENABLED		0x04	/*!< Playback restriction by playback count is enabled */

typedef enum {
	NXMP4FWRITER_KDDIEXT_CPGD_OFF		= 0,	/*!< Serial transfer, mail attachment allowed */
	NXMP4FWRITER_KDDIEXT_CPGD_ON		= 1	/*!< Serial transfer, mail attachment NOT allowed */
} E_NXMP4FWRITER_KDDIEXT_CPGD_ATTRIBUTE;

typedef struct {
	NXUINT8							flags;			/*!< Playback restriction flags : bitwise combination of NXMP4FWRITER_KDDIEXT_DRM_~ values */
	E_NXMP4FWRITER_KDDIEXT_CPGD_ATTRIBUTE	attributes;		/*!< Copy guard attribute */
	NXUINT32							limit_date;		/*!< Set expiration date and time (1904/1/1 0:00 in seconds by GMT). Playback is not allowed after this time */
	NXUINT32							limit_period;	/*!< Set effective period (number of days) after downloading */
	NXUINT32							limit_count;	/*!< Number of times replayable. '1' means reaplayable only once */
} NxMP4FWriterKDDIExtensionDigitalRightsManagementStruct;

// KDDI Extension - Content Property Information
typedef struct {
	NXUINT8		*title;		/*!< this field shall be initialized with NULL.  Non-zero pointer value means this field shall be written */
	NXUINT32	title_len;	/*!< byte length of 'title' field : Up to 200 bytes */

	NXUINT8		*right;		/*!< this field shall be initialized with NULL.  Non-zero pointer value means this field shall be written */
	NXUINT32	right_len;	/*!< byte length of 'right' field : Up to 80 bytes */
	
	NXUINT8		*author;	/*!< this field shall be initialized with NULL.  Non-zero pointer value means this field shall be written */
	NXUINT32	author_len;	/*!< byte length of 'author' field : Up to 80 bytes */
	
	NXUINT8		*memo;		/*!< this field shall be initialized with NULL.  Non-zero pointer value means this field shall be written */
	NXUINT32	memo_len;	/*!< byte length of 'memo' field : Up to 4000 bytes */
	
	NXUINT32	*version;	/*!< this field shall be initialized with NULL.  Non-zero pointer value means this field shall be written */
} NxMP4FWriterKDDIExtensionContentPropertyInformationStruct;

// KDDI Extension - Movie Mail Information
#define	NXMP4FWRITER_KDDIEXT_MVML_TELOP_EDIT_INHIBITED					0x01	/*!< Telop editing inhibited */
#define	NXMP4FWRITER_KDDIEXT_MVML_VIDEO_ES_EDIT_INHIBITED				0x02	/*!< Video ES editing inhibited */
#define	NXMP4FWRITER_KDDIEXT_MVML_AUDIO_ES_EDIT_INHIBITED				0x04	/*!< Audio ES editing inhibited */
#define	NXMP4FWRITER_KDDIEXT_MVML_PROPERTY_INFORMATION_EDIT_INHIBITED	0x08	/*!< Property information editing inhibited */

typedef enum {
	NXMP4FWRITER_KDDIEXT_MVML_REC_MODE_MEVDO		= 0x04,		/*!< Video : 128x96 64000bps, Audio : AMR 12200bps */
	NXMP4FWRITER_KDDIEXT_MVML_REC_MODE_LEVDO		= 0x05		/*!< Video : 176x144 64000bps, Audio : AMR 12200bps */
} E_NXMP4FWRITER_KDDIEXT_MVML_REC_MODE;

/*! parameter struct for NxMP4FWriterAddKDDIExtensionMovieMailInformation() */
typedef struct {
	NXUINT8									permission;	/*!< Edit inhibit flag : bitwise combination of NXMP4FWRITER_KDDIEXT_MVML_~ values */
	E_NXMP4FWRITER_KDDIEXT_MVML_REC_MODE	rec_mode;	/*!< recording mode */
	NXUINT32								rec_date;	/*!< Set the local time of terminal in seconds from 1970/1/1 0:00 UTC. */
} NxMP4FWriterKDDIExtensionMovieMailInformationStruct;

// KDDI Extension - Encoder Information
typedef struct {
	NXUINT8		device[8];		/*!< Set 'KDDI-' + first NXDOUBLE-byte of device type + '0x00' as single-byte. This original setting will be inherited even after editing or sound-editing.  Device type is to be available in February 2005 */
	NXUINT8		model[8];		/*!< Define a model name in a character string. '0x00' is set for unused byte．This original setting will be inherited even after editing or sound-editing.  Example: 'C5001T' + '0x00' as NXDOUBLE-byte.  Model name is to be available in February 2005 */
	NXUINT8		encoder[8];		/*!< Define encoder information in a character string. Set first NXDOUBLE-byte of device type + mobile-dependent character information. '0x00' is set for unused byte．This original setting will be inherited even after editing or sound-editing */
	NXUINT8		multiplexer[8];	/*!< Define multiplexer information in a character string. Set first NXDOUBLE-byte of device type + mobile-dependent character information. '0x00' is set for unused byte. For editing or sound-editing, a mobile that edited will set its value. */
} NxMP4FWriterKDDIExtensionEncoderInformationStruct;

// KDDI Extension - Multimedia Ringer Information
typedef struct {
	NXUINT8		data[128];		/*!< Secret key to verify this file is available for multimedia ringer */
} NxMP4FWriterKDDIExtensionMultimediaRingerInformationStruct;


typedef struct
{
	NxMP4FWriterKDDIExtensionDigitalRightsManagementStruct		*cpgd;
	NxMP4FWriterKDDIExtensionContentPropertyInformationStruct	*prop;
	NxMP4FWriterKDDIExtensionMovieMailInformationStruct			*mvml;
	NxMP4FWriterKDDIExtensionEncoderInformationStruct			*enci;
	NxMP4FWriterKDDIExtensionMultimediaRingerInformationStruct	*chku;
} NxMP4FWriterKDDIExtensionStruct;


typedef struct {
	NXUINT32 nStartTS;
	NXUINT32 nDuration;
	NXUINT8 *pCurrentFileName;
	NXUINT8 *pPrevFileName;
}nxFFWriter_TS_Info, *pNxFFWriter_TS_Info;

typedef struct {

	NXINT8	strTitle[30];
	NXINT8	strArtist[30];
	NXINT8	strAlbum[30];
	NXINT8	strYear[4];
	NXINT8	strComment[28];
	NXINT32		nTrack;		// Album TRack
	NXINT32		nGenre;		// Genre

} NxFFWriterID3Tag, *NxFFWriterID3TagPtr;

typedef struct
{
	NXUINT32	uCurrentSize;	// 내부 data 에 저장된 크기
	NXUINT32	uMaxSize;		// data 의 최대 크기
	NXUINT8		*pData;			// Create 시에는 내부에서 할당, 만약 uMaxSize 보다 큰 경우에 재할당.
} _NxFFWriterUdtaBox, NxFFWriterUdtaBox;

typedef struct
{
	NXUINT32	uSize;			// box size ( size for data + 4 + 4 + 4 )
	NXUINT32	uType;			// auth, perf, gnre etc.
	NXUINT32	uVersionFlag;	// ( 8 + 24 ) bits
	NXUINT8		*pData;			// Data of box
	NXUINT32	uDataSize;		// data size
} _UdtaBox, UdtaBox;

typedef struct 
{
	NXINT64		nAudioDuration;
	NXINT64		nVideoDuration;
	NXINT64		nTextDuration;
} NxFFWriterLastDuration, *pLastDuration;

// File API;
typedef NXVOID *(* __NxFFWriterFAOpen)(NXCHAR* pFilename, NEXSALFileMode eMode);
typedef NXINT32	(* __NxFFWriterFAClose)(NXVOID* hFile);
typedef NXINT32	(* __NxFFWriterFASeek)(NXVOID* hFile, NXINT32 iOffSet, NXUINT32 iFromWhere);
typedef NXINT64 (* __NxFFWriterFASeek64)(NXVOID* hFile, NXINT64 iOffSet, NXUINT32 iFromWhere);
typedef NXSSIZE	(* __NxFFWriterFARead)(NXVOID* hFile, NXINT8* pBuf, NXUSIZE dwSize);
typedef NXSSIZE	(* __NxFFWriterFAWrite)(NXVOID* hFile, NXINT8* pBuf, NXUSIZE dwSize);
typedef NXINT32 (* __NxFFWriterFARemove) (NXCHAR *a_strFilePath);
typedef NXINT32 (* __NxFFWriterFARename) (NXCHAR *a_strOldName, NXCHAR *a_strNewName);
	
typedef struct
{
	__NxFFWriterFAOpen					m_Open;
	__NxFFWriterFAClose					m_Close;
	__NxFFWriterFASeek					m_Seek;
	__NxFFWriterFASeek64				m_Seek64;
	__NxFFWriterFARead					m_Read;
	__NxFFWriterFAWrite					m_Write;	
	__NxFFWriterFARemove				m_Remove;
	__NxFFWriterFARename				m_Rename;
} __NxFFWriterFileAPI, *__pNxFFWriterFileAPI;

typedef struct _NxFFWriter
{
	__NxFFWriterFileAPI	*pFileAPI;					// File API

	NXUINT32	AvailableRAMMemory;					// 0: temp_file 이용, non-zero: 가용 메모리 사이즈
	NXUINT32	*nCurrentMemory;					// 현재 메모리 사용량
	NXUINT64	MaxFileSize;						// k3g/3gpp/3g2 파일 생성 가용 메모리 사이즈, AvailableRAMMemory가 양수일 경우에만 사용됨.
	//NXUINT32 MaxFileSize;							// k3g/3gpp/3g2 파일 생성 가용 메모리 사이즈, AvailableRAMMemory가 양수일 경우에만 사용됨.
	NXUINT32	MaxRecordingTime;					// millisecond 단위, streaming Recording Mode에서만 지원함. 0일 경우 MaxFileSize와 AvailableMemory에 따라 저장됨.

	NEX_FILEFORMAT_TYPE	MediaFileFormatType;			// File Container 이름		
	NEX_FILEFORMAT_TYPE MediaFileFormatSubType;			// Container 중 Sub Spec EX) 3GP 의 경우 MediaFileFormatType은 eNEX_FF_MP4 이고
														// MediaFileFormatSubType은 NXFFW_STANDARD_3GP로 설정 해야 한다.

	NXUINT32	StreamingRecord;						// 0: local recording, not zero: streaming recording
	NXUINT8		bStreamingLargeFrame;					// make large frame length file
	NXUINT32	MPEG4system;							// 0: not support BIFS, not zero: support BIFS
	NXUINT32	TSflag;									// 0: baselayer recording, not zero: enhancement layer recording 
	NXUINT32	CreationTime;							// mvhd, mdhd (creation time)
	NXUINT8		bLargeFileSize;							// TRUE : file size large than 4GByte. FALSE : file size less than 4GByte.
														// 우선 MP4 컨테이너만 지원
	NXUINT8		bVideoExist;							// TRUE : Video exist, FALSE : Video not exist
	NXUINT8		bAudioExist;
	NXUINT8		bTextExist;

	NXUINT8		bTWOPASS;
	NXUINT8		bFreeBox;
	NXUINT32	uFreeBoxMaxSize;
	NXUINT8		uFreeOffset;

	NXUINT8		bVRVideoMetaData;					//SetExtInfo로 외부에서 해당 Data를 셋팅한다.
	NXUINT32	uVRVideoMetaDataSize;
	NXINT8		*pVRVideoMetaData;

	NXUINT8					bLastDuration;						// SetExtInfo 로 외부에서 설정.
	NxFFWriterLastDuration	*pLastDuration;
	
    NXUINT32 VOTI;									// VideoCodingType : 0 (None), MPEG-4, H.263, H.264, JPEG... (NxFFReader에 Codec Type과 일치)
	NXUINT32 H263FrameRate;							// 0 (variable frame rate), 1 (29.97Hz), 2 (14.485Hz), 3 (7.243Hz), ....
    NXUINT32 VideoBufferSizeDB[2];
    NXUINT32 VideoWidth;
    NXUINT32 VideoHeight;
	NXUINT32 DisplayVideoWidth;						// Aspect Ratio를 위해 값을 설정 안하면 VideoWidth와 같은 값이 된다.
	NXUINT32 DisplayVideoHeight;					// Aspect Ratio를 위해 값을 설정 안하면 VideoHeight와 같은 값이 된다.
	NXUINT32 VideoDecoderSpecificInfoSize[2];		// For Streaming Recording
	NXUINT8 *pVideoDecoderSpecificInfo[2];			// For Streaming Recording
	NXUINT32 profile_level_id;						// Only for H.264 (Profile, Compatibility, Level, lengthSizeMinusOne)
	NXUINT32 parameter_sets_bytecnt;				// Only for H.264
	NXUINT8 *parameter_sets;						// Only for H.264
	NXUINT32 RotationValue;							// 0, 90, 180, 270 도 넣어주어야 함. default value : 0
	
    NXUINT32 AOTI;									// AudioCodingType : None(0x00), AAC, MP3, AMR, EVRC, QCELP, SMV, AMRWB, G711, G723, QCELP_ALT, BSAC.... (NxFFReader에 Codec Type과 일치)
	NXUINT32 AudioBufferSizeDB;
	NXUINT32 AudioDecoderSpecificInfoSize;			// For Streaming Recording
	NXUINT8 *pAudioDecoderSpecificInfo;				// For Streaming Recording
	NXUINT32 G711ChannelNum;						// For G711 support

    NXUINT32 TextCodingType;						// TextCodingType : 0 (None), 1 (T.140) 
	NXUINT32 TextBufferSizeDB;
	NXUINT32 TextDecoderSpecificInfoSize;			// For Streaming Recording
	NXUINT8	 *pTextDecoderSpecificInfo;			// For Streaming Recording

	NXUINT32 BIFSBufferSizeDB;
	NXUINT32 ODBufferSizeDB;


	NXUINT32 MidiFileFlag;							// For Streaming Recording
	NXUINT32 LinkDataFlag;							// For Streaming Recording
	NXUINT32 LinkDataSize;							// For Streaming Recording
	NXUINT8 *LinkData;							// For Streaming Recording	

	NxFFWriterUdtaBox	UdtaBox;					// 	For Streaming Recording	 (3gpp udta box)
	NXUINT32 MovieDuration;							// Duration of Current Movie
	NXINT64		MovieSize;								// Size of Current Movie

	NXUINT32 frames_per_sample;						// AMR recording시 한 sample에 몇 frame을 writing할지를
													// 외부에서 setting할 수 있도록 함. 범위 : 1~16

	//For MP4
	NXINT8		bUseTimeScale;						
	NXUINT32	uVTimeScale;						//mp4 video track timescale
	NXUINT32	uATimeScale;						//mp4 audio track timescale
	NXUINT32	uTTimeScale;						//mp4 text  track timescale

	NxMP4FWriterKDDIExtensionStruct *kddiext;
	NxFFWriterID3TagPtr	pID3Tag;
	
	// for DRM 
	NXFF_DRM_TYPE	DRMType;					// DRM Type
	NXVOID			*pDRMInfo;					// DRM Info. Refer NxMP4FFDRM.h

	// Internal Use Only
	NXVOID			*pModule;					// Internal Use Only
	NXVOID			*pRoot;						// Internal Use Only
	NXVOID			*pSetValue;					// Internal Use Only
	NXVOID			*bInternal;					// Internal Use Only

	NXUINT32		WriteBufferSize;

	NXVOID			*pWriteBuffer;
} NxFFWriter, *pNxFFWriter;

typedef enum {
	NXFFW_MEDIA_TYPE_AUDIO					=	0,
	NXFFW_MEDIA_TYPE_BASE_LAYER_VIDEO		=	1,
	NXFFW_MEDIA_TYPE_ENHANCED_LAYER_VIDEO	=	2,
	NXFFW_MEDIA_TYPE_SKTTT					=	3,
	NXFFW_MEDIA_TYPE_BIFS					=	4,	
	NXFFW_MEDIA_TYPE_3GPTT					=	5,
	NXFFW_MEDIA_TYPE_DUMMY					= MAX_SIGNED32BIT
}NXFFW_MEDIA_TYPE;


typedef enum{
	NXFF_FRAME_TYPE_UNINDEXED				= 0,				// AUDIO와 p-Frame처럼 인덱스 처리가 필요하지 않은 Frame
	NXFF_FRAME_TYPE_INDEXED					= 1,				// VIDEO의 i-Frame등의 인덱스 처리가 필요한 Frame
	NXFF_FRAME_TYPE_UNKNOWN					= 2,				// 내부에서 FRAME TYPE을 설정
	NXFF_FRAME_TYPE_DUMMY					= MAX_SIGNED32BIT
}NXFF_FRAME_TYPE;

#ifdef __cplusplus
extern "C" {
#endif	//__cplusplus




pNxFFWriter NxFFWriterCreate();

NXVOID NxFFWriterDestroy(pNxFFWriter pWC);



// NxFFWriterInit
// return value
// minus values: FS error, 0: SUCCESS, plus values: Fail
// 1: mp4 file open error
// 2: midi file open error
// 3: TSflag 0일 경우 VideoCodingType이 MPEG-4도 H.263,H.264도 아닌 경우
// 4: moov->nVOTI, pWC->AudioCodingType, pWC->TextCodingType 이 모두 0인 경우
// 즉, moov->trak[i++].mdia.hdlr.HandlerType 이 VIDE,SOUN,SKTEXT 중 하나도 해당사항이 없는 경우
// 5: moov->MetaDataAnalysisBuffer allocation error
// 6: buf_meta_data_writing.bfrstart allocation error
// 7: AVI file writing 시 streamingrecord 값이 0이 아닌 경우
// 10: 0 < pWC->AvailableRAMMemory < 2*SDRAM_ANALYSIS_BUF_SIZE 일 경우
// 11: 정의되지 않은 standard 지정시
// 12: Failed to create temp file (NAND version)
// 13: Failed to allocate temporary file writing buffer (NAND Version)
// 14: streaming recording not supported
// 15: not supported video resolution
// 16: not supported video codec
// 17: not supported audio codec
// 20: avi file 저장시 첫 frame에 start code prefix(0x000001) 로 시작하지 않는 경우
// 21: avi file 저장시 첫 frame 입력시 DSI만 입력되고 첫 I-VOP 입력이 같이 되지 않은 경우
// 100: Undefined Error 
NXINT32 NxFFWriterInit( pNxFFWriter pWC, NXUINT8 *mp4filename, NXUINT8 *midifilename, NXUINT8 *nand_tempfilename);
////////////////////////////////////////////////////////////////////////////////////////////////////
// NxFFWriteFrame
// MediaType (0: Audio/Speech, 1: Video (BaseLayer), 2: Video (EnhLayer), 3: Text, 4: BIFS)
// return value
// minus values: FS error, 0: SUCCESS, plus values: Fail
// 1: MediaType Input Error (Streaming Mode에서 MediaType이 0,1,2,3 중 하나도 아닌 경우)
// 2: AudioCodingType Input Error (AAC,AMR,EVRC,QCELP,SMV,AMRWB,G711,QCELP_ALT,MP3 중 하나도 아닌경우)
// 3: MediaType Input Error (Camcording Mode에서 MediaType이 0,1 중 하나도 아닌 경우)
// 4: readbuf allocation error
// 5: even though AudioCodingType is G.711 (0xDF), G.711 channel number is neither 1 nor 2.
// 10: moov->AvailableSDRAMSize < ((moov->MetaDataAnalysisBufferWrittenBitsNum>>3) + 1000 일 경우
// 11: Estimated File Size is greater than moov->AvailableNANDSize
// 12: (KDDI) Estimated 'moov' box size is greater than maximum value defined.
// 13: 전체 저장시간이 MaxRecordingTime 보다 큰 경우
// 14: NxFFWriteFrame() 함수를 통하여 파일 Write 를 할 수 없는 경우. (첫 번째 프레임이 Sync Sample 이 아닌 경우.)
// 100: Undefined Error 
// 300:	MPEG TS에서 Segment 한개의 Segment가 Writing 된 경우
////////////////////////////////////////////////////////////////////////////////////////////////////
NXINT32 NxFFWriteFrame(pNxFFWriter pWC, NXFFW_MEDIA_TYPE MediaType, NXUINT32 samplesize, NXUINT8 *samplebuf, NXUINT32 CTS, NXUINT32 DTS, NXFF_FRAME_TYPE FrameType);
/******************************************************************************
* -- NxFFWriterClose 
  * return values
  * minus values: FS error, 0: SUCCESS, plus values: Fail
  * 1: VideoCodingType이 MPEG4V이면서 mpeg-4 baselayer sample count가 0인 경우
  * 2: VideoCodingType이 H263/H264이면서 h.263,h.264 sample count가 0인 경우
  * 3: AudioCodingType이 0이 아니면서 audio sample count가 0인 경우
  * 4: Memory내에서 Local Camcorder 모드에서 설정되지 않은 media type 2가 추출된 경우
  * 5: KDDI Extension : failed to write meta data : buffer overflow
  100: Undefined Error
  ******************************************************************************/
NXINT32 NxFFWriterClose(pNxFFWriter pWC);
NXINT32 NxFFWriterFlush(pNxFFWriter pWC);


NXINT32 NxFFWriterSetExtInfo(pNxFFWriter pWC, NxFFWriterExtInfoType tpInfoType, NXINT64 nInput, NXVOID* pParam);
NXINT32 NxFFWriterGetExtInfo(pNxFFWriter pWC, NxFFWriterExtInfoType tpInfoType, NXINT64 *pOutput, NXVOID** pResult);

NXINT32 NxFFWriterGetVersionNum(NXINT32 mode);
NXINT8 *NxFFWriterGetVersionInfo(NXINT32 mode);


// 0 OK
// 1 duaration over
// 2 size over
// 파일에 첫번째 I-Frame과 첫번째 Audio Frame은 쓴 후 호출 해야 한다
// 함수 호출 순서는
// 1. Writer init
// 2. 첫 I-Frame 첫Audio Frame Write
// 3. 다음 I-Frame 까지 Frame의 Size 계산, 그리고 그 i-Frame 이전 까지 Audio를 Frame size 계산
// 4. 함수 호출 후 return이 
//		0 이면 Audio Video Frame write 후 3번 부터 반복
//		아니면 Writer Close 후 1번 부터 반복
NXINT32 NxFFWriterCheckDurationAndSize(pNxFFWriter pWC, NXUINT32 nVSampleCnt, NXUINT32 nNextVCTS, NXUINT32 nVSize, NXUINT32 nASampleCnt, NXUINT32 nNextACTS, NXUINT32 nASize);


/******************************************************************************
 * Name : udta box write
 *	
 * return value
 *	0				: Success
 *	positive number : Error
******************************************************************************/
NXINT32	NxFFWriteUdta(IN pNxFFWriter pWC, IN UdtaBox *pUdtabox);

//Version
	///////////////////////////////////////////////////////////////////////////////
// NxFFWriter 관련
/******************************************************************************
 * Return False or True
 * 현재 NxFFWriter Library의 Version을 체크
 *
 * @return			True or False
******************************************************************************/
NXBOOL NxFFWriter_CheckSameVersion(NXINT32 a_nMajor, NXINT32 a_nMinor, NXINT32 a_nPatch, NXCHAR *a_strBranch);

///////////////////////////////////////////////////////////////////////////////
// NxFFWriter 관련
/******************************************************************************
 * Return False or True
 * 현재 NxFFWriter Library Compatibility Num을 체크
 *
 * @return			True or False
******************************************************************************/
NXBOOL NxFFWriter_CheckCompatibleVersion(NXINT32 a_nCompatibilityNum);

/******************************************************************************
 * Return Version
 * 현재 NxFFWriter Library의 버전을 반환
 *
 * @return			Version of Current NxFFWriter Library
******************************************************************************/							
const NXCHAR* NxFFWriter_GetVersionString();
const NXINT32 NxFFWriter_GetMajorVersion();
const NXINT32 NxFFWriter_GetMinorVersion();
const NXINT32 NxFFWriter_GetPatchVersion();
const NXCHAR* NxFFWriter_GetBranchVersion();
const NXCHAR* NxFFWriter_GetVersionInfo();

#ifdef __cplusplus
}
#endif	// __cplusplus


#endif /* __NxFFWriterAPI_H__ */
