///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file	NexSound_DataDefine.h
/// @date	2014/12/30 12:45
/// 
/// @author	Audio Solution Team / NexStreaming Corp.
/// 		Contact: as@nexstreaming.com
/// 
/// @brief	This header file defines data types, namespaces, and enum tags necessary to use the NexSound&trade;&nbsp;library.
/// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

typedef short			Int16;
typedef unsigned short	Uint16;
typedef int				Int32;
typedef unsigned int	Uint32;

#ifndef NULL
#define NULL	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define PURE	= 0

/// @brief	Namespace which contains tags for the NexSound&trade;&nbsp;processors. 
namespace Processor
{		
	/// @brief	An enumeration of the tags for the NexSound&trade;&nsbp;processors.
	enum TAG
	{
		// bypass
		Normal = 0,		///<	Play audio normally and bypass sound processing by NexSound&trade;.

		// Sound Effects		
		EarComfort,		///<	NexEarComfort
		LiveConcert,	///<	NexLiveConcert
		StereoChorus,	///<	NexStereoChorus
		MusicEnhancer,	///<	NexMusicEnhancer
		StereoReverb,	///<	NexStereoReverb
		/**
		* \brief	NexDialogEnhancer for Headphones or Speakers.
		*
		* The appropriate DialogEnhancer mode is determined by the Output mode set with \c NexSound_OutputMode.
		* If the output mode is \c NEXSOUND_OUTPUT_MODE_HEADPHONES, NexDialogEnhancer will be prepared for earphones, but if
		* the output mode is \c NEXSOUND_OUTPUT_MODE_SPEAKER or \c NEXSOUND_OUTPUT_MODE_EXTERNAL_SPEAKER the NexDialogEnhancer
		* will be prepared for speakers.
		*/		
		DialogEnhancer,

		/**
		 * \brief	NexCinemaSurround for Headphones or Speakers.  
		 * 
		 * The appropriate CinemaSurround mode is determined by the Output mode set with \c NexSound_OutputMode.
		 * If the output mode is \c NEXSOUND_OUTPUT_MODE_HEADPHONES, NexCinemaSurround will be prepared for earphones, but if
		 * the output mode is \c NEXSOUND_OUTPUT_MODE_SPEAKER or \c NEXSOUND_OUTPUT_MODE_EXTERNAL_SPEAKER the CinemaSurround 
		 * will be prepared for speakers.
		 */
		CinemaSurround,	

		// Sound Solutions		
		MaxVolume,		///< NexMaxVolume
		SpeedControl,	///< NexSpeedControl
		PitchControl,	///< NexPitchControl
		VocalRemover,	///< NexVocalRemover
		VoiceChanger,	///< NexVoiceChanger
		ExpanderGate,	///< NexExpanderGate
		Equalizer,		///< NexEqualizer
		Sweep,
		Flanger,		
		Tremolo,
		Feedback,
		Distortion,
		LowPass,
		HighPass,

		// Additional Functions
		Resampler,		///< NexResampler

		// All Processors
		NexSound,		///< Representation of all of the NexSound&trade;&nbsp;processors.
	};
}

