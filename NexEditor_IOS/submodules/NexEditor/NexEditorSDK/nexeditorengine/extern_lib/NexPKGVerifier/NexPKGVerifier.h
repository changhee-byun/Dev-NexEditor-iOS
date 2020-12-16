/******************************************************************************
* File Name   :	NexPKGVerifier.h
* Description : APK verify routine.
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2019 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
Robin	2019/01/15	Draft.
-----------------------------------------------------------------------------*/

#ifndef _NEX_PKG_VERIFIER_
#define _NEX_PKG_VERIFIER_

#include <jni.h>

#define DEX_FILES_CNT		3
#define MD_SIZE			64
#define ENC_SIZE		1

#define NPV_MAJOR_VERSION	0
#define NPV_MINOR_VERSION	4
#define NPV_PATCH_VERSION	7

#define NPV_ERROR_NONE				0x00000000
// Error code for JOB. These can be combined.
#define NPV_ERROR_INIT				0x00010000
#define NPV_ERROR_DUPLICATE_SO		0x00020000
#define NPV_ERROR_PKG_LIST			0x00040000
#define NPV_ERROR_DEX_MD			0x00080000
#define NPV_ERROR_DEX_MD2			0x01000000
#define NPV_ERROR_DEX_MD3			0x02000000
#define NPV_ERROR_SO_MD				0x04000000
// Error code for others. These cannot be combined.
#define NPV_ERROR_GENERAL			0x00000001
#define NPV_ERROR_PACKAGE_MANAGER	0x00000002
#define NPV_ERROR_GET_PATH			0x00000003
#define NPV_ERROR_ATTACH_THREAD		0x00000004
#define NPV_ERROR_DSO_CNT_ZERO		0x00000005
#define NPV_ERROR_DSO_CNT_MULTI		0x00000006
#define NPV_ERROR_DSO_INVALID_PATH	0x00000007
#define NPV_ERROR_DEX_MD_MISMATCH	0x00000008
#define NPV_ERROR_SO_MD_MISMATCH	0x00000009
#define NPV_ERROR_SO_MD_FILE_SIZE	0x0000000A


typedef jint (*FuncFail)(JNIEnv *, jobject );

/*
** Verify package is hacked or not.
** Param :
**		(in) JNIEnv *env	: JAVA env
**		(in) iThreadCnt		: Thread count for multi-thread processing.
**		(in) func			: Function pointer which is called when the apk is hacked.
*/
int startPKGVerifier(JNIEnv *env, int iThreadCnt, const char* strLibPath, FuncFail func);

#endif // _NEX_PKG_VERIFIER_
