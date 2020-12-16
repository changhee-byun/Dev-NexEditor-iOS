/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_AudioGain.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
kyebeom song		2013/2/19	Draft.
-----------------------------------------------------------------------------*/

#include "NEXVIDEOEDITOR_AudioGain.h"

int Nx_MULSHIFTQ30(short x, int y)
{
	int z;
	z = (int)(((int)x * (long long)y)>> 30);
	return z;
}

unsigned int getGain(int nVolume)
{
	return VolumeToGain[nVolume];
}

unsigned int calcVolume(unsigned int uiCTS, unsigned int uiStartCTS, unsigned int uiEndCTS, unsigned int uiStartVolume, unsigned int uiEndVolume)
{
	float fDelta = 0.0f;
	unsigned int uiVolume = 0;
	if( uiEndVolume == uiStartVolume)
	{
		fDelta = 0;
	}
	else
	{
		int tmpCTSGap = uiEndCTS - uiStartCTS;
		fDelta = tmpCTSGap == 0?0:(float)((int)(uiEndVolume - uiStartVolume))/tmpCTSGap;
	}						

	// calc volume gain.
	if( uiStartCTS == uiEndCTS)
	{
		return uiStartVolume;
	}
	else
	{
		uiVolume = uiStartVolume + fDelta*(uiCTS -uiStartCTS);

		if( uiStartVolume < uiEndVolume)
		{
			uiVolume = uiVolume<uiEndVolume?uiVolume:uiEndVolume;
		}
		else
		{
			uiVolume = uiVolume>uiEndVolume?uiVolume:uiEndVolume;
		}
	}
	return uiVolume;
}

void processVolume(int iChannels, short* pPCM, int iPCMCount, int iStartVolume, int iEndVolume)
{
	if( iChannels == 1 )
	{
		gainFeedMono(pPCM, iPCMCount / 2, iStartVolume, iEndVolume);
	}
	else if( iChannels == 2 )
	{
		iPCMCount = iPCMCount / iChannels;
		gainFeedStereo(pPCM, iPCMCount / 2, iStartVolume, iEndVolume);		
	}
	else
	{
		iPCMCount = iPCMCount / iChannels;
		gainFeedMulti(pPCM, iChannels, iPCMCount / 2, iStartVolume, iEndVolume);
	}
}

void gainFeedMono(short *pnSignal0, int framesize,  int fromGain, int toGain)
{
	int		tmp;
	short	*pnSignals = pnSignal0;
	int		currGain= 0;
	int		gainInterval = 0;
	int		countInterval = 0;

	fromGain= fromGain< 0 ? 0 : fromGain;
	fromGain = fromGain > 200 ? 200 : fromGain;

	toGain = toGain < 0 ? 0 : toGain;
	toGain = toGain > 200 ? 200 : toGain;

	currGain = fromGain;
	if( fromGain != toGain && toGain + 1 != fromGain )
	{
		gainInterval = framesize/(toGain - fromGain + 1) ;
		gainInterval = gainInterval>0?gainInterval:(-gainInterval);
	}
	else
	{
		gainInterval = framesize;
	}
		
	
	int iRealGain = VolumeToGain[currGain];
	if( iRealGain == 1073741825 && gainInterval == framesize  ) return;

	do
	{
		tmp = (Nx_MULSHIFTQ30(*pnSignals, iRealGain));
		if(tmp > 32767)  
			*pnSignals = 32767;
		else if(tmp < -32768) 
			*pnSignals = -32768;
		else 
			*pnSignals = tmp;

		pnSignals++;

		countInterval++;
		if( countInterval >= gainInterval)
		{
			if(fromGain < toGain)
			{
				currGain++;
				currGain = currGain > toGain? toGain:currGain;
			}
			else
			{
				currGain--;
				currGain = currGain < toGain? toGain:currGain;
			}
			countInterval = 0;
			iRealGain = VolumeToGain[currGain];
		}

	}while( --framesize != 0);
}


void gainFeedStereo(short *pnSignal0, int framesize, int fromGain, int toGain)
{
	short	*pnSignals = pnSignal0;
	int		tmp;
	int		currGain= 0;
	int		gainInterval = 0;
	int		countInterval = 0;

	fromGain= fromGain< 0 ? 0 : fromGain;
	fromGain = fromGain > 200 ? 200 : fromGain;

	toGain = toGain < 0 ? 0 : toGain;
	toGain = toGain > 200 ? 200 : toGain;

	currGain = fromGain;
	if( fromGain != toGain && toGain + 1 != fromGain )
	{
		gainInterval = framesize/(toGain - fromGain + 1) ;
		gainInterval = gainInterval>0?gainInterval:(-gainInterval);
	}
	else
	{
		gainInterval = framesize;
	}

	int iRealGain = VolumeToGain[currGain];
	if( iRealGain == 1073741825 && gainInterval == framesize ) return;

	do
	{
		tmp = (Nx_MULSHIFTQ30(*pnSignals, iRealGain));
		if(tmp > 32767)  
			*pnSignals = 32767;
		else if(tmp < -32768) 
			*pnSignals = -32768;
		else 
			*pnSignals = tmp;

		pnSignals++;

		tmp = (Nx_MULSHIFTQ30(*pnSignals, iRealGain));
		if(tmp > 32767)  
			*pnSignals = 32767;
		else if(tmp < -32768) 
			*pnSignals = -32768;
		else 
			*pnSignals = tmp;
	
		pnSignals++;

		countInterval++;
		if( countInterval >= gainInterval)
		{
			if(fromGain < toGain)
			{
				currGain++;
				currGain = currGain > toGain? toGain:currGain;
			}
			else
			{
				currGain--;
				currGain = currGain < toGain? toGain:currGain;
			}

			countInterval = 0;
			iRealGain = VolumeToGain[currGain];
		}		

	}while( --framesize != 0);
}

void gainFeedMulti(short *pnSignal0, int nChannels, int framesize, int fromGain, int toGain)		// gain Range 0 ~ 200
{
	int		tmp;
	short	*pnSignals = pnSignal0;
	int		currGain = 0;
	int		gainInterval = 0;
	int		countInterval = 0;

	fromGain = fromGain< 0 ? 0 : fromGain;
	fromGain = fromGain > 200 ? 200 : fromGain;

	toGain = toGain < 0 ? 0 : toGain;
	toGain = toGain > 200 ? 200 : toGain;

	currGain = fromGain;
	if (fromGain != toGain && toGain + 1 != fromGain)
	{
		gainInterval = framesize / (toGain - fromGain + 1);
		gainInterval = gainInterval>0 ? gainInterval : (-gainInterval);
	}
	else
	{
		gainInterval = framesize;
	}


	int iRealGain = VolumeToGain[currGain];
	if (iRealGain == 1073741825 && gainInterval == framesize) return;

	do
	{
		for (int i = 0; i < nChannels; i++)
		{
			tmp = (Nx_MULSHIFTQ30(*pnSignals, iRealGain));
			if (tmp > 32767)
				*pnSignals = 32767;
			else if (tmp < -32768)
				*pnSignals = -32768;
			else
				*pnSignals = tmp;

			pnSignals++;
		}

		countInterval++;
		if (countInterval >= gainInterval)
		{
			if (fromGain < toGain)
			{
				currGain++;
				currGain = currGain > toGain ? toGain : currGain;
			}
			else
			{
				currGain--;
				currGain = currGain < toGain ? toGain : currGain;
			}
			countInterval = 0;
			iRealGain = VolumeToGain[currGain];
		}

	} while (--framesize != 0);
}