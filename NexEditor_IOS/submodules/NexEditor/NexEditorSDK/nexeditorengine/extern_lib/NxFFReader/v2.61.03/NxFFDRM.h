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
*		File Name			: NxFFDRM.h
*		Author				: NexStreaming MiddleWare Team (mw@nexstreaming.com)
*		Description			: DRM Spec���� ������ �ִ� ���
*		Revision History	: 
*		Comment				: 
*
******************************************************************************/

#ifndef _NXFF_DRM_H_
#define _NXFF_DRM_H_

#include "NexMediaDef.h"
#include "NexTypeDef.h"

// for DRM
typedef enum {
	NXFF_NO_DRM						= 0x00000000,
	NXFF_WINDOWS_MEDIA_DRM			= 0x00000100,
	NXFF_MS_PLAYREADY_DRM			= 0x00000110,	// MS PlayReady DRM
	NXFF_SStreaming_PRDRM			= 0x00000120,	// Smooth Streaming DRM
	NXFF_PIFF_PRDRM					= 0x00000130,	// PIFF PlayReady DRM
	NXFF_OMA_PDCFV2_DRM				= 0x00000200,
	NXFF_OMA_BCAST_APDCF_DRM		= 0x00000210,
	NXFF_DIVX_DRM					= 0x00000300,
	NXFF_DIVX_PLUS_DRM				= 0x00000310,	// DivX Plus

	NXFF_DECE_UVDRM					= 0x00000400,
	NXFF_HLS_FRAME_DRM				= 0x00000800,	// HLS
	NXFF_ISOBMFF_DRM				= 0x00001000,	// ISO/IEC 23001-7
	NXFF_UNSUPPORT_DRM				= 0x7FFFFFFF
}NXFF_DRM_TYPE;

//for WIndows Media DRM (NXFF_WINDOWS_MEDIA_DRM)
typedef NXINT32(*pNxFFReader_WMDRM_Descramble) (NXUINT8* pInputBuffer,  NXUINT32 uiInputBufferSize, 
											NXUINT8* pOutputBuffer, NXUINT32* puiOutputBufferSize,
											NXUINT8* pIVBuffer, NXUINT32 uiIVBufferSize,
											NXVOID * pUserData);

typedef NXINT32(*pNxFFReader_PRDRM_Descramble) (NXUINT8* pInputBuffer,  NXUINT32 uiInputBufferSize, 
											NXUINT8* pOutputBuffer, NXUINT32* puiOutputBufferSize,
											NXUINT8* pIVBuffer, NXUINT32 uiIVBufferSize,
											NXVOID * pUserData);

typedef enum{
	NXFF_WMDRM_DEC_PAYLOAD	= 0x00000001,			// Payload�� Descrambling �Ѵ�.
	NXFF_WMDRM_DEC_PACKET	= 0x00000010,			// Packet�� Descrambling �Ѵ�.
	NXFF_WMDRM_DEC_FRAME	= 0x00000020,			// Frame�� Descrambling �Ѵ�.
	NXFF_WMDRM_DEC_PADDING	= 0x7FFFFFFF
}NXFF_WM_DRM_DEC_TYPE_T;

// DRM Type�� NXFF_WINDOWS_MEDIA_DRM�� ��� 
typedef struct _NxFF_WM_DRM_T {
	// pMidOffset�� nDividedPayloadCount ��ŭ ��� �ִ�.
	NXUINT32	pMidOffset[128];				// for divided ASF Media Object		// used only WMDRM Client Module
	NXUINT8		nDividedPayloadCount;		// for divided ASF Media Object		// used only WMDRM Client Module
	NXUINT8		*pDRMheader;				// for ASF DRM
	NXUINT32	nDRMheadersize;				// for ASF DRM
	NXUINT32	nRepDataLen;
	NXUINT8		*pRepData;
	// �ܺο��� ��� �����ִ�Descrambler ���� api �� �ڷ���
	// payload�� scramble �Ǿ� ���� ��� ��� �ȴ�.
	NXFF_WM_DRM_DEC_TYPE_T	nDecType;			// Descrambler�� Type �ܺο��� Seting ���־�� �Ѵ�.
	NXVOID		*pUserData;
	pNxFFReader_WMDRM_Descramble pWMDRMDEC;
} nxFF_WM_DRM_T, *pNxFF_WM_DRM_T;

