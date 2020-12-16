/*
 *  NexSecureUpdater.h
 *  NexSecureBinaryEditor
 *
 *  Created by Matthew Feinberg on 8/16/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

//#include "NexSecureWare.h"

int NexSecureUpdater_getChunkLen( void );													// Returns length of a standard chunk
int NexSecureUpdater_isValidChunk( void *data );											// Returns non-zero for a valid chunk; zero for invalid
char* NexSecureUpdater_getFirstChunk( const void *data, int datalen );							// Returns string pointer to beginning of first chunk
int NexSecureUpdater_updateAllChunks( void *data, int datalen, char* newChunkData );		// Returns number updated; zero if error or none found
int NexSecureUpdater_countAllChunks( const void *data, int datalen );								// Returns number of chunks
char* NexSecureUpdater_getVersionInfo( const void *data, int datalen );