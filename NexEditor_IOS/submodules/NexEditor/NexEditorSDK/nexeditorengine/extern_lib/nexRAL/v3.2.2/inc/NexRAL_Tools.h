/*-----------------------------------------------------------------------------
File Name   :	NexRAL_Tools.h
Description :	the header file for the client of nexRAL.
------------------------------------------------------------------------------

NexStreaming Confidential Proprietary
Copyright (C) 2009 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation

Revision History was described at the bottom of this file.
-----------------------------------------------------------------------------*/

#ifndef _NEXRAL_TOOLS_DOT_H_INCLUDED_
#define _NEXRAL_TOOLS_DOT_H_INCLUDED_

#define NEXRALTOOLS_MAJOR_VERSION	1
#define NEXRALTOOLS_MINOR_VERSION	0
#define NEXRALTOOLS_PATCH_NUM		0

#include "NexSAL_Internal.h"
#include "NexRAL.h"

#define NEXRAL_TIMEDTEXT_HTMLTAG_MAX_CNT				50
#define NEXRAL_TIMEDTEXT_HTMLTAG_MAX_MULTI_CNT			10

#define NEXRAL_TIMEDTEXT_HTMLTAG_GRAPH_MAX_CNT			(NEXRAL_TIMEDTEXT_HTMLTAG_MAX_CNT + NEXRAL_TIMEDTEXT_HTMLTAG_MAX_CNT / 2)

#define NEXRAL_TIMEDTEXT_HTMLTAG_RET_OK					0x00
#define NEXRAL_TIMEDTEXT_HTMLTAG_RET_CANNT_FIND_TAG		0x00

#define NEXRAL_TIMEDTEXT_DELSTR_MAX_STR_LEN				8
#define NEXRAL_TIMEDTEXT_HTMLTAG_MAX_STR_LEN			64
#define NEXRAL_TIMEDTEXT_HTMLTAG_DETAIL_MAX_STR_LEN		16

#define NEXRAL_HTMLTAGTYPE_U							0x0001
#define NEXRAL_HTMLTAGTYPE_TT							0x0002
#define NEXRAL_HTMLTAGTYPE_STRONG						0x0004
#define NEXRAL_HTMLTAGTYPE_KBD							0x0008
#define NEXRAL_HTMLTAGTYPE_I							0x0010
#define NEXRAL_HTMLTAGTYPE_FONT_SIZE					0x0020
#define NEXRAL_HTMLTAGTYPE_FONT_COLOR					0x0040
#define NEXRAL_HTMLTAGTYPE_FONT_OTHERS					0x0080
#define NEXRAL_HTMLTAGTYPE_EM							0x0100
#define NEXRAL_HTMLTAGTYPE_CODE							0x0200
#define NEXRAL_HTMLTAGTYPE_B							0x0400

#define	NEXFONT_NAME_LENGTH				128

#define NEXFONT_EFFECT_NONE				0x00
#define NEXFONT_EFFECT_BLINK			0x01
#define NEXFONT_EFFECT_UNDERLINE		0x02

#define NEXFONT_BKMODE_TRANSPARENT		0x01
#define NEXFONT_BKMODE_OPAQUE			0x02

#define NEXRGB(r,g,b)					((unsigned int)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned int)(unsigned char)(b))<<16)))

#define NEXGetRValue(rgb)				((unsigned char)((rgb) >> 16))
#define NEXGetGValue(rgb)				((unsigned char)(((unsigned short)(rgb)) >> 8))
#define NEXGetBValue(rgb)				((unsigned char)(rgb))

typedef enum _tagNEXRAL_HTMLTAGRET_
{
	NEXRAL_HTMLTAGRET_OK = 0,
	NEXRAL_HTMLTAGRET_FAIL,
	NEXRAL_HTMLTAGRET_MEM_ERROR,
	NEXRAL_HTMLTAGRET_INVALID_PARAM,
	NEXRAL_HTMLTAGRET_WRONG_PARAM,
	NEXRAL_HTMLTAGRET_NO_TAG,
	NEXRAL_HTMLTAGRET_INVALID_TAG,
	NEXRAL_HTMLTAGRET_NOT_SUPP_TAG,
	NEXRAL_HTMLTAGRET_TOO_MANY_TAG,
	NEXRAL_HTMLTAGRET_INVALID_TEXT,
	NEXRAL_HTMLTAGRET_OPEN_TAG,
	NEXRAL_HTMLTAGRET_CLOSE_TAG,
	NEXRAL_HTMLTAGRET_SINGL_TAG,
	NEXRAL_HTMLTAGRET_CLEAR_TAG,

	NEXRAL_HTMLTAGRET_FOR4BYTESALIGN  = 0x7FFFFFFF

} NEXRAL_HTMLTAGRET;

