/*-----------------------------------------------------------------------------
	File Name   :	NexCAL.h
	Description :	the header file for both user of nexCAL.
					(the client and the provider)
 ------------------------------------------------------------------------------

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

 NexStreaming Confidential Proprietary
 Copyright (C) 2014 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation

 Revision History was described at the bottom of this file.
-----------------------------------------------------------------------------*/

/**
* @mainpage NexCAL documentation
*
* NexCAL is an abbreviation for Nexstreaming's Codec Abstraction Layer. As inferred from the name,
* NexCAL is a codec interface module between codec clients such as NexPlayer and codec providers. 
*
* Codec implementation varies per target platforms. Porting Nexstreaming's solutions to a specific target platform
* is the job of implementing some of abstraction layers including this NexCAL. 
*
* This document describes the meaning of NexCAL APIs and the calling sequences. As NexCAL is an interface module,
* there are two parts which are using NexCAL. One is the client such as NexPlayer and the other is the codec provider.
* This document describes from a codec provider's point of view. 
*
* NexCAL helps the integrator of Nexstreaming's multimedia solutions to concentrate on the codec part, and a simple
* test code using NexCAL makes possible for easy debugging of codec. 
*
* NexCAL is solely for Nexstreaming multimedia solutions, hence codec types supported by NexCAL is limited for Nexstreaming solutions
*
*  @note Important: NexCAL refers to NexCommonDef.h, NexMediaDef.h, NexTypeDef.h and NexSAL.
*/

/**
 * @defgroup cal_apis NexCAL APIs
 * @brief NexCAL APIs
 * @{
 *   @defgroup cal_basic Basic definitions
 *   @brief NexCAL Basic definitions
 *   @{
 *   @}
 *   @defgroup cal_type Types
 *   @brief Types
 *   @{
 *   @}
 *   @defgroup cal_functions Functions
 *   @brief Functions
 *   @{
 *   @}
 * @}
 * @defgroup codec_func  Codec Functions
 * @brief The each function which is described in this chapter is an entry of the table described in NEXCALVideoDecoder,
 *			NEXCALAudioDecoder, NEXCALImageDecoder, NEXCALTextDecoder, NEXCALVideoEncoder, NEXCALAudioEncoder.
 *			a_pUserData is the common parameter that all functions in this chapter have. This parameter value should be same
 *			as the registered value using init function, so, the client of NexCAL must pass the value which is obtained by calling
 *			init function to the other functions. The provider of NexCAL should implement the following functions for the client of NexCAL.
 * @{
 *   @defgroup codec_common Common
 *   @{
 *   @}
 *   @defgroup video_dec NEXCALVideoDecoder
 *   @{
 *   @}
 *   @defgroup audio_dec NEXCALAudioDecoder
 *   @{
 *   @}
 *   @defgroup image_dec NEXCALImageDecoder
 *   @{
 *   @} 
 *   @defgroup text_dec NEXCALTextDecoder
 *   @{
 *   @}
 *   @defgroup video_enc NEXCALVideoEncoder
 *   @{
 *   @} 
 *   @defgroup audio_enc NEXCALAudioEncoder
 *   @{
 *   @}
 * @} 
*/

#ifndef _NEX_CAL_DOT_H_INCLUDED_
#define _NEX_CAL_DOT_H_INCLUDED_

#include "NexMediaDef.h"
#include "NexUtil.h"

#define NEXCAL_VERSION_MAJOR		4
#define NEXCAL_VERSION_MINOR		6
#define NEXCAL_VERSION_PATCH		0
#define NEXCAL_VERSION_BRANCH		"OFFICIAL"

#define NEXCAL_COMPATIBILITY_NUM	9


#if !defined(_NEX_AL_DLL_)
	#define _NEX_AL_DLL_
#endif

#if (defined(_WIN8) || defined(WIN32) || defined(WINCE)) && defined(_NEX_AL_DLL_)
	#if defined(NEXCAL_EXPORTS)
		#define NEXCAL_API __declspec(dllexport)
	#else
		#define NEXCAL_API __declspec(dllimport)
	#endif
#elif (defined(SYMBIAN60))
	#if (defined(NEXCAL_EXPORTS)
		#define NEXCAL_API	EXPORT_C
	#else
		#define NEXCAL_API	IMPORT_C
	#endif
#else
	#define NEXCAL_API
#endif

#define NTHFROMLSBMASK(n) (0x01<<(n))

/**
 * @ingroup cal_type
 * NexCAL identifier returned by nexCAL_GetHandle()
 * @see nexCAL_GetHandle
*/
typedef struct NEXCALHandle_struct *NEXCALHandle;
typedef struct NEXCALCodecHandle_struct *NEXCALCodecHandle;

/**
 * @ingroup cal_type
 * Enumeration type indicating codec media types
*/
typedef enum
{
	NEXCAL_MEDIATYPE_VIDEO = 0,			///< Video
	NEXCAL_MEDIATYPE_AUDIO = 1,			///< Audio
	NEXCAL_MEDIATYPE_TEXT  = 2,			///< Text
	NEXCAL_MEDIATYPE_IMAGE = 3,			///< Image
	NEXCAL_MEDIATYPE_VIDEO_EXTRA = 4, 	///< Video for PCCW
	NEXCAL_MEDIATYPE_THUMBNAIL = 5,		///< Thumbnail
} NEXCALMediaType;

/**
 * @ingroup cal_type
 * Enumeration type indicating the mode of codec
*/
typedef enum
{
	NEXCAL_MODE_DECODER = 0,	///< decoder mode
	NEXCAL_MODE_ENCODER = 1		///< encoder mode

} NEXCALMode;

/**
 * @ingroup cal_type
 * Enumeration type indicating the Policy of codec
*/
typedef enum
{
	NEXCAL_POLICY_NONE      = 0,	///< None
	NEXCAL_POLICY_HWONLY    = 1,	///< HW only
	NEXCAL_POLICY_SWONLY    = 2,	///< SW only
	NEXCAL_POLICY_HWSW      = 3,	///< priority HW -> SW
	NEXCAL_POLICY_SWHW      = 4,	///< priority SW -> HW
} NEXCALPolicy;

/**
 * @ingroup cal_type
 *	Methods to get a particular codec type
*/
typedef enum
{
	NEXCAL_METHOD_FOLLOW_POLICY 		= 0,
	NEXCAL_METHOD_HW					= 1,
	NEXCAL_METHOD_PLATFORM_SW			= 2,
	NEXCAL_METHOD_OWN_SW				= 3,
} NEXCALMethod;

// Video Decoder Directives
#define NEXCAL_INIT_VDEC_RET(ret) ((ret) = 0)
#define NEXCAL_SET_VDEC_RET(ret, flag) ((ret) = (ret) | (flag))
#define NEXCAL_DEL_VDEC_RET(ret, flag) ((ret) = (ret) & ~(flag))
#define NEXCAL_CHECK_VDEC_RET(ret, flag) ((ret)&(flag) ? TRUE : FALSE)

#define NEXCAL_VDEC_DECODING_SUCCESS	NTHFROMLSBMASK(0)
#define NEXCAL_VDEC_NEXT_FRAME			NTHFROMLSBMASK(1)
#define NEXCAL_VDEC_OUTPUT_EXIST		NTHFROMLSBMASK(2)
#define NEXCAL_VDEC_OUTPUT_TIMEOUT		NTHFROMLSBMASK(3)
#define NEXCAL_VDEC_EOS 				NTHFROMLSBMASK(4)
#define NEXCAL_VDEC_SETTING_CHANGE		NTHFROMLSBMASK(5)
#define NEXCAL_VDEC_RESET_OUTPUT_YUV	NTHFROMLSBMASK(6)
#define NEXCAL_VDEC_DECODE_WAIT			NTHFROMLSBMASK(7)

// Audio Decoder Directives
#define NEXCAL_INIT_ADEC_RET(ret) ((ret) = 0)
#define NEXCAL_SET_ADEC_RET(ret, flag) ((ret) = (ret) | (flag))
#define NEXCAL_DEL_ADEC_RET(ret, flag) ((ret) = (ret) & ~(flag))
#define NEXCAL_CHECK_ADEC_RET(ret, flag) ((ret)&(flag) ? TRUE : FALSE)

#define NEXCAL_ADEC_DECODING_SUCCESS 	NTHFROMLSBMASK(0)
#define NEXCAL_ADEC_NEXT_FRAME			NTHFROMLSBMASK(1)
#define NEXCAL_ADEC_OUTPUT_EXIST		NTHFROMLSBMASK(2)
#define NEXCAL_ADEC_OUTPUT_TIMEOUT		NTHFROMLSBMASK(3)
#define NEXCAL_ADEC_OUTPUT_CHANGED		NTHFROMLSBMASK(4)
#define NEXCAL_ADEC_EOS 				NTHFROMLSBMASK(5)
#define NEXCAL_ADEC_SETTING_CHANGED		NTHFROMLSBMASK(6)

// Text Decoder Directives
#define NEXCAL_INIT_TDEC_RET(ret) ((ret) = 0)
#define NEXCAL_SET_TDEC_RET(ret, flag) ((ret) = (ret) | (flag))
#define NEXCAL_DEL_TDEC_RET(ret, flag) ((ret) = (ret) & ~(flag))
#define NEXCAL_CHECK_TDEC_RET(ret, flag) ((ret)&(flag) ? TRUE : FALSE)

#define NEXCAL_TDEC_DECODING_SUCCESS	NTHFROMLSBMASK(0)
#define NEXCAL_TDEC_NEXT_FRAME			NTHFROMLSBMASK(1)
#define NEXCAL_TDEC_OUTPUT_EXIST		NTHFROMLSBMASK(2)
#define NEXCAL_TDEC_EOS 				NTHFROMLSBMASK(3)

// Image Decoder Directives
#define NEXCAL_INIT_IDEC_RET(ret) ((ret) = 0)
#define NEXCAL_SET_IDEC_RET(ret, flag) ((ret) = (ret) | (flag))
#define NEXCAL_DEL_IDEC_RET(ret, flag) ((ret) = (ret) & ~(flag))
#define NEXCAL_CHECK_IDEC_RET(ret, flag) ((ret)&(flag) ? TRUE : FALSE)

#define NEXCAL_IDEC_DECODING_SUCCESS	NTHFROMLSBMASK(0)
#define NEXCAL_IDEC_NEXT_FRAME			NTHFROMLSBMASK(1)
#define NEXCAL_IDEC_OUTPUT_EXIST		NTHFROMLSBMASK(2)
#define NEXCAL_IDEC_OUTPUT_TIMEOUT		NTHFROMLSBMASK(3)
#define NEXCAL_IDEC_EOS 				NTHFROMLSBMASK(4)

// Video Encoder Directives
#define NEXCAL_INIT_VENC_RET(ret) ((ret) = 0)
#define NEXCAL_SET_VENC_RET(ret, flag) ((ret) = (ret) | (flag))
#define NEXCAL_DEL_VENC_RET(ret, flag) ((ret) = (ret) & ~(flag))
#define NEXCAL_CHECK_VENC_RET(ret, flag) ((ret)&(flag) ? TRUE : FALSE)

#define NEXCAL_VENC_ENCODING_SUCCESS	NTHFROMLSBMASK(0)
#define NEXCAL_VENC_NEXT_INPUT			NTHFROMLSBMASK(1)
#define NEXCAL_VENC_OUTPUT_EXIST		NTHFROMLSBMASK(2)
#define NEXCAL_VENC_OUTPUT_TIMEOUT		NTHFROMLSBMASK(3)
#define NEXCAL_VENC_EOS 				NTHFROMLSBMASK(4)

// Audio Encoder Directives
#define NEXCAL_INIT_AENC_RET(ret) ((ret) = 0)
#define NEXCAL_SET_AENC_RET(ret, flag) ((ret) = (ret) | (flag))
#define NEXCAL_DEL_AENC_RET(ret, flag) ((ret) = (ret) & ~(flag))
#define NEXCAL_CHECK_AENC_RET(ret, flag) ((ret)&(flag) ? TRUE : FALSE)

#define NEXCAL_AENC_ENCODING_SUCCESS	NTHFROMLSBMASK(0)
#define NEXCAL_AENC_NEXT_INPUT			NTHFROMLSBMASK(1)
#define NEXCAL_AENC_OUTPUT_EXIST		NTHFROMLSBMASK(2)
#define NEXCAL_AENC_OUTPUT_TIMEOUT		NTHFROMLSBMASK(3)
#define NEXCAL_AENC_EOS 				NTHFROMLSBMASK(4)

//AUDIO Init Mode
#define NEXCAL_ADEC_MODE_NONE					0
#define NEXCAL_ADEC_MODE_MEDIACRYPTO			NTHFROMLSBMASK(0)

//AUDIO Flag type list
#define NEXCAL_ADEC_FLAG_NONE					0x00000000
#define NEXCAL_ADEC_FLAG_END_OF_STREAM			0x00000001

//VIDEO Init Mode
#define NEXCAL_VDEC_MODE_NONE					0
#define NEXCAL_VDEC_MODE_SEAMLESS				NTHFROMLSBMASK(0)
#define NEXCAL_VDEC_MODE_MEDIACRYPTO			NTHFROMLSBMASK(1)

