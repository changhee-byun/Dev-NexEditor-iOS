/*********************************************************************************/
/* date : 2012/01/26    By : NexStreaming Ltd.									 */
/*********************************************************************************/
#ifndef NEXSOUND_H_2039482038402
#define NEXSOUND_H_2039482038402

#if defined(__cplusplus)
	extern "C" {
#endif

typedef short          Int16;
typedef unsigned short Uint16;
typedef long           Int32; 
typedef unsigned long  Uint32;

// supported Sampling Rate
#define NEX_FS_08000   8000
#define NEX_FS_11025  11025
#define NEX_FS_12000  12000
#define NEX_FS_16000  16000
#define NEX_FS_22050  22050
#define NEX_FS_24000  24000
#define NEX_FS_32000  32000
#define NEX_FS_44100  44100
#define NEX_FS_48000  48000

#define NEX_HEADSETMODE				0
#define NEX_SPEAKERMODE				1
#define NEX_MULTICHSPEEDMODE		3
#define NEX_MULTICHSPEEDINTERMODE	4
// Mode define
#define NEX_MODE_NORMAL			0
#define NEX_MODE_EARCOMFORT		1
#define NEX_MODE_REVERB			2
#define NEX_MODE_STEREOCHORUS	3
#define NEX_MODE_MUSICENHANCE	4

#define NEX_MODE_VIRTUALCINEMA	6
#define NEX_MODE_COMPRESSOR		7
#define NEX_MODE_COMPRESSOROFF	8

// return value
#define NEXSOUND_SUCCESS		0
#define NEXSOUND_INVALIDHANDLE	-1
#define NEXSOUND_INVALIDSIZE	-2
#define NEXSOUND_NOTSUPPORT		-3
#define NEXSOUND_OUTOFRANGE		-4
#define NEXSOUND_OUTOFMEMORY	-5
#define NEXSOUND_INVALIDPARAM	-6


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// @Prototype   const char* NxSoundGetVersionInfo( int mode );
// 
// @Desc
//      This function Get Version Information
//
// @Param - mode : [in] Mode of Version
//
//		       * 0 :The name of Module
//		       * 1 :Version
//		       * 2 :Time and Date
//		       * 3 :architecture
//		       * 4 :compiler
//		       * 5 :spec
//
// @Return - String of Version
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* NxSoundGetVersionInfo( int mode );

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// @Prototype   Uint32 NxSoundInit(Uint32 nFs, Uint16 nChannel, Uint16 nOutSamplesPerCh);
// 
// @Desc
//      This function initializes the internal buffer and sets the audio information for the given content.
//		It must be called under the following conditions:
//			1.	At the beginning of processing content.
//			2.	Whenever there is a change in the audio information (for example, in the sampling rate or number of channels). 
//
// @Param - nFs : [in] Supported Sampling Rates (in Hz): 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
//		  - nChannel : [in] Supported Number of Channels : 1 or 2
//		  - nOutSamplesPerCh : [in] The number of output samples per channel
//								( 768 ~ 1152 )
//		  - nOutputMode : [in] The Output Mode
//							* NEX_HEADSETMODE : Head set Mode
//							* NEX_SPEAKERMODE : Speaker Mode
//							* NEX_MULTICHSPEEDMODE : Multi Channel Mode( Max 22 ch ) Only Speed Control is srted
//							* NEX_MULTICHSPEEDINTERMODE : Multi Channel Mode( Max 22 ch ) for Interleaved buffer, Only Speed Control is supported
//		  - nBit : [in] bit resolution
//					* 16 : 16bit
//					* 24 : 24bit
//					* 32 : 32bit
//
// @Return - The handle of the initialized NxSound module.
//		   - 0 if failed
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Uint32	NxSoundInit(Uint32 nFs, Uint16 nChannel, Uint16 nOutSamplesPerCh);
Uint32	NxSoundInit(Uint32 nFs, Uint16 nChannel, Uint16 nOutSamplesPerCh, Int16 nOutputMode, Int16 nBit);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// @Prototype   void	NxSoundRelease(Uint32 uHandle);
// 
// @Desc
//     This function releases the NxSound module.
//
// @Param - uHandle : [in] The handle of the NxSound module for the given content.
//		  
// @Return - none
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void	NxSoundRelease(Uint32 uHandle);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// @Prototype   Uint32	NxSoundProcess(Uint32 uHandle,Int16 *pIn, Int16 *pOut,Uint32 nSize);
// 
// @Desc
//      This function processes the PCM data of the given content.
//
// @Param - uHandle : [in] The handle of NxSound module where data will be processed.
//		  - pIn : [in] The pointer to the buffer of original data. The buffer is defined by NxSoundSetSpeed.
//		  - pOut : [out] The pointer to the output buffer. The buffer is defined by the Constructor.
//		  - nSize: [in]  The number of input samples to be processed.(Samples per channel)
//		  
// @Return - none
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Int32	NxSoundProcess(Uint32 uHandle,Int16 *pIn, Int16 *pOut,Uint32 nSize);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// @Prototype   NxSoundSetSpeed(Uint32 uHandle,Int16 PlaySpeed);
// 
// @Desc
//      This function adjusts the user-defined speed control information.  
//		It is called after calling InitNexASC, and whenever the user changes the playback speed.
//
// @Param - uHandle : The handle of the NxSound module where playback speed will be changed.
//		  - PlaySpeed : The change in speed to apply to the content.
//						Range of Speed Control (in %): from -75(-4x) to +100(2x), where -75 indicates a quater normal speed, and +100 produces twice normal speed.
//						Note that if NxSoundInit is called, it resets the speed control to 0% (the original speed).
//
// @Return - The number of input samples per channel.  
//           For example, when the number of output samples is 1024, and playback speed is -50%, 
//			 then the returned number of input samples will be 512.
//			 In this case, this function must be called after setting the number of input samples to 512.
//			 If a value outside of the accepted range is passed, the function will return 0.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Int32	NxSoundSetSpeed(Uint32 uHandle,Int16 PlaySpeed);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// @Prototype   Int32 NxSoundSetParam(Uint32 uHandle,Int16 NEX_Mode, Uint32 Strength, Uint32 Bass_Strength);
// 
// @Desc
//      This function sets the parameters of the modes in Audio Solutions.
//		It is called every time the user adjusts an audio setting in Audio Solutions.
//
// @Param - uHandle : The handle of the NxSound module where parameters will be set.
//		  - NEX_Mode : This is an integer indicating the Audio Solutions mode to be set.  This will be one of:
//						0:  NORMAL (default values)
//						1:  EarComfort
//						2:  Reverb
//						3:  Stereo Chorus
//						4:  Music Enhancer
//						6:  Cinema Sound
//
//		  - Strength : The Strength preset to be applied to the chosen Audio Solutions mode.  
//						This will usually be an integer between 0 and 6, but doesn¡¯t apply to the Virtual Cinema mode.
//		  - Bass_Strength : The Bass Strength preset to be applied to the chosen Audio Solutions mode.  
//						This will usually be an integer between 0 and 6, but doesn¡¯t apply to the Virtual Cinema mode.
//
// @Return - 0 when Parameter setting is successful, or -1 if an unsupported value is passed.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Int32	NxSoundSetParam(Uint32 uHandle,Int16 NEX_Mode, Uint32 Strength, Uint32 Bass_Strength);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// @Prototype   Int32 NxSoundSetAutoVolumeParam(Uint32 uHandle,Int16 bEnable, Uint32 Strength, Uint32 Release);
// 
// @Desc
//     
//
// @Param - uHandle : The handle of the NxSound module where parameters will be set.
//		  - bEnable : This will be one of:
//						0:  Disable
//						1:  Enable
//
//		  - Strength : The Strength preset to be applied to the Auto Volume.  
//						This will usually be an integer between 0 and 6
//		  - Release : The Release Time preset to be applied to the Auto Volume.  
//						This will usually be an integer between 0 and 6
//
// @Return 0 when Parameter setting is successful.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Int32	NxSoundSetAutoVolumeParam(Uint32 uHandle,Int16 bEnable, Uint32 Strength, Uint32 Release);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// @Prototype   Int32	NxSoundPreVolume(Uint32 uHandle, float fVolume);
// 
// @Desc
//     Let NxSound library knows applied volume
//
// @Param - uHandle : The handle of the NxSound module where parameters will be set.
//		  - fVolume : Set applied volume value
//						0 ~ 1:  Disable
//
// @Return 0 when Parameter setting is successful.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Int32	NxSoundPreVolume(Uint32 uHandle, float fVolume);

Uint32	NxSoundClearBuffer(Uint32 uHandle);

#ifdef __cplusplus
	}
#endif

#endif //NEXSOUND_H_2039482038402

