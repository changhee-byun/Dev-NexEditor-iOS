/******************************************************************************
 * File Name   : NexChunkParser.h
 * Description : Common media definition.
 *******************************************************************************
 
	  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	  PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2010-2014 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

/**
* @mainpage NexChunkParser documentation
*
* This document belongs to Nextreaming Corporation.\n
* NexChunkParser guarantees frame of audio/video that can be decoded and be made of chunk.
* Also NexChunkParser calclulates PTS(Presentation Time Stamp) in the case of H264 and H265.
* @note Important: NexChunkParser refers to NexCommonDef.h, NexMediaDef.h, NexTypeDef.h and NexSAL.
*/

/**
 * @defgroup chunk_basic Basic
 * @brief Basic definition for NexChunkPasrer
 * @{
 * @}
 * @defgroup chunk_audio Audio
 * @brief Audio definitino for NexChunkParser
 * @{
 * @}
 * @defgroup chunk_video Video
 * @brief Video definition for NexChunkPaser
 * @{
 * @}
 */


#ifndef __NEX_CHUNK_PARSER_H__
#define __NEX_CHUNK_PARSER_H__

#include "NexUtil.h"

#define NEXCHUNKPARSER_VERSION_MAJOR	1
#define NEXCHUNKPARSER_VERSION_MINOR	3
#define NEXCHUNKPARSER_VERSION_PATCH	2
#define NEXCHUNKPARSER_VERSION_BRANCH	"OFFICIAL"

#define NEXCHUNKPARSER_COMPATIBILITY	3

/**
 * @ingroup chunk_audio
 * No option
 */
#define AUDIOCHUNKPARSER_GETFRAME_OPTION_NORMAL		0
/**
 * @ingroup chunk_audio
 * force the chunk parser to deliver the remain chunk
 */
#define AUDIOCHUNKPARSER_GETFRAME_OPTION_FORCEGET	1
/**
 * @ingroup chunk_audio
 * The chunk parser decides whether to deliver the remain chunk or not
 */
#define AUDIOCHUNKPARSER_GETFRAME_OPTION_GETREMAIN	2

/**
 * @ingroup chunk_audio
 * @brief Handle of audio chunk
 */
typedef void* AUDIOCHUNK_HANDLE;

/**
 * @ingroup chunk_video
 * No option
 */
#define VIDEOCHUNKPARSER_GETFRAME_OPTION_NORMAL		0
/**
 * @ingroup chunk_video
 * force the chunk parser to deliver the remain chunk
 */
#define VIDEOCHUNKPARSER_GETFRAME_OPTION_FORCEGET	1
/**
 * @ingroup chunk_video
 * The chunk parser decides whether to deliver the remain chunk or not
 */
#define VIDEOCHUNKPARSER_GETFRAME_OPTION_GETREMAIN	2

/**
 * @ingroup chunk_video
 * @brief Handle of video chunk
 */
typedef NXVOID* VIDEOCHUNK_HANDLE;

/**
 * @ingroup chunk_video
 * @brief Structures to use video information
 */
typedef struct
{
	NEX_CODEC_TYPE m_eCodecType;		///< codec type
	NXUINT32 m_uFourCC;					///< FourCC
	NEX_FILEFORMAT_TYPE m_eFFType;		///< file format type
	NXUINT32 m_uNalHeaderLen;			///< NAL Header Length 
	NXUINT8* m_pDSI;					///< DSI
	NXUINT32 m_uDSILen;					///< length of DSI
	NXUINT8* m_pExtraDSI;				///< Extra DSI
	NXUINT32 m_uExtraDSILen;			///< length of DSI
	NXUINT8* m_pExtraData;				///< BitmapInfo->m_pExtraData.
	NXBOOL m_bEncrypted;				///< encrypted or not
	NXUINT32 m_uTimescale;				///< Timescale value. if nTimescale is 0, it means user wants the calculated time with millisecond scale not using timescale.
	NXVOID* m_pReserved;				///< Reserverd
} VIDEOCHUNK_EXTINFO;