// DRM Type�� NXFF_MS_PLAYREADY_DRM�� ��� 
typedef struct _NxFF_ASF_PlayerReady_DRM_T {
	// pMidOffset�� nDividedPayloadCount ��ŭ ��� �ִ�.
	NXUINT8		nDividedPayloadCount;
	NXUINT32	pMidOffset[128];
	NXUINT32	nRepDataLen;
	NXUINT8		*pRepData;
	NXUINT32	nStreamNum;					// Stream Number
	NXUINT32	nPSIObjectLen;
	NXUINT8		*pPSIObject;
	// �ܺο��� ��� �����ִ�Descrambler ���� api �� �ڷ���
	// payload�� scramble �Ǿ� ���� ��� ��� �ȴ�.
	NXFF_WM_DRM_DEC_TYPE_T	nDecType;			// Descrambler�� Type �ܺο��� Seting ���־�� �Ѵ�.
	NXVOID		*pUserData;
	pNxFFReader_PRDRM_Descramble pPRDRMDEC;
} nxFF_ASFPL_DRM_T, *pNxFF_ASFPL_DRM_T;


// Smooth Streaming PlayReadDMR ����
typedef NXINT32(*pNxFFReader_SStreamingDRM_Descramble) (NXUINT8* pInputBuffer,  NXUINT32 uiInputBufferSize, 
										NXUINT8* pOutputBuffer, NXUINT32* puiOutputBufferSize,
										NXUINT8 *pSampleEncBox, NXUINT32 nSampleEncBoxLen, 
										NXUINT32 nSampleIDX, NXUINT32 nTrackID, NXVOID * pUserData);

typedef struct _NXFF_SSTREAMING_PRDRM_T {
	NXUINT32	nSampleEncBoxLen;
	NXUINT8		*pSampleEncBox;
	NXUINT32	nIVLen;
	NXUINT32	nIVStartOffset;		// Initial Vector offset

	NXVOID		*pUserData;			// Ȥ�� �ʿ��� �� �ִ� �����͸� �ܺο��� Setting ���ش�.
									// �̵����ʹ� ���ο��� free���� �ʴ´�.
	pNxFFReader_SStreamingDRM_Descramble pDescrambler;
}nxFF_SSTREAMING_PRDRM_T, *pNxFF_SSTREAMING_PRDRM_T;


// PIFF PlayReady DMR ����
typedef NXINT32(*pNxFFReader_PIFF_PRDRM_Descrambler) (NXUINT8* pInputBuffer,  NXUINT32 uiInputBufferSize, 
										NXUINT8* pOutputBuffer, NXUINT32* puiOutputBufferSize,
										NXUINT8 *pSampleEncBox, NXUINT32 nSampleEncBoxLen, 
										NXUINT32 nSampleIDX, NXUINT32 nTrackID, NXVOID * pUserData);

typedef struct _NXFF_PIFF_PRDRM_T {
	NXUINT32	nASampleEncBoxLen;	// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.
	NXUINT32	nAMSEL;				// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.
	NXUINT8		*pASampleEncBox;	// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.
	NXUINT32	nVSampleEncBoxLen;	// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.
	NXUINT32	nVMSEL;				// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.
	NXUINT8		*pVSampleEncBox;	// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.

	NXVOID		*pUserData;			// Ȥ�� �ʿ��� �� �ִ� �����͸� �ܺο��� Setting ���ش�.
									// �̵����ʹ� ���ο��� free���� �ʴ´�.

	pNxFFReader_PIFF_PRDRM_Descrambler pDescrambler;
}nxFF_PIFF_PRDRM_T, *pNxFF_PIFF_PRDRM_T;


// DECE UV DMR ����
typedef NXINT32(*pNxFFReader_DECE_UV_Descrambler) (NXUINT8* pInputBuffer,  NXUINT32 uiInputBufferSize, 
										NXUINT8* pOutputBuffer, NXUINT32* puiOutputBufferSize,
										NXUINT8 *pSampleEncBox, NXUINT32 nSampleEncBoxLen, 
										NXUINT32 nSampleIDX, NXUINT32 nTrackID, NXVOID * pUserData);

