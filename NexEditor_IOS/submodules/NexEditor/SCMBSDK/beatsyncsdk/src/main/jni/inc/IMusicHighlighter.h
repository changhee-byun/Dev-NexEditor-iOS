/**
* @file	IMusicHighlighter.h
* @date	2017/09/21
* @author	Audio Solution Team / NexStreaming Corp.
* 		Contact: as@nexstreaming.com
*
* @brief	This header file defines interface classes and functions to be used by the Music Highlighter library.
*/
#ifdef MUSIC_HIGHLIGHTER_EXPORT
#define DLLAPI __declspec(dllexport)
#elif MUSIC_HIGHLIGHTER_IMPORT
#define DLLAPI __declspec(dllimport)
#else // For android library
#define DLLAPI
#endif

#ifndef MUSIC_HIGHLIGHTER_H_170921
#define MUSIC_HIGHLIGHTER_H_170921

#if defined(__cplusplus)
extern "C" {
#endif

#include "MusicHighlighter_DataDefinition.h"
	
#define PURE	= 0

	typedef	void(*ProgressCallbackFunction)(int, void*);		// data type for progress callback function pointer

	class IMusicHighlighter
	{
	public:

		virtual	~IMusicHighlighter() {}
		virtual	int Initialize(int channels, int sampling_rate, int bits_per_sample) PURE;
		
		virtual	int SetMusicSource(char* pcm_buffer, int byte_size, bool end_of_data = false) PURE;
		virtual	int PreprocessMusicHighlighter() PURE;

		virtual int	ProcessBeatSync() PURE;
		virtual	int ProcessMusicHighlighter(float target_duration = 0, float duration_error_range = -1) PURE;

		virtual	int PrepareResult(int list_index = 0) PURE;
		virtual	int GetMusicHighlightBuffer(char* buffer, int sample_size) PURE;

		virtual	int Set(MusicHL::Tag param_tag, int	set_value) PURE;
		virtual	int Set(MusicHL::Tag param_tag, float set_value) PURE;
		virtual	int Set(MusicHL::Tag param_tag, void* set_value) PURE;

		virtual	int Get(MusicHL::Tag param_tag, int* get_value) PURE;
		virtual	int Get(MusicHL::Tag param_tag, float* get_value) PURE;
		virtual	int Get(MusicHL::Tag param_tag, void* get_value, int get_value_info = -1) PURE;

		virtual	int RegisterProgressCallbackFunction(ProgressCallbackFunction progress_callback_function, void* context) PURE;

		virtual	int Release() PURE;

		// return : number of beat lists.
		// param :
		// selection [IN] : index for selected beat list. if -1, then function returns number of beat lists.
		// name [OUT] : name of selected beat list.
		// transition_rate_level [IN] : 0 ~ 3
		// fx_rate_level [IN] : 0 ~ 3
		virtual int SelectBeat(int selection=-1, char* name=0, int transition_rate_level=0, int fx_rate_level=0) PURE;
	};	

	typedef	IMusicHighlighter*	IMusicHighlighter_Ptr;

	DLLAPI IMusicHighlighter_Ptr CreateMusicHighlighter();
	DLLAPI void DestroyMusicHighlighter(IMusicHighlighter_Ptr music_highlighter_instance);

#ifdef __cplusplus
}
#endif

#endif

