/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_DrawInfo.cpp
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

#include "NEXVIDEOEDITOR_DrawInfo.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NexThemeRenderer.h"

CNexDrawInfo::CNexDrawInfo()
{
	mActive			= FALSE;

	mID				= -1;
	mTrackID		= -1;
	mSubEffectID	= -1;

	mEffectIndex	= 0;
	mTotalMasterEffect	= 0;

	mEffectID		= NULL;
	mEffectUID		= 0;
	mEffectType		= 0;
	mTitle			= NULL;

	mStartTime		= 0;
	mEndTime		= 0;

	mRotateState	= 0;
	mUserRotateState = 0;
	mLUT			= 0;
	mCustomLUT_A 	= 0;
	mCustomLUT_B 	= 0;
	mCustomLUT_Power= 0;

	mBrightness		= 0;
	mSaturation		= 0;
    mHue            = 0;
	mContrast		= 0;
	mTintColor		= 0;

}

CNexDrawInfo::~CNexDrawInfo()
{
	if( mEffectID != NULL ) {
		nexSAL_MemFree(mEffectID);
		mEffectID = NULL;
	}

	if( mTitle != NULL )
	{
		nexSAL_MemFree(mTitle);
		mTitle = NULL;
	}
}

void CNexDrawInfo::setDrawInfo(unsigned int id,  unsigned int trackid, unsigned int subeffectid, unsigned int isTransition, unsigned int starttime, unsigned int endtime, int rotate, int user_rotate, int translate_x, int translate_y, int lut, int custom_lut_a, int custom_lut_b, int custom_lut_power)
{
	mID				= id;
	mTrackID		= trackid;
	mSubEffectID	= subeffectid;

	mEffectType		= isTransition;

	mStartTime		= starttime;
	mEndTime		= endtime;

	mRotateState	= rotate;
	mUserRotateState = user_rotate;
	mTranslateX 	= translate_x;
	mTranslateY 	= translate_y;
	mLUT			= lut;
	mCustomLUT_A    = custom_lut_a;
	mCustomLUT_B    = custom_lut_b;
	mCustomLUT_Power= custom_lut_power;
}

void CNexDrawInfo::setEffect(const char* effect)
{
	if( mEffectID != NULL )
	{
		nexSAL_MemFree(mEffectID);
		mEffectID = NULL;
	}

	if( effect != NULL )
	{
		mEffectID	= (char*)nexSAL_MemAlloc(strlen(effect) + 1);
		strcpy(mEffectID, effect);
	}
}

void CNexDrawInfo::setTitle(const char* title)
{
	if( mTitle != NULL )
	{
		nexSAL_MemFree(mTitle);
		mTitle = NULL;
	}

	if( title != NULL )
	{
		mTitle	= (char*)nexSAL_MemAlloc(strlen(title) + 1);
		strcpy(mTitle, title);
	}
}

void CNexDrawInfo::setStartRect(int left, int top, int right, int bottom)
{
	mStart.setRect(left, top, right, bottom);
}

void CNexDrawInfo::setEndRect(int left, int top, int right, int bottom)
{
	mEnd.setRect(left, top, right, bottom);
}

void CNexDrawInfo::setFaceRect(int left, int top, int right, int bottom)
{
	mFace.setRect(left, top, right, bottom);
}

void CNexDrawInfo::setColor(int brightness, int saturation, int contrast, int tintcolor, int hue)
{
	mBrightness		= brightness;
	mSaturation		= saturation;
    mHue            = hue;
	mContrast		= contrast;
	mTintColor		= tintcolor;
}

