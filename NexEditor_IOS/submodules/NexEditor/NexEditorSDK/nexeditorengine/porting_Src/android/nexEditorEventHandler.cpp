#include <jni.h>
#include "nexEditorEventHandler.h"
#include "string.h"
#include <android/bitmap.h>

#include <android/log.h>

#include "NexJNIEnvStack.h"

#define  LOG_TAG    "NEXEDITOR"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

jstring NewStringUTF(JNIEnv* env, char* text){
	
	size_t str_size = strlen(text);
	jbyteArray array = env->NewByteArray(str_size);
    env->SetByteArrayRegion(array, 0, str_size, (const jbyte*)text);


    jstring strEncode = env->NewStringUTF("UTF-8");
    jclass cls = env->FindClass("java/lang/String");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "([BLjava/lang/String;)V");
    jstring object = (jstring) env->NewObject(cls, ctor, array, strEncode);

	env->DeleteLocalRef(strEncode);
	env->DeleteLocalRef(array);
	env->DeleteLocalRef(cls);
    return object;
}

typedef struct EventHandler_UserData_ {
	unsigned char*	pImage1;
	//unsigned char*	pImage2;
} EventHandler_UserData;

CNexVideoEditoerEventHandler::CNexVideoEditoerEventHandler()
{
	//m_vm			= NULL;
	m_eventObj		= NULL;
	m_callbackCapture				= NULL;
	m_callbackThumb				= NULL;
	m_callbackHighLightIndex		= NULL;

	m_callbackGetThemeImage		= NULL;
	m_callbackGetImageUsingFileID	= NULL;
	m_callbackGetImageUsingTextID	= NULL;
	m_callbackReleaseImageID		= NULL;

	m_callbackGetThemeFile			= NULL;

	m_callbackGetAudioTrackID		= NULL;
	m_callbackReleaseAudioTrackID	= NULL;
	

	m_objAudioManager 			= NULL;
	m_callbackGetAudioManagerID 	= NULL;
	
	m_callbackPrepareCustomLayerID	= NULL;
	
	m_notifyEventID	= NULL;
	m_notifyErrorID	= NULL;
	m_getLUTWithID = NULL;
	m_getVignetteTexID = NULL;
	m_getAssetResourceKey = NULL;
}

CNexVideoEditoerEventHandler::~CNexVideoEditoerEventHandler()
{
	LOGI("[nexEditorEventHandler.cpp  %d] ~CNexVideoEditoerEventHandler In", __LINE__);
	if ( m_eventObj || m_objAudioManager ) {
		NexJNIEnvStack env;
		if( m_eventObj  )
			env->DeleteGlobalRef(m_eventObj);	
		if( m_objAudioManager)
			env->DeleteGlobalRef(m_objAudioManager);
	}

	m_eventObj					= NULL;
	m_callbackCapture				= NULL;
	m_callbackGetThemeImage		= NULL;
	m_callbackGetImageUsingFileID	= NULL;
	m_callbackGetImageUsingTextID	= NULL;
	m_callbackReleaseImageID		= NULL;

	m_callbackGetThemeFile			= NULL;

	m_callbackGetAudioTrackID		= NULL;
	m_callbackReleaseAudioTrackID	= NULL;
	
	m_objAudioManager			= NULL;
	m_callbackGetAudioManagerID	= NULL;
	
	m_callbackPrepareCustomLayerID	= NULL;
	
	m_notifyEventID				= NULL;
	m_notifyErrorID				= NULL;
	m_getLUTWithID = NULL;
    m_getVignetteTexID = NULL;
    m_getAssetResourceKey = NULL;
	LOGI("[nexEditorEventHandler.cpp  %d] ~CNexVideoEditoerEventHandler Out", __LINE__);
}

int CNexVideoEditoerEventHandler::notifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3, unsigned int uiParam4)
{
	if( m_eventObj == NULL || m_notifyEventID == NULL )
		return 1;

	int		iRet	= 1;
	NexJNIEnvStack env;

	iRet = env->CallIntMethod(m_eventObj, m_notifyEventID, uiEventType, uiParam1, uiParam2, uiParam3, uiParam4);

	return iRet;
}

int CNexVideoEditoerEventHandler::notifyError(unsigned int uiEventType, unsigned int uiResult, unsigned int uiParam1, unsigned int uiParam2)
{
	if( m_eventObj == NULL || m_notifyErrorID == NULL )
		return 1;

	int		iRet	= 1;
	NexJNIEnvStack env;

	LOGI("[nexEditorEventHandler.cpp  %d] notifyError Type(%d), Param(%d %d %d)", __LINE__, uiEventType, uiResult, uiParam1, uiParam2);
	iRet = env->CallIntMethod(m_eventObj, m_notifyErrorID, uiEventType, uiResult, uiParam1, uiParam2);

	return iRet;
}