/// @brief	Namespace which contains tags for the parameters used by the NexSound&trade;&nbsp;processors. 
namespace ParamCommand
{
	/** @brief	An enumeration of tags for the parameters used by NexSound&trade;.
         *  Some parameters may be used with multiple NexSound&trade;&nbsp;processors, but some must only be used with 
         *  specific processors.  The prefix, for example, \c SpeedControl_ in the parameter \c SpeedControl_PlaySpeed, indicates
         *  when a parameter should be used with a particular processor, in this case the NexSound&trade;&nbsp; \c SpeedControl processor only.
         *
         *  Note also that in the case of the CinemaSurround processor, different parameters are used depending on the \c CinemaSurround mode 
         *  (headphones or speakers) in use.
         *
         * \since version 3.0
         */
	enum TAG
	{
		// Parameters used by most processors.
		ProcessorStrength,		///<	Effect strength. (0 ~ 6)
		BassStrength,			///<	Bass strength. (0 ~ 6)
		Enable,					///<	Enable/disable processor. (0 : disable, 1 : enable)
		JsonDoc,				///<	JSON document.
		JsonPath,				///<	JSON file path.		
		/** \brief NexSound&trade;&nbsp;output mode.
		* Possible modes include:
		*    - \c NEXSOUND_OUTPUT_MODE_HEADPHONES,
		*    - \c NEXSOUND_OUTPUT_MODE_SPEAKER,
		*    - \c NEXSOUND_OUTPUT_MODE_EXTERNAL_SPEAKER,
		*/
		NexSound_OutputMode,
		NexSound_PreVolumeGain,	///<	Pre-volume gain. (0 ~ 100)
		NexSound_SetMultiCh2DimArrayInput,	///< Set to \c true (1) or \c false (0). Whether or not the input buffer has a 2 dimensional array for a multi-channel source.
		NexSound_SetMultiChOutput,	///< Set to \c true (1) or \c false (0). Whether or not the output buffer is multi-channel.
		NexSound_UseOneSpeaker,	///<	Set \c true (1) or \c false (0).  Whether or not NexSound&trade;&nbsp;is to be used in only ONE speaker.		
		NexSound_Headroom,	///< Set value of headroom for all modes. ( -6 ~ 0)
		NexSound_HeadroomForHeadphones,	///< Set value of headroom for headphones mode. ( -6 ~ 0)
		NexSound_HeadroomForSpeakers,	///< Set value of headroom for speakers mode. ( -6 ~ 0)
		NexSound_HeadroomForExtSpeaker,	///< Set value of headroom for external speakers mode. ( -6 ~ 0)
		NexSound_UseJsonChainOnly,		///< Set to \c true (1) or \c false (0). Whether or not NexSound&trade;&nbsp;is using JSON file only for processing chain.
		NexSound_ConvertStereo,			///< Set to \c true (1) or \c false (0). Whether or not NexSound&trade;&nbsp;makes output channel stereo automatically. (default:true)

		SpeedControl_PlaySpeed,				///< Playback speed. (0.0625 ~ 1.0 ~ 16.0)
		SpeedControl_Input_SamplePerChannel,	///< Number of input samples per channel. Use this tag to get current number of processing samples changed by speed control.
		SpeedControl_Output_SamplePerChannel,	///< Number of output samples per channel.
		SpeedControl_Maintain_Pitch,			///< Set to \c true (1) or \c false (0). Whether the pitch should remain the same when Speed Control is ON.

		MaxVolume_Strength,				///< MaxVolume strength (0 ~ 6)
		MaxVolume_Release,				///< MaxVolume release (0 ~ 6)		
		MaxVolume_MakeupVolume,			///< Makeup volume dB (0 ~ 40)
		MaxVolume_Threshold,			///< Threshold dB (-40 ~ 0)
		MaxVolume_HighCutFreq,			///< MaxVolume High cut Frequency (optional) (20 ~ 20000)
		MaxVolume_UseExternalSpeaker,	///< Not currently used. Enables external speaker.

		PitchControl_PitchIndex,		///< Pitch index. (-12 ~ 0 ~ 12)

		VoiceChanger_VoiceType,			///< Voice type (0 ~ 4)

		DialogEnhancer_Clarity, ///< Dialog clarity (0 ~ 12)
		DialogEnhancer_Volume,	///< Dialog volume (0 ~ 6)

		StereoReverb_Direct,	/// Stereo reverb direct level (-9000 ~ 0)
		StereoReverb_Reflection,	/// Stereo reverb reflection level (-9000 ~ 1000)
		StereoReverb_Reverb,	/// Stereo reverb reverb level (-9000 ~ 2000)

