/******************************************************************************
 * File Name   : ObjCNexEditorEventHandler.mm
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

#import "NexSAL_Internal.h"
#import "NexEditorLog.h"
#import "ImageLoaderProtocol.h"
#import "CustomLayerProtocol.h"
#import "ObjCNexEditorEventHandler.h"

#define LOG_TAG @"ObjCNexEditorEventHandler"

ObjCNexEditorEventHandler::ObjCNexEditorEventHandler(id<NexEditorEventDelegate> listener)
{
    eventListener = listener;
}

ObjCNexEditorEventHandler::~ObjCNexEditorEventHandler()
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[nexEditorEventHandler.cpp  %d] ~ObjCNexEditorEventHandler In", __LINE__);
    
    eventListener = nil;
    
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[nexEditorEventHandler.cpp  %d] ~ObjCNexEditorEventHandler Out", __LINE__);
}

int ObjCNexEditorEventHandler::notifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3, unsigned int uiParam4)
{
    return [eventListener notifyEvent:uiEventType param1:uiParam1 param2:uiParam2 param3:uiParam3 param4:uiParam4];
}

int ObjCNexEditorEventHandler::notifyError(unsigned int uiEventType, unsigned int uiResult, unsigned int uiParam1, unsigned int uiParam2)
{
    return [eventListener notifyError:uiEventType result:uiResult param1:uiParam1 param2:uiParam2];
}

int ObjCNexEditorEventHandler::callbackCapture(int iWidth, int iHeight, int iSize, char* pBuffer)
{
    return [eventListener callbackCapture:iWidth height:iHeight size:iSize data:pBuffer];
}

int ObjCNexEditorEventHandler::callbackThumb(int iMode, int iTag, int iTime, int iWidth, int iHeight, int iCount, int iTotal, int iSize, char* pBuffer)
{
    return [eventListener callbackThumb:iMode tag:iTag time:iTime width:iWidth height:iHeight count:iCount total:iTotal size:iSize data:pBuffer];
}

int ObjCNexEditorEventHandler::callbackHighLightIndex(int iCount, int* pBuffer)
{
    return [eventListener callbackHighLightIndex:iCount data:pBuffer];
}

void ObjCNexEditorEventHandler::callbackGetThemeImage(char* pImageFilePath, int asyncmode, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData)
{
    if( pImageFilePath == NULL || uiFileLen <= 0 || pWidth == NULL || pHeight == NULL || pBitForPixel == NULL || ppImageData == NULL || pImageDataSize == NULL )
        return;
    
    id<ImageLoaderProtocol> imageLoader = [eventListener getImageLoader];
    
    NSUInteger width, height;
    void *pixelData = NULL;
    void *imageResource = NULL;
    
    *pWidth = 0;
    *pHeight = 0;
    *pBitForPixel = 0;
    *pImageDataSize = 0;
    *ppImageData = NULL;

    NSString *pathInfo = [NSString stringWithUTF8String:pImageFilePath];
    
    if (strstr(pImageFilePath, "[Text]") && strstr(pImageFilePath, ";;")) {
        size_t l1 = strlen(pImageFilePath);
        size_t l2 = strlen(strstr(pImageFilePath, ";;"));
        
        char *pDesc = (char *)calloc((l1 - l2)+1, sizeof(char));
        char *pText = strstr(pImageFilePath, ";;") + 2;
        
        strncpy(pDesc, pImageFilePath, (l1 - l2));
        
        NSString *desc = [NSString stringWithUTF8String:pDesc];
        NSString *text = [NSString stringWithUTF8String:pText];
        
        if ( [imageLoader openThemeTextDesc:desc text:text width:&width height:&height pixelData:&pixelData imageResource:&imageResource] ) {
            *pWidth = (int)width;
            *pHeight = (int)height;
            *pBitForPixel = 32;
            *pImageDataSize = (int)(width * height * 4);
            *ppImageData = (unsigned char*)pixelData;
            *ppUserData = imageResource;
        }
        free(pDesc);
    }
    else {
        if( [imageLoader openThemeImage:pathInfo width:&width height:&height pixelData:&pixelData imageResource:&imageResource] ) {
            *pWidth = (int)width;
            *pHeight = (int)height;
            *pBitForPixel = 32;
            *pImageDataSize = (int)(width * height * 4);
            *ppImageData = (unsigned char*)pixelData;
            *ppUserData = imageResource;
        }
    }
}

/*  Notice !!!!!!
 *  Collage, template3.0의 경우 하나의 scene에 다수의 이미지를 로드하는 경우가 생기는데,
 *  이 때 다수의 원본의 데이터를 전달하게 되면 어느 정도의 시간이 걸리기에 이를 보안하기 위해 iThumb, pLoadedType 2개 변수가 추가되었다고 합니다.
 *  지금은 iThumb 값은 무시하고, pLoadedType에는 1값을 전달합니다.
 */