//VIDEO Flag type list
#define NEXCAL_VDEC_FLAG_NONE					0
#define NEXCAL_VDEC_FLAG_ENHANCEMENT			NTHFROMLSBMASK(0)
#define NEXCAL_VDEC_FLAG_DECODE_ONLY			NTHFROMLSBMASK(1)
#define NEXCAL_VDEC_FLAG_END_OF_STREAM			NTHFROMLSBMASK(2)
#define NEXCAL_VDEC_FLAG_ENCRYPTED				NTHFROMLSBMASK(3)
#define NEXCAL_VDEC_FLAG_OUTPUT_ONLY			NTHFROMLSBMASK(4)
#define NEXCAL_VDEC_FLAG_DECODE_RESET			NTHFROMLSBMASK(5)
#define NEXCAL_VDEC_FLAG_IFRAME_MODE			NTHFROMLSBMASK(6)

//TEXT Init Mode
#define NEXCAL_TDEC_MODE_NONE					0
#define NEXCAL_TDEC_MODE_MEDIACRYPTO			NTHFROMLSBMASK(0)


// Error type list
#define NEXCAL_ERROR_NONE						0
#define NEXCAL_ERROR_FAIL						1
#define NEXCAL_ERROR_INVALID_PARAMETER			2
#define NEXCAL_ERROR_NOT_SUPPORT_AUDIO_CODEC	3
#define NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC	4
#define NEXCAL_ERROR_INVALID_CODEC				5
#define NEXCAL_ERROR_CORRUPTED_FRAME			6
#define NEXCAL_ERROR_SYSTEM_FAIL				7
#define NEXCAL_ERROR_MEMORY_FAIL				8
#define NEXCAL_ERROR_RESOURCE_FAIL				9
#define NEXCAL_ERROR_NOT_SUPPORT_DEVICE			10
#define NEXCAL_ERROR_OVER_SPEC 					11
#define NEXCAL_ERROR_OUTPUT_NOT_EXIST			12
#define NEXCAL_ERROR_UNSUPPORTED_RESOLUTION		13


/**
 * @ingroup codec_common
 * This function gets the property that each specific codec has
 * @param[in] a_uProperty The property indicator which is required by the client
 * @param[out] a_pqValue The requested property value 
 * @param[in] a_pUserData The User-Data
 * @return 0: Success, Others: Fail
 * @note The property of codec varies. However NexCAL defines the minimal properties which make
 *			an influence on the implementation method of the client of NexCAL.
*/
typedef NXINT32 (*NEXCALGetProperty)(NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData);

/**
 * @ingroup codec_common
 * This function sets the property that each specific codec has
 * @param[in] a_uProperty The property indicator which is required by the client
 * @param[in] Set the Value about the requested property
 * @param[in] a_pUserData The User-Data
 * @return 0: Success, Others: Fail
 * @note The property of codec varies. However NexCAL defines the minimal properties which make
 *			an influence on the implementation method of the client of NexCAL.
*/
typedef NXINT32 (*NEXCALSetProperty)(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData);

/**
 * @ingroup video_dec
 * This function initializes the video decoder.
 * @param[in] eCodecType Indicates the codec object type indication values
 * @param[in] a_pConfig Pointer that refers to the information of video base layer
 * @param[in] a_nConfigLen Length of the information of video base Layer in byte
 * @param[in] a_pFrame Frame data
 * @param[in] a_nFrameLen Length of frame
 * @param[in] a_pInitInfo Infomation that needs when initializing
 * @param[in] a_pExtraInfo Extra infomation that needs when decoding
 * @param[in] a_nNALHeaderSize Additional information according to the value of a_eCodecType. In current version, it is valid only if the a_eCodecType
 *				value is eNEX_CODEC_V_H264 and means that the number of bytes which represents NAL Header Length. Other a_eCodecType value is not valid. 
 * @param[out] a_pnWidth Pointer to the width of video resolution. This value must be set if succeeded.
 * @param[out] a_pnHeight Pointer to the height of video resolution. This value must be set if succeeded.
 * @param[out] a_pnPitch Pointer to the pitch information in pixel. This value must be set if succeeded.
 * @param[in] a_uMode Video initialization mode
 *				- NEXCAL_VDEC_MODE_NONE
 *				- NEXCAL_VDEC_MODE_SEAMLESS
 *				- NEXCAL_VDEC_MODE_MEDIACRYPTO
 * @param[in] a_uUserDataType  User data type (default: unique client id). If there is not under special conditions, value is NXUSERDATA_PLAYERHANDLE.
 * @param[in,out] a_ppUserData When this function is called by client, this parameter is set with the value that can distinguish client. So, 2 or more 
 *					Nexstreaming's Application Engines are run in Nexstreaming's Application Engine address space at the same time, Codec provider
 *					can distinguish the client by this parameter. Codec provider can set the value before returning. Client that received this value passed 
 *					the value continuously when called this function continuously.
 * @return 0: Success, Others: Fail
 * @note This function is used to initialize the video decoder. This function sets a_pnWidth, a_pnHeight and a_pnPitch of contents before return.
 *			These resolution values are used to initialize the NexRAL in client, but do not have a direct effect on client internally.  If the body of
 *			NexCAL and NexRAL are tightly coupled and didn't know these value, these values can be set as 0.\n\n
 *			The meaning of a_pConfig, a_pFrame depends on video object types.\n\n
 *			1) eNEX_CODEC_V_H263\n
 *				a_pConfig: Pointer to first frame buffer\n
 *				a_pFrame: Normal frame\n
 *			2) eNEX_CODEC_V_MPEG4V\n
 *				a_pConfig: Pointer to DSI(Decoder Specific Information) of base layer
 * 							For the syntax of DSI, refer to 6.2 Visual bitstream syntax of 1SO/IEC 14496-2.
 *							First 7 bytes are the start code prefix (0x000001) and visual_object_sequence_start_code (0xB0).
 *							Normally, it is described in sequence of visual sequence object header, visual object header and video object layer header. User data can be included in DSI.\n
 *				a_pFrame: Pointer to DSI of enhanced layer if TS (Temporal scalability) is set. Otherwise it is set to NULL.\n
 *			3) eNEX_CODEC_V_H264\n
 *				a_pConfig: Pointer to Parameter Set of H.264. This syntax consists of SPS (sequence parameter set) and PPS (picture parameter set) described in.
 *							This byte stream format might be two types. One of them is "Annex B byte stream format" which is described in "Text of ISO/IEC 14496 10 Advanced
 *							Video Coding 3rd Edition" and consists as Raw data type like Figure 2. The structure of PS. Refer to Appendix A.3.1. for more information about whether
 *							can be inputted all these two formats, or only one.\n
 *				a_pFrame: Normal frame
*/
typedef NXINT32 (*NEXCALVideoInit)(NEX_CODEC_TYPE eCodecType, 
								   NXUINT8 *a_pConfig, 
								   NXINT32 a_nConfigLen, 
								   NXUINT8 *a_pFrame, 
								   NXINT32 a_nFrameLen, 
								   NXVOID *a_pInitInfo,
								   NXVOID *a_pExtraInfo,
								   NXINT32 a_nNALHeaderSize, 
								   NXINT32 *a_pnWidth, 
								   NXINT32 *a_pnHeight, 
								   NXINT32 *a_pnPitch, 
								   NXUINT32 a_uMode, 
								   NXUINT32 a_uUserDataType, 
								   NXVOID **a_ppUserData);

/**
 * @ingroup video_dec
 * This function closes the video decoder.
 * @param[in] a_pUserData  The user-data which was got when the client called Init.
 * @return 0: Success, Others: Fail
 * @note When the video decoder will no longer be used, this function must be called.
*/
typedef NXINT32 (*NEXCALVideoDeinit)(NXVOID *a_pUserData);

/**
 * @ingroup video_dec
 * This function decodes video frame.
 * @param[in] a_pFrame Pointer of one frame to be decoded 
 * @param[in] a_nFrameLen Size of the frame to be decoded
 * @param[in] a_pExtraInfo Extra infomation that needs when decoding
 * @param[in] a_uDTS Decoder time stamp
 * @param[in] a_uPTS Presentation time stamp
 * @param[in] a_nFlag Video decode status flag (OR operation)
 *					- NEXCAL_VDEC_FLAG_NONE
 *					- NEXCAL_VDEC_FLAG_ENHANCEMENT
 *					- NEXCAL_VDEC_FLAG_DECODE_ONLY
 *					- NEXCAL_VDEC_FLAG_END_OF_STREAM
 *					- NEXCAL_VDEC_FLAG_ENCRYPTED
 *					- NEXCAL_VDEC_FLAG_OUTPUT_ONLY
 *					- NEXCAL_VDEC_FLAG_DECODE_RESET
 *					- NEXCAL_VDEC_FLAG_IFRAME_MODE
 * @param[out] a_puDecodeResult Decoding result (OR operation)
 * @param[in] a_pUserData The user-data which is obtained when the client called Init.
 * @return 0: Success, Others: Fail
*/
typedef NXINT32 (*NEXCALVideoDecode)(NXUINT8 *a_pFrame, 
									 NXINT32 a_nFrameLen, 
									 NXVOID *a_pExtraInfo,
									 NXUINT32 a_uDTS, 
									 NXUINT32 a_uPTS, 
									 NXINT32 a_nFlag, 
									 NXUINT32 *a_puDecodeResult, 
									 NXVOID *a_pUserData);

/**
 * @ingroup video_dec
 * This function gets decoded ouput(display) data. This data is composed of output pixel format.
 * @param[out] a_ppBits1 First output buffer pointer
 *							For example, if the type of output buffer is OUTPUTBUFFER_TYPE_SEPERATED_YUV420 then this value points pointer to Y-plane buffer.
 * @param[out] a_ppBits2 Second output buffer pointer
 *							For example, if the type of output buffer is OUTPUTBUFFER_TYPE_SEPERATED_YUV420 then this value points pointer to U-plane buffer.
 * @param[out] a_ppBits3 Third output buffer pointer
 *							For example, if the type of output buffer is OUTPUTBUFFER_TYPE_SEPERATED_YUV420 then this value points pointer to V-plane buffer.
 * @param[out] a_puPTS Presentation time stamp
 * @param[in] a_pUserData The user-data which was got when the client called Init.
 * @return 0: Success, Others: Fail
 * @note If the provider of NexCAL provides the specific renderer, three parameters for output can be used freely.
 *		In some cases, the client of NexCAL should not use three parameters for output. 
*/
typedef NXINT32 (*NEXCALVideoGetOutput)(NXUINT8 **a_ppBits1,
										NXUINT8 **a_ppBits2, 
										NXUINT8 **a_ppBits3, 
										NXUINT32 *a_puPTS, 
										NXVOID *a_pUserData);


/**
 * @ingroup video_dec
 * This function resets decoder.
 * @param[in] a_pUserData The user-data which was got when the client called Init.
 * @return 0: Success, Others: Fail
*/
typedef NXINT32 (*NEXCALVideoReset)(NXVOID *a_pUserData);

/**
 * @ingroup video_dec
 * This function gets information data.
 * @param[in] a_uIndex  Request Infomation index
 *						- NEXCAL_VIDEO_GETINFO_WIDTH
 *						- NEXCAL_VIDEO_GETINFO_HEIGHT
 *						- NEXCAL_VIDEO_GETINFO_WIDTHPITCH 
 *						- NEXCAL_VIDEO_GETINFO_HEIGHTPITCH
 *						- NEXCAL_VIDEO_GETINFO_NUM_INPUTBUFFERS
 *						- NEXCAL_VIDEO_GETINFO_INPUTBUFFER_SIZE
 *						- NEXCAL_VIDEO_GETINFO_NUM_OUTPUTBUFFERS
 *						- NEXCAL_VIDEO_GETINFO_OUTPUTBUFFER_SIZE
 *						- NEXCAL_VIDEO_GETINFO_RENDERER_RESET
 *						- NEXCAL_VIDEO_GETINFO_AVERAGE_DECODING_TIME  
 *						- NEXCAL_VIDEO_GETINFO_CODEC_CLASS
 *						- NEXCAL_VIDEO_GETINFO_NEED_CODEC_REINIT
 * @param[out] a_puResult The requested Info value
 * @param[in] a_pUserData The user-data which was got when the client called Init.
 * @return 0: Success, Others: Fail
*/
typedef NXINT32 (*NEXCALVideoGetInfo)(NXUINT32 a_uIndex, NXUINT32 *a_puResult, NXVOID *a_pUserData);