typedef struct _NXFF_DECE_UVDRM_T {
	NXUINT32 nASampleEncBoxLen;		// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.
	NXUINT32 nAMSEL;				// �ܺο������ʿ�������ο�������Ѵ�.
	NXUINT8 *pASampleEncBox;		// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.
	NXUINT32 nVSampleEncBoxLen;		// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.
	NXUINT32 nVMSEL;				// �ܺο������ʿ�������ο�������Ѵ�.
	NXUINT8 *pVSampleEncBox;		// �ܺο����� �ʿ���� ���ο��� ����Ѵ�.

	NXVOID *pUserData;				// Ȥ�� �ʿ��� �� �ִ� �����͸� �ܺο��� Setting ���ش�.
									// �̵����ʹ� ���ο��� free���� �ʴ´�.

	pNxFFReader_DECE_UV_Descrambler pDescrambler;
}nxFF_DECE_UVDRM_T, *pNxFF_DECE_UVDRM_T;

//for OMA_PDCF (NXFF_OMA_PDCF_DRM)
typedef struct _NXFF_OMA_DRM_PDCF_V2_TRACK_T {
	// Orignal Format Box 'frma'
	NXUINT32   frma_data_format;             // format of decrypted, encoded data

	// Scheme Type Box 'schm'
	NXUINT32   schm_scheme_type;             // 4CC identifying the scheme. must be 'odkm'
	NXUINT32   schm_scheme_version;			 // scheme version. must be 0x00000300
	NXUINT32   schm_scheme_urilen;           // not in spec. 
	NXUINT8		*schm_scheme_uri;             // browser uri use NULL

	// OMADRMCommonHeader Box 'ohdr'
	NXUINT8		odhr_encryption_method;       // Encryption method  0x02 => AES128_CTR
	NXUINT8		odhr_padding_scheme;          // Padding type	0
	NXUINT64    odhr_plaintext_length;// Plaintext content length in bytes 0

	NXUINT16	odhr_content_ID_length;       // Length of ContentID field in bytes, BCID
	NXUINT16	odhr_rights_issuer_URL_length;// Rights Issuer URL field length in bytes 
	NXUINT16	odhr_textual_headers_length;  // Length of the TextualHeaders array in bytes
	NXCHAR		*odhr_content_ID;             // Content ID string BCID
	NXCHAR		*odhr_rights_issuer_URL;      // Rights Issuer URL string
	NXCHAR		*odhr_textual_headers;        // Additional headers as Name:Value pairs NULL

	// OMADRMAUFormatBox 'odaf'
	// bit(1) SelectiveEncryption;		Must be 1 (DRM_DCF_2.0)
	// bit(7) Reserved;					Must be 0
	NXUINT8		odaf_flags;                   // odaf flags 1
	NXUINT8		odaf_key_indicator_length;    // Size of the key indicator in bytes 0
	NXUINT8		odaf_IV_length;               // size of the IV in bytes 16 

	// OMADRMGroupID Box 'grpi'
	NXUINT16	grpi_GroupIDLength;					//length of the Group ID URI
	NXUINT8		grpi_GKEncryptionMethod;					//Group key encryption algorithm
	NXUINT16	grpi_GKLength;							//length of the encrypted Group key
	NXCHAR		*grpi_GroupID;							//Group ID URI
	NXCHAR		*grpi_GroupKey;							//Encrypted Group key and encryption information

	// OMADRMTransactionTracking Box 'odtt'
	NXCHAR		odtt_TransactionID[16];					//value to enable transaction tracking = not in use

	// OMADRMRightsObject Box 'odrb'
	NXCHAR		*odrb_Data;								//binary Rights Object
															//�Ｚ������Ͽ�����޶������ֽ��ϴ�.

	NXUINT8		pCIEKandIV[32];					// writer ���� CIEK �� IV�� ���⿡ �־� �д�.
	NXVOID		*pExtraData;							// ��Ÿ �ӽ� ������ ���⿡���� OMADRMRecordingInformationBlckBase �� ����ִ�.
}nxFF_OMA_DRM_PDCF_V2_TRACK_T, *pNxFF_OMA_DRM_PDCF_V2_TRACK_T;



typedef struct _NxFF_OMA_DRM_PDCF_V2_T {
	NXUINT32			nAutomSize;			// Writer ����
	NXUINT32			nDRMTrackNumber;    //encv,enca�ǰ���
	NXINT32				nENCVTrack;         //�����Ұ������
	NXINT32				nENCATrack;         //�����Ұ������
	nxFF_OMA_DRM_PDCF_V2_TRACK_T  **ppDRMTracks; //Tracks
	NXUINT8 *pDRMFrameBuf;		// �ػ� * 1.5;
}nxFF_OMA_DRM_PDCF_V2_T, *pNxFF_OMA_DRM_PDCF_V2_T;