void ObjCNexEditorEventHandler::callbackGetImageUsingFile(char* pImageFilePath, int iThumb, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData, int* pLoadedType)
{
    if( pImageFilePath == NULL || uiFileLen <= 0 || pWidth == NULL || pHeight == NULL || pBitForPixel == NULL || ppImageData == NULL || pImageDataSize == NULL )
        return;
    
    id<ImageLoaderProtocol> imageLoader = [eventListener getImageLoader];
    
    NSUInteger width, height;
    void *pixelData = NULL;
    void *imageResource = NULL;
    
    *pWidth = 0;
    *pHeight = 0;
    *pBitForPixel = 0;
    *pImageDataSize = 0;
    *ppImageData = NULL;
    *pLoadedType = 1;
    
    if( [imageLoader openFile:[NSString stringWithUTF8String:pImageFilePath] width:&width height:&height pixelData:&pixelData imageResource:&imageResource] ) {
        *pWidth = (int)width;
        *pHeight = (int)height;
        *pBitForPixel = 32;
        *pImageDataSize = (int)(width * height * 4);
        *ppImageData = (unsigned char*)pixelData;
        *ppUserData = imageResource;
    }
}

void ObjCNexEditorEventHandler::callbackGetImageUsingText(char* pText, unsigned int uiTextLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppTextImageData, int* pTextImageDataSize, void** ppUserData)
{
    if( pText == NULL || uiTextLen <= 0 || pWidth == NULL || pHeight == NULL || pBitForPixel == NULL || ppTextImageData == NULL || pTextImageDataSize == NULL )
        return;
    
    id<ImageLoaderProtocol> imageLoader = [eventListener getImageLoader];
    
    NSUInteger width, height;
    void *pixelData = NULL;
    void *imageResource = NULL;
    
    *pWidth = 0;
    *pHeight = 0;
    *pBitForPixel = 0;
    *pTextImageDataSize = 0;
    *ppTextImageData = NULL;

    if( [imageLoader openFile:[NSString stringWithUTF8String:pText] width:&width height:&height pixelData:&pixelData imageResource:&imageResource] ) {
        *pWidth = (int)width;
        *pHeight = (int)height;
        *pBitForPixel = 32;
        *pTextImageDataSize = (int)(width * height * 4);
        *ppTextImageData = (unsigned char*)pixelData;
        *ppUserData = imageResource;
    }
}

void ObjCNexEditorEventHandler::callbackReleaseImage(void** ppUserData)
{
    id<ImageLoaderProtocol> imageLoader = [eventListener getImageLoader];
    [imageLoader releaseImageResource:*ppUserData];
    return;
}

void ObjCNexEditorEventHandler::callbackReleaseAudioManager()
{
    // TODO. we must know the purpose of this callback
    return;
}

