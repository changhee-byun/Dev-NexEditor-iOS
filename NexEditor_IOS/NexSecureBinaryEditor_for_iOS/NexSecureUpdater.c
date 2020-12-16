/*
 *  NexSecureUpdater.c
 *  NexSecureBinaryEditor
 *
 *  Created by Matthew Feinberg on 8/16/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "NexSecureUpdater.h"
#include <string.h>

const int chunkLen = 22441;//2344; //808;
const char startStr[] = {0x54, 0x6b, 0x56, 0x59, 0x56, 0x46, 0x4a, 0x46, 0x51, 0x55, 0x31, 0x4a, 0x54, 0x6b, 0x63, 0x75, 0x51, 0x30, 0x39, 0x4e, 0x58, 0x31, 0x59, 0x7a, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41};//"TkVYVFJFQU1JTkcuQ09NAAAAAAAAAAAA";
const char endStr[] = {0x3d, 0x3d};//"==";

int NexSecureUpdater_getChunkLen( void ) {
	return chunkLen;
}

int NexSecureUpdater_isValidChunk( void *data ) {
	char *pStart = (char*)data;
	int check = 0;
	
	// Make sure it starts with the start-marker string
//	for( i=0; i<sizeof(startStr)-1; i++ )
	{
		if (memcmp(pStart, startStr, sizeof(startStr)) == 0)//pStart[i]!=startStr[i] )
		{
			check++;
//			break;
			//return 0;
		}
	}
	
	// Make sure it ends with the end-marker string
	
//	if(check)
//	{
////		for( i=0; i<sizeof(endStr)-1; i++ )
//		{
//			if( memcmp(pEnd, endStr, sizeof(endStr)) == 0)// pEnd[i]!=endStr[i] )
//			{
//				check++;
////				break;
//			}
////			return 0;
//		}
//	}
	// Check for null termination
//	if( pStart[chunkLen]!=0 )
//		return 0;

//	if(check == 2)
//		return 1;
//	else
//		return 0;
	return check;
}

char* NexSecureUpdater_getVersionInfo( const void *data, int datalen ) {
	const char searchstr[] = "$_NxVersionInfo100!";
	char *pData = (char*)data;
	int i,j;
	int bFound;
	for( i=0; i<datalen-(sizeof(searchstr)-1); i++ ) {
		bFound = 1;
		for( j=0; j<sizeof(searchstr)-1; j++ ) {
			if (pData[i+j]!=searchstr[j]) {
				bFound = 0;
				break;
			}
		}
		if( bFound )
			return pData+i;
	}
	return (char*)0;
}

char* NexSecureUpdater_getFirstChunk( const void *data, int datalen ) {
	char *pData = (char*)data;
	int i;

	for( i=0; i<=datalen; i++ ) {
		if( NexSecureUpdater_isValidChunk(pData+i) )
			return pData+i;
	}
	
	return (char*)0;
}

int NexSecureUpdater_countAllChunks( const void *data, int datalen ) {
	char *pData = (char*)data;
	int i;
	int nFound = 0;
	
	for( i=0; i<=datalen; i++ ) {
		if( NexSecureUpdater_isValidChunk(pData+i) )
			nFound++;
	}
	
	return nFound;
}

int NexSecureUpdater_updateAllChunks( void *data, int datalen, char* newChunkData ) {
	char *pData = (char*)data;
	int i;
	int nFound = 0;
	
//	if( strlen(newChunkData)!=chunkLen )
//		return 0;
	
	for( i=0; i<=datalen-chunkLen; i++ ) {
		if( NexSecureUpdater_isValidChunk(pData+i) ) {
			memcpy( pData+i, newChunkData, chunkLen );
			nFound++;
		}
	}
	
	return nFound;
}

