#ifndef _CALBODY_HEADER_
#define _CALBODY_HEADER_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#	define FALSE	0
#endif

#ifndef TRUE                    
#	define TRUE		1
#endif


#define NEXCAL_TRACE_CATEGORY_FLOW						0
#define NEXCAL_TRACE_CATEGORY_E_AUDIO					1
#define NEXCAL_TRACE_CATEGORY_E_VIDEO					2
#define NEXCAL_TRACE_CATEGORY_P_AUDIO				3
#define NEXCAL_TRACE_CATEGORY_P_VIDEO					4
#define NEXCAL_TRACE_CATEGORY_E_SYS						5
#define NEXCAL_TRACE_CATEGORY_P_SYS						6
#define NEXCAL_TRACE_CATEGORY_TEXT						7
#define NEXCAL_TRACE_CATEGORY_DLOAD					8
#define NEXCAL_TRACE_CATEGORY_INFO						9
#define NEXCAL_TRACE_CATEGORY_WARNING				10
#define NEXCAL_TRACE_CATEGORY_ERR							11
#define NEXCAL_TRACE_CATEGORY_F_READER				12
#define NEXCAL_TRACE_CATEGORY_F_WRITER				13
#define NEXCAL_TRACE_CATEGORY_PVPD						14
#define NEXCAL_TRACE_CATEGORY_PROTOCOL				15
#define NEXCAL_TRACE_CATEGORY_CRAL						16
#define NEXCAL_TRACE_CATEGORY_SOURCE					17
#define NEXCAL_TRACE_CATEGORY_TARGET					18
#define NEXCAL_TRACE_CATEGORY_DIVXDRM					19
#define NEXCAL_TRACE_CATEGORY_RFC						20
#define NEXCAL_TRACE_CATEGORY_NONE						21

void* nex_malloc( int iSize );
void nex_free( void *p );


unsigned int nexCALBody_Video_GetProperty( unsigned int uProperty, unsigned int *puValue, void* uUserData );
unsigned int nexCALBody_Video_SetProperty( unsigned int uProperty, unsigned int uValue, void* uUserData );
unsigned int nexCALBody_Audio_GetProperty( unsigned int uProperty, unsigned int *puValue, void* uUserData );
unsigned int nexCALBody_Audio_SetProperty( unsigned int uProperty, unsigned int uValue, void* uUserData );

unsigned int nexCALBody_Text_GetProperty( unsigned int uProperty, unsigned int *puValue );

void 	nexCAL_TraceCat( int iCategory, int iLevel, const char* pszFormat, ... );
void * 	nexCAL_MemAlloc(unsigned int uiSize);
void * 	nexCAL_MemCalloc(unsigned int uiNum, unsigned int uiSize);
void 	nexCAL_MemFree(void *pMem);
void 	nexCAL_TaskSleep(unsigned int uiSleep);
void 	nexCAL_MemDump(void *pMem, unsigned int uiLen);
unsigned int nexCAL_GetTickCount();


unsigned int nexCALBody_Dump_AudioInit(const char* strDumpPath, unsigned int uiLen, unsigned char* pData);
unsigned int nexCALBody_Dump_VideoInit(const  char* strDumpPath, unsigned int uiLen, unsigned char* pData);
unsigned int nexCALBody_Dump_Audio(const  char* strDumpPath, unsigned int uiLen, unsigned int uiDTS, unsigned char* pData);
unsigned int nexCALBody_Dump_Video(const  char* strDumpPath, unsigned int uiLen, unsigned int uiDTS, unsigned char* pData);

#ifdef __cplusplus
}
#endif

#endif