int CNexVideoEditoerEventHandler::callbackCapture(int iWidth, int iHeight, int iSize, char* pBuffer)
{
	if( m_eventObj == NULL || m_notifyEventID == NULL )
		return NULL;

	int			iRet	= 1;
	NexJNIEnvStack env;

	LOGI("[nexEditorEventHandler.cpp  %d] callbackCapture(%d %d %d 0x%p)", __LINE__, iWidth, iHeight, iSize, pBuffer);

	if( iWidth <= 0 || iHeight <= 0 || iSize == 0 || pBuffer == NULL )
	{
		env->CallIntMethod(m_eventObj, m_callbackCapture, 0, 0, 0, 0);
	}
	else
	{
		jbyteArray arr = env->NewByteArray(iSize);
		env->SetByteArrayRegion(arr, 0, iSize, (const jbyte*)pBuffer);
		iRet = env->CallIntMethod(m_eventObj, m_callbackCapture, iWidth, iHeight, iSize, arr);
		free(pBuffer);
		env->DeleteLocalRef(arr);
	}

	return iRet;
}

int CNexVideoEditoerEventHandler::callbackThumb(int iMode, int iTag, int iTime, int iWidth, int iHeight, int iCount, int iTotal, int iSize, char* pBuffer)
{
	if( m_eventObj == NULL || m_notifyEventID == NULL )
		return NULL;

	int			iRet	= 1;
	NexJNIEnvStack env;

	if( iSize == 0 || pBuffer == NULL )
	{
		env->CallIntMethod(m_eventObj, m_callbackThumb, 0, 0, 0, 0, 0, 0);
	}
	else
	{
		jbyteArray arr = env->NewByteArray(iSize);
		env->SetByteArrayRegion(arr, 0, iSize, (const jbyte*)pBuffer);
		iRet = env->CallIntMethod(m_eventObj, m_callbackThumb, iMode, iTag, iTime, iWidth, iHeight, iCount, iTotal, iSize, arr);
		free(pBuffer);
		env->DeleteLocalRef(arr);
	}

	return iRet;
}

int CNexVideoEditoerEventHandler::callbackHighLightIndex(int iCount, int* pBuffer)
{
	if( m_eventObj == NULL || m_notifyEventID == NULL )
		return NULL;

	int			iRet	= 1;
	NexJNIEnvStack env;

	if( iCount == 0 || pBuffer == NULL )
	{
		env->CallIntMethod(m_eventObj, m_callbackHighLightIndex, 0, 0);
	}
	else
	{
		jintArray arr = env->NewIntArray(iCount);
		env->SetIntArrayRegion(arr, 0, iCount, (const jint*)pBuffer);
		iRet = env->CallIntMethod(m_eventObj, m_callbackHighLightIndex, iCount, arr);
		free(pBuffer);
		env->DeleteLocalRef(arr);
	}

	return iRet;
}

int CNexVideoEditoerEventHandler::callbackCheckImageWorkDone()
{
	if( m_eventObj == NULL || m_notifyEventID == NULL )
		return NULL;

	int			iRet	= 1;
	NexJNIEnvStack env;

	iRet = env->CallIntMethod(m_eventObj, m_callbackCheckImageWorkDone);
	return iRet;
}

