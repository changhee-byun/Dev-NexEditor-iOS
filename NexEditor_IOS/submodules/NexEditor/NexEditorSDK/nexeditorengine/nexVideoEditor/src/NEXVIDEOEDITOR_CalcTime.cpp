/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_CalcTime.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
case		2015/07/20	Draft.
-----------------------------------------------------------------------------*/

#include "stdlib.h"
#include "NexSAL_Internal.h"
#include "NEXVIDEOEDITOR_CalcTime.h"

CCalcTime::CCalcTime()
{
}

CCalcTime::~CCalcTime()
{
}

unsigned int CCalcTime::applySpeed(int iSpeedFactor, unsigned int uiTime)
{
	unsigned int uiReturnTime = 0;
	if( iSpeedFactor != 100 )
	{
		//int iSpeed = (iSpeedFactor==13? 25: iSpeedFactor);
		int iSpeed = iSpeedFactor;

		if( iSpeedFactor == 3 || iSpeedFactor == 6 || iSpeedFactor == 13 )
		{
			iSpeed = 25;
		}
		uiReturnTime = uiTime * 100 / iSpeed;

		if(iSpeedFactor == 13)
		{
			uiReturnTime *= 2;
		}
		else if( iSpeedFactor == 6 )
		{
			uiReturnTime *= 4;
		}
		else if( iSpeedFactor == 3 )
		{
			uiReturnTime *= 8;
		}
	}
	else
	{
		uiReturnTime = uiTime;
	}
	return uiReturnTime;
}

unsigned int CCalcTime::applySpeed(unsigned int uiBaseTime, unsigned int uiStartTrim, int iSpeedFactor, unsigned int uiTime)
{
	unsigned int uiReturnTime = 0;
	if( iSpeedFactor != 100 )
	{
		//int iSpeed = (iSpeedFactor==13? 25: iSpeedFactor);
		int iSpeed = iSpeedFactor;

		if( iSpeedFactor == 3 || iSpeedFactor == 6 || iSpeedFactor == 13 )
		{
			iSpeed = 25;
		}

		// for LG Harmony 601 issue on directexport feature.
		int gap = int(uiTime - uiStartTrim);

		if( gap < 0 )
		{
			if( gap > -5 )
			gap = 0;
		}

		uiReturnTime = gap * 100 / iSpeed;

		if(iSpeedFactor == 13)
		{
			uiReturnTime *= 2;
		}
		else if(iSpeedFactor == 6 )
		{
			uiReturnTime *= 4;
		}
		else if(iSpeedFactor == 3 )
		{
			uiReturnTime *= 8;
		}
		uiReturnTime += uiBaseTime;
	}
	else
	{
		uiReturnTime = uiTime + uiBaseTime;
		if( uiReturnTime >= uiStartTrim )
		{
			uiReturnTime -= uiStartTrim;
		}
		else
		{
			if( (uiStartTrim - uiReturnTime) < 5 )
			{
				uiReturnTime = 0;
			}
			else
			{
				uiReturnTime -= uiStartTrim;
			}
			// uiStartTrim = uiStartTrim < 5 ? 0 : uiStartTrim;
			// uiReturnTime -= uiStartTrim;
		}
	}
	return uiReturnTime;
}

unsigned int CCalcTime::restoreSpeed(int iSpeedFactor, unsigned int uiTime)
{
	unsigned int uiReturnTime = uiTime;
	
	//int iSpeed = (iSpeedFactor==13? 25: iSpeedFactor);
	int iSpeed = iSpeedFactor;

	if( iSpeedFactor == 3 || iSpeedFactor == 6 || iSpeedFactor == 13 )
	{
		iSpeed = 25;
	}

	if(iSpeedFactor == 13)
	{
		uiReturnTime = uiTime / 2;
	}
	else if( iSpeedFactor == 6 )
	{
		uiReturnTime = uiTime / 4;
	}
	else if( iSpeedFactor == 3 )
	{
		uiReturnTime = uiTime / 8;
	}
	uiReturnTime = (uiReturnTime * iSpeed / 100);

	return uiReturnTime;
}

unsigned int CCalcTime::restoreSpeed(unsigned int uiStartTrim, int iSpeedFactor, unsigned int uiTime)
{
	unsigned int uiReturnTime = uiTime;
	
	//int iSpeed = (iSpeedFactor==13? 25: iSpeedFactor);
	int iSpeed = iSpeedFactor;
	if( iSpeedFactor == 6 || iSpeedFactor == 13 )
	{
		iSpeed = 25;
	}
	if(iSpeedFactor == 13)
	{
		uiReturnTime = uiTime / 2;
	}
	else if( iSpeedFactor == 6  )
	{
		uiReturnTime = uiTime / 4;
	}

	uiReturnTime = (uiReturnTime * iSpeed / 100) + uiStartTrim;
	return uiReturnTime;
}

unsigned int CCalcTime::restoreSpeed(unsigned int uiBaseTime, unsigned int uiStartTrim, int iSpeedFactor, unsigned int uiTime)
{
	unsigned int uiReturnTime = 0;
	
	if( iSpeedFactor != 100 )
	{
		//int iSpeed = (iSpeedFactor==13? 25: iSpeedFactor);
		int iSpeed = iSpeedFactor;
		if( iSpeedFactor == 3 || iSpeedFactor == 6 || iSpeedFactor == 13 )
		{
			iSpeed = 25;
		}


		// for LG Harmony 601 issue on directexport feature.
		int gap = int(uiTime - uiBaseTime);

		if( gap < 0 )
		{
			if( gap > -5 )
				gap = 0;
		}

		uiReturnTime = gap * iSpeed /100;

		if(iSpeedFactor == 13)
		{
			uiReturnTime /= 2;
		}
		else if( iSpeedFactor == 6 )
		{
			uiReturnTime /= 4;
		}
		else if( iSpeedFactor == 3 )
		{
			uiReturnTime /= 8;
		}
		uiReturnTime += uiStartTrim;
	}
	else
	{
		uiReturnTime = uiTime - uiBaseTime + uiStartTrim;	
	}
    
	return uiReturnTime;
}

