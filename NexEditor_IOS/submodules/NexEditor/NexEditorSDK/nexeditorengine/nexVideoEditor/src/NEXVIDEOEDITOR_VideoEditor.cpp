/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoEditor.cpp
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_MsgInfo.h"
#include "NEXVIDEOEDITOR_Thumbnail.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"
#include "NEXVIDEOEDITOR_DrawInfoVec.h"
#include "NexSAL_Internal.h"

#if defined(_ANDROID)
#include "porting_android .h"
#elif defined(__APPLE__)
#include "porting_ios.h"
#include "NexThemeRenderer_Platform_iOS-CAPI.h"
#endif

#include "NEXVIDEOEDITOR_GL_VideoRenderTask.h"

#define  LOG_TAG    "NEXEDITOR"
#if defined(_ANDROID)
#include <android/log.h>

#include "NexJNIEnvStack.h"

//#include <cutils/properties.h>

#ifndef LOGE
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

#endif

void print_conf(NexConf* conf);

NXT_Error FreeThemeImageCallback( NXT_ImageInfo* pinfo, void* cbdata );
NXT_Error LoadThemeImageCallback( NXT_ImageInfo* pinfo, char* path, int asyncmode, /*NXT_PixelFormat requestedColorFormat, */ void* cbdata );
NXT_Error callbackPrepareCustomLayer(	int iParam1, int iParam2, int iParam3, int iParam4, int iParam5, 
											int iParam6, int iParam7, int iParam8, int iParam9, int iParam10, 
											int iParam11, int iParam12, int iParam13, int iParam14, int iParam15);

int getLUTTextureWithID(int lut_resource_id, int export_flag){

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getLUTTextureWithID(0x%x)", __LINE__, pEditor);
	if( pEditor )
	{
		int iRet = pEditor->getLUTWithID(lut_resource_id, export_flag);
		SAFE_RELEASE(pEditor);
		return iRet;
	}
	SAFE_RELEASE(pEditor);
    return 1;
}

int getEffectImagePath(const char* input, char* output){
    
    CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
    // nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getEffectImagePath(0x%x)", __LINE__, pEditor);
    if( pEditor )
    {
        int ret = pEditor->callbackGetEffectImagePath(input, output);
        SAFE_RELEASE(pEditor);
        return ret;
    }
    SAFE_RELEASE(pEditor);
    return 1;
}

int getVignetteTexID(int export_flag){

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getVignetteTexID(0x%x)", __LINE__, pEditor);
	if( pEditor )
	{
		int iRet = pEditor->getVignetteTexID(export_flag);
		SAFE_RELEASE(pEditor);
		return iRet;
	}
	SAFE_RELEASE(pEditor);
	
	return 0;
}

int LoadThemeFileCallback(char** ppOutputData, int* pLength, char* path, void* cbdata)
{
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	// CNexVideoEditor* pEditor = (CNexVideoEditor*)cbdata;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] LoadThemeFileCallback(%s 0x%x)", __LINE__, path, pEditor);
	if( pEditor )
	{
		*pLength = 0;		

		pEditor->callbackGetThemeFile(path, strlen(path), pLength, ppOutputData);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] LoadThemeFileCallback(%d 0x%x)", __LINE__, *pLength, *ppOutputData);
		SAFE_RELEASE(pEditor);

		if(*pLength <= 0)
			return 1;
	}

	return 0;	
}

static void swapRB(unsigned char* pbuf, int width, int height){

	#if defined(ANDROID)
		for(int i = 0; i < width*height; ++i){

			unsigned char r = pbuf[i*4+0];
			pbuf[i*4+0] = pbuf[i*4+2];
			pbuf[i*4+2] = r;
		}
	#endif
}

NXT_Error LoadThemeImageCallback( NXT_ImageInfo* pinfo, char* path, int asyncmode, /*NXT_PixelFormat requestedColorFormat, */ void* cbdata )
{
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	// CNexVideoEditor* pEditor = (CNexVideoEditor*)cbdata;
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] LoadThemeImageCallback(%s 0x%x)", __LINE__, path, pEditor);
	if( pEditor )
	{
		int iWidth = 0;
		int iHeight = 0;
		int iBitForPixel = 0;
		int iImageDataSize = 0;
		unsigned char* pImageData = NULL;
		void* pUserData = NULL;
		
		pEditor->callbackGetThemeImage(path, asyncmode, strlen(path), &iWidth, &iHeight, &iBitForPixel, &pImageData, &iImageDataSize, &pUserData);

		swapRB(pImageData, iWidth, iHeight);

		pinfo->width				= iWidth;                  		// Width of image (in pixels)
		pinfo->height				= iHeight;                 		// Height of image (in pixels)
		pinfo->pitch				= iWidth;                  		// Pitch of image (in pixels)
		pinfo->pixels				= (int*)pImageData;			// Pointer to pixels
		pinfo->freeImageCallback	= FreeThemeImageCallback;	// Function to call to free pixel data
		pinfo->cbprivate0           = pUserData;

		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "NEXVIDEOEDITOR_VideoEditor.cpp %d] LoadThemeImage(%d %d 0x%x 0x%x)", __LINE__, iWidth, iHeight, pImageData, pinfo->cbprivate0);
		SAFE_RELEASE(pEditor);
	}

	return NXT_Error_None;
}

NXT_Error FreeThemeImageCallback( NXT_ImageInfo* pinfo, void* cbdata )
{
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	// CNexVideoEditor* pEditor = (CNexVideoEditor*)cbdata;
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] FreeThemeImageCallback(0x%x)", __LINE__, pEditor);
	if( pEditor && pinfo)
	{
		pEditor->callbackReleaseImage(&(pinfo->cbprivate0));
	}
	SAFE_RELEASE(pEditor);
	return NXT_Error_None;
}

NXT_Error callbackPrepareCustomLayer(	int iParam1, int iParam2, int iParam3, int iParam4, int iParam5, 
											int iParam6, int iParam7, int iParam8, int iParam9, int iParam10, 
											int iParam11, int iParam12, int iParam13, int iParam14, int iParam15, int iParam16, int iParam17, int iParam18)
{
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] callbackPrepareCustomLayer(0x%x)", __LINE__, pEditor);
	if( pEditor )
	{
	    /*
	    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "callbackPrepareCustomLayer params Dex :"
	        "p1=%d ,p2=%d ,p3=%d ,p4=%d ,p5=%d ,p6=%d ,p7=%d ,p8=%d ,p9=%d ,p10=%d ,p11=%d ,p12=%d ,p13=%d ,p14=%d ,p15=%d"
	        
	        ,iParam1,iParam2,iParam3,iParam4,iParam5,iParam6,iParam7,iParam8
	        ,iParam9,iParam10,iParam11,iParam12,iParam13,iParam14,iParam15
	        );


	    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "callbackPrepareCustomLayer params Hex :"
	        "p1=%x ,p2=%x ,p3=%x ,p4=%x ,p5=%x ,p6=%x ,p7=%x ,p8=%x ,p9=%x ,p10=%x ,p11=%x ,p12=%x ,p13=%x ,p14=%x ,p15=%x"
	        
	        ,iParam1,iParam2,iParam3,iParam4,iParam5,iParam6,iParam7,iParam8
	        ,iParam9,iParam10,iParam11,iParam12,iParam13,iParam14,iParam15
	        );
	    */

		int iRet = pEditor->callbackCustomLayer(iParam1, iParam2, iParam3, iParam4, iParam5, iParam6, iParam7, iParam8, iParam9, iParam10, iParam11, iParam12, iParam13, iParam14, iParam15, iParam16, iParam17, iParam18);
		if( iRet == 0 )
		{
			SAFE_RELEASE(pEditor);
			return NXT_Error_None;
		}
	}
	SAFE_RELEASE(pEditor);
	return NXT_Error_None;
}


#define MSM8x26_MC_SW_DEFAULT_BASELINE_LEVEL		40
#define MSM8x26_MC_SW_DEFAULT_BASELINE_LEVEL_SIZE	2097152

CNexVideoEditor*	CNexVideoEditor::m_pThisEditor			= NULL;
int				CNexVideoEditor::m_iSupportedWidth		= SUPPORTED_CONTENT_WIDTH;
int				CNexVideoEditor::m_iSupportedHeight		= SUPPORTED_CONTENT_HEIGHT;
int				CNexVideoEditor::m_iSupportedMaxMemSize	= SUPPORTED_CONTENT_WIDTH * SUPPORTED_CONTENT_HEIGHT;
int				CNexVideoEditor::m_iSupportedMinWidth	= SUPPORTED_CONTENT_MIN_WIDTH;
int				CNexVideoEditor::m_iSupportedMinHeight	= SUPPORTED_CONTENT_MIN_HEIGHT;

int				CNexVideoEditor::m_iSupportedMaxProfile	= eNEX_AVC_PROFILE_HIGH;
int				CNexVideoEditor::m_iSupportedMaxLevel	= SUPPORTED_H264_MAX_LEVEL;

int				CNexVideoEditor::m_iSupportedMaxFPS		= SUPPORTED_VIDEO_MAX_FPS;
int				CNexVideoEditor::m_iInputMaxFPS			= SUPPORTED_VIDEO_MAX_FPS;
NXBOOL				CNexVideoEditor::m_bNexEditorSDK				= FALSE;

int				CNexVideoEditor::m_iMC_HW_H264BaselineMaxLevel		= 0;
int				CNexVideoEditor::m_iMC_HW_H264MainMaxLevel			= 0;
int				CNexVideoEditor::m_iMC_HW_H264HighMaxLevel			= 0;

int				CNexVideoEditor::m_iMC_HW_H264BaselineMaxSize		= 0;
int				CNexVideoEditor::m_iMC_HW_H264MainMaxSize			= 0;
int				CNexVideoEditor::m_iMC_HW_H264HighMaxSize			= 0;	

int				CNexVideoEditor::m_iMC_SW_H264BaselineMaxLevel		= 0;
int				CNexVideoEditor::m_iMC_SW_H264MainMaxLevel			= 0;
int				CNexVideoEditor::m_iMC_SW_H264HighMaxLevel			= 0;

int				CNexVideoEditor::m_iMC_SW_H264BaselineMaxSize		= 0;
int				CNexVideoEditor::m_iMC_SW_H264MainMaxSize			= 0;
int				CNexVideoEditor::m_iMC_SW_H264HighMaxSize			= 0;	

int				CNexVideoEditor::m_iNX_SW_H264BaselineMaxLevel		= 41;
int				CNexVideoEditor::m_iNX_SW_H264MainMaxLevel			= 41;
int				CNexVideoEditor::m_iNX_SW_H264HighMaxLevel			= 41;

int				CNexVideoEditor::m_iNX_SW_H264BaselineMaxSize		= 2097152;
int				CNexVideoEditor::m_iNX_SW_H264MainMaxSize			= 2097152;
int				CNexVideoEditor::m_iNX_SW_H264HighMaxSize			= 2097152;	

int				CNexVideoEditor::m_iSupportDecoderMaxCount			= SUPPORTED_DECODE_MAXCOUNT;
int				CNexVideoEditor::m_iSupportEncoderMaxCount			= SUPPORTED_ENCODE_MAXCOUNT;
int				CNexVideoEditor::m_iUseSurfaceMediaSource			= 0;
int				CNexVideoEditor::m_isRGBADevice						= 1;
int				CNexVideoEditor::m_iSupportAACProfile 				= 0;
NXBOOL			CNexVideoEditor::m_bSupportAACSWCodec				= FALSE;

NXBOOL			CNexVideoEditor::m_bSuppoertSWH264Codec				= FALSE;
NXBOOL			CNexVideoEditor::m_bSuppoertSWMP4Codec				= FALSE;
NXBOOL			CNexVideoEditor::m_bPrepareVideoCodec				= FALSE;
NXBOOL			CNexVideoEditor::m_bSupportSWMCH264					= FALSE;
NXBOOL			CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec		= TRUE;
int   			CNexVideoEditor::m_iVideoLayerProject 				= 0;

NXBOOL          CNexVideoEditor::m_bForceDirectExport 				= FALSE; //yoon for LG
int				CNexVideoEditor::m_iDeviceMaxLightLevel 			= 550; //Hdr
float			CNexVideoEditor::m_fDeviceMaxGamma 					= 2.4; //Hdr

NXBOOL 			CNexVideoEditor::m_bHDR2SDR = 1;
int				CNexVideoEditor::m_iSupportContentDuration			= SUPPORTED_CONTENT_DURATION;
NXBOOL			CNexVideoEditor::m_bSupportPeakMeter = TRUE;

NXBOOL			CNexVideoEditor::m_bVideoDecoderSW = FALSE;

NXT_HThemeRenderer CNexVideoEditor::getRenderer(int export_flag){

	if(export_flag)
		return m_hExportRenderer;
	return m_hRenderer;
}

//---------------------------------------------------------------------------
CNexVideoEditor::CNexVideoEditor( char* strLibPath, char* strModelName, int iAPILevel, int* pProperties, void* pVM, void* wstl)
{
#if defined(_ANDROID)
	NexJNIEnvStack::setJavaVM(pVM);

#endif
	m_pThisEditor = this;
	m_pVideoEditorEventHandler	= NULL;

	m_pProjectManager			= NULL;

	m_pVoiceRecordPath		= NULL;

	m_pPCMTempBuffer		= NULL;
	m_uiPCMTempBufferSize	= 0;	

	m_ullTotalSampleCount		= 0;
	m_uiCurrentTime			= 0;

	m_pFileWriter				= NULL; // MONGTO 20130320  FOR VoiceRecorder 

	m_uiSampleRate			= 0;
	m_uiChannels				= 0;
	m_uiBitPerSample			= 0;

	m_uiAudioEncUserData		= 0;
	
	strcpy(m_strDeviceModel, strModelName);
	m_iDeviceAPILevel			= iAPILevel;
	m_iAudioMultiChannelOut = 0;
	m_bSupportFrameTimeChecker = 0;
	m_pNativeSurface			= 0;
    iVideoTrackUUIDMode = 0; //yoon
    m_i360VideoTrackPositionXAngle = 180;//yoon
    m_i360VideoTrackPositionYAngle = 0;//yoon
    m_i360VideoViewMode = 0; //0-normal , 1-360
	m_PropertyVec.clear();

	m_iBrightness = 0;
	m_iContrast = 0;
	m_iSaturation = 0;
	m_iVignette = 0;
	m_iVignetteRange = 0;
	m_iSharpness = 0;

#if 0
	{
		char pValue[64];
		sprintf(pValue, "%d", 1920*1088*2);
		setProperty("HardWareCodecMemSize", pValue);		

		sprintf(pValue, "%d", 2);
		setProperty("HardWareDecMaxCount", pValue);		

		sprintf(pValue, "%d", 1);
		setProperty("HardWareEncMaxCount", pValue);		
	}
#endif

#ifdef _ANDROID
	NexConfReader conf_reader;
	bool external = conf_reader.setNexConf(&m_Config, "/sdcard/nexeditor.conf");
	print_conf(&m_Config);

	CNexVideoEditor::m_iSupportedWidth				= m_Config.properties.video_maxwidth;
	CNexVideoEditor::m_iSupportedHeight				= m_Config.properties.video_maxheight;

	CNexVideoEditor::m_iSupportDecoderMaxCount		= m_Config.properties.hw_dec_count;
	CNexVideoEditor::m_iSupportEncoderMaxCount		= m_Config.properties.hw_enc_count;
	NXT_ThemeRenderer_SetLogLevel( m_Config.log.themerender );
	int logLevel = m_Config.log.engine;
	
	if( !external ){
    	if( pProperties ) {
    		int *p;
    		for( p = pProperties; *p; p+=2 ) {
    			int param = *(p+1);
    			switch( *p ) {
    				case 3:
    					logLevel = param;
    					break ;
    			}
    		}
    	}
    }
	m_Config.log.engine = logLevel;
	
#endif
	// NXT_ThemeRenderer_SetLogLevel( 7 );
	CNexVideoEditor::m_bSupportAACSWCodec 			= TRUE;
	bool isXiaomiMI5 = FALSE;
#ifdef _ANDROID
	isXiaomiMI5 = isXiaomiMI5C();
#endif
	m_hContext = NXT_ThemeRendererContext_Create(DEFAULT_MAX_TEXTURE_CACHE_SIZE, TRUE, isXiaomiMI5);
	m_hRenderer				= NXT_ThemeRenderer_Create(NXT_RendererType_InternalPreviewContext, pProperties, m_hContext, NULL);
#ifdef _ANDROID	
	NXT_ThemeRenderer_SetForceRTT(m_hRenderer, isSDM660Device()|isXiaomiMI5);
#elif defined(__APPLE__)
    NXT_ThemeRenderer_SetForceRTT(m_hRenderer, FALSE);	
#endif
	NXT_ThemeRenderer_RegisterImageCallbacks(m_hRenderer, LoadThemeImageCallback, FreeThemeImageCallback, LoadThemeFileCallback, (void*)this);	
	NXT_ThemeRenderer_RegisterCustomRenderCallback(m_hRenderer, callbackPrepareCustomLayer);
	NXT_ThemeRenderer_RegisterGetLutTexWithHash(m_hRenderer, getLUTTextureWithID);
	NXT_ThemeRenderer_RegisterGetEffectImagePath(m_hRenderer, getEffectImagePath);
	NXT_ThemeRenderer_RegisterGetVignetteTexID(m_hRenderer, ::getVignetteTexID);
	NXT_ThemeRenderer_SetDeviceLightLevel(m_hRenderer, CNexVideoEditor::m_iDeviceMaxLightLevel );
	NXT_ThemeRenderer_SetDeviceGamma(m_hRenderer, CNexVideoEditor::m_fDeviceMaxGamma);

	NXT_ThemeRenderer_SetBrightness(m_hRenderer, m_iBrightness);
	NXT_ThemeRenderer_SetSaturation(m_hRenderer, m_iSaturation);
	NXT_ThemeRenderer_SetContrast(m_hRenderer, m_iContrast);
	NXT_ThemeRenderer_SetVignette(m_hRenderer, m_iVignette);
	NXT_ThemeRenderer_SetVignetteRange(m_hRenderer, m_iVignetteRange);
	NXT_ThemeRenderer_SetSharpness(m_hRenderer, m_iSharpness);

#ifdef _ANDROID
	NXT_ThemeRenderer_SetJavaVM(m_hRenderer, (JavaVM *)pVM);
	NXT_ThemeRenderer_SetWrapperForSurfaceTextureListener( m_hRenderer, (jclass)wstl);
	m_hExportRenderer		= NXT_ThemeRenderer_Create(NXT_RendererType_InternalExportContext, pProperties, m_hContext, m_hRenderer);
#elif defined(__APPLE__)
	m_hExportContext = NXT_ThemeRendererContext_Create(DEFAULT_MAX_TEXTURE_CACHE_SIZE, TRUE, isXiaomiMI5);
	m_hExportRenderer		= NXT_ThemeRenderer_Create(NXT_RendererType_InternalExportContext, pProperties, m_hExportContext, m_hRenderer);
#endif
#ifdef _ANDROID
	NXT_ThemeRenderer_SetForceRTT(m_hExportRenderer, isSDM660Device()|isXiaomiMI5);
#elif defined(__APPLE__)
    NXT_ThemeRenderer_SetForceRTT(m_hRenderer, FALSE);	
#endif
	NXT_ThemeRenderer_SetExportMode(m_hExportRenderer);
	NXT_ThemeRenderer_RegisterImageCallbacks(m_hExportRenderer, LoadThemeImageCallback, FreeThemeImageCallback, LoadThemeFileCallback, (void*)this);	
	NXT_ThemeRenderer_RegisterCustomRenderCallback(m_hExportRenderer, callbackPrepareCustomLayer);
	NXT_ThemeRenderer_RegisterGetLutTexWithHash(m_hExportRenderer, getLUTTextureWithID);
	NXT_ThemeRenderer_RegisterGetEffectImagePath(m_hExportRenderer, getEffectImagePath);
	NXT_ThemeRenderer_RegisterGetVignetteTexID(m_hExportRenderer, ::getVignetteTexID);
	NXT_ThemeRenderer_SetDeviceLightLevel(m_hExportRenderer, CNexVideoEditor::m_iDeviceMaxLightLevel);
	NXT_ThemeRenderer_SetDeviceGamma(m_hExportRenderer, CNexVideoEditor::m_fDeviceMaxGamma);

	NXT_ThemeRenderer_SetBrightness(m_hExportRenderer, m_iBrightness);
	NXT_ThemeRenderer_SetSaturation(m_hExportRenderer, m_iSaturation);
	NXT_ThemeRenderer_SetContrast(m_hExportRenderer, m_iContrast);
	NXT_ThemeRenderer_SetVignette(m_hExportRenderer, m_iVignette);
	NXT_ThemeRenderer_SetVignetteRange(m_hExportRenderer, m_iVignetteRange);
	NXT_ThemeRenderer_SetSharpness(m_hExportRenderer, m_iSharpness);

#if defined(_ANDROID)
	NXT_ThemeRenderer_SetJavaVM( m_hExportRenderer, (JavaVM*)pVM );
	NXT_ThemeRenderer_SetWrapperForSurfaceTextureListener( m_hExportRenderer, (jclass)wstl);

	m_hOutputSurface = NULL;
	ANativeWindow* surface = NULL;
 	NXT_ThemeRenderer_CreateOutputSurface( m_hExportRenderer, &surface);
	if( surface )
	{
		m_hOutputSurface = (void*)surface;
	}
    registerJavaVM(pVM);//yoon
	registerSAL(m_Config.log.engine);
	initCalRalHandle();
	registerCAL(strLibPath, m_strDeviceModel, iAPILevel, m_Config.log.codec);
#elif defined(__APPLE__)
	m_hOutputSurface = NULL;
	registerSAL();
	registerCAL("libName", "modelName", 0, 0);
#endif
	loadRAL(strLibPath, m_strDeviceModel, iAPILevel, 0 /* TODO: need to make config.log.audiorenderer or something.. */);

	// RYU 20130619 
	CNexCodecManager codecManager;
	NEXCALCodecHandle hAACCodec = CNexCodecManager::getCodec( NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_AAC);
	if( hAACCodec)
	{
		NXINT64 uiResult = 0;
		nexCAL_AudioDecoderGetProperty(hAACCodec, NEXCAL_PROPERTY_AAC_SUPPORT_MAIN, &uiResult);
		if( uiResult == 0)
			CNexVideoEditor::m_iSupportAACProfile |= SUPPORT_AAC_MAIN;

		nexCAL_AudioDecoderGetProperty(hAACCodec, NEXCAL_PROPERTY_AAC_SUPPORT_LTP, &uiResult);
		if( uiResult == 0)
			CNexVideoEditor::m_iSupportAACProfile |= SUPPORT_AAC_LTP;
		
		nexCAL_AudioDecoderGetProperty(hAACCodec, NEXCAL_PROPERTY_AAC_SUPPORT_SBR, &uiResult);
		if( uiResult == 0)
			CNexVideoEditor::m_iSupportAACProfile |= SUPPORT_AAC_SBR;
		
		nexCAL_AudioDecoderGetProperty(hAACCodec, NEXCAL_PROPERTY_AAC_SUPPORT_PS, &uiResult);
		if( uiResult == 0)
			CNexVideoEditor::m_iSupportAACProfile |= SUPPORT_AAC_PS;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] possible AAC Codec 0x%x", __LINE__, m_iSupportAACProfile);

		CNexCodecManager::releaseCodec( hAACCodec );
		hAACCodec = NULL;
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] AAC Codec handle is null", __LINE__);
	}

	CNexVideoEditor::m_bSupportAACSWCodec	= CNexCodecManager::canUse_SW_AAC();
	CNexVideoEditor::m_bSuppoertSWH264Codec	= CNexCodecManager::canUse_SW_H264();
	CNexVideoEditor::m_bSuppoertSWMP4Codec	= CNexCodecManager::canUse_SW_MP4();
