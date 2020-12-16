/******************************************************************************
 * File Name   : porting_ios.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#ifndef porting_ios_h
#define porting_ios_h

void registerSAL(void);
void unregisterSAL(void);
void registerCAL(char* strLibPath, char* strModelName, int iAPILevel, int iLogLevel);
void unregisterCAL();
void loadRAL( char* strLibPath, char* strModelName, int iAPILevel, int iLogLevel );
void unloadRAL();
void* getRALgetAudioRenderFuncs();
int * getSupportedDecoderCodecTypes( bool isVideo );
int * getSupportedEncoderCodecTypes( bool isVideo );
int countSupportedDecoderCodecTypes( bool isVideo );
int countSupportedEncoderCodecTypes( bool isVideo );

#endif /* porting_ios_h */