int ObjCNexEditorEventHandler::callbackCustomLayer(int iParam1, int iParam2, int iParam3, int iParam4,
                                                   int iParam5, int iParam6, int iParam7, int iParam8,
                                                   int iParam9, int iParam10, int iParam11, int iParam12,
                                                   int iParam13, int iParam14, int iParam15, int iParam16,
                                                   int iParam17, int iParam18)
{
    id callbackCustomLayer = [eventListener callbackCustomLayer:iParam1 param2:iParam2 param3:iParam3 param4:iParam4
                                                         param5:iParam5 param6:iParam6 param7:iParam7 param8:iParam8
                                                         param9:iParam9 param10:iParam10 param11:iParam11 param12:iParam12
                                                        param13:iParam13 param14:iParam14 param15:iParam15 param16:iParam16
                                                        param17:iParam17 param18:iParam18];
    if(callbackCustomLayer == nil) {
        NexLogE(LOG_TAG, @"callbackCustomLayer is fail");
        return 0;
    }
    
    return 1;
}

int ObjCNexEditorEventHandler::callbackUserRequest(int type, void * userData)
{
//    id<CustomLayerProtocol> customLayer = [eventListener getCustomLayerLoader];
//
//    [customLayer clearResourceInRenderContext:<#(NexLayerRenderContextType)#>]
    return 0;
}

int ObjCNexEditorEventHandler::callbackHighLightIndexForVAS(int iCount, int *pBuffer, int *pBuffer2)
{
    // Dont need to implement this function because of the android feature
    return 1;
}

void ObjCNexEditorEventHandler::callbackGetThemeFile(char* pImageFilePath, unsigned int uiFileLen, int* pLength, char** ppImageData)
{
    id<ImageLoaderProtocol> imageLoader = [eventListener getImageLoader];
    
    NSUInteger dataLength;
    void *fileData = NULL;
    
    *pLength = 0;
    *ppImageData = NULL;

    if( [imageLoader openThemeFile:[NSString stringWithUTF8String:pImageFilePath] dataLength:&dataLength fileData:&fileData] ) {
        *pLength = (int)dataLength;
        *ppImageData = (char *)fileData;
    }
}

void ObjCNexEditorEventHandler::callbackGetImageHeaderUsingFile(char* pImageFilePath, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel)
{
    if( pImageFilePath == NULL || uiFileLen <= 0 || pWidth == NULL || pHeight == NULL || pBitForPixel == NULL )
        return;
    
    id<ImageLoaderProtocol> imageLoader = [eventListener getImageLoader];
    
    NSUInteger width, height;
    void *pixelData = NULL;
    void *imageResource = NULL;
    
    *pWidth = 0;
    *pHeight = 0;
    *pBitForPixel = 0;

    if( [imageLoader openFile:[NSString stringWithUTF8String:pImageFilePath] width:&width height:&height pixelData:&pixelData imageResource:&imageResource] ) {
        *pWidth = (int)width;
        *pHeight = (int)height;
        *pBitForPixel = 32;
    }
}

void* ObjCNexEditorEventHandler::callbackGetAudioTrack(int iSampleRate, int iChannels)
{
    // Dont need to implement this function because of the android feature
    return NULL;
}

void ObjCNexEditorEventHandler::callbackReleaseAudioTrack()
{
    // Dont need to implement this function because of the android feature
    return;
}


void* ObjCNexEditorEventHandler::callbackGetAudioManager()
{
    // Dont need to implement this function because of the android feature
    return NULL;
}

int ObjCNexEditorEventHandler::getLUTWithID(int lut_resource_id, int export_flag)
{
    return [eventListener getLUTWithID:lut_resource_id exportFlag:export_flag];
}

int ObjCNexEditorEventHandler::getVignetteTexID(int export_flag)
{
    return [eventListener getVignetteTextID:export_flag];
}

int ObjCNexEditorEventHandler::getAssetResourceKey(const char * input_resourcpath, char *output_key)
{
    // Dont need to implement this function because of the android feature
    return 0;
}

int ObjCNexEditorEventHandler::callbackCheckImageWorkDone()
{
    // Hold because of not supporting the Async mode in NexThemeRenderer
    return 0;
}
