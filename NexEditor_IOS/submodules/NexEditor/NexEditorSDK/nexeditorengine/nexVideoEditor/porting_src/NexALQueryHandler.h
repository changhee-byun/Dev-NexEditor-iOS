#ifndef __CALQUERYHANDLER_H__
#define __CALQUERYHANDLER_H__

int CalQueryHandlerForHW(	int nCommand
								, NEXCALMediaType eMediaType
								, NEXCALMode eMode
								, unsigned int uCodecObjectTypeIndication
								, unsigned int uUserData
								, unsigned char* pConfig, int nConfigLen
								, int nReserved1, int nReserved2 );


typedef struct _CALQUERY_USERDATA
{
 	char m_strModelName[255];
	int	m_iPlatformInfo;
}CALQUERY_USERDATA;

#endif