		// The following parameters are used in CinemaSurround for headphones mode.
		CinemaSurroundForHeadphones_LowBoostFreq,	///< Center frequency of low frequency enhancement. (0 ~ 18)
		CinemaSurroundForHeadphones_LowBoostGain,	///< Gain of low frequency enhancement. (0 ~ 6)
		CinemaSurroundForHeadphones_HighBoost,		///< High frequency enhancement by head shadow effect. A higher value will include more high frequency sound. (0 ~ 5)
		CinemaSurroundForHeadphones_OriginalGain,	///< Gain of the original stereo signal for effect source. (0 ~ 96)
		CinemaSurroundForHeadphones_LmRGain,		///< Signal gain of the left ch minus right ch for effect source. (0 ~ 96)
		CinemaSurroundForHeadphones_LpRGain,		///< Signal gain of the left ch plus right ch for effect source. (0 ~ 96)
		CinemaSurroundForHeadphones_DirectGain,		///< Gain of the direct signal. (0 ~ 80)
		CinemaSurroundForHeadphones_EarlyGain,		///< Gain of the early reflection sound.  (0 ~ 180)
		CinemaSurroundForHeadphones_LateGain,		///< Gain of the late reflection sound.  (0 ~ 180)
		CinemaSurroundForHeadphones_LpRPostGain,	///< Signal gain of the left ch plus right ch without any effect. (0 ~ 96)
		CinemaSurroundForHeadphones_PeakFilterGain,	///< Gain of the peak filter. (0 ~ 12)
		CinemaSurroundForHeadphones_PeakFilterFreq,	///< Frequency of the peak filter. (0 ~ 4500)

		// The following parameters are used in CinemaSurround for speakers mode.
		CinemaSurroundForSpeaker_FrontGain,		///< The gain value for front speaker. (0 ~ 25)
		CinemaSurroundForSpeaker_Sp2Sp,	///< Distance index between stereo speakers used in CinemaSurround for speakers. (0 ~ 100)
		CinemaSurroundForSpeaker_Sp2Ls,	///< Distance index between center of stereo speakers and listener. Used in CinemaSurround for speakers. (0 ~ 100)
		CinemaSurroundForSpeaker_MidHigh,		///< Effect strength for mid-high frequencies. (0 ~ 10)
		CinemaSurroundForSpeaker_MidLow,		///< Effect strength for mid-low frequencies. (0 ~ 10)
		CinemaSurroundForSpeaker_MonoGain,		///< Center sound enhancement.	(0 ~ 10)
		CinemaSurroundForSpeaker_LPFBoost,		///< Gain for low frequency enhancement. (0 ~ 10)
		CinemaSurroundForSpeaker_LPFFc,			///< Center frequency for low frequency enhancement. (0 ~ 18)
		CinemaSurroundForSpeaker_EffectGain,	///< Overall effect gain. (0 ~ 25)						
		CinemaSurroundForSpeaker_OutputGain,	///< Output gain. (0 ~ 25)		

		Resampler_Input_SamplingRate,		///< Input sampling rate. (1 ~ 384000)
		Resampler_Input_Channels,			///< Input channels.	(1 ~ 6)
		Resampler_Input_NumberOfSamples,	///< Number of samples in ALL channels.  
		Resampler_Output_SamplingRate,		///< Output sampling rate. (1 ~ 384000)
		Resampler_Output_NumberOfSamples,	///< Number of samples to output after resampling.
		Resampler_Quality_Level,			///< Resampling quality level (0 ~ 3), where 0 is low quality and 3 is high quality.  Note that processing in higher quality takes more time than low quality.
		Resampler_BitPerSample,				///< Bits per sample.
		Resampler_Initialized,				///< Not used. Initialized factor. 

		ExpanderGate_Threshold,				///< Expander/Gate Threshold (dB) (-90 ~ 0)
		ExpanderGate_Ratio,					///< Expander/Gate Ratio (0 ~ 100)
		ExpanderGate_AttackTime,			///< Expander/Gate Millisecond time for attack. (10us ~ 300)
		ExpanderGate_HoldTime,				///< Expander/Gate Millisecond time for hold. (5 ~ 4000)
		ExpanderGate_ReleaseTime,			///< Expander/Gate Millisecond time for Release. (5 ~ 4000)
		ExpanderGate_Range,					///< Expander/Gate Range (dB) (-90 ~ 0)

		Equalizer_Band,						///< Equalizer band (1 ~ 9), <64Hz, 125Hz, 250Hz, 500Hz, 1kHz, 2kHz, 4kHz, 8kHz, 16kHz>		
		Equalizer_Gain,						///< Equalizer Gain (dB) (-12 ~ 12)
		Equalizer_Fc,						///< Equalizer Center Frequency (Hz) (20 ~ 22000) 
		Equalizer_Q,						///< Equalizer Q (1 ~ 20)
		Equalizer_Type,						///< Equalizer type (0: Shelving, 1: Peaking) for 1 or 9 band only.		