/**
 * @ingroup audio_dec
 * This function initializes the audio decoder.
 * @param[in] a_eCodecType Indicates the codec (object) type indication value
 * @param[in] a_pConfig Pointer that refers to the decoder specific information. This value is just for few decoders such as AAC.
 * @param[in] a_nConfigLen Length of the decoder specific information in byte
 * @param[in] a_pFrame Pointer to the first frame, if pConfig is NULL. 
 * @param[in] a_nFrameLen Length of the first frame
 * @param[in] a_pInitInfo Infomation that needs when initializing
 * @param[in] a_pExtraInfo Extra infomation that needs when decoding
 * @param[out] a_puSamplingRate Pointer to the sampling rate of audio. If this value can be obtained, the value should be set in Hz otherwise set as "0".
 * @param[out] a_puNumOfChannels Pointer to the number of channels. If this value can be obtained the value should be set, Otherwise set as "0". (Ex. mono: 1, stereo: 2)
 * @param[out] a_puBitsPerSample Pointer to the number of bits per sample. If this value can be obtained, the value should be set, Otherwise set as "0".
 *								The client of NexCAL regard the default value (0) as 16 bits.
 * @param[out] a_puNumOfSamplesPerChannel Pointer to the number of samples per channel. If this value can be obtained, the value should be set, Otherwise set as "0".
 * @param[in] a_uMode Video initialization mode
 *					- NEXCAL_ADEC_MODE_NONE
 *					- NEXCAL_ADEC_MODE_MEDIACRYPTO
 * @param[in] a_uUserDataType  User data type (default: unique client id)\n
 *						If there is not under special conditions, value is NXUSERDATA_PLAYERHANDLE.
 * @param[in,out] a_ppUserData [in] When this function is called by client , this parameter is set with the value that can distinguish client. So, 2 or more
 *								Nexstreaming's Application Engines are run in Nexstreaming's Application Engine address space   at the same time, Codec provider can
 *								distinguish the client by this parameter. 
 *								Codec provider can set the value before returning. Client that received this value passed the value continuously when called this function continuously.\n
 *								[out] Codec provider can freely use this parameter. 
 * @return 0: Success, Others: Fail
 * @note This function sets a_puSamplingRate, a_puNumOfChannels, a_puBitsPerSample and a_puNumOfSamplesPerChannel of contents for output.
 *		If offered decoder from third party, it might happen not to parse 4 parameter above. In this case, when ask 'SET_RIGHT_OUTPUT_INFORMATION'
 *		of GetProperty, should return 'NO'. In this case, Client (Nexstreaming Application Engine) ignores output parameter values of this function, and does
 *		and use parsing as far as parsing is possible.\n\n
 *		The meaning of pConfig, pFrame depends on audio object type.\n\n
 *		1) eNEX_CODEC_A_AAC\n
 *			a_pConfig is pointer to MPEG-4 Audio Specific Configuration Information.
 *				Refer to 1.6.Interface to ISO/IEC 14496-1 of ISO/IEC 14496-3.\n
 *			a_pFrame has no meaning\n
 *		2) Other Value\n
 *			a_pConfig has no meaning\n
 *			a_pFrame is the pointer of first frame.
*/
typedef NXINT32 (*NEXCALAudioInit)(NEX_CODEC_TYPE a_eCodecType, 
								   NXUINT8 *a_pConfig, 
								   NXINT32 a_nConfigLen, 
								   NXUINT8 *a_pFrame, 
								   NXINT32 a_nFrameLen, 
								   NXVOID *a_pInitInfo,
								   NXVOID *a_pExtraInfo,
								   NXUINT32 *a_puSamplingRate, 
								   NXUINT32 *a_puNumOfChannels, 
								   NXUINT32 *a_puBitsPerSample, 
								   NXUINT32 *a_puNumOfSamplesPerChannel, 
								   NXUINT32 a_uMode,
								   NXUINT32 a_uUserDataType, 
								   NXVOID **a_ppUserData);

/**
 * @ingroup audio_dec
 * This function closes the audio decoder.
 * @param[in] a_pUserData The user-data which was obtained when Init is called by the client.
 * @return 0: Success, Others: Fail
 * @note When the audio decoder is no longer used, this function must be called.
*/
typedef NXINT32 (*NEXCALAudioDeinit)(NXVOID *a_pUserData);

/**
 * @ingroup audio_dec
 * This function decodes the audio frame.
 * @param[in] a_pFrame Pointer that refers to audio codec data to be decoded.
 * @param[in] a_nFrameLen Length of the audio frame data to be decoded
 * @param[in] a_pExtraInfo  Extra infomation that needs when decoding
 * @param[out] a_pDest Pointer that refers to the location where decoded frame data will be saved.
 *						If this value contains real PCM data, the client must provide a sufficient space holding
 *						the result of decoder. The space can be calculated using the output of Init function. 
 *						Based on properties of the decoder, output format can be either single memory buffer
 *						or two independent buffers for left and right. This property can be obtained using GetProperty, 
 *						and for latter case, left out pointer can be pDest and right out pointer can be "pDest+lengthof(pDest)/2"
 * @param[in] a_pnWrittenPCMSize Size of PCM that is output of decoder
 * @param[in] a_uDTS Decoder time stamp
 * @param[in] a_puOutputTime output time calculated by amount of PCM
 * @param[in] a_nFlag This flag controls the operation of decoder
 *						- NEXCAL_ADEC_FLAG_NONE
 *						- NEXCAL_ADEC_FLAG_END_OF_STREAM
 * @param[in] a_puDecodeResult Decoding result
 * @param[in] a_pUserData The user-data which was obtained when Init is called by the client.
 * @return 0: Success, Others: Fail
*/
typedef NXINT32 (*NEXCALAudioDecode)(NXUINT8 *a_pFrame, 
									 NXINT32 a_nFrameLen,
									 NXVOID *a_pExtraInfo,
									 NXVOID *a_pDest,
									 NXINT32 *a_pnWrittenPCMSize, 
									 NXUINT32 a_uDTS, 
									 NXUINT32 *a_puOutputTime, 
									 NXINT32 a_nFlag, 
									 NXUINT32 *a_puDecodeResult, 
									 NXVOID *a_pUserData);

/**
 * @ingroup audio_dec
 * This function resets audio decoder.
 * @param[in] a_pUserData The user-data which was obtained when Init is called by the client.
 * @return 0: Success, Others: Fail
 * @note When random access (including FW/REW) occurs or packet loss occurs, this function is called to eliminate noise by resetting audio decoder.
*/
typedef NXINT32 (*NEXCALAudioReset)(NXVOID *a_pUserData);

/**
 * @ingroup audio_dec
 * This function gets information data.
 * @param[in] a_uIndex Request Information index
 *					- NEXCAL_AUDIO_GETINFO_SAMPLINGRATE
 *					- NEXCAL_AUDIO_GETINFO_NUMOFCHNNELS
 *					- NEXCAL_AUDIO_GETINFO_BITSPERSAMPLE
 *					- NEXCAL_AUDIO_GETINFO_NUMOFSAMPLESPERCHANNEL
 * @param[out] a_puResult The requested Info value 
 * @param[in] a_pUserData The user-data which was obtained when Init is called by the client. 
 * @return 0: Success, Others: Fail
*/
typedef NXINT32 (*NEXCALAudioGetInfo)(NXUINT32 a_uIndex, NXUINT32 *a_puResult, NXVOID *a_pUserData);

/**
 * @ingroup image_dec
 * This function gets information data.
 * @param[in] a_eCodecType Indicates the codec object type indication values
 * @param[in] a_strFileName Image file name
 * @param[in] a_pSrc Image data buffer pointer
 * @param[in] a_uSrcLen Image data buffer length
 * @param[out] a_pnWidth Pointer to the width of video resolution
 * @param[out] a_pnHeight Pointer to the height of video resolution
 * @param[out] a_pnPitch Pointer to the pitch information in pixel
 * @param[in] a_uUserDataType User data type (default: unique client id)
 *						 If there is not under special conditions, value is NXUSERDATA_PLAYERHANDLE.
 * @param[out] a_ppUserData The user-data. The provider can pass custom value. This value will be passed with
 *							the last parameter by the client, when other functions are called.
 * @return 0: Success, Others: Fail
 * @note This function is used to extract width, height and pitch information using header information of image data. 
 *			Input data can either be image file or buffer pointer. Therefore, when image file is entered, a_pFilename 
 *			should be full path of image file, and a_pSrc and a_uSrcLen should have value of NULL and "0". 
 *			When image data buffer value is entered as input data, a_ strFileName should be Null, and a_pSrc and 
 *			a_uSrcLen should have values of buffer pointer and buffer length values.
*/
typedef NXINT32 (*NEXCALImageGetHeader)(NEX_CODEC_TYPE a_eCodecType, 
										NXCHAR *a_strFileName, 
										NXUINT8 *a_pSrc, 
										NXUINT32 a_uSrcLen, 
										NXINT32 *a_pnWidth, 
										NXINT32 *a_pnHeight, 
										NXINT32 *a_pnPitch, 
										NXUINT32 a_uUserDataType, 
										NXVOID **a_ppUserData);

/**
 * @ingroup image_dec
 * This function decodes the video frame.
 * @param[in] a_strFileName Fullpath of image file
 * @param[in] a_pSrc Pointer of image data buffer
 * @param[in] a_uSrcLen Buffer length of image data buffer
 * @param[out] a_pnWidth Width of decoded image
 * @param[out] a_pnHeight Height of decoded image
 * @param[out] a_pnPitch Pitch of decoded image buffer
 * @param[out] a_pBits1 Pointer to save decoded Y data of image
 * @param[out] a_pBits2 Pointer to save decoded U data of image
 * @param[out] a_pBits3 Pointer to save decoded V data of image
 * @param[out] a_puDecodeResult Decoding result
 * @param[in] a_pUserData The user-data which will be obtained when GetHeader is called.
 * @return 0: Success, Others: Fail
 * @note The output format of decoded image is obtained by calling GetProperty function and based on this obtained value.
 *			YUV can be outputted as a single buffer. When this happens, set YUV buffer's pointer as input data for pBits1 and pBits2 and pBits3 shall be ignored.
*/
typedef NXINT32 (*NEXCALImageDecode)(NXCHAR *a_strFileName, 
									 NXUINT8 *a_pSrc, 
									 NXUINT32 a_uSrcLen, 
									 NXINT32 *a_pnWidth, 
									 NXINT32 *a_pnHeight, 
									 NXINT32 *a_pnPitch, 
									 NXUINT8 *a_pBits1, 
									 NXUINT8 *a_pBits2, 
									 NXUINT8 *a_pBits3, 
									 NXUINT32 *a_puDecodeResult, 
									 NXVOID *a_pUserData);

/**
 * @ingroup video_enc
 * This function initializes the video encoder.
 * @param[in] a_eCodecType Indicates the codec object type indication values
 * @param[in] a_ppConfig Pointer that refers to the information of video base layer
 * @param[in] a_pnConfigLen Length of the information of video base Layer in byte
 * @param[in] a_nQuality Quality
 * @param[in] a_nWidth the width of video resolution
 * @param[in] a_nHeight the height of video resolution
 * @param[in] a_nPitch the pitch of video resloution
 * @param[in] a_nFPS Frame Per Second
 * @param[in] a_bCBR 
 * @param[in] a_nBitrate  
 * @param[in] a_ppUserData 
 * @return 0: Success, Others: Fail
 * @note The output format of decoded image is obtained by calling GetProperty function and based on this obtained value.
 *			YUV can be outputted as a single buffer. When this happens, set YUV buffer's pointer as input data for pBits1 and pBits2 and pBits3 shall be ignored.
*/
typedef NXINT32 (*NEXCALVideoEncoderInit)(NEX_CODEC_TYPE a_eCodecType, 
										  NXUINT8 **a_ppConfig, 
										  NXINT32 *a_pnConfigLen, 
										  NXINT32 a_nQuality, 
										  NXINT32 a_nWidth, 
										  NXINT32 a_nHeight, 
										  NXINT32 a_nPitch,
										  NXINT32 a_nFPS, 
										  NXBOOL a_bCBR, 
										  NXINT32 a_nBitrate, 
										  NXVOID **a_ppUserData);

/**
 * @ingroup video_enc
 * This function closes the video encoder.
 * @param[in] a_pUserData The user-data which was obtained when Init is called by the client.
 * @return 0: Success, Others: Fail 
*/
typedef NXINT32 (*NEXCALVideoEncoderDeinit)(NXVOID *a_pUserData);

/**
 * @ingroup video_enc
 *  
 * @param[in]a_pData1 
 * @param[in]a_pData2 
 * @param[in]a_pData3 
 * @param[in]a_uPTS 
 * @param[in]a_puEncodeResult 
 * @param[in]a_pUserData 
 * @return 0: Success, Others: Fail
*/
typedef NXINT32 (*NEXCALVideoEncoderEncode)(NXUINT8 *a_pData1, 
											NXUINT8 *a_pData2, 
											NXUINT8 *a_pData3, 
											NXUINT32 a_uPTS, 
											NXUINT32 *a_puEncodeResult, 
											NXVOID *a_pUserData);

/**
 * @ingroup video_enc
 * 
 * @param[in]a_ppOutData 
 * @param[in]a_pnOutLen 
 * @param[in]a_puPTS 
 * @param[in]a_pUserData  
 * @return 0: Success, Others: Fail 
*/
typedef NXINT32 (*NEXCALVideoEncoderGetOutput)(NXUINT8 **a_ppOutData,
											   NXINT32 *a_pnOutLen,
											   NXUINT32 *a_puPTS,
											   NXVOID *a_pUserData);

/**
 * @ingroup video_enc
 * 
 * @param[in]a_ppOutData 
 * @param[in]a_puOutLen 
 * @param[in]a_pUserData  
 * @return 0: Success, Others: Fail 
*/
typedef NXINT32 (*NEXCALVideoEncoderSkip)(NXUINT8 **a_ppOutData, NXUINT32 *a_puOutLen, NXVOID *a_pUserData);