typedef struct _tagNEXRAL_HTMLTAGPROP_
{
	unsigned int    m_uiHTMLTagType;

	unsigned long   m_ulFontSize;		// if font tag then size
	unsigned long   m_ulFontColor;		// if font tag then color RGB value

} NEXRAL_HTMLTAGPROP, *PNEXRAL_HTMLTAGPROP;

typedef struct _tagNEXRAL_HTMLTAGELEM_
{
	int                 m_nTagIdx;
	NEXRAL_HTMLTAGPROP  m_stTagProp;

	int                 m_nStartIdx;
	int                 m_nEndIdx;

} NEXRAL_HTMLTAGELEM, *PNEXRAL_HTMLTAGELEM;

typedef struct _tagNEXRAL_HTMLTAGGRAPHELEM_
{
	NEXRAL_HTMLTAGPROP  m_stTagProp;

	int                 m_nStartIdx;
	int                 m_nEndIdx;

} NEXRAL_HTMLTAGGRAPHELEM, *PNEXRAL_HTMLTAGGRAPHELEM;

typedef struct _tagNEXCOLOR_
{
	unsigned char   m_cRed;
	unsigned char   m_cGreen;
	unsigned char   m_cBlue;
	unsigned char   m_cDummy;

} NEXCOLOR, *PNEXCOLOR;

typedef enum _tagNEXFONTWEIGHT_
{
	NEXFONTWEIGHT_THIN = 100,
	NEXFONTWEIGHT_EXTRALIGHT = 200,
	NEXFONTWEIGHT_LIGHT = 300,
	NEXFONTWEIGHT_NORMAL = 400,
	NEXFONTWEIGHT_MEDIUM = 500,
	NEXFONTWEIGHT_SEMIBOLD = 600,
	NEXFONTWEIGHT_BOLD = 700,
	NEXFONTWEIGHT_EXTRABOLD = 800,
	NEXFONTWEIGHT_HEAVY = 900

} NEXFONTWEIGHT;

typedef enum _tagNEXFONTSHAPE_
{
	NEXFONTSHAPE_NONE = 0,
	NEXFONTSHAPE_ITALIC,
	NEXFONTSHAPE_TYPEWRITER,

} NEXFONTSHAPE;

typedef struct _tagNEXFONTSTYLE_
{
	unsigned int    m_uiSize;
	NEXFONTWEIGHT   m_enWeight;

	unsigned int    m_uiEffect;
	NEXFONTSHAPE    m_enShape;

	NEXCOLOR        m_stColor;
	NEXCOLOR        m_stBKColor;

	unsigned int    m_uiBKMode;

} NEXFONTSTYLE, *PNEXFONTSTYLE;

typedef struct _tagNEXTIMEDTEXTINFO_
{
	NEXFONTSTYLE    m_stFontStyle; 

	int             m_nStartIdx;
	int             m_nEndIdx;

} NEXTIMEDTEXTINFO, *PNEXTIMEDTEXTINFO;

typedef struct _tagNEXCAPTIONINFO_
{
	PNEXTIMEDTEXTINFO   *m_pparrTimedText;
	int                 m_nTimedTextCnt;

	NEXFONTSTYLE        m_stDefaultFontStyle;
	char                *m_pszCaptionString;
	unsigned int        m_uiCaptionStrLen;

} NEXCAPTIONINFO, *PNEXCAPTIONINFO;

typedef enum _tagNEX3DPIXEL_
{
	NEX3DPIXEL_RGB565 = 0,
	NEX3DPIXEL_RGB888

} NEX3DPIXEL;

typedef enum _tagNEX3DCONVTYPE_
{
	NEX3DCONV_NONE = 0,
	NEX3DCONV_RED_CYAN_FULLCOLOR,
	NEX3DCONV_RED_CYAN_OPTIMIZED,
	NEX3DCONV_AMBER_BLUE_FULLCOLOR,
	NEX3DCONV_GREEN_MAGENTA_FULLCOLOR,
	NEX3DCONV_COPY_LEFT_IMAGE,
	NEX3DCONV_COPY_RIGHT_IMAGE

} NEX3DCONVTYPE;

