/*-----------------------------------------------------------------------------
	File Name   :	NexRAL.h
	Description :	the header file for both user of nexRAL.
					(the client and the provider)
 ------------------------------------------------------------------------------

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

 NexStreaming Confidential Proprietary
 Copyright (C) 2008~2011 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation

 Revision History was described at the bottom of this file.
-----------------------------------------------------------------------------*/

/**
* @mainpage NexRAL documentation
*
* NexRAL is an abbreviation of Nexstreaming's Rendering Adaptation Layer. As it can be inferred from the name, NexRAL is a rendering
* interface module between rendering clients such as NexPlayer and rendering providers.
*
* As NexRAL is an interface module, there are two parts which uses NexRAL. One is the rendering client such as NexPlayer and the other is the rendering provider.
* This document contains description from both points of view, and definitions of NexRAL APIs and their calling sequences.
*
* According to the target platform, rendering implementation varies. Porting Nexstreaming's solutions to the specific target platform requires implementing some
* of adaptation layers including NexRAL.
*
* NexRAL helps integrators of Nexstreaming's multimedia solutions to concentrate on the rendering part, and a simple test code using NexRAL helps easy debugging of renderer .
*
* @note Important : NexRAL refers to NexCommonDef.h, NexMediaDef.h, NexTypeDef.h and NexSAL.
*/

/**
 * @defgroup ral_api  NexRAL APIs 
 * @{
 *   @defgroup ral_basic Basic definitions
 *   @brief NexRAL Basic definitions
 *   @{
 *   @} 
 *   @defgroup ral_type Types
 *   @{
 *   @}
 *   @defgroup ral_function Functions
 *   @{
 *   @}
 * @}
 * @defgroup renderer_func Renderer Functions
 * @{
 *   @defgroup renderer_video Video
 *   @{
 *   @}
 *   @defgroup renderer_audio Audio
 *   @{
 *   @}
 *   @defgroup renderer_text Text
 *   @{
 *   @}
 * @}
*/

#ifndef _NEX_RAL_DOT_H_INCLUDED_
#define _NEX_RAL_DOT_H_INCLUDED_

#include "NexMediaDef.h"

#define NEXRAL_VERSION_MAJOR		3
#define NEXRAL_VERSION_MINOR		2
#define NEXRAL_VERSION_PATCH		2
#define NEXRAL_VERSION_BRANCH		"OFFICIAL"

#define NEXRAL_COMPATIBILITY_NUM	4


#define _NEX_AL_DLL_

#if (defined(_WIN8) || defined(WIN32) || defined(WINCE)) && defined(_NEX_AL_DLL_)
	#if defined(NEXRAL_EXPORTS)
		#define NEXRAL_API __declspec(dllexport)
	#else
		#define NEXRAL_API __declspec(dllimport)
	#endif
#else
	#define NEXRAL_API
#endif

/**
 * @ingroup ral_type
 * @brief NexRAL identifier returned by nexRAL_GetHandle()
 * @see nexRAL_GetHandle
*/
typedef struct NEXRALHandle_struct *NEXRALHandle;

/**
 * @ingroup ral_type
 * @brief Enumeration type indicating the renderer media type
*/
typedef enum
{
	NEXRAL_MEDIATYPE_VIDEO = 0,	///< Video
	NEXRAL_MEDIATYPE_AUDIO = 1,	///< Audio
	NEXRAL_MEDIATYPE_TEXT  = 2,	///< Text
	NEXRAL_MEDIATYPE_IMAGE = 3,	///< Image
} NEXRALMediaType;

//VIDEO Display Return type list
#define NEXRAL_VIDEO_RET_DISPLAY_SUCCESS		1
#define NEXRAL_VIDEO_RET_DISPLAY_NEXT_FRAME		2