#ifdef _ANDROID
	CNexVideoEditor::m_bSupportSWMCH264		= isMSM8x26Device() ? CNexCodecManager::canUse_MC_SW_H264() : FALSE;
#endif

	// Move to property because if sdk it did not need.
	// If kinamaster have to call the 'checkMonoInfoFromCodec' with '1' property
	//	if( CNexVideoEditor::m_bNexEditorSDK == FALSE )
	//		CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec		= CNexCodecManager::canUseMonoInfoFromCodec();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Codec state(H:%d M:%d A:%d)", 
		__LINE__, CNexVideoEditor::m_bSuppoertSWH264Codec, CNexVideoEditor::m_bSuppoertSWMP4Codec, CNexVideoEditor::m_bSupportAACSWCodec);

	char pValue[64];
	sprintf(pValue, "%s", CNexVideoEditor::m_bSuppoertSWH264Codec ? "true" : "false");
	setProperty("canUseSoftwareCodec", pValue);
	
	sprintf(pValue, "%s", CNexVideoEditor::m_bSupportSWMCH264 ? "true" : "false");
	setProperty("canUseMCSoftwareCodec", pValue);

	sprintf(pValue, "%d", 0);
	setProperty("AudioMultiChannelOut", pValue);

	sprintf(pValue, "%d", 1);
	setProperty("SupportFrameTimeChecker", pValue);

	sprintf(pValue, "%d", 0);
	setProperty("CreationTime", pValue);

#ifdef _ANDROID
	int iBaselineLevel = 0;
	int iBaselineSize = 0;
	int iMainLevel = 0;
	int iMainSize = 0;
	int iHighLevel = 0;
	int iHighSize = 0;

	if( CNexCodecManager::prepare_MC_AVC_Level(FALSE, TRUE, &iBaselineLevel, &iBaselineSize, &iMainLevel, &iMainSize, &iHighLevel, &iHighSize) ) // HW
	{
		CNexVideoEditor::m_iMC_HW_H264BaselineMaxLevel	= iBaselineLevel;
		CNexVideoEditor::m_iMC_HW_H264MainMaxLevel		= iMainLevel;
		CNexVideoEditor::m_iMC_HW_H264HighMaxLevel		= iHighLevel;

		CNexVideoEditor::m_iMC_HW_H264BaselineMaxSize		= iBaselineSize;
		CNexVideoEditor::m_iMC_HW_H264MainMaxSize		= iMainSize;
		CNexVideoEditor::m_iMC_HW_H264HighMaxSize			= iHighSize;	

		char pValue[64];
		sprintf(pValue, "%d", iBaselineLevel);
		setProperty("MCHWAVCDecBaselineLevel", pValue);
		
		sprintf(pValue, "%d", iMainLevel);
		setProperty("MCHWAVCDecMainLevel", pValue);
		
		sprintf(pValue, "%d", iHighLevel);
		setProperty("MCHWAVCDecHighLevel", pValue);

		sprintf(pValue, "%d", iBaselineSize);
		setProperty("MCHWAVCDecBaselineLevelSize", pValue);
		
		sprintf(pValue, "%d", iMainSize);
		setProperty("MCHWAVCDecMainLevelSize", pValue);
		
		sprintf(pValue, "%d", iHighSize);
		setProperty("MCHWAVCDecHighLevelSize", pValue);		
	}

	if( CNexCodecManager::prepare_MC_AVC_Level(TRUE, TRUE, &iBaselineLevel, &iBaselineSize, &iMainLevel, &iMainSize, &iHighLevel, &iHighSize) ) // HW
	{
		char pValue[64];
		sprintf(pValue, "%d", iBaselineLevel);
		setProperty("MCHWAVCEncBaselineLevel", pValue);
		
		sprintf(pValue, "%d", iMainLevel);
		setProperty("MCHWAVCEncMainLevel", pValue);
		
		sprintf(pValue, "%d", iHighLevel);
		setProperty("MCHWAVCEncHighLevel", pValue);

		sprintf(pValue, "%d", iBaselineSize);
		setProperty("MCHWAVCEncBaselineLevelSize", pValue);
		
		sprintf(pValue, "%d", iMainSize);
		setProperty("MCHWAVCEncMainLevelSize", pValue);
		
		sprintf(pValue, "%d", iHighSize);
		setProperty("MCHWAVCEncHighLevelSize", pValue);		
	}	
	
	if( CNexVideoEditor::m_bSupportSWMCH264 && CNexCodecManager::prepare_MC_AVC_Level(FALSE, FALSE, &iBaselineLevel, &iBaselineSize, &iMainLevel, &iMainSize, &iHighLevel, &iHighSize) )	// soft
	{
		CNexVideoEditor::m_iMC_SW_H264BaselineMaxLevel	= iBaselineLevel;
		CNexVideoEditor::m_iMC_SW_H264MainMaxLevel		= 0;
		CNexVideoEditor::m_iMC_SW_H264HighMaxLevel		= 0;

		CNexVideoEditor::m_iMC_SW_H264BaselineMaxSize		= iBaselineSize;
		CNexVideoEditor::m_iMC_SW_H264MainMaxSize		= 0;
		CNexVideoEditor::m_iMC_SW_H264HighMaxSize			= 0;

		if( iBaselineLevel == 0 || iBaselineSize == 0 )
		{
			CNexVideoEditor::m_iMC_SW_H264BaselineMaxLevel	= MSM8x26_MC_SW_DEFAULT_BASELINE_LEVEL;
			CNexVideoEditor::m_iMC_SW_H264BaselineMaxSize		= MSM8x26_MC_SW_DEFAULT_BASELINE_LEVEL_SIZE;
		}
		
		char pValue[64];
		sprintf(pValue, "%d", iBaselineLevel);
		setProperty("MCSWAVCDecBaselineLevel", pValue);
		
		sprintf(pValue, "%d", iMainLevel);
		setProperty("MCSWAVCDecMainLevel", pValue);
		
		sprintf(pValue, "%d", iHighLevel);
		setProperty("MCSWAVCDecHighLevel", pValue);

		sprintf(pValue, "%d", iBaselineSize);
		setProperty("MCSWAVCDecBaselineLevelSize", pValue);
		
		sprintf(pValue, "%d", iMainSize);
		setProperty("MCSWAVCDecMainLevelSize", pValue);
		
		sprintf(pValue, "%d", iHighSize);
		setProperty("MCSWAVCDecHighLevelSize", pValue);
	}
#endif

	if( CNexVideoEditor::m_bSuppoertSWH264Codec )	// soft
	{
		char pValue[64];
		sprintf(pValue, "%d", CNexVideoEditor::m_iNX_SW_H264BaselineMaxLevel);
		setProperty("NXSWAVCDecBaselineLevel", pValue);
		
		sprintf(pValue, "%d", CNexVideoEditor::m_iNX_SW_H264MainMaxLevel);
		setProperty("NXSWAVCDecMainLevel", pValue);
		
		sprintf(pValue, "%d", CNexVideoEditor::m_iNX_SW_H264HighMaxLevel	);
		setProperty("NXSWAVCDecHighLevel", pValue);

		sprintf(pValue, "%d", CNexVideoEditor::m_iNX_SW_H264BaselineMaxSize);
		setProperty("NXSWAVCDecBaselineLevelSize", pValue);
		
		sprintf(pValue, "%d", CNexVideoEditor::m_iNX_SW_H264MainMaxSize);
		setProperty("NXSWAVCDecMainLevelSize", pValue);
		
		sprintf(pValue, "%d", CNexVideoEditor::m_iNX_SW_H264HighMaxSize);
		setProperty("NXSWAVCDecHighLevelSize", pValue);
	}	

#ifdef _ANDROID
	// for ZIRA 1929
 	if( isMSM8x26Device() )
	{
		if( CNexVideoEditor::m_bSupportSWMCH264 == FALSE )
		{
			CNexVideoEditor::m_iSupportedWidth		= CONTENT_720P_WIDTH;
			CNexVideoEditor::m_iSupportedHeight		= CONTENT_720P_HEIGHT;
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Changed support width height(%d %d)", 
				__LINE__,  CNexVideoEditor::m_iSupportedWidth, CNexVideoEditor::m_iSupportedHeight);
		}
		else
		{
			CNexVideoEditor::m_iSupportedMaxProfile	= eNEX_AVC_PROFILE_BASELINE;
			CNexVideoEditor::m_iSupportedMaxLevel	= CNexVideoEditor::m_iMC_SW_H264BaselineMaxLevel;
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Changed support profile and level(%d %d)", 
				__LINE__,  CNexVideoEditor::m_iSupportedMaxProfile, CNexVideoEditor::m_iSupportedMaxLevel);
		}
	}
#endif	
	m_pMediaCodecInputSurf = NULL;

	m_pIDRCheckItem = NULL;

#ifdef _ANDROID
	if( isMSM8974Device() || isMSM8x26Device() )
		m_bPrepareVideoCodec = TRUE;

	m_fPreviewScaleFactor = 1.0f;
	m_iThumbnailRoutine = 2;

#if defined(__ARM_ARCH_7__)
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] CNexVideoEditor 1", __LINE__);
#endif

#if defined(__aarch64__)
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] CNexVideoEditor 2", __LINE__);
#endif

#if defined(__i386__)
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] CNexVideoEditor 3", __LINE__);
#endif

#if defined(__ia64__)
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] CNexVideoEditor 4", __LINE__);
#endif
#endif
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "for error m_hRenderer:0x%p m_hExportRenderer:0x%p", m_hRenderer, m_hExportRenderer);

	m_iEncInfoCnt = 0;
	m_pEncInfo = NULL;
}

CNexVideoEditor::~CNexVideoEditor( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] ~~~CNexVideoEditor", __LINE__);

	SAFE_RELEASE(m_pIDRCheckItem);
	for (int i=0 ; i<m_iEncInfoCnt ; i++) {
		if ( m_pEncInfo[i] ) {
			delete m_pEncInfo[i];
		}
	}
	delete m_pEncInfo;

	m_pThisEditor = NULL;
	if( m_pProjectManager )
	{
		m_pProjectManager->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
		m_pProjectManager->SendSimpleCommand(MESSAGE_CLOSE_PROJECT);
		m_pProjectManager->End(1000);
		SAFE_RELEASE(m_pProjectManager);
	}

	SAFE_RELEASE(m_pVideoEditorEventHandler);

	if( m_pVoiceRecordPath )
	{
		nexSAL_MemFree(m_pVoiceRecordPath);
		m_pVoiceRecordPath = NULL;
	}

	if( m_pPCMTempBuffer )
	{
		nexSAL_MemFree(m_pPCMTempBuffer);
		m_pPCMTempBuffer = NULL;
	}

	SAFE_RELEASE(m_pFileWriter);

	m_PropertyVec.clear();

	if( m_hRenderer )
	{
		NXT_ThemeRenderer_ClearTransitionEffect(m_hRenderer);
		NXT_ThemeRenderer_ClearClipEffect(m_hRenderer);
		
		NXT_ThemeRenderer_Destroy(m_hRenderer, FALSE);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Destroy Renderer End", __LINE__);
		m_hRenderer = NULL;
	}

	if( m_hExportRenderer )
	{
#ifdef _ANDROID
		if( m_hOutputSurface )
		{
			NXT_ThemeRenderer_DestroyOutputSurface(m_hExportRenderer, (ANativeWindow*) m_hOutputSurface);
			m_hOutputSurface = NULL;
		}
#endif
		NXT_ThemeRenderer_ClearTransitionEffect(m_hExportRenderer);
		NXT_ThemeRenderer_ClearClipEffect(m_hExportRenderer);
		
		NXT_ThemeRenderer_Destroy(m_hExportRenderer, FALSE);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Destroy Export Renderer End", __LINE__);
		m_hExportRenderer = NULL;
	}

	if(m_hContext){

		NXT_ThemeRendererContext_Destroy(m_hContext);
		m_hContext = NULL;
	}
#ifdef __APPLE__
	if(m_hExportContext){
		
		NXT_ThemeRendererContext_Destroy(m_hExportContext);
		m_hExportContext = NULL;
	}
#endif
	unloadRAL();
#if defined(_ANDROID)
	unregisterCAL(CNexVideoEditor::m_bNexEditorSDK);
	unregisterSALLogforFile();
#elif defined(__APPLE__)
	unregisterCAL();
#endif	
	unregisterSAL();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] ~~~CNexVideoEditor", __LINE__);
}

CNexVideoEditor* CNexVideoEditor::getVideoEditor()
{
	SAFE_ADDREF(m_pThisEditor);
	return m_pThisEditor;
}

char* CNexVideoEditor::getDeviceModel()
{
	if( m_pThisEditor )
		return m_pThisEditor->m_strDeviceModel;
	return NULL;
}

int CNexVideoEditor::getDeviceAPILevel()
{
	if( m_pThisEditor )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getDeviceAPILevel(%d)", __LINE__, m_pThisEditor->m_iDeviceAPILevel);
		return m_pThisEditor->m_iDeviceAPILevel;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getDeviceAPILevel(%d)", __LINE__, m_pThisEditor->m_iDeviceAPILevel);
	return -1;
}

int CNexVideoEditor::getConfigLogLevel()
{
	if( m_pThisEditor )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getConfigLogLevel(%d)", __LINE__, m_pThisEditor->m_Config.log.engine);
		return m_pThisEditor->m_Config.log.engine;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getConfigLogLevel(%d)", __LINE__, m_pThisEditor->m_Config.log.engine);
	return -1;
}

NXBOOL CNexVideoEditor::existProperty(const char* pName)
{
	if( pName == NULL )
		return FALSE;

	for( int i = 0; i < m_PropertyVec.size(); i++)
	{
		if( m_PropertyVec[i].isName(pName))
		{
			return TRUE;
		}
	}
	return FALSE;
}


int CNexVideoEditor::getThemeProperty(int nPropertyName, void** ppRetValue)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getThemeProperty", __LINE__);
    NXT_ThemeRenderer_GetProperty(m_hRenderer, nPropertyName, ppRetValue);
    return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::setLayerWithRect(int x, int y, int width, int height, int scale)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setLayerWithRect", __LINE__);
#ifdef __APPLE__
    NXT_ThemeRenderer_SurfaceChanged(m_hRenderer, x, y, width, height,scale);