void CNexVideoEditoerEventHandler::callbackGetThemeImage(char* pImageFilePath, int asyncmode, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData)
{
	if( pImageFilePath == NULL || uiFileLen <= 0 || pWidth == NULL || pHeight == NULL || pBitForPixel == NULL || ppImageData == NULL || pImageDataSize == NULL )
		return;

	*pWidth = 0;
	*pHeight = 0;
	*pBitForPixel = 0;
	*ppImageData = NULL;
	*pImageDataSize = 0;

	// LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeImage", __LINE__);
	//bool attached = false;

	NexJNIEnvStack env;

	//jstring strPath = NewStringUTF(env.get(), pImageFilePath);
	jstring strPath = env->NewStringUTF((const char*)pImageFilePath);

	if( strPath == NULL ) return;
	
	env.addAutoDeleteLocalRef(strPath);

	jobject obj = (jobject)env->CallObjectMethod(m_eventObj, m_callbackGetThemeImage, strPath, asyncmode);

	if( obj == NULL ) {
		LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeImage failed(obj is null)", __LINE__);
		return;
	}

	env.addAutoDeleteLocalRef(obj);

	if( env->ExceptionCheck() )	{
		LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeImage occured java exception", __LINE__);
		return;
	}

	jclass bitmapclass = env->GetObjectClass(obj);
	if( bitmapclass == NULL ) {
		LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeImage failed(get class is failed)", __LINE__);
		return;
	}

	env.addAutoDeleteLocalRef(bitmapclass);

	jmethodID	bitmapMethodID_getWidth;
	bitmapMethodID_getWidth = env->GetMethodID(bitmapclass, "getWidth", "()I");
	if( bitmapMethodID_getWidth == NULL )
		return;

	*pWidth = (int)env->CallIntMethod(obj, bitmapMethodID_getWidth);

	jmethodID bitmapMethodID_getHeight = env->GetMethodID(bitmapclass, "getHeight", "()I");
	if( bitmapMethodID_getHeight == NULL ) {
		*pWidth = 0;
		return;
	}

	*pHeight = (int)env->CallIntMethod(obj, bitmapMethodID_getHeight);

	*pBitForPixel		= 32;
	*pImageDataSize 	= *pWidth * *pHeight * 4;

	EventHandler_UserData* pEHUserData		= new EventHandler_UserData;
	if( pEHUserData == NULL )
	{
		*pWidth = 0;
		*pHeight = 0;
		*pImageDataSize = 0;
		return;
	}

	memset(pEHUserData, 0x00, sizeof(EventHandler_UserData));
	pEHUserData->pImage1 = new unsigned char[*pImageDataSize];
	if (pEHUserData->pImage1 == NULL)
	{
		*pWidth = 0;
		*pHeight = 0;
		*pImageDataSize = 0;
		delete pEHUserData;
		pEHUserData = NULL;
		return;
	}
	*ppImageData		= pEHUserData->pImage1;
		
	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeImage Width(%d) Height(%d)", __LINE__, *pWidth, *pHeight);	
		
	jintArray array = env->NewIntArray(*pWidth * *pHeight);
	if( array == NULL )
	{
		*pWidth = 0;
		*pHeight = 0;
		*pImageDataSize = 0;
		*ppImageData = NULL;
		delete [] pEHUserData->pImage1;
		pEHUserData->pImage1 = NULL;
		delete pEHUserData;
		pEHUserData = NULL;

		return;
	}

	// LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeImage Width(%d) Height(%d)", __LINE__, *pWidth, *pHeight);

	jmethodID bitmapMethodID_getPixels = env->GetMethodID(bitmapclass, "getPixels", "([I)V");
	env->CallVoidMethod(obj, bitmapMethodID_getPixels, array);

	//LOGI("(ERIC) GTI %p,%p,%p", bitmapMethodID_getWidth, bitmapMethodID_getHeight, bitmapMethodID_getPixels );

	jboolean isCopy = 123;
	jint *pixels = env->GetIntArrayElements(array, &isCopy);
	if( pixels == NULL )
	{
		*pWidth = 0;
		*pHeight = 0;
		*ppImageData = NULL;
		*pImageDataSize = 0;

		env->DeleteLocalRef(array);
		array = NULL;

		delete [] pEHUserData->pImage1;
		pEHUserData->pImage1 = NULL;
		delete pEHUserData;
		pEHUserData = NULL; 

		LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeImage(GetIntArrayElements is failed 0x%p)", __LINE__, pixels);
		return;
	}

	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeImage copy ImageData (isCopy=%d)", __LINE__, (int)isCopy);
	memcpy(*ppImageData, pixels, *pImageDataSize);
	env->ReleaseIntArrayElements(array, pixels, 0);
	env->DeleteLocalRef(array);
	array = NULL;

//	jmethodID bitmapMethodID_recycle = env->GetMethodID(bitmapclass, "recycle", "()V");
//	env->CallVoidMethod(obj, bitmapMethodID_recycle);

	*ppUserData = (void*)pEHUserData;

	// LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeImage Successed", __LINE__);
}

