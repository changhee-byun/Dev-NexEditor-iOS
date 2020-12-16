/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_DrawInfo.h
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

#ifndef __NEXVIDEOEDITOR_DRAWINFO_H__
#define __NEXVIDEOEDITOR_DRAWINFO_H__

#include "NexSAL_Internal.h"
#include "nexIRef.h"
#include "nexDef.h"
#include "nexEvent.h"
#include "nexLock.h"
#include <vector>

#include "NEXVIDEOEDITOR_Interface.h"
#include "NEXVIDEOEDITOR_Rect.h"
#include "NEXVIDEOEDITOR_Types.h"

enum{

	TITLE_EFFECT = 0,
	TRANSITION_EFFECT = 1,
};

class CNexDrawInfo: public CNxRef<IDrawInfo>
{
public:
	CNexDrawInfo();
	virtual ~CNexDrawInfo();

	virtual void setDrawInfo(unsigned int id,  unsigned int trackid, unsigned int subeffectid, unsigned int isTransition, unsigned int starttime, unsigned int endtime, int rotate, int user_rotate, int translate_x, int translate_y, int lut, int custom_lut_a, int custom_lut_b, int custom_lut_power);

	virtual void setEffect(const char* effect);
	virtual void setTitle(const char* title);
	virtual void setStartRect(int left, int top, int right, int bottom);
	virtual void setEndRect(int left, int top, int right, int bottom);
	virtual void setFaceRect(int left, int top, int right, int bottom);
	virtual void setColor(int brightness, int saturation, int contrast, int tintcolor, int hue);


	void applyDrawInfo(void* pRender, unsigned int uiTime);
	void applyDrawInfo(void* pRender, unsigned int uiTime, int textureID);
	void resetDrawInfo(void* pRender);

	void applyPosition(void* pRender, unsigned int uiTime, int textureID);
	float calcPos(float fPos1, float fPos2, float fPer);

	void printDrawInfo();

	NXBOOL					mActive;

	unsigned int			mID;
	unsigned int			mTrackID;
	unsigned int			mSubEffectID;

	int 					mEffectIndex;
	int 					mTotalMasterEffect;

	char*					mEffectID;
	unsigned 				mEffectUID;
	int 					mEffectType;//0 Title Effect, 1 transition Effect
	char*					mTitle;

	unsigned int			mStartTime;
	unsigned int			mEndTime;

	int						mRotateState;
	int  					mUserRotateState;
	int 					mTranslateX;
	int 					mTranslateY;
	int						mLUT;
	int 					mCustomLUT_A;
	int 					mCustomLUT_B;
	int 					mCustomLUT_Power;

	int						mBrightness;
	int						mSaturation;
    int                     mHue;
	int						mContrast;
	int						mTintColor;

	CRectangle				mStart;
	CRectangle				mEnd;
	CRectangle				mFace;
};

typedef std::vector<CNexDrawInfo*>	CDrawInfoVec;
typedef CDrawInfoVec::iterator		CDrawInfoVecItr;

#endif // __NEXVIDEOEDITOR_DRAWINFO_H__