//NexRAL Error type list
#define NEXRAL_ERROR_NONE						0
#define NEXRAL_ERROR_INVALID_PARAMETER			1
#define NEXRAL_ERROR_SYSTEM_FAIL				2
#define NEXRAL_ERROR_MEMORY_FAIL				3
#define NEXRAL_ERROR_RESOURCE_FAIL				4
#define NEXRAL_ERROR_NOT_SUPPORT_DEVICE			5
#define NEXRAL_ERROR_OVER_SPEC 					6
#define NEXRAL_ERROR_CREATE_VIDEO_INFO			10	
#define NEXRAL_ERROR_CREATE_AUDIO_INFO			11
#define NEXRAL_ERROR_CREATE_TEXT_INFO			12
#define NEXRAL_ERROR_FAIL						-1

/**
 * @ingroup ral_type
 * @brief Structure that holds video renderer functions used by NexRAL clients
*/
typedef struct _tag_NEXRALVideoRenderer
{
/**
 * @ingroup renderer_video
 * @brief Gets the video renderer property
 * @param[in] a_uProperty Property indicator the client requires
 * @param[out] a_pqValue Value about the requested property
 * @param[in] a_pUserData The User-Data
 * @return 0: Success, Other values: Failure
 * @note The property of rendering can vary. However, NexRAL defines the minimal properties that makes an influence on the implementation
 			method of the NexRAL client. Property is not meaningful in current version of NexRAL. However, this is for future expandability.
*/	
	NXINT32 (*GetProperty)(NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData);

/**
 * @ingroup renderer_video
 * @brief Sets the video renderer property
 * @param[in] a_uProperty Property indicator the client requires
 * @param[in] a_qValue Set the Value about the requested property
 * @param[in] a_pUserData The User-Data
 * @return 0: Success, Other values: Failure
 * @note The property of rendering can vary. However, NexRAL defines the minimal properties that makes an influence on the implementation
 			method of the NexRAL client. Property is not meaningful in current version of NexRAL. However, this is for future expandability.
*/	
	NXINT32 (*SetProperty)(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData);
	
/**
 * @ingroup renderer_video
 * @brief Sets the video renderer property
 * @param[in] a_eCodecType Indicates the codec object type indication values. For general renderer, this value can be ignored.
 * @param[in] a_uWidth Image frame width transferred from NEXRALVideoRenderer::Decode (in pixel). This value can be used for elsewhere purpose.
 * @param[in] a_uHeight Image frame height transferred from NEXRALVideoRenderer::Decode (in pixel). This value can be used for elsewhere purpose.
 * @param[in] a_uPitch  Image frame pitch transferred from NEXRALVideoRenderer::Decode (in pixel). This value can be used for elsewhere purpose.
 * @param[out] a_ppUserData User-data. The provider can pass the custom value. This value will be passed with last parameter by the client, when other functions are called.
 * @return 0: Success, Other values: Failure
 * @note This function is used to initialize the video renderer. For values of uWidth, uHeight and uPitch, values transferred using NEXCALVideoDecoder::Init are used. 
 *			Hence, if NexCAL and NexRAL is implemented simultaneously for specific codec, arbitrarily values can be used.
*/
	NXINT32 (*Init)(NEX_CODEC_TYPE a_eCodecType, NXUINT32 a_uWidth, NXUINT32 a_uHeight, NXUINT32 a_uPitch, NXVOID **a_ppUserData);

/**
 * @ingroup renderer_video
 * @brief Closes the video renderer
 * @param[in] a_pUserData User data which is obtained when Init is called by the client
 * @return 0: Success, Other values: Failure
 * @note This function is used to close the video renderer. When the video renderer is not in use, and wants to close the renderer, this function must be called.
*/
	NXINT32 (*Deinit)(NXVOID *a_pUserData);

/**
 * @ingroup renderer_video
 * @brief Renders single video frame
 * @param[in] a_bDisplay decide to display
 * @param[in] a_uCTS Current Time Value
 * @param[in] a_pBits1 Pointer of image to be displayed. This value will have different meanings based on input buffer format of the renderer.
 * @param[in] a_pBits2 Pointer of image to be displayed. This value will have different meanings based on input buffer format of the renderer.
 * @param[in] a_pBits3 Pointer of image to be displayed. This value will have different meanings based on input buffer format of the renderer.
 * @param[out] a_pResult The result of display
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client
 * @return 0: Success, Other values: Failure
 * @note This function is used to close the video renderer. When the video renderer is not in use, and wants to close the renderer, this function must be called.
*/
	NXINT32 (*Display)(NXBOOL a_bDisplay, NXUINT32 a_uCTS, NXUINT8 *a_pBits1, NXUINT8 *a_pBits2, NXUINT8 *a_pBits3, NXUINT32 *a_pResult, NXVOID *a_pUserData);
	
/**
 * @ingroup renderer_video
 * @brief Temporarily stops currently rendering
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*Pause)(NXVOID *a_pUserData);
		
/**
 * @ingroup renderer_video
 * @brief Restart rendering
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*Resume)(NXVOID *a_pUserData);

/**
 * @ingroup renderer_video
 * @brief clear buffer of renderer
 * @param[in] a_uCTS Current Time Value
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*Flush)(NXUINT32 a_uCTS, NXVOID *a_pUserData);
} NEXRALVideoRenderer;

/**
 * @ingroup ral_type
 * @brief Structure that holds audio renderer functions used by NexRAL clients.
*/
typedef struct _tag_NEXRALAudioRenderer
{
/**
 * @ingroup renderer_audio
 * @brief Gets the video renderer property
 * @param[in] a_uProperty Property indicator the client requires
 * @param[out] a_pqValue Value about the requested property
 * @param[in] a_pUserData The User-Data
 * @return 0: Success, Other values: Failure
 * @note The property of rendering can vary. However, NexRAL defines the minimal properties that makes an influence on the implementation
 *			method of the NexRAL client. Property is not meaningful in current version of NexRAL. However, this is for future expandability.
*/		
	NXINT32 (*GetProperty)(NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Sets the video renderer property
 * @param[in] a_uProperty Property indicator the client requires
 * @param[in] a_qValue Set the Value about the requested property
 * @param[in] a_pUserData The User-Data
 * @return 0: Success, Other values: Failure
 * @note The property of rendering can vary. However, NexRAL defines the minimal properties that makes an influence on the implementation
 *			method of the NexRAL client. Property is not meaningful in current version of NexRAL. However, this is for future expandability.
*/
	NXINT32 (*SetProperty)(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Initializes the audio renderer
 * @param[in] a_eCodecType Indicates the codec object type indication values. For general renderer, this value can be ignored.
 * @param[in] a_uSamplingRate Sampling rate in Hz
 * @param[in] a_uNumOfChannels Number of channels
 * @param[in] a_uBitsPerSample Number of bits per sample
 * @param[in] a_uNumOfSamplesPerChannel Number of samples per channel
 * @param[out] a_ppUserData User-data. The provider can pass custom value. This value will be passed with last parameter by the client, when other functions are called.
 * @return 0: Success, Other values: Failure
 * @note For values of a_uSamplingRate, a_uNumOfChannels and a_uNumOfSamplesPerChannel values transferred using NexCALVideoDecoder::Init are used. 
 *			Hence, if NexCAL and NexRAL is implemented simultaneously for specific codec, arbitrarily values can be used.
*/
	NXINT32 (*Init)(NEX_CODEC_TYPE a_eCodecType, NXUINT32 a_uSamplingRate, NXUINT32 a_uNumOfChannels, NXUINT32 a_uBitsPerSample, NXUINT32 a_uNumOfSamplesPerChannel, NXVOID **a_ppUserData);

/**
 * @ingroup renderer_audio
 * @brief Closes the audio renderer
 * @param[in] a_pUserData User data which is obtained when Init is called by the client
 * @return 0: Success, Other values: Failure
 * @note This function is used for closing the audio renderer. When the audio renderer is not in use, and wants to close the renderer, this function must be called.
*/
	NXINT32 (*Deinit)(NXVOID *a_pUserData);
	
/**
 * @ingroup renderer_audio
 * @brief Gets an empty buffer. This buffer is used as destination buffer of the audio decoder
 * @param[out] a_nMaxBufferSize Address that pointer of audio buffer is to be saved which will hold data in specific format. If bits per sample is 8 then must typecast this pointer to char pointer.
 * @param[in] a_nMaxBufferSize Buffer size of Audio renderer
 * @param[in] a_pUserData User data which is obtained when Init is called by the client
 * @return 0: Success, Other values: Failure
 * @note This function is used for closing the audio renderer. When the audio renderer is not in use, and wants to close the renderer, this function must be called.
*/
	NXINT32 (*GetEmptyBuffer)(NXVOID **a_ppEmptyBuffer, NXINT32 *a_nMaxBufferSize, NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Consumes and renders the buffer
 * @param[in] a_pBuffer Rendering target data implemented in input buffer format based on provider's perspective.
 *						Buffer pointer transferred through GetEmptyBuffer is passed to pBuffer as unchanged.
 * @param[in] a_nBufferLen Length of a_pBuffer
 * @param[in] a_uTime Timestmp that is calculated by decoder
 * @param[in] a_bDecodeSuccess Normality check value of data in pBuffer
 *								- 0 : decoding failure
 *								- Other values : decoding success
 *								 If it is private input buffer format, this value can be processed as meaningless data.
 * @param[in] a_bEndFrame flag parameter of end frame
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
 * @note GetEmptyBuffer and ConsumeBuffer should be called in sequential order, and must be matched together.
*/
	NXINT32 (*ConsumeBuffer)(NXVOID *a_pBuffer, NXINT32 a_nBufferLen, NXUINT32 a_uTime, NXBOOL a_bDecodeSuccess, NXBOOL a_bEndFrame, NXVOID *a_pUserData);
	
/**
 * @ingroup renderer_audio
 * @brief Processes mute. In order to process mute, fill buffer with mute values before calling ConsumeBuffer.
 * @param[in] a_pBuffer  Rendering target buffer pointer, which is to be converted and filled as mute sound.
 *						Through GetEmptyBuffer, transferred buffer pointer should be passed on to pBuffer without any changes.
 * @param[out] a_bSetPCMSize whether to set PCM size or not. If not, renderer mutes as much as default size.
 * @param[in] a_pnPCMSize Size of One Frame in Byte
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
 * @note GetEmptyBuffer and ConsumeBuffer should be called in sequential order, and must be matched together.
*/
	NXINT32 (*SetBufferMute)(NXVOID *a_pBuffer, NXBOOL a_bSetPCMSize, NXINT32 *a_pnPCMSize, NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Gets current CTS. Current CTS is CTS of buffer currently being outputted by the device.
 * @param[out] a_puCTS Pointer to put current CTS. Provider shall fill current CTS to this address and return it.
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
 * @note This function can be called from different tasks.
*/
	NXINT32 (*GetCurrentCTS)(NXUINT32 *a_puCTS, NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Clears buffer of nexRAL's implementation part
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
 * @note In case of implementing NexRAL, it can be operated as round-robin form with small number of buffers. It may be meaningless to clear emptied buffer always, 
 *			in terms of performance, client shall call this function when it is required to clear the buffer.
 *			During the playback, the client can jump to arbitrary location. In this case, there may be buffers which have requested for rendering to renderer through
 *			ConsumeBuffer, but were not outputted on the actual device. This function is called when buffers such as these are needed to be cleared. 
 *			For some specific implementation, no work may be required at all.
*/
	NXINT32 (*ClearBuffer)(NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Temporarily stops currently rendering.
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*Pause)(NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Restart rendering
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*Resume)(NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Clear buffer of audio renderer and set timestamp
 * @param[in] a_uCTS Timestamp to set after clear buffer of audio renderer
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*Flush)(NXUINT32 a_uCTS, NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Set the Current time
 * @param[in] a_uCTS Timestamp to set the current timestamp
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*SetTime)(NXUINT32 a_uCTS, NXVOID *a_pUserData);

/**
 * @ingroup renderer_audio
 * @brief Set the playback speed
 * @param[in] a_nRate The change in speed to apply to the content of Speed Control (in %): from -75(0.25x) to +100(2.00x), 0%(the original speed).
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*SetPlaybackRate)(NXINT32 a_nRate, NXVOID *a_pUserData); 

/**
 * @ingroup renderer_audio
 * @brief Auto volume
 * @param[in] a_bEnable Set enable/disable auto volume
 * @param[in] a_uStrength The Strength preset to be applied to the Auto Volume. This will usually be an integer between 0 and 6.
 * @param[in] a_bRelease ???
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*mavenSetAutoVolumeParam) (NXBOOL a_bEnable, NXUINT32 a_uStrength, NXBOOL a_bRelease);

/**
 * @ingroup renderer_audio
 * @brief This function adjusts the pitch control information.
 * @param[in] a_nRate The index of pitch to apply to the content
 *						Range of pitch control (index): {-12, -11, -10, ... -1, 0, 1, ... 10, 11, 12}
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*SetPlayPitchRate)(NXINT32 a_nRate, NXVOID *a_pUserData); 
} NEXRALAudioRenderer;

/**
 * @ingroup ral_type
 * @brief Structure that holds text renderer functions used by NexRAL clients.
*/
typedef struct _tag_NEXRALTextRenderer
{
/**
 * @ingroup renderer_text
 * @brief Gets the property that the specific Text renderer has
 * @param[in] a_uProperty Property indicator the client requires
 * @param[in] a_pqValue Requested property value
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
 * @note The property of rendering can vary. However, NexRAL defines the minimal properties that makes an influence on the implementation
 *			method of the NexRAL client. Property is not meaningful in current version of NexRAL. However, this is for future expandability.
*/	
	NXINT32 (*GetProperty)(NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData);

/**
 * @ingroup renderer_text
 * @brief Sets the property that the specific Text renderer has
 * @param[in] a_uProperty Property indicator the client requires
 * @param[in] a_qValue Set the property value
 * @param[in] a_pUserData User-data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
 * @note The property of rendering can vary. However, NexRAL defines the minimal properties that makes an influence on the implementation
 *			method of the NexRAL client. Property is not meaningful in current version of NexRAL. However, this is for future expandability.
*/	
	NXINT32 (*SetProperty)(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData);

/**
 * @ingroup renderer_text
 * @brief Initializes the Text renderer
 * @param[in] a_eCodecType Indicates the codec object type indication values. For general renderer, this value can be ignored.
 * @param[in] a_pConfig Pointer to DSI(Decoder Specific Information) of base layer
 * @param[in] a_nConfigLen Length of the information of Text base Layer in byte
 * @param[out] a_ppUserData User-data. The provider can pass custom value. This value will be passed with last parameter by the client, when other functions are called.
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*Init)(NEX_CODEC_TYPE a_eCodecType, NXVOID *a_pConfig, NXINT32 a_nConfigLen, NXVOID **a_ppUserData);

/**
 * @ingroup renderer_text
 * @brief Closes the Text renderer
 * @param[in] a_pUserData User data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
 * @note This function is used for closing the Text renderer. When the Text renderer is not in use, and wants to close the renderer, this function must be called.
*/
	NXINT32 (*Deinit)(NXVOID *a_pUserData);

/**
 * @ingroup renderer_text
 * @brief Draw the Text 
 * @param[in] a_pTextBuffer Pointer to Caption Infomation 
 * @param[in] a_nTextBufferLen Length of the information of a_pTextBuffer
 * @param[in] a_pUserData User data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*DrawText)(NXVOID *a_pTextBuffer, NXINT32 a_nTextBufferLen, NXVOID *a_pUserData);

/**
 * @ingroup renderer_text
 * @brief Erase the Text  
 * @param[in] a_pUserData User data which is obtained when Init is called by the client.
 * @return 0: Success, Other values: Failure
*/
	NXINT32 (*EraseText)(NXVOID *a_pUserData);
} NEXRALTextRenderer;

typedef NXINT32 (*NEXRALGetRendererNotification)(NXBOOL a_bRelease, NXVOID *a_pClientID, NXVOID *a_pUserData);

#ifdef __cplusplus
extern "C" {
#endif

/**
* @ingroup ral_basic
* @brief return version info with string
* @return Pointer of string which consists of 3 digits with one word and it means major version number, 
*	minor version number, patch version number, and branch version name in sequence. (Ex. "1.0.0.OFFICIAL")
*/
	NEXRAL_API const NXCHAR* nexRAL_GetVersionString();

/**
* @ingroup ral_basic
* @brief return major version with NXINT32
* @return Major version number
*/
	NEXRAL_API NXINT32 nexRAL_GetMajorVersion();

/**
* @ingroup ral_basic
* @brief return minor version with NXINT32
* @return Minor version number
*/
	NEXRAL_API NXINT32 nexRAL_GetMinorVersion();

/**
* @ingroup ral_basic
* @brief return patch version with NXINT32
* @return Patch version number
*/
	NEXRAL_API NXINT32 nexRAL_GetPatchVersion();

/**
* @ingroup ral_basic
* @brief return branch version with NXINT32
* @return Branch version string
*/
	NEXRAL_API const NXCHAR* nexRAL_GetBranchVersion();

/**
* @ingroup ral_basic
* @brief return version info with string
* @return Version information
*/
	NEXRAL_API const NXCHAR* nexRAL_GetVersionInfo();

/**
* @ingroup ral_basic
* @brief check library version whether the same or not
* @param[in] a_nMajor major version
* @param[in] a_nMinor minor version
* @param[in] a_nPatch patch version
* @param[in] a_strBranch branch version
* @return TRUE: the same version, FALSE: another version
*/
	NEXRAL_API NXBOOL nexRAL_CheckSameVersion(NXINT32 a_nMajor, NXINT32 a_nMinor, NXINT32 a_nPatch, NXCHAR *a_strBranch);

/**
* @ingroup ral_basic
* @brief check compatibility number
* @param[in] a_nCompatibilityNum compatibility number
* @return TRUE: compatible, FALSE: not compatible
*/
	NEXRAL_API NXBOOL nexRAL_CheckCompatibleVersion(NXINT32 a_nCompatibilityNum);

/**
 * @ingroup ral_function
 * @brief Get the handle of NexRAL. If NexRAL instance exists, then returns previous handle value, otherwise creates the NexRAL instance.
 * @param[in] a_nFrom caller type indicator (0: nexRAL provider; 1: nexRAL client)
 * @param[in] a_nCompatibilityNum NexRAL's nCompatibilityNum. Generally, inputs the nCompatibilityNum value defined in NexRAL header file.
 * @return 0: Fail, Other values: Handle values
 * @note NexRALHandle value is needed to call this function, hence getting handle of nexRAL is necessary.
*/
	NEXRAL_API NEXRALHandle nexRAL_GetHandle(NXINT32 a_nFrom, NXINT32 a_nCompatibilityNum);

/**
 * @ingroup ral_function
 * @brief Get the handle of NexRAL. If NexRAL instance exists, then returns previous handle value, otherwise creates the NexRAL instance.
 * @param[in] a_hRAL Value of NEXRALHandle. Obtained using nexRAL_GetHandle().
 * @return None
 * @note As per nexRAL_GetHandle the reference count of handle is decreases by one, and the reference count becomes 0, then the instance of NexRAL is removed.
*/
	NEXRAL_API NXVOID nexRAL_ReleaseHandle(NEXRALHandle a_hRAL);

/**
 * @ingroup ral_function
 * @brief Get the handle of NexRAL. If NexRAL instance exists, then returns previous handle value, otherwise creates the NexRAL instance.
 * @param[in] a_hRAL Value of NEXRALHandle. Obtained using nexRAL_GetHandle().
 * @param[in] a_eMediaType Renderer media type
 * @param[in] a_uBufferFormat Buffer formats which can be processed by the renderer. 
 *								If the provider of NexCAL and NexRAL is the same, this value can be custom value.
 * @param[in] a_pFuncs Pointer to interface of the video renderer.
 *						- If the eMediaType is NEXRAL_MEDIATYPE_VIDEO then this pointer indicates NEXRALVideoRenderer.
 *						- If the eMediaType is NEXRAL_MEDIATYPE_AUDIO then this pointer indicates NEXRALAudioRenderer.
 *						- If the eMediaType is NEXRAL_MEDIATYPE_TEXT then this pointer indicates NEXRALTextRenderer.
 *						- Ignore other values in this version of NexRAL.
 * @param[in] nFuncTableSize table size (in bytes).
 * @param[in] a_pNotificationCB register renderer notification callback
 * @param[in] a_pUserData The user-data
 * @return None
 * @note The memory which pFuncs points can be reused after this API is returned, because NexRAL copies the memory to another memory of NexRAL.
 *		The memory allocated in NexRAL will be freed when the reference count of the NexRAL becomes 0.
*/
	NEXRAL_API NXINT32 nexRAL_RegisterRenderer(NEXRALHandle a_hRAL, NEXRALMediaType a_eMediaType, NXUINT32 a_uBufferFormat, NXVOID *a_pFuncs, NXINT32 nFuncTableSize, NEXRALGetRendererNotification a_pNotificationCB, NXVOID *a_pUserData);

/**
 * @ingroup ral_function
 * @brief Gets the renderer interface, consisted of functions. NexRAL finds an entry with the combination value of eType and uCodecObjectTypeIndication,
 *		and returns the interface (pFuncs) registered by nexRAL_RegisterCodec. Entries registered in NexRAL can be obtained from uCodeObjectTypeIndication
 *		value. However, exact eType value must be transferred. 
 * @param[in] a_hRAL Value of NEXRALHandle returned by nexRAL_GetHandle()
 * @param[in] a_eMediaType Rendering media type
 * @param[in] a_uBufferFormat Buffer formats which can be processed by the renderer. 
 *								If the provider of NexCAL and NexRAL is the same, this value can be custom value.
 * @param[in] a_pClientID Client ID to ues identificaion when registering rederer notification callback
 * @return The pointer of registered rendering interface, which is a table of the rendering functions.
 * @note The pointer of interface, which is the return value, indicates to a table of functions inside nexRAL.
 *			Hence, the client of NexRAL should not modify the value that the return value points to.
*/
	NEXRAL_API NXVOID* nexRAL_GetRenderer(NEXRALHandle a_hRAL, NEXRALMediaType a_eMediaType, NXUINT32 a_uBufferFormat, NXVOID *a_pClientID);

/**
 * @ingroup ral_function
 * @brief Gets the renderer interface, consisted of functions. NexRAL finds an entry with the combination value of eType and uCodecObjectTypeIndication,
 *		and returns the interface (pFuncs) registered by nexRAL_RegisterCodec. Entries registered in NexRAL can be obtained from uCodeObjectTypeIndication
 *		value. However, exact eType value must be transferred. 
 * @param[in] a_hRenderer Pointer to interface of the renderer.
 * @param[in] a_pClientID Client ID to ues identificaion when registering rederer notification callback
 * @return None
*/
	NEXRAL_API NXVOID nexRAL_ReleaseRenderer(NXVOID *a_hRenderer, NXVOID *a_pClientID);

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------------------------------------------
//Video Render	Set/Get, Value : Normal or 3D or 360VR
#define	NEXRAL_PROPERTY_VIDEO_RENDER_MODE				0x00000001
#define	NEXRAL_PROPERTY_VIDEO_RENDER_MODE_NORMAL		0
#define	NEXRAL_PROPERTY_VIDEO_RENDER_MODE_3D			1
#define	NEXRAL_PROPERTY_VIDEO_RENDER_MODE_360VR		2

//Video Render	Set, Value : MetaData Size
#define	NEXRAL_PROPERTY_VIDEO_360VR_META_SIZE			0x00000002

//Video Render	Set, Value : MetaData Data
#define	NEXRAL_PROPERTY_VIDEO_360VR_META_DATA			0x00000003

//Video Render	Set, Value : Depend on UI & RAL Body
#define	NEXRAL_PROPERTY_VIDEO_360VR_COORDINATES			0x00000004

//Video Render Set, Value : NEXPLAYER3DType
#define	NEXRAL_PROPERTY_VIDEO_3D_TYPE					0x00000005

//------------------------------------------------------------------------------------------------------------
// Audio Renderer Get, Value : Normal or LPA
#define	NEXRAL_PROPERTY_AUDIO_RENDERER_MODE				0x20000001
#define	NEXRAL_PROPERTY_AUDIO_RENDERER_MODE_NORMAL	0
#define	NEXRAL_PROPERTY_AUDIO_RENDERER_MODE_LPA		1

// Audio Render Set/Get, Value : TRUE or FALSE
#define	NEXRAL_PROPERTY_AUDIO_ARM_EMULATOR_MODE		0x20000002

// Audio Render Set/Get, Value : -6 ~ 0
#define	NEXRAL_PROPERTY_AUDIO_NEXSOUND_HEADROOM_VALUE		0x20000003

//------------------------------------------------------------------------------------------------------------
#define NEXRAL_PROPERTY_USER_START							0x70000000

#endif //_NEX_RAL_DOT_H_INCLUDED_
/*-----------------------------------------------------------------------------
 Revision History:
 Author		Date	Version	Description of Changes
 ------------------------------------------------------------------------------
ysh		11-04-12			bDisplay parameter was added to NEXRALVideoRenderer::Display function. (1 = display, 0 = skip)
ysh		11-05-31			uUserData parameter was added to GetProperty.
ysh		11-06-02			SetProperty was introduced. (including AUDIO_CLOCK)
ysh		11-06-02			Video::Display was modified, pause/resume/flush were added to Video RAL.
shoh	14-09-02	3.0.0	changed version system
							modified to work regardless of data type models (support 64bit OS)
							changed return type (NXUINT32 -> NXINT32)
							changed value paramter of Get/Setproperty (NXUINT32 -> NXINT64)
shoh	15-07-31	3.1.0	modified to set time when calling ConsumeBuffer() (changed GetEmptyBuffer/ConsumeBuffer interfaces)
Robin	15-08-04	3.1.1	Add NEXRAL_SET_PROPERTY_NEXSOUND_HEADROOM_VALUE property. It is numeric value between -6 to 0
shoh	15-08-12	3.2.0	added a parameter to SetBufferMute (SetBufferMute can be set PCM size)
							NXINT32 (*SetBufferMute)(NXVOID *a_pBuffer, NXBOOL a_bSetPCMSize, NXINT32 *a_pnPCMSize, NXVOID *a_pUserData);
Robin	16-06.14			1. Remove Property
								A.	NEXRAL_PROPERTY_AVSYNC_OWNER
								B.	NEXRAL_PROPERTY_CIQ_FRAMERATE
								C.	NEXRAL_SET_PROPERTY_MEDIA_CLOCK_FUNC
								D.	NEXRAL_SET_PROPERTY_MEDIA_CLOCK_USERDATA
								E.	NEXRAL_SET_PROPERTY_LGE_AMAS_MODE
								F.	NEXRAL_SET_PROPERTY_ARM_EMULATOR_MODE
								G.	NEXRAL_PROPERTY_RENDER_MODE_NONE
								H.	NEXRAL_PROPERTY_RENDER_MODE_SW
								I.	NEXRAL_PROPERTY_RENDER_MODE_PARTIAL_HW
								J.	NEXRAL_PROPERTY_RENDER_MODE_HW
						2. Change Property
								A.	NEXRAL_SET_PROPERTY_MEDIA_3D_CONTENT => NEXRAL_PROPERTY_VIDEO_3D_TYPE
								B.	NEXRAL_SET_PROPERTY_ARM_EMULATOR_MODE => NEXRAL_PROPERTY_AUDIO_ARM_EMULATOR_MODE
								C.	NEXRAL_SET_PROPERTY_NEXSOUND_HEADROOM_VALUE => NEXRAL_PROPERTY_AUDIO_NEXSOUND_HEADROOM_VALUE
						3. Add 360vr property
								A.	NEXRAL_PROPERTY_VIDEO_RENDER_MODE
								B.	NEXRAL_PROPERTY_VIDEO_360VR_META_SIZE
								C.	NEXRAL_PROPERTY_VIDEO_360VR_META_DATA
								D.	NEXRAL_PROPERTY_VIDEO_360VR_COORDINATES
								E.	NEXRAL_PROPERTY_VIDEO_3D_TYPE
 ----------------------------------------------------------------------------*/