void CNexVideoEditoerEventHandler::callbackGetImageUsingFile(char* pImageFilePath, int iThumb, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData, int* pLoadedType)
{
	if( pImageFilePath == NULL || uiFileLen <= 0 || pWidth == NULL || pHeight == NULL || pBitForPixel == NULL || ppImageData == NULL || pImageDataSize == NULL || pLoadedType == NULL)
		return;

	*pWidth = 0;
	*pHeight = 0;
	*pBitForPixel = 0;
	*ppImageData = NULL;
	*pImageDataSize = 0;
	*pLoadedType = 1;

	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile", __LINE__);
	NexJNIEnvStack env;

	//jstring strPath = NewStringUTF(env.get(), pImageFilePath);
	jstring strPath = env->NewStringUTF(pImageFilePath);
	if( strPath == NULL )
		return;

	env.addAutoDeleteLocalRef(strPath);
	jint isThumb = iThumb;

	jobject obj = (jobject)env->CallObjectMethod(m_eventObj, m_callbackGetImageUsingFileID, strPath, isThumb);


	if( obj == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile failed(obj is null)", __LINE__);
		return;
	}

	env.addAutoDeleteLocalRef(obj);

	jclass bitmapclass = env->GetObjectClass(obj);
	if( bitmapclass == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile failed(get class is failed)", __LINE__);
		return;
	}

	env.addAutoDeleteLocalRef(bitmapclass);

	jmethodID	bitmapMethodID_getWidth;
	bitmapMethodID_getWidth = env->GetMethodID(bitmapclass, "getWidth", "()I");
	if( bitmapMethodID_getWidth == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile failed(get methmod is failed)", __LINE__);
		return;
	}

	*pWidth = (int)env->CallIntMethod(obj, bitmapMethodID_getWidth);

	jmethodID bitmapMethodID_getHeight = env->GetMethodID(bitmapclass, "getHeight", "()I");
	if( bitmapMethodID_getHeight == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile failed(get methmod is failed)", __LINE__);
		*pWidth = 0;
		return;
	}

	*pHeight = (int)env->CallIntMethod(obj, bitmapMethodID_getHeight);

	jmethodID bitmapMethodID_getLoadedType = env->GetMethodID(bitmapclass, "getLoadedType", "()I");
	if( bitmapMethodID_getLoadedType == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile failed(get methmod is failed)", __LINE__);
		*pWidth = 0;
		return;
	}
	*pLoadedType = (int)env->CallIntMethod(obj, bitmapMethodID_getLoadedType);

	LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile Width(%d) Height(%d) LoadedType(%d)", __LINE__, *pWidth, *pHeight, *pLoadedType);

	*pBitForPixel		= 32;
	*pImageDataSize 	= *pWidth * *pHeight * 4;

	EventHandler_UserData* pEHUserData = new EventHandler_UserData;
	if( pEHUserData == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile failed(do not allocate rgb buffer )", __LINE__);
		*pWidth = 0;
		*pHeight = 0;
		*pImageDataSize = 0;
		return;
	}

	memset(pEHUserData, 0x00, sizeof(EventHandler_UserData));
	pEHUserData->pImage1 = new unsigned char[*pImageDataSize];
	if (pEHUserData->pImage1 == NULL)
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile failed(do not allocate rgb buffer )", __LINE__);
		*pWidth = 0;
		*pHeight = 0;
		*pImageDataSize = 0;
		delete pEHUserData;
		pEHUserData = NULL;
		return;
	}
	*ppImageData = pEHUserData->pImage1;

    int maxRows = 200;
    if( *pHeight < maxRows )
        maxRows = *pHeight;
	
	jintArray array = env->NewIntArray(*pWidth * maxRows);
	if( array == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile failed(do not allocate int array)", __LINE__);
		*pWidth = 0;
		*pHeight = 0;
		*ppImageData = NULL;
		*pImageDataSize = 0;
		delete [] pEHUserData->pImage1;
		pEHUserData->pImage1 = NULL;
		delete pEHUserData;
		pEHUserData = NULL;
		return;
	}

    jmethodID bitmapMethodID_getPixels = env->GetMethodID(bitmapclass, "getPixels", "([IIIIIII)V");

	//LOGI("(ERIC) GIUF %p,%p,%p", bitmapMethodID_getWidth, bitmapMethodID_getHeight, bitmapMethodID_getPixels );

    for( int y=0; y<*pHeight; y+=maxRows ) {
        // getPixels(int[] pixels, int offset, int stride, int x, int y, int width, int height)
        int height = (*pHeight) - y;
        if( height > maxRows )
            height = maxRows;
            
        env->CallVoidMethod(obj, bitmapMethodID_getPixels, array, 0, *pWidth, 0, y, *pWidth, height);
        jboolean isCopy = 123;
        jint *pixels = env->GetIntArrayElements(array, &isCopy);
        LOGI("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile copy ImageData (maxRows=%d, isCopy=%d, y=%d, *pWidth=%d, *pHeight=%d, height=%d, pixels=0x%16p)", __LINE__, maxRows, (int)isCopy, y, *pWidth, *pHeight, height, pixels);
        if( pixels ) {
            memcpy((*ppImageData)+(y * *pWidth * 4), pixels, *pWidth * height * 4);
        }
        env->ReleaseIntArrayElements(array, pixels, 0);
    }
    
	env->DeleteLocalRef(array);
	array = NULL;

//	jmethodID bitmapMethodID_recycle = env->GetMethodID(bitmapclass, "recycle", "()V");
//	env->CallVoidMethod(obj, bitmapMethodID_recycle);

	*ppUserData = (void*)pEHUserData;

	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingFile Successed", __LINE__);
}

