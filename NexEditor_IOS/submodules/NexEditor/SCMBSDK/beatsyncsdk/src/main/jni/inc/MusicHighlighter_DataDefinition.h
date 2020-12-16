#pragma once

namespace MusicHL
{	
	enum Tag
	{
		TargetDuration = 0,
		IncludingIntro,
		AutoMode,
		DurationErrorRange,
		RMSErrorRange,
		MininumClipInterval,
		CrossfadeTime,
		Forced1Bar,
		NumberOfPatternChordMatched,
		NumberOfBeatSetToChord,
		BeatDetectionAlgorithm,
		ShortTimeMode,
		ResetSourceBufferIndex,
		UseBeatDetectionOnly,
		AudioPath,
		AssetFolderPath,
		AssetIconPath,

		// getting values only.
		Bpm,
		BeatDetectionResultInfo,
		ChordInfo,	// char*

		NumberOfMusicHLResults,
		MusicHLResultProperties,

		ScoreKey,
		PatternMatchInfo,
		ResultMessage,

		// BeatSync result,
		BeatSync_ListOfBeatPos,
		BeatSync_ListOfTransitionPos,
		BeatSync_ListOfEffectPos,
		BeatSync_ListOfTemplatePos,

		BeatSync_SizeOfBeatPosList,
		BeatSync_SizeOfTransitionPosList,
		BeatSync_SizeOfEffectPosList,
		BeatSync_SizeOfTemplateList,

		BeatSync_TimeForTemplatePos,

		UserDefined, // for set position index.. 
	};
}

//////////////////////////////////////////////////////////////////////////
// return value

#define MUSIC_HIGHLIGHTER_SUCCESS						0x0
#define MUSIC_HIGHLIGHTER_NOT_ENOUGH_SOURCE				0x0001
#define MUSIC_HIGHLIGHTER_OVERFLOW_SIZE					0x0002
#define MUSIC_HIGHLIGHTER_BUSY							0x0004
#define MUSIC_HIGHLIGHTER_FAILED_TO_MAKE_RESULT			0x0008
#define MUSIC_HIGHLIGHTER_BUFFER_REMAINED				0x0010
#define MUSIC_HIGHLIGHTER_PATTERN_CHORD_NOT_FOUND		0x0020
#define MUSIC_HIGHLIGHTER_NOT_INTIALIZED				0x0040
#define MUSIC_HIGHLIGHTER_INVALID_INDEX					0x0080
#define MUSIC_HIGHLIGHTER_PREPROCESSING_INCOMPLETED		0x0100
#define MUSIC_HIGHLIGHTER_INVALID_PARM					0x0200
#define MUSIC_HIGHLIGHTER_UNMATCHED_PARAM_TYPE			0x0400
#define MUSIC_HIGHLIGHTER_FAILED_TO_FIND_BEAT			0x0800
#define MUSIC_HIGHLIGHTER_OUT_OF_DATE					0x1000