#endif
    return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::getProperty(const char* pName, char* pValue)
{
	if( pName == NULL || pValue == NULL )
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;

	for( int i = 0; i < m_PropertyVec.size(); i++)
	{
		if( m_PropertyVec[i].isName(pName))
		{
			strcpy(pValue, m_PropertyVec[i].getProperty(pName));
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	strcpy(pValue, "");
	return NEXVIDEOEDITOR_ERROR_GENERAL;
}

int CNexVideoEditor::getPropertyInt(const char* pName, int iDefaultValue)
{
	if( pName == NULL )
		return iDefaultValue;

	for( int i = 0; i < m_PropertyVec.size(); i++)
	{
		if( m_PropertyVec[i].isName(pName))
		{
			return atoi(m_PropertyVec[i].getProperty(pName));
		}
	}
	return iDefaultValue;
}

NXBOOL CNexVideoEditor::getPropertyBoolean(const char* pName, NXBOOL bDefaultValue)
{
	if( pName == NULL )
		return bDefaultValue;

	for( int i = 0; i < m_PropertyVec.size(); i++)
	{
		if( m_PropertyVec[i].isName(pName))
		{
			return atoi(m_PropertyVec[i].getProperty(pName)) == 0 ? FALSE : TRUE;
		}
	}
	return bDefaultValue;
}

int CNexVideoEditor::setProperty(const char* pName, const char* pValue)
{
	if( pName == NULL || pValue == NULL )
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;

#ifdef _ANDROID
	if( strcmp("DebugDumpForFile", pName) == 0 )
	{
		registerSALLogforFile(pValue);
	}
#endif	
	if( strcmp("SupportedMaxFPS", pName) == 0 )
	{
		int iFPS = atoi(pValue);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] set new supported FPS(%d)", __LINE__, iFPS);
		CNexVideoEditor::m_iSupportedMaxFPS = iFPS;
	}	
	
	if( strcmp("InputMaxFPS", pName) == 0 )
	{
		int iFPS = atoi(pValue);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] set new input max FPS(%d)", __LINE__, iFPS);
		CNexVideoEditor::m_iInputMaxFPS = iFPS;
	}
	
	if( strcmp("useNexEditorSDK", pName) == 0 )
	{
		int sdk = atoi(pValue);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] set KineMaster Mode(%d)", __LINE__, sdk);
		CNexVideoEditor::m_bNexEditorSDK = sdk == 1 ? TRUE : FALSE;
	}
	
	if( strcmp("DeviceExtendMode", pName) == 0 )
	{
		CNexVideoEditor::m_iSupportedMaxProfile	= eNEX_AVC_PROFILE_HIGH;
		CNexVideoEditor::m_iSupportedMaxLevel	= SUPPORTED_H264_MAX_LEVEL;
	}	
	
	if( strcmp("AudioMultiChannelOut", pName) == 0 )
	{
		CNexVideoEditor::m_iAudioMultiChannelOut = atoi(pValue);
	}	
	
	if( strcmp("SupportFrameTimeChecker", pName) == 0 )
	{
		CNexVideoEditor::m_bSupportFrameTimeChecker = atoi(pValue);
	}	

    //yoon for LG.
    if( strcmp("forceDirectExport", pName) == 0 )
    {
        int val = atoi(pValue);
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] set forceDirectExport(%d)", __LINE__, val);
        if( val )
            CNexVideoEditor::m_bForceDirectExport    = TRUE;
        else
            CNexVideoEditor::m_bForceDirectExport    = FALSE;
    }

    if( strcmp("DeviceMaxLightLevel", pName) == 0 ) //hdr
    {
        int val = atoi(pValue);
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] set DeviceMaxLightLevel(%d)", __LINE__, val);
        if( val )
            CNexVideoEditor::m_iDeviceMaxLightLevel    = val;
        else
            CNexVideoEditor::m_iDeviceMaxLightLevel    = 550;
        
        if (m_hExportRenderer){
			NXT_ThemeRenderer_SetDeviceLightLevel(m_hExportRenderer, CNexVideoEditor::m_iDeviceMaxLightLevel);
		}
		if (m_hRenderer){
			NXT_ThemeRenderer_SetDeviceLightLevel(m_hRenderer, CNexVideoEditor::m_iDeviceMaxLightLevel);
		}
    	
    }

    if( strcmp("DeviceMaxGamma", pName) == 0 ) //hdr
    {
        int val = atoi(pValue);
        float fval = (float)val/1000.0;
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] set DeviceMaxGamma(%f)", __LINE__, fval);
        if( val )
            CNexVideoEditor::m_fDeviceMaxGamma    = fval;
        else
            CNexVideoEditor::m_fDeviceMaxGamma    = 2.4;
        
       	if (m_hExportRenderer){
			NXT_ThemeRenderer_SetDeviceGamma(m_hExportRenderer, CNexVideoEditor::m_fDeviceMaxGamma);
		}
		if (m_hRenderer){
			NXT_ThemeRenderer_SetDeviceGamma(m_hRenderer, CNexVideoEditor::m_fDeviceMaxGamma);
		}
            	
    }

		if( strcmp("HDR2SDR", pName) == 0 ) //hdr
		{
			
			int val = atoi(pValue);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] set HDR2SDR(%d)", __LINE__, val);
			if( val )
				CNexVideoEditor::m_bHDR2SDR =  TRUE;
			else		
				CNexVideoEditor::m_bHDR2SDR =  FALSE;
		}
		
	if( strcmp("checkMonoInfoFromCodec", pName) == 0 )
	{
		int val = atoi(pValue);
		if( val == 1 ) {
			CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec		= CNexCodecManager::canUseMonoInfoFromCodec();
		}
	}

	if( strcmp("supportContentDuration", pName) == 0 )
	{
		int val = atoi(pValue);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] suppoetContentDuration(%d)", __LINE__, val);
		CNexVideoEditor::m_iSupportContentDuration = val;
	}

	if( strcmp("supportPeakMeter", pName) == 0 )
	{
		int val = atoi(pValue);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] supportPeakMeter(%d)", __LINE__, val);
		if ( val )
			CNexVideoEditor::m_bSupportPeakMeter = TRUE;
		else
			CNexVideoEditor::m_bSupportPeakMeter = FALSE;
	}

	if( strcmp("useVideoDecoderSW", pName) == 0 )
	{
		int val = atoi(pValue);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] useVideoDecoderSW(%d)", __LINE__, val);
		if ( val )
			CNexVideoEditor::m_bVideoDecoderSW = TRUE;
		else
			CNexVideoEditor::m_bVideoDecoderSW = FALSE;
	}

	for(int i = 0; i < m_PropertyVec.size(); i++)
	{
		if( m_PropertyVec[i].isName(pName) )
		{
			m_PropertyVec[i].setProperty(pValue);
			return NEXVIDEOEDITOR_ERROR_NONE; 
		}
	}
	
	CNexProperty property(pName, pValue);
	m_PropertyVec.insert(m_PropertyVec.end(), property);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

void CNexVideoEditor::commandMarker(int iTag)
{
	if( m_pProjectManager )
	{
		CNxMsgInfo* pMsg = new CNxMsgInfo(MESSAGE_CMD_MARKER);
		if( pMsg )
		{
			pMsg->m_nResult = iTag;
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
		}
	}	
}

int CNexVideoEditor::prepareSurface(void* pSurface)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] prepareSurface(0x%x)", __LINE__, pSurface);

	if( m_hRenderer == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] prepareSurface(renderer did not create:0x%x)", __LINE__, m_hRenderer);
		return 1;
	}

	m_pNativeSurface = (ANativeWindow*)pSurface;

	CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
	if( pVideoRender )
	{
		pVideoRender->SendSimpleCommand(MESSAGE_DUMMY);
		if( pVideoRender->prepareSurface(pSurface, m_fPreviewScaleFactor) == FALSE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] prepareSurface(SetNativeWindow failed)", __LINE__);
			SAFE_RELEASE(pVideoRender);
			return 1;
		}
		SAFE_RELEASE(pVideoRender);
	}
	else
	{
		int iWidth =0;
		int iHeight =0;
		int iFormat =0;

		if( pSurface != NULL)
		{
			iWidth = ANativeWindow_getWidth((ANativeWindow*)pSurface);
			iHeight = ANativeWindow_getHeight((ANativeWindow*)pSurface);
			iFormat = ANativeWindow_getFormat((ANativeWindow*)pSurface);				
		}

		if( NXT_ThemeRenderer_SetNativeWindow(m_hRenderer, m_pNativeSurface, (int)((float)iWidth * m_fPreviewScaleFactor), (int)((float)iHeight * m_fPreviewScaleFactor))  != NXT_Error_None )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] prepareSurface(SetNativeWindow failed)", __LINE__);
			return 1;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] prepareSurface(render : 0x%x surface : 0x%x)", __LINE__, m_hRenderer, pSurface);
#endif//_ANDROID
	return 0;

}

void CNexVideoEditor::prepareAudio()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] prepareAudio", __LINE__);
}

int CNexVideoEditor::setVideoPosition(int iLeft, int iTop, int iRight, int iBottom)
{
#ifdef _ANDROID
	if( m_hRenderer )
		NXT_ThemeRenderer_SurfaceChanged( m_hRenderer, iRight, iBottom );
#endif

	if( m_pProjectManager )
	{
		CNxMsgVideoRenderPos* pVideoRenderPos = new CNxMsgVideoRenderPos;
		// Default Render Info
		pVideoRenderPos->m_iLeft		= iLeft;
		pVideoRenderPos->m_iTop		= iTop;
		pVideoRenderPos->m_iRight		= iRight;
		pVideoRenderPos->m_iBottom	= iBottom;

		m_pProjectManager->SendCommand(pVideoRenderPos);
		SAFE_RELEASE(pVideoRenderPos);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::createProject(CNexExportWriter *pWriter)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] createProject(0x%x)", __LINE__, m_pProjectManager);
	if( m_pProjectManager )
	{
		m_pProjectManager->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
		m_pProjectManager->SendSimpleCommand(MESSAGE_CLOSE_PROJECT);
		m_pProjectManager->End(1000);
		SAFE_RELEASE(m_pProjectManager);
	}

	if( m_hRenderer == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] createProject(renderer did not create:0x%x)", __LINE__, m_hRenderer);
		return NEXVIDEOEDITOR_ERROR_RENDERER_INIT;
	}

	if( m_hExportRenderer == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] createProject(renderer did not create:0x%x)", __LINE__, m_hExportRenderer);
		return NEXVIDEOEDITOR_ERROR_RENDERER_INIT;
	}


	m_pProjectManager = new CNexProjectManager(this, pWriter);
	if( m_pProjectManager == NULL )
	{
		NXT_ThemeRenderer_Destroy(m_hRenderer, FALSE);
		m_hRenderer = NULL;
		NXT_ThemeRenderer_Destroy(m_hExportRenderer, FALSE);
		m_hExportRenderer = NULL;
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	if( m_pProjectManager->setThemeRenderer(m_hRenderer) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		SAFE_RELEASE(m_pProjectManager);
		NXT_ThemeRenderer_Destroy(m_hRenderer, FALSE);
		m_hRenderer = NULL;
		NXT_ThemeRenderer_Destroy(m_hExportRenderer, FALSE);
		m_hExportRenderer = NULL;
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}

	if( m_pProjectManager->setExportThemeRenderer(m_hExportRenderer) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		SAFE_RELEASE(m_pProjectManager);
		NXT_ThemeRenderer_Destroy(m_hRenderer, FALSE);
		m_hRenderer = NULL;
		NXT_ThemeRenderer_Destroy(m_hExportRenderer, FALSE);
		m_hExportRenderer = NULL;
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}	

	if( m_pProjectManager->Begin() == FALSE )
	{
		SAFE_RELEASE(m_pProjectManager);
		NXT_ThemeRenderer_Destroy(m_hRenderer, FALSE);
		m_hRenderer = NULL;
		NXT_ThemeRenderer_Destroy(m_hExportRenderer, FALSE);
		m_hExportRenderer = NULL;
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Change state idle", __LINE__);
	m_pProjectManager->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::closeProject()
{
	if( m_pProjectManager )
	{
	    m_pProjectManager->closePreExecute(); //yoon
		#if 0	
		if(m_pProjectManager->getCurrentState() == PLAY_STATE_PAUSE)
		{
			if(m_pProjectManager->getPlayEnd() == FALSE)
			{
				m_pProjectManager->SendResumeCommand(PLAY_STATE_RESUME);
		 	}
		}
		#endif
		
		m_pProjectManager->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
		m_pProjectManager->SendSimpleCommand(MESSAGE_CLOSE_PROJECT);
		m_pProjectManager->End(3000);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] closeProject Project Handle RefCount(%d)", __LINE__, m_pProjectManager->GetRefCnt());		
		SAFE_RELEASE(m_pProjectManager);
	}

	// m_pProjectManager->SendSimpleCommand(MESSAGE_CLOSE_PROJECT);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::clearProject(int iOption)
{
	if( m_pProjectManager )
	{
		if( (iOption & 0x00000001 ) == 0x00000001 )
		{
		}
		else
		{
			m_pProjectManager->SendSimpleCommand(MESSAGE_CMD_CLEAR_PROJECT);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] clearProject", __LINE__);
		}

		if( (iOption & 0x00000010 ) == 0x00000010 )
		{
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] MESSAGE_CLEAR_TEXTURE", __LINE__);
			m_pProjectManager->SendSimpleCommand(MESSAGE_CLEAR_TEXTURE);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] clearProject is failed because project manager is null", __LINE__);
	return NEXVIDEOEDITOR_ERROR_INVALID_STATE;
}

int CNexVideoEditor::setProjectEffect(char* pEffectID)
{
	if( m_pProjectManager )
	{
		CNxMsgSetProjectEffect* pMsg = new CNxMsgSetProjectEffect(pEffectID);
		if( pMsg == NULL )
		{
			return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
		}
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] clearProject is failed because project manager is null", __LINE__);
	return NEXVIDEOEDITOR_ERROR_INVALID_STATE;
}
 
int CNexVideoEditor::setProjectVolumeFade(int iFadeInTime, int iFadeOutTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setProjectVolumeFade In(%d %d)", __LINE__, iFadeInTime, iFadeOutTime);
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setProjectVolumeFade failed(reason: Project was not created)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}
	CNxMsgProjectVolumeFade* setVolumeFade = new CNxMsgProjectVolumeFade(iFadeInTime, iFadeOutTime);
	if( setVolumeFade )
	{
		m_pProjectManager->SendCommand(setVolumeFade);
		SAFE_RELEASE(setVolumeFade);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setProjectVolumeFade Out", __LINE__);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::setProjectVolume(int iProjectVolume)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setProjectVolume In(%d %d)", __LINE__, iProjectVolume);
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setProjectVolume failed(reason: Project was not created)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}
	CNxMsgProjectVolume* setProjectVolume = new CNxMsgProjectVolume(iProjectVolume);
	if( setProjectVolume )
	{
		m_pProjectManager->SendCommand(setProjectVolume);
		SAFE_RELEASE(setProjectVolume);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setProjectVolume Out", __LINE__);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::setProjectManualVolumeControl(int iManualVolumeControl)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setProjectManualVolumeControl In(%d)", __LINE__, iManualVolumeControl);
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setProjectManualVolumeControl failed(reason: Project was not created)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}
	CNxMsgSetManualVolumeControl* setManualVolumeControl = new CNxMsgSetManualVolumeControl((NXBOOL)iManualVolumeControl);
	if( setManualVolumeControl )
	{
		m_pProjectManager->SendCommand(setManualVolumeControl);
		SAFE_RELEASE(setManualVolumeControl);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setProjectManualVolumeControl Out", __LINE__);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::encodeProject(char* pEncodeFilePath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, int iMaxFileDuration, int iFPS, int iProjectWidth, int iProjectHeight, int iSamplingRate, int iAudioBitrate, int iProfile, int iLevel, int iVideoCodecType, int iFlag)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] encodeProject(%d %d %d %lld %d %d) (%d %d) (%d) (%d) (%d) (%d) (%d) (%d)",
		__LINE__, iWidth, iHeight, iBitrate, llMaxFileSize, iMaxFileDuration, iFPS, iProjectWidth, iProjectHeight, iSamplingRate, iProfile, iLevel, iVideoCodecType, iFlag);

	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}

	m_pProjectManager->SendSimpleCommand(MESSAGE_CMD_CLEAR_PROJECT);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] export clearProject", __LINE__);

	CNxMsgEncodeProject* pMsg = new CNxMsgEncodeProject(pEncodeFilePath, iWidth, iHeight, iBitrate, llMaxFileSize, iMaxFileDuration, iFPS, iSamplingRate, iAudioBitrate, iProfile, iLevel, iVideoCodecType, iFlag);
	if( pMsg == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}

#ifdef _ANDROID
	// for use thumbnail process.
	if( CNexVideoEditor::m_bNexEditorSDK )
	{
	}
	else
#endif
	{
#if defined(_ANDROID)
		if( m_hExportRenderer == NULL || m_hOutputSurface == NULL )
#elif defined(__APPLE__)
		if( m_hExportRenderer == NULL )
#endif
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] export renderer and surface did not initailize", __LINE__);
			return NEXVIDEOEDITOR_ERROR_UNKNOWN;
		}
#if defined(__APPLE__)

        NXT_ThemeRenderer_SetRenderTargetInformation( m_hExportRenderer, iWidth, iHeight );
#elif defined(_ANDROID)
		NXT_ThemeRenderer_SetNativeWindow( m_hExportRenderer, (ANativeWindow*)NULL, 0, 0);

		if( NXT_ThemeRenderer_SetNativeWindow( m_hExportRenderer, (ANativeWindow*)m_hOutputSurface, iWidth, iHeight )  != NXT_Error_None )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setNativeWindow when start export(%d %d) failed", __LINE__, iWidth, iHeight);
			return NEXVIDEOEDITOR_ERROR_UNKNOWN;
		}
#endif
	}

	//Jeff-------------------------------------------------------------------------------------------------------
	if( (iFlag & HQ_SCALE_FLAG) ==  HQ_SCALE_FLAG )
	{
#ifdef _ANDROID
		NXT_ThemeRenderer_SetHQScale(m_hExportRenderer, iProjectWidth, iProjectHeight, iWidth, iHeight);
#endif
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NexThemeRenderer NexVideoEditor_VideoEditor.cpp %d] SetHQScaler:SrcW:%d SrcH:%d DstW:%d DstH:%d.", __LINE__, 1920, 1080, iWidth, iHeight);
	}
	//-----------------------------------------------------------------------------------------------------------
	m_pProjectManager->SendCommand(pMsg);
	SAFE_RELEASE(pMsg);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::encodeProjectJpeg(void* nativeWindow, char* pEncodeFilePath, int iWidth, int iHeight, int iQuality, int iFlag)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}

	CNxMsgEncodeProjectJpeg* pMsg = new CNxMsgEncodeProjectJpeg(nativeWindow, pEncodeFilePath, iWidth, iHeight, iQuality, iFlag);
	if( pMsg == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}

	m_pProjectManager->SendCommand(pMsg);
	SAFE_RELEASE(pMsg);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::addVisualClip(unsigned int uiNextToClipID, char* pFile, int iNewClipID)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	CNxMsgAddClip* pAddClip = new CNxMsgAddClip(TRUE, uiNextToClipID, 0, pFile, iNewClipID);
	if( pAddClip == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	m_pProjectManager->SendCommand(pAddClip);
	SAFE_RELEASE(pAddClip);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::addAudioClip(unsigned int uiStartTime, char* pFile, int iNewClipID)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	CNxMsgAddClip* pAddClip = new CNxMsgAddClip(FALSE, INVALID_CLIP_ID, uiStartTime, pFile, iNewClipID);
	if( pAddClip == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	m_pProjectManager->SendCommand(pAddClip);
	SAFE_RELEASE(pAddClip);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::moveVisualClip(unsigned int uiNextToClipID, int imovedClipID, int isEventSend)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}

	CNxMsgMoveClip* pMoveClip = new CNxMsgMoveClip(uiNextToClipID, 0, imovedClipID, isEventSend);
	if( pMoveClip == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	m_pProjectManager->SendCommand(pMoveClip);
	SAFE_RELEASE(pMoveClip);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::moveAudioClip(unsigned int uiStartTime, int imovedClipID, int isEventSend)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	CNxMsgMoveClip* pMoveClip = new CNxMsgMoveClip(INVALID_CLIP_ID, uiStartTime, imovedClipID, isEventSend);
	if( pMoveClip == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	m_pProjectManager->SendCommand(pMoveClip);
	SAFE_RELEASE(pMoveClip);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::deleteClip(int iClipID)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	CNxMsgDeleteClip* pDeleteClip = new CNxMsgDeleteClip(iClipID);
	if( pDeleteClip == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	m_pProjectManager->SendCommand(pDeleteClip);
	SAFE_RELEASE(pDeleteClip);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::updateClipInfo()
{
	if( m_pProjectManager )
	{
		CNxMsgUpdateClipInfo* pMsg = new CNxMsgUpdateClipInfo();
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::updateClipInfo(IClipItem* pClip)
{
	if( m_pProjectManager )
	{
		CNxMsgUpdateClipInfo* pMsg = new CNxMsgUpdateClipInfo(pClip);
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::updateLoadList()
{
	if( m_pProjectManager )
	{
		m_pProjectManager->SendSimpleCommand(MESSAGE_UPDATE_LOADLIST);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::setBackgroundMusic(const char* pFile, int iNewClipID)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setBackgroundMusic In", __LINE__);
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setBackgroundMusic failed(reason: Project was not created)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}
	CNxMsgBackgroundMusic* setBackgroundMusic = new CNxMsgBackgroundMusic(pFile, iNewClipID);
	if( setBackgroundMusic )
	{
		m_pProjectManager->SendCommand(setBackgroundMusic);
		SAFE_RELEASE(setBackgroundMusic);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setBackgroundMusic Out", __LINE__);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::setBackgroundMusicVolume(int iVolume, int iFadeInTime, int iFadeOutTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setBackgroundMusicVolume In(%d %d %d)", __LINE__, iVolume, iFadeInTime, iFadeOutTime);
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setBackgroundMusicVolume failed(reason: Project was not created)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}
	CNxMsgBackgroundMusicVolume* setBackgroundMusicVol = new CNxMsgBackgroundMusicVolume(iVolume, iFadeInTime, iFadeOutTime);
	if( setBackgroundMusicVol )
	{
		m_pProjectManager->SendCommand(setBackgroundMusicVol);
		SAFE_RELEASE(setBackgroundMusicVol);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setBackgroundMusicVolume Out", __LINE__);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CNexVideoEditor::getDuration()
{
#if 0	
	m_pProjectManager->SendSimpleCommand(MESSAGE_GET_DURATION);
	return NEXVIDEOEDITOR_ERROR_NONE;
#else
	return m_pProjectManager ? m_pProjectManager->getDuration(): 0;
#endif
}

int CNexVideoEditor::setTime(unsigned int uiTime, int iDisplay, int iIDRFrame)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	CNxMsgSetTime* pSetTime = new CNxMsgSetTime(uiTime, iDisplay, iIDRFrame, m_hExportRenderer, m_hOutputSurface);
	if( pSetTime == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}

	m_pProjectManager->SendCommand(pSetTime);
	SAFE_RELEASE(pSetTime);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned char* CNexVideoEditor::setTimeThumbData(unsigned int uiTime)
{
	if( m_pProjectManager == NULL )
	{
		return NULL;
	}

	CClipList* pList = m_pProjectManager->getClipList();

	if( pList == NULL )
	{
		SAFE_RELEASE(pList);
		return NULL;
	}

	unsigned char* pData = NULL;
	pData = pList->getPreviewThumbData(uiTime);
	SAFE_RELEASE(pList);
	return pData;
}

int CNexVideoEditor::captureCurrentFrame()
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	m_pProjectManager->SendSimpleCommand(MESSAGE_CAPTURE);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::startPlay(int iMuteAudio)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	NEXVIDEOEDITOR_MESSAGE cmd = iMuteAudio == 1 ? MESSAGE_AUDIO_OFF : MESSAGE_AUDIO_ON;
	m_pProjectManager->SendSimpleCommand(cmd);
	m_pProjectManager->SendSimpleStateChangeCommand(PLAY_STATE_RUN);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::pausePlay()
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	m_pProjectManager->SendSimpleCommand(MESSAGE_PAUSE_RENDERER);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::resumePlay()
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	m_pProjectManager->SendSimpleCommand(MESSAGE_RESUME_RENDERER);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::stopPlay(int iFlag)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	#if 0    
	if(m_pProjectManager->getCurrentState() == PLAY_STATE_PAUSE)
	{
		if(m_pProjectManager->getPlayEnd() == FALSE)
		{
			m_pProjectManager->SendResumeCommand(PLAY_STATE_RESUME);
	 	}
	}
	#endif    
	
	CNxMsgChangeState* pMsg = new CNxMsgChangeState(PLAY_STATE_IDLE, iFlag);
	m_pProjectManager->SendCommand(pMsg);
	SAFE_RELEASE(pMsg);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::pauseExport()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] pauseExport %d", __LINE__, m_bSupportFrameTimeChecker);

	if(!m_bSupportFrameTimeChecker)
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;
	m_pProjectManager->SendPauseCommand(PLAY_STATE_PAUSE);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::resumeExport()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] resumeExport %d", __LINE__, m_bSupportFrameTimeChecker);
	if(!m_bSupportFrameTimeChecker)
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;
	m_pProjectManager->SendResumeCommand(PLAY_STATE_RESUME);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

IClipList* CNexVideoEditor::getClipList()
{
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipList failed(reason: Project was not created)", __LINE__);
		return NULL;
	}
	return (IClipList*)m_pProjectManager->getClipList();
}

IClipList* CNexVideoEditor::createClipList()
{
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] createClipList failed(reason: Project was not created)", __LINE__);
		return NULL;
	}
	CClipList* pNewClipList = new CClipList;
	if( pNewClipList == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] createClipList failed(reason: ClipList create failed)", __LINE__);
		return NULL;
	}
	
	return (IClipList*)pNewClipList;
}

int CNexVideoEditor::updateClipList(IClipList* pCliplist, int iOption)
{
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] updateClipList failed(reason: Project was not created)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_INVALID_INFO;
	}
	
	CNxMsgAsyncUpdateCliplist* pMsg = new CNxMsgAsyncUpdateCliplist(pCliplist, iOption);
	if( pMsg == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] updateClipList failed(reason: msg create failed)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
	}

	m_pProjectManager->SendCommand(pMsg);
	SAFE_RELEASE(pMsg);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] updateClipList End(%d)", __LINE__, iOption);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

