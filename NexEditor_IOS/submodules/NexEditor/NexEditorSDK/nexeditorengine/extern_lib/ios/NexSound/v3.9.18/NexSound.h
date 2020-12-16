/**
 * @file	NexSound.h
 * @date	2015/04/09 12:45
 * @author	Audio Solution Team / NexStreaming Corp.
 * 		Contact: as@nexstreaming.com
 * 
 * @brief	This header file defines interface classes and functions to be used by the NexSound&trade;&nbsp;library.
 */
#ifdef NEX_SOUND_DLL_EXPORT
#define DLLAPI __declspec(dllexport)
#elif NEX_SOUND_DLL_IMPORT
#define DLLAPI __declspec(dllimport)
#else // For android library
#define DLLAPI
#endif

#ifndef NEXSOUNDLIB_H_20150225
#define NEXSOUNDLIB_H_20150225

#if defined(__cplusplus)
extern "C" {
#endif

#include "NexSound_DataDefine.h"

	/**
	* \mainpage NexSound&trade;&nbsp;TRM
	*
	* \section legal Legal Notices
	*
	* \par Disclaimer for Intellectual property
	* <i>This product is designed for general purpose, and accordingly the customer is
	* responsible for all or any of intellectual property licenses required for
	* actual application. NexStreaming Corporation does not provide any
	* indemnification for any intellectual properties owned by third party.</i>
	*
	* \par Copyright
	* Copyright for all documents, drawings and programs related with this
	* specification are owned by NexStreaming Corporation. All or any part of the
	* specification shall not be reproduced nor distributed without prior written
	* approval by NexStreaming Corporation. Content and configuration of all or any
	* part of the specification shall not be modified nor distributed without prior
	* written approval by NexStreaming Corporation.
	*
	* \par
	* &copy; Copyright 2013-2015 NexStreaming Corp. All rights reserved.
	*
	* \section abstract Abstract
	*
	* This document describes the NexSound&trade;&nbsp;library for processing audio and
	* how to use them.  It is a work in progress and will continuously be added to.
	*
	* \section overview Overview
	*
	* NexSound&trade;&nbsp;is a set of post-processing audio solutions that enhances the
	* audio quality of multimedia content on mobile devices.  NexSound&trade;&nbsp;provides
	* distinctive user listening experiences on earphones and mobile device speakers,
	* including surround sound effects and automatic maximization for mobile device users to
	* have a better listening environment anytime and anywhere.
	*
	* \section processors  NexSound&trade; Processors and Features
	*
	* - <b>Max Volume</b>:  Automatically maintains a constant volume level for all different kinds of sound sources such as movies, music, or mobile TV.
	* - <b>Surround Sound</b>:  Creates the feeling of cinematic surround sound on ordinary earphones and stereo speakers.
	* - <b>EarComfort</b>:  Provides a comfortable and natural listening environment on earphones.
	* - <b>Music Enhancer</b>:  Reinforces low frequency and widens the stereo image.
	* - <b>Resampler</b>: Converts the sampling rate of the input signal.
	* - <b>Live Concert</b>:  Creates the feeling of listening to live music in an actual concert hall.
	* - <b>Stereo Chorus</b>:  Creates a wide and comfortable stereo image from side to side.
	* - <b>Speed Control</b>:  Adjusts the playback speed of speech and multimedia sound.
	* - <b>Pitch Control</b>:  Adjusts pitch of speech and multimedia sound.
	* - <b>Vocal Remover</b>:  Remove or reduce the vocal from original music and turn them into karaoke tracks automatically.
	*
	*/

	/** \defgroup apifunc NexSound&trade; API Functions
	*
	* The API functions that can be called with the NexSound&trade;&nbsp;library to process audio.
        *
	* \see INexSound class
	*/

	/** \defgroup types INexSound Types
	*
	* Types that are part of the NexSound&trade;&nbsp;library and defined for the \c INexSound class.
        * 
        * \see INexSound class
	*/



/** @class		INexSound
 *  @brief		This interface class defines NexSound&trade;&nbsp;library functions to be called by an application to process audio with NexSound&trade;.  
 *
 * \see Please also see the \ref types section for type definitions in the \c INexSound class.
 * \see Please also see the \ref apifunc section for more details on the NexSound&trade;&nsbp;API functions.
 */
class INexSound
{
public:

	virtual	~INexSound() {}
	
	
	/**
	 * \ingroup apifunc 
	 * @brief		This function returns a string that contains NexSound&trade;&nbsp;library information.
	 * 
	 * Currently, this version of the NexSound&trade;&nbsp;library can only retrieve information about the main NexSound&trade;&nbsp;processor 
	 * (although future versions may be able to get information about each individual sound effect processor).  This means that currently
	 * the only possible value that should be assigned to the \c eProcessorTag parameter is \c Processor::NexSound. 
	 * 
	 * @param  	eProcessorTag	The tag of the processor to retrieve information about.  Currently only \c Processor::NexSound supported here.
	 * @param  	mode			Information to be retrieved, returned as a string, indicated by one of the following integers: 
	 *								- 0 : The name of module.
	 *								- 1 : The current version.
	 *								- 2 : The date and time when the NexSound&trade;&nbsp;library was created.
	 *								- 3 : The architecture of the NexSound&trade;&nbsp;library (ARM, x86).
	 *								- 4 : The compiler of the NexSound&trade;&nbsp;library, and whether the library is a \c debug or \c release version.  
	 *								- 5 : NexSound&trade;&nbsp;specifications.  Not currently supported.   
	 * 
	 * @return 	The NexSound&trade;&nbsp;library information requested, as a \c string, as specified by the 
	 *          parameters \c eProcessorTag and \c mode.
	 *
	 *
	 * @since version 3.0
	 */
	virtual	const char* NexSoundGetLibraryInfo(Processor::TAG eProcessorTag, int mode) PURE;	


	/**
	 * \ingroup apifunc 
	 * @brief		This function initializes the NexSound&trade;&trade;processor.
	 * 
	 * This method has to be called <b>before</b> any other \c INexSound methods.
	 * To initialize all of the NexSound&trade;&nbsp;processors, \c Processor::NexSound value should be assigned to 
	 * the \c eProcessorTag parameter except when using additional functions such as \c resampler. 
	 * To use an additional function, for example the \c resampler, it has to be initialized separately (for the resampler, \c Processor::Resampler).
	 *
	 * @param  	eProcessorTag		The tag of the processor to be initialized.
	 * @param  	nChannels			The number of channels of input PCM data.
	 * @param  	nSamplingRate		The sampling rate of input PCM data.
	 * @param  	nBitPerSample		The number of bits per sample of input PCM data.
	 * @param  	nOutSamplesPerCh	The number of output samples per channel for data being processed.
	 * @param  	nOutputMode			The output mode from the processor.  This will be one of the following possible output modes of NexSound&trade;: 
	 *									- \c NEXSOUND_OUTPUT_MODE_HEADPHONES:  Headphones or earphones mode.
	 *									- \c NEXSOUND_OUTPUT_MODE_SPEAKER:  Internal speakers (without bass woofer) mode. 
	 *									- \c NEXSOUND_OUTPUT_MODE_EXTERNAL_SPEAKER:  External speakers mode.
	 *									- \c NEXSOUND_OUTPUT_MODE_MULTICH_SPEED:  Speed and pitch control only mode.
	 *									- \c NEXSOUND_OUTPUT_MODE_MULTICH_SPEED_INTER:  Speed and pitch control only mode.  (interleaved buffer)
	 *
	 * @return 			\c NEXSOUND_SUCCESS in the case that the function has been executed successfully or one of the following error codes:
	 *						- \c NEXSOUND_INVALIDHANDLE:  An error has occurred because of an invalid handle (may be due to the use of uninitialized processor).
	 *						- \c NEXSOUND_INVALIDSIZE:  An error has occurred because of an invalid number of samples. 
	 *						- \c NEXSOUND_NOTSUPPORT:  An error has occurred because the processor tag selected is unsupported (for example, in cases where a processor is deactivated).
	 *						- \c NEXSOUND_OUTOFRANGE:  An error has occurred because the value set for a parameter is out of range.  
	 *						- \c NEXSOUND_OUTOFMEMORY:  An error has occurred because of a shortage of storage space. 
	 *						- \c NEXSOUND_INVALIDPARAM:  An error has occurred because one of the parameters is invalid.
	 *						- \c NEXSOUND_OUTOFDATE:  An error has occurred because the NexSound&trade;&nbsp;library has expired.
	 *
	 * @since version 3.0
	 */
	virtual	int			NexSoundInitialize(Processor::TAG eProcessorTag, short nChannels = 2, int nSamplingRate = 44100, short nBitPerSample = 16, unsigned short nOutSamplesPerCh = 0, short nOutputMode = NEXSOUND_OUTPUT_MODE_HEADPHONES) PURE;


	/**
	 * \ingroup apifunc 
	 * @brief	This function processes input data using the NexSound&trade;&nbsp;processor.
	 *
	 * To process data with all of the NexSound&trade;&nbsp;processors, the value \c Processor::NexSound has to be assigned to the \c eProcessorTag parameter
	 * except when using additional functions such as \c resampler.
	 * To use an additional function, for example the \c resampler, it has to be initialized separately (for the resampler, \c Processor::Resampler) with \c NexSoundInitialize().
	 *
	 * @param  	eProcessorTag		The tag of the processor where data will be processed.
	 * @param  	pIn[in]				A pointer to the buffer that stores input PCM data.  
	 * @param  	pOut[out]			A pointer to the buffer where returned PCM data should be stored after processing.
	 * @param  	nSamplePerChannel	The number of samples per channel to be processed.
	 *
	 * @return 			\c NEXSOUND_SUCCESS if the function has been executed successfully, or one of the following error codes:
	 *						- \c NEXSOUND_INVALIDHANDLE:  An error has occurred because of an invalid handle (may be due to the use of uninitialized processor).
	 *						- \c NEXSOUND_INVALIDSIZE:  An error has occurred because of an invalid number of samples. 
	 *						- \c NEXSOUND_NOTSUPPORT:  An error has occurred because the processor tag selected is unsupported (for example, in cases where a processor is deactivated).
	 *						- \c NEXSOUND_OUTOFRANGE:  An error has occurred because the value set for a parameter is out of range.  
	 *						- \c NEXSOUND_OUTOFMEMORY:  An error has occurred because of a shortage of storage space. 
	 *						- \c NEXSOUND_INVALIDPARAM:  An error has occurred because one of the parameters is invalid.
	 *						- \c NEXSOUND_OUTOFDATE:  An error has occurred because the NexSound&trade;&nbsp;library has expired.
	 *
	 * @since version 3.0
	 */
	virtual	int			NexSoundProcess(Processor::TAG eProcessorTag, short *pIn, short *pOut, unsigned int nSamplePerChannel) PURE;

	
	/**
	 * \ingroup apifunc 
	 * @brief		This function sets the values of parameters defined in the \c ParamCommand namespace, for parameters which have integer values.
	 *
	 * The parameters to be set are defined in the \c ParamCommand namespace and are identified by tags.  Each \c ParamCommand tag may or may not include a prefix.
	 * Any \c ParamCommand tag with a prefix should <em>only</em> be used with a Processor tag with the same prefix, and any \c ParamCommand tag <em>without</em>
	 * a prefix can be used with most of the Processor tags.
	 * 
	 * For example, \c ParamCommand::SpeedControl_PlaySpeed has the prefix \c SpeedControl_* so should only be used with \c Processor::SpeedControl.  
	 * On the other hand, \c ParamCommand::ProcessorStrength can be used with the \c Processor::EarComfort, \c Processor::LiveConcert, and \c Processor::StereoChorus processors 
	 * but can't be used with \c Processor::SpeedControl as this parameter is not necessary for speed control.
	 *
	 * \warning  This function should only be used to set parameters with <em>integer</em> values.  Parameters that use pointers should be set with \c NexSoundSetParam(Processor::TAG eProcessorTag, ParamCommand::TAG eCommandTag, void* pValue) instead.   
	 * 
	 * @param  	eProcessorTag		The tag of the processor where the parameter will be set.
	 * @param  	eCommandTag			The \c ParamCommand tag of the parameter to be set.
	 * @param  	nValue				The new value of the parameter to be set, as an \c integer.
	 * 
	 * @return 	\c NEXSOUND_SUCCESS if the function has been executed successfully, or one of the following error codes:
	 *						- \c NEXSOUND_INVALIDHANDLE:  An error has occurred because of an invalid handle (may be due to the use of uninitialized processor).
	 *						- \c NEXSOUND_INVALIDSIZE:  An error has occurred because of an invalid number of samples. 
	 *						- \c NEXSOUND_NOTSUPPORT:  An error has occurred because the processor tag selected is unsupported (for example, in cases where a processor is deactivated).
	 *						- \c NEXSOUND_OUTOFRANGE:  An error has occurred because the value set for a parameter is out of range.  
	 *						- \c NEXSOUND_OUTOFMEMORY:  An error has occurred because of a shortage of storage space. 
	 *						- \c NEXSOUND_INVALIDPARAM:  An error has occurred because one of the parameters is invalid.
	 *						- \c NEXSOUND_OUTOFDATE:  An error has occurred because the NexSound&trade;&nbsp;library has expired.
	 *
	 * \see ParamCommand namespace
	 *
	 * @since version 3.0
	 */
	virtual	int			NexSoundSetParam(Processor::TAG eProcessorTag, ParamCommand::TAG eCommandTag, int nValue) PURE;


	/**
	* \ingroup apifunc
	* @brief		This function sets the values of parameters defined in the \c ParamCommand namespace, for parameters which have integer values.
	*
	* The parameters to be set are defined in the \c ParamCommand namespace and are identified by tags.  Each \c ParamCommand tag may or may not include a prefix.
	* Any \c ParamCommand tag with a prefix should <em>only</em> be used with a Processor tag with the same prefix, and any \c ParamCommand tag <em>without</em>
	* a prefix can be used with most of the Processor tags.
	*
	* For example, \c ParamCommand::SpeedControl_PlaySpeed has the prefix \c SpeedControl_* so should only be used with \c Processor::SpeedControl.
	* On the other hand, \c ParamCommand::ProcessorStrength can be used with the \c Processor::EarComfort, \c Processor::LiveConcert, and \c Processor::StereoChorus processors
	* but can't be used with \c Processor::SpeedControl as this parameter is not necessary for speed control.
	*
	* \warning  This function should only be used to set parameters with <em>integer</em> values.  Parameters that use pointers should be set with \c NexSoundSetParam(Processor::TAG eProcessorTag, ParamCommand::TAG eCommandTag, void* pValue) instead.
	*
	* @param  	eProcessorTag		The tag of the processor where the parameter will be set.
	* @param  	eCommandTag			The \c ParamCommand tag of the parameter to be set.
	* @param  	fValue				The new value of the parameter to be set, as an \c float.
	*
	* @return 	\c NEXSOUND_SUCCESS if the function has been executed successfully, or one of the following error codes:
	*						- \c NEXSOUND_INVALIDHANDLE:  An error has occurred because of an invalid handle (may be due to the use of uninitialized processor).
	*						- \c NEXSOUND_INVALIDSIZE:  An error has occurred because of an invalid number of samples.
	*						- \c NEXSOUND_NOTSUPPORT:  An error has occurred because the processor tag selected is unsupported (for example, in cases where a processor is deactivated).
	*						- \c NEXSOUND_OUTOFRANGE:  An error has occurred because the value set for a parameter is out of range.
	*						- \c NEXSOUND_OUTOFMEMORY:  An error has occurred because of a shortage of storage space.
	*						- \c NEXSOUND_INVALIDPARAM:  An error has occurred because one of the parameters is invalid.
	*						- \c NEXSOUND_OUTOFDATE:  An error has occurred because the NexSound&trade;&nbsp;library has expired.
	*
	* \see ParamCommand namespace
	*
	* @since version 3.0
	*/
	virtual	int			NexSoundSetParam(Processor::TAG eProcessorTag, ParamCommand::TAG eCommandTag, float fValue) PURE;
	
	
	/**
	 * \ingroup apifunc 
	 * @brief		This function sets the values of parameters defined in the \c ParamCommand namespace which take pointer values. 
	 *
	 * The parameters to be set are defined in the \c ParamCommand namespace and are identified by tags.  Each \c ParamCommand tag may or may not include a prefix.
	 * Any \c ParamCommand tag with a prefix should <em>only</em> be used with a Processor tag with the same prefix, and any \c ParamCommand tag <em>without</em>
	 * a prefix can be used with most of the Processor tags.
	 * 
	 * For example, \c ParamCommand::SpeedControl_PlaySpeed has the prefix \c SpeedControl_* so should only be used with \c Processor::SpeedControl.  
	 * On the other hand, \c ParamCommand::ProcessorStrength can be used with the \c Processor::EarComfort, \c Processor::LiveConcert, and \c Processor::StereoChorus processors 
	 * but can't be used with \c Processor::SpeedControl as this parameter is not necessary for speed control.
	 *
	 * \warning  This function should only be used to set parameters with <em>pointer</em> values.  Parameters that use integer values should
	 *           be set with \c NexSoundSetParam(Processor::TAG eProcessorTag, ParamCommand::TAG eCommandTag, int nValue) instead.   
	 *
	 * @param  	eProcessorTag	The tag of the processor where the parameter will be set.
	 * @param  	eCommandTag		The \c ParamCommand tag of the parameter to be set.
	 * @param  	pValue[in]	A pointer to the new value of the parameter to be set. 
	 * 
	 * @return  \c NEXSOUND_SUCCESS if the function has been executed successfully, or one of the following error codes:
	 *						- \c NEXSOUND_INVALIDHANDLE:  An error has occurred because of an invalid handle (may be due to the use of uninitialized processor).
	 *						- \c NEXSOUND_INVALIDSIZE:  An error has occurred because of an invalid number of samples. 
	 *						- \c NEXSOUND_NOTSUPPORT:  An error has occurred because the processor tag selected is unsupported (for example, in cases where a processor is deactivated).
	 *						- \c NEXSOUND_OUTOFRANGE:  An error has occurred because the value set for a parameter is out of range.  
	 *						- \c NEXSOUND_OUTOFMEMORY:  An error has occurred because of a shortage of storage space. 
	 *						- \c NEXSOUND_INVALIDPARAM:  An error has occurred because one of the parameters is invalid.
	 *						- \c NEXSOUND_OUTOFDATE:  An error has occurred because the NexSound&trade;&nbsp;library has expired.
	 *						
	 * \see ParamCommand namespace
	 * 
	 * @since version 3.0
	 */
	virtual	int			NexSoundSetParam(Processor::TAG eProcessorTag, ParamCommand::TAG eCommandTag, void* pValue) PURE;


	/**
	 * \ingroup apifunc 
	 * @brief	This function gets the value of a parameter defined in the \c ParamCommand namespace.
	 *
	 * Similar to the \c NexSoundSetParam functions, the parameters to be retrieved are defined in the \c ParamCommand namespace 
	 * and are identified by tags.  Each \c ParamCommand tag may or may not include a prefix.
	 * Any \c ParamCommand tag with a prefix should <em>only</em> be used with a Processor tag with the same prefix, and 
	 * any \c ParamCommand tag <em>without</em> a prefix can be used with most of the Processor tags.
	 * 
	 * For example, \c ParamCommand::SpeedControl_PlaySpeed has the prefix \c SpeedControl_* so should only be used 
	 * with \c Processor::SpeedControl.  On the other hand, \c ParamCommand::ProcessorStrength can be used with 
	 * the \c Processor::EarComfort, \c Processor::LiveConcert, and \c Processor::StereoChorus processors but can't be used with \c Processor::SpeedControl 
	 * as this parameter is not necessary for speed control.
	 *
	 * \note  The retrieved value of the parameter is stored in the pointer of the parameter, \c pValue.
	 * 
	 * @param  	eProcessorTag	The tag of the processor that contains the parameter that should be retrieved.
	 * @param  	eCommandTag		The \c ParamCommand tag of the parameter value to be retrieved.
	 * @param  	pValue[out]	A pointer to value of the parameter to be retrieved.  
	 *
	 * @return 		\c NEXSOUND_SUCCESS if the function has been executed successfully, or one of the following error codes:
	 *						- \c NEXSOUND_INVALIDHANDLE:  An error has occurred because of an invalid handle (may be due to the use of uninitialized processor).
	 *						- \c NEXSOUND_INVALIDSIZE:  An error has occurred because of an invalid number of samples. 
	 *						- \c NEXSOUND_NOTSUPPORT:  An error has occurred because the processor tag selected is unsupported (for example, in cases where a processor is deactivated).
	 *						- \c NEXSOUND_OUTOFRANGE:  An error has occurred because the value set for a parameter is out of range.  
	 *						- \c NEXSOUND_OUTOFMEMORY:  An error has occurred because of a shortage of storage space. 
	 *						- \c NEXSOUND_INVALIDPARAM:  An error has occurred because one of the parameters is invalid.
	 *						- \c NEXSOUND_OUTOFDATE:  An error has occurred because the NexSound&trade;&nbsp;library has expired.
	 * 
	 * \see ParamCommand namespace
	 * 
	 * @since version 3.0
	 */
	virtual	int			NexSoundGetParam(Processor::TAG eProcessorTag, ParamCommand::TAG eCommandTag, void* pValue) PURE;



	/**  
	 * \ingroup apifunc 
	 * @brief		This function clears the buffer in the NexSound&trade;&nbsp;processor.
	 *  
	 * \note To clear all of the NexSound&trade;&nbsp;processor buffers, use the \c Processor::NexSound tag for 
	 *       the parameter \c eProcessorTag.
	 *
	 * @param  	eProcessorTag		The tag for the processor where the buffer should be cleared.
	 *
	 * @return 		\c NEXSOUND_SUCCESS if the function has been executed successfully, or one of the following error codes:
	 *						- \c NEXSOUND_INVALIDHANDLE:  An error has occurred because of an invalid handle (may be due to the use of uninitialized processor).
	 *						- \c NEXSOUND_INVALIDSIZE:  An error has occurred because of an invalid number of samples. 
	 *						- \c NEXSOUND_NOTSUPPORT:  An error has occurred because the processor tag selected is unsupported (for example, in cases where a processor is deactivated).
	 *						- \c NEXSOUND_OUTOFRANGE:  An error has occurred because the value set for a parameter is out of range.  
	 *						- \c NEXSOUND_OUTOFMEMORY:  An error has occurred because of a shortage of storage space. 
	 *						- \c NEXSOUND_INVALIDPARAM:  An error has occurred because one of the parameters is invalid.
	 *						- \c NEXSOUND_OUTOFDATE:  An error has occurred because the NexSound&trade;&nbsp;library has expired.
	 *
	 * \since version 3.0	
	 */
	virtual	int			NexSoundClearBuffer(Processor::TAG eProcessorTag) PURE;
	
	
	/** 
	 * \ingroup apifunc 
	 * @brief		This function releases the NexSound&trade;&nbsp;processor.
	 * 
	 * This function can be used to release one or all of the NexSound&trade;&nbsp;processors when no longer needed.
	 * If all the NexSound&trade;&nbsp;processors should be released, use the \c Processor::NexSound tag.\n  
	 * 
	 * \note  Note that to use the NexSound&trade;&nbsp;library again after calling \c NexSoundRelease(), it is necessary
	 *        to call the \c NexSoundInitialize() function again first.
	 * 
	 * @param  	eProcessorTag		The tag of the processor to be released.
	 * 
	 * @return 		\c NEXSOUND_SUCCESS if the function has been executed successfully, or one of the following error codes:
	 *						- \c NEXSOUND_INVALIDHANDLE:  An error has occurred because of an invalid handle (may be due to the use of uninitialized processor).
	 *						- \c NEXSOUND_INVALIDSIZE:  An error has occurred because of an invalid number of samples. 
	 *						- \c NEXSOUND_NOTSUPPORT:  An error has occurred because the processor tag selected is unsupported (for example, in cases where a processor is deactivated).
	 *						- \c NEXSOUND_OUTOFRANGE:  An error has occurred because the value set for a parameter is out of range.  
	 *						- \c NEXSOUND_OUTOFMEMORY:  An error has occurred because of a shortage of storage space. 
	 *						- \c NEXSOUND_INVALIDPARAM:  An error has occurred because one of the parameters is invalid.
	 *						- \c NEXSOUND_OUTOFDATE:  An error has occurred because the NexSound&trade;&nbsp;library has expired.
	 *						
	 * @since version 3.0
	 * @date   	2015/04/09
	 */
	virtual	int			NexSoundRelease(Processor::TAG eProcessorTag) PURE;
};


/** 
 * \ingroup types
 * @brief       This is a user-defined \c INexSound type.
 * \see CreateNexSound()
 * \since version 3.0
 */
typedef	INexSound		INEXSOUND;			

typedef INexSound*		LP_INEXSOUND;		///< \c INexSound Pointer Type


/** 
 * \ingroup apifunc 
 * @brief	This function creates an instance of \c INexSound which is used to call and use the NexSound&trade;&nbsp;library processors.
 * 
 * To properly use the NexSound&trade;&nbsp;library, an instance must be created using this function, and a pointer to that instance 
 * will be returned.  
 * 
 * This function must be called before any other NexSound&trade;&nbsp;functions may be called. Then the other
 * \c INexSound functions can be called using the returned pointer \c LP_INEXSOUND, which points to the instance created by this function.
 * 
 * Once processing with NexSound&trade;&nbsp;has completed, the instance should be released (call \c NexSoundRelease) and 
 * destroyed (\c DestroyNexSound).
 * 
 * @return	A pointer, \c LP_INEXSOUND, to the new instance of \c INexSound.
 *
 * \see DestroyNexSound()
 * \since version 3.0
 * @date   	2015/04/09		   
 */ 
DLLAPI LP_INEXSOUND CreateNexSound();


/** 
 * \ingroup apifunc 
 * @brief	This function destroys the \c INexSound instance indicated by the pointer passed by the \c lpNexSoundInstance parameter.
 * 
 * When all processing with the NexSound&trade;&nbsp;processors is complete, the instance created by \c CreateNexSound() should be released and then
 * destroyed with a call to this function.
 * 
 * \note  The pointer returned when an instance was created by \c CreateNexSound must be used to destroy the same instance when calling this function.
 * 
 * @param  	lpNexSoundInstance		The pointer to the \c INexSound instance to be destroyed.
 
 * @return 	None.
 * 
 * \see CreateNexSound()
 * \since version 3.0
 * @date   	2015/04/09
 */ 
DLLAPI void DestroyNexSound(LP_INEXSOUND lpNexSoundInstance);


#ifdef __cplusplus
}
#endif

#endif