void CNexVideoEditoerEventHandler::callbackGetImageUsingText(char* pText, unsigned int uiTextLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppTextImageData, int* pTextImageDataSize, void** ppUserData)
{
	if( pText == NULL || uiTextLen <= 0 || pWidth == NULL || pHeight == NULL || pBitForPixel == NULL || ppTextImageData == NULL || pTextImageDataSize == NULL )
		return;

	*pWidth = 0;
	*pHeight = 0;
	*pBitForPixel = 0;
	*ppTextImageData = NULL;
	*pTextImageDataSize = 0;

	NexJNIEnvStack env;

	jstring strPath = NewStringUTF(env.get(), pText);
	if( strPath == NULL )
	{
		return;
	}
	
	jobject obj = (jobject)env->CallObjectMethod(m_eventObj, m_callbackGetImageUsingTextID, strPath);

	env->DeleteLocalRef(strPath); strPath = NULL;

	if( obj == NULL )
	{
		return;
	}

	env.addAutoDeleteLocalRef(obj);

	jclass bitmapclass = env->GetObjectClass(obj);
	if( bitmapclass == NULL )
	{
		return;
	}

	env.addAutoDeleteLocalRef(bitmapclass);

	jmethodID	bitmapMethodID_getWidth;
	bitmapMethodID_getWidth = env->GetMethodID(bitmapclass, "getWidth", "()I");
	if( bitmapMethodID_getWidth == NULL )
	{
		return;
	}

	*pWidth = (int)env->CallIntMethod(obj, bitmapMethodID_getWidth);

	jmethodID bitmapMethodID_getHeight = env->GetMethodID(bitmapclass, "getHeight", "()I");
	if( bitmapMethodID_getHeight == NULL )
	{
		*pWidth = 0;
		return;
	}

	*pHeight = (int)env->CallIntMethod(obj, bitmapMethodID_getHeight);	

	*pBitForPixel			= 32;
	*pTextImageDataSize 	= *pWidth * *pHeight * 4;

	EventHandler_UserData* pEHUserData = new EventHandler_UserData;
	if( pEHUserData == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingText failed(do not allocate rgb buffer )", __LINE__);
		*pWidth = 0;
		*pHeight = 0;
		*pTextImageDataSize = 0;
		return;
	}

	memset(pEHUserData, 0x00, sizeof(EventHandler_UserData));
	pEHUserData->pImage1 = new unsigned char[*pTextImageDataSize];
	if (pEHUserData->pImage1 == NULL)
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageUsingText failed(do not allocate rgb buffer )", __LINE__);
		*pWidth = 0;
		*pHeight = 0;
		*pTextImageDataSize = 0;
		delete pEHUserData;
		pEHUserData = NULL;
		return;
	}
	*ppTextImageData = pEHUserData->pImage1;

	jintArray array = env->NewIntArray(*pWidth * *pHeight);
	if( array == NULL )
	{
		*pWidth = 0;
		*pHeight = 0;
		*ppTextImageData = NULL;
		*pTextImageDataSize = 0;
		delete [] pEHUserData->pImage1;
		pEHUserData->pImage1 = NULL;
		delete pEHUserData;
		pEHUserData = NULL;
		return;
	}
		
	
	jmethodID bitmapMethodID_getPixels = env->GetMethodID(bitmapclass, "getPixels", "([I)V");
	//LOGI("(ERIC) GIUT %p,%p,%p", bitmapMethodID_getWidth, bitmapMethodID_getHeight, bitmapMethodID_getPixels );
	env->CallVoidMethod(obj, bitmapMethodID_getPixels, array);

	jint *pixels = env->GetIntArrayElements(array, 0);
	if( pixels )
	{
		*pWidth = 0;
		*pHeight = 0;
		*pTextImageDataSize = 0;
		*ppTextImageData = NULL;
		delete [] pEHUserData->pImage1;
		pEHUserData->pImage1 = NULL;
		delete pEHUserData;
		pEHUserData = NULL;
		env->DeleteLocalRef(array);
		array = NULL;
		return;
	}

	memcpy(*ppTextImageData, pixels, *pTextImageDataSize);
	env->ReleaseIntArrayElements(array, pixels, 0);

	env->DeleteLocalRef(array);
	array = NULL;

//	jmethodID bitmapMethodID_recycle = env->GetMethodID(bitmapclass, "recycle", "()V");
//	env->CallVoidMethod(obj, bitmapMethodID_recycle);

	*ppUserData = (void*)pEHUserData;
}

void CNexVideoEditoerEventHandler::callbackReleaseImage(void** ppUserData)
{
//	bool attached = false;
	NexJNIEnvStack env;

	EventHandler_UserData* pEHUserData = (EventHandler_UserData*) *ppUserData;
	if( pEHUserData )
	{
		if( pEHUserData->pImage1 )
		{
			delete [] pEHUserData->pImage1;
			pEHUserData->pImage1 = NULL;
		}
		LOGI("[nexEditorEventHandler.cpp  %d] callbackReleaseImage  pEHUserData(0x%p)", __LINE__, pEHUserData);
		delete pEHUserData;
		pEHUserData = NULL;
	}

	env->CallVoidMethod(m_eventObj, m_callbackReleaseImageID);
}