IEffectItem* CNexVideoEditor::createEffectItem()
{
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] createEffectItem failed(reason: Project was not created)", __LINE__);
		return NULL;
	}
	CVideoEffectItem* pNewEffectItem = new CVideoEffectItem;
	if( pNewEffectItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] createEffectItem failed(reason: EffectItem create failed)", __LINE__);
		return NULL;
	}
	
	return (IEffectItem*)pNewEffectItem;
}

int CNexVideoEditor::updateEffectList(IClipList* pCliplist, int iOption)
{
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] updateClipList failed(reason: Project was not created)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_INVALID_INFO;
	}
	
	CNxMsgAsyncUpdateEffectlist* pMsg = new CNxMsgAsyncUpdateEffectlist(pCliplist, iOption);
	if( pMsg == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] updateClipList failed(reason: msg create failed)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
	}

	m_pProjectManager->SendCommand(pMsg);
	SAFE_RELEASE(pMsg);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] updateClipList End(%d)", __LINE__, iOption);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::loadTheme(const char* pThemeResource, int iFlags)
{
	if( pThemeResource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] loadTheme Resource invalid parameter", __LINE__);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] loadTheme Flags(0x%x)", __LINE__, iFlags);

#if 0	
	if(iFlags == 0x00000001)// Preview
	{
		// RYU 20130529 directly pass themeResource to renderer insead of seteffect etc.
		NXT_ThemeRenderer_LoadThemesAndEffects(m_hRenderer, pThemeResource, iFlags);
	}
	else
	{
		NXT_ThemeRenderer_LoadThemesAndEffects(m_hExportRenderer, pThemeResource, iFlags);
	}
#else
	if( m_pProjectManager )
	{
		CNxMsgLoadThemeAndEffect* pMsg = new CNxMsgLoadThemeAndEffect(pThemeResource, iFlags);
		if( pMsg )
		{
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
		}
	}
#endif
	return NEXVIDEOEDITOR_ERROR_NONE;
}

char* CNexVideoEditor::getLoadThemeError(int iFlags)
{
	if(iFlags == 0x00000001)// Preview
	{
	}
	else
	{
	}
	return (char*)"test error code";
}

int CNexVideoEditor::clearRenderItems(int iFlags)
{
	if( m_pProjectManager )
	{
		CNxMsgClearRenderItems* pMsg = new CNxMsgClearRenderItems(iFlags);
		if( pMsg )
		{
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
		}
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::loadRenderItem(const char* pID, const char* pThemeResource, int iFlags)
{
	if( pThemeResource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] loadTheme Resource invalid parameter", __LINE__);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] loadTheme Flags(0x%x)", __LINE__, iFlags);

#if 0	
	if(iFlags == 0x00000001)// Preview
	{
		// RYU 20130529 directly pass themeResource to renderer insead of seteffect etc.
		NXT_ThemeRenderer_LoadThemesAndEffects(m_hRenderer, pThemeResource, iFlags);
	}
	else
	{
		NXT_ThemeRenderer_LoadThemesAndEffects(m_hExportRenderer, pThemeResource, iFlags);
	}
#else
	if( m_pProjectManager )
	{
		CNxMsgLoadRenderItem* pMsg = new CNxMsgLoadRenderItem(pID, pThemeResource, iFlags);
		if( pMsg )
		{
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
		}
	}
#endif
	return NEXVIDEOEDITOR_ERROR_NONE;
}

char* CNexVideoEditor::getLoadRenderItemError(int iFlags)
{
	if(iFlags == 0x00000001)// Preview
	{
	}
	else
	{
	}
	return (char*)"test error code";
}

char* CNexVideoEditor::getDefaultClipEffect()
{
	// RYU 20130530 removed routine.
	return (char*)THEME_RENDERER_DEFAULT_EFFECT;
}

char* CNexVideoEditor::getDefaultTitleEffect()
{
	return (char*)THEME_RENDERER_DEFAULT_TITLE_EFFECT;
}

int CNexVideoEditor::setEventHandler(INexVideoEditorEventHandler* pEventHandler)
{
	if( pEventHandler == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	SAFE_RELEASE(m_pVideoEditorEventHandler);
	pEventHandler->AddRef();
	m_pVideoEditorEventHandler = pEventHandler;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

IClipInfo* CNexVideoEditor::getClipInfo(char* pFilePath, char* pThumbFilePath, int isVideoThumbnail, int isAudioPCMLevel, int isBackground, int iUserTag)
{
	if( pFilePath == NULL || pThumbFilePath == NULL || m_pProjectManager == NULL)
		return NULL;

	unsigned int uiStart = nexSAL_GetTickCount();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipInfo Start(%s) Thumb(%s) option(%d %d %d)", 
		__LINE__, pFilePath, pThumbFilePath, isVideoThumbnail, isAudioPCMLevel, isBackground);

	if( isBackground )
	{
		CNxMsgBackgroundGetInfo* pMsg = new CNxMsgBackgroundGetInfo(pFilePath, pThumbFilePath, isVideoThumbnail, isAudioPCMLevel, NULL,NULL, iUserTag);
		if( pMsg )
		{
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipInfo Send Command End", 
				__LINE__, pFilePath, isVideoThumbnail, isAudioPCMLevel, isBackground);
		}
		return NULL;
	}
	
	NEXSALFileHandle	pFile = nexSAL_FileOpen(pThumbFilePath, (NEXSALFileMode)(NEXSAL_FILE_READ));
	if( pFile != NEXSAL_INVALID_HANDLE )
	{
		int nRead = 0, nSeek = 0;
		THUMBNAIL thumbnail;
		memset(&thumbnail, 0x00, sizeof(THUMBNAIL));
		nSeek = nexSAL_FileSeek(pFile, -(sizeof(THUMBNAIL)), NEXSAL_SEEK_END);
		nRead = nexSAL_FileRead(pFile, &thumbnail, sizeof(THUMBNAIL));
		nexSAL_FileClose(pFile);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Thumbnail read from thumbnail file(%d %d)", __LINE__, nSeek, nRead);
		if( nRead == sizeof(THUMBNAIL))
		{
			CClipInfo* pClipInfo = new CClipInfo;
			if( pClipInfo == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Create clipInfo failed", __LINE__);
				return NULL;
			}

			thumbnail.m_iDuration				= makeBigendian( (unsigned int)thumbnail.m_iDuration) ;
			thumbnail.m_isVideo				= makeBigendian( (unsigned int)thumbnail.m_isVideo) ;
			thumbnail.m_isAudio				= makeBigendian( (unsigned int)thumbnail.m_isAudio) ;
			thumbnail.m_iVideoWidth			= makeBigendian( (unsigned int)thumbnail.m_iVideoWidth) ;
			thumbnail.m_iVideoHeight			= makeBigendian( (unsigned int)thumbnail.m_iVideoHeight) ;
			thumbnail.m_iThumbnailWidth		= makeBigendian( (unsigned int)thumbnail.m_iThumbnailWidth);
			thumbnail.m_iThumbnailHeight		= makeBigendian( (unsigned int)thumbnail.m_iThumbnailHeight);

			thumbnail.m_iSeekTablePos			= makeBigendian( (unsigned int)thumbnail.m_iSeekTablePos);
			thumbnail.m_iSeekTableCount		= makeBigendian( (unsigned int)thumbnail.m_iSeekTableCount);
			thumbnail.m_iPCMLevelPos			= makeBigendian( (unsigned int)thumbnail.m_iPCMLevelPos);
			thumbnail.m_iPCMLevelCount		= makeBigendian( (unsigned int)thumbnail.m_iPCMLevelCount);

			thumbnail.m_iLargeThumbWidth			= makeBigendian( (unsigned int)thumbnail.m_iLargeThumbWidth);
			thumbnail.m_iLargeThumbHeight		= makeBigendian( (unsigned int)thumbnail.m_iLargeThumbHeight);
			thumbnail.m_iLargeThumbPos			= makeBigendian( (unsigned int)thumbnail.m_iLargeThumbPos);
			thumbnail.m_iLargeThumbSize		= makeBigendian( (unsigned int)thumbnail.m_iLargeThumbSize);
			
			thumbnail.m_iThumbnailInfoPos		= makeBigendian( (unsigned int)thumbnail.m_iThumbnailInfoPos);
			thumbnail.m_iThumbnailInfoCount	= makeBigendian( (unsigned int)thumbnail.m_iThumbnailInfoCount);
			
			pClipInfo->setExistAudio(thumbnail.m_isAudio);
			pClipInfo->setExistVideo(thumbnail.m_isVideo);
			pClipInfo->setClipAudioDuration(thumbnail.m_iDuration);
			pClipInfo->setClipVideoDuration(thumbnail.m_iDuration);
			pClipInfo->setWidth(thumbnail.m_iVideoWidth);
			pClipInfo->setHeight(thumbnail.m_iVideoHeight);
			pClipInfo->setSeekPointCount(thumbnail.m_iSeekTableCount);
			
			pClipInfo->setThumbnailPath((const char *)pThumbFilePath);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Thumbnail Info", __LINE__);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] \t Audio Exist : %d", __LINE__, thumbnail.m_isAudio);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] \t Video Exist : %d", __LINE__, thumbnail.m_isVideo);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] \t Clip Duration : %d", __LINE__, thumbnail.m_iDuration);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] \t Width : %d", __LINE__, thumbnail.m_iVideoWidth);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] \t Height : %d", __LINE__, thumbnail.m_iVideoHeight);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] \t Thumb Width : %d", __LINE__, thumbnail.m_iThumbnailWidth);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] \t Thumb Height : %d", __LINE__, thumbnail.m_iThumbnailHeight);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] \t ThumbPath : %s", __LINE__, pThumbFilePath);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipInfo Out(%s)", __LINE__, pFilePath);
			return dynamic_cast<IClipInfo*>(pClipInfo);
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipInfo Out failed(%s)", __LINE__, pFilePath);
		return NULL;
	}

	CClipItem* pItem = new CClipItem(INVALID_CLIP_ID);
	if( pItem == NULL)
		return NULL;
	
	pItem->setThumbnailPath(pThumbFilePath);

	NEXVIDEOEDITOR_ERROR eRet = (NEXVIDEOEDITOR_ERROR)pItem->parseClipFile(pFilePath, isVideoThumbnail, isAudioPCMLevel);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		SAFE_RELEASE(pItem);
		return NULL;
	}

	CClipInfo* pClipInfo = new CClipInfo;
	if( pClipInfo == NULL )
	{
		SAFE_RELEASE(pItem);
		return NULL;
	}
	
	pClipInfo->setThumbnailPath(pThumbFilePath);
	if( pItem->getTotalTime() <= CNexVideoEditor::m_iSupportContentDuration )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Not Support Clip - reason(Clip play time is short)", __LINE__);		
		SAFE_RELEASE(pClipInfo);
		SAFE_RELEASE(pItem);
		return NULL;
	}

	if( pItem->isAudioExist() )
	{
		pClipInfo->setClipAudioDuration(pItem->getTotalAudioTime());
		pClipInfo->setExistAudio(TRUE);
		pClipInfo->setAudioCodecType(pItem->getCodecType(NXFF_MEDIA_TYPE_AUDIO));	
	}

	if( pItem->isVideoExist() )
	{
		pClipInfo->setClipVideoDuration(pItem->getTotalVideoTime());
		pClipInfo->setExistVideo(TRUE);
		pClipInfo->setVideoCodecType(pItem->getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO));
		pClipInfo->setWidth(pItem->getWidth());
		pClipInfo->setHeight(pItem->getHeight());
		pClipInfo->setDisplayWidth(pItem->getDisplayWidth());
		pClipInfo->setDisplayHeight(pItem->getDisplayHeight());
		pClipInfo->setThumbnailPath(pItem->getThumbnailPath());
		pClipInfo->setSeekPointCount(pItem->getSeekPointCount());
	}
	
	SAFE_RELEASE(pItem);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipInfo Out(Time(%u) %s)", __LINE__, nexSAL_GetTickCount() - uiStart, pFilePath);
	return dynamic_cast<IClipInfo*>(pClipInfo);
}

IClipInfo* CNexVideoEditor::getClipInfo_Sync(char* pFilePath, int iFlag, int iUserTag, int* pRet)
{
	if( pFilePath == NULL || pRet == NULL )
	{
		if( pRet )
		{
			*pRet = (int)NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
		}
		return NULL;
	}

	unsigned int uiStart = nexSAL_GetTickCount();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipInfo_Sync(%s) Flag(%d) ", __LINE__, pFilePath, iFlag);

	CClipItem* pItem = new CClipItem(INVALID_CLIP_ID);
	if( pItem == NULL)
	{
		*pRet = (int)NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		return NULL;
	}
	
	NEXVIDEOEDITOR_ERROR eRet = (NEXVIDEOEDITOR_ERROR)pItem->parseClipFile(pFilePath, iFlag);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		*pRet = (int)eRet;
		SAFE_RELEASE(pItem);
		return NULL;
	}

	CClipInfo* pClipInfo = new CClipInfo;
	if( pClipInfo == NULL )
	{
		*pRet = (int)NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		SAFE_RELEASE(pItem);
		return NULL;
	}
	pClipInfo->setClipAudioDuration(pItem->getTotalAudioTime());
	pClipInfo->setClipVideoDuration(pItem->getTotalVideoTime());

	unsigned int uiAudioEditBoxTime = 0, uiVideoEditBoxTime = 0;
	pItem->getEditBoxTIme(&uiAudioEditBoxTime, &uiVideoEditBoxTime);
	pClipInfo->setEditBoxTime(uiAudioEditBoxTime, uiVideoEditBoxTime);

	if( pItem->isAudioExist() )
	{
		//not support below audio samplingrate in nexresampler
		if( pItem->getAudioSampleRate() < 2 || pItem->getAudioSampleRate() > 384000)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipInfo_Sync Audio SampleRate(%d) ", __LINE__, pItem->getAudioSampleRate());
			*pRet = (int)NEXVIDEOEDITOR_ERROR_UNSUPPORT_AUDIO_SAMPLINGRATE;
			SAFE_RELEASE(pItem);
			return NULL;
		}

		pClipInfo->setExistAudio(TRUE);
		pClipInfo->setAudioCodecType(pItem->getCodecType(NXFF_MEDIA_TYPE_AUDIO));
		pClipInfo->setAudioBitRate(pItem->getAudioBitRate());
		pClipInfo->setAudioSampleRate(pItem->getAudioSampleRate());
		pClipInfo->setAudioChannels(pItem->getAudioChannels());
	}

	if( pItem->isVideoExist() )
	{
		pClipInfo->setExistVideo(TRUE);
		pClipInfo->setVideoCodecType(pItem->getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO));
		pClipInfo->setWidth(pItem->getWidth());
		pClipInfo->setHeight(pItem->getHeight());
		pClipInfo->setDisplayWidth(pItem->getDisplayWidth());
		pClipInfo->setDisplayHeight(pItem->getDisplayHeight());
		pClipInfo->setVideoFPS(pItem->getVideoFPS());
		pClipInfo->setVideoFPSFloat(pItem->getVideoFPSFloat());
		pClipInfo->setVideoH264Profile(pItem->getVideoH264Profile());
		pClipInfo->setVideoH264Level(pItem->getVideoH264Level());
		pClipInfo->setVideoH264Interlaced((pItem->getVideoH264Interlaced()));
		pClipInfo->setVideoOrientation(pItem->getVideoOrientation());

		pClipInfo->setVideoBitRate(pItem->getVideoBitRate());

		pClipInfo->setThumbnailPath(pItem->getThumbnailPath());
		pClipInfo->setSeekPointCount(pItem->getSeekPointCount());

		int size = 0;
		const char* puuid = (const char*)pItem->getVideoTrackUUID(&size);
		pClipInfo->setVideoUUID(puuid,size);
		pClipInfo->setVideoRenderType(pItem->getVideoRenderMode());
		pClipInfo->setVideoHDRType(pItem->getHdrMetaData(NULL));
	}

	if( (iFlag & GET_CLIPINFO_INCLUDE_SEEKTABLE) ==  GET_CLIPINFO_INCLUDE_SEEKTABLE )
	{
		int iSeekTableCount = pItem->getSeekTableCount();
		if( iSeekTableCount > 0 )
		{
			for( int i = 0; i < iSeekTableCount; i++)
			{
				pClipInfo->addSeekTableItem(pItem->getSeekTableValue(i));
			}
		}
	}
	
	SAFE_RELEASE(pItem);
	pClipInfo->printClipInfo();
	*pRet = (int)NEXVIDEOEDITOR_ERROR_NONE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipInfo successed Out(Time(%u) %s)", __LINE__, nexSAL_GetTickCount() - uiStart, pFilePath);
	return dynamic_cast<IClipInfo*>(pClipInfo);
}

