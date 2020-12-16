/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_DrawInfoVec.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
case		2017/05/15	Draft.
-----------------------------------------------------------------------------*/

#include "NEXVIDEOEDITOR_DrawInfoVec.h"
#include "NexThemeRenderer.h"

CNexDrawInfoVec::CNexDrawInfoVec()
{
	mDrawVec.clear();
}

CNexDrawInfoVec::~CNexDrawInfoVec()
{
	clearDrawInfo();
}

IDrawInfo* CNexDrawInfoVec::createDrawInfo()
{
	CNexDrawInfo* pInfo = new CNexDrawInfo();
	return (IDrawInfo*)pInfo;
}

void CNexDrawInfoVec::addDrawInfo(IDrawInfo* info)
{
	SAFE_ADDREF(info);

	mDrawVec.insert(mDrawVec.end(), (CNexDrawInfo*)info);
}

void CNexDrawInfoVec::clearDrawInfo()
{
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		mDrawVec[i]->Release();
	}
	mDrawVec.clear();
}

int CNexDrawInfoVec::sizeDrawInfos()
{
	return (int)mDrawVec.size();
}

void CNexDrawInfoVec::applyDrawInfo(void* pRender, unsigned int uiTime)
{
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		mDrawVec[i]->applyDrawInfo(pRender, uiTime);
	}
}

void CNexDrawInfoVec::applyDrawInfo(void* pRender, unsigned int uiTime, int textureID)
{
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		mDrawVec[i]->applyDrawInfo(pRender, uiTime, textureID);
	}
}

void CNexDrawInfoVec::resetDrawInfos(void* pRender)
{
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		mDrawVec[i]->resetDrawInfo(pRender);
	}
}

CNexDrawInfoVec* CNexDrawInfoVec::getDrawInfoList(unsigned int trackID)
{
	CNexDrawInfoVec* newList = new CNexDrawInfoVec();

	for(int i = 0; i < mDrawVec.size(); i++ )
	{
		if( mDrawVec[i]->mTrackID == trackID )
		{
			newList->addDrawInfo(mDrawVec[i]);
		}
	}

	if( newList->sizeDrawInfos() <= 0 )
		SAFE_RELEASE(newList);
	return newList;
}

void CNexDrawInfoVec::updateDrawInfo() {
	int idx = 0;
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		if( mDrawVec[i]->mEffectType == 0 )
		{
			mDrawVec[i]->mEffectIndex = idx;
			idx++;
		}
		else if( mDrawVec[i]->mEffectType == 1 )
		{
			mDrawVec[i]->mEffectIndex = idx;
		}
	}

	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		if( mDrawVec[i]->mEffectType == 0 )
		{
			mDrawVec[i]->mTotalMasterEffect = idx;
		}
	}
}

void CNexDrawInfoVec::updateDrawInfo(CNexDrawInfo* pInfo)
{
	if( pInfo == NULL ) return;
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		if (mDrawVec[i]->mID == pInfo->mID && mDrawVec[i]->mTrackID == pInfo->mTrackID)
		{
			mDrawVec[i]->setTitle(pInfo->mTitle);
			mDrawVec[i]->mLUT = pInfo->mLUT;
			mDrawVec[i]->mCustomLUT_A = pInfo->mCustomLUT_A;
			mDrawVec[i]->mCustomLUT_B = pInfo->mCustomLUT_B;
			mDrawVec[i]->mCustomLUT_Power = pInfo->mCustomLUT_Power;

			mDrawVec[i]->mRotateState = pInfo->mRotateState;

			mDrawVec[i]->mUserRotateState = pInfo->mUserRotateState;

			mDrawVec[i]->mTranslateX = pInfo->mTranslateX;
			mDrawVec[i]->mTranslateY = pInfo->mTranslateY;

			mDrawVec[i]->mBrightness = pInfo->mBrightness;
			mDrawVec[i]->mSaturation = pInfo->mSaturation;
            mDrawVec[i]->mHue = pInfo->mHue;
			mDrawVec[i]->mContrast = pInfo->mContrast;
			mDrawVec[i]->mTintColor = pInfo->mTintColor;

			mDrawVec[i]->setStartRect(pInfo->mStart.getLeft(), pInfo->mStart.getTop(), pInfo->mStart.getRight(), pInfo->mStart.getBottom());
			mDrawVec[i]->setEndRect(pInfo->mEnd.getLeft(), pInfo->mEnd.getTop(), pInfo->mEnd.getRight(), pInfo->mEnd.getBottom());
			mDrawVec[i]->setFaceRect(pInfo->mFace.getLeft(), pInfo->mFace.getTop(), pInfo->mFace.getRight(), pInfo->mFace.getBottom());
		}
	}
}

CNexDrawInfo* CNexDrawInfoVec::getDrawInfo(int id)
{
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		if( mDrawVec[i]->mID == id )
		{
			SAFE_ADDREF(mDrawVec[i])
			return mDrawVec[i];
		}
	}
	return NULL;
}

NXBOOL CNexDrawInfoVec::isDrawTime(unsigned int uiTime)
{
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		if( mDrawVec[i]->mStartTime <= uiTime && uiTime < mDrawVec[i]->mEndTime )
		{
			return TRUE;
		}
	}
	return FALSE;
}

NXBOOL CNexDrawInfoVec::checkFaceDetectDoneAtTime(unsigned int uiTime)
{
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		if( mDrawVec[i]->mStartTime <= uiTime && uiTime < mDrawVec[i]->mEndTime )
		{
			if( mDrawVec[i]->mFace.getWidth() <= 0 )
				return FALSE;
		}
	}
	return TRUE;
}

void CNexDrawInfoVec::printDrawInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfoVec.cpp %d] ---------------- printDrawInfo ----------------", __LINE__);
	for( int i = 0; i < mDrawVec.size(); i++ )
	{
		mDrawVec[i]->printDrawInfo();
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfoVec.cpp %d] ---------------- printDrawInfo ----------------", __LINE__);
}