/**
 * @ingroup audio_enc
 * 
 * @param[in] a_eCodecType 
 * @param[in] a_ppConfig 
 * @param[in] a_pnConfigLen  
 * @param[in] a_uSamplingRate   
 * @param[in] a_uNumOfChannels   
 * @param[in] a_nBitRate   
 * @param[in] a_ppUserData   
 * @return 0: Success, Others: Fail 
*/
typedef NXINT32 (*NEXCALAudioEncoderInit)(NEX_CODEC_TYPE a_eCodecType, 
										  NXUINT8 **a_ppConfig, 
										  NXINT32 *a_pnConfigLen, 
										  NXUINT32 a_uSamplingRate, 
										  NXUINT32 a_uNumOfChannels, 
										  NXINT32 a_nBitRate, 
										  NXVOID **a_ppUserData);

/**
 * @ingroup audio_enc
 * 
 * @param[in] a_pUserData 
 * @return 0: Success, Others: Fail 
*/
typedef NXINT32 (*NEXCALAudioEncoderDeinit)(NXVOID *a_pUserData);

/**
 * @ingroup audio_enc
 * 
 * @param[in] a_pData 
 * @param[in] a_nLen 
 * @param[in] a_ppOutData 
 * @param[in] a_puOutLen 
 * @param[in] a_puEncodeResult 
 * @param[in] a_pUserData 
 * @return 0: Success, Others: Fail 
*/
typedef NXINT32 (*NEXCALAudioEncoderEncode)(NXUINT8 *a_pData,
											NXINT32 a_nLen,
											NXUINT8 **a_ppOutData,
											NXUINT32 *a_puOutLen,
											NXUINT32 *a_puEncodeResult,
											NXVOID *a_pUserData);

/**
 * @ingroup audio_enc
 * 
 * @param[in] a_pUserData 
 * @return 0: Success, Others: Fail 
*/
typedef NXINT32 (*NEXCALAudioEncoderReset)(NXVOID *a_pUserData);

/**
 * @ingroup text_dec
 * This function initializes the video decoder.
 * @param[in] a_eCodecType Indicates the codec object type indication values
 * @param[in] a_pConfig Pointer that refers to the information of video base layer
 * @param[in] a_nConfigLen Length of the information of video base Layer in byte
 * @param[in] a_pInitInfo Infomation that needs when initializing
 * @param[in] a_uMode Video initialization mode
 *					- NEXCAL_TDEC_MODE_NONE
 *					- NEXCAL_TDEC_MODE_MEDIACRYPTO
 * @param[in] a_uUserDataType  User data type (default: unique client id)
 *								If there is not under special conditions, value is NXUSERDATA_PLAYERHANDLE.
 * @param[in,out] a_ppUserData When this function is called by client , this parameter is set with the value that can distinguish client.
 *								So, 2 or more Nexstreaming's Application Engines are run in Nexstreaming's Application Engine address space
 *								at the same time, Codec provider can distinguish the client by this parameter. Codec provider can set the value
 *								before returning. Client that received this value passed the value continuously when called this function continuously.
 * @return 0: Success, Others: Fail 
*/
typedef NXINT32 (*NEXCALTextDecoderInit)(NEX_CODEC_TYPE a_eCodecType, 
										 NXUINT8 *a_pConfig, 
										 NXINT32 a_nConfigLen, 
										 NXVOID *a_pInitInfo,
										 NXUINT32 a_uMode,
										 NXUINT32 a_uUserDataType, 
										 NXVOID **a_ppUserData);

/**
 * @ingroup text_dec
 * This function closes the video decoder.
 * @param[in] a_pUserData The user-data which was got when the client called Init.
 * @return 0: Success, Others: Fail
 * @note When the text decoder will no longer be used, this function must be called.
*/
typedef NXINT32 (*NEXCALTextDecoderDeinit)(NXVOID *a_pUserData);

/**
 * @ingroup text_dec
 * This function decodes video frame.
 * @param[in] a_pSource Pointer that refers to text data to be decoded.
 * @param[in] a_pDest Pointer that refers to the location where decoded frame data will be saved.
 * @param[in] a_pExtraInfo
 * @param[in] a_puDecodeResult Decoding result (0: success, others: fail)
 * @param[in] a_pUserData The user-data which is obtained when the client called Init.
 * @return 0: Success, Others: Fail
*/
typedef NXINT32 (*NEXCALTextDecoderDecode)(NXVOID *a_pSource, NXVOID *a_pDest, NXVOID *a_pExtraInfo, NXUINT32 *a_puDecodeResult, NXVOID *a_pUserData);

/**
 * @ingroup text_dec
 * This function resets text decoder.
 * @param[in] a_pUserData The user-data which is obtained when the client called Init.
 * @return 0: Success, Others: Fail
*/
typedef NXINT32 (*NEXCALTextDecoderReset)(NXVOID *a_pUserData);

/**
 * @ingroup text_dec
 * This function gets information data.
 * @param[in] a_uIndex Request Information index
 * @param[in] a_puResult The requested Info value
 * @param[in] a_pUserData The user-data which is obtained when the client called Init.
 * @return 0: Success, Others: Fail
*/
typedef NXINT32 (*NEXCALTextDecoderGetInfo)(NXUINT32 a_uIndex, NXUINT32 *a_puResult, NXVOID *a_pUserData);

/**
 * @ingroup cal_type
 * Structure that holds video decoder functions used by NexCAL
*/
typedef struct _tag_NEXCALVideoDecoder
{
	NEXCALGetProperty       GetProperty;
	NEXCALSetProperty       SetProperty;
	NEXCALVideoInit         Init;
	NEXCALVideoDeinit       Deinit;
	NEXCALVideoDecode       Decode; 
	NEXCALVideoGetOutput    GetOutput;
	NEXCALVideoReset        Reset;
	NEXCALVideoGetInfo      GetInfo;

} NEXCALVideoDecoder;

/**
 * @ingroup cal_type
 * Structure that holds audio decoder functions used by NexCAL clients
*/
typedef struct _tag_NEXCALAudioDecoder
{
	NEXCALGetProperty       GetProperty;
	NEXCALSetProperty       SetProperty;
	NEXCALAudioInit         Init;
	NEXCALAudioDeinit       Deinit;
	NEXCALAudioDecode       Decode;
	NEXCALAudioReset        Reset;
	NEXCALAudioGetInfo      GetInfo;

} NEXCALAudioDecoder;

/**
 * @ingroup cal_type
 * Structure that holds audio decoder functions used by NexCAL clients
*/
typedef struct _tag_NEXCALImageDecoder
{
	NEXCALGetProperty       GetProperty;
	NEXCALSetProperty       SetProperty;
	NEXCALImageGetHeader    GetHeader;
	NEXCALImageDecode       Decode;

} NEXCALImageDecoder;

/**
 * @ingroup cal_type
 * Structure that holds audio decoder functions used by NexCAL clients
*/
typedef struct _tag_NEXCALVideoEncoder
{
	NEXCALGetProperty           GetProperty;
	NEXCALSetProperty           SetProperty;
	NEXCALVideoEncoderInit      Init;
	NEXCALVideoEncoderDeinit    Deinit;
	NEXCALVideoEncoderEncode    Encode;
	NEXCALVideoEncoderGetOutput GetOutput;
	NEXCALVideoEncoderSkip      Skip;

} NEXCALVideoEncoder;

/**
 * @ingroup cal_type
 * Structure that holds audio decoder functions used by NexCAL clients
*/
typedef struct _tag_NEXCALAudioEncoder
{
	NEXCALGetProperty           GetProperty;
	NEXCALSetProperty           SetProperty;
	NEXCALAudioEncoderInit      Init; 
	NEXCALAudioEncoderDeinit    Deinit;
	NEXCALAudioEncoderEncode    Encode;
	NEXCALAudioEncoderReset     Reset;

} NEXCALAudioEncoder;

/**
 * @ingroup cal_type
 * Structure that holds audio decoder functions used by NexCAL clients
*/
typedef struct _tag_NEXCALTextDecoder
{
	NEXCALGetProperty           GetProperty;
	NEXCALSetProperty           SetProperty;
	NEXCALTextDecoderInit       Init;
	NEXCALTextDecoderDeinit     Deinit;
	NEXCALTextDecoderDecode     Decode;
	NEXCALTextDecoderReset      Reset;

} NEXCALTextDecoder;


#define NEXCAL_INIT_STATIC_PROPERTY(property) ((property) = 0)
#define NEXCAL_SET_STATIC_PROPERTY(property, flag) ((property) = (property) | (flag))
#define NEXCAL_DEL_STATIC_PROPERTY(property, flag) ((property) = (property) & ~(flag))
#define NEXCAL_CHECK_STATIC_PROPERTY(property, flag) (property&(flag) ? TRUE : FALSE)

#define NEXCAL_STATIC_PROPERTY_SW_CODEC 				NTHFROMLSBMASK(0)
#define NEXCAL_STATIC_PROPERTY_HW_CODEC 				NTHFROMLSBMASK(1)
#define NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC			NTHFROMLSBMASK(2)

//GetCodec Flag type list
#define NEXCAL_GETCODEC_FLAG_NONE						0
#define NEXCAL_GETCODEC_FLAG_INSTANT_OUTPUT				NTHFROMLSBMASK(0)	//Get codec that can pull out an output frame instantly when decoding one frame.

#define NEXCAL_QUERY_WHITELIST			1
#define NEXCAL_QUERY_BLACKLIST			2
#define NEXCAL_QUERY_CLUSTERED			3
#define NEXCAL_QUERY_OUTPUTBUFFER_TYPE	4
#define NEXCAL_QUERY_AVC_MAX_PROFILE	5
#define NEXCAL_QUERY_INSTANT_OUTPUT		6	// If return value is 0, the codec cannot support instant output.


#define NEXCAL_QUERY_OUTPUTBUFFER_TYPE_UNKNOWN 0

/**
 * @ingroup cal_type
 * Query interface to get/set information of codec before getCodec
 *
 * @param[in] a_nCommand Query command
 *			- NEXCAL_QUERY_WHITELIST
 *			- NEXCAL_QUERY_BLACKLIST
 *			- NEXCAL_QUERY_CLUSTERED
 *			- NEXCAL_QUERY_OUTPUTBUFFER_TYPE
 *			- NEXCAL_QUERY_AVC_MAX_PROFILE
 *			- NEXCAL_QUERY_INSTANT_OUTPUT
 * @param[in] a_eMediaType Indicates the type of codec
 * @param[in] a_eMode Indicates the mode of codec
 * @param[in] a_eCodecType Indicates the codec object type indication values
 * @param[in] a_pUserData User data
 * @param[in] a_pConfig Codec specific info
 * @param[in] a_nConfigLen length of a_pConfig
 * @param[in] a_pReserved1 reserved parameter
 * @param[in] a_pReserved2 reserved parameter
 * @return Result of query
 *
 * @see NEXCALMediaType
 * @see NEXCALMode 
*/
typedef NXINT32 (*NEXCALQueryHandler)(NXINT32 a_nCommand,
									  NEXCALMediaType a_eMediaType,
									  NEXCALMode a_eMode,
									  NEX_CODEC_TYPE a_eCodecType,
									  NXVOID *a_pUserData,
									  NXUINT8 *a_pConfig,
									  NXINT32 a_nConfigLen,
									  NXVOID *a_pReserved1,
									  NXVOID *a_pReserved2);

/**
 * @ingroup cal_type
 * Interface called when load/unload codec to get codec functions and query handler 
 *
 * @param[in] a_bRelease codec release or not (TRUE/FALSE)
 * @param[in] a_eMediaType Indicates the type of codec 
 * @param[in] a_eMode Indicates the mode of codec
 * @param[in] a_eCodecType Indicates the codec object type indication values
 * @param[in] a_pUserData User data
 * @param[in] a_pClientID Client unique ID
 * @param[in] a_pFuncs pointer of codec functions
 * @param[in] a_nFuncSize size of a_pFuncs
 * @param[in] a_ppQueryHandler pointer of query handler
 * @param[in] a_ppUserDataForQueryHandler User data for query handler
 * @return 0: Success, Others: Fail
 *
 * @see NEXCALMediaType
 * @see NEXCALMode
 * @see NEXCALQueryHandler
*/
typedef NXINT32 (*NEXCALOnLoad)(NXBOOL a_bRelease,
								NEXCALMediaType a_eMediaType,
								NEXCALMode a_eMode,
								NEX_CODEC_TYPE a_eCodecType,
								NXUINT32 a_uStaticProperties,
								NXVOID *a_pUserData,
								NXVOID *a_pClientID,
								NXVOID *a_pFuncs,
								NXINT32 a_nFuncSize,
								NEXCALQueryHandler *a_ppQueryHandler,
								NXVOID **a_ppUserDataForQueryHandler);