int CNexVideoEditor::getClipVideoThumb(char* pFilePath, char* pThumbFilePath, int iWidth, int iHeight, int iStartTime, int iEndTime, int iCount, int iFlag, int iUserTag)
{
#if defined(_ANDROID)
	if( m_hExportRenderer == NULL || m_hOutputSurface == NULL )
#elif defined(__APPLE__)
	if( m_hExportRenderer == NULL )
#endif
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Renderer did not initailize", __LINE__);
		return 1;
	}
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}

	/*
	// RYU 20140106 move this routine in thumbnail.cpp
	NXT_ThemeRenderer_SetNativeWindow(	m_hExportRenderer, 
											(ANativeWindow*)m_hOutputSurface, 
											THUMBNAIL_VIDEO_MODE_VIDEO_WIDTH, 
											THUMBNAIL_VIDEO_MODE_VIDEO_HEIGHT );
	
	// Case : for mantis 9228
	NXT_ThemeRenderer_ClearTransitionEffect(m_hExportRenderer);
	NXT_ThemeRenderer_ClearClipEffect(m_hExportRenderer);
	*/
	
	CNxMsgBackgroundGetInfo* pMsg = new CNxMsgBackgroundGetInfo(	pFilePath, 
																pThumbFilePath, 
																1, 
																0, 
																m_hExportRenderer, 
																m_hOutputSurface, 
																iWidth,
																iHeight,
																iStartTime,
																iEndTime,
																iCount,
																iFlag,
																iUserTag);
	if( pMsg )
	{
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipVideoThumb Send Command", __LINE__);
	}
	return 0;
}

int CNexVideoEditor::getClipVideoThumbWithTimeTable(char* pFilePath, char* pThumbFilePath, int iWidth, int iHeight, int iSize, int* pTimeTable, int iFlag, int iUserTag)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipVideoThumbWithTimeTable In", __LINE__);
#if defined(_ANDROID)
	if( m_hExportRenderer == NULL || m_hOutputSurface == NULL )
#elif defined(__APPLE__)
	if( m_hExportRenderer == NULL )
#endif
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Renderer did not initailize", __LINE__);
		return 1;
	}

	CNxMsgBackgroundGetInfo* pMsg = new CNxMsgBackgroundGetInfo(	pFilePath, 
																	pThumbFilePath,
																	1, 
																	0, 
																	iWidth,
																	iHeight,
																	pTimeTable,
																	iSize,
																	m_hExportRenderer, 
																	m_hOutputSurface, 
																	iFlag,
																	iUserTag);

	if( pMsg )
	{
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipVideoThumbWithTimeTable Send Command", __LINE__);
	}
	return 0;
}

int CNexVideoEditor::getClipAudioThumb(char* pFilePath, char* pThumbFilePath, int iUserTag)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	CNxMsgBackgroundGetInfo* pMsg = new CNxMsgBackgroundGetInfo(pFilePath, pThumbFilePath, 0, 1, m_hExportRenderer, m_hOutputSurface, iUserTag);
	if( pMsg )
	{
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipAudioThumb Send Command", __LINE__);
	}
	return 0;
}

int CNexVideoEditor::getClipAudioThumbPartial(char* pFilePath, char* pThumbFilePath, int iStartTime, int iEndTime, int iUseCount, int iSkipCount, int iUserTag)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	CNxMsgBackgroundGetInfo* pMsg = new CNxMsgBackgroundGetInfo(pFilePath, pThumbFilePath, 0, 1, iStartTime, iEndTime, iUseCount, iSkipCount, m_hExportRenderer, m_hOutputSurface, iUserTag);
	if( pMsg )
	{
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipAudioThumb Send Command", __LINE__);
	}
	return 0;
}

int CNexVideoEditor::getClipStopThumb(int iUserTag)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	CNxMsgBackgroundGetInfoStop* pMsg = new CNxMsgBackgroundGetInfoStop(iUserTag);
	if( pMsg )
	{
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipStopThumb Send Command", __LINE__);
	}
	return 0;
}

int CNexVideoEditor::getClipVideoThumbRaw(char* pFilePath, int* pTimeTable, int iTimeTableCount, int iFlag, int iUserTag)
{
#if defined(_ANDROID)
	if( m_hExportRenderer == NULL || m_hOutputSurface == NULL )
#elif defined(__APPLE__)
	if( m_hExportRenderer == NULL )
#endif
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Renderer did not initailize", __LINE__);
		return 1;
	}

	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	/*
	// RYU 20140106 move this routine in thumbnail.cpp
	NXT_ThemeRenderer_SetNativeWindow(	m_hExportRenderer, 
											(ANativeWindow*)m_hOutputSurface, 
											THUMBNAIL_VIDEO_MODE_VIDEO_WIDTH, 
											THUMBNAIL_VIDEO_MODE_VIDEO_HEIGHT );
	
	// Case : for mantis 9228
	NXT_ThemeRenderer_ClearTransitionEffect(m_hExportRenderer);
	NXT_ThemeRenderer_ClearClipEffect(m_hExportRenderer);
	*/
	
	CNxMsgBackgroundGetInfo* pMsg = new CNxMsgBackgroundGetInfo(pFilePath, 1, 0, pTimeTable, iTimeTableCount, m_hExportRenderer, m_hOutputSurface, iFlag, iUserTag);
	if( pMsg )
	{
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipVideoThumb Send Command", __LINE__);
	}
	return 0;
}

int CNexVideoEditor::getClipAudioThumbRaw(char* pFilePath, int iStartTIme, int iEndTime, int iFlag, int iUserTag)
{
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	CNxMsgBackgroundGetInfo* pMsg = new CNxMsgBackgroundGetInfo(pFilePath, 0, 1, iStartTIme, iEndTime, m_hExportRenderer, m_hOutputSurface, iFlag, iUserTag);
	if( pMsg )
	{
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getClipAudioThumb Send Command", __LINE__);
	}
	return 0;
}

void CNexVideoEditor::clearScreen(int iTag)
{
	if( m_pProjectManager )
	{
		CNxMsgClearScreen* pMag = new CNxMsgClearScreen(iTag);
		if( pMag )
		{
			m_pProjectManager->SendCommand(pMag);
			SAFE_RELEASE(pMag);
		}
	}
}

int CNexVideoEditor::fastOptionPreview(const char* pStringOption, int iDisplay)
{
	if( pStringOption == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastOptionPreview failed because param was null", __LINE__);
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;
	}

	if( m_pProjectManager )
	{
		CNxMsgFastOptionPreview* pMag = new CNxMsgFastOptionPreview(pStringOption,iDisplay);
		if( pMag )
		{
			m_pProjectManager->SendCommand(pMag);
			SAFE_RELEASE(pMag);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::clearTrackCache()
{
	if( m_pProjectManager )
	{
		m_pProjectManager->SendSimpleCommand(MESSAGE_CLEAR_TRACK_CACHE);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::getChipsetType()
{
#ifdef _ANDROID
	return getCheckChipsetType();
#endif
	return 0;
}

int CNexVideoEditor::getVersionNumber(int iVersion)
{
	if( iVersion == 1 )
		return NEXEDITOR_MAJOR_VERSION;
	
	if( iVersion == 2 )
		return NEXEDITOR_MINOR_VERSION;

	if( iVersion == 3 )
#ifdef NX_CERT_DEBUG_MODE
		return (NEXEDITOR_PATCH_VERSION%2==1 ? NEXEDITOR_PATCH_VERSION+1:NEXEDITOR_PATCH_VERSION);
#else
		return NEXEDITOR_PATCH_VERSION;
#endif

	return -1;
}

int CNexVideoEditor::startVoiceRecorder(char* pFilePath, int iSampleRate, int iChannels, int iBitForSample)
{
	unsigned char*	pConfig		= NULL;
	int				iConfigLen	= 0;
	unsigned int 		uiRet		= 0;
	long long 		llMaxFileSize = 	0;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] startVoiceRecorder In(%s) (%d %d %d)", __LINE__, 
		pFilePath, iSampleRate, iChannels, iBitForSample);

	m_uiSampleRate 	= iSampleRate;
	m_uiChannels 	= iChannels;
	m_uiBitPerSample 	= iBitForSample;
	m_ullTotalSampleCount = 0;
	
	if( m_pProjectManager == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
	}
	
	if( pFilePath == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( m_pVoiceRecordPath )
	{
		nexSAL_MemFree(m_pVoiceRecordPath);
		m_pVoiceRecordPath = NULL;
	}

	m_pVoiceRecordPath = (char*)nexSAL_MemAlloc(strlen(pFilePath) + 1);
	if( m_pVoiceRecordPath == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] startVoiceRecorder fail because memalloc is failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	strcpy(m_pVoiceRecordPath, pFilePath);


	if( m_pPCMTempBuffer )
	{
		nexSAL_MemFree(m_pPCMTempBuffer);
		m_pPCMTempBuffer = NULL;
	}	

	m_pPCMTempBuffer = (unsigned char*)nexSAL_MemAlloc(EDITOR_DEFAULT_AUDIOSAMPLESIZE);
	if( m_pPCMTempBuffer == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [NEXVIDEOEDITOR_VideoEditor.cpp %d] startVoiceRecorder temp buffer alloc failed", __LINE__);
		goto ErrReturn;
	}
	m_uiPCMTempBufferSize = 0;

	m_pFileWriter = new CNexFileWriter;
	if( m_pFileWriter == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [NEXVIDEOEDITOR_VideoEditor.cpp %d] startVoiceRecorder FileWriter create failed", __LINE__);
		goto ErrReturn;
	}
	
	llMaxFileSize = 	((m_uiSampleRate)*m_uiChannels*(m_uiBitPerSample/8)*((m_pProjectManager->getDuration() + 1000)/1000)); // SamplingRate* channels* Quantization size * duratinon(msec) / sec

	if( m_pFileWriter->initFileWriter(llMaxFileSize, m_pProjectManager->getDuration()) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] FileWrite initFileWriter failed", __LINE__);
		goto ErrReturn;
	}

	if( m_pFileWriter->setFilePath(m_pVoiceRecordPath) == FALSE )
		
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] FileWrite setFilePath failed", __LINE__);
		goto ErrReturn;
	}
	
	if( m_pFileWriter->setAudioCodecInfo(eNEX_CODEC_A_AAC, m_uiSampleRate, m_uiChannels, 128*1024) == FALSE)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] FileWrite setAudioCodecInfo failed", __LINE__);
		goto ErrReturn;
	}

	m_pFileWriter->setAudioOnlyMode(TRUE);

	if( m_pFileWriter->startFileWriter() == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] FileWrite initFileWriter failed", __LINE__);
		goto ErrReturn;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] startVoiceRecorder Out  DURATION(%d) MAX(%d)", __LINE__, m_pProjectManager->getDuration(), llMaxFileSize);
	return NEXVIDEOEDITOR_ERROR_NONE;

ErrReturn:
	if( m_pPCMTempBuffer )
	{
		nexSAL_MemFree(m_pPCMTempBuffer);
		m_pPCMTempBuffer = NULL;
	}
	m_uiPCMTempBufferSize = 0;
	
	if( m_pVoiceRecordPath )
	{
		nexSAL_MemFree(m_pVoiceRecordPath);
		m_pVoiceRecordPath = NULL;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] startVoiceRecorder Fail Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
}

int CNexVideoEditor::processVoiceRecorder(int iPCMSize, unsigned char* pPCMBuffer)
{
	unsigned char*	pBitStream		= NULL;
	unsigned int		uiBitStreamLen	= 0;
	unsigned int		uiAEncRet		= 0;
	unsigned int		uiRet			= 0;
	unsigned int 		uiSize			= 0;
	unsigned int 		uiDuration		= 0;
	unsigned int		uiTime = (unsigned int)((m_ullTotalSampleCount *1000)/(m_uiBitPerSample/8*m_uiChannels*m_uiSampleRate));
	unsigned int 		uiInterval = (iPCMSize * 1000)/(m_uiBitPerSample/8*m_uiChannels*m_uiSampleRate) +1;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] processVoiceRecorder In(%d 0x%x) Time(%d)", __LINE__, iPCMSize, pPCMBuffer, uiTime);
	if( iPCMSize <= 0 || pPCMBuffer == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( m_pFileWriter == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] processVoiceRecorder fail because handle is null", __LINE__);
		goto ErrReturn;
	}

	if( iPCMSize > 0 && m_pFileWriter)
	{
		m_pFileWriter->setAudioFrameForVoice(uiTime, pPCMBuffer, iPCMSize, &uiDuration, &uiSize);
	}

	m_pFileWriter->setBaseVideoFrameTime(uiTime);
	m_ullTotalSampleCount += iPCMSize;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] processVoiceRecorder Out (%d, %d)", __LINE__, uiDuration, uiSize);

	return NEXVIDEOEDITOR_ERROR_NONE;

ErrReturn:
	if( m_pPCMTempBuffer )
	{
		nexSAL_MemFree(m_pPCMTempBuffer);
		m_pPCMTempBuffer = NULL;
	}
	m_uiPCMTempBufferSize = 0;
	
	if( m_pVoiceRecordPath )
	{
		nexSAL_MemFree(m_pVoiceRecordPath);
		m_pVoiceRecordPath = NULL;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] processVoiceRecorder fail Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED;

}

IClipInfo* CNexVideoEditor::endVoiceRecorder()
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	CClipItem* pItem		= NULL;
	CClipInfo* pClipInfo	= NULL;
	unsigned int uiDuration	= 0;
	unsigned int bCancel 		= FALSE;
	int FWRet = 0;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] endVoiceRecorder In", __LINE__);

	if( m_pVoiceRecordPath == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] endVoiceRecorder fail because m_pVoiceRecordPath is NULL", __LINE__);
		goto ErrReturn;
	}

	if( m_pPCMTempBuffer )
	{
		nexSAL_MemFree(m_pPCMTempBuffer);
		m_pPCMTempBuffer = NULL;
	}
	m_uiPCMTempBufferSize = 0;

	// MONGTO 20130320
	FWRet = m_pFileWriter->endFileWriter(&uiDuration, bCancel);
	SAFE_RELEASE(m_pFileWriter);
	m_pFileWriter = NULL;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] endFileWriter (%d, %d, %d)", __LINE__, FWRet, uiDuration, bCancel);

	pItem = new CClipItem(INVALID_CLIP_ID);
	if( pItem == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] endVoiceRecorder because do not create clip", __LINE__);
		return NULL;
	}
	
	eRet = (NEXVIDEOEDITOR_ERROR)pItem->parseClipFile(m_pVoiceRecordPath, FALSE, FALSE);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] endVoiceRecorder because do not parseClipFile", __LINE__);
		goto ErrReturn;
	}

	pClipInfo = new CClipInfo;
	if( pClipInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] endVoiceRecorder because do not ClipInfo", __LINE__);
		goto ErrReturn;
	}

	if( pItem->getTotalTime() < 1000 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Not Support Clip - reason(Clip play time is short)", __LINE__);		
		goto ErrReturn;
	}

	if( pItem->isAudioExist() )
	{
		pClipInfo->setClipAudioDuration(pItem->getTotalTime());
		pClipInfo->setExistAudio(TRUE);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] endVoiceRecorder because is not audioClip", __LINE__);
		goto ErrReturn;
		
	}

	SAFE_RELEASE(pItem);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] endVoiceRecorder Out(Clip Dur %d)", __LINE__, pClipInfo->getClipAudioDuration());

	return dynamic_cast<IClipInfo*>(pClipInfo);

ErrReturn:
	if( m_pPCMTempBuffer )
	{
		nexSAL_MemFree(m_pPCMTempBuffer);
		m_pPCMTempBuffer = NULL;
	}
	m_uiPCMTempBufferSize = 0;
	
	if( m_pVoiceRecordPath )
	{
		nexSAL_MemFree(m_pVoiceRecordPath);
		m_pVoiceRecordPath = NULL;
	}

	SAFE_RELEASE(pClipInfo);
	SAFE_RELEASE(pItem);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] endVoiceRecorder fail Out", __LINE__);
	return NULL;
}

