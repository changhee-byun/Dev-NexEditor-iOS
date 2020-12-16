/******************************************************************************
* File Name   :	nexSal_trace.c
* Description :	source file
*******************************************************************************

NexStreaming Confidential Proprietary
Copyright (C) 2008 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#include "NexSAL_API.h"
#include "NexSAL_Internal.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define NEXSAL_MEMDUMP_MAXBYTES_PER_LINE 16
#define _DBGPRTF NEXSAL_TRACE

#ifdef WINCE
	#define myvsnprintf _vsnprintf
#else
	#define myvsnprintf vsnprintf
#endif

NEXSAL_API NXVOID nexSAL_MemDump(NXVOID *a_pSrc, NXINT32 a_nSize)
{
	NXINT32 i, j, nLineCnt;
	NXUINT8 *pAddress = (NXUINT8*)a_pSrc;
	NXUINT8 uc;
	NXUINT8 pLineBuffer[NEXSAL_MEMDUMP_MAXBYTES_PER_LINE*3+2];
	NXUINT8 *pBuffer = (NXUINT8*)a_pSrc;

	_DBGPRTF("[nexSAL] Memory Dump - Addr : 0x%08X, Size : %d\n", pBuffer, a_nSize);
	_DBGPRTF("---------------------------------------------------------\n");

	for (i = 0, j = 0, nLineCnt = 0; i < a_nSize; i++)
	{
		if ((i % NEXSAL_MEMDUMP_MAXBYTES_PER_LINE) == 0)
		{
			if (nLineCnt != 0)
			{
				pLineBuffer[j] = '\0';
				_DBGPRTF("0x%08X  %s\n", pAddress, pLineBuffer);
			}

			// address
			pAddress = (NXUINT8*)a_pSrc + i;
			j =0;

			nLineCnt++;
		}
		uc = (*(pBuffer+i)) >> 4;
		if (uc < 10)
		{
			pLineBuffer[j] = uc+'0';
		}
		else
		{
			pLineBuffer[j] = uc-10+'A';
		}

		j++;

		uc = (*(pBuffer+i)) & 0x0F;
		if (uc < 10)
		{
			pLineBuffer[j] = uc+'0';
		}
		else
		{
			pLineBuffer[j] = uc-10+'A';
		}

		j++; pLineBuffer[j++] = ' ';

	}

	pLineBuffer[j] = '\0';
	_DBGPRTF("0x%08X  %s\n", pAddress, pLineBuffer);
	_DBGPRTF("---------------------------------------------------------\n");
}


static NXINT32 g_iTCLevel[NEXSAL_MAX_TRACE_CATEGORY];
static NXCHAR g_aszTCPrefix[NEXSAL_MAX_TRACE_CATEGORY][NEXSAL_MAX_TRACE_PREFIX+1] = { { 0, },};

NEXSAL_API NXVOID nexSAL_TraceSetPrefix(NXINT32 a_nCategory, const NXCHAR *a_szPrefix)
{
	NXINT32 i;

	if (a_nCategory >= NEXSAL_MAX_TRACE_CATEGORY || a_nCategory < 0)
	{
		return;
	}

	for (i = 0 ; i < NEXSAL_MAX_TRACE_PREFIX ; i++)
	{
		if (0x00 == (g_aszTCPrefix[a_nCategory][i] = a_szPrefix[i]))
		{
			break;
		}
	}

	g_aszTCPrefix[a_nCategory][NEXSAL_MAX_TRACE_PREFIX] = 0x00;
}

NEXSAL_API NXBOOL nexSAL_TraceCondition(NXINT32 a_nCategory, NXINT32 a_nLevel)
{
	if (g_iTCLevel[a_nCategory] < 0)
	{
		return FALSE;
	}

	if (g_iTCLevel[a_nCategory] < a_nLevel)
	{
		return FALSE;
	}

	return TRUE;
}

NEXSAL_API NXVOID nexSAL_TraceSetCondition(NXINT32 a_nCategory, NXINT32 a_nLevel)
{
	if (a_nCategory >= NEXSAL_MAX_TRACE_CATEGORY)
	{
		//NEXSAL_TRACE("[NEXDEBUG] Module Setting Error. Module : %d\r\n", iModule);
		return;
	}

	if (a_nLevel < 0)
	{
		g_iTCLevel[a_nCategory] = -1;
	}
	else if (a_nLevel < NEXSAL_MAX_TRACE_LEVEL)
	{
		g_iTCLevel[a_nCategory] = a_nLevel;
	}
}

NEXSAL_API NXVOID nexSAL_TraceGetCondition(NXINT32 a_nCategory, NXINT32 *a_pnLevel)
{
	*a_pnLevel = g_iTCLevel[a_nCategory];
}

#define tmin(a,b) ((a)>(b) ? (b):(a))

NEXSAL_API NXVOID nexSAL_TraceCat(NXINT32 a_nCategory, NXINT32 a_nLevel, const NXCHAR *a_strFormat, ...)
{
	NXINT32 iLen = 0;
	va_list va;
	NXCHAR sBuf[NEXSAL_MAX_DEBUG_STRING_LENGTH];

	if (FALSE == nexSAL_TraceCondition(a_nCategory, a_nLevel))
		return;

	sBuf[NEXSAL_MAX_TRACE_PREFIX] = sBuf[NEXSAL_MAX_DEBUG_STRING_LENGTH-1] = '\0';

#ifdef WIN32//Nx_robin__150723 Add "Thread ID" and "Tick"
	sprintf(sBuf, "[0x%p][%u] ", nexSAL_TaskCurrent(), nexSAL_GetTickCount());
	iLen = (NXINT32)strlen(sBuf);
#endif
	strncpy(sBuf+iLen, &g_aszTCPrefix[a_nCategory][0], NEXSAL_MAX_TRACE_PREFIX);

	iLen += (NXINT32)tmin(strlen(&g_aszTCPrefix[a_nCategory][0]),NEXSAL_MAX_TRACE_PREFIX);
	sBuf[iLen++] = '0'+(a_nLevel%10);
	sBuf[iLen++] = ':';
	sBuf[iLen] = '\0';

	va_start(va, a_strFormat);
	myvsnprintf(sBuf+iLen, NEXSAL_MAX_DEBUG_STRING_LENGTH-1-iLen, a_strFormat, va);
	va_end(va);

	nexSAL_DebugOutputString(sBuf);
}

NEXSAL_API NXVOID nexSAL_TraceCat2(NEXSALHandle hSAL, NXINT32 a_nCategory, NXINT32 a_nLevel, const NXCHAR *a_strFormat, ...)
{
	NXINT32 iLen = 0;
	va_list va;
	NXCHAR sBuf[NEXSAL_MAX_DEBUG_STRING_LENGTH] = {'\0',};
	NXCHAR strTemp[20];
	NXUINT32 uTempLen = 0;

	if (FALSE == nexSAL_TraceCondition(a_nCategory, a_nLevel))
		return;

	sprintf(strTemp, "[%"PRIuS"]", nexSAL_GetUID(hSAL));
	uTempLen = (NXUINT32)strlen(strTemp);
	strncpy(sBuf, strTemp, uTempLen);

	strncpy(sBuf+uTempLen, &g_aszTCPrefix[a_nCategory][0], NEXSAL_MAX_TRACE_PREFIX);

	iLen = uTempLen + tmin(strlen(&g_aszTCPrefix[a_nCategory][0]),NEXSAL_MAX_TRACE_PREFIX);
	sBuf[iLen++] = '0'+(a_nLevel%10);
	sBuf[iLen++] = ':';
	sBuf[iLen] = '\0';

	va_start(va, a_strFormat);
	myvsnprintf(sBuf+iLen, NEXSAL_MAX_DEBUG_STRING_LENGTH-1-iLen, a_strFormat, va);
	va_end(va);

	nexSAL_DebugOutputString(sBuf);
}


/*-----------------------------------------------------------------------------
Revision History:
Author		Date			Version		Description of Changes
-------------------------------------------------------------------------------
ysh			2008/07/25		2.6.0		First version.
-----------------------------------------------------------------------------*/

