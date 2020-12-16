#ifndef _PORTING_ANDROID_HEADER_INCLUDED_
#define _PORTING_ANDROID_HEADER_INCLUDED_

//yoon
void registerJavaVM( void * java_vm );

void initCalRalHandle();

void registerSAL( int nLogLevel);			// JDKIM 2010/11/11
void unregisterSAL();

void registerSALLogforFile(const char* pFilePath);
void unregisterSALLogforFile();

void registerCAL( char* strLibPath, char* strModelName, int iAPILevel, int iLogLevel );
void unregisterCAL(int bSDK);

void loadRAL( char* strLibPath, char* strModelName, int iAPILevel, int iLogLevel );
void unloadRAL();

void* getCALCallbackFunc();
void* getCALInputSurf();
void* getCALsetTimeStampOnSurf();
void* getCALresetVideoEncoder();
void* getCALsetCropAchieveResolution();

void* getRALgetAudioRenderFuncs();

#endif //_PORTING_ANDROID_HEADER_INCLUDED_
