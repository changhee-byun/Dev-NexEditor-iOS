/******************************************************************************
 * File Name   : ObjCNexEditorEventHandler.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#ifndef ObjCNexEditorEventHandler_h
#define ObjCNexEditorEventHandler_h

#import "NEXVIDEOEDITOR_Interface.h"
#import "NEXVIDEOEDITOR_EventTypes.h"
#import "NexEditorEventDelegate.h"

//---------------------------------------------------------------------------
class ObjCNexEditorEventHandler : public CNxRef<INexVideoEditorEventHandler>
{
public:
    ObjCNexEditorEventHandler(id<NexEditorEventDelegate> listener);
    
    virtual ~ObjCNexEditorEventHandler();
    
    virtual int notifyEvent(unsigned int uiEventType, unsigned int uiParam1 = 0, unsigned int uiParam2 = 0, unsigned int uiParam3 = 0, unsigned int uiParam4 = 0);
    virtual int notifyError(unsigned int uiEventType, unsigned int uiResult, unsigned int uiParam1 = 0, unsigned int uiParam2 = 0);
    virtual int callbackCapture(int iWidth, int iHeight, int iSize, char* pBuffer);
    
    virtual void callbackGetThemeImage(char* pImageFilePath, int asyncmode, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData);
    virtual int callbackCheckImageWorkDone();
    virtual void callbackGetImageUsingFile(char* pImageFilePath, int iThumb, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData, int* pLoadedType);
    virtual void callbackGetImageUsingText(char* pText, unsigned int uiTextLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppTextImageData, int* pTextImageDataSize, void** ppUserData);
    virtual void callbackReleaseImage(void** ppUserData);
    virtual void callbackGetImageHeaderUsingFile(char* pImageFilePath, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel);
    
    virtual void* callbackGetAudioTrack(int iSampleRate, int iChannels);
    virtual void callbackReleaseAudioTrack();
    
    virtual void* callbackGetAudioManager();
    virtual void callbackReleaseAudioManager();
    
    /**
     * /param iParam1 shader
     * /param iParam2 cts
     * /param iParam3 a_position
     * /param iParam4 a_texCoord
     * /param iParam5 u_mvp_matrix
     * /param iParam6 u_tex_matrix
     * /param iParam7 u_alpha
     * /param iParam8 u_realX
     * /param iParam9 u_realY
     * /param iParam10 u_colorconv
     * /param iParam11 u_textureSampler
     * /param iParam12 u_textureSampler_for_mask
     * /param iParam13 u_ambient_light
     * /param iParam14 u_diffuse_light
     * /param iParam15 u_specular_light
     * /param iParam16 flags
     * /param iParam17 screen_dimension_width
     * /param iParam18 screen_dimension_height
     */
    virtual int callbackCustomLayer(int iParam1, int iParam2, int iParam3, int iParam4, int iParam5, 
                                    int iParam6, int iParam7, int iParam8, int iParam9, int iParam10, 
                                    int iParam11, int iParam12, int iParam13, int iParam14, int iParam15, int iParam16, int iParam17, int iParam18);
    virtual int callbackUserRequest(int type, void* userData);

    virtual int callbackThumb(int iMode, int iTag, int iTime, int iWidth, int iHeight, int iCount, int iTotal, int iSize, char* pBuffer);
    virtual int callbackHighLightIndex(int iCount, int* pBuffer);
    virtual int callbackHighLightIndexForVAS(int iCount, int* pBuffer, int* pBuffer2);
    virtual void callbackGetThemeFile(char* pImageFilePath, unsigned int uiFileLen, int* pLength, char** ppImageData);
    
    virtual int getLUTWithID(int lut_resource_id, int export_flag);
    virtual int getVignetteTexID(int export_flag);
    virtual int getAssetResourceKey(const char * input_resourcpath, char *output_key);
    
private:
    id<NexEditorEventDelegate> eventListener;   
};

#endif /* ObjCNexEditorEventHandler_h */