void CNexVideoEditoerEventHandler::callbackGetImageHeaderUsingFile(char* pImageFilePath, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel)
{
	if( pImageFilePath == NULL || uiFileLen <= 0 || pWidth == NULL || pHeight == NULL || pBitForPixel == NULL )
		return;

	*pWidth = 0;
	*pHeight = 0;
	*pBitForPixel = 0;

	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetImageHeaderUsingFile", __LINE__);

	NexJNIEnvStack env;

	jstring strPath = NewStringUTF(env.get(), pImageFilePath);
	if( strPath == NULL )
	{
		return;
	}
	
	jobject obj = (jobject)env->CallObjectMethod(m_eventObj, m_callbackGetImageUsingFileID, strPath, 0);

	env->DeleteLocalRef(strPath); strPath = NULL;

	if( obj == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageHeaderUsingFile failed(obj is null)", __LINE__);
		return;
	}

	env.addAutoDeleteLocalRef(obj);

	jclass bitmapclass = env->GetObjectClass(obj);
	if( bitmapclass == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageHeaderUsingFile failed(get class is failed)", __LINE__);
		return;
	}

	env.addAutoDeleteLocalRef(bitmapclass);

	jmethodID	bitmapMethodID_getWidth, bitmapMethodID_getHeight, bitmapMethodID_getPixels;
	bitmapMethodID_getWidth = env->GetMethodID(bitmapclass, "getWidth", "()I");
	if( bitmapMethodID_getWidth == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageHeaderUsingFile failed(get methmod is failed)", __LINE__);
		return;
	}

	*pWidth = (int)env->CallIntMethod(obj, bitmapMethodID_getWidth);

	bitmapMethodID_getHeight = env->GetMethodID(bitmapclass, "getHeight", "()I");
	if( bitmapMethodID_getHeight == NULL )
	{
		LOGE("[nexEditorEventHandler.cpp  %d] callbackGetImageHeaderUsingFile failed(get methmod is failed)", __LINE__);
		*pWidth = 0;
		return;
	}

	//bitmapMethodID_getPixels = env->GetMethodID(bitmapclass, "getHeight", "()I");
	//LOGI("(ERIC) GIHUF %p,%p,%p", bitmapMethodID_getWidth, bitmapMethodID_getHeight, bitmapMethodID_getPixels );

	*pHeight = (int)env->CallIntMethod(obj, bitmapMethodID_getHeight);
	*pBitForPixel		= 32;

//	jmethodID bitmapMethodID_recycle = env->GetMethodID(bitmapclass, "recycle", "()V");
//	env->CallVoidMethod(obj, bitmapMethodID_recycle);

	env->CallVoidMethod(m_eventObj, m_callbackReleaseImageID);

	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetImageHeaderUsingFile Successed(%d %d %d)", __LINE__, *pWidth, *pHeight, *pBitForPixel);
}

void* CNexVideoEditoerEventHandler::callbackGetAudioTrack(int iSampleRate, int iChannels)
{
	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetAudioTrack In(%d %d)", __LINE__, iSampleRate, iChannels);
	NexJNIEnvStack env;

	jobject obj = (jobject)env->CallObjectMethod(m_eventObj, m_callbackGetAudioTrackID, iSampleRate, iChannels);
	if( obj == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] callbackGetAudioTrack failed", __LINE__);
		return NULL;
	}

	env.addAutoDeleteLocalRef(obj);

	jclass classAudioTrack = env->GetObjectClass(obj);	
	if( classAudioTrack == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] callbackGetAudioTrack failed", __LINE__);
		return NULL;
	}

	env.addAutoDeleteLocalRef(classAudioTrack);

	jfieldID audiotrack_native = env->GetFieldID(classAudioTrack, "mNativeTrackInJavaObj", "I");
	if( audiotrack_native == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] callbackGetAudioTrack failed", __LINE__);
		return NULL;
	}

	void* pAudioTrack =  (void*)env->GetIntField(obj, audiotrack_native);
	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetAudioTrack Out(0x%p)", __LINE__, pAudioTrack);

	return pAudioTrack;
}

void CNexVideoEditoerEventHandler::callbackReleaseAudioTrack()
{
	LOGI("[nexEditorEventHandler.cpp  %d] callbackReleaseAudioTrack In", __LINE__);

	NexJNIEnvStack env;
	env->CallVoidMethod(m_eventObj, m_callbackReleaseAudioTrackID);

	LOGI("[nexEditorEventHandler.cpp  %d] callbackReleaseAudioTrack Out", __LINE__);
}


void* CNexVideoEditoerEventHandler::callbackGetAudioManager()
{
 	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetAudioManager", __LINE__);

	NexJNIEnvStack env;

	if( m_objAudioManager != NULL)
	{
		LOGI("[nexEditorEventHandler.cpp  %d] callbackGetAudioManager delete audioManager", __LINE__);
		env->DeleteGlobalRef(m_objAudioManager);
		m_objAudioManager = NULL;
	}

	jobject audioManager = (jobject)env->CallObjectMethod(m_eventObj, m_callbackGetAudioManagerID);
	if( audioManager == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] callbackGetAudioManager failed", __LINE__);
		return NULL;
	}

	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetAudioManager Out(0x%p)", __LINE__, audioManager);

	m_objAudioManager = env->NewGlobalRef(audioManager);
	env->DeleteLocalRef(audioManager);

	return (void*)m_objAudioManager;
	
}


void CNexVideoEditoerEventHandler::callbackReleaseAudioManager()
{
 	LOGI("[nexEditorEventHandler.cpp  %d] callbackReleaseAudioManager", __LINE__);

	NexJNIEnvStack env;

	if( m_objAudioManager != NULL)
	{
		LOGI("[nexEditorEventHandler.cpp  %d] callbackReleaseAudioManager delete audioManager", __LINE__);
		env->DeleteGlobalRef(m_objAudioManager);
		m_objAudioManager = NULL;
	}
	else
	{
		LOGI("[nexEditorEventHandler.cpp  %d] callbackReleaseAudioManager audioManager already is NULL", __LINE__);
	}

	return;
}

