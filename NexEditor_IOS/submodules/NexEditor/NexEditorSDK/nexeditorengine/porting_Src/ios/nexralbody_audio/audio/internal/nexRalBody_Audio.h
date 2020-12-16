//
//  nexRalBody_Audio.h
//  nexRalBody
//
//  Created by Matthew Feinberg on 4/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NexRALBody_Types.h"
#import "NexTypeDef.h"
#import "NexMediaDef.h"

@interface NexRALBodyAudio : NexRALBody
@property (nonatomic) float volume;
@end

//unsigned int nexCALBody_Audio_GetProperty( unsigned int uProperty, unsigned int *puValue );

// Audio Renderer define
NXINT32 nexRALBody_Audio_GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *uUserData );
NXINT32 nexRALBody_Audio_SetProperty( NXUINT32 uProperty, NXINT64 uValue, NXVOID *uUserData );
NXINT32 nexRALBody_Audio_Init(	NEX_CODEC_TYPE uCodecObjectTypeIndication
							  , NXUINT32 uSamplingRate
							  , NXUINT32 uNumOfChannels
							  , NXUINT32 uBitPerSample
							  , NXUINT32 uNumOfSamplesPerChannel
							  , NXVOID **puRALUserData );
NXINT32 nexRALBody_Audio_Deinit( NXVOID *uRALUserData );
NXINT32 nexRALBody_Audio_GetEmptyBuffer( NXVOID** ppEmptyBuffer, NXINT32* nMaxBufferSize, NXVOID * uRALUserData );
NXINT32 nexRALBody_Audio_ConsumeBuffer( NXVOID* pBuffer, NXINT32 nBufferLen, NXUINT32 uCTS, NXINT32 isDecodeSuccess, NXBOOL bEndFrame, NXVOID *uRALUserData );
NXINT32 nexRALBody_Audio_SetBufferMute( NXVOID *pBuffer, NXBOOL bSetPCMSize, NXINT32* piWrittenPCMSize, NXVOID *uRALUserData );
NXINT32 nexRALBody_Audio_GetCurrentCTS( NXUINT32 *puCTS, NXVOID *uRALUserData );
NXINT32 nexRALBody_Audio_ClearBuffer( NXVOID *uRALUserData );
NXINT32 nexRALBody_Audio_Pause( NXVOID *uRALUserData );
NXINT32 nexRALBody_Audio_Resume( NXVOID *uRALUserData );		
NXINT32 nexRALBody_Audio_Flush( NXUINT32 uCTS, NXVOID *uRALUserData );
NXINT32 nexRALBody_Audio_SetTime( NXUINT32 uCTS, NXVOID *uRALUserData );
NXINT32 nexRALBody_Audio_PlaybackRate( NXINT32 iRate, NXVOID *uRALUserData );	