int CNexVideoEditor::getCurrentRenderImage(unsigned int uiTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getCurrentRenderImage", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::transcodingStart(const char* pSrcClipPath, const char* pDstClipPath, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, int iBitrate, long long llMaxFileSize, int iFPS, int iFlag, int iSpeedFactor, const char* pUser)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] transcodingStart() Start", __LINE__);

	if( pSrcClipPath == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if( pDstClipPath == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] SrcClipPath(%d):%s", __LINE__, strlen(pSrcClipPath), pSrcClipPath);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] DstClipPath(%d):%s", __LINE__, strlen(pDstClipPath), pDstClipPath);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] W:%d, H:%d, DisW:%d, DisH:%d Bit:%d FileSize:%lld FPS(%d) Flag(0x%x), %d", __LINE__,
		iWidth, iHeight, iDisplayWidth, iDisplayHeight, iBitrate, llMaxFileSize, iFPS, iFlag, iSpeedFactor);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] transcodingStart() Start", __LINE__);

	if( m_pProjectManager )
	{
		CNxMsgTranscodingInfo* pTranscodingInfo = new CNxMsgTranscodingInfo;

		if( pTranscodingInfo )
		{
			pTranscodingInfo->m_nMsgType = MESSAGE_TRANSCODING_START;

			pTranscodingInfo->m_pFrameRenderer = (void*)m_hExportRenderer;
			pTranscodingInfo->m_pOutputSurface = (void*)m_hOutputSurface;
			pTranscodingInfo->m_iWidth = iWidth;
			pTranscodingInfo->m_iHeight = iHeight;
			pTranscodingInfo->m_iDisplayWidth = iDisplayWidth;
			pTranscodingInfo->m_iDisplayHeight = iDisplayHeight;
			pTranscodingInfo->m_iBitrate = iBitrate;
			pTranscodingInfo->m_llMaxFileSize =llMaxFileSize;
			pTranscodingInfo->m_iFPS = iFPS == 0 ? EDITOR_DEFAULT_FRAME_RATE : iFPS;
			pTranscodingInfo->m_iFlag = iFlag;
			pTranscodingInfo->m_iSpeedFactor = iSpeedFactor;

			pTranscodingInfo->m_strSrcClipPath = (char*)nexSAL_MemAlloc(strlen(pSrcClipPath)+1);
			pTranscodingInfo->m_strDstClipPath = (char*)nexSAL_MemAlloc(strlen(pDstClipPath)+1);
            
			strcpy(pTranscodingInfo->m_strSrcClipPath, pSrcClipPath);
			strcpy(pTranscodingInfo->m_strDstClipPath, pDstClipPath);

			pTranscodingInfo->setUserData(pUser);
			m_pProjectManager->SendCommand(pTranscodingInfo);
			SAFE_RELEASE(pTranscodingInfo);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}
int CNexVideoEditor::transcodingStop()
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] transcodingStop() Start", __LINE__);

	if( m_pProjectManager )
	{
		m_pProjectManager->SendSimpleCommand(MESSAGE_TRANSCODING_STOP);
			return NEXVIDEOEDITOR_ERROR_NONE;
	}
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::highlightStart(const char* pSrcClipPath, int iIndexMode, int iRequestInterval, int iRequestCount, int iOutputMode, const char* pDstClipPath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, int iDecodeMode)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] highlightStart() Start", __LINE__);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] RequestInterval(%d), RequestCount(%d), IndexMode(%d), OutputMode(%d), DecodeMode(%d)", __LINE__, iRequestInterval, iRequestCount, iIndexMode, iOutputMode, iDecodeMode);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] SrcClipPath(%d):%s", __LINE__, strlen(pSrcClipPath), pSrcClipPath);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] SrcClipPath(%d):%s", __LINE__, strlen(pSrcClipPath), pSrcClipPath);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] W:%d, H:%d, Bit:%d FileSize:%lld", __LINE__,
		iWidth, iHeight, iBitrate, llMaxFileSize);

	if( pSrcClipPath == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if(iOutputMode == 1)
	{
		if( pDstClipPath == NULL )
			return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] highlightStart() Start", __LINE__);

	if( m_pProjectManager )
	{
		CNxMsgHighLightIndex* pHighLightIndex = new CNxMsgHighLightIndex;

		if( pHighLightIndex )
		{
			pHighLightIndex->m_nMsgType = MESSAGE_MAKE_HIGHLIGHT_START;

			pHighLightIndex->m_pFrameRenderer = (void*)m_hExportRenderer;
			pHighLightIndex->m_pOutputSurface = (void*)m_hOutputSurface;
			pHighLightIndex->m_iIndexMode = iIndexMode;
			pHighLightIndex->m_iRequestInterval = iRequestInterval;
			pHighLightIndex->m_iRequestCount = iRequestCount;
			pHighLightIndex->m_iOutputMode = iOutputMode;
			pHighLightIndex->m_iWidth = iWidth;
			pHighLightIndex->m_iHeight = iHeight;
			pHighLightIndex->m_iBitrate = iBitrate;
			pHighLightIndex->m_llMaxFileSize =llMaxFileSize;
			pHighLightIndex->m_iDecodeMode = iDecodeMode;

			pHighLightIndex->m_strSrcClipPath = (char*)nexSAL_MemAlloc(strlen(pSrcClipPath)+1);
			strcpy(pHighLightIndex->m_strSrcClipPath, pSrcClipPath);

			if(pDstClipPath != NULL)
			{
				pHighLightIndex->m_strDstClipPath = (char*)nexSAL_MemAlloc(strlen(pDstClipPath)+1);
				strcpy(pHighLightIndex->m_strDstClipPath, pDstClipPath);
			}          

			m_pProjectManager->SendCommand(pHighLightIndex);
			SAFE_RELEASE(pHighLightIndex);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;	
}

int CNexVideoEditor::highlightStop()
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] highlightStop() Start", __LINE__);

	if( m_pProjectManager )
	{
		m_pProjectManager->SendSimpleCommand(MESSAGE_MAKE_HIGHLIGHT_STOP);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::checkDirectExport(int option)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkDirectExport(%d)", __LINE__, option);

	if( m_pProjectManager )
	{
		CNxMsgCheckDirectExport* pMsg = new CNxMsgCheckDirectExport();
		if( pMsg != NULL ) {
			pMsg->m_iFlag = option;
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkDirectExport send command", __LINE__);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkDirectExport End with no action", __LINE__);
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::directExport(const char* pEncodeFilePath, long long llMaxFileSize, int iMaxFileDuration, const char* pUserData, int iFlag)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] directExport() Start", __LINE__);

	if( m_pProjectManager )
	{
		CNxMsgDirectExport* pMsg = new CNxMsgDirectExport(pEncodeFilePath, llMaxFileSize, iMaxFileDuration, pUserData, iFlag);

		if( pMsg )
		{
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] directExport() End with send command", __LINE__);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] directExport() End with no action", __LINE__);
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::checkPFrameDirectExportSync(const char* pSrc)
{
#ifdef _ANDROID
	if( pSrc == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkPFrameDirectExportSync failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	unsigned int uiStart = nexSAL_GetTickCount();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkPFrameDirectExportSync(%s)", __LINE__, pSrc);

	CClipItem* pItem = new CClipItem(INVALID_CLIP_ID);
	if( pItem == NULL)
	{
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}

	NEXVIDEOEDITOR_ERROR eRet = (NEXVIDEOEDITOR_ERROR)pItem->setClipPath(pSrc);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkPFrameDirectExportSync failed(%d)", __LINE__, eRet);

		SAFE_RELEASE(pItem);
		return eRet;
	}

	if( pItem->compareVideoDSIWithEncoder() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkPFrameDirectExportSync end", __LINE__);
		SAFE_RELEASE(pItem);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkPFrameDirectExportSync failed", __LINE__);

	SAFE_RELEASE(pItem);
	return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_DSI_DIFF_ERROR;
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::reverseStart(const char* pSrcClipPath, const char* pDstClipPath, const char* pTempClipPath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, int iStartTime, int iEndTime, int iDecodeMode)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] reverseStart() Start %d", __LINE__, m_bSupportFrameTimeChecker);

	if(!m_bSupportFrameTimeChecker)
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;

	if( pSrcClipPath == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if( pDstClipPath == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if( pTempClipPath == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] SrcClipPath(%d):%s", __LINE__, strlen(pSrcClipPath), pSrcClipPath);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] DstClipPath(%d):%s", __LINE__, strlen(pDstClipPath), pDstClipPath);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] TempClipPath(%d):%s", __LINE__, strlen(pTempClipPath), pTempClipPath);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] W:%d, H:%d, Bit:%d FileSize:%lld, start %d, end %d", __LINE__,
		iWidth, iHeight, iBitrate, llMaxFileSize, iStartTime, iEndTime);

	if( m_pProjectManager )
	{
		CNxMsgReverseInfo* pReverseInfo = new CNxMsgReverseInfo;

		if( pReverseInfo )
		{
			pReverseInfo->m_nMsgType = MESSAGE_MAKE_REVERSE_START;

			pReverseInfo->m_pFrameRenderer = (void*)m_hExportRenderer;
			pReverseInfo->m_pOutputSurface = (void*)m_hOutputSurface;
			pReverseInfo->m_iWidth = iWidth;
			pReverseInfo->m_iHeight = iHeight;
			pReverseInfo->m_iBitrate = iBitrate;
			pReverseInfo->m_llMaxFileSize =llMaxFileSize;
			pReverseInfo->m_uStartTime = iStartTime;
			pReverseInfo->m_uEndTime = iEndTime;
			pReverseInfo->m_iDecodeMode = iDecodeMode;			

			pReverseInfo->m_strSrcClipPath = (char*)nexSAL_MemAlloc(strlen(pSrcClipPath)+1);
			strcpy(pReverseInfo->m_strSrcClipPath, pSrcClipPath);

			pReverseInfo->m_strDstClipPath = (char*)nexSAL_MemAlloc(strlen(pDstClipPath)+1);
			strcpy(pReverseInfo->m_strDstClipPath, pDstClipPath);

			pReverseInfo->m_strTempClipPath = (char*)nexSAL_MemAlloc(strlen(pTempClipPath)+1);
			strcpy(pReverseInfo->m_strTempClipPath, pTempClipPath);

			m_pProjectManager->SendCommand(pReverseInfo);
			SAFE_RELEASE(pReverseInfo);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;	
}

int CNexVideoEditor::reverseStop()
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] reverseStop() Start %d", __LINE__, m_bSupportFrameTimeChecker);

	if(!m_bSupportFrameTimeChecker)
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;

	if( m_pProjectManager )
	{
		m_pProjectManager->SendSimpleCommand(MESSAGE_MAKE_REVERSE_STOP);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::fastPreviewStart(unsigned int uiStartTime, unsigned int uiEndTime, int displayWidth, int displayHeight)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewStart(%d %d %d %d)", __LINE__, uiStartTime, uiEndTime, displayWidth, displayHeight);
	if( m_pProjectManager )
	{
		CNxMsgFastPreview* pMsg = new CNxMsgFastPreview;
		if( pMsg )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewStart Start", __LINE__);
			pMsg->m_nMsgType = MESSAGE_FAST_PREVIEW_START;
			pMsg->m_uiStartTime = uiStartTime;
			pMsg->m_uiEndTime = uiEndTime;
			pMsg->m_iWidth = displayWidth;
			pMsg->m_iHeight = displayHeight;
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewStart End", __LINE__);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewStart  End failed", __LINE__);
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::fastPreviewStop()
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewStop", __LINE__);
	if( m_pProjectManager )
	{
		CNxMsgFastPreview* pMsg = new CNxMsgFastPreview;
		if( pMsg )
		{
			pMsg->m_nMsgType = MESSAGE_FAST_PREVIEW_STOP;
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewStop  End", __LINE__);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewStop  End failed", __LINE__);
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::fastPreviewTime(unsigned int uiTime)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewTime(%d)", __LINE__, uiTime);
	if( m_pProjectManager )
	{
		CNxMsgFastPreview* pMsg = new CNxMsgFastPreview;
		if( pMsg )
		{
			pMsg->m_nMsgType = MESSAGE_FAST_PREVIEW_TIME;
			pMsg->m_uiTime = uiTime;
			m_pProjectManager->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewTime  End", __LINE__);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] fastPreviewTime  End failed", __LINE__);
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::checkIDRStart(const char* pSrcClipPath)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkIDRStart(0x%x)", __LINE__, pSrcClipPath);

	SAFE_RELEASE(m_pIDRCheckItem);

	m_pIDRCheckItem = new CClipItem(INVALID_CLIP_ID);
	if( m_pIDRCheckItem == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkIDRStart alloc failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	
	NEXVIDEOEDITOR_ERROR eRet = (NEXVIDEOEDITOR_ERROR)m_pIDRCheckItem->parseClipFile(pSrcClipPath, GET_CLIPINFO_CHECK_IDR_MODE);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkIDRStart failed(%d)", __LINE__, eRet);
		return eRet;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkIDRStart End", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::checkIDREnd()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkIDREnd(%d)", __LINE__, m_pIDRCheckItem);

	SAFE_RELEASE(m_pIDRCheckItem);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkIDREnd End", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::checkIDRTime(unsigned int uiTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkIDRTime(0x%x, %d)", __LINE__, m_pIDRCheckItem, uiTime);

	if( m_pIDRCheckItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkIDRTime failed(0x%x, %d)", __LINE__, m_pIDRCheckItem, uiTime);
		return -1;
	}

	int iRet = m_pIDRCheckItem->findIDRFrameTime(uiTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] checkIDRTime End(%d)", __LINE__, iRet);
	return iRet;
}

int CNexVideoEditor::setVolumeWhilePlay(int iMasterVolume, int iSlaveVolume)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setVolumeWhilePlay(%d, %d)", __LINE__, iMasterVolume, iSlaveVolume);
	CNxMsgSetVolumeWhilePlay* pMsg = new CNxMsgSetVolumeWhilePlay;

	pMsg->m_iMasterVolume = iMasterVolume;
	pMsg->m_iSlaveVolume = iSlaveVolume;

	if( m_pProjectManager != NULL )
	{
		m_pProjectManager->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setVolumeWhilePlay Out(%d, %d)", __LINE__, iMasterVolume, iSlaveVolume);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setVolumeWhilePlay Out(%d, %d)", __LINE__, iMasterVolume, iSlaveVolume);
	SAFE_RELEASE(pMsg);

	return NEXVIDEOEDITOR_ERROR_PROJECT_NOT_CREATE;
}

int CNexVideoEditor::getVolumeWhilePlay(int* pMasterVolume, int* pSlaveVolume)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getVolumeWhilePlay", __LINE__);

	if( m_pProjectManager != NULL )
	{
		return m_pProjectManager->getTotalAudioVolumeWhilePlay(pMasterVolume, pSlaveVolume);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getVolumeWhilePlay m_pProjectManager is NULL (fail)", __LINE__);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::addUDTA(int iType, const char* pData)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] addUDTA In(0x%x %s)", __LINE__, iType, pData);

#ifdef _ANDROID
	if( pData == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	CNexUDTA udta;
	if( udta.setUDTA(iType, pData) == FALSE )
	{
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	m_vecUDTA.insert(m_vecUDTA.end(), udta);
#endif
	return NEXVIDEOEDITOR_ERROR_NONE;

}

int CNexVideoEditor::clearUDTA()
{
#ifdef _ANDROID
	m_vecUDTA.clear();
#endif
	return NEXVIDEOEDITOR_ERROR_NONE;
}

//yoon
int CNexVideoEditor::setVideoTrackUUID(int mode, unsigned char *byteUUID)
{
    iVideoTrackUUIDMode = mode; //0 - none , 1- set 1st video uuid, 2 - (TODO:) set user uuid 
    return NEXVIDEOEDITOR_ERROR_NONE;
}

//yoon
int CNexVideoEditor::set360VideoTrackPosition(int iXAngle,int iYAngle, int iflags)
{
    m_i360VideoTrackPositionXAngle = iXAngle;
    m_i360VideoTrackPositionYAngle = iYAngle;
    m_i360VideoViewMode = iflags;
    return NEXVIDEOEDITOR_ERROR_NONE;
}

//yoon
int CNexVideoEditor::get360VideoTrackPositionRadian(int iAxis)
{
    int rval = 0;
    switch (iAxis){
        case 1: //X
            rval = m_i360VideoTrackPositionXAngle ;
            break;
        case 2:    //Y
            rval = m_i360VideoTrackPositionYAngle ;
            break;

        case 3:    //viewMode
            rval = m_i360VideoViewMode ;
            break;
            
        default:            
            break;
    }
    return rval;
}

//yoon
int CNexVideoEditor::releaseLUTTexture(int lut_resource_id)
{
    if( m_hExportRenderer ){
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] %s export renderer release LUT", __LINE__, __func__);
        NXT_ThemeRenderer_ReleaseLUTTexture(m_hExportRenderer, lut_resource_id);
    }
    
    if (m_hRenderer){
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] %s preview renderer release LUT", __LINE__, __func__);
		NXT_ThemeRenderer_ReleaseLUTTexture(m_hRenderer, lut_resource_id);
	}
    return 0;
}

//Jeff
int CNexVideoEditor::setBaseFilterRenderItem(const char* uid){

	if (m_hExportRenderer){

		NXT_ThemeRenderer_SetBaseFilterRenderItemUID(m_hExportRenderer, uid);
	}
	if (m_hRenderer){

		NXT_ThemeRenderer_SetBaseFilterRenderItemUID(m_hRenderer, uid);
	}
	return 1;
}

int CNexVideoEditor::setDeviceLightLevel(int lightLevel)
{
	if (m_hExportRenderer){

		NXT_ThemeRenderer_SetDeviceLightLevel(m_hExportRenderer, lightLevel);
	}
	if (m_hRenderer){

		NXT_ThemeRenderer_SetDeviceLightLevel(m_hRenderer, lightLevel);
	}
	return 1;
}

int CNexVideoEditor::setDeviceGamma(float gamma)
{
	if (m_hExportRenderer){

		NXT_ThemeRenderer_SetDeviceGamma(m_hExportRenderer, gamma);
	}
	if (m_hRenderer){
		
		NXT_ThemeRenderer_SetDeviceGamma(m_hRenderer, gamma);
	}
	return 1;
}

int CNexVideoEditor::setForceRTT(int val)
{
	if (m_hExportRenderer){

		NXT_ThemeRenderer_SetForceRTT(m_hExportRenderer, val);
	}
	if (m_hRenderer){

		NXT_ThemeRenderer_SetForceRTT(m_hRenderer, val);
	}
	return 1;
}
int CNexVideoEditor::setThumbnailRoutine(int val)
{
	m_iThumbnailRoutine = val;
	return 0;
}
int CNexVideoEditor::getThumbnailRoutine()
{
	return m_iThumbnailRoutine;
}

int CNexVideoEditor::getTexNameForClipID(int export_flag, int clipid){

	const float** ppmatrix;
	NXT_HThemeRenderer renderer = getRenderer(export_flag);
	if(renderer)
		return NXT_ThemeRenderer_GetTextureNameForVideoLayer(renderer, clipid);
	return 0;
}

int CNexVideoEditor::setTexNameForLUT(int export_flag, int clipid, float x, float y){

	NXT_HThemeRenderer renderer = getRenderer(export_flag);
	if(renderer)
		return NXT_ThemeRenderer_SetTextureNameForLUT(renderer, clipid, x, y);
	return -1;
}

int CNexVideoEditor::getTexNameForMask(int export_flag){

	NXT_HThemeRenderer renderer = getRenderer(export_flag);
	if(renderer)
		return NXT_ThemeRenderer_GetTextureNameForMask(renderer);
	return 0;
}

int CNexVideoEditor::getTexNameForBlend(int export_flag){
	if (export_flag){
		if (m_hExportRenderer){
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] %s", __LINE__, __func__);
			return NXT_ThemeRenderer_GetTextureNameForBlend(m_hExportRenderer);
		}
		else
			return 0;
	}
	else{
		if (m_hRenderer){
			return NXT_ThemeRenderer_GetTextureNameForBlend(m_hRenderer);
		}
		else
			return 0;
	}
	return 0;
}

int CNexVideoEditor::swapBlendMain(int export_flag){
	if (export_flag){
		if (m_hExportRenderer){
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] %s", __LINE__, __func__);
			return NXT_ThemeRenderer_SwapBlendMain(m_hExportRenderer);
		}
		else
			return 0;
	}
	else{
		if (m_hRenderer){
			return NXT_ThemeRenderer_SwapBlendMain(m_hRenderer);
		}
		else
			return 0;
	}
	return 0;
}

int CNexVideoEditor::getTexNameForWhite(int export_flag){

	NXT_HThemeRenderer renderer = getRenderer(export_flag);
	if(renderer)
		return NXT_ThemeRenderer_GetTextureNameForWhite(renderer);
	return 0;
}

int CNexVideoEditor::cleanupMaskWithWhite(int export_flag){

	NXT_HThemeRenderer renderer = getRenderer(export_flag);
	if(renderer)
		return NXT_ThemeRenderer_CleanupMaskWithWhite(renderer);
	return -1;
}

int CNexVideoEditor::setRenderToMask(int export_flag){

	NXT_HThemeRenderer renderer = getRenderer(export_flag);
	if(renderer)
		return NXT_ThemeRenderer_SetRenderToMask(renderer);
	return -1;
}

int CNexVideoEditor::setRenderToDefault(int export_flag){

	NXT_HThemeRenderer renderer = getRenderer(export_flag);
	if(renderer)
		return NXT_ThemeRenderer_SetRenderToDefault(renderer);
	return -1;
}

int* CNexVideoEditor::createCubeLUT(const char* stream){

	return NXT_ThemeRenderer_CreateCubeLUT(stream);
}

int* CNexVideoEditor::createLGLUT(const char* stream){

	return NXT_ThemeRenderer_CreateLGLUT(stream);
}

int CNexVideoEditor::setBrightness(int value){

	m_iBrightness = value;

	if (m_hRenderer) {
		NXT_ThemeRenderer_SetBrightness(m_hRenderer, value);
	}
	if(m_hExportRenderer) {
		NXT_ThemeRenderer_SetBrightness(m_hExportRenderer, value);
	}
	
	return 0;
}

int CNexVideoEditor::setContrast(int value){

	m_iContrast = value;

	if (m_hRenderer) {
		NXT_ThemeRenderer_SetContrast(m_hRenderer, value);
	}
	if(m_hExportRenderer) {
		NXT_ThemeRenderer_SetContrast(m_hExportRenderer, value);
	}

	return 0;
}

int CNexVideoEditor::setSaturation(int value){

	m_iSaturation = value;

	if (m_hRenderer) {
		NXT_ThemeRenderer_SetSaturation(m_hRenderer, value);
	}
	if(m_hExportRenderer) {
		NXT_ThemeRenderer_SetSaturation(m_hExportRenderer, value);
	}

	return 0;
}

int CNexVideoEditor::setVignette(int value){

	m_iVignette = value;

	if (m_hRenderer) {
		NXT_ThemeRenderer_SetVignette(m_hRenderer, value);
	}
	if(m_hExportRenderer) {
		NXT_ThemeRenderer_SetVignette(m_hExportRenderer, value);
	}

	return 0;
}

int CNexVideoEditor::setVignetteRange(int value){

	m_iVignetteRange = value;

	if (m_hRenderer){
		NXT_ThemeRenderer_SetVignetteRange(m_hRenderer, value);
	}
	if(m_hExportRenderer){
		NXT_ThemeRenderer_SetVignetteRange(m_hExportRenderer, value);
	}

	return 0;
}

int CNexVideoEditor::setSharpness(int value){

	m_iSharpness = value;

	if (m_hRenderer) {
		NXT_ThemeRenderer_SetSharpness(m_hRenderer, value);
	}
	if (m_hExportRenderer){
		NXT_ThemeRenderer_SetSharpness(m_hExportRenderer, value);
	}

	return 0;
}

int CNexVideoEditor::getBrightness(){
	return m_iBrightness;
}

int CNexVideoEditor::getContrast(){
	return m_iContrast;
}

int CNexVideoEditor::getSaturation(){
	return m_iSaturation;
}

int CNexVideoEditor::getVignette(){
	return m_iVignette;
}

int CNexVideoEditor::getVignetteRange(){
	return m_iVignetteRange;
}

int CNexVideoEditor::getSharpness(){
	return m_iSharpness;
}

