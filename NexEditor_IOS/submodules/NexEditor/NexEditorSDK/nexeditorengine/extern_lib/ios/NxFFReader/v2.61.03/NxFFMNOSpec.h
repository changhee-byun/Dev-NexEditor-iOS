/******************************************************************************
   __________________________________________________________________________
  |                                                                          |
  |                                                                          |
  |    	  Copyrightⓒ 2002-2018 NexStreaming Corp. All rights reserved.      |
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
*		File Name			: NxFFMNOSpec.h
*		Author				: NexStreaming MiddleWare Team (mw@nexstreaming.com)
*		Description			: MNO Spec관련 내용을 넣는 헤더
*		Revision History	: 
*		Comment				: 
*
******************************************************************************/

#ifndef _NXFF_MNO_SPEC_
#define _NXFF_MNO_SPEC_

typedef enum {
	NXFF_MNO_NONE		= 0x00000000,
	NXFF_NNO_SKT_SKDV	= 0x00001000,
	NXFF_MNO_KDDI		= 0x00002000,
	NXFF_MNO_DUMMY		= 0x7FFFFFFF
}NXFF_MNO_TYPE;

// For SKT Spec

// for SKDV
typedef struct {
	NXUINT32	sktm_audio_entry_count;
	NXUINT32	*sktm_audio_TrackID;
	NXUINT32	*sktm_audio_ISO_639_language_code;
	NXUINT8		*sktm_audio_description_string_length;
	NXUINT8		**sktm_audio_description_string;
	NXUINT32	sktm_subtitle_entry_count;
	NXUINT32	*sktm_subtitle_TrackID;
	NXUINT32	*sktm_subtitle_ISO_639_language_code;
	NXUINT8		*sktm_subtitle_description_string_length;
	NXUINT8		**sktm_subtitle_description_string;
	NXUINT32	sktm_chapter_entry_count;
	NXUINT32	*sktm_chapter_TrackID;
	NXUINT32	*sktm_chapter_sample_number;
	NXUINT32	*sktm_chapter_sample_timestamp;
	NXUINT8		*sktm_chapter_description_string_length;
	NXUINT8		**sktm_chapter_description_string;
	NXUINT32	skmm_entry_count;
}NxFFMNOSKDVInfo, *pNxFFMNOSKDVInfo;

// for other MNO Spec...
// PVPD will be comes here

// Smooth Streaming 에서 tfrf 정보를 얻오오기 위한 자료구조
// NxFFR_GetExtInfo의 NXFF_EXTINFO_GET_SSTREAMING_TFRFINFO 참조
typedef struct _NxFFSSTFRFInfoElement32 {
	NXUINT32	nFragmentAT;
	NXUINT32	nFragmentD;
}NxFFSSTFRFInfoElement32, *pNxFFSSTFRFInfoElement32;

typedef struct _NxFFSSTFRFInfoElement64 {
	NXUINT64	nFragmentAT;
	NXUINT64	nFragmentD;
}NxFFSSTFRFInfoElement64, *pNxFFSSTFRFInfoElement64;

typedef struct _NxFFSSTFRFInfo {
	NXUINT8		nVersion;		
	NXUINT8		nFragmentCnt;
	NXVOID		*pFlagments; // nVsersion가 1이면 pNxFFSSTFRFInfoElement64 이고 아니면 32이다.
}NxFFSSTFRFInfo, *pNxFFSSTFRFInfo;


#if 0
// 아래 자료구조들은 더이상 사용하지 않습니다.
// 3GPP Timed Text 관련 Define
#define	NXMP4FREADER_3GPPTEXT_FACE_STYLE_BOLD			0x01
#define	NXMP4FREADER_3GPPTEXT_FACE_STYLE_ITALIC			0x02
#define	NXMP4FREADER_3GPPTEXT_FACE_STYLE_UNDERLINE	0x04
typedef struct
{
	NXUINT16	start_char;
	NXUINT16	end_char;
	NXUINT16	font_ID;
	NXUINT8	face_style_flags;
	NXUINT8	font_size;
	NXUINT8	text_color_rgba[4];
} NxFFReader3GPPTextStyleRecord;

typedef struct
{
	NXUINT16	entry_count;
	NxFFReader3GPPTextStyleRecord	*text_styles;
} NxFFReader3GPPTextStyle;

typedef struct
{
	NXUINT16	startcharoffset;
	NXUINT16	endcharoffset;
} NxFFReader3GPPTextHighlight;

typedef struct
{
	NXUINT8	highlight_color_rgba[4];
} NxFFReader3GPPTextHighlightColor;

typedef struct
{
	NXUINT32	highlight_end_time;
	NXUINT16	startcharoffset;
	NXUINT16	endcharoffset;
} NxFFReader3GPPTextKaraokeEntry;

typedef struct
{
	NXUINT32	highlight_start_time;
	NXUINT16	entry_count;
	NxFFReader3GPPTextKaraokeEntry	*entry;
} NxFFReader3GPPTextKaraoke;

typedef struct
{
	NXUINT32	scroll_delay;
} NxFFReader3GPPTextScrollDelay;

typedef struct
{
	NXUINT16	startcharoffset;
	NXUINT16	endcharoffset;
	NXUINT8	URLLength;
	NXUINT8	*URLString;
	NXUINT8	altLength;
	NXUINT8	*altString;
} NxFFReader3GPPTextHyperText;

typedef struct
{
	NXINT16			top;
	NXINT16			left;
	NXINT16			bottom;
	NXINT16			right;
} NxFFReader3GPPTextTextBox;