#ifdef __cplusplus
extern "C" {
#endif
	NEXRAL_API const char* nexRALTools_GetVersion();

	NEXRAL_API NEXRAL_HTMLTAGRET nexRALTools_PreparseString(char *pszString, 
															 int nStrLen, 
															 unsigned int *puiLineCnt);

	NEXRAL_API NEXRAL_HTMLTAGRET nexRALTools_HTMLTagParser(char* pszString, 
															int nLen, 
															PNEXRAL_HTMLTAGGRAPHELEM parrstHTMLTagGraphElem, 
															int *pnHTMLTagGraphElemCnt);

	NEXRAL_API int nexRALTools_FindNReplaceString(char* pszDest, int nDestLen, char* pszSrcStr, int nSrcStrLen, char* pszTarStr, int nTarStrLen);

	NEXRAL_API int nexRALTools_TimedTextConstruct(/*[in]*/ PNEXCAPTIONINFO *ppstNexCaptionInfo);

	NEXRAL_API int nexRALTools_TimedTextSetDefaultFontStyle(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
															 /*[in]*/ PNEXFONTSTYLE pstDefaultFontStyle);

	NEXRAL_API int nexRALTools_TimedTextSetDefaultFontStyle_Size(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																  /*[in]*/ unsigned int uiSize);

	NEXRAL_API int nexRALTools_TimedTextSetDefaultFontStyle_Weight(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																	/*[in]*/ NEXFONTWEIGHT enWeight);

	NEXRAL_API int nexRALTools_TimedTextSetDefaultFontStyle_Effect(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																	/*[in]*/ unsigned int   uiEffect);

	NEXRAL_API int nexRALTools_TimedTextSetDefaultFontStyle_Shape(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																   /*[in]*/ NEXFONTSHAPE enShape);

	NEXRAL_API int nexRALTools_TimedTextSetDefaultFontStyle_Color(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																   /*[in]*/ NEXCOLOR stColor);

	NEXRAL_API int nexRALTools_TimedTextSetDefaultFontStyle_BKColor(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																	 /*[in]*/ NEXCOLOR stBKColor);

	NEXRAL_API int nexRALTools_TimedTextSetDefaultFontStyle_BKMode(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																	/*[in]*/ unsigned int   uiBKMode);

	NEXRAL_API int nexRALTools_TimedTextGetDefaultFontStyle_Size(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																  /*[out]*/ unsigned int *puiSize);

	NEXRAL_API int nexRALTools_TimedTextGetDefaultFontStyle_Weight(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																	/*[out]*/ NEXFONTWEIGHT *penWeight);

	NEXRAL_API int nexRALTools_TimedTextGetDefaultFontStyle_Effect(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																	/*[out]*/ unsigned int *puiEffect);

	NEXRAL_API int nexRALTools_TimedTextGetDefaultFontStyle_Shape(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																   /*[out]*/ NEXFONTSHAPE  *penShape);

	NEXRAL_API int nexRALTools_TimedTextGetDefaultFontStyle_Color(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																   /*[out]*/ NEXCOLOR *pstColor);

	NEXRAL_API int nexRALTools_TimedTextGetDefaultFontStyle_BKColor(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																	 /*[out]*/ NEXCOLOR *pstBKColor);

	NEXRAL_API int nexRALTools_TimedTextGetDefaultFontStyle_BKMode(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																	/*[out]*/ unsigned int *puiBKMode);


	NEXRAL_API NEXRAL_HTMLTAGRET nexRALTools_TimedTextSetCaptionString(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo, 
																		/*[in]*/ char* pszCaptionString, 
																		/*[in]*/ int nLen,
																		/*[out]*/ unsigned int *puiLineCnt);

	NEXRAL_API NEXRAL_HTMLTAGRET nexRALTools_TimedTextGetCaptionString(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																		/*[out]*/ char **ppszCaptionString,
																		/*[out]*/ int  *pnLen);

	NEXRAL_API NEXRAL_HTMLTAGRET nexRALTools_TimedTextUpdateCaptionStrLen(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo, 
																		   /*[in]*/ int nLen);

	NEXRAL_API NEXRAL_HTMLTAGRET nexRALTools_TimedTextParseTagInfo(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo);


	NEXRAL_API NEXRAL_HTMLTAGRET nexRALTools_TimedTextGetParseResult(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
																	  /*[out]*/ char** ppszParseResult, 
																	  /*[out]*/ int *pnStrLen,
																	  /*[out]*/ int *pnTimedTextCnt);

	NEXRAL_API int nexRALTools_TimedTextGetTimedTextInfo(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo,
														  /*[out]*/ PNEXTIMEDTEXTINFO *ppstNexTimedTextInfo,
														  /*[in]*/  unsigned int  uiIndex);

	NEXRAL_API int nexRALTools_TimedTextClearTagInfo(/*[in]*/ PNEXCAPTIONINFO pstNexCaptionInfo);

	NEXRAL_API int nexRALTools_TimedTextDestruct(/*[in]*/ PNEXCAPTIONINFO *ppstNexCaptionInfo);

	NEXRAL_API void nexRALTools_ConvStereocopicTo3D(NEX3DPIXEL e3DPixel, NEX3DCONVTYPE e3DConv, void *pDest, void *pLeftSrc, void* pRightSrc, int iWidth, int iHeight, int iPitch);

#ifdef __cplusplus
}
#endif
#endif