int CNexVideoEditoerEventHandler::callbackCustomLayer(	int iParam1, int iParam2, int iParam3, int iParam4, int iParam5, 
									int iParam6, int iParam7, int iParam8, int iParam9, int iParam10, 
									int iParam11, int iParam12, int iParam13, int iParam14, int iParam15, int iParam16, int iParam17, int iParam18)
{
	NexJNIEnvStack env;

	int iRet = env->CallIntMethod(m_eventObj, m_callbackPrepareCustomLayerID, iParam1, iParam2, iParam3, iParam4, iParam5, 
									iParam6, iParam7, iParam8, iParam9, iParam10, iParam11, iParam12, iParam13, iParam14, iParam15, iParam16, iParam17, iParam18);

	//LOGI("[nexEditorEventHandler.cpp  %d] callbackCustomLayer", __LINE__);
	return iRet;
}

int CNexVideoEditoerEventHandler::getLUTWithID(int lut_resource_id, int export_flag){

	LOGI("[nexEditorEventHandler.cpp  %d] getLUTWithHash", __LINE__);

	NexJNIEnvStack env;

	int iRet = env->CallIntMethod(m_eventObj, m_getLUTWithID, lut_resource_id, export_flag);

	return iRet;	
}

int CNexVideoEditoerEventHandler::getVignetteTexID(int export_flag){

	LOGI("[nexEditorEventHandler.cpp  %d] getVignetteTexID", __LINE__);

	NexJNIEnvStack env;

	int iRet = env->CallIntMethod(m_eventObj, m_getVignetteTexID, export_flag);

	return iRet;	
}

int CNexVideoEditoerEventHandler::getAssetResourceKey(const char * input_resourcpath, char *output_key){

	LOGI("[nexEditorEventHandler.cpp  %d] getAssetResourceKey", __LINE__);

	NexJNIEnvStack env;
	jstring strPath = env->NewStringUTF(input_resourcpath);

	jstring resultJNIStr = (jstring)env->CallObjectMethod(m_eventObj, m_getAssetResourceKey, strPath);
    
    env->DeleteLocalRef(strPath); strPath = NULL;
    
    
    const char *resultCStr = env->GetStringUTFChars( resultJNIStr, NULL);
    
    if( resultCStr == NULL ){
        if( output_key ){
            strcpy( output_key, input_resourcpath );
        }
        env->ReleaseStringUTFChars(resultJNIStr, resultCStr);
        env->DeleteLocalRef(resultJNIStr);
        
        return -1;
    }
    
    env->DeleteLocalRef(strPath); strPath = NULL;
    
    LOGI("[nexEditorEventHandler.cpp  %d] getAssetResourceKey new key=%s",__LINE__, resultCStr);
    if( output_key ){
        strcpy( output_key, resultCStr );
    }
    env->ReleaseStringUTFChars(resultJNIStr, resultCStr);
    env->DeleteLocalRef(resultJNIStr);
	return 0;	
}



