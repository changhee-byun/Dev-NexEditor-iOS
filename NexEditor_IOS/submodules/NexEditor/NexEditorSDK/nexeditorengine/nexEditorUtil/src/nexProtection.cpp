/******************************************************************************
* File Name   :	nexProtection.cpp
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

#include "nexProtection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "NEXEDITOR"
#ifdef ANDROID
#include <android/log.h>

#include <sys/system_properties.h>

#include <time.h>

#ifndef LOGE
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

#ifndef LOGI
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#endif
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <sys/time.h>
extern "C" {
    void NSLog(CFStringRef format, ...);
}
#define LOGE(format, ...)   NSLog(CFSTR(format), ##__VA_ARGS__)
#define LOGI(format, ...)   NSLog(CFSTR(format), ##__VA_ARGS__)
#endif

#define APP_ID_NUMBER 100	//150
#define ARRAY_SIZE 128

char gPackageName[ARRAY_SIZE] = {0,};

NXINT32 getSDKInfo(NEXSDKInformation* pstSDKInfo)
{
    if(pstSDKInfo == NULL)
        return  1;

    memset(pstSDKInfo, 0, sizeof(NEXSDKInformation));
    pstSDKInfo->bCheckMFGLock = 1;
	pstSDKInfo->bCheckPKGLock=1;

    int i, j;
	
    NEXSECUREINFO 	stSecureInfo;
    memset(&stSecureInfo, 0, sizeof(NEXSECUREINFO));
		
    if(NexSecure_Create(NEXSECURE_MAJOR_VERSION, NEXSECURE_MINOR_VERSION) == NEXSECURE_ERROR_NONE) {
        NexSecure_SetBitsData((unsigned char*)NexSDKInfo_GetInfoPtr(), NexSDKInfo_GetInfoSize());
        NexSecure_GetInfo(&stSecureInfo);

        char packageName[12] = {0};
		
        for(i=0, j=0; i<stSecureInfo.m_uiSecureItemCount; i++) {		
            if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "ProjectID") == 0)
            {
                strcpy(pstSDKInfo->pProjectID, stSecureInfo.m_SecureItem[i].m_strItemValue);
            }
            else if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "SDKName") == 0)
            {
                strcpy(pstSDKInfo->pSDKName, stSecureInfo.m_SecureItem[i].m_strItemValue);
            }
            else if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "CKTimeLock") == 0)
            {
                pstSDKInfo->bCheckTimeLock= atoi(stSecureInfo.m_SecureItem[i].m_strItemValue);
                //if ( pstSDKInfo->bCheckTimeLock ) LOGI("[nexProtection.cpp %d] Time lock is enabled\n", __LINE__);
            }
            else if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "LOCKStart") == 0)
            {
                strcpy(pstSDKInfo->LockStart, stSecureInfo.m_SecureItem[i].m_strItemValue);
            }
            else if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "LOCKEnd") == 0)
            {
                strcpy(pstSDKInfo->LockEnd, stSecureInfo.m_SecureItem[i].m_strItemValue);
            }
            // JDKIM : end
            else if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "AppUCode") == 0)
            {
                strcpy(pstSDKInfo->AppUCode, stSecureInfo.m_SecureItem[i].m_strItemValue);
            }
            else if (strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "LICENSEKEY") == 0)
            {
                strcpy(pstSDKInfo->LicenseKey, stSecureInfo.m_SecureItem[i].m_strItemValue);
            }
            else if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "CKPKGName") == 0)
            {
                pstSDKInfo->bCheckPKGname = atoi(stSecureInfo.m_SecureItem[i].m_strItemValue);
            }
            else if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "WaterMark") == 0)
            {
                pstSDKInfo->bCheckWaterMark= atoi(stSecureInfo.m_SecureItem[i].m_strItemValue);
            }
			else if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "CKMFGLock") == 0)
            {
                pstSDKInfo->bCheckMFGLock= atoi(stSecureInfo.m_SecureItem[i].m_strItemValue);
                //if ( pstSDKInfo->bCheckMFGLock ) LOGI("[nexProtection.cpp %d] Manufacture lock is enabled\n", __LINE__);
            }
			else if(strcmp(stSecureInfo.m_SecureItem[i].m_strItemName, "CKPKGLock") == 0)
            {
                pstSDKInfo->bCheckPKGLock= atoi(stSecureInfo.m_SecureItem[i].m_strItemValue);
                //if ( pstSDKInfo->bCheckPKGLock ) LOGI("[nexProtection.cpp %d] Package lock is enabled\n", __LINE__);
            }

            if(strncmp(stSecureInfo.m_SecureItem[i].m_strItemName, "PKGName", 7) == 0)
            {
				if(strlen(stSecureInfo.m_SecureItem[i].m_strItemValue) != 0)
				{
					strcpy(pstSDKInfo->pPKGName[j], stSecureInfo.m_SecureItem[i].m_strItemValue);
					//LOGI("[nexProtection.cpp %d] PKGName = [%s]\n", __LINE__, pstSDKInfo->pPKGName[j]);
					j++;
				}
            }
        }
        //LOGI("[nexProtection.cpp %d] Lock state[%d/%d/%d]\n", __LINE__, pstSDKInfo->bCheckTimeLock, pstSDKInfo->bCheckMFGLock, pstSDKInfo->bCheckPKGLock);

        NexSecure_Destroy();
        return 0;
    }
    return 1;	
}

int checkTimeout(NEXSDKInformation *pstSDKInfo)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Check time lock
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(pstSDKInfo->bCheckTimeLock) {
        // Current date
        tm *tmCurr;
        timeval tv;

        gettimeofday(&tv, NULL);
        tmCurr = localtime(&tv.tv_sec);

        LOGI("[nexProtection.cpp %d] Current Time(%ld sec) : %d/%d/%d  %d:%d:%d\n",__LINE__,tv.tv_sec, tmCurr->tm_year+1900, tmCurr->tm_mon+1, tmCurr->tm_mday, tmCurr->tm_hour, tmCurr->tm_min, tmCurr->tm_sec);

        // Compare Lock Date
        int nCurrYear, nCurrMonth, nCurrDay;
        int nStartYear = 0, nStartMonth = 0, nStartDay = 0;
        int nEndYear = 0, nEndMonth = 0, nEndDay = 0;
        time_t	tCurr = 0, tStart = 0, tEnd = 0;
        tm			tmStart; 
        tm			tmEnd;

        memset(&tmStart, 0, sizeof(tm));
        memset(&tmEnd, 0, sizeof(tm));

        nCurrYear		= tmCurr->tm_year+1900;
        nCurrMonth		= tmCurr->tm_mon+1;
        nCurrDay		= tmCurr->tm_mday;

        tCurr = tv.tv_sec;

        if(pstSDKInfo->LockStart[0] != '0') {
            sscanf(pstSDKInfo->LockStart, "%d/%d/%d", &nStartYear, &nStartMonth, &nStartDay);

            tmStart.tm_year = nStartYear - 1900;
            tmStart.tm_mon = nStartMonth - 1;
            tmStart.tm_mday = nStartDay;

            tStart = mktime(&tmStart);

            LOGI("[nexProtection.cpp %d] Start Time(%ld sec) : %d/%d/%d\n",__LINE__,tStart, nStartYear, nStartMonth, nStartDay);
        }

        if(pstSDKInfo->LockEnd[0] != '0') {
            sscanf(pstSDKInfo->LockEnd, "%d/%d/%d", &nEndYear, &nEndMonth, &nEndDay);

            tmEnd.tm_year = nEndYear - 1900;
            tmEnd.tm_mon = nEndMonth - 1;
            tmEnd.tm_mday = nEndDay;
            tmEnd.tm_hour = 23;
            tmEnd.tm_min = 59;
            tmEnd.tm_sec = 59;

            tEnd = mktime(&tmEnd);

            LOGI("[nexProtection.cpp %d] End Time(%ld sec) : %d/%d/%d\n",__LINE__, tEnd, nEndYear, nEndMonth, nEndDay);
        }

        if(tStart == 0 && tEnd == 0) {
            LOGI("[nexProtection.cpp %d] No Time Lock\n",__LINE__);
        } else if(tStart == 0) {
            if(tCurr > tEnd) {
                LOGI("[nexProtection.cpp %d] This SDK is expired at %d/%d/%d\n",__LINE__, nEndYear, nEndMonth, nEndDay);
                return 1;
            }
        } else if(tEnd == 0) {
            if(tCurr < tStart) {
                LOGI("[nexProtection.cpp %d] This SDK is not validate. It's available from %d/%d/%d\n",__LINE__, nStartYear, nStartMonth, nStartDay);
                return 1;
            }
        } else {
            if(tCurr < tStart) {
                LOGI("[nexProtection.cpp %d] This SDK is not validate. It's available from %d/%d/%d\n",__LINE__, nStartYear, nStartMonth, nStartDay);
                return 1;
            } else if(tCurr > tEnd) {
                LOGI("[nexProtection.cpp %d] This SDK is expired at %d/%d/%d\n",__LINE__, nEndYear, nEndMonth, nEndDay);
                return 1;
            }
        }
    }
    return 0;
}

char* strToUpper(char *str)
{
    int i = 0;
    char *p = (char*) malloc((strlen(str) + 1) * sizeof(char));
    for(; str[i] != '\0'; ++i) {
        if((str[i] >= 'a') && (str[i] <= 'z'))
            p[i] = str[i] + 'A' - 'a';
        else
            p[i] = str[i];
    }
    p[i] = '\0';
    return p;
}

#ifdef ANDROID
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#include <android/log.h>
#include <dlfcn.h>

typedef int (*PFN_SYSTEM_PROP_GET)(const char *, char *);
static int __nex_system_property_get(const char* name, char* value)
{
    static PFN_SYSTEM_PROP_GET __real_system_property_get = NULL;
    if (!__real_system_property_get) {
        // libc.so should already be open, get a handle to it.
        void *handle = dlopen("libc.so", RTLD_NOLOAD);
        if (!handle) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot dlopen libc.so: %s.\n", dlerror());
        } else {
            __real_system_property_get = (PFN_SYSTEM_PROP_GET)dlsym(handle, "__system_property_get");
        }
        if (!__real_system_property_get) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot resolve __system_property_get(): %s.\n", dlerror());
        }
    }
    return (*__real_system_property_get)(name, value);
} 
#endif

int systemPropertyGet(const char* p,char* v)
{
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
    return __nex_system_property_get( p,v);
#else
	return __system_property_get(p,v);
#endif
}

bool checkManufacturerName(NEXSDKInformation* pstSDKInfo)
{
    bool ret = false;
    
	if(pstSDKInfo->bCheckMFGLock == 0) {
		//LOGI("[nexProtection.cpp %d] No manufacture lock!\n", __LINE__);
		return false;
	}
    
    for(int j=0; j<APP_ID_NUMBER; j++) {
        if(strlen(pstSDKInfo->pPKGName[j]) != 0) {
            //
            char aManufacturer[128] = {0,};
            char* pManufacturerToUpper = NULL;
            
            systemPropertyGet("ro.product.manufacturer", aManufacturer);
            
            pManufacturerToUpper = strToUpper(aManufacturer);
            
            //
            if(strncmp(pstSDKInfo->pPKGName[j], pManufacturerToUpper,strlen(pstSDKInfo->pPKGName[j])) == 0) {
            	LOGI("[nexProtection.cpp %d] Manufacture lock is pass! : pstSDKInfo->pPKGName[%d](%s), package name(%s)", __LINE__, j, pstSDKInfo->pPKGName[j], pManufacturerToUpper);
                ret = false;
                break;
            } else {
                LOGI("[nexProtection.cpp %d] not available manufacturer: pstSDKInfo->pProjectID(%s), pManufacturertoupper(%s)", __LINE__, pstSDKInfo->pPKGName[j], pManufacturerToUpper);
                ret = true;
                continue;
            }
        } else {
            break;
        }
    }
    
    return ret;
}
#endif //#ifdef _ANDROID

bool checkPackageName(NEXSDKInformation* pstSDKInfo, char* pPackageName)
{
	bool ret = false;
	
	if(pstSDKInfo->bCheckPKGLock == 0) {
		//LOGI("[nexProtection.cpp %d] No package lock!\n", __LINE__);
		return false;
	}
	
	for(int j=0; j<APP_ID_NUMBER; j++) {
		if(strlen(pstSDKInfo->pPKGName[j]) != 0) {
            //
            if(strlen(pPackageName) != 0) {
                if(strcmp(pstSDKInfo->pPKGName[j], pPackageName) == 0) {
                	LOGI("[nexProtection.cpp %d] Package lock is pass! : pstSDKInfo->pPKGName[%d](%s), package name(%s)", __LINE__, j, pstSDKInfo->pPKGName[j], pPackageName);
                    ret = false;
                    break;
                } else {
                    LOGI("[nexProtection.cpp %d] not available pacakge: pstSDKInfo->pPKGName[%d](%s), package name(%s)", __LINE__, j, pstSDKInfo->pPKGName[j], pPackageName);
                    ret = true;
                    continue;
                }
            }
		} else {
			break;
		}
	}

	return ret;
}

void setPackageName4Protection(const char* packageName)
{
	memset(gPackageName, 0x0, ARRAY_SIZE);
	strcpy(gPackageName, packageName);
}

bool checkSDKProtection()
{
	NEXSDKInformation stSDKInfo;

	if(getSDKInfo(&stSDKInfo)) {
		return false;
	}
	//
	if(checkTimeout(&stSDKInfo)) {
		LOGI("[nexProtection.cpp %d] createEditor failed for timeout", __LINE__);
		return true;
    }
#ifdef ANDROID	
    if(checkManufacturerName(&stSDKInfo)) {
        LOGI("[nexProtection.cpp %d] createEditor failed for not available manufacture", __LINE__);
        return true;
    }
#endif//#ifdef _ANDROID
    if(checkPackageName(&stSDKInfo, gPackageName)) {
        LOGI("[nexProtection.cpp %d] createEditor failed for not available package_name. (%s)", __LINE__, gPackageName);
        return true;
    }
	
	return false;
}