#ifdef __cplusplus
extern "C" 
{
#endif
/**
* @ingroup chunk_basic
* @brief return version info with string
* @return Pointer of string which consists of 3 digits with one word and it means major version number, 
*	minor version number, patch version number, and branch version name in sequence. (Ex. "1.0.0.OFFICIAL")
*/
	const NXCHAR* NexChunkParser_GetVersionString();

/**
* @ingroup chunk_basic
* @brief return major version with NXINT32
* @return Major version number
*/
	const NXINT32 NexChunkParser_GetMajorVersion();

/**
* @ingroup chunk_basic
* @brief return minor version with NXINT32
* @return Minor version number
*/
	const NXINT32 NexChunkParser_GetMinorVersion();

/**
* @ingroup chunk_basic
* @brief return patch version with NXINT32
* @return Patch version number
*/
	const NXINT32 NexChunkParser_GetPatchVersion();

/**
* @ingroup chunk_basic
* @brief return branch version with NXINT32
* @return Branch version string
*/
	const NXCHAR* NexChunkParser_GetBranchVersion();

/**
* @ingroup chunk_basic
* @brief return version info with string
* @return Version information
*/
	const NXCHAR* NexChunkParser_GetVersionInfo();

/**
* @ingroup chunk_basic
* @brief check library version whether the same or not
* @param[in] a_nMajor major version
* @param[in] a_nMinor minor version
* @param[in] a_nPatch patch version
* @param[in] a_strBranch branch version
* @return TRUE: the same version, FALSE: another version
*/
	NXBOOL NexChunkParser_CheckSameVersion(IN NXINT32 a_nMajor, IN NXINT32 a_nMinor, IN NXINT32 a_nPatch, IN NXCHAR *a_strBranch);

/**
* @ingroup chunk_basic
* @brief check compatibility number
* @param[in] a_nCompatibilityNum compatibility number
* @return TRUE: compatible, FALSE: not compatible
*/
	NXBOOL NexChunkParser_CheckCompatibleVersion(IN NXINT32 a_nCompatibilityNum);

/**
* @ingroup chunk_audio
* @brief This function creates instance of AudioChunkParser.
* @param[in] a_eCodecType codec type
* @param[in] a_uFourCC FourCC
* @param[in] a_eFFType file format type
* @param[in] a_pWaveFormatEx pointer of WaveFormatEx
* @param[in] a_uTimescale Timescale value. if nTimescale is 0, it means user wants the calculated time with millisecond scale not using timescale.
* @param[in] a_pUserData reserved parmater
* @return Handle of AudioChunkParser
*/
	AUDIOCHUNK_HANDLE AudioChunkParser_CreateInstance(IN NEX_CODEC_TYPE a_eCodecType, IN NXUINT32 a_uFourCC, IN NEX_FILEFORMAT_TYPE a_eFFType, IN NXVOID *a_pWaveFormatEx, IN NXUINT32 a_uTimescale, IN NXVOID *a_pUserData);

/**
* @ingroup chunk_audio
* @brief This function destroys instance of AudioChunkParser.
* @param[in] a_hChunkInfo handle of AudioChunkParser
* @return None
*/
	NXVOID AudioChunkParser_Destroy(IN AUDIOCHUNK_HANDLE a_hChunkInfo);

/**
* @ingroup chunk_audio
* @brief This function resets instance of AudioChunkParser after special even like seek.
* @param[in] a_hChunkInfo handle of AudioChunkParser
* @return None
*/
	NXVOID AudioChunkParser_Reset(IN AUDIOCHUNK_HANDLE a_hChunkInfo);

/**
* @ingroup chunk_audio
* @brief This function puts chunk into chunk pool to get audio frames.
* @param[in] a_hChunkInfo handle of AudioChunkParser
* @param[in] a_pChunk pointer of chunk form parser
* @param[in] a_uChunkLen length of chunk from parser
* @param[in] a_uDTS DTS of chunk from parser (32bit variable)
* @param[in] a_uPTS PTS of chunk from parser (32bit variable)
* @return 0: success, Others: fail
*/
	NXINT32 AudioChunkParser_PutFrame32(IN AUDIOCHUNK_HANDLE a_hChunkInfo, IN NXUINT8 *a_pChunk, IN NXUINT32 a_uChunkLen, IN NXUINT32 a_uDTS, IN NXUINT32 a_uPTS);

/**
* @ingroup chunk_audio
* @brief This function gets an audio frame from chunk pool. \n
*		『If return value is success and *a_ppFrame is NULL, it means chunk pool is insufficient to make frame and need to put more chunks into chunk pool.
* @param[in] a_hChunkInfo handle of AudioChunkParser
* @param[in] a_uGetOption mode of getting audio frame\n
*				- AUDIOCHUNKPARSER_GETFRAME_OPTION_NORMAL: deliver a verified frame\n
*				- AUDIOCHUNKPARSER_GETFRAME_OPTION_FORCEGET: force the chunk parser to deliver remain chunk\n
*				- AUDIOCHUNKPARSER_GETFRAME_OPTION_GETREMAIN: chunk parser decides whether to deliver remain chunk or not.
* @param[out] a_ppFrame a verified audio frame
* @param[out] a_puFrameLen length of a_ppFrame 
* @param[out] a_puFrameDTS DTS of a_ppFrame (32bit variable)
* @param[out] a_puFramePTS PTS of a_ppFrame (32bit variable)
* @return 0: success, Others: fail
*/
	NXINT32 AudioChunkParser_GetFrame32(IN AUDIOCHUNK_HANDLE a_hChunkInfo, IN NXUINT32 a_uGetOption, OUT NXUINT8 **a_ppFrame, OUT NXUINT32 *a_puFrameLen, OUT NXUINT32 *a_puFrameDTS, OUT NXUINT32 *a_puFramePTS);

/**
* @ingroup chunk_audio
* @brief This function puts chunk into chunk pool to get audio frames.
* @param[in] a_hChunkInfo handle of AudioChunkParser
* @param[in] a_pChunk pointer of chunk form parser
* @param[in] a_uChunkLen length of chunk from parser
* @param[in] a_qDTS DTS of chunk from parser (64bit variable)
* @param[in] a_qPTS PTS of chunk from parser (64bit variable)
* @return 0: success, Others: fail
*/
	NXINT32 AudioChunkParser_PutFrame64(IN AUDIOCHUNK_HANDLE a_hChunkInfo, IN NXUINT8 *a_pChunk, IN NXUINT32 a_uChunkLen, IN NXINT64 a_qDTS, IN NXINT64 a_qPTS);

/**
* @ingroup chunk_audio
* @brief This function gets an audio frame from chunk pool.\n
*		『If return value is success and *a_ppFrame is NULL, it means chunk pool is insufficient to make frame and need to put more chunks into chunk pool.
* @param[in] a_hChunkInfo handle of AudioChunkParser
* @param[in] a_uGetOption mode of getting audio frame\n
*				- AUDIOCHUNKPARSER_GETFRAME_OPTION_NORMAL: deliver a verified frame\n
*				- AUDIOCHUNKPARSER_GETFRAME_OPTION_FORCEGET: force the chunk parser to deliver remain chunk\n
*				- AUDIOCHUNKPARSER_GETFRAME_OPTION_GETREMAIN: chunk parser decides whether to deliver remain chunk or not.
* @param[out] a_ppFrame a verified audio frame
* @param[out] a_puFrameLen length of a_ppFrame 
* @param[out] a_pqFrameDTS DTS of a_ppFrame (64bit variable)
* @param[out] a_pqFramePTS PTS of a_ppFrame (64bit variable)
* @return 0: success, Others: fail
*/
	NXINT32 AudioChunkParser_GetFrame64(IN AUDIOCHUNK_HANDLE a_hChunkInfo, IN NXUINT32 a_uGetOption, OUT NXUINT8 **a_ppFrame, OUT NXUINT32 *a_puFrameLen, OUT NXINT64 *a_pqFrameDTS, OUT NXINT64 *a_pqFramePTS);

/**
* @ingroup chunk_video
* @brief This function creates instance of VideoChunkParser.
* @param[in] a_pExtInfo information to create instance of VideoChunkParser
* @ref VIDEOCHUNK_EXTINFO
* @param[in] a_pUserData reserved parmater
* @return Handle of VideoChunkParser
*/
	VIDEOCHUNK_HANDLE VideoChunkParser_CreateInstance(IN VIDEOCHUNK_EXTINFO *a_pExtInfo, IN NXVOID *a_pUserData);

/**
* @ingroup chunk_video
* @brief This function destroys instance of VideoChunkParser.
* @param[in] a_hChunkInfo handle of VideoChunkParser
* @return None
*/
	NXVOID VideoChunkParser_Destroy(IN VIDEOCHUNK_HANDLE a_hChunkInfo);

/**
* @ingroup chunk_video
* @brief This function resets instance of VideoChunkParser after special even like seek.
* @param[in] a_hChunkInfo handle of VideoChunkParser
* @return None
*/
	NXVOID VideoChunkParser_Reset(IN VIDEOCHUNK_HANDLE a_hChunkInfo);

/**
* @ingroup chunk_video
* @brief This function puts chunk into chunk pool to get video frames.
* @param[in] a_hChunkInfo handle of VideoChunkParser
* @param[in] a_pChunk pointer of chunk form parser
* @param[in] a_uChunkLen length of chunk from parser
* @param[in] a_uDTS DTS of chunk from parser (32bit variable)
* @param[in] a_uPTS PTS of chunk from parser (32bit variable)
* @return 0: success, Others: fail
*/
	NXINT32 VideoChunkParser_PutFrame32(IN VIDEOCHUNK_HANDLE a_hChunkInfo, IN NXUINT8 *a_pChunk, IN NXUINT32 a_uChunkLen, IN NXUINT32 a_uDTS, IN NXUINT32 a_uPTS);

/**
* @ingroup chunk_video
* @brief This function gets an audio frame from chunk pool.\n
*		『If return value is success and *a_ppFrame is NULL, it means chunk pool is insufficient to make frame and need to put more chunks into chunk pool.
* @param[in] a_hChunkInfo handle of VideoChunkParser
* @param[in] a_uGetOption mode of getting audio frame\n
*				- VIDEOCHUNKPARSER_GETFRAME_OPTION_NORMAL: deliver a verified frame\n
*				- VIDEOCHUNKPARSER_GETFRAME_OPTION_FORCEGET: force the chunk parser to deliver remain chunk\n
*				- VIDEOCHUNKPARSER_GETFRAME_OPTION_GETREMAIN: chunk parser decides whether to deliver remain chunk or not.
* @param[out] a_ppFrame a verified video frame
* @param[out] a_puFrameLen length of a_ppFrame 
* @param[out] a_puFrameDTS DTS of a_ppFrame (32bit variable)
* @param[out] a_puFramePTS PTS of a_ppFrame (32bit variable)
* @param[out] a_pbIFrame I-frame or not
* @return 0: success, Others: fail
*/
	NXINT32 VideoChunkParser_GetFrame32(IN VIDEOCHUNK_HANDLE a_hChunkInfo, IN NXUINT32 a_uGetOption, OUT NXUINT8 **a_ppFrame, OUT NXUINT32 *a_puFrameLen, OUT NXUINT32 *a_puFrameDTS, OUT NXUINT32 *a_puFramePTS, OUT NXBOOL *a_pbIFrame);

/**
* @ingroup chunk_video
* @brief This function puts chunk into chunk pool to get video frames.
* @param[in] a_hChunkInfo handle of VideoChunkParser
* @param[in] a_pChunk pointer of chunk form parser
* @param[in] a_uChunkLen length of chunk from parser
* @param[in] a_qDTS DTS of chunk from parser (64bit variable)
* @param[in] a_qPTS PTS of chunk from parser (64bit variable)
* @return 0: success, Others: fail
*/
	NXINT32 VideoChunkParser_PutFrame64(IN VIDEOCHUNK_HANDLE a_hChunkInfo, IN NXUINT8 *a_pChunk, IN NXUINT32 a_uChunkLen, IN NXINT64 a_qDTS, IN NXINT64 a_qPTS);

/**
* @ingroup chunk_video
* @brief This function gets an audio frame from chunk pool.\n
*		『If return value is success and *a_ppFrame is NULL, it means chunk pool is insufficient to make frame and need to put more chunks into chunk pool.
* @param[in] a_hChunkInfo handle of VideoChunkParser
* @param[in] a_uGetOption mode of getting audio frame\n
*				- VIDEOCHUNKPARSER_GETFRAME_OPTION_NORMAL: deliver a verified frame\n
*				- VIDEOCHUNKPARSER_GETFRAME_OPTION_FORCEGET: force the chunk parser to deliver remain chunk\n
*				- VIDEOCHUNKPARSER_GETFRAME_OPTION_GETREMAIN: chunk parser decides whether to deliver remain chunk or not.
* @param[out] a_ppFrame a verified video frame
* @param[out] a_puFrameLen length of a_ppFrame 
* @param[out] a_pqFrameDTS DTS of a_ppFrame (64bit variable)
* @param[out] a_pqFramePTS PTS of a_ppFrame (64bit variable)
* @param[out] a_pbIFrame I-frame or not
* @return 0: success, Others: fail
*/
	NXINT32 VideoChunkParser_GetFrame64(IN VIDEOCHUNK_HANDLE a_hChunkInfo, IN NXUINT32 a_uGetOption, OUT NXUINT8 **a_ppFrame, OUT NXUINT32 *a_puFrameLen, OUT NXINT64 *a_pqFrameDTS, OUT NXINT64 *a_pqFramePTS, OUT NXBOOL *a_pbIFrame);

/**
* @ingroup chunk_video
* @brief This function checks I-frame.
* @param[in] a_pFrame a verified audio frame
* @param[in] a_uFrameSize length of a_ppFrame
* @param[in] a_pExtInfo information to check I-frame
* @param[in] a_pUserData pointer to user data
* @ref VIDEOCHUNK_EXTINFO
* @return 0: success, Others: fail
*/
	NXBOOL VideoChunkParser_IsIFrame(IN NXUINT8* a_pFrame, IN NXUINT32 a_uFrameSize, IN NXVOID *a_pExtInfo, IN NXVOID *a_pUserData);

#ifdef __cplusplus
}
#endif
/**
 * @}
 */

#endif //__NEX_CHUNK_PARSER_H__

