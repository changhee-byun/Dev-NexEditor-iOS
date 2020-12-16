#include <jni.h>
#include <android/log.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "NxAnimatedGIFAPI.h"
#include "NxYUVtoRGBAPI.h"

#define  LOG_TAG    "nexAniGif"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define ECODE_None                  0
#define ECODE_INVALID_Argument      -1
#define ECODE_MAX_Handle            -2
#define ECODE_INVALID_Handle        -3
#define ECODE_FailOfCreate          -4
#define ECODE_FailOfInit            -5
#define ECODE_FailOfEncode          -6
#define ECODE_INVALID_State         -7

#define FrameFlag_EoS               1

extern "C" {

// handle management apis
#define MAX_HANDLE_COUNT 16
#define OUTPUT_BLOCK_COUNT 8

static PNXANIGIFHANDLE s_DB[MAX_HANDLE_COUNT];
static int s_handle_count = 0;
static int s_dump_count;

static int _getEmptyHandle(){
    for(int i = 0 ; i < MAX_HANDLE_COUNT ; i++ ){
        if( s_DB[i] == NULL ){
            return i;
        }
    }
    return ECODE_MAX_Handle;
}

static int _createHandle(){
    int i = _getEmptyHandle();
    if( i < 0 ){
        return i;
    }

    s_handle_count++;

    PNXANIGIFHANDLE pSC =  NxAniGIFOpen();
    if( pSC == NULL ){
        LOGE("[nxAniGifjni.cpp %d] _createHandle create fail!", __LINE__);
        s_handle_count--;
        return ECODE_FailOfCreate;
    }

    s_DB[i] = pSC;
    s_DB[i]->pOutputGIF = NULL;
    s_DB[i]->pInputBitStream = NULL;
    return i;
}

static int _destoryHandle(int index){
    if( s_DB[index] != NULL ){
        if( s_DB[index]->pOutputGIF != NULL ){
            free(s_DB[index]->pOutputGIF);
            s_DB[index]->pOutputGIF = NULL;
        }

        NxAniGIFClose(s_DB[index]);
        s_DB[index] = 0;
        s_handle_count--;
        return ECODE_None;
    }
    return ECODE_INVALID_Handle;
}
//------------

// file i/o apis
static int Read(int fd, unsigned char *buff , int size )
{
    int remain = size;
    int nread;
    unsigned char *p = buff;
    while(1)
    {
        nread = read(fd,p,remain);
        if( nread <= 0 ){
            LOGE("[nexAniGifjni.cpp %d] Read nread(%d)", __LINE__, nread);
            return -1;
        }
        remain -= nread;
        if( remain == 0 )
            return size;
        p += nread;
    }
    return size;
}

static int Write(int fd, unsigned char *buff , int size )
{
    int remain = size;
    int nwrite;
    unsigned char *p = buff;
    while(1)
    {
        nwrite = write(fd,p,remain);
        if( nwrite <= 0 ){
            LOGE("[nexAniGifjni.cpp %d] Write nwrite(%d)", __LINE__, nwrite);
            return -1;
        }
        remain -= nwrite;
        if( remain == 0 )
            return size;
        p += nwrite;
    }
    return size;
}

//------------

//time lock
#include <time.h>

int time_lock(){
    time_t t = time(NULL);
    struct tm *  st = gmtime(&t);
    LOGE("[nexAniGifjni.cpp %d] time_lock(%d,%d,%d)", __LINE__, st->tm_year+1900 , st->tm_mon+1, st->tm_mday );
    int year = st->tm_year+1900;
    int mon = st->tm_mon+1;
    int day = st->tm_mday;

    if( year > 2018 )
        return  -1;

    return 0;
}
//------------

// thread apis
typedef struct _encoderParam{
    int handle;
    int rgbaFd;
    int outFd;
    int format;
}EncoderParam;

static void *thread_encode_file2file(void *arg)
{
    int handle;
    int rgbaFd;
    int outFd;
    int buffSize = 0;
    unsigned char *pbuffer = NULL;
    unsigned char *preadbuffer = NULL;
    int rval;
    int count = 1;
    int total = 0;
    int format = 0;
    int readSize = 0;
    off_t startOff, endOff;

    EncoderParam *param = (EncoderParam*)arg;
    handle = param->handle;
    rgbaFd = param->rgbaFd;
    outFd = param->outFd;
    format = param->format;
    free(param);

    startOff = lseek(rgbaFd,0,SEEK_SET);
    endOff = lseek(rgbaFd,0,SEEK_END);
    total = endOff - startOff;
    lseek(rgbaFd,0,SEEK_SET);

    buffSize = s_DB[handle]->width * s_DB[handle]->height * 4;
    readSize = buffSize;
    pbuffer = (unsigned char *)malloc(buffSize);
    if(format == 1 ){
        readSize = (s_DB[handle]->width * s_DB[handle]->height * 3) /2;
        preadbuffer = (unsigned char *)malloc(readSize);
    }

    total /= readSize;

    LOGE("[nexAniGifjni.cpp %d] thread_encode start(%d)..., NxYUVtoRGBGetVersionNum(%d)", __LINE__,total, NxYUVtoRGBGetVersionNum(2));
    while(1){

        if(format == 1 ){
            if( Read(rgbaFd, preadbuffer, readSize) < 0 ){
                break;
            }
            unsigned char * ptrY = preadbuffer;
            unsigned char * ptrU = ptrY+(s_DB[handle]->width*s_DB[handle]->height);
            unsigned char * ptrV = ptrU+(s_DB[handle]->width*s_DB[handle]->height/4);
            //NxYUV420toRGB888((unsigned int*)(pbuffer),ptrY, ptrU , ptrV ,s_DB[handle]->width,s_DB[handle]->height,s_DB[handle]->width,s_DB[handle]->width);
            NxYUV420toRGB888_BC((unsigned int*)(pbuffer),ptrY, ptrU , ptrV ,s_DB[handle]->width,s_DB[handle]->height,s_DB[handle]->width,s_DB[handle]->width,0,128);
        }else{
            if( Read(rgbaFd, pbuffer, readSize) < 0 ){
                break;
            }
        }

        s_DB[handle]->pInputBitStream = pbuffer;
        if( count == total ){
            s_DB[handle]->bLastFrm = 1;
        }else{
            s_DB[handle]->bLastFrm = 0;
        }
        LOGE("[nexAniGifjni.cpp %d] thread_encode encode start(%d)", __LINE__, count);
        rval = NxAniGIFStart(s_DB[handle]);
        LOGE("[nexAniGifjni.cpp %d] thread_encode encode end(%d)", __LINE__, count);

        if( rval == 0 && s_DB[handle]->nOutputGifSize == 0 ){
            LOGE("[nexAniGifjni.cpp %d] thread_encode need next frame end(%d)", __LINE__, count);
            count++;
            continue;
        }

        if( rval == 0 && s_DB[handle]->nOutputGifSize > 0 ){
            if( Write(outFd,s_DB[handle]->pOutputGIF,s_DB[handle]->nOutputGifSize) < 0 ){
                break;
            }
        }else{
            LOGE("[nexAniGifjni.cpp %d] thread_encode encode fail!(%d)", __LINE__, rval);
            break;
        }
        if( count == total ){
            break;
        }
        count++;
    }
    if( preadbuffer ){
        free(preadbuffer);
    }
    //unsigned char byte[]={0x3b};
    //write(outFd,byte,1);
    free(pbuffer);
    close(rgbaFd);
    close(outFd);
    LOGE("[nexAniGifjni.cpp %d] thread_encode end...", __LINE__);
    _destoryHandle(handle);

	//pthread_exit((void *) 0);
	return NULL;
	LOGE("[nexAniGifjni.cpp %d] thread_encode pthread_exit", __LINE__);
}


// buffer management apis
typedef struct _buffer_pool{
    unsigned char * pool;
    unsigned char * pstart;
    unsigned char * pend;
    int pool_size;
    int input_size;
    int eos;
    pthread_mutex_t mutex;
}BufferPool;

static int createBufferPool(BufferPool *bp, int size){
    bp->pool =(unsigned char *)malloc(size);
    if( bp->pool == NULL )
        return -1;
    bp->pstart = bp->pool;
    bp->pend = bp->pool;
    bp->pool_size = size;
    bp->input_size = 0;
    bp->eos = 0;
    pthread_mutex_init( &bp->mutex, NULL );
    return 0;
}

static void destroyBufferPool(BufferPool *bp){
    if( bp->pool ){
        free(bp->pool);
        bp->pool = NULL;
    }
    pthread_mutex_destroy(&bp->mutex);
}

static int remainSizeBufferPool(BufferPool *bp){
    return (bp->pool_size - bp->input_size);
}

static int putBufferPool(BufferPool *bp, unsigned char *buf, int size ){
    int remain = 0;
    pthread_mutex_lock(&bp->mutex);
    remain = remainSizeBufferPool(bp);
    //LOGE("[nexAniGifjni.cpp %d] putBufferPool remain(%d)!", __LINE__,remain);
    if ( remain < size ){
        pthread_mutex_unlock(&bp->mutex);
        return -1;
    }

    unsigned char *plast = bp->pool + bp->pool_size;
    int remain_size = size;
    if( bp->pend + size > plast ){
        remain_size -= (plast -  bp->pend);
        if( (plast -  bp->pend) > 0 )
            memcpy( bp->pend, buf , plast -  bp->pend );
        memcpy( bp->pool, &buf[plast -  bp->pend] , remain_size );
        bp->pend = bp->pool + remain_size;
    }else{
        memcpy( bp->pend, buf , size );
        bp->pend += size;
    }
    bp->input_size += size;
    pthread_mutex_unlock(&bp->mutex);
    return 0;
}

static int popBufferPool(BufferPool *bp, unsigned char *buf, int size ){
    int input_size = 0;
    unsigned char * p = NULL;
    unsigned char *plast = bp->pool + bp->pool_size;

    pthread_mutex_lock(&bp->mutex);

    input_size = bp->pool_size - remainSizeBufferPool(bp);
    p = bp->pstart;

    if( (input_size - size)  < 0 ){
        pthread_mutex_unlock(&bp->mutex);
        return 0;
    }

    bp->pstart += size;

    if(  plast <=  bp->pstart ){
        bp->pstart = bp->pool + (bp->pstart - plast);
    }

    memcpy(buf,p,size);

    bp->input_size -= size;
    pthread_mutex_unlock(&bp->mutex);
    return size;
}
//----------------------


static int s_useBufferPool;
static pthread_t s_t_output;
static BufferPool s_stPool;

static void *thread_input(void *arg)
{
    unsigned char * pbuffer;
    int buffSize;
    int handle;
    int rgbaFd;
    int format;
    EncoderParam *param = (EncoderParam*)arg;
    handle = param->handle;
    rgbaFd = param->rgbaFd;
    format = param->format;
    free(param);

    buffSize = s_DB[handle]->width * s_DB[handle]->height * 4;
    pbuffer = (unsigned char *)malloc(buffSize);


    while(1){
        if( Read(rgbaFd, pbuffer, buffSize) < 0 ){
            break;
        }
        while(1){
            if( putBufferPool(&s_stPool,pbuffer,buffSize) == 0 )
                break;
            usleep(40000);
        }
    }

    s_stPool.eos = 1;
    free(pbuffer);
    close(rgbaFd);

    LOGE("[nexAniGifjni.cpp %d] thread_input end!", __LINE__);
    pthread_exit((void *) 0);
}

static void *thread_output(void *arg)
{
    unsigned char * pbuffer;
    int buffSize;
    int outFd;
    int rval;
    int handle;
    int count = 0;
    int max = 0;
    int format = 0;

    EncoderParam *param = (EncoderParam*)arg;
    handle = param->handle;
    outFd = param->outFd;
    format = param->format;
    free(param);

    buffSize = s_DB[handle]->width * s_DB[handle]->height * 4;
    max = s_DB[handle]->width * s_DB[handle]->height * OUTPUT_BLOCK_COUNT;
    pbuffer = (unsigned char *)malloc(buffSize);
    if( pbuffer == NULL ){
        LOGE("[nexAniGifjni.cpp %d] thread_output malloc fail!", __LINE__);
        close(outFd);
        pthread_exit((void *) 1);
    }

    while(1){
        if( popBufferPool(&s_stPool,pbuffer,buffSize) == 0 ){
            if( s_stPool.eos ){
                break;
            }
            //LOGE("[nexAniGifjni.cpp %d] thread_output NULL !", __LINE__);
            usleep(10000);
            continue;
        }
        s_DB[handle]->pInputBitStream = pbuffer;

        rval = NxAniGIFStart(s_DB[handle]);
        LOGI("[nexAniGifjni.cpp %d] thread_output encode end(%d)", __LINE__, count);

        if( rval == 0 && s_DB[handle]->nOutputGifSize == 0 ){
            //LOGI("[nexAniGifjni.cpp %d] thread_output need next frame end(%d)", __LINE__, count);
            count++;
            continue;
         }

        if( rval == 0 && s_DB[handle]->nOutputGifSize > 0 ){

            if( s_DB[handle]->nOutputGifSize > max ){
                LOGE("[nexAniGifjni.cpp %d] fatal(%d) size memory overflow(%d)", __LINE__, handle,s_DB[handle]->nOutputGifSize);
            }

            if( Write(outFd,s_DB[handle]->pOutputGIF,s_DB[handle]->nOutputGifSize) < 0 ){
                break;
            }
        }else{
            LOGE("[nexAniGifjni.cpp %d] thread_output encode fail!(%d)", __LINE__, rval);
            break;
        }

        count++;
        //LOGE("[nexAniGifjni.cpp %d] thread_output (%d~%d)!", __LINE__,szbuf[0],szbuf[9]);
        //usleep(100000);
    }

    s_DB[handle]->pInputBitStream = NULL;
    s_DB[handle]->bLastFrm = 1;
    rval = NxAniGIFStart(s_DB[handle]);
    if( rval == 0 && s_DB[handle]->nOutputGifSize > 0 ){
        Write(outFd,s_DB[handle]->pOutputGIF,s_DB[handle]->nOutputGifSize);
    }

    LOGI("[nexAniGifjni.cpp %d] thread_output end!", __LINE__);
    free(pbuffer);
    close(outFd);
    pthread_exit((void *) 0);
}

static int test_pool(int handle, const char * cachedRGBAFile, const char * outFile){
    pthread_t input_t;
    pthread_t output_t;
    int status;
    int rgbaFd;
    int outFd;
    EncoderParam * inputParam;
    EncoderParam * outputParam;
    int buffSize;

/*
    if( s_DB[handle]->nNumCores > 1 ){
        s_DB[handle]->nNumCores = 1;
        NxAniGIFInit(s_DB[handle]);
    }
*/
    s_useBufferPool = 1;
    buffSize = s_DB[handle]->width * s_DB[handle]->height * 4;
    createBufferPool(&s_stPool,buffSize*4);

    rgbaFd = open(cachedRGBAFile,O_RDONLY,0666);
    if( rgbaFd < 0 ){
        LOGE("[nexAniGifjni.cpp %d] encoderAsync rgbaFd open fail!", __LINE__);
        return -1;
    }

    outFd = open(outFile,O_CREAT|O_WRONLY|O_TRUNC,0666);
    if( outFd < 0 ){
        LOGE("[nexAniGifjni.cpp %d] encoderAsync outFd open fail!", __LINE__);
	    close(rgbaFd);
        return -1;
    }

    inputParam = (EncoderParam*)malloc(sizeof(EncoderParam));
    inputParam->handle = handle;
    inputParam->rgbaFd = rgbaFd;
    inputParam->outFd = outFd;
    inputParam->format = 0;

    if (pthread_create(&input_t, NULL, &thread_input, (void *)inputParam) != 0) {
        LOGE("[nexAniGifjni.cpp %d] thread_input pthread_create fail!", __LINE__);
        return -2;
    }

    outputParam = (EncoderParam*)malloc(sizeof(EncoderParam));
    outputParam->handle = handle;
    outputParam->rgbaFd = rgbaFd;
    outputParam->outFd = outFd;
    outputParam->format = 0;
    if (pthread_create(&output_t, NULL, &thread_output, (void *)outputParam) != 0) {
        LOGE("[nexAniGifjni.cpp %d] thread_output pthread_create fail!", __LINE__);
        return -3;
    }
    //pthread_join(input_t, (void **)&status);
    pthread_join(output_t, (void **)&status);

    destroyBufferPool(&s_stPool);
    s_useBufferPool = 0;

    LOGE("[nexAniGifjni.cpp %d] test_pool End", __LINE__);
    return 0;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_getHandle(JNIEnv* env, jobject thiz){
    int handle;

    if( time_lock() < 0 ){
        LOGE("[nexAniGifjni.cpp %d] getHandle create handle fail!(%d)", __LINE__,handle);
        return ECODE_INVALID_Handle;
    }

    LOGI("libNxAniGIF version: %d.%d.%d"
        ,NxAniGIFGetVersionNum(0)
        ,NxAniGIFGetVersionNum(1)
        ,NxAniGIFGetVersionNum(2) );

   	handle = _createHandle();
    if( handle < 0 ){
        LOGE("[nexAniGifjni.cpp %d] getHandle create handle fail!(%d)", __LINE__,handle);
        return ECODE_INVALID_Handle;
    }
    return handle;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_freeHandle(JNIEnv* env, jobject thiz, jint handle){

    if( s_DB[handle] == NULL ){
        LOGE("[nexAniGifjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }
    return _destoryHandle(handle);
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_initHandle(JNIEnv* env, jobject thiz, jint handle, jint cpuCoreCount,  jint width, jint height, jint mode, jint delayMs, jint quantization_method, jfloat quantization_gamma, jint sample_factor, jint rgb666){
    int iRet;

    if( s_DB[handle] == NULL ){
        LOGE("[nexAniGifjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }

    if( s_DB[handle]->pOutputGIF != NULL ){
        free(s_DB[handle]->pOutputGIF);
    }

    s_DB[handle]->pOutputGIF = (unsigned char*)malloc(sizeof(unsigned char)*width*height*OUTPUT_BLOCK_COUNT);
    s_DB[handle]->width = width;
    s_DB[handle]->height = height;
    s_DB[handle]->mode = mode;
    s_DB[handle]->delayMs = delayMs;
    if (rgb666)
        s_DB[handle]->nUseChacheSize = 1;
    else // rgb888
        s_DB[handle]->nUseChacheSize = 4160*3120*2;

    s_DB[handle]->quantization_method = quantization_method;
    s_DB[handle]->quantization_gamma = quantization_gamma;
    s_DB[handle]->sample_factor = sample_factor;

    s_DB[handle]->nNumCores = cpuCoreCount; //test
//    s_DB[handle]->bOutputRepeat = 1; // repeat

    iRet = NxAniGIFInit(s_DB[handle]);
    LOGI("[nexAniGifjni.cpp %d] NxAniGIFInit result(%u) (cpu:%d,w:%d,h:%d,m:%d,d:%d)(qm:%d,qg:%f,sf:%d)", __LINE__, iRet,cpuCoreCount,width,height,mode,delayMs,quantization_method,quantization_gamma,sample_factor);
    return iRet;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_pushByteRGBAFixels(JNIEnv* env, jobject thiz, jint handle,jint flags,  jbyteArray rgba_fixel){
    jbyte *jarr;
    int length;
    int iRet;

    if( s_DB[handle] == NULL ){
        LOGE("[nexAniGifjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }

    if( (rgba_fixel == NULL) && (flags == FrameFlag_EoS) ){
        LOGE("[nexAniGifjni.cpp %d] EoS(%d)", __LINE__, handle);
        s_DB[handle]->bLastFrm = 1;
        s_DB[handle]->pInputBitStream = NULL;
        iRet = NxAniGIFStart(s_DB[handle]);
        return s_DB[handle]->nOutputGifSize;
    }

    jsize size = env->GetArrayLength(rgba_fixel);
    jarr = env->GetByteArrayElements(rgba_fixel,NULL);
    if( jarr == NULL){
        LOGE("[nexAniGifjni.cpp %d] invalid input data! size(%d)", __LINE__, size);
        return ECODE_INVALID_Argument;
    }

    //LOGI("[nxAniGifjni.cpp %d] push[%x][%x][%x][%x])(input size=%d)", __LINE__, (unsigned char )jarr[0],(unsigned char )jarr[1],(unsigned char )jarr[2],(unsigned char )jarr[3], size);
    s_DB[handle]->bLastFrm = flags;
    s_DB[handle]->pInputBitStream = (unsigned char *)jarr;
    iRet = NxAniGIFStart(s_DB[handle]);
    //LOGI("[nxAniGifjni.cpp %d] NxAniGIFStart result(%d)(output size=%d)", __LINE__, iRet,s_DB[handle]->nOutputGifSize);
    env->ReleaseByteArrayElements(rgba_fixel, jarr, 0);

    if( iRet < 0 ){
        LOGE("[nexAniGifjni.cpp %d] NxAniGIFStart error(%d)", __LINE__, iRet);
        return ECODE_FailOfEncode;
    }

    length = s_DB[handle]->nOutputGifSize;

    return length;
}

JNIEXPORT jbyteArray JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_popResult(JNIEnv* env, jobject thiz, jint handle){
    jbyteArray jarr;
    int max = 0;
    if( s_DB[handle] == NULL ){
        LOGE("[nexAniGifjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        jarr = env->NewByteArray(0);
        return jarr;
    }
    jsize length = (jsize)s_DB[handle]->nOutputGifSize;

    if( length == 0 ){
        LOGE("[nexAniGifjni.cpp %d] output size zero(%d)", __LINE__, handle);
        jarr = env->NewByteArray(0);
        return jarr;
    }

    max = s_DB[handle]->width * s_DB[handle]->height * OUTPUT_BLOCK_COUNT;

    if( length > max ){
        LOGE("[nexAniGifjni.cpp %d] fatal(%d) size memory overflow(%d)", __LINE__, handle,length);
    }
    /*
    LOGI("[nxAniGifjni.cpp %d] gif output size(%d) [%d][%d][%d][%d]", __LINE__, length
        ,s_DB[handle]->pOutputGIF[0]
        ,s_DB[handle]->pOutputGIF[1]
        ,s_DB[handle]->pOutputGIF[2]
        ,s_DB[handle]->pOutputGIF[3]
        );
        */
    //jbyte *pb;
    jarr = env->NewByteArray(length);
    env->SetByteArrayRegion(jarr, 0, length, (const jbyte*)s_DB[handle]->pOutputGIF);
    /*
    pb = env->GetByteArrayElements(jarr,0);
    if( pb != NULL ){
        memcpy(pb,s_DB[handle]->pOutputGIF,length);
        env->ReleaseByteArrayElements(jarr,pb,0);
    }else{
        LOGE("[nxAniGifjni.cpp %d] GetByteArrayElements fail!(%d)", __LINE__, handle);
    }
    */
    return jarr;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_test(JNIEnv* env, jobject thiz, jint handle, jstring cachedRGBAFile, jstring outFile ) {
    const char *pszRgbaPath;
    const char *pszOutPath;

    if( s_DB[handle] == NULL ){
        LOGE("[nexAniGifjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return -1;
    }

	pszRgbaPath = env->GetStringUTFChars(cachedRGBAFile, NULL);
	if( pszRgbaPath == NULL )
	{
		LOGE("[nexAniGifjni.cpp %d] encoderAsync failed because did not get cachedRGBAFile path", __LINE__);
		return -1;
	}

	pszOutPath = env->GetStringUTFChars(outFile, NULL);
	if( pszOutPath == NULL )
	{
		LOGE("[nexAniGifjni.cpp %d] encoderAsync failed because did not get pszOutPath path", __LINE__);
		return -1;
	}

	test_pool(handle,pszRgbaPath,pszOutPath);

    env->ReleaseStringUTFChars(cachedRGBAFile, pszRgbaPath);
    env->ReleaseStringUTFChars(outFile, pszOutPath);


    return 0;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_encodeFileToFile(JNIEnv* env, jobject thiz, jint handle, jstring cachedRGBAFile,jint format, jstring outFile, jint async ) {
    int rgbaFd = -1;
    int outFd = -1;
    const char *pszRgbaPath;
    const char *pszOutPath;
    EncoderParam *param;
    if( s_DB[handle] == NULL ){
        LOGE("[nexAniGifjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return -1;
    }

	pszRgbaPath = env->GetStringUTFChars(cachedRGBAFile, NULL);
	if( pszRgbaPath == NULL )
	{
		LOGE("[nexAniGifjni.cpp %d] encoderAsync failed because did not get cachedRGBAFile path", __LINE__);
		return -1;
	}

	pszOutPath = env->GetStringUTFChars(outFile, NULL);
	if( pszOutPath == NULL )
	{
		LOGE("[nexAniGifjni.cpp %d] encoderAsync failed because did not get pszOutPath path", __LINE__);
		return -1;
	}

    rgbaFd = open(pszRgbaPath,O_RDONLY,0666);
    if( rgbaFd < 0 ){
        LOGE("[nexAniGifjni.cpp %d] encoderAsync rgbaFd open fail!", __LINE__);
        env->ReleaseStringUTFChars(cachedRGBAFile, pszRgbaPath);
	    env->ReleaseStringUTFChars(outFile, pszOutPath);
        return -1;
    }

    outFd = open(pszOutPath,O_CREAT|O_WRONLY|O_TRUNC,0666);
    if( outFd < 0 ){
        LOGE("[nexAniGifjni.cpp %d] encoderAsync outFd open fail!", __LINE__);
        env->ReleaseStringUTFChars(cachedRGBAFile, pszRgbaPath);
	    env->ReleaseStringUTFChars(outFile, pszOutPath);
	    close(rgbaFd);
        return -1;
    }

    env->ReleaseStringUTFChars(cachedRGBAFile, pszRgbaPath);
	env->ReleaseStringUTFChars(outFile, pszOutPath);

    param = (EncoderParam*)malloc(sizeof(EncoderParam));
    param->handle = handle;
    param->rgbaFd = rgbaFd;
    param->outFd = outFd;
    param->format = format;
    if( async ){
        pthread_t some_thread;
        if (pthread_create(&some_thread, NULL, &thread_encode_file2file, (void *)param) != 0) {
            LOGE("[nexAniGifjni.cpp %d] encoderAsync pthread_create fail!", __LINE__);
            return -1;
        }
    }else{
        thread_encode_file2file(param);
    }
    LOGE("[nexAniGifjni.cpp %d] encodeFileToFile end", __LINE__);
    return 0;
}



JNIEXPORT jint JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_useBufferPool(JNIEnv* env, jobject thiz, jint handle, jstring outFile ) {
    int buffSize;
    EncoderParam * param;
    const char *pszOutPath;
    int outFd;
    int rval;

    if( s_useBufferPool == 1 ){
        LOGE("[nexAniGifjni.cpp %d] useBufferPool already(%d)", __LINE__, handle);
        return ECODE_INVALID_State;
    }

    if( s_DB[handle] == NULL ){
        LOGE("[nexAniGifjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }

   	pszOutPath = env->GetStringUTFChars(outFile, NULL);
   	if( pszOutPath == NULL )
   	{
   		LOGE("[nexAniGifjni.cpp %d] useBufferPool failed because did not get pszOutPath path", __LINE__);
   		return ECODE_INVALID_Argument;
   	}

    outFd = open(pszOutPath,O_CREAT|O_WRONLY|O_TRUNC,0666);
    env->ReleaseStringUTFChars(outFile, pszOutPath);
    if( outFd < 0 ){
        LOGE("[nexAniGifjni.cpp %d] useBufferPool outFd open fail!", __LINE__);
        return ECODE_INVALID_Argument;
    }

    buffSize = s_DB[handle]->width * s_DB[handle]->height * 4;
    if( createBufferPool(&s_stPool,buffSize*4) != 0 ){
        LOGE("[nexAniGifjni.cpp %d] useBufferPool mem alloc fail!", __LINE__);
        return ECODE_FailOfCreate;
    }

    s_useBufferPool = 1;

    param = (EncoderParam*)malloc(sizeof(EncoderParam));
    param->handle = handle;
    param->outFd = outFd;
    if (pthread_create(&s_t_output, NULL, &thread_output, (void *)param) != 0) {
        LOGE("[nexAniGifjni.cpp %d] useBufferPool pthread_create fail!", __LINE__);
        return ECODE_FailOfCreate;
    }
    LOGI("[nexAniGifjni.cpp %d] useBufferPool pthread_create id=%lu", __LINE__,s_t_output);

    return 0;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_pushByteRGBA2BufferPool(JNIEnv* env, jobject thiz, jint handle,jint flags,  jbyteArray rgba_fixel){
    jbyte *jarr;
    int length;
    int iRet;
    if( s_useBufferPool == 0 ){
        return ECODE_INVALID_State;
    }

    if( s_DB[handle] == NULL ){
        LOGE("[nexAniGifjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }

    if( (rgba_fixel == NULL) && (flags == FrameFlag_EoS) ){
        LOGE("[nexAniGifjni.cpp %d] pushByteRGBA2BufferPool EoS(%d)", __LINE__, handle);
        s_stPool.eos = 1;
        return 0;
    }

    jsize size = env->GetArrayLength(rgba_fixel);
    jarr = env->GetByteArrayElements(rgba_fixel,NULL);
    if( jarr == NULL){
        LOGE("[nexAniGifjni.cpp %d] pushByteRGBA2BufferPool invalid input data! size(%d)", __LINE__, size);
        return ECODE_INVALID_Argument;
    }

    while(1){
        if( putBufferPool(&s_stPool,(unsigned char *)jarr,size) == 0 )
            break;
        usleep(10000);
    }
    env->ReleaseByteArrayElements(rgba_fixel, jarr, 0);

    return 0;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_anigifsdk_nexAniGifSDK_outputWait(JNIEnv* env, jobject thiz){
    //LOGE("[nexAniGifjni.cpp %d] outputWait(%d) thread_id=%lu start", __LINE__,s_useBufferPool,s_t_output);
    void* pRet;
    pthread_join(s_t_output, &pRet);
    s_t_output = 0;
    destroyBufferPool(&s_stPool);
    s_useBufferPool = 0;
    //LOGE("[nexAniGifjni.cpp %d] outputWait() End", __LINE__);
    return 0;
}

}//extern C