//for OMA BCast APDCF
// not implement yet
typedef struct _NxFF_OMA_BCAST_PDCF_OBKI_T {
	NXUINT8			obki_flags;		// bit(1) KeyIssuerPresent; indicates that the key issuer URL is present
									// bit(1) STKMPresent;		indicates that the STKM is present (only to be used for DCF)
									// bit(1) TBKPresent;		indicates that the TerminalBindingKey information is present
									// bit(1) TBKIssuerURLPresent;	indicates that the TBK issuer URL for TBK  is present
									// bit(4) rfu;				reserved for future use
	NXUINT8			obki_key_ID_type;		// indicates the type of key id that follows
	NXUINT8			obki_Key_ID_length; 	// KeyID length in bytes
	NXUINT8			*obki_Key_ID;					// key_id
	NXUINT16		obki_key_issuer_URL_length;		// KeyIssuer URL field length in bytes
	NXCHAR			*obki_key_issuer_URL;			// KeyIssuer URL string
	NXUINT16		obki_STKM_length;	 	// STKM field length in bytes
	NXUINT8			*obki_STKM;				// STKM
	NXUINT32		obki_TBK_id; 			// TerminalBindingKeyID
	NXUINT16		obki_TBK_issuer_URL_length; 	// TBK Issuer URL field length in bytes
	NXCHAR			*obki_TBK_issuer_URL;			// TBKIssuer URL string
}nxFF_OMA_BCAST_PDCF_OBKI_T, *pNxFF_OMA_BCAST_PDCF_OBKI_T;

typedef struct _NxFF_OMA_BCAST_PDCF_DRM_TRACK_T {
	// Orignal Format Box 'frma'
	NXUINT32	frma_data_format;		// format of decrypted, encoded data

	// Scheme Type Box 'schm'
	NXUINT32	schm_scheme_type;		// 4CC identifying the scheme. must be 'odkm'
	NXUINT32	schm_scheme_version;	// scheme version. must be 0x00000300
	NXUINT32	schm_scheme_urilen;		// not in spec. 
	NXUINT8	*schm_scheme_uri;		// browser uri

	// OMADRMCommonHeader Box 'ohdr'
	NXUINT8		odhr_encryption_method;		// Encryption method
	NXUINT8		odhr_padding_scheme;		// Padding type
	NXUINT32	odhr_plaintext_length;		// Plaintext content length in bytes
	NXUINT16	odhr_content_ID_length;		// Length of ContentID field in bytes
	NXUINT16	odhr_rights_issuer_URL_length;	// Rights Issuer URL field length in bytes
	NXUINT16	odhr_textual_headers_length;	// Length of the TextualHeaders array in bytes
	NXCHAR		*odhr_content_ID;			// Content ID string
	NXCHAR		*odhr_rights_issuer_URL;	// Rights Issuer URL string
	NXCHAR		*odhr_textual_headers;		// Additional headers as Name:Value pairs
	
	// OMABCAST Key Info Box 'obki'
	NXUINT8		obki_key_infos_number;			// indicates the number of key infos that follow
	nxFF_OMA_BCAST_PDCF_OBKI_T	**obki_key_infos;	// array of key info 

	// OMADRMSalt Box 'oslt'
	NXUINT8		oslt_salt_length;	// Length of the Salt field in bits. MUST be 64
	NXUINT8		*oslt_salt;			// Salt needed for AES_128_BYTE_CTR

	// OMADRMAUFormatBox 'odaf'
	NXUINT8		odaf_flags;					// odaf flags
	NXUINT8		odaf_key_indicator_length;	// Size of the key indicator in bytes
	NXUINT8		odaf_IV_length;				// size of the IV in bytes	
}nxFF_OMA_BCAST_PDCF_DRM_TRACK_T, *pNxFF_OMA_BCAST_PDCF_DRM_TRACK_T;