#ifdef __cplusplus
extern "C" {
#endif

/**
* @ingroup cal_basic
* @brief return version info with string
* @return Pointer of string which consists of 3 digits with one word and it means major version number, 
*	minor version number, patch version number, and branch version name in sequence. (Ex. "1.0.0.OFFICIAL")
*/
	NEXCAL_API const NXCHAR* nexCAL_GetVersionString();

/**
* @ingroup cal_basic
* @brief return major version with NXINT32
* @return Major version number
*/
	NEXCAL_API NXINT32 nexCAL_GetMajorVersion();

/**
* @ingroup cal_basic
* @brief return minor version with NXINT32
* @return Minor version number
*/
	NEXCAL_API NXINT32 nexCAL_GetMinorVersion();

/**
* @ingroup cal_basic
* @brief return patch version with NXINT32
* @return Patch version number
*/
	NEXCAL_API NXINT32 nexCAL_GetPatchVersion();

/**
* @ingroup cal_basic
* @brief return branch version with NXINT32
* @return Branch version string
*/
	NEXCAL_API const NXCHAR* nexCAL_GetBranchVersion();


/**
* @ingroup cal_basic
* @brief return version info with string
* @return Version information
*/
	NEXCAL_API const NXCHAR* nexCAL_GetVersionInfo();

/**
* @ingroup cal_basic
* @brief check library version whether the same or not
* @param[in] a_nMajor major version
* @param[in] a_nMinor minor version
* @param[in] a_nPatch patch version
* @param[in] a_strBranch branch version
* @return TRUE: the same version, FALSE: another version
*/
	NEXCAL_API NXBOOL nexCAL_CheckSameVersion(const NXINT32 a_nMajor, const NXINT32 a_nMinor, const NXINT32 a_nPatch, const NXCHAR *a_strBranch);

/**
* @ingroup cal_basic
* @brief check compatibility number
* @param[in] a_nCompatibilityNum compatibility number
* @return TRUE: compatible, FALSE: not compatible
*/
	NEXCAL_API NXBOOL nexCAL_CheckCompatibleVersion(const NXINT32 a_nCompatibilityNum);

/**
* @ingroup cal_functions
* @brief This function gets the handle of NexCAL
* @param[in] a_nFrom (deprecated)
*				- 0: NexCAL provider 
*				- 1: NexCAL client
* @param[in] a_nCompatibilityNum NexCAL's Compatibility Number
*								NEXCAL_COMPATIBILITY_NUM in header file of library
* @return NULL: fail, Others: NexCAL handle
* @note Before calling the function which needs NEXCALHandle value, the handle of NexCAL must be obtained using this API.
*		Only a single value is existed for the instance of NexCAL, and it is managed as reference count.
*/
	NEXCAL_API NEXCALHandle nexCAL_GetHandle(NXINT32 a_nFrom, const NXINT32 a_nCompatibilityNum);

/**
* @ingroup cal_functions
* @brief This function releases the handle of NexCAL
* @param[in] a_hCAL The value of NEXCALHandle filled by nexCAL_GetHandle( )
* @return None
* @note As it will be mention nexCAL_GetHandle, the reference count of handle is decreased by one,
* 		and when the reference count becomes 0 then the instance of NexCAL is removed.
* @see nexCAL_GetHandle
*/
	NEXCAL_API NXVOID nexCAL_ReleaseHandle(NEXCALHandle a_hCAL);

/**
* @ingroup cal_functions
* @brief This function gets codec policy
* @param[in] a_hCAL The value of NEXCALHandle filled by nexCAL_GetHandle( )
* @param[in] a_pePolicy Policy of codec
* @return 0: fail, 1: success
* @see NEXCALHandle
* @see NEXCALPolicy
*/
	NEXCAL_API NXINT32 nexCAL_GetPolicy(NEXCALHandle a_hCAL, NEXCALPolicy *a_pePolicy);

/**
* @ingroup cal_functions
* @brief This function sets codec policy
* @param[in] a_hCAL The value of NEXCALHandle filled by nexCAL_GetHandle( )
* @param[in] a_ePolicy Policy of codec
* @return 0: fail, 1: success
* @see NEXCALHandle
* @see NEXCALPolicy
*/
	NEXCAL_API NXINT32 nexCAL_SetPolicy(NEXCALHandle a_hCAL, const NEXCALPolicy a_ePolicy);

/**
* @ingroup cal_functions
* @brief This function registers codec interface to NexCAL
* @param[in] a_hCAL The value of NEXCALHandle filled by nexCAL_GetHandle()
* @param[in] a_eMediaType Indicates the type of codec
* @param[in] a_eMode Indicates the mode of codec
* @param[in] a_eCodecType Indicates the codec object type indication values
* @param[in] a_pfnOnLoad Registers function called when load/unload codec to get codec functions and query handler
* @param[in] a_pUserData User data
* @param[in] a_uStaticProperties Sets codec properties before getCodec
*						- NEXCAL_STATIC_PROPERTY_HW
*						- NEXCAL_STATIC_PROPERTY_SW
*
*						- NEXCAL_STATIC_PROPERTY_PATENT_OFF
*						- NEXCAL_STATIC_PROPERTY_PATENT_ON
*
*						- NEXCAL_STATIC_PROPERTY_PERFORMACE_HIGH
*						- NEXCAL_STATIC_PROPERTY_PERFORMACE_MEDIUM
*						- NEXCAL_STATIC_PROPERTY_PERFORMACE_LOW
*
*						- NEXCAL_STATIC_PROPERTY_POWERCONSUMPTION_LOW
*						- NEXCAL_STATIC_PROPERTY_POWERCONSUMPTION_MEDIUM
*						- NEXCAL_STATIC_PROPERTY_POWERCONSUMPTION_HIGH
* @return 0: fail, 1: success
* @see nexCAL_GetHandle
* @see NEXCALHandle
* @see NEXCALMediaType
* @see NEXCALMode
* @see NEXCALOnLoad
*/
	NEXCAL_API NXINT32 nexCAL_RegisterCodec(NEXCALHandle a_hCAL, 
											NEXCALMediaType a_eMediaType, 
											NEXCALMode a_eMode, 
											NEX_CODEC_TYPE a_eCodecType, 
											NEXCALOnLoad a_pfnOnLoad, 
											NXVOID *a_pUserData, 
											NXUINT32 a_uStaticProperties);

/**
* @ingroup cal_functions
* @brief This function unregisters codec interface to NexCAL
* @param[in] a_hCAL The value of NEXCALHandle filled by nexCAL_GetHandle()
* @param[in] a_eMediaType Indicates the type of codec
* @param[in] a_eMode Indicates the mode of codec
* @param[in] a_eCodecType Indicates the codec object type indication values
* @param[in] a_pfnOnLoad Registers function called when load/unload codec to get codec functions and query handler
* @param[in] a_pUserData User data
* @return 0: fail, 1: success
* @see nexCAL_GetHandle
* @see NEXCALHandle
* @see NEXCALMediaType
* @see NEXCALMode
* @see NEXCALOnLoad
*/
	NEXCAL_API NXINT32 nexCAL_UnregisterCodec(NEXCALHandle a_hCAL, 
											  NEXCALMediaType a_eMediaType, 
											  NEXCALMode a_eMode, 
											  NEX_CODEC_TYPE a_eCodecType, 
											  NEXCALOnLoad a_pfnOnLoad, 
											  NXVOID *a_pUserData);

/**
* @ingroup cal_functions
* @brief This function gets codec handle
* @param[in] a_hCAL The value of NEXCALHandle filled by nexCAL_GetHandle()
* @param[in] a_eMediaType Indicates the type of codec
* @param[in] a_eMode Indicates the mode of codec
* @param[in] a_eCodecType Indicates the codec object type indication values
* @param[in] a_pConfig Codec specific information
* @param[in] a_nConfigLen length of a_pConfig
* @param[in] a_uFlag Flag to set codec
*				- NEXCAL_GETCODEC_FLAG_NONE
*				- NEXCAL_GETCODEC_FLAG_INSTANT_OUTPUT
* @param[in] a_eMethod method to select codec
* @param[in] a_hAfterThis When the previous codec handle failed in initialization, input previous codec
*							handle then the next codec handle will be found in registered codec handle list.
*							If this parameter is NULL, the first codec handle will be found in registered codec handle list.
* @param[in] a_pClientID Unique client ID

* @return NULL: fail, Others: Codec handle values
* @see nexCAL_GetHandle
* @see NEXCALHandle
* @see NEXCALMediaType
* @see NEXCALCodecHandle
*/
	NEXCAL_API NEXCALCodecHandle nexCAL_GetCodec(NEXCALHandle a_hCAL,
												 NEXCALMediaType a_eMediaType,
												 NEXCALMode a_eMode,
												 NEX_CODEC_TYPE a_eCodecType,
												 NXUINT8 *a_pConfig,
												 NXINT32 a_nConfigLen,
												 NXUINT32 a_uFlag,
												 NEXCALMethod a_eMethod,
												 NEXCALCodecHandle a_hAfterThis,
												 NXVOID *a_pClientID);

/**
* @ingroup cal_functions
* @brief This function release codec
* @param[in] a_hCodec The value of NEXCALCodecHandle filled by nexCAL_GetCodec()
* @param[in] a_pClientID Unique client ID
* @return None
* @see NEXCALCodecHandle
*/
	NEXCAL_API NXVOID nexCAL_ReleaseCodec(NEXCALCodecHandle a_hCodec, NXVOID *a_pClientID);


/**
* @ingroup cal_functions
* @deprecated This function returns the last error value of video for each clients
* @param[in] a_hCAL The value of NEXCALHandle filled by nexCAL_GetHandle()
* @param[in] a_eMediaType Indicates the type of codec
* @return NEXCAL_ERROR_NONE: Success, Others: Fail
* @see NEXCALHandle
* @see NEXCALMediaType
*/
	NEXCAL_API NXUINT32 nexCAL_GetCodecLastErrorInfo(NEXCALHandle a_hCAL, NEXCALMediaType a_eMediaType);

/**
* @ingroup cal_functions
* @deprecated This function dumps frames
* @param[in] a_hCAL The value of NEXCALHandle filled by nexCAL_GetHandle()
* @param[in] a_strFileName string of file path
* @param[in] a_eMediaType Indicates the type of codec
* @param[in] a_pReserved reserved
* @return None
* @see NEXCALHandle
* @see NEXCALMediaType
*/
	NEXCAL_API NXVOID nexCAL_DumpFrames(NEXCALHandle a_hCAL, const NXCHAR *a_strFileName, NEXCALMediaType a_eMediaType, NXVOID *a_pReserved);

/**
* @ingroup cal_functions
* This function prints all of the registered codecs
* @param[in] a_hCAL The value of NEXCALHandle filled by nexCAL_GetHandle()
* @return None
* @see NEXCALHandle
*/
	NEXCAL_API NXVOID nexCAL_PrintAllCALEntries(NEXCALHandle a_hCAL);


//-----------------------------------------------------------------------------
//	The client of nexCAL should use the following Wrapper functions
//-----------------------------------------------------------------------------

	NEXCAL_API NXINT32 nexCAL_SetClientIdentifier(NEXCALCodecHandle a_hCodec, NXVOID *a_pClientIdentifier);


//-----------------------------------------------------------------------------
//		Video Decoder Wrapper
//-----------------------------------------------------------------------------

	NEXCAL_API NXINT32 nexCAL_VideoDecoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue);

	NEXCAL_API NXINT32 nexCAL_VideoDecoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue);

	NEXCAL_API NXINT32 nexCAL_VideoDecoderInit(NEXCALCodecHandle a_hCodec, 
											   NEX_CODEC_TYPE a_eCodecType, 
											   NXUINT8 *a_pConfig, 
											   NXINT32 a_nConfigLen, 
											   NXUINT8 *a_pFrame, 
											   NXINT32 a_nFrameLen, 
											   NXVOID *a_pInitInfo,
											   NXVOID *a_pExtraInfo,
											   NXINT32 a_nNALHeaderSize, 
											   NXINT32 *a_pnWidth, 
											   NXINT32 *a_pnHeight, 
											   NXINT32 *a_pnPitch, 
											   NXUINT32 a_uMode, 
											   NXUINT32 a_uUserDataType, 
											   NXVOID *a_pClientIdentifier);

	NEXCAL_API NXINT32 nexCAL_VideoDecoderDeinit(NEXCALCodecHandle a_hCodec);

	NEXCAL_API NXINT32 nexCAL_VideoDecoderDecode(NEXCALCodecHandle a_hCodec, 
												 NXUINT8 *a_pFrame, 
												 NXINT32 a_nFrameLen, 
												 NXVOID *a_pExtraInfo,
												 NXUINT32 a_uDTS, 
												 NXUINT32 a_uPTS, 
												 NXINT32 a_nFlag, 
												 NXUINT32 *a_puDecodeResult);

	NEXCAL_API NXINT32 nexCAL_VideoDecoderGetOutput(NEXCALCodecHandle a_hCodec, 
													NXUINT8 **a_ppBits1, 
													NXUINT8 **a_ppBits2, 
													NXUINT8 **a_ppBits3, 
													NXUINT32 *a_puPTS);

	NEXCAL_API NXINT32 nexCAL_VideoDecoderReset(NEXCALCodecHandle a_hCodec);

	NEXCAL_API NXINT32 nexCAL_VideoDecoderGetInfo(NEXCALCodecHandle a_hCodec, NXUINT32 a_uIndex, NXUINT32 *a_puResult);