int CNexVideoEditor::setTaskSleep(int sleep)
{
#ifdef _ANDROID
//	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setTaskSleep(%d) Start", __LINE__, sleep);
//
//	if( m_pProjectManager )
//	{
//		CNxMsgSetTaskSleep* pTaskSleep = new CNxMsgSetTaskSleep(sleep);
//		m_pProjectManager->SendCommand(pTaskSleep);
//		SAFE_RELEASE(pTaskSleep);
//		return NEXVIDEOEDITOR_ERROR_NONE;
//	}
#endif
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::setGIFMode(int mode)
{
	if( m_pProjectManager )
	{
		int result = NEXVIDEOEDITOR_ERROR_INVALID_STATE;
		CNxMsgSetGIFMode* msg = new CNxMsgSetGIFMode();
		msg->m_mode = mode;
		m_pProjectManager->SendCommand(msg);
		if( msg->waitProcessDone(1500) )
		{
			result = msg->m_nResult;
		}
		SAFE_RELEASE(msg);
		return result;
	}
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

#ifdef FOR_LAYER_FEATURE
ILayerItem* CNexVideoEditor::createLayerItem()
{
	ILayerItem* pLayer = (ILayerItem*) new CLayerItem;
	return pLayer;
}

int CNexVideoEditor::addLayerItem(ILayerItem* pLayer)
{
	if( m_pProjectManager )
	{
		CNxMsgAddLayerItem* pAddLayerItem = new CNxMsgAddLayerItem(pLayer);

		if( pAddLayerItem )
		{
			m_pProjectManager->SendCommand(pAddLayerItem);
			SAFE_RELEASE(pAddLayerItem);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;	
}

int CNexVideoEditor::deleteLayerItem(int iLayerID)
{
	if( m_pProjectManager )
	{
		CNxMsgDeleteLayerItem* pDeleteLayerItem = new CNxMsgDeleteLayerItem(iLayerID);

		if( pDeleteLayerItem )
		{
			m_pProjectManager->SendCommand(pDeleteLayerItem);
			SAFE_RELEASE(pDeleteLayerItem);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::clearLayerItem()
{
	if( m_pProjectManager )
	{
		m_pProjectManager->SendSimpleCommand(MESSAGE_SET_CLEAR_LAYER_ITEM);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;	
}
#endif

int CNexVideoEditor::getJPEGImageHeader(char* pFile, int* pWidth, int* pHeight, int* pPitch)
{
	NEXCALCodecHandle hJPEGCodec = CNexCodecManager::getCodec( NEXCAL_MEDIATYPE_IMAGE, NEXCAL_MODE_DECODER, eNEX_CODEC_V_JPEG);
	if( hJPEGCodec == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] get JPEG Codec failed(0x%x)", __LINE__, hJPEGCodec);
		return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
	}

	unsigned int uiRet = nexCAL_ImageDecoderGetHeader(hJPEGCodec, eNEX_CODEC_V_JPEG, pFile, NULL, 0, pWidth, pHeight, pPitch, 0, 0);

	if( uiRet != 0 )
	{
		*pWidth		= 0;
		*pHeight	= 0;
		*pPitch		= 0;
		CNexCodecManager::releaseCodec(hJPEGCodec);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] JPEG get Header failed(0x%x) ret(%d)", __LINE__, hJPEGCodec, uiRet);
		return NEXVIDEOEDITOR_ERROR_UNSUPPORT_FORMAT;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] JPEG get Header sucessed", __LINE__);
	CNexCodecManager::releaseCodec(hJPEGCodec);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::getJPEGImageData(char* pFile, int* pWidth, int* pHeight, int* pPitch, unsigned char* pY, unsigned char* pU, unsigned char* pV)
{
	NEXCALCodecHandle hJPEGCodec = CNexCodecManager::getCodec( NEXCAL_MEDIATYPE_IMAGE, NEXCAL_MODE_DECODER, eNEX_CODEC_V_JPEG);
	if( hJPEGCodec == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] get JPEG Codec failed(0x%x)", __LINE__, hJPEGCodec);
		return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
	}

	unsigned int uiResult;
	unsigned int uiRet = nexCAL_ImageDecoderDecode(hJPEGCodec, pFile, NULL, 0, pWidth, pHeight, pPitch, pY, pU, pV, &uiResult);

	if( uiRet != 0 )
	{
		*pWidth		= 0;
		*pHeight	= 0;
		*pPitch		= 0;
		CNexCodecManager::releaseCodec(hJPEGCodec);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] JPEG get Image data failed(0x%x) ret(%d)", __LINE__, hJPEGCodec, uiRet);
		return NEXVIDEOEDITOR_ERROR_UNSUPPORT_FORMAT;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] JPEG Decoder sucessed", __LINE__);
	CNexCodecManager::releaseCodec(hJPEGCodec);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

void* CNexVideoEditor::getExportRenderer()
{
	return m_hExportRenderer;
}

void* CNexVideoEditor::getExportOutputSurface()
{
#ifdef _ANDROID
	return m_hOutputSurface;
#endif
	return NULL;
}

void* CNexVideoEditor::getMediaCodecCallback()
{
#ifdef _ANDROID
	return getCALCallbackFunc();
#endif
	return NULL;
}

void* CNexVideoEditor::getMediaCodecInputSurf()
{
#ifdef _ANDROID
	return getCALInputSurf();
#endif
	return NULL;
}

void* CNexVideoEditor::getMediaCodecSetTimeStampOnSurf()
{
#ifdef _ANDROID
	return getCALsetTimeStampOnSurf();
#endif
	return NULL;
}

void* CNexVideoEditor::getMediaCodecResetVideoEncoder()
{
#ifdef _ANDROID
	return getCALresetVideoEncoder();
#endif
	return NULL;
}

void* CNexVideoEditor::getMediaCodecSetCropAchieveResolution()
{
#ifdef _ANDROID
	return getCALsetCropAchieveResolution();
#endif
	return NULL;
}

void* CNexVideoEditor::getAudioRenderFuncs()
{
	return getRALgetAudioRenderFuncs();
}

int CNexVideoEditor::notifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3, unsigned int uiParam4)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}
	
	switch(uiEventType)
	{
		case MESSAGE_STATE_CHANGE_DONE:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_STATE_CHANGE, uiParam1, uiParam2, uiParam3);
			break;

		case MESSAGE_UPDATE_CURRENTIME:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_CURRENTTIME, uiParam1);
			break;

		case MESSAGE_OPEN_PROJECT_PROGRESS:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_OPEN_PROJECT, uiParam1);
			break;

		case MESSAGE_OPEN_PROJECT_DONE:
		{
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_OPEN_PROJECT, 100);
			break;
		}

		case MESSAGE_CLOSE_PROJECT_DONE:
		{
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_CLOSE_PROJECT, 100);
			break;
		}

		case MESSAGE_ENCODE_PROJECT_PROGRESS:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_ENCODE_PROJECT, uiParam1);
			break;
			
		case MESSAGE_ENCODE_PROJECT_DONE:
			if( uiParam1 == 0 )
			{
				m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_ENCODE_PROJECT, uiParam1);
			}
			else
			{
				if( m_pProjectManager )
					m_pProjectManager->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);				
				m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_ENCODE_ERROR, uiParam1);
			}
			break;

		case MESSAGE_ADD_CLIP_PROGRESS:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_ADDCLIP_DONE);
			break;
		case MESSAGE_ADD_CLIP_DONE:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_ADDCLIP_DONE, uiParam1, uiParam2, uiParam3, uiParam4);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send event AddClipDone(0x%x)", __LINE__, uiParam1);
			break;

		case MESSAGE_SAVE_PROJECT_DONE:
		case MESSAGE_MOVE_CLIP_PROGRESS:
			break;
		case MESSAGE_MOVE_CLIP_DONE:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_MOVECLIP_DONE, uiParam1, uiParam2);
			break;
		case MESSAGE_UPDATE_CLIP_DONE:
		{
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_UPDATECLIP_DONE, uiParam1);
			break;
		}
		case MESSAGE_DELETE_CLIP_PROGRESS:
			break;
		case MESSAGE_DELETE_CLIP_DONE:
		{
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_DELETECLIP_DONE, uiParam1);
			break;
		}
		case MESSAGE_GET_DURATION_DONE:
			break;

		case MESSAGE_SET_TIME_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send event SetTimeDone(%d %d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3, uiParam4);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_SETTIME_DONE, uiParam1, uiParam2, uiParam3, uiParam4);
			break;
			
		case MESSAGE_LOAD_THEME_FILE_DONE:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_THEME_LOAD_DONE, uiParam1);
			break;
			
		case MESSAGE_CLIP_PLAY:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_PLAY_ERROR, uiParam1, uiParam2);
			break;
			
		case MESSAGE_CLIP_PLAY_END:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_PLAY_END);
			break;
		case MESSAGE_CMD_MARKER_CHECKED:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_CMD_MARKER_CHECKED, uiParam1);
			break;
		case MESSAGE_VIDEO_STARTED:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_VIDEO_STARTED);
			break;
		case MESSAGE_GETCLIPINFO_BACKGROUND_DONE:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_GETCLIPINFO_DONE, uiParam1, uiParam2, uiParam3);
			break;
		case MESSAGE_CAPTURE_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send event capture done(%d)", __LINE__, uiParam1);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_CAPTURE_DONE, uiParam1);
			break;
		case MESSAGE_PREPARE_CLIP_LOADING:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send event prepare clip loading(clip id%d)", __LINE__, uiParam1);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_PREPARE_CLIP_LOADING, uiParam1);
			break;
		case MESSAGE_VDEC_INIT_FAIL:
		case MESSAGE_ADEC_INIT_FAIL:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send event decoder init failed", __LINE__);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_PLAY_ERROR, uiParam1, uiParam2, uiParam3);
			break;

		case MESSAGE_VDEC_DEC_FAIL:
		case MESSAGE_ADEC_DEC_FAIL:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send event decoder failed", __LINE__);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_PLAY_ERROR, uiParam1, uiParam2, uiParam3);
			break;
		case MESSAGE_ARAL_INIT_FAIL:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send event renderer init failed", __LINE__);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_PLAY_ERROR, uiParam1, uiParam2, uiParam3);
			break;
		case MESSAGE_AUDIO_RENDER_FAIL:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send event audio renderer failed", __LINE__);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_PLAY_ERROR, uiParam1, uiParam2, uiParam3);
			break;

		case MESSAGE_TRANSCODING_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_TRANSCODING_DONE event(%d %d)", __LINE__, uiParam1, uiParam2);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_TRANSCODING_DONE, uiParam1, uiParam2);
			break;
			
		case MESSAGE_TRANSCODING_PROGRESS:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_TRANSCODING_PROGRESS event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_TRANSCODING_PROGRESS, uiParam1, uiParam2, uiParam3);
			break;

		case MESSAGE_SET_PROJECT_VOLUME_FADE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_SET_PROJECT_VOLUME_FADE event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_PROJECT_VOLUME_FADE, uiParam1, uiParam2, uiParam3);
			break;

		case MESSAGE_FAST_OPTION_PREVIEW_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_FAST_OPTION_PREVIEW_DONE event(%d %d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3, uiParam4);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_FAST_OPTION_PREVIEW_DONE, uiParam1, uiParam2, uiParam3, uiParam4);
			break;
		case MESSAGE_GETTHUMB_RAWDATA_DONE:
			m_pVideoEditorEventHandler->notifyEvent(THUMBNAIL_GEETTHUMB_RAWDATA_DONE, uiParam1, uiParam2, uiParam3);
			break;			
		case MESSAGE_MAKE_HIGHLIGHT_START_DONE:
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_DONE, uiParam1, uiParam2);
			break;			
		case MESSAGE_MAKE_HIGHLIGHT_PROGRESS_INDEX:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_TRANSCODING_PROGRESS event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_PROGRESS_INDEX, uiParam1, uiParam2, uiParam3);
			break;			
		case MESSAGE_MAKE_HIGHLIGHT_PROGRESS:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_MAKE_HIGHLIGHT_PROGRESS event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_PROGRESS, uiParam1, uiParam2, uiParam3);
			break;			
			
		case MESSAGE_MAKE_CHECK_DIRECT_EXPORT_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_MAKE_CHECK_DIRECT_EXPORT_DONE event(%d %d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3, uiParam4);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_CHECK_DIRECT_EXPORT, uiParam1, uiParam2, uiParam3, uiParam4);
			break;
			
		case MESSAGE_MAKE_DIRECT_EXPORT_DONE:
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_MAKE_DIRECT_EXPORT_DONE event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
		
				if( uiParam1 != NEXVIDEOEDITOR_ERROR_NONE )
				{
					if( m_pProjectManager != NULL)
						m_pProjectManager->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
				}
				m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_DIRECT_EXPORT_DONE, uiParam1, uiParam2, uiParam3);
			}
			break;

		case MESSAGE_MAKE_DIRECT_EXPORT_PROGRESS:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send VIDEOEDITOR_EVENT_DIRECT_EXPORT_PROGRESS event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_DIRECT_EXPORT_PROGRESS, uiParam1, uiParam2, uiParam3);
			break;
			
		case MESSAGE_GETCLIPINFO_BACKGROUND_STOP_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_GETCLIPINFO_BACKGROUND_STOP_DONE event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_GETCLIPINFO_STOP_DONE, uiParam1, uiParam2, uiParam3);
			break;
			
		case MESSAGE_ASYNC_UPDATE_CLIPLIST_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_ASYNC_UPDATE_CLIPLIST_DONE event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_ASYNC_LOADLIST, uiParam1, uiParam2, uiParam3);
			break;
			
		case MESSAGE_FAST_PREVIEW_START_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_FAST_PREVIEW_START_DONE event(%d %d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3, uiParam4);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_FAST_PREVIEW_START_DONE, uiParam1, uiParam2, uiParam3);
			break;

		case MESSAGE_FAST_PREVIEW_STOP_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_FAST_PREVIEW_STOP_DONE event(%d %d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3, uiParam4);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_FAST_PREVIEW_STOP_DONE, uiParam1);
			break;

		case MESSAGE_FAST_PREVIEW_TIME_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_FAST_PREVIEW_TIME_DONE event(%d %d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3, uiParam4);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_FAST_PREVIEW_TIME_DONE, uiParam1);
			break;
		case MESSAGE_HIGHLIGHT_THUMBNAIL_PROGRESS:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_HIGHLIGHT_THUMBNAIL_PROGRESS event(%d %d %d)", __LINE__, uiParam1, uiParam2);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_HIGHLIGHT_THUMBNAIL_PROGRESS, uiParam1, uiParam2);
			break;
		case MESSAGE_MAKE_REVERSE_START_DONE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_MAKE_REVERSE_START_DONE event(%d %d)", __LINE__, uiParam1, uiParam2);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_MAKE_REVERSE_DONE, uiParam1, uiParam2);
			break;
		case MESSAGE_MAKE_REVERSE_PROGRESS:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_TRANSCODING_PROGRESS event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_MAKE_REVERSE_PROGRESS, uiParam1, uiParam2, uiParam3);
			break;
		case MESSAGE_PREVIEW_PEAKMETER:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Send MESSAGE_PREVIEW_PEAKMETER event(%d %d %d)", __LINE__, uiParam1, uiParam2, uiParam3);
			m_pVideoEditorEventHandler->notifyEvent(VIDEOEDITOR_EVENT_PREVIEW_PEAKMETER, uiParam1, uiParam2, uiParam3);
			break;
		default:
			break;

	};

	// m_pVideoEditorEventHandler->notifyEvent(uiEventType, uiParam1, uiParam2, uiParam3, uiParam4);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::notifyError(unsigned int uiEventType, unsigned int uiResult, unsigned int uiParam1, unsigned int uiParam2)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}
	m_pVideoEditorEventHandler->notifyError(uiEventType, uiResult, uiParam1, uiParam2);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::callbackCapture(int iWidth, int iHeight, int iSize, char* pBuffer)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}
	return m_pVideoEditorEventHandler->callbackCapture(iWidth, iHeight, iSize, pBuffer);
}

int CNexVideoEditor::callbackGetThemeFile(char* pImageFilePath, unsigned int uiFileLen, int* pLength, char** ppImageData)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}

	m_pVideoEditorEventHandler->callbackGetThemeFile(pImageFilePath, uiFileLen, pLength, ppImageData);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::callbackGetThemeImage(char* pImageFilePath, int asyncmode, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}

	m_pVideoEditorEventHandler->callbackGetThemeImage(pImageFilePath, asyncmode, uiFileLen, pWidth, pHeight, pBitForPixel, ppImageData, pImageDataSize, ppUserData);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::callbackCheckImageWorkDone(){

	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}

	return m_pVideoEditorEventHandler->callbackCheckImageWorkDone();
}

int CNexVideoEditor::callbackGetImageUsingFile(char* pImageFilePath, int iThumb, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData, int* pLoadedType)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}

	m_pVideoEditorEventHandler->callbackGetImageUsingFile(pImageFilePath, iThumb, uiFileLen, pWidth, pHeight, pBitForPixel, ppImageData, pImageDataSize, ppUserData, pLoadedType);
	if (*ppImageData == NULL && *pImageDataSize == 0)
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::callbackGetImageUsingText(char* pText, unsigned int uiTextLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppTextImageData, int* pTextImageDataSize, void** ppUserData)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}

	m_pVideoEditorEventHandler->callbackGetImageUsingText(pText, uiTextLen, pWidth, pHeight, pBitForPixel, ppTextImageData, pTextImageDataSize, ppUserData);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::callbackReleaseImage(void** ppUserData)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}
	m_pVideoEditorEventHandler->callbackReleaseImage(ppUserData);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::callbackGetImageHeaderUsingFile(char* pImageFilePath, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}

	m_pVideoEditorEventHandler->callbackGetImageHeaderUsingFile(pImageFilePath, uiFileLen, pWidth, pHeight, pBitForPixel);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

void* CNexVideoEditor::callbackGetAudioTrack(int iSampleRate, int iChannels)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NULL;
	}
	void* pAudioTrack = m_pVideoEditorEventHandler->callbackGetAudioTrack(iSampleRate, iChannels);
	return pAudioTrack;
}

void CNexVideoEditor::callbackReleaseAudioTrack()
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return;
	}
	m_pVideoEditorEventHandler->callbackReleaseAudioTrack();
}

void* CNexVideoEditor::callbackGetAudioManager()
{
	if( m_pVideoEditorEventHandler == NULL)
	{
		return NULL;
 	}

	void* pAudioManager = m_pVideoEditorEventHandler->callbackGetAudioManager();
    return pAudioManager;
}

void CNexVideoEditor::callbackReleaseAudioManager()
{
	if( m_pVideoEditorEventHandler == NULL)
	{
		return ;
 	}
	m_pVideoEditorEventHandler->callbackReleaseAudioManager();
}

int CNexVideoEditor::getLUTWithID(int lut_resource_id, int export_flag){

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getLUTWithID(0x%x)", __LINE__, lut_resource_id);
	if( m_pVideoEditorEventHandler == NULL)
	{
		return 0;
 	}
	
	return m_pVideoEditorEventHandler->getLUTWithID(lut_resource_id, export_flag);	
}

int CNexVideoEditor::callbackGetEffectImagePath(const char* input, char* output)
{

//	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] callbackGetEffectImagePath()", __LINE__);
#if defined(__APPLE__)
    static const int RESULT_NOCHANGE = 0;
    // iOS SDK: path in input should be used as it as
    return RESULT_NOCHANGE;
#elif defined(_ANDROID)
    int prefix_len = strlen("[ThemeImage]com.nexstreaming.kmsdk.");
    if( strncmp(input,"[ThemeImage]com.nexstreaming.kmsdk.",prefix_len) == 0 ){
/*
       	if( m_pVideoEditorEventHandler == NULL)
    	{
	    	return 0;
 	    }

        m_pVideoEditorEventHandler->getAssetResourceKey(input,output);
*/
        char szbuf[256];
        
        //char szout[256];
        size_t next = 0;
        
        char * file = strchr(input,'/');
        
        if( file == NULL ){
            return 0;
        }
       
            
        //nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] callbackGetEffectImagePath(%s)", __LINE__, output);
        
        memcpy( szbuf, input,  file - input );
        szbuf[file - input] = 0x00;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] callbackGetEffectImagePath szBuf(%s)", __LINE__, szbuf);

		char *force_effect = strstr(szbuf,".force_effect");
		if( force_effect != NULL ) {

			szbuf[force_effect-szbuf] = 0x00;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] callbackGetEffectImagePath szBuf2(%s)", __LINE__, szbuf);
		}



        char *last = strrchr(szbuf, '.');
        if( last ){
            size_t len = last - szbuf;
            //memcpy(szbuf,szbuf,len);
            szbuf[len] =0x00;
        }
    
        char *point = strstr(szbuf,".effect.");
        if( point == NULL ){
            point = strstr(szbuf,".transition.");
            next = strlen(".transition");
        }else{
            next = strlen(".effect");
        }
    
        if( point ){
            size_t len = point - szbuf;

            memcpy(output,szbuf,len);
            output[len] =0x00;
            strcat(output,point+next);
            strcat(output,file);
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] callbackGetEffectImagePath(%s)", __LINE__, output);
        }else{
            return 0;
        }
       /*
        char *dele = strrchr(output, '.');
        if( dele )
        {
            size_t len = dele - output;
            memcpy( output, input,  len );
            output[len] =0x00;
           
            strcat(output,file);
           //printf("[NEXVIDEOEDITOR_VideoEditor.cpp %d] callbackGetEffectImagePath() output('%s')", __LINE__,output);
            return 1;
        }
       */
       return 1;
       
    }