		NoiseReduction_NoiseGain,			///< Noise reduction gain
		NoiseReduction_Sensitivity,			///< Noise reduction Sensitivity ( 0 ~ 48.00)
		NoiseReduction_FreqSmoothing,       ///< Noise reduction Frequency Smoothing 
		NoiseReduction_HighPassFreq,		///< Noise reduction High Pass freq
		NoiseReduction_LowPassFreq,			///< Noise reduction Low Pass freq
		NoiseReduction_FilterEnable,		///< Noise reduction Filter Enable
		NoiseReduction_ChorusEnable,		///< Noise reduction Chorus Enable
		NoiseReduction_ChorusStrenght,		///< Noise reduction Chorus Enable

		NexDeHumm_CenterFreq,				///< DeHumm Center Frequency
		NexDeHumm_Q,						///< DeHumm Q
		NexDeHumm_SelectFreqSet,			///< Select Frequency set : (set[0] : 60 , 120, 180 .... : set[1] : 50, 100, 150, ....)

		NexDeEsser_CenterFreq,				///< DeEsser Center Frequency
		NexDeEsser_NotchQ,				///< DeEsser Notch Filter Q

		LowPass_Freq,						///< Frequency of the Low-pass filter	(20 - 22000 Hz)
		HighPass_Freq,						///< Frequency of the High-pass filter	(20 - 22000 Hz)

		// Any type of value.
		UseParameterStructure1,			///< Not currently used. Any type of value.
		UseParameterStructure2,			///< Not currently used. Any type of value.
		UseParameterStructure3,			///< Not currently used. Any type of value.
		UseParameterStructure4,			///< Not currently used. Any type of value.
		UseParameterStructure5,			///< Not currently used. Any type of value.
		UseParameterStructure6,			///< Not currently used. Any type of value.

		ErrorMessage,
	};
}

// Output modes for NexSound.
#define NEXSOUND_OUTPUT_MODE_HEADPHONES				0	///< Output mode for headphones or earphones.
#define NEXSOUND_OUTPUT_MODE_SPEAKER				1	///< Output mode for an internal speaker that does not have a bass woofer. 
#define NEXSOUND_OUTPUT_MODE_EXTERNAL_SPEAKER		2	///< Output mode for a typical external speaker. 

// Supported sampling rates
#define NEX_FS_08000   8000  ///< One of the sampling rates supported by NexSound&trade;..
#define NEX_FS_11025  11025  ///< One of the sampling rates supported by NexSound&trade;.
#define NEX_FS_12000  12000  ///< One of the sampling rates supported by NexSound&trade;.
#define NEX_FS_16000  16000  ///< One of the sampling rates supported by NexSound&trade;.
#define NEX_FS_22050  22050  ///< One of the sampling rates supported by NexSound&trade;.
#define NEX_FS_24000  24000  ///< One of the sampling rates supported by NexSound&trade;.
#define NEX_FS_32000  32000  ///< One of the sampling rates supported by NexSound&trade;.
#define NEX_FS_44100  44100  ///< One of the sampling rates supported by NexSound&trade;.
#define NEX_FS_48000  48000  ///< One of the sampling rates supported by NexSound&trade;.

// Possible Return values
#define NEXSOUND_SUCCESS		0	///< The function has been executed successfully.
#define NEXSOUND_INVALIDHANDLE	0x0001	///< An error has occurred because of an invalid handle (may be due to the use of an uninitialized processor).
#define NEXSOUND_INVALIDSIZE	0x0002	///< An error has occurred because of an invalid number of samples.
#define NEXSOUND_NOTSUPPORT		0x0004	///< An error has occurred because the processor tag selected is unsupported (for example, in cases where a processor is deactivated).
#define NEXSOUND_OUTOFRANGE		0x0008	///< An error has occurred because the value set for a parameter is out of range.
#define NEXSOUND_OUTOFMEMORY	0x0010	///< An error has occurred because of a shortage of storage space.
#define NEXSOUND_INVALIDPARAM	0x0020	///< An error has occurred because one of the parameters is invalid.
#define NEXSOUND_OUTOFDATE		0x0040	///< An error has occurred because the NexSound&trade;&nbsp;library has expired.
#define NEXSOUND_NOT_INITIALIZE	0x0080	///< An error has occurred because the NexSound&trade;&nbsp;library is not initialized.