//-----------------------------------------------------------------------------
//		Audio Decoder Wrapper
//-----------------------------------------------------------------------------

	NEXCAL_API NXINT32 nexCAL_AudioDecoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue);

	NEXCAL_API NXINT32 nexCAL_AudioDecoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue);


	NEXCAL_API NXINT32 nexCAL_AudioDecoderInit(NEXCALCodecHandle a_hCodec, 
											   NEX_CODEC_TYPE a_eCodecType, 
											   NXUINT8 *a_pConfig, 
											   NXINT32 a_nConfigLen, 
											   NXUINT8 *a_pFrame, 
											   NXINT32 a_nFrameLen, 
											   NXVOID *a_pInitInfo,
											   NXVOID *a_pExtraInfo,
											   NXUINT32 *a_puSamplingRate, 
											   NXUINT32 *a_puNumOfChannels, 
											   NXUINT32 *a_puBitsPerSample, 
											   NXUINT32 *a_puNumOfSamplesPerChannel, 
											   NXUINT32 a_uMode,
											   NXUINT32 a_uUserDataType, 
											   NXVOID *a_pClientIdentifier);

	NEXCAL_API NXINT32 nexCAL_AudioDecoderDeinit(NEXCALCodecHandle a_hCodec);

	NEXCAL_API NXINT32 nexCAL_AudioDecoderDecode(NEXCALCodecHandle a_hCodec, 
												 NXUINT8 *a_pFrame, 
												 NXINT32 a_nFrameLen, 
												 NXVOID *a_pExtraInfo,
												 NXVOID *a_pDest, 
												 NXINT32 *a_puWrittenPCMSize, 
												 NXUINT32 a_uDTS, 
												 NXUINT32 *a_puOutputTime, 
												 NXINT32 a_nFlag, 
												 NXUINT32 *a_puDecodeResult);

	NEXCAL_API NXINT32 nexCAL_AudioDecoderReset(NEXCALCodecHandle a_hCodec);

	NEXCAL_API NXINT32 nexCAL_AudioDecoderGetInfo(NEXCALCodecHandle a_hCodec, NXUINT32 a_uIndex, NXUINT32 *a_puResult);

/*-----------------------------------------------------------------------------
		Text Decoder Wrapper
-----------------------------------------------------------------------------*/

	NEXCAL_API NXINT32 nexCAL_TextDecoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue);

	NEXCAL_API NXINT32 nexCAL_TextDecoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue);

	NEXCAL_API NXINT32 nexCAL_TextDecoderInit(NEXCALCodecHandle a_hCodec,
											  NEX_CODEC_TYPE a_eCodecType,
											  NXUINT8 *a_pConfig,
											  NXINT32 a_nConfigLen,
											  NXVOID *a_pInitInfo,
											  NXUINT32 a_uMode,
											  NXUINT32 a_uUserDataType,
											  NXVOID *a_pClientIdentifier);

	NEXCAL_API NXINT32 nexCAL_TextDecoderDeinit(NEXCALCodecHandle a_hCodec);

	NEXCAL_API NXINT32 nexCAL_TextDecoderDecode(NEXCALCodecHandle a_hCodec,
												NXVOID *a_pSource,
												NXVOID *a_pDest,
												NXVOID *a_pExtraInfo,
												NXUINT32 *a_puDecodeResult);

	NEXCAL_API NXINT32 nexCAL_TextDecoderReset(NEXCALCodecHandle a_hCodec);

	NEXCAL_API NXINT32 nexCAL_TextDecoderGetInfo(NEXCALCodecHandle a_hCodec, NXUINT32 a_uIndex, NXUINT32 *a_puResult);

//-----------------------------------------------------------------------------
//		Image Decoder Wrapper
//-----------------------------------------------------------------------------

	NEXCAL_API NXINT32 nexCAL_ImageDecoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue);

	NEXCAL_API NXINT32 nexCAL_IamgeDecoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue);

	NEXCAL_API NXINT32 nexCAL_ImageDecoderGetHeader(NEXCALCodecHandle a_hCodec, 
													NEX_CODEC_TYPE a_eCodecType, 
													NXCHAR *a_strpFileName, 
													NXUINT8 *a_pSrc, 
													NXUINT32 a_uSrcLen, 
													NXINT32 *a_pnWidth, 
													NXINT32 *a_pnHeight, 
													NXINT32 *a_pnPitch, 
													NXUINT32 a_uUserDataType, 
													NXVOID *a_pClientIdentifier);

	NEXCAL_API NXINT32 nexCAL_ImageDecoderDecode(NEXCALCodecHandle a_hCodec, 
												 NXCHAR *a_strFileName, 
												 NXUINT8 *a_pSrc, 
												 NXUINT32 a_uSrcLen, 
												 NXINT32 *a_pnWidth, 
												 NXINT32 *a_pnHeight, 
												 NXINT32 *a_pnPitch, 
												 NXUINT8 *a_pBits1, 
												 NXUINT8 *a_pBits2, 
												 NXUINT8 *a_pBits3, 
												 NXUINT32 *a_puDecodeResult);


//-----------------------------------------------------------------------------
//		Video Encoder Wrapper
//-----------------------------------------------------------------------------

	NEXCAL_API NXINT32 nexCAL_VideoEncoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue);

	NEXCAL_API NXINT32 nexCAL_VideoEncoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue);

	NEXCAL_API NXINT32 nexCAL_VideoEncoderInit(NEXCALCodecHandle a_hCodec, 
											   NEX_CODEC_TYPE a_eCodecType, 
											   NXUINT8 **a_ppConfig, 
											   NXINT32 *a_pnConfigLen, 
											   NXINT32 a_nQuality,
											   NXINT32 a_nWidth,
											   NXINT32 a_nHeight,
											   NXINT32 a_nPitch,
											   NXINT32 a_nFPS,
											   NXBOOL a_bCBR,
											   NXINT32 a_nBitRate,
											   NXVOID *a_pClientIdentifier);

	NEXCAL_API NXINT32 nexCAL_VideoEncoderDeinit(NEXCALCodecHandle a_hCodec);

	NEXCAL_API NXINT32 nexCAL_VideoEncoderEncode(NEXCALCodecHandle a_hCodec,
												 NXUINT8 *a_pData1,
												 NXUINT8 *a_pData2,
												 NXUINT8 *a_pData3,
												 NXUINT32 a_uPTS,
												 NXUINT32 *a_puEncodeResult);

	NEXCAL_API NXINT32 nexCAL_VideoEncoderGetOutput(NEXCALCodecHandle a_hCodec,
													NXUINT8 **a_ppOutData,
													NXINT32 *a_pnOutLen,
													NXUINT32 *a_puPTS);

	NEXCAL_API NXINT32 nexCAL_VideoEncoderSkip(NEXCALCodecHandle a_hCodec, NXUINT8 **a_ppOutData, NXUINT32 *a_puOutLen);

//-----------------------------------------------------------------------------
//		Audio Encoder Wrapper
//-----------------------------------------------------------------------------

	NEXCAL_API NXINT32 nexCAL_AudioEncoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue);

	NEXCAL_API NXINT32 nexCAL_AudioEncoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue);

	NEXCAL_API  NXINT32 nexCAL_AudioEncoderInit(NEXCALCodecHandle a_hCodec, 
												NEX_CODEC_TYPE a_eCodecType, 
												NXUINT8 **a_ppConfig, 
												NXINT32 *a_pnConfigLen, 
												NXUINT32 a_uSamplingRate, 
												NXUINT32 a_uNumOfChannels, 
												NXINT32 a_nBitrate, 
												NXVOID *a_pClientIdentifier);

	NEXCAL_API NXINT32 nexCAL_AudioEncoderDeinit(NEXCALCodecHandle a_hCodec);

	NEXCAL_API  NXINT32 nexCAL_AudioEncoderEncode(NEXCALCodecHandle a_hCodec,
												  NXUINT8 *a_pData,
												  NXINT32 a_nLen,
												  NXUINT8 **a_ppOutData,
												  NXUINT32 *a_puOutLen,
												  NXUINT32 *a_puEncodeResult);

	NEXCAL_API NXINT32 nexCAL_AudioEncoderReset(NEXCALCodecHandle a_hCodec);

#ifdef __cplusplus
}
#endif

#define NEXCAL_PROPERTY_AVC_MAX_LEVEL 52
#define NEXCAL_PROPERTY_HEVC_MAX_LEVEL 62

//-----------------------------------------------------------------------------
//		GetProperty()'s properties and values
//-----------------------------------------------------------------------------

/**
 * @defgroup codec_properties Propertise of Codec
 * @{
*/

#define NEXCAL_PROPERTY_ANSWERIS_DONTCARE						0	///< Don't care
#define NEXCAL_PROPERTY_ANSWERIS_NO								1	///< General renderer can be used
#define NEXCAL_PROPERTY_ANSWERIS_YES							2	///< Specifically designated renderer should be used

/**
 * Property to codec implementation method which are generally used for debugging.
*/
#define NEXCAL_PROPERTY_CODEC_IMPLEMENT						0x00000001
#define NEXCAL_PROPERTY_CODEC_IMPLEMENT_NONE					0
#define NEXCAL_PROPERTY_CODEC_IMPLEMENT_SW						1	///< Software codec
#define NEXCAL_PROPERTY_CODEC_IMPLEMENT_PARTIAL_HW				2	///< Partly implemented in HW
#define NEXCAL_PROPERTY_CODEC_IMPLEMENT_HW						3	///< Based on platform that NexCAL is executing and implemented in HW

/**
 * Property to decide whether to use specific renderer
*/
#define NEXCAL_PROPERTY_NEED_EXCLUSIVE_RENDERER				0x00000002
//NEXCAL_PROPERTY_ANSWERIS_DONTCARE
//NEXCAL_PROPERTY_ANSWERIS_NO
//NEXCAL_PROPERTY_ANSWERIS_YES

#define NEXCAL_PROPERTY_COMPONENT_NAME						0x00000003

// Video ----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * In case of Video decoder, it means property for NEXCALVideoDecoder::Init function's puWidth, puHeight and puPitch,
 * in case of Audio decoder, it means that whether can fill by right value to output parameter that correspond to 
 * NEXCALAudioDecoder::Init funnction's puSamplingRate, puNumOfChannels, puBitsPerSample, and puNumOfSamplesPerChannel.
*/
#define NEXCAL_PROPERTY_SET_RIGHT_OUTPUT_INFORMATION		0x00000100
//NEXCAL_PROPERTY_ANSWERIS_DONTCARE
//NEXCAL_PROPERTY_ANSWERIS_NO
//NEXCAL_PROPERTY_ANSWERIS_YES

/**
 * Property to decide whether to reinit codec for seamless play when track changes
*/
#define NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT			0x00000101
#define NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT_NO				0	///< Cannot set the value
#define NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT_YES			1	///< Can set the value

/**
 * Property for NEXCALVideoDecoder::GetOutput function's ppY, ppU and ppV values
*/
#define NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE			0x00000102

/**
 * Property for NEXCALVideoEncoder::GetOutput function's values
 * @note The method of presentation in memory which holds actual decoded data must be able to access and to fix by returned values of piWidth, piHeight and piPitch.\n
 *		- To client\n
 *			If it is NOT_MEMORY value, must not perform processing such as post-processing using this value. ppY, ppU, ppV values must be transferred to renderer to process as it is.\n
 *		- To provider\n
 *			If it is not cases described in above, should set as VIDEO_BUFFER_NOT_MEMORY value. Values transferred to renderer afterwards are ppY, ppU and ppV values as it is, 
 *			hence specific renderer can be provided together.
*/
#define NEXCAL_PROPERTY_VIDEO_INPUT_BUFFER_TYPE				0x00000103

// Video Buffer Type for NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, NEXCAL_PROPERTY_VIDEO_INPUT_BUFFER_TYPE
#define NEXCAL_PROPERTY_VIDEO_BUFFER_UNKNOWN					0

/**
 * ppBits1, ppBits2 and ppBit3 values are not set as pointer toward memory. Client must not consider these values as memory
*/
#define NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY					NTHFROMLSBMASK(0)
/**
 * ppBits1, ppBits2 and ppBits3 values of YUV420 format indicates Y,U,V memory
*/
#define NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420			NTHFROMLSBMASK(1)
/**
 * In ppY of YUV 420 format, Y, U, V values are merged
*/
#define NEXCAL_PROPERTY_VIDEO_BUFFER_MERGED_YUV420				NTHFROMLSBMASK(2)
/**
 * ppBits1 holds memory pointer in RGB888 format
*/
#define NEXCAL_PROPERTY_VIDEO_BUFFER_RGB888						NTHFROMLSBMASK(3)
/**
 * ppBits1 holds memory pointer in RGB565 format
*/
#define NEXCAL_PROPERTY_VIDEO_BUFFER_RGB565						NTHFROMLSBMASK(4)
/**
 * y planar, uv-interleaved planar
*/
#define NEXCAL_PROPERTY_VIDEO_BUFFER_NV12						NTHFROMLSBMASK(5)
#define NEXCAL_PROPERTY_VIDEO_BUFFER_TILED						NTHFROMLSBMASK(6)
/**
 * In ppY of YUV 420 format, Y, V, U values are merged
*/
#define NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YVU420			NTHFROMLSBMASK(7)
/**
 * y planar, vu-interleaved planar
*/
#define NEXCAL_PROPERTY_VIDEO_BUFFER_NV21						NTHFROMLSBMASK(8)

/**
 * Property to decide whether to use P-frame when codec initializes
*/
#define NEXCAL_PROPERTY_VIDEO_INIT_WITH_PFRAME				0x00000104
#define NEXCAL_PROPERTY_VIDEO_INIT_WITH_PFRAME_NOT_OK			0	///< initilize with key frame 
#define NEXCAL_PROPERTY_VIDEO_INIT_WITH_PFRAME_OK				1	///< initilize with P-frame

#define NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT			0x00000105
#define NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO			0
#define NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES			1

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H264
 * Property to get output buffer count 
*/
#define NEXCAL_PROPERTY_AVAILABLE_OUTPUT_BUFFER_COUNT		0x00000106
#define NEXCAL_PROPERTY_AVAILABLE_OUTPUT_BUFFER_COUNT_NO		0