typedef struct
{
	NXUINT16	startcharoffset;
	NXUINT16	endcharoffset;
} NxFFReader3GPPTextBlink;

#define	NXMP4FREADER_3GPPTEXT_WRAP_FLAG_NOWRAP		0x00
#define	NXMP4FREADER_3GPPTEXT_WRAP_FLAG_SOFT_WRAP	0x01
typedef struct
{
	NXUINT8	wrap_flag;
} NxFFReader3GPPTextTextWrap;

typedef enum {
	NXMP4FREADER_3GPPTEXT_MODIFIER_STYL	=	0x7374796C,	// 'styl'
	NXMP4FREADER_3GPPTEXT_MODIFIER_HLIT	= 	0x686C6974,	// 'hlit'
	NXMP4FREADER_3GPPTEXT_MODIFIER_HCLR	= 	0x68636C72,	// 'hclr'
	NXMP4FREADER_3GPPTEXT_MODIFIER_KROK	= 	0x6B726F6B,	// 'krok'
	NXMP4FREADER_3GPPTEXT_MODIFIER_DLAY	= 	0x646C6179,	// 'dlay'
	NXMP4FREADER_3GPPTEXT_MODIFIER_HREF	= 	0x68726566,	// 'href'
	NXMP4FREADER_3GPPTEXT_MODIFIER_TBOX	= 	0x74626F78,	// 'tbox'
	NXMP4FREADER_3GPPTEXT_MODIFIER_BLNK	= 	0x626C6E6B,	// 'blnk'
	NXMP4FREADER_3GPPTEXT_MODIFIER_TWRP	= 	0x74777270	// 'twrp'
} E_NXMP4FREADER_3GPPTEXT_MODIFIER;
 
typedef struct
{
	NXUINT32	modifier_type;	// E_NXMP4FREADER_3GPPTEXT_MODIFIER
	union {
		NxFFReader3GPPTextStyle					styl;
		NxFFReader3GPPTextHighlight				hlit;
		NxFFReader3GPPTextHighlightColor		hclr;
		NxFFReader3GPPTextKaraoke				krok;
		NxFFReader3GPPTextScrollDelay			dlay;
		NxFFReader3GPPTextHyperText				href;
		NxFFReader3GPPTextTextBox				tbox;
		NxFFReader3GPPTextBlink					blnk;
		NxFFReader3GPPTextTextWrap				twrp;
	} text_modifier;
} NxFFReader3GPPTextSampleModifier;

typedef struct
{
	NXUINT16		text_length;
	NXUINT8		*text;	// text[text_length]
	NXUINT32		modifier_num;
	NxFFReader3GPPTextSampleModifier	*modifier;
} NxFFReader3GPPTextSample;

typedef struct
{
	NXUINT16	font_ID;
	NXUINT8	font_name_length;
	NXUINT8	*font;
} NxFFReader3GPPTextFontRecord;

typedef struct
{
	NXUINT32	entry_count;
	NxFFReader3GPPTextFontRecord	*font_entry;
} NxFFReader3GPPTextFontTable;

// displayFlags
#define	NXMP4FREADER_3GPPTEXT_DISPLAY_SCROLL_IN			0x00000020	/*!< the text is initially invisible, just outside the text box, and enters the box in the indicated direction, scrolling until it is in the normal position */
#define	NXMP4FREADER_3GPPTEXT_DISPLAY_SCROLL_OUT		0x00000040	/*!< the text scrolls from the normal position, in the indicated direction, until it is completely outside the text box */
#define	NXMP4FREADER_3GPPTEXT_DISPLAY_SCROLLDIR_UP		0x00000000	/*!< text is vertically scrolled up ('credits style'), entering from the bottom of the bottom and leaving towards the top */
#define	NXMP4FREADER_3GPPTEXT_DISPLAY_SCROLLDIR_LEFT	0x00000080	/*!< text is horizontally scrolled ('marquee style'), entering from the right and leaving towards the left */
#define	NXMP4FREADER_3GPPTEXT_DISPLAY_SCROLLDIR_DOWN	0x00000100	/*!< text is vertically scrolled down, entering from the top and leaving towards the bottom */
#define	NXMP4FREADER_3GPPTEXT_DISPLAY_SCROLLDIR_RIGHT	0x00000180	/*!< text is horizontally scrolled, entering from the left and leaving towards the right */
#define	NXMP4FREADER_3GPPTEXT_DISPLAY_CONT_KROK			0x00000800	/*!< ignore the starting offset and highlight all text from the beginning of the sample to the ending offset */
#define	NXMP4FREADER_3GPPTEXT_DISPLAY_WRITE_VERT		0x00020000	/*!< If vertical text is requested by the content author, characters are laid out vertically from top to bottom */

typedef struct
{
	NXUINT32	displayFlags;
	NXCHAR			horizontal_justification;
	NXCHAR			vertical_justification;
	NXUINT8	background_color_rgba[4];
	
	NxFFReader3GPPTextTextBox		default_text_box;
	NxFFReader3GPPTextStyleRecord	default_style;
	NxFFReader3GPPTextFontTable		font_table;
} NxFFReader3GPPTextInfo_Entry;

typedef struct
{
	NXUINT32	region_tx;
	NXUINT32	region_ty;
	NXUINT32	region_width;
	NXUINT32	region_height;

	NXUINT32	entry_count;
	NxFFReader3GPPTextInfo_Entry	*entry;
} NxFFReader3GPPTextInfo;

#endif

#endif //_NXFF_MNO_SPEC_