typedef struct _NxFF_OMA_BCAST_PDCF_DRM_T {
	NXUINT32	nDRMTrackNumber;		// encv/enca �� ����
	NXINT32		nENCVTrack;				// encv�� ID ������ ���� ��� ���ϴ� ���...
	NXINT32		nENCATrack;				// enca�� ID ������ ���� ��� ���ϴ� ���...
	nxFF_OMA_BCAST_PDCF_DRM_TRACK_T **ppDRMTraks;	//Tracks... 

	// OMA Key Sample Entry Box 'oksd'
	NXUINT8		oksd_sample_version;			// sample version
	NXUINT8		oksd_sample_type;				// sample type
	NXUINT32	oksd_terminal_binding_Key_ID;	// from the SG
	NXUINT16	oksd_rights_issuer_URI_length;	// Rights Issuer URI field length in bytes
	NXCHAR		*oksd_rights_issuer_URI;		// Rights Issuer URI string

}nxFF_OMA_BCAST_PDCF_DRM_T, *pNxFF_OMA_BCAST_PDCF_DRM_T;


// nMediatype = 0 Audio 
//              1 Video
typedef NXINT32(*pNxFFReader_DIVXDRM_InsertDecryptInfo) (NXUINT32 nMediatype, NXUINT8* pFrameBuffer, NXUINT32* pFrameBufferSize, NXVOID* pUserData);


typedef struct _NxFF_DIVX_DRM_T {
	NXUINT32 nDRMKeys;

	struct _DRM_Key {
		NXUINT32 nKeyLen;
		NXUINT8 *pKey;
	} *pDRMKeys;

	NXVOID*    pUserData;
    pNxFFReader_DIVXDRM_InsertDecryptInfo   pInsertDecryptInfo;

}nxFF_DIVX_DRM_T, *pNxFF_DIVX_DRM_T;


// NXFF_HLS_FRAME_DRM ���� �ڷᱸ��
typedef NXINT32(*pNxFFReader_HLS_FRAMEDecrypter) (NXUINT8 *pKey, NXUINT32 nKeyLen, NXUINT8 *pIV, NXUINT32 nIVLen, NXUINT8* pFrameBuffer, NXUINT32 nFrameBufferLen);

typedef struct _NxFF_HLS_FRAME_DRM_T {
	NXUINT32	nKeyLen;
	NXUINT8		pKey[32];
	NXUINT32	nIVLen;
	NXUINT8		pIV[32];
	
	pNxFFReader_HLS_FRAMEDecrypter	pDecrypter;
}nxFF_HLS_FRAME_DRM_T, *pNxFF_HLS_FRAME_DRM_T;


/******************************************************************************
 * Function Name : 
 * 
 * Return Value Description
 *
 * Comment : ISO/IEC 23001-7 Common encryption in ISO base media file format 
******************************************************************************/
typedef NXINT32(*pNxFFReader_ISOBMFF_FrameDecrypter) (NXINT64 hSH, NXCHAR *pSAI, NXUINT32 dwSAILen, NXCHAR *pEncFrame, NXUINT32 dwEncFrameLen, NXCHAR *pDecFrame, NXUINT32 *dwDecFrameLen, NXVOID *pUserData);

#define ISOBMFF_DRM_MAX_RO		300



typedef struct _NxFF_ISOBMFF_DRM_T {
	NXINT64		hSH;				// DRM ����Ҷ� �־� �־�� ��.
	NXVOID*		pUserData;			// �ܺο��� ����. ���ο����� ������� ����

	// ���� ������ �ܺο��� ����ؾ� �Ѵ�.
	NXUINT32	nRO;								// Right Object ����
	NXCHAR		pRONames[ISOBMFF_DRM_MAX_RO][5];	// Right Object Name with null terminator symbol.
	NXUINT32	nROSize[ISOBMFF_DRM_MAX_RO];		// Right Object Size;
	NXCHAR		**pROs;								// �ϴ� Right Object ISOBMFF_DRM_MAX_RO ������ �����Ѵ�.

	// ��Ÿ ���� ����. �ܺο����� ������� ����
	NXUINT32	nInfo;
	NXCHAR		*pInfo;
	NXVOID		*pSAInfos;

	// �ܺο��� ������� Callback
	pNxFFReader_ISOBMFF_FrameDecrypter	pDecrypter;
}nxff_ISOBMFF_DRM_T, *pNxff_ISOBMFF_DRM_T;


// NXFF_ISOBMFF_DRM���� Mediacryptor�� ����Ҷ� KID�� ��� �ö� ���
typedef struct {
	NXUINT32	NIU1;		// not in use
	NXUINT8		pKID[16]; 
	NXUINT32    NIU2;		// not in use
}nxff_ISOBMFF_DRM_KID_T, *pNxff_ISOBMFF_DRM_KID_T;


#endif //_NXFF_DRM_H_