//Nx_robin__140728 nexCAL_VideoDecoderInit()에서 pConfigEnhance에 전달되는 첫 프레임을 Decode할지의 여부. SDK1(YES), AndroidCase2(NO)
/**
 * Property to decide whether to decode the first frame when video initializes
*/
#define NEXCAL_PROPERTY_DECODE_IN_INIT						0x00000107
#define NEXCAL_PROPERTY_DECODE_IN_INIT_YES						0		///< decode during initialization
#define NEXCAL_PROPERTY_DECODE_IN_INIT_NO						1		///< not decode during initialization

// Audio ----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Property for the ouput buffer type. The format of decoded result which would hold a_pDest of  NEXCALAudioDecoder::Decoder function
 * @note If it is NOT_MEMORY value, the client must not perform processing such as post-processing using this value.
 */
#define NEXCAL_PROPERTY_AUDIO_OUTPUT_BUFFER_TYPE			0x00000200

/**
 * Property for the input buffer type. The format of encoded result which would hold a_ppOutData of NEXCALAudioencoder::encoder function
*/
#define NEXCAL_PROPERTY_AUDIO_INPUT_BUFFER_TYPE 			0x00000201

// Audio buffer type for NEXCAL_PROPERTY_AUDIO_OUTPUT_BUFFER_TYPE, NEXCAL_PROPERTY_AUDIO_INPUT_BUFFER_TYPE
#define NEXCAL_PROPERTY_AUDIO_BUFFER_UNKNOWN					0
#define NEXCAL_PROPERTY_AUDIO_BUFFER_BYPASS						NTHFROMLSBMASK(0)	///< For pDest value, instead of decoded raw data, custom value for interface with renderer will be set.
#define NEXCAL_PROPERTY_AUDIO_BUFFER_PCM						NTHFROMLSBMASK(1)	///< pDest holds memory pointer in PCM format. This value can be interpreted by the values obtained during NEXCALAudioDecoder::Init.

/**
 * Property to get whether to get sample per channel
*/
#define NEXCAL_PROPERTY_HAVE_SAMPLE_PER_CHANNEL				0x00000202
#define NEXCAL_PROPERTY_HAVE_SAMPLE_PER_CHANNEL_YES				1	///< have sample per channel
#define NEXCAL_PROPERTY_HAVE_SAMPLE_PER_CHANNEL_NO				2	///< not have sample per channel

/**
 * Property to get whether to support audio multi-channel
*/
#define NEXCAL_PROPERTY_SUPPORT_ES_AUDIO					0x00000203
#define NEXCAL_PROPERTY_SUPPORT_ES_AUDIO_YES					1	///< support audio multi-channel
#define NEXCAL_PROPERTY_SUPPORT_ES_AUDIO_NO						0	///< not support audio multi-channel

// Text ---------------------------------------------------------------------------------------------------------------------------------------------------------- 

/**
 * Property for the ouput buffer type. The format of decoded result which would hold a_pDest of NEXCALTextDecoder:: 
 * @note If it is NOT_MEMORY value, the client must not perform processing such as post-processing using this value.
*/
#define NEXCAL_PROPERTY_TEXT_OUTPUT_BUFFER_TYPE				0x00000300

/**
 * Property for the input buffer type
*/
#define NEXCAL_PROPERTY_TEXT_INPUT_BUFFER_TYPE				0x00000301

// Text buffer type for NEXCAL_PROPERTY_TEXT_OUTPUT_BUFFER_TYPE, NEXCAL_PROPERTY_TEXT_INPUT_BUFFER_TYPE
#define NEXCAL_PROPERTY_TEXT_BUFFER_UNKNOWN						0
#define NEXCAL_PROPERTY_TEXT_BUFFER_NOT_MEMORY					NTHFROMLSBMASK(0)


// For Specified Codec ----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H264, eNEX_CODEC_V_HEVC\n
 * Property for bytestream format on memory indicated by pData parameter, which is used by entering NEXCALVideoDecoder::Decode function.
*/
#define NEXCAL_PROPERTY_BYTESTREAM_FORMAT					0x00000400

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H264, eNEX_CODEC_V_HEVC\n
 * Property for InitPS format on memory indicated by pData parameter, which is used by entering NEXCALVideoDecoder::Decode function.
*/
#define NEXCAL_PROPERTY_INITPS_FORMAT						0x00000401

#define NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANY					0	///< Below two values can be processed
#define NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW					1	///< Format those have 1~4 bytes size (NAL units) information is attached in the front.
#define NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB					2	///< Refer to Annex B of ISO/IEC 14496-10 
#define NEXCAL_PROPERTY_BYTESTREAMFORMAT_ENCRYPTED				3	///< Encrypted stream

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H264\n
 * Property to decide whether to support config-stream in the beginning of frames
*/
#define NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM				0x00000410
#define NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_NOT_OK			0	///< If not support, eliminate config-stream
#define NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_OK				1	///< support config-stream

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H264\n
 * Property to get supportable profiles
*/
#define NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE					0x00000411
#define NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL			0x00000412	///< Level in baseline 
#define NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL				0x00000413	///< Level in main 
#define NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL			0x00000414	///< Level in extended
#define NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL				0x00000415	///< Level in high
#define NEXCAL_PROPERTY_AVC_HIGH10_SUPPORT_LEVEL			0x00000416	///< Level in high10
#define NEXCAL_PROPERTY_AVC_HIGH422_SUPPORT_LEVEL			0x00000417	///< Level in high422
#define NEXCAL_PROPERTY_AVC_HIGH444_SUPPORT_LEVEL			0x00000418	///< Level in high444 

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H264\n
 * Property to decide whether to support configuration record
*/
#define NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER 	  		0x00000419
#define NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER_NOT_OK		0	///< If not support, eliminate configuratino record
#define NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER_OK			1	///< support configuration record

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H264\n
 * Property to decide whether to eliminate B-frame that nal_ref_idc is 0
*/
#define NEXCAL_PROPERTY_AVC_SKIP_NOTREFIDC_BFRAME			0x00000420
#define NEXCAL_PROPERTY_AVC_SKIP_NOTREFIDC_BFRAME_NOT_OK		0	///< skip B-frame that is not ref IDC
#define NEXCAL_PROPERTY_AVC_SKIP_NOTREFIDC_BFRAME_OK			1	///< decode all B-frame

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_HEVC\n
 * Property to decide whether to support config-stream in the beginning of frames
*/
#define NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM			0x00000500
#define NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_NOT_OK 		0	///< If not support, eliminate config-stream
#define NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_OK	  			1	///< support config-stream

/**
 * Target Codec Object Type Indication value: NEXOTI_HEVC\n
 * Property to get supportable profiles
*/
#define NEXCAL_PROPERTY_HEVC_SUPPORT_PROFILE 				0x00000501
#define NEXCAL_PROPERTY_HEVC_MAIN_SUPPORT_LEVEL				0x00000502	///< Leve in main profile of HEVC 
#define NEXCAL_PROPERTY_HEVC_MAIN10_SUPPORT_LEVEL			0x00000503	///< Leve in main10 profile of HEVC
#define NEXCAL_PROPERTY_HEVC_MAINSP_SUPPORT_LEVEL			0x00000504	///< Leve in mainSP profile of HEVC

/**
 * Target Codec Object Type Indication value: NEXOTI_HEVC\n
 * Property to decide whether to support configuration stream
*/
#define NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER			0x00000505
#define NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER_NOT_OK 		0	///< not support HEVC configuration record
#define NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER_OK   		1	///< support HEVC configuration record

/**
 * Target Codec Object Type Indication value: NEXOTI_HEVC\n
 * Property to decide whether to eliminate B-frame that nal_ref_idc is 0
*/
#define NEXCAL_PROPERTY_HEVC_SKIP_NOTREFIDC_BFRAME			0x00000506
#define NEXCAL_PROPERTY_HEVC_SKIP_NOTREFIDC_BFRAME_NOT_OK 		0	///< skip B-frame that is not ref IDC
#define NEXCAL_PROPERTY_HEVC_SKIP_NOTREFIDC_BFRAME_OK 			1	///< decode all B-frame

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_WVC1\n
 * Property to get supportable profiles 
*/
#define NEXCAL_PROPERTY_VC1_SUPPORTABLE_PROFILE				0x00000600
#define NEXCAL_PROPERTY_VC1_SIMPLE_PROFILE      				0x01 << 0	///< VC1 simple profile
#define NEXCAL_PROPERTY_VC1_MAIN_PROFILE        				0x01 << 1	///< VC1 main profile
#define NEXCAL_PROPERTY_VC1_COMPLEX_PROFILE     				0x01 << 2	///< VC1 complex profile
#define NEXCAL_PROPERTY_VC1_ADVANCED_PROFILE    				0x01 << 3	///< VC1 advanced profile

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_WVC1\n
 * Property to decide whether to support YUV420
*/
#define NEXCAL_PROPERTY_VC1_SUPPORT_NOYUV420				0x00000601
#define NEXCAL_PROPERTY_VC1_SUPPORT_NOYUV420_YES				0	///< support even if YUV420 is not
#define NEXCAL_PROPERTY_VC1_SUPPORT_NOYUV420_NO					1	///< not support if YUV420 is not

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_WVC1\n
 * Property to decide whether to skip BFrame
*/
#define NEXCAL_PROPERTY_VC1_SKIP_BFRAME						0x00000602
#define NEXCAL_PROPERTY_VC1_SKIP_BFRAME_NOT_OK					0	///< not skip B-frame
#define NEXCAL_PROPERTY_VC1_SKIP_BFRAME_OK						1	///< skip B-frame

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_MPEG4V\n
 * Property for start code which can be consumed bytestream on memory indicated by pData parameter, which is used by entering NEXCALVideoDecoder::Decode function.
*/
#define NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE			0x00000700
#define NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE_ANY			0	///< The start of visual bitstream can be made from any values. Ex) It is acceptable to start from video_object_start_code
#define NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE_VOP			1	///< The start of visual bitstream must be made from vop_start_code.

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_MPEG4V\n
 * Property to decide whether to support level 3 for simple profile
*/
#define NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT				0x00000701
#define NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT_YES				0	///< support Mpeg4 simple profile level3
#define NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT_NO				1	///< not support Mpeg4 simple profile level3

#define NEXCAL_PROPERTY_MPEG4_SUPPORT_COREPROFILE			0x00000702
#define NEXCAL_PROPERTY_MPEG4_SUPPORT_COREPROFILE_YES			0
#define NEXCAL_PROPERTY_MPEG4_SUPPORT_COREPROFILE_NO			1

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_MPEG4V\n
 * Property to decide to support any GMC
*/
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC					0x00000703
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_NO					0x01 << 0	///< GMC original value : 0
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_1					0x01 << 1	///< GMC original value : 1
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_2					0x01 << 2	///< GMC original value : 2
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_3					0x01 << 3	///< GMC original value : 3
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_YES					0xFF

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_MPEG4V\n
 * Property to decide to support any wrap point
*/
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT		0x00000704
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT_NO			0x00		///< no GMC wrappoint
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT_0			0x01 << 0	///< GMC wrappoint original value : 0
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT_1			0x01 << 1	///< GMC wrappoint original value : 1
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT_2			0x01 << 2	///< GMC wrappoint original value : 2
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT_3			0x01 << 3	///< GMC wrappoint original value : 3
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT_4			0x01 << 4	///< GMC wrappoint original value : 4
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT_ALL		0xFF

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_MPEG4V\n
 * Property to decide whether to support interlace
*/
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE			0x00000705
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE_YES			0	///> support interlaced content
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE_NO				1	///> not support interlaced content

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_MPEG4V\n
 * Property to decide whether to support QPEL
*/
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL					0x00000706
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL_YES					0	///> support mpeg4 video QPEL
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL_NO					1	///> not support mpeg4 video QPEL

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_MPEG4V\n
 * Property to get supportable profiles
*/
#define NEXCAL_PROPERTY_MPEG4V_SUPPORT_PROFILE 				0x00000707

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H263\n
 * Property to decide whether to support EXTENDED P TYPE
*/
#define NEXCAL_PROPERTY_H263_ACCEPT_EXTENDEDPTYPE			0x00000800
#define NEXCAL_PROPERTY_H263_ACCEPT_EXTENDEDPTYPE_YES			0	///> accept extended P-type
#define NEXCAL_PROPERTY_H263_ACCEPT_EXTENDEDPTYPE_NO			1	///> not accept extended P-type

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H263\n
 * Property to decide whether to support emulated start code
*/
#define NEXCAL_PROPERTY_H263_EMULATED_START_CODE_PLAY		0x00000801
#define NEXCAL_PROPERTY_H263_EMULATED_START_CODE_PLAY_YES		0	///> play emulated start code
#define NEXCAL_PROPERTY_H263_EMULATED_START_CODE_PLAY_NO		1	///> not play emulated start code

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_H263\n
 * Property to decide whether to support UMV
*/
#define NEXCAL_PROPERTY_H263_SUPPORT_UMV					0x00000802
#define NEXCAL_PROPERTY_H263_SUPPORT_UMV_YES					0	///> support H263 UMV
#define NEXCAL_PROPERTY_H263_SUPPORT_UMV_NO						1	///> not support H263 UMV

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_A_AMR\n
 * Property to decide whether to support SID