int CNexVideoEditoerEventHandler::setEventObject(jobject eventObj)
{
	if( eventObj == NULL ) {
		return 1;
	}

	NexJNIEnvStack env;

	if ( env.get() == NULL ) 
		return 1;

	m_eventObj	= env->NewGlobalRef(eventObj);
	if( m_eventObj == NULL ) {
		LOGE("[nexEditorEventHandler.cpp  %d] setEventObject NewGlobalRef failed from eventObj", __LINE__);
		return 1;
	}

	jclass eventClass = env->GetObjectClass(m_eventObj);
	if( eventClass == NULL )
	{
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
		
	}

	env.addAutoDeleteLocalRef(eventClass);

	m_callbackCapture = env->GetMethodID(eventClass, "callbackCapture", "(III[B)I");
	if( m_callbackCapture == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackCapture failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}

	m_callbackThumb = env->GetMethodID(eventClass, "callbackThumb", "(IIIIIIII[B)I");
	if( m_callbackThumb == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackThumb failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}	

	m_callbackHighLightIndex = env->GetMethodID(eventClass, "callbackHighLightIndex", "(I[I)I");
	if( m_callbackHighLightIndex == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackHighLightIndex failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}

	m_callbackCheckImageWorkDone = env->GetMethodID(eventClass, "callbackCheckImageWorkDone", "()I");
	if( m_callbackCheckImageWorkDone == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackCheckImageWorkDone failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}	

#ifdef FOR_PROJECT_Kinemaster
	m_callbackGetThemeImage = env->GetMethodID(eventClass, "callbackGetThemeImage", "(Ljava/lang/String;I)Lcom/nextreaming/nexvideoeditor/NexImage;");
#else
	m_callbackGetThemeImage = env->GetMethodID(eventClass, "callbackGetThemeImage", "(Ljava/lang/String;I)Lcom/nexstreaming/kminternal/nexvideoeditor/NexImage;");
#endif
	
	if( m_callbackGetThemeImage == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackOpenGLRenderID failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}	

	m_callbackGetThemeFile = env->GetMethodID(eventClass, "callbackGetThemeFile", "(Ljava/lang/String;)[B");
	
	if( m_callbackGetThemeFile == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackGetThemeFile failed", __LINE__);

		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}	

#ifdef FOR_PROJECT_Kinemaster
	m_callbackGetImageUsingFileID = env->GetMethodID(eventClass, "callbackGetImageUsingFile", "(Ljava/lang/String;I)Lcom/nextreaming/nexvideoeditor/NexImage;");
#else
	m_callbackGetImageUsingFileID = env->GetMethodID(eventClass, "callbackGetImageUsingFile", "(Ljava/lang/String;I)Lcom/nexstreaming/kminternal/nexvideoeditor/NexImage;");
#endif
	
	if( m_callbackGetImageUsingFileID == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackOpenGLRenderID failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}

#ifdef FOR_PROJECT_Kinemaster
	m_callbackGetImageUsingTextID = env->GetMethodID(eventClass, "callbackGetImageUsingText", "(Ljava/lang/String;)Lcom/nextreaming/nexvideoeditor/NexImage;");
#else
	m_callbackGetImageUsingTextID = env->GetMethodID(eventClass, "callbackGetImageUsingText", "(Ljava/lang/String;)Lcom/nexstreaming/kminternal/nexvideoeditor/NexImage;");
#endif
	
	if( m_callbackGetImageUsingTextID == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackOpenGLRenderID failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}

	m_callbackReleaseImageID = env->GetMethodID(eventClass, "callbackReleaseImage", "()V");
	if( m_callbackReleaseImageID == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackOpenGLRenderID failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}

	m_callbackGetAudioTrackID = env->GetMethodID(eventClass, "callbackGetAudioTrack", "(II)Landroid/media/AudioTrack;");
	if( m_callbackGetAudioTrackID == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackGetAudioTrackID failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}


	m_callbackReleaseAudioTrackID = env->GetMethodID(eventClass, "callbackReleaseAudioTrack", "()V");
	if( m_callbackReleaseAudioTrackID == NULL )
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackReleaseAudioTrackID failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}
	
	m_callbackGetAudioManagerID = env->GetMethodID(eventClass, "callbackGetAudioManager","()Landroid/media/AudioManager;");
	if( m_callbackGetAudioManagerID == NULL)
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackGetAudioManagerID failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}
	
	m_callbackPrepareCustomLayerID = env->GetMethodID(eventClass, "callbackPrepareCustomLayer","(IIIIIIIIIIIIIIIIII)I");
	if( m_callbackPrepareCustomLayerID == NULL)
	{
		LOGI("[nexEditorEventHandler.cpp  %d] GetMethodID m_callbackPrepareCustomLayerID failed", __LINE__);
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}
	
	m_notifyEventID = env->GetMethodID(eventClass, "notifyEvent", "(IIIII)I");
	if( m_notifyEventID == NULL )
	{
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj	= NULL;
		return 1;
	}
	
	m_notifyErrorID = env->GetMethodID(eventClass, "notifyError", "(IIII)I");
	if( m_notifyErrorID == NULL )
	{
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj		= NULL;
		m_notifyEventID	= NULL;
		m_notifyErrorID	= NULL;
		return 1;
	}

	m_getLUTWithID = env->GetMethodID(eventClass, "getLutTextWithID", "(II)I");
	if( m_getLUTWithID == NULL )
	{
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj		= NULL;
		m_notifyEventID	= NULL;
		m_notifyErrorID	= NULL;
		return 1;
	}
	
	m_getVignetteTexID = env->GetMethodID(eventClass, "getVignetteTexID", "(I)I");
	if( m_getVignetteTexID == NULL )
	{
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj		= NULL;
		m_notifyEventID	= NULL;
		m_notifyErrorID	= NULL;
		return 1;
	}
	
	
	m_getAssetResourceKey = env->GetMethodID(eventClass, "getAssetResourceKey", "(Ljava/lang/String;)Ljava/lang/String;");
	if( m_getAssetResourceKey == NULL )
	{
		env->DeleteGlobalRef(m_eventObj);	
		m_eventObj		= NULL;
		m_notifyEventID	= NULL;
		m_notifyErrorID	= NULL;
		return 1;
	}
	
	return 0;
}

void CNexVideoEditoerEventHandler::callbackGetThemeFile(char* pFilePath, unsigned int uiFileLen, int* pLength, char** ppFileData)
{
	if( pFilePath == NULL || uiFileLen <= 0 || pLength == NULL || ppFileData == NULL)
		return;

	*pLength = 0;
	*ppFileData = NULL;

	LOGI("[nexEditorEventHandler.cpp  %d] callbackGetThemeFile", __LINE__);
	//bool attached = false;

	NexJNIEnvStack env;

	jstring strPath = env->NewStringUTF(pFilePath);
	if( strPath == NULL )
	{
		return;
	}

	jbyteArray arr = (jbyteArray)env->CallObjectMethod(m_eventObj, m_callbackGetThemeFile, strPath);

	env->DeleteLocalRef(strPath);

	if(arr){

		jboolean isCopy = 0;
		jbyte* contents = env->GetByteArrayElements(arr, &isCopy);
		*pLength = env->GetArrayLength(arr);
		*ppFileData	= new char[*pLength + 1];
		memcpy(*ppFileData, contents, *pLength);
		(*ppFileData)[*pLength] = 0;
		env->ReleaseByteArrayElements(arr, contents, 0);
	}

	env->DeleteLocalRef(arr);
}
