#include <jni.h>
#include <android/log.h>
#include <string.h>
#include "NxMETAAPI.h"
#include <stdio.h>
#include "SALBody_File.h"
#include "SALBody_Debug.h"
#include "SALBody_Mem.h"
#include "SALBody_SyncObj.h"
#include "SALBody_Task.h"
#include "SALBody_Time.h"
#include "KISA_SHA256.h"

#define  LOG_TAG    "nxMETA"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


#define ECODE_None                  0
#define ECODE_INVALID_Argument      -1
#define ECODE_MAX_Handle            -2
#define ECODE_INVALID_Handle        -3
#define ECODE_FailOfCreate          -4
#define ECODE_FailOfInit            -5
#define ECODE_FailOfUpdate          -6

extern "C" {

static void _NullDebugPrintf( char* pszFormat, ... )
{
}

static void _NullDebugOutputString( char* pszOutput )
{
}


void registerSAL( int nLogLevel )			// JDKIM 2010/11/11
{
	{
		nexSALBODY_SyncObjectsInit();

		// JDKIM 2010/11/11
		if(nLogLevel >= 1)
		{
			nexSAL_RegisterFunction( NEXSAL_REG_DBG_PRINTF, (void*)nexSALBody_DebugPrintf, NULL );
			nexSAL_RegisterFunction( NEXSAL_REG_DBG_OUTPUTSTRING, (void*)nexSALBody_DebugOutputString, NULL );
		}
		else
		{
			nexSAL_RegisterFunction( NEXSAL_REG_DBG_PRINTF, (void*)_NullDebugPrintf, NULL );
			nexSAL_RegisterFunction( NEXSAL_REG_DBG_OUTPUTSTRING, (void*)_NullDebugOutputString, NULL );
		}

		// JDKIM : end

		// Heap
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_ALLOC, (void*)nexSALBody_MemAlloc, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_CALLOC, (void*)nexSALBody_MemCalloc, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_FREE, (void*)nexSALBody_MemFree, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_ALLOC2, (void*)nexSALBody_MemAlloc2, NULL );			// JDKIM 2010/09/09
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_FREE2, (void*)nexSALBody_MemFree2, NULL );				// JDKIM 2010/09/09

		// File
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_OPENA, (void*)nexSALBody_FileOpen, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_CLOSE, (void*)nexSALBody_FileClose, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_READ, (void*)nexSALBody_FileRead, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_WRITE, (void*)nexSALBody_FileWrite, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_SEEK, (void*)nexSALBody_FileSeek, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_SEEK64, (void*)nexSALBody_FileSeek64, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_SIZE, (void*)nexSALBody_FileSize, NULL);
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_REMOVEA, (void*)nexSALBody_FileRemove, NULL);


		// Sync Objects
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_CREATE, (void*)nexSALBody_EventCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_DELETE, (void*)nexSALBody_EventDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_SET, (void*)nexSALBody_EventSet, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_WAIT, (void*)nexSALBody_EventWait, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_CLEAR, (void*)nexSALBody_EventClear, NULL );

		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_CREATE, (void*)nexSALBody_MutexCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_DELETE, (void*)nexSALBody_MutexDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_LOCK, (void*)nexSALBody_MutexLock, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_UNLOCK, (void*)nexSALBody_MutexUnlock, NULL );

		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_CREATE, (void*)nexSALBody_SemaphoreCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_DELETE, (void*)nexSALBody_SemaphoreDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_RELEASE, (void*)nexSALBody_SemaphoreRelease, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_WAIT, (void*)nexSALBody_SemaphoreWait, NULL );

		// Task
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_CREATE, (void*)nexSALBody_TaskCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_DELETE, (void*)nexSALBody_TaskDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_SLEEP, (void*)nexSALBody_TaskSleep, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_WAIT, (void*)nexSALBody_TaskWait, NULL );


		nexSAL_RegisterFunction( NEXSAL_REG_GETTICKCOUNT, (void*)nexSALBody_GetTickCount, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_GETMSECFROMEPOCH, (void*)nexSALBody_GetMSecFromEpoch, NULL );

		// JDKIM 2010/11/01
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_FLOW, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_E_AUDIO, 0);// nLogLevel);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_E_VIDEO, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_P_AUDIO, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_P_VIDEO, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_E_SYS, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_P_SYS, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_TEXT, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_DLOAD, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_INFO, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_WARNING, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_ERR, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_F_READER, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_F_WRITER, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_PVPD, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_PROTOCOL, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_CRAL, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_SOURCE, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_TARGET, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_DIVXDRM, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_RFC, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_NONE, 0);
		// JDKIM : end
	}