*/
#define NEXCAL_PROPERTY_SUPPORT_TO_DECODE_AMR_SID			0x00000900
#define NEXCAL_PROPERTY_SUPPORT_TO_DECODE_AMR_SID_YES       	0	///> support AMR SID
#define NEXCAL_PROPERTY_SUPPORT_TO_DECODE_AMR_SID_NO            1	///> not support AMR SID

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_A_AAC\n
 * Property to get whether to support mute frame
*/
#define NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME           0x00001000
#define NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME_NO			0	///> Error return when calling Getsilence frame
#define NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME_YES			1	///> support aac mute frame

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_A_AAC\n
 * Property to get whether to support ADTS header
*/
#define NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER			0x00001001
#define NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER_YES			0	///> support ADTS header
#define NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER_N0			1	///> not support ADTS header

// For Encoder ----------------------------------------------------------------------------------------------------------------------------------------------------------

#define NEXCAL_PROPERTY_AAC_SUPPORT_MAIN					0x00010000	///< support AAC main profile (UPPORT_YES or SUPPORT_NO)
#define NEXCAL_PROPERTY_AAC_SUPPORT_LTP 					0x00010001	///< support AAC LTP profile (UPPORT_YES or SUPPORT_NO)
#define NEXCAL_PROPERTY_AAC_SUPPORT_SBR 					0x00010002	///< support AAC SBR profile (UPPORT_YES or SUPPORT_NO)
#define NEXCAL_PROPERTY_AAC_SUPPORT_PS						0x00010003	///< support AAC PS profile (UPPORT_YES or SUPPORT_NO)
#define NEXCAL_PROPERTY_AAC_SUPPORT_SSR						0x00010004	///< support AAC SSR profile (UPPORT_YES or SUPPORT_NO)

#define NEXCAL_PROPERTY_GET_DECODER_INFO					0x00011000 ///< Decoder's property to get/set codec user-data 
#define NEXCAL_PROPERTY_GET_ENCODER_INFO					0x00011001 ///< Encoder's property to get/set codec user-data 

// Unused or unidentified ----------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef _NEX_SOUND_EFFECT_ENABLE_
/**
 * Property to get whether to support Virtual Cinema Sound
*/
	#define NEXCAL_PROPERTY_SOUND_VIRTUAL_CINEMA_ONOFF		0x00050000
	#define NEXCAL_PROPERTY_SOUND_VIRTUAL_CINEMA_OFF			0	///< sound virtual cinema off
	#define NEXCAL_PROPERTY_SOUND_VIRTUAL_CINEMA_ON				1	///< sound virtual cinema on
#endif

#ifdef _DOLBYMODE_ONOFF_SUPPORTED_
/**
 * Property to get whether to support Dolby Sound
*/
	#define NEXCAL_PROPERTY_SOUND_DOLBY_CONTROL				0x00050001
	#define NEXCAL_PROPERTY_SOUND_DOLBY_OFF						0	///< Dolby off
	#define NEXCAL_PROPERTY_SOUND_DOLBY_ON						1	///< Dolby on
#endif

#ifdef	_LGE_AMAS_SUPPORTED_
/**
 * Property to get whether to support LGE AMAS mode for LGE model
*/
	#define NEXCAL_PROPERTY_LGE_AMAS_MODE					0x00050002
	#define NEXCAL_PROPERTY_LGE_AMAS_MODE_NOT_SUPPORT			0	///< not support LGE AMAS
	#define NEXCAL_PROPERTY_LGE_AMAS_MODE_SUPPORT				1	///< support LGE AMAS

/**
 * Re-initialization of AMAS mode for LGE model
*/
	#define NEXCAL_PROPERTY_LGE_AMAS_FLUSH					0x00050003
#endif	//	_LGE_AMAS_SUPPORTED_

#define NEXCAL_PROPERTY_SUPPORT_TO_PAYLOAD_TYPE				0x00050004
#define NEXCAL_PROPERTY_SUPPORT_TO_PAYLOAD_TYPE_NOT_OK			0
#define NEXCAL_PROPERTY_SUPPORT_TO_PAYLOAD_TYPE_OK				1

#define NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_SUPPORTED	0x00050005			// Get only
#define NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR				0x00050006			// Get/Set
#define NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_INTERNAL			NTHFROMLSBMASK(0)	///< Use internal buffer
#define NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_EXTERNAL			NTHFROMLSBMASK(1)	///< Use external buffer


// Private CAL Property Start Here ----------------------------------------------------------------------------------------------------------------------------------------------------------
#define NEXCAL_PROPERTY_USER_START							0x70000000


//-----------------------------------------------------------------------------
//		SetProperty()'s properties and values
//-----------------------------------------------------------------------------

// For General Codec
//
// 	uValue is function pointer. this prototype is "NXUINT32 _GetCurrentMediaCTS(NXINT32 nMedia, NXUINT32 uUserData)"
#define NEXCAL_SET_PROPERTY_MEDIA_CLOCK_FUNC			0x00000001	///< Property to set current time through GetCurrentMediaCTS
//	uValue is UserData of _GetCurrentMediaCTS.
#define NEXCAL_SET_PROPERTY_MEDIA_CLOCK_USERDATA		0x00000002	///< Property to set user-data when setting NEXCAL_SET_PROPERTY_MEDIA_CLOCK_FUNC

#define NEXCAL_PROPERTY_ITUNSMPB_DELAY_N_ZERO_PADDING	0x00000003


// For AVC Encoder
#define NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE 		0x0000004
#define NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_BASELINE		1
#define NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN			2
#define NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_EXTENDED		3
#define NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH			4
#define NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH10			5
#define NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH422		6
#define NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH444		7
#define NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL			0x0000005

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_HEVC\n
 * Property to set supportable profiles
*/
#define NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE 		0x00000100
#define NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN					1
#define NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN10				2
#define NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAINSTILLPICTURE		100
/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_HEVC\n
 * Property to set supportable level
*/
#define NEXCAL_PROPERTY_HEVC_ENCODER_SET_LEVEL			0x00000101

/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_MPEG4V\n
 * Property to set supportable profiles
*/
#define NEXCAL_PROPERTY_MPEG4V_ENCODER_SET_PROFILE		0x00000200
/**
 * Target Codec Object Type Indication value: eNEX_CODEC_V_MPEG4V\n
 * Property to set supportable level
*/
#define NEXCAL_PROPERTY_MPEG4V_ENCODER_SET_LEVEL		0x00000201

//-----------------------------------------------------------------------------
//		AudioGetInfo()'s indexes and values
//-----------------------------------------------------------------------------
#define NEXCAL_AUDIO_GETINFO_SAMPLINGRATE				0x00000001
#define NEXCAL_AUDIO_GETINFO_NUMOFCHNNELS				0x00000002
#define NEXCAL_AUDIO_GETINFO_BITSPERSAMPLE 				0x00000003
#define NEXCAL_AUDIO_GETINFO_NUMOFSAMPLESPERCHANNEL		0x00000004

//-----------------------------------------------------------------------------
//		VideoGetInfo()'s indexes and values
//-----------------------------------------------------------------------------
#define NEXCAL_VIDEO_GETINFO_WIDTH						0x00000001
#define NEXCAL_VIDEO_GETINFO_HEIGHT						0x00000002
#define NEXCAL_VIDEO_GETINFO_WIDTHPITCH					0x00000003
#define NEXCAL_VIDEO_GETINFO_HEIGHTPITCH				0x00000004
#define NEXCAL_VIDEO_GETINFO_NUM_INPUTBUFFERS			0x00000005
#define NEXCAL_VIDEO_GETINFO_NUM_INPUTBUFFERS_DEFAULT 		1
#define NEXCAL_VIDEO_GETINFO_INPUTBUFFER_SIZE			0x00000006
#define NEXCAL_VIDEO_GETINFO_INPUTBUFFER_SIZE_DEFAULT 		0x7FFFFFFF
#define NEXCAL_VIDEO_GETINFO_NUM_OUTPUTBUFFERS			0x00000007
#define NEXCAL_VIDEO_GETINFO_NUM_OUTPUTBUFFERS_DEFAULT 		1
#define NEXCAL_VIDEO_GETINFO_OUTPUTBUFFER_SIZE			0x00000008
#define NEXCAL_VIDEO_GETINFO_OUTPUTBUFFER_SIZE_DEFAULT		0x7FFFFFFF
#define NEXCAL_VIDEO_GETINFO_RENDERER_RESET				0x00000009
#define NEX_CAL_VIDEO_GETINFO_USE_RENDERER_RESET			1 // default nexplayer re-init renderer.
#define NEX_CAL_VIDEO_GETINFO_NOT_USE_RENDERER_RESET		0 // default nexplayer re-init renderer.
#define NEXCAL_VIDEO_GETINFO_AVERAGE_DECODING_TIME		0x0000000A //Average video decoding time
#define NEXCAL_VIDEO_GETINFO_CODEC_CLASS				0x0000000B // Codec Type (SW or HW)
#define NEXCAL_VIDEO_GETINFO_CODEC_CLASS_SW					0
#define NEXCAL_VIDEO_GETINFO_CODEC_CLASS_HW					1
#define NEXCAL_VIDEO_GETINFO_NEED_CODEC_REINIT			0x0000000C

#endif //_NEX_CAL_DOT_H_INCLUDED_

//ethan
#define NEX_CAL_TEXT_CEA608_CARRIAGE_RETURN				0
#define NEX_CAL_TEXT_CEA608_IGNORE_TEXTMODE				1
#define NEX_CAL_TEXT_CEA708_CALLBACK_FUNC				2
#define NEX_CAL_TEXT_CEA708_CALLBACK_TYPE				3
#define NEX_CAL_TEXT_CEA708_DISPLAY_MODE				4
#define NEX_CAL_TEXT_CEA708_COOKIE						5
#define NEX_CAL_TEXT_LOGLEVEL							10

#define EIA708CC_CALLABCKTYPE_SERVICE_BLOCK_RAW 		1
#define EIA708CC_CALLABCKTYPE_SERVICE_BLOCK_BYTE 		2

#define EIA708CC_DISPLAY_MODE_4X3 						0
#define EIA708CC_DISPLAY_MODE_16X9 						1

#define NEX_CAL_TEXT_WEBVTT_PARSE_TYPE					1

/**
 * @}
*/

/*-----------------------------------------------------------------------------
 Revision History:
 Author		Date		Version		Description of Changes
 ------------------------------------------------------------------------------
 ysh		07-04-07					Draft.
 ysh		07-06-14					Revision.
 ysh		11-02-22					nexCAL_VideoDecoderGetInfo was introduced.
									NEXCAL_VDEC_RET_SETTINGS_CHANGED was added.
									NEXCAL_VDEC_RET_INPUT_THUMB was removed.
 ysh		11-05-31					uUserData parameter was added to GetProperty.
 ysh		11-06-01					SetProperty was introduced. (including AUDIO_CLOCK)
 ysh		12-07-31		3.0.0		revision - dynamic loading mechanism
 ysh		13-01-07		3.3.0		GetCodec was modified to support the multiple same available entries.
 shoh	14-09-02		4.0.0		changed version system
									modified to work regardless of data type models (support 64bit OS)
									changed value paramter of Get/Setproperty (NXUINT32 -> NXINT64)
									changed return value type (NXUINT32 -> NXINT32)									
shoh	14-12-23		4.0.1		1. modified to use NexCodecUtils APIs
									2. added frame dump features
									3. modified NexCAL_Wrapper for HEVC by jongbea.park
shoh	15-03-05		4.0.2		1. changed SW codec if contents are over spec in HW codec by Martin
									2. added NEXCAL_VIDEO_GETINFO_NEED_CODEC_REINIT
shoh	15-03-18		4.0.3		1. changed DSI stream format of H26x when using frame instead of DSI because original DSI is not valid
shoh	15-03-26		4.0.4		1. added NEXCAL_PROPERTY_GET_DECODER_INFO, NEXCAL_PROPERTY_GET_ENCODER_INFO property
									2. removed compile warning
shoh	15-07-29		4.2.0		1. changed interfaces of A/V/T decoder
									2. added defines of decode mode
									3. changed error value
									4. typo (NXCAL_PORPERTY_AVC_MAX_LEVEL -> NEXCAL_PROPERTY_AVC_MAX_LEVEL)
									5. added define (NEXCAL_PROPERTY_HEVC_MAX_LEVEL)
shoh	15-07-31		4.3.0		1. modified to calculate output time of audio PCM in CALBody (changed NEXCALAudioDecode/nexCAL_AudioDecoderDecode interface)
shoh	15-08-10		4.3.1		1. If profile property of AVC, HEVC do not set in CALBody, profile check is skipped.
									2. fixed wrong profile comparison in AVC, HEVC
shoh	15-09-21		4.4.0		1. changed flag setting method
										NEXCAL_INIT_(A/V/T)DEC_RET(ret)
										NEXCAL_SET_(A/V/T)DEC_RET(ret, flag)
										NEXCAL_DEL_(A/V/T)DEC_RET(ret, flag)
										NEXCAL_CHECK_(A/V/T)DEC_RET(ret, flag)
									2. reorganizing properties
									3. removed compile warning
shoh	18-02-01		4.6.0		1. Add method flexibly to get codec when calling nexCAL_GetCodec()
									2. Add EOS define into encoder directives
									3. Modify static properties (sw/hw/platform codec)
									4. Add properties of supportable profiles/level for MPEG4V, HEVC
 ----------------------------------------------------------------------------*/