void CNexDrawInfo::applyDrawInfo(void* pRender, unsigned int uiTime)
{
	if( uiTime < mStartTime || uiTime > mEndTime )
	{
		if( uiTime > mEndTime && mActive )
		{
			if( mEffectUID > 0 ) {
				NXT_ThemeRenderer_EndEDL((NXT_HThemeRenderer)pRender, mEffectUID);
				mEffectUID = 0;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] TM30 clear effect:%s", __LINE__, mEffectID);
			mActive = FALSE;
		}
		return;
	}

	if( mActive == FALSE)
	{
		if(mEffectType == TITLE_EFFECT){

			mEffectUID = NXT_ThemeRenderer_StartTitleEDL((NXT_HThemeRenderer)pRender, mEffectID, mTitle, mSubEffectID, mEffectIndex, mTotalMasterEffect, mStartTime, mEndTime);
		}
		else{
			mEffectUID = NXT_ThemeRenderer_StartTransitionEDL((NXT_HThemeRenderer)pRender, mEffectID, NULL, mEffectIndex, mTotalMasterEffect, mStartTime, mEndTime);
		}
		mActive = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] TM30 set effect:%s ID:%d trackID:%d", __LINE__, mEffectID, mID, mTrackID);
	}

	NXT_ThemeRenderer_SetEffectOptions((NXT_HThemeRenderer)pRender, mEffectUID, mTitle);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[DrawInfo.cpp %d] TM30 set optiopn:%s ID:%d trackID:%d", __LINE__, mTitle, mID, mTrackID);
}

void CNexDrawInfo::applyDrawInfo(void* pRender, unsigned int uiTime, int textureID)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[DrawInfo.cpp %d] TID(%d) applyDrawInfo uiTime:%d mStartTime:%d mEndTime:%d mLUT:%d", __LINE__, textureID, uiTime, mStartTime, mEndTime, mLUT);

	if( uiTime < mStartTime || uiTime > mEndTime )
	{
		if( uiTime > mEndTime && mActive )
		{
			// call clear effect;
			NXT_ThemeRenderer_ReleaseDecoTexture((NXT_HThemeRenderer)pRender, mSubEffectID, 1);
			mActive = FALSE;
		}
		return;
	}

	if( mActive == FALSE)
	{
		// get deco target
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] TM30 uiTime:%d mStartTime:%d mEndTime:%d mLUT:%d", __LINE__, uiTime, mStartTime, mEndTime, mLUT);
		NXT_ThemeRenderer_CreateDecoTexture((NXT_HThemeRenderer)pRender, mSubEffectID);
		NXT_ThemeRenderer_LinkToBaseTrack((NXT_HThemeRenderer)pRender, mSubEffectID, mTrackID);
		mActive = TRUE;
	}

	int deco_texture_id = NXT_ThemeRenderer_GetDecoTexture((NXT_HThemeRenderer )pRender, mSubEffectID);

	applyPosition(pRender, uiTime, deco_texture_id);
	NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pRender, (NXT_TextureID)deco_texture_id, (unsigned int)mRotateState);
	NXT_ThemeRenderer_SetUserRotation((NXT_HThemeRenderer )pRender, (NXT_TextureID)deco_texture_id, 360 - (unsigned int)mUserRotateState);
	NXT_ThemeRenderer_SetTranslation((NXT_HThemeRenderer )pRender, (NXT_TextureID)deco_texture_id, mTranslateX, mTranslateY);
	NXT_ThemeRenderer_SetLUT((NXT_HThemeRenderer)pRender, mSubEffectID, mLUT);
	NXT_ThemeRenderer_SetCustomLUTA((NXT_HThemeRenderer)pRender, mSubEffectID, mCustomLUT_A);
	NXT_ThemeRenderer_SetCustomLUTB((NXT_HThemeRenderer)pRender, mSubEffectID, mCustomLUT_B);
	NXT_ThemeRenderer_SetCustomLUTPower((NXT_HThemeRenderer)pRender, mSubEffectID, mCustomLUT_Power);
	NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pRender, (NXT_TextureID)deco_texture_id, mBrightness, mContrast, mSaturation, mTintColor, mHue, 0);
	NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pRender, (NXT_TextureID)deco_texture_id, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[DrawInfo.cpp %d] TM30 uiTime:%d mStartTime:%d mEndTime:%d mLUT:%d", __LINE__, uiTime, mStartTime, mEndTime, mLUT);
}