//	pEngine->m_iSALRef++;
}

void unregisterSAL()
{
	//if(pEngine)
	{
		nexSALBODY_SyncObjectsDeinit();
		//pEngine->m_iSALRef--;
	}
}

#define MAX_HANDLE_COUNT 16

static pNxMETA s_DB[MAX_HANDLE_COUNT];
static int s_handle_count = 0;

static NxMETAFileAPI s_FileAPI={
    nexSALBody_FileOpen,
    nexSALBody_FileClose,
    nexSALBody_FileSeek,
    nexSALBody_FileSeek64,
    nexSALBody_FileRead,
    nexSALBody_FileWrite,
    nexSALBody_FileSize
};


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
    if( s_handle_count == 1 ){
        registerSAL(1);
    }

    pNxMETA pSC =  NxMETACreate();
    if( pSC == NULL ){
        LOGE("[nxSCMBSDKjni.cpp %d] _createHandle create fail!", __LINE__);
        s_handle_count--;
        if( s_handle_count == 0 ){
            unregisterSAL();
        }
        return ECODE_FailOfCreate;
    }


    NEX_FF_RETURN eRetval;
    eRetval = NxMETARegisterFileAPI(pSC, &s_FileAPI);
    if(eNEX_FF_RET_SUCCESS != eRetval)
    {
    // error
        LOGE("[nxSCMBSDKjni.cpp %d] _createHandle registe fail!", __LINE__);
        s_handle_count--;
        if( s_handle_count == 0 ){
            unregisterSAL();
        }
        return ECODE_FailOfCreate;
    }

    s_DB[i] = pSC;
    return i;
}

static int _destoryHandle(int index){
    if( s_DB[index] != NULL ){
        NxMETADestroy(s_DB[index]);
        s_DB[index] = 0;
        s_handle_count--;
        if( s_handle_count == 0 ){
            unregisterSAL();
        }
        return ECODE_None;
    }
    return ECODE_INVALID_Handle;
}

char* jstringTostring(JNIEnv* env, jstring jstr, const char * pend )
{
        char* rtn = NULL;
        int pend_len = 0;

        jclass clsstring = env->FindClass( "java/lang/String");
        jstring strencode = env->NewStringUTF("utf-8");
        jmethodID mid = env->GetMethodID( clsstring, "getBytes", "(Ljava/lang/String;)[B");
        jbyteArray barr= (jbyteArray)env->CallObjectMethod( jstr, mid, strencode);
        jsize alen = env->GetArrayLength( barr);
        jbyte* ba = env->GetByteArrayElements( barr, JNI_FALSE);


        if( pend != NULL ){
            pend_len = strlen(pend);
        }

        if (alen > 0)
        {
                rtn = (char*)malloc(pend_len + alen + 1);
                if( pend_len > 0 ){
                    memcpy(rtn, pend, pend_len);
                }
                memcpy(rtn+pend_len, ba, alen);
                rtn[pend_len + alen] = 0;
        }
        env->ReleaseByteArrayElements( barr, ba, 0);
        return rtn;
}

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