#endif    
    return 0;
}

int CNexVideoEditor::getVignetteTexID(int export_flag){

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getVignetteTexID()", __LINE__);
	if( m_pVideoEditorEventHandler == NULL)
	{
		return 0;
 	}
	
	return m_pVideoEditorEventHandler->getVignetteTexID(export_flag);	
}

int CNexVideoEditor::createRenderItem(int export_flag, const char* effect_id){

	NXT_HThemeRenderer renderer = export_flag?m_hExportRenderer:m_hRenderer;
	NXT_ThemeRenderer_AquireContext(renderer);
	int id = NXT_ThemeRenderer_GetRenderItemEffectID(renderer, effect_id);
	if(id < 0){

		char* poutput = NULL;
		int length = 0;
		if(0 == callbackGetThemeFile((char*)effect_id, strlen(effect_id), &length, &poutput)){

            if(length <= 0 || poutput == NULL){

                NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                return -1;
            }
            else{

                NXT_ThemeRenderer_GetRenderItem(renderer, effect_id, NULL, poutput, 0, LoadThemeFileCallback, this);
                id = NXT_ThemeRenderer_GetRenderItemEffectID(renderer, effect_id);
                if(poutput)
                    delete[] poutput;
            }
                
		}
        else{

            NXT_ThemeRenderer_ReleaseContext(renderer, 0);
            return -1;
        }
	}

	int ret = NXT_ThemeRenderer_CreateRenderItem(renderer, id);
	NXT_ThemeRenderer_ReleaseContext(renderer, 0);
	return ret;
}

int CNexVideoEditor::releaseRenderItem(int export_flag, int effect_id){

	NXT_HThemeRenderer renderer = export_flag?m_hExportRenderer:m_hRenderer;
	NXT_ThemeRenderer_ReleaseRenderItem(renderer, effect_id);

	return 0;
}

int CNexVideoEditor::drawRenderItemOverlay(int effect_id, int tex_id_for_second, int export_flag, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha_val, int mask_enabled){

	NXT_HThemeRenderer renderer = export_flag?m_hExportRenderer:m_hRenderer;
	NXT_ThemeRenderer_DrawRenderItemOverlay(renderer, effect_id, tex_id_for_second, effect_options, current_time, start_time, end_time, pmatrix, left, top, right, bottom, alpha_val, mask_enabled);
	return 0;
}


int CNexVideoEditor::createNexEDL(int export_flag, const char* effect_id){

	NXT_HThemeRenderer renderer = export_flag?m_hExportRenderer:m_hRenderer;

	NXT_ThemeRenderer_AquireContext(renderer);

	NXT_HThemeSet themeset = NXT_ThemeRenderer_GetKEDLEffectID(renderer, effect_id);
	if(themeset == NULL){

		char* poutput = NULL;
		int length = 0;
        if(0 == callbackGetThemeFile((char*)effect_id, strlen(effect_id), &length, &poutput)){

            if(length <= 0 || poutput == NULL)
                return -1;
            else {

                themeset = NXT_ThemeRenderer_GetKEDLItem(renderer, effect_id, poutput);
                if(poutput)
                    delete[] poutput;
            }
		}
        else
            return -1;
	}

	int ret = NXT_ThemeRenderer_CreateNexEDL(renderer, effect_id, themeset);
	NXT_ThemeRenderer_ReleaseContext(renderer, 0);
	return ret;
}

int CNexVideoEditor::releaseNexEDL(int export_flag, int effect_id){

	NXT_HThemeRenderer renderer = export_flag?m_hExportRenderer:m_hRenderer;
	NXT_ThemeRenderer_ReleaseRenderItem(renderer, effect_id);

	return 0;
}

int CNexVideoEditor::drawNexEDLOverlay(int effect_id, int export_flag, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha_val, int mode){

	NXT_HThemeRenderer renderer = export_flag?m_hExportRenderer:m_hRenderer;
	NXT_ThemeRenderer_DrawNexEDLOverlay(renderer, effect_id, effect_options, current_time, start_time, end_time, pmatrix, left, top, right, bottom, alpha_val, mode);
	return 0;
}

int CNexVideoEditor::callbackCustomLayer(	int iParam1, int iParam2, int iParam3, int iParam4, int iParam5, 
									int iParam6, int iParam7, int iParam8, int iParam9, int iParam10, 
									int iParam11, int iParam12, int iParam13, int iParam14, int iParam15, int iParam16, int iParam17, int iParam18 )
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] callbackCustomLayer(0x%x)", __LINE__, m_pVideoEditorEventHandler);
	if( m_pVideoEditorEventHandler == NULL)
	{
		return 1;
 	}
	
	return m_pVideoEditorEventHandler->callbackCustomLayer(iParam1, iParam2, iParam3, iParam4, iParam5, iParam6, iParam7, iParam8, iParam9, iParam10, iParam11, iParam12, iParam13, iParam14, iParam15, iParam16, iParam17, iParam18);
}

int CNexVideoEditor::callbackThumb(int iMode, int iTag, int iTime, int iWidth, int iHeight, int iCount, int iTotal, int iSize, char* pBuffer)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}
	return m_pVideoEditorEventHandler->callbackThumb(iMode, iTag, iTime, iWidth, iHeight, iCount, iTotal, iSize, pBuffer);
}

int CNexVideoEditor::callbackHighLightIndex(int iCount, int* pBuffer)
{
	if( m_pVideoEditorEventHandler == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_EVENTHANDLER;
	}
	return m_pVideoEditorEventHandler->callbackHighLightIndex(iCount, pBuffer);
}

int CNexVideoEditor::getUDTACount()
{
#ifdef _ANDROID
	return (int)m_vecUDTA.size();
#endif
	return 0;
}

CNexUDTA* CNexVideoEditor::getUDTA(int iIndex)
{
#ifdef _ANDROID
	if( iIndex < 0 || iIndex >= m_vecUDTA.size() )
		return NULL;
	return &m_vecUDTA[iIndex];
#endif
	return NULL;
}

//yoon 
unsigned char * CNexVideoEditor::getVideoTrackUUID(int * pSize)
{
    unsigned char * ucUUID =  NULL;
    int iSize = 0;
  	if( m_pProjectManager == NULL )
	{
    	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getVideoTrackUUID(0x%x)", __LINE__, m_pVideoEditorEventHandler);
		return NULL;
	}

	CClipList* pClipList = m_pProjectManager->getClipList();
	if( pClipList )
	{
		pClipList->lockClipList();
		for( int i = 0; i < pClipList->getClipCount(); i++)
		{
			CClipItem* pClip = (CClipItem*)pClipList->getClip(i);
			if( pClip->isVideoExist() ){
			    ucUUID = pClip->getVideoTrackUUID(&iSize);
			    SAFE_RELEASE(pClip);
    			if( ucUUID != NULL && iSize > 0  )
	    		{
		    	    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getVideoTrackUUID(0x%x) getUUID=%d", __LINE__, m_pVideoEditorEventHandler,iSize);
			        *pSize =iSize;
			    }    
			    break;
            }			
			SAFE_RELEASE(pClip);
		}
		pClipList->unlockClipList();
		SAFE_RELEASE(pClipList);
	}
	return ucUUID;
}

int CNexVideoEditor::getVideoTrackUUIDMode()
{
    return iVideoTrackUUIDMode;
}

unsigned int CNexVideoEditor::getAudioSessionID()
{
	return m_pProjectManager ? m_pProjectManager->getAudioSessionID(): 0;
}

int CNexVideoEditor::setPreviewScaleFactor(float fPreviewScaleFactor){

	if(fPreviewScaleFactor <= 0.0f && fPreviewScaleFactor > 1.0f)
		return 1;

	m_fPreviewScaleFactor = fPreviewScaleFactor;
	return 0;
}

float CNexVideoEditor::getPreviewScaleFactor(){

	return m_fPreviewScaleFactor;
}

int CNexVideoEditor::pushLoadedBitmap(const char* path, int* pixels, int width, int height, int loadedtype){

	// if(!NXT_ThemeRenderer_CheckUploadOk(m_hRenderer))
	// 	return 1;
	NXT_Theme_SetTextureInfoAsync(m_hRenderer, path, pixels, width, height);
	return 0;
}

int CNexVideoEditor::removeBitmap(const char* path){

	NXT_Theme_RemoveTextureInfoAsync(m_hRenderer, path);
	return 0;
}

int CNexVideoEditor::setDrawInfoList(IDrawInfoList* master, IDrawInfoList* sub)
{
	if( m_pProjectManager )
	{
		CNxMsgSetDrawInfoList* pDrawInfoList = new CNxMsgSetDrawInfoList(master, sub);

		if( pDrawInfoList )
		{
			m_pProjectManager->SendCommand(pDrawInfoList);
			SAFE_RELEASE(pDrawInfoList);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

int CNexVideoEditor::setDrawInfo(IDrawInfo* drawinfo)
{
	if( m_pProjectManager )
	{
		CNxMsgSetDrawInfo* pDrawInfo = new CNxMsgSetDrawInfo(drawinfo);

		if( pDrawInfo )
		{
			m_pProjectManager->SendCommand(pDrawInfo);
			SAFE_RELEASE(pDrawInfo);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	return NEXVIDEOEDITOR_ERROR_NO_ACTION;
}

IDrawInfoList* CNexVideoEditor::createDrawInfoList()
{
	CNexDrawInfoVec* pList = new CNexDrawInfoVec();
	return (IDrawInfoList*)pList;
}

IDrawInfo* CNexVideoEditor::createDrawInfo()
{
	IDrawInfo* pInfo = new CNexDrawInfo();
	return pInfo;
}

int CNexVideoEditor::updateRenderInfo(unsigned int clipid, unsigned int face_detected, RECT& start, RECT& end, RECT& face){

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] updateRenderInfo In(%d, start(%d, %d, %d, %d) end(%d, %d, %d, %d))", __LINE__, clipid, start.left, start.top, start.right, start.bottom, end.left, end.top, end.right, end.bottom);;
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] updateRenderInfo failed(reason: Project was not created)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}
	CNxMsgUpdateRenderInfo* updateRenderInfo = new CNxMsgUpdateRenderInfo(clipid, face_detected, start, end, face);
	if(updateRenderInfo)
	{
		m_pProjectManager->SendCommand(updateRenderInfo);
		SAFE_RELEASE(updateRenderInfo);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] updateRenderInfo Out", __LINE__);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexVideoEditor::resetFaceDetectInfo(unsigned int clipid){

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] resetFaceDetectInfo In(%d))", __LINE__, clipid);;
	if( m_pProjectManager == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] resetFaceDetectInfo failed(reason: Project was not created)", __LINE__);
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}
	CNxMsgResetFaceDetectInfo* updateRenderInfo = new CNxMsgResetFaceDetectInfo(clipid);
	if(updateRenderInfo)
	{
		m_pProjectManager->SendCommand(updateRenderInfo);
		SAFE_RELEASE(updateRenderInfo);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] resetFaceDetectInfo Out", __LINE__);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

void CNexVideoEditor::setIsGLOperationAllowed(bool allowed)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setIsGLOperationAllowed(%s)", __LINE__, allowed?"TRUE":"FALSE");
	CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
	if( pVideoRender )
	{
		pVideoRender->setIsGLOperationAllowed(allowed);
		SAFE_RELEASE(pVideoRender);
	}
}

int CNexVideoEditor::setEncInfo(const int infoCnt, const NXCHAR* pInfo[]){
	int	i=0, iLen=0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] setEncInfo In) Cnt=%d", __LINE__, infoCnt);

	if ( m_pEncInfo && m_iEncInfoCnt ) {
		for( i=0 ; i<m_iEncInfoCnt ; i++ ) {
			if ( m_pEncInfo[i] ) delete m_pEncInfo[i];
		}
		delete m_pEncInfo;
	}
	m_iEncInfoCnt = infoCnt;
	m_pEncInfo = new NXCHAR*[m_iEncInfoCnt];
	
	for (i=0 ; i<m_iEncInfoCnt ; i++) {
		iLen = strlen(pInfo[i]);
		
		m_pEncInfo[i] = new NXCHAR[iLen+1];
		if ( m_pEncInfo[i] == NULL ) {
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] m_pEncInfo[%d] is null!)", __LINE__, i);
			return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		}
		memset(m_pEncInfo[i], 0x00, iLen+1);
		memcpy(m_pEncInfo[i], pInfo[i], iLen);
	}

#if 0
	for (i=0 ; i<m_iEncInfoCnt ; i++) {
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] pInfo[%d]=[%s]\n", __LINE__, i, m_pEncInfo[i]);
	}
#endif
	return NEXVIDEOEDITOR_ERROR_NONE;
}

//---------------------------------------------------------------------------

void print_conf(NexConf* conf)
{
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "-------------------------------------------\n");
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[LOG]\n");
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "conf->log.engine: %d\n", conf->log.engine);
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "conf->log.codec: %d\n", conf->log.codec);
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "conf->log.themerender: %d\n", conf->log.themerender);

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "\n[PROPERTY]\n");
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "conf->properties.video_maxwidth: %d\n", conf->properties.video_maxwidth);
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "conf->properties.video_maxheight: %d\n", conf->properties.video_maxheight);
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "conf->properties.videobuffer_maxsize: %d\n", conf->properties.videobuffer_maxsize);
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "conf->properties.hw_dec_count: %d\n", conf->properties.hw_dec_count);
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "conf->properties.hw_enc_count: %d\n", conf->properties.hw_enc_count);
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "-------------------------------------------\n");
}

static int g_iUserData = 0;

INexVideoEditor* getNexVideoEditor(){

	return CNexVideoEditor::getVideoEditor();
}

INexVideoEditor* CreateNexVideoEditor(int iMajor, int iMinor, char* strLibPath, char* strModelName, int iAPILevel, int iUserData, int* pProperties, void* pVM, void* wstl)
{
#ifdef _ANDROID

	if( NEXEDITOR_INERFACE_MAJOR_VERSION != iMajor || NEXEDITOR_INERFACE_MINOR_VERSION != iMinor || strLibPath == NULL )
	{
		LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] Version mismatch EngineVersion(%d %d) (%d %d)", __LINE__, NEXEDITOR_INERFACE_MAJOR_VERSION, NEXEDITOR_INERFACE_MINOR_VERSION, iMajor, iMinor);
		return NULL;
	}

	CNexVideoEditor::m_isRGBADevice = getRGBADeviceProperty();

	LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] CreateNexVideoEditor UserData(0x%x 0x%x) Version(%d %d %d)", __LINE__, iUserData, g_iUserData ^ 0x10101010, NEXEDITOR_MAJOR_VERSION, NEXEDITOR_MINOR_VERSION, NEXEDITOR_PATCH_VERSION);
	if( iUserData == (g_iUserData ^ 0x84562845) )
	{
		SUPPORT_DEVICE_INFO* pDeviceInfo = getSupportDeviceInfo(strModelName);
		if( pDeviceInfo != NULL )
		{
			CNexVideoEditor::m_iSupportedWidth						= pDeviceInfo->m_iSupportWidth;
			CNexVideoEditor::m_iSupportedHeight						= pDeviceInfo->m_iSupportHeight;
			CNexVideoEditor::m_iSupportedMaxProfile					= pDeviceInfo->m_iSupportH264Profile;

			CNexVideoEditor::m_iSupportDecoderMaxCount				= pDeviceInfo->m_iSupportDecoderMaxCnt;
			CNexVideoEditor::m_iSupportEncoderMaxCount				= pDeviceInfo->m_iSupportEncoderMaxCnt;
			CNexVideoEditor::m_iUseSurfaceMediaSource				= pDeviceInfo->m_iUseSurfaceMediaSource;	

			LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] CreateNexVideoEditor with Support Info", __LINE__);
			LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] SupportWH(%d %d) Support HW Cnt(%d %d %d) ExtraInfo(%d)", __LINE__,
				CNexVideoEditor::m_iSupportedWidth,
				CNexVideoEditor::m_iSupportedHeight,
				CNexVideoEditor::m_iSupportedMaxProfile,
				CNexVideoEditor::m_iSupportDecoderMaxCount,
				CNexVideoEditor::m_iSupportEncoderMaxCount,	
				CNexVideoEditor::m_iUseSurfaceMediaSource);
		}
		else
		{
			LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] CreateNexVideoEditor Support Info", __LINE__);
			LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] SupportWH(%d %d) Support HW Cnt(%d %d %d) ExtraInfo(%d)", __LINE__,
				CNexVideoEditor::m_iSupportedWidth,
				CNexVideoEditor::m_iSupportedHeight,
				CNexVideoEditor::m_iSupportedMaxProfile,
				CNexVideoEditor::m_iSupportDecoderMaxCount,
				CNexVideoEditor::m_iSupportEncoderMaxCount,	
				CNexVideoEditor::m_iUseSurfaceMediaSource);
		}
	}
	else if( iUserData == (g_iUserData ^ 0x20202020) )
	{
		SUPPORT_DEVICE_INFO* pDeviceInfo = getSupportDeviceInfo(strModelName);
		if( pDeviceInfo == NULL )
		{
			LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] CreateNexVideoEditor Not Supported device", __LINE__);
			return NULL;
		}
		
		CNexVideoEditor::m_iSupportedWidth						= pDeviceInfo->m_iSupportWidth;
		CNexVideoEditor::m_iSupportedHeight						= pDeviceInfo->m_iSupportHeight;
		CNexVideoEditor::m_iSupportedMaxProfile					= pDeviceInfo->m_iSupportH264Profile;

		CNexVideoEditor::m_iSupportDecoderMaxCount				= pDeviceInfo->m_iSupportDecoderMaxCnt;
		CNexVideoEditor::m_iSupportEncoderMaxCount				= pDeviceInfo->m_iSupportEncoderMaxCnt;
		CNexVideoEditor::m_iUseSurfaceMediaSource				= pDeviceInfo->m_iUseSurfaceMediaSource;	

		LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] CreateNexVideoEditor Support Info", __LINE__);
		LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] SupportWH(%d %d) Support HW Cnt(%d %d %d) ExtraInfo(%d)", __LINE__,
			CNexVideoEditor::m_iSupportedWidth,
			CNexVideoEditor::m_iSupportedHeight,
			CNexVideoEditor::m_iSupportedMaxProfile,
			CNexVideoEditor::m_iSupportDecoderMaxCount,
			CNexVideoEditor::m_iSupportEncoderMaxCount,	
			CNexVideoEditor::m_iUseSurfaceMediaSource);
	}
	else
	{
		LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] CreateNexVideoEditor Not Supported device", __LINE__);
		return NULL;
	}
#endif
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] CreateNexVideoEditor Version(%d %d %d)",
		__LINE__, NEXEDITOR_MAJOR_VERSION, NEXEDITOR_MINOR_VERSION, NEXEDITOR_PATCH_VERSION);

	INexVideoEditor* pINexVideoEditor = (INexVideoEditor*)new CNexVideoEditor(strLibPath, strModelName, iAPILevel, pProperties, pVM, wstl);
	if( pINexVideoEditor == NULL )
		return NULL;
	return pINexVideoEditor;
}

INexVideoEditor* CreateNexThumb(int iMajor, int iMinor, char* strLibPath, char* strModelName, int iAPILevel, int* pProperties, void* pVM, void* wstl)
{
	if( NEXEDITOR_MAJOR_VERSION	!= iMajor || NEXEDITOR_MINOR_VERSION != iMinor || strLibPath == NULL )
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] Version mismatch EngineVersion(%d %d) (%d %d)", __LINE__, NEXEDITOR_MAJOR_VERSION, NEXEDITOR_MINOR_VERSION, iMajor, iMinor);
		return NULL;
	}

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] CreateNexVideoEditor Version(%d %d %d)",
		__LINE__, NEXEDITOR_MAJOR_VERSION, NEXEDITOR_MINOR_VERSION, NEXEDITOR_PATCH_VERSION);

	INexVideoEditor* pINexVideoEditor = (INexVideoEditor*)new CNexVideoEditor(strLibPath, strModelName, iAPILevel, pProperties, pVM,wstl);
	if( pINexVideoEditor == NULL )
		return NULL;
	return pINexVideoEditor;
}

int InitUserData()
{
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] InitUserData", __LINE__);

	srand(rand());

	g_iUserData = rand();
	return g_iUserData;
}

int getSystemProperty(const char* pStrName, char* pValue)
{
#ifdef _ANDROID
	if( getSupportSystemProperty(pStrName, pValue) )
		return NEXVIDEOEDITOR_ERROR_NONE;
#endif
	return NEXVIDEOEDITOR_ERROR_UNKNOWN;
}