void CNexDrawInfo::resetDrawInfo(void* pRender)
{
	if( mActive )
	{
		NXT_ThemeRenderer_ReleaseDecoTexture((NXT_HThemeRenderer)pRender, mSubEffectID, 1);

		if(mEffectUID > 0) {
			NXT_ThemeRenderer_EndEDL((NXT_HThemeRenderer)pRender, mEffectUID);
			mEffectUID = 0;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] TM30 clear effect:%s by resetDrawInfo", __LINE__, mEffectID);
		mActive = FALSE;
	}
}

void CNexDrawInfo::applyPosition(void* pRender, unsigned int uiTime, int textureID)
{
	unsigned int uiDur = mEndTime - mStartTime;
	float fPer = (float)(uiTime - mStartTime) / uiDur;

	float fleft 			= calcPos((float)mStart.getLeft(), (float)mEnd.getLeft(), fPer);
	float fright 			= calcPos((float)mStart.getRight(), (float)mEnd.getRight(), fPer);

	float ftop				= calcPos((float)mStart.getTop(), (float)mEnd.getTop(), fPer);
	float fbottom			= calcPos((float)mStart.getBottom(), (float)mEnd.getBottom(), fPer);

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] ID(%d) Pos(S(%d %d %d %d) E(%d %d %d %d))", __LINE__, muiTrackID, mStart.left, mStart.top, mStart.right, mStart.bottom, mEnd.left, mEnd.top, mEnd.right, mEnd.bottom);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] ID(%d) TextureID(%d) SetTextureInput Rect(%.2f %.2f %.2f %.2f) for video frame fPer(%f)", __LINE__, mTrackID, textureID, fleft, ftop, fright, fbottom, fPer);
	NXT_ThemeRenderer_SetTextureInputRect((NXT_HThemeRenderer )pRender, (NXT_TextureID)textureID, fleft, ftop, fright,  fbottom);
}

float CNexDrawInfo::calcPos(float fPos1, float fPos2, float fPer)
{
	float fNewPos = 0;
	if( fPos1 < fPos2 )
	{
		fNewPos = fPos1 + ((fPos2 - fPos1) * fPer);
	}
	else
	{
		fNewPos = fPos1 - ((fPos1 - fPos2) * fPer);
	}
	return fNewPos;
}

void CNexDrawInfo::printDrawInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] -------------------------------------------", __LINE__);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mID : %d", __LINE__, mID);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mTrackID : %d", __LINE__, mTrackID);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mSubEffectID : %d", __LINE__, mSubEffectID);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mActive : %d", __LINE__, mActive);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mEffectID : %s", __LINE__, mEffectID);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mTitle : %s", __LINE__, mTitle == NULL ? "" : mTitle);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mEffectType : %d", __LINE__, mEffectType);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mEffectIndex : %d", __LINE__, mEffectIndex);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mTotalMasterEffect : %d", __LINE__, mTotalMasterEffect);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mStartTime : %d", __LINE__, mStartTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mEndTime : %d", __LINE__, mEndTime);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mUserRotateState : %d", __LINE__, mUserRotateState);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mRotateState : %d", __LINE__, mRotateState);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mLUT(%d %d %d %d)", __LINE__, mLUT, mCustomLUT_A, mCustomLUT_B, mCustomLUT_Power);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mStart(%d %d %d %d)", __LINE__, mStart.getLeft(), mStart.getTop(), mStart.getRight(), mStart.getBottom());
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mEnd(%d %d %d %d)", __LINE__, mEnd.getLeft(), mEnd.getTop(), mEnd.getRight(), mEnd.getBottom());
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] mFace(%d %d %d %d)", __LINE__, mFace.getLeft(), mFace.getTop(), mFace.getRight(), mFace.getBottom());

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DrawInfo.cpp %d] -------------------------------------------", __LINE__);
}