JNIEXPORT jint JNICALL Java_com_nexstreaming_scmbsdk_nxSCMBSDK_checkValidate(JNIEnv* env, jobject thiz, jobject context){

    time_lock();

	jclass context_clazz = env->GetObjectClass( context);
	jmethodID methodID_func = env->GetMethodID( context_clazz,"getPackageManager", "()Landroid/content/pm/PackageManager;");
	jobject package_manager = env->CallObjectMethod( context, methodID_func);

    jmethodID mid_packagename = env->GetMethodID( context_clazz, "getPackageName", "()Ljava/lang/String;");
    jstring packageName = (jstring) env->CallObjectMethod( context, mid_packagename);
    /*
    char * pn = jstringTostring(env,packageName,NULL);
    LOGE("[nxSCMBSDKjni.cpp %d] checkValidate package name(%s)", __LINE__,pn);
    free(pn);
    */

	jclass pm_clazz = env->GetObjectClass( package_manager);
	jmethodID methodID_pm = env->GetMethodID( pm_clazz,	"getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
	jobject package_info = env->CallObjectMethod(package_manager,methodID_pm, packageName, 64);

	jclass pi_cls = env->GetObjectClass( package_info);
    jfieldID fid = env->GetFieldID( pi_cls, "signatures", "[Landroid/content/pm/Signature;");
    jobjectArray signatures = (jobjectArray)env->GetObjectField( package_info, fid);
    jobject sign = env->GetObjectArrayElement( signatures, 0);

    jclass sign_cls = env->GetObjectClass( sign);
    jmethodID mid = env->GetMethodID(sign_cls, "toCharsString", "()Ljava/lang/String;");
    if(mid == NULL){
        return -1;
     }

    jstring signString = (jstring)env->CallObjectMethod( sign, mid);
    char * ss = jstringTostring(env, signString,"7G*_}edU4#WM}P9:{X}5QoW");
    //LOGE("[nxSCMBSDKjni.cpp %d] checkValidate sign(%s)", __LINE__,ss);
    BYTE szDigest[32];
    SHA256_Encrpyt( (const BYTE *)ss, strlen(ss), szDigest );
    free(ss);
/*
    char szPrintDigest[512], *ptr;
    ptr = szPrintDigest;
    ptr += sprintf(ptr,"unsigned char validDigest[] ={");
    for(int i = 0 ; i < 32 ; i++ ){
        ptr += sprintf(ptr,"0x%02X,",szDigest[i]);
    }
    ptr += sprintf(ptr,"0x00};");
    ptr = 0x00;
*/
//    LOGE("[nxSCMBSDKjni.cpp %d] checkValidate Digest[%s]", __LINE__,szPrintDigest);

/*
    BYTE validDigest[] ={0x41,0x67,0x78,0xD3,0xC5,0x17,0x6B,0x1B,0xB8,0x25,0x22,0x32,0x21,0xFA,0xC0,0x52,0xE9,0x79,0xD1,0xBF,0xEB,0x1C,0x31,0xFC,0xEC,0x1A,0xF2,0x0E,0x3C,0x05,0xE3,0x62,0x00};

    int mismatch = 0;
    for( int j = 0 ; j < 32 ; j++ ){
        if( szDigest[j] != validDigest[j] ){
            mismatch++;
        }
    }

    LOGE("[nxSCMBSDKjni.cpp %d] checkValidate mismatch count(%d)", __LINE__,mismatch);
*/
	return 0;
}


JNIEXPORT jint JNICALL Java_com_nexstreaming_scmbsdk_nxSCMBSDK_openFile(JNIEnv* env, jobject thiz, jstring filePath){
    const char *strPath = env->GetStringUTFChars(filePath, NULL);
    int handle;
   	if( strPath == NULL )
   	{
   		LOGE("[nxSCMBSDKjni.cpp %d] openFile failed because did not get file path", __LINE__);
   		return ECODE_INVALID_Argument;
   	}

   	LOGI("[nxSCMBSDKjni.cpp %d](ver:%s) openFile path(%s) 23", __LINE__,NxMETA_GetVersionString(),strPath);

   	handle = _createHandle();
    if( handle < 0 ){
        env->ReleaseStringUTFChars(filePath, strPath);
        LOGE("[nxSCMBSDKjni.cpp %d] openFile create handle fail!(%d)", __LINE__,handle);
        return ECODE_INVALID_Handle;
    }

    NEX_FF_RETURN eRetval;
    eRetval = NxMETAInit(s_DB[handle], (NXUINT8 *)strPath);
    if(eNEX_FF_RET_SUCCESS != eRetval)
    {
        env->ReleaseStringUTFChars(filePath, strPath);
        LOGE("[nxSCMBSDKjni.cpp %d] openFile init fail!(%d)", __LINE__,eRetval);
        return ECODE_FailOfInit;
    }

    env->ReleaseStringUTFChars(filePath, strPath);
    return handle;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_scmbsdk_nxSCMBSDK_closeFile(JNIEnv* env, jobject thiz, jint handle){
    if( s_DB[handle] == NULL ){
        LOGE("[nxSCMBSDKjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }
    NxMETAClose(s_DB[handle]);
    return _destoryHandle(handle);
}

jobject createEntry(JNIEnv * env,int type, int speed , long long start, long long end, unsigned char *extra, int extra_size)
{
	jfieldID		id;
	jobject 		objEntry;
	jmethodID	entry_construct;
	jclass		entry_ref_class;


	entry_ref_class = env->FindClass("com/nexstreaming/scmbsdk/nxSCMBSDK$Entry");

	if( entry_ref_class == NULL )
	{
	    LOGE("[nxSCMBSDKjni.cpp %d] createEntry entry_ref_class fail!(%d,%lld,%lld)", __LINE__, speed,start,end);
		return NULL;
	}
	entry_construct = env->GetMethodID(entry_ref_class, "<init>", "()V");
	if( entry_construct == NULL )
	{
	    LOGE("[nxSCMBSDKjni.cpp %d] createEntry construct fail!(%d,%lld,%lld)", __LINE__, speed,start,end);
		env->DeleteLocalRef(entry_ref_class);
		return NULL;
	}

	objEntry = env->NewObject(entry_ref_class, entry_construct);

	if( objEntry == NULL )
	{
	    LOGE("[nxSCMBSDKjni.cpp %d] createEntry objEntry fail!(%d,%lld,%lld)", __LINE__, speed,start,end);
		env->DeleteLocalRef(entry_ref_class);
		return NULL;
	}

	id = env->GetFieldID(entry_ref_class, "mType", "I");
	env->SetIntField(objEntry, id, type);

	id = env->GetFieldID(entry_ref_class, "mSpeed", "I");
	env->SetIntField(objEntry, id, speed);

	id = env->GetFieldID(entry_ref_class, "mStartMs", "J");
	env->SetLongField(objEntry, id, start);

	id = env->GetFieldID(entry_ref_class, "mEndMs", "J");
	env->SetLongField(objEntry, id, end);

    if( extra_size > 0 ){
        jbyteArray arr = env->NewByteArray(extra_size);
        env->SetByteArrayRegion(arr, 0, extra_size, (jbyte*)extra);
        id = env->GetFieldID(entry_ref_class, "mExtra", "[B");
        env->SetObjectField(objEntry, id, arr);

        //LOGI("[nxSCMBSDKjni.cpp %d] createEntry objEntry extra value is [%x][%x][%x][%x] size=%d", __LINE__,extra[0],extra[1],extra[2],extra[3],extra_size);
    }else{
    //TOODO
        ;
    }

	env->DeleteLocalRef(entry_ref_class);
	return objEntry;
}

JNIEXPORT jobjectArray  JNICALL Java_com_nexstreaming_scmbsdk_nxSCMBSDK_getEntries(JNIEnv* env, jobject thiz, jint handle, jint type){
    jobjectArray ret = NULL;

    if( s_DB[handle] == NULL ){
        LOGE("[nxSCMBSDKjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return NULL;
    }

    pNxMETA pSC = s_DB[handle];
    NEX_FF_RETURN eRetval;
    NXINT32 nEntryRet = 0;
    if( type  == 0 ){
        eRetval = NxMETASCExistEntryCheck(pSC, &nEntryRet);
    }else if( type == 1 ){
        eRetval = NxMETAExistEntryCheck(pSC, &nEntryRet);
    }
    if(eNEX_FF_RET_SUCCESS != eRetval)
    {
        LOGE("[nxSCMBSDKjni.cpp %d] getEntries Check(%d)", __LINE__,eRetval);
        return NULL;
    }
    if(nEntryRet < 0)
    {
    // No scmb atom in content
        LOGE("[nxSCMBSDKjni.cpp %d] getEntries Check(%d)", __LINE__,nEntryRet);
        return NULL;
    }
    else if(nEntryRet == 0)
    {
    // exist scmb atom, but no entry in scmb atom
        LOGE("[nxSCMBSDKjni.cpp %d] getEntries Check(%d)", __LINE__,nEntryRet);
        return NULL;
    }

    if( type == 0 )
    {
        SCMB_Entry *pEntry;
        pEntry = (SCMB_Entry*)calloc(nEntryRet, sizeof(SCMB_Entry));
        eRetval = NxMETASCEntry(pSC, NXMETA_GET_ENTRY, nEntryRet, pEntry);
        if(eNEX_FF_RET_SUCCESS != eRetval)
        {
        // error
            LOGE("[nxSCMBSDKjni.cpp %d] getEntries GET_ENTRY(%d,%d)", __LINE__,nEntryRet,eRetval);
            return NULL;
        }
        //LOGI("[nxSCMBSDKjni.cpp %d] getEntries GET_ENTRY(%d)", __LINE__,nEntryRet);
        jclass myClass = env->FindClass("com/nexstreaming/scmbsdk/nxSCMBSDK$Entry");
        ret= (jobjectArray)env->NewObjectArray(nEntryRet,myClass,NULL);

        for( int i = 0 ; i < nEntryRet ; i++ ){
            int speed = (int)(pEntry[i].uDivision*100/1000);
            long long start= (long long)(pEntry[i].uStart);
            long long end= (long long)(pEntry[i].uEnd);
            //LOGI("[nxSCMBSDKjni.cpp %d] getEntries index(%d)(%d,%lld,%lld,%d)", __LINE__,i,speed,start,end,pEntry[i].nExtLen);
            env->SetObjectArrayElement(ret,i,createEntry(env,type,speed,start,end, (unsigned char *)pEntry[i].pExt, (int)pEntry[i].nExtLen ));
            if( pEntry[i].pExt )
                free(pEntry[i].pExt);
        }

        return ret;
    }else if( type == 1 ){
       NXMT_Entry *pEntry;
        pEntry = (NXMT_Entry*)calloc(nEntryRet, sizeof(NXMT_Entry));
        eRetval = NxMETAEntry(pSC, NXMETA_GET_ENTRY, nEntryRet, pEntry);
        if(eNEX_FF_RET_SUCCESS != eRetval)
        {
        // error
            LOGE("[nxSCMBSDKjni.cpp %d] getEntries GET_ENTRY(%d,%d)", __LINE__,nEntryRet,eRetval);
            return NULL;
        }
        //LOGI("[nxSCMBSDKjni.cpp %d] getEntries GET_ENTRY(%d)", __LINE__,nEntryRet);
        jclass myClass = env->FindClass("com/nexstreaming/scmbsdk/nxSCMBSDK$Entry");
        ret= (jobjectArray)env->NewObjectArray(nEntryRet,myClass,NULL);

        for( int i = 0 ; i < nEntryRet ; i++ ){
            int speed = 0;
            long long start= 0;
            long long end= 0;
            //LOGI("[nxSCMBSDKjni.cpp %d] getEntries index(%d)(%d,%lld,%lld,%d)", __LINE__,i,speed,start,end,pEntry[i].nExtLen);
            env->SetObjectArrayElement(ret,i,createEntry(env,type,speed,start,end, (unsigned char *)pEntry[i].pBuf, (int)pEntry[i].nLength ));
            if( pEntry[i].pBuf )
                free(pEntry[i].pBuf);
        }

        return ret;
    }
    return NULL;
}

JNIEXPORT jint  JNICALL Java_com_nexstreaming_scmbsdk_nxSCMBSDK_setEntries(JNIEnv* env, jobject thiz, jint handle, jobjectArray entrys){
    if( s_DB[handle] == NULL ){
        LOGE("[nxSCMBSDKjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }
    pNxMETA pSC = s_DB[handle];
    NEX_FF_RETURN eRetval;
    SCMB_Entry *pEntry;

    jint type = 0;

    NxMETASCEntry(pSC, NXMETA_DEL_ENTRY, 0, NULL); //delete all

    if( entrys == NULL ){
        NxMETASCUpdate(pSC);
        return ECODE_None;
    }

    int count = env->GetArrayLength(entrys);

    if( count == 0 ){
        NxMETASCUpdate(pSC);
        return ECODE_None;
    }

    pEntry = (SCMB_Entry*)calloc(count, sizeof(SCMB_Entry));

    for (int i=0; i<count; i++) {
        jobject objEntry = env->GetObjectArrayElement(entrys, i);
		if( objEntry == NULL )
		{
		    LOGI("[nxSCMBSDKjni.cpp %d] setEntrys() objEntry is null", __LINE__);
			continue;
		}

		jfieldID		id;
		jclass		entry_ref_class;
		entry_ref_class = env->GetObjectClass(objEntry);
		if( entry_ref_class == NULL )
		{
		    LOGI("[nxSCMBSDKjni.cpp %d] setEntrys() entry_ref_class is null", __LINE__);
			env->DeleteLocalRef(objEntry);
			continue;
		}

		id = env->GetFieldID(entry_ref_class, "mType", "I");
		type = env->GetIntField(objEntry, id);

		id = env->GetFieldID(entry_ref_class, "mSpeed", "I");
		jint speed = env->GetIntField(objEntry, id);

		id = env->GetFieldID(entry_ref_class, "mStartMs", "J");
		jlong start = env->GetLongField(objEntry, id);

		id = env->GetFieldID(entry_ref_class, "mEndMs", "J");
		jlong end = env->GetLongField(objEntry, id);

        id = env->GetFieldID(entry_ref_class, "mExtra", "[B");
        jbyteArray extra = (jbyteArray)env->GetObjectField (objEntry, id);
        if( extra != NULL ){
            jsize extra_size = env->GetArrayLength(extra);
            if( extra_size == 0 ){
                pEntry[i].pExt = NULL;
                pEntry[i].nExtLen = 0;
            }else{
                pEntry[i].pExt = (NXUINT8*)malloc(extra_size);
                pEntry[i].nExtLen = extra_size;
                jbyte* pBuffer = (jbyte *)env->GetByteArrayElements(extra, 0);
                memcpy(pEntry[i].pExt, pBuffer,extra_size);
                env->ReleaseByteArrayElements(extra, pBuffer, 0);
                //LOGI("[nxSCMBSDKjni.cpp %d] setEntrys() (%d)extra value is [%x][%x][%x][%x] size=%d", __LINE__,i,pEntry[i].pExt[0],pEntry[i].pExt[1],pEntry[i].pExt[2],pEntry[i].pExt[3],extra_size);
            }
            env->DeleteLocalRef(extra);
        }

		pEntry[i].uDivision = (NXUINT32)(speed*1000/100);
		pEntry[i].uStart = (NXUINT64)(start);
		pEntry[i].uEnd = (NXUINT64)(end);


		//LOGI("[nxSCMBSDKjni.cpp %d] setEntrys() index(%d), (%d,%lld,%lld)", __LINE__,i,pEntry[i].uDivision,pEntry[i].uStart,pEntry[i].uEnd);

		env->DeleteLocalRef(objEntry);
		env->DeleteLocalRef(entry_ref_class);
    }
/*
    for( int j = 0 ; j < count ; j++ ){
        LOGI("[nxSCMBSDKjni.cpp %d] setEntrys() in memory index(%d), (%d,%lld,%lld)", __LINE__,j,pEntry[j].uDivision,pEntry[j].uStart,pEntry[j].uEnd);
    }
    */
    eRetval = NxMETASCEntry(pSC, NXMETA_ADD_ENTRY, count, pEntry);
    if(eNEX_FF_RET_SUCCESS != eRetval)
    {
        for( int j = 0 ; j < count ; j++ ){
            if( pEntry[j].pExt ){
                free(pEntry[j].pExt);
            }
        }

        LOGE("[nxSCMBSDKjni.cpp %d] setEntrys() add entry fail!(%d)", __LINE__,eRetval);
        return ECODE_FailOfUpdate;
    }

	eRetval = NxMETASCUpdate(pSC);
    if(eNEX_FF_RET_SUCCESS != eRetval)
    {
        for( int j = 0 ; j < count ; j++ ){
            if( pEntry[j].pExt ){
                free(pEntry[j].pExt);
            }
        }

         LOGE("[nxSCMBSDKjni.cpp %d] setEntrys() update fail!(%d)", __LINE__,eRetval);
         return ECODE_FailOfUpdate;
    }

    for( int j = 0 ; j < count ; j++ ){
        if( pEntry[j].pExt ){
            free(pEntry[j].pExt);
        }

    }
     LOGE("[nxSCMBSDKjni.cpp %d] setEntrys() type(%d) success!", __LINE__,type);
    return ECODE_None;
}

JNIEXPORT jint  JNICALL Java_com_nexstreaming_scmbsdk_nxSCMBSDK_setImageEntries(JNIEnv* env, jobject thiz, jint handle, jobjectArray entrys){
    if( s_DB[handle] == NULL ){
        LOGE("[nxSCMBSDKjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }
    pNxMETA pSC = s_DB[handle];
    NEX_FF_RETURN eRetval;
    NXMT_Entry *pEntry;
    int isOk = 1;
    if( entrys == NULL ){
        return ECODE_INVALID_Argument;
    }

    int count = env->GetArrayLength(entrys);

    if( count == 0 ){
        return ECODE_INVALID_Argument;
    }

    NXINT32 nEntryRet = 0;

    eRetval = NxMETAExistEntryCheck(pSC, &nEntryRet);

    if(eNEX_FF_RET_SUCCESS != eRetval)
    {
        LOGE("[nxSCMBSDKjni.cpp %d] setImageEntries Check(%d)", __LINE__,eRetval);
        return ECODE_FailOfUpdate;
    }

    if(nEntryRet > 0)
    {
        LOGE("[nxSCMBSDKjni.cpp %d] setImageEntries() include images(%d)", __LINE__,nEntryRet);
        return ECODE_FailOfUpdate;
    }


    pEntry = (NXMT_Entry*)calloc(count, sizeof(NXMT_Entry));

    for (int i=0; i<count; i++) {
        jobject objEntry = env->GetObjectArrayElement(entrys, i);
		if( objEntry == NULL )
		{
		    LOGI("[nxSCMBSDKjni.cpp %d] setImageEntries() objEntry is null", __LINE__);
			continue;
		}

		jfieldID		id;
		jclass		entry_ref_class;
		entry_ref_class = env->GetObjectClass(objEntry);
		if( entry_ref_class == NULL )
		{
		    LOGI("[nxSCMBSDKjni.cpp %d] setImageEntries() entry_ref_class is null", __LINE__);
			env->DeleteLocalRef(objEntry);
			continue;
		}

		id = env->GetFieldID(entry_ref_class, "mType", "I");
		jint type = env->GetIntField(objEntry, id);

		id = env->GetFieldID(entry_ref_class, "mSpeed", "I");
		jint speed = env->GetIntField(objEntry, id);

		id = env->GetFieldID(entry_ref_class, "mStartMs", "J");
		jlong start = env->GetLongField(objEntry, id);

		id = env->GetFieldID(entry_ref_class, "mEndMs", "J");
		jlong end = env->GetLongField(objEntry, id);

        id = env->GetFieldID(entry_ref_class, "mExtra", "[B");
        jbyteArray extra = (jbyteArray)env->GetObjectField (objEntry, id);
        if( extra != NULL ){
            jsize extra_size = env->GetArrayLength(extra);
            if( extra_size == 0 ){
                pEntry[i].pBuf = NULL;
                pEntry[i].nLength = 0;
                LOGI("[nxSCMBSDKjni.cpp %d] setImageEntries() data is null");
                isOk = 0;
            }else{
                pEntry[i].pBuf = (NXUINT8*)malloc(extra_size);
                pEntry[i].nLength = extra_size;
                jbyte* pBuffer = (jbyte *)env->GetByteArrayElements(extra, 0);
                memcpy(pEntry[i].pBuf, pBuffer,extra_size);
                env->ReleaseByteArrayElements(extra, pBuffer, 0);
                //LOGI("[nxSCMBSDKjni.cpp %d] setEntrys() (%d)extra value is [%x][%x][%x][%x] size=%d", __LINE__,i,pEntry[i].pExt[0],pEntry[i].pExt[1],pEntry[i].pExt[2],pEntry[i].pExt[3],extra_size);
            }
            env->DeleteLocalRef(extra);
        }else{
            LOGI("[nxSCMBSDKjni.cpp %d] setImageEntries() data is null");
            isOk = 0;
        }
		pEntry[i].nType = 0;

		//LOGI("[nxSCMBSDKjni.cpp %d] setEntrys() index(%d), (%d,%lld,%lld)", __LINE__,i,pEntry[i].uDivision,pEntry[i].uStart,pEntry[i].uEnd);

		env->DeleteLocalRef(objEntry);
		env->DeleteLocalRef(entry_ref_class);
    }

    if( isOk == 0 ){
        for( int j = 0 ; j < count ; j++ ){
            if( pEntry[j].pBuf ){
                free(pEntry[j].pBuf);
            }
        }

        LOGE("[nxSCMBSDKjni.cpp %d] setImageEntries() add entry fail! INVALID_Argument", __LINE__);
        return ECODE_INVALID_Argument;
    }

    eRetval = NxMETAEntry(pSC, NXMETA_ADD_ENTRY, count, pEntry);
    if(eNEX_FF_RET_SUCCESS != eRetval)
    {
        for( int j = 0 ; j < count ; j++ ){
            if( pEntry[j].pBuf ){
                free(pEntry[j].pBuf);
            }
        }

        LOGE("[nxSCMBSDKjni.cpp %d] setImageEntries() add entry fail!(%d)", __LINE__,eRetval);
        return ECODE_FailOfUpdate;
    }

    eRetval = NxMETAUpdate(pSC);
    if(eNEX_FF_RET_SUCCESS != eRetval)
    {
        for( int j = 0 ; j < count ; j++ ){
            if( pEntry[j].pBuf ){
                free(pEntry[j].pBuf);
            }
        }

         LOGE("[nxSCMBSDKjni.cpp %d] setImageEntries() update fail!(%d)", __LINE__,eRetval);
         return ECODE_FailOfUpdate;
    }

    for( int j = 0 ; j < count ; j++ ){
        if( pEntry[j].pBuf ){
            free(pEntry[j].pBuf);
        }
    }

    return ECODE_None;
}

}
