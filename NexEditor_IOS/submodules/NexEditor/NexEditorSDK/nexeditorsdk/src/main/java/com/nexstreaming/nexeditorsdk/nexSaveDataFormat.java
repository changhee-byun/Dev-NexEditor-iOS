package com.nexstreaming.nexeditorsdk;

import android.graphics.Rect;
import android.graphics.RectF;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * Created by jeongwook.yoon on 2017-10-12.
 */

final public class nexSaveDataFormat {
    public int nexSaveDataFormatVersion = 0;
    public nexProjectOf project;
    public nexCollageOf collage;
    public nexSaveDataFormat() {

    }

    public static class nexCollageOf{
        public List <nexCollageDrawInfoOf> drawInfos;
        public List <nexCollageTitleInfoOf> titleInfos;
    }

    public static class nexCollageDrawInfoOf{
        public RectF scaledRect;
        public String userLut;
    }

    public static class nexCollageTitleInfoOf{
        public String userText;
        public String userFont;
        public String userFillColor;
        public String userStrokeColor;
        public String userDropShadowColor;
    }

    public static class nexProjectOf{
        //private String mThemeId = EditorGlobal.DEFAULT_THEME_ID;
        //private String mLetterbox  = nexApplicationConfig.getDefaultLetterboxEffect();
        //private nexTheme mTheme = null;
        //private boolean mEmptyTheme = false;

        public int mProjectVolume = 100;
        public int mManualVolCtl = 0;
        public int mAudioFadeInTime = 200;
        public int mAudioFadeOutTime = 5000;

        public String mOpeningTitle = null;
        public String mEndingTitle = null;

        public float mBGMVolumeScale = 0.5f;
        //private int mId;
        //private static int sNextId = 1;

        public boolean mUseThemeMusic2BGM = true;
        public boolean mLoopBGM = true;
        public int mStartTimeBGM = 0;
        public nexClipOf mBackGroundMusic = null;
        public int mBGMTrimStartTime;
        public int mBGMTrimEndTime;

        //OverlayAsset mOverlayTemplate;

        //private int mExportMode;

        public int mTemplateApplyMode = 0;
        public boolean mTemplateOverlappedTransition = true;

        //private List<nexOverlayItem> mOverlayItems = new ArrayList<nexOverlayItem>(); --> TODO:
        //private List<nexOverlayItem> m_externalView_overlayItems = null;

        public List<nexClipOf> mPrimaryItems; //= new ArrayList<nexClipOf>();
        //private List<nexClip> m_externalView_primaryItems = null;
        public List<nexAudioItemOf> mSecondaryItems; //= new ArrayList<nexAudioItem>();
        //private List<nexAudioItem> m_externalView_secondaryItems = null;

        public List<nexDrawInfo> mTopEffectInfo; //= new ArrayList<>();
        public List<nexDrawInfo> mSubEffectInfo; //= new ArrayList<>();
    }


    //sync nex clip
    public static class nexClipOf{
        public String mPath = null;
        public String mTransCodingPath = null;
        //public boolean mProjectAttachment; --> internal set
        public nexEffectOf mClipEffect = null;
        public nexEffectOf mTransitionEffect = null;
        public boolean misMustDownSize;
        //private boolean m_gettingThumbnails=false; -->internal false set
        //private boolean m_getThumbnailsFailed=false; -->internal false set
        //private nexObserver mObserver; -->internal set
        public boolean  mAudioOnOff = true;
        //color adjust
        public int m_Brightness;
        public int m_Contrast;
        public int m_Saturation;
        public boolean mVignette;
        public boolean mFacedetectProcessed = false;
        public int     mFaceDetected = 0;
        public Rect    mFaceRect ;//= new Rect(); internal set

        public nexCropOf mCrop;
        //private int index = 0; --> no use
        //private int count = 0; --> no use
        public int mTitleEffectStartTime;
        public int mTitleEffectEndTime;

        public nexAudioEnvelopOf mAudioEnvelop = null;

        public nexAudioEditOf mAudioEdit;

        public int mTemplateEffectID = 0;
        public String mCollageDrawInfoID;
        public int mTemplateAudioPos = 0;
        public List<nexDrawInfo> mDrawInfos;
        public boolean mOverlappedTransition = true;
        public boolean mMediaInfoUseCache = true;
        public int mStartTime = 0;
        public int mEndTime = 0;
        public int mDuration = nexProject.kAutoThemeClipDuration;
        public nexClip.ClipInfo mInfo;
        public nexVideoClipEditOf mVideoEdit;
        public int mCustomLUT_A = 0;
        public int mCustomLUT_B = 0;
        public int mCustomLUT_Power = 100000;
        public nexColorEffectOf mColorEffect;
        public boolean mPropertySlowVideoMode;
        public int mAVSyncAudioStartTime = 0;
        public int m_ClipVolume = 100;
        public int m_BGMVolume = 100;
        public int mRotate;
    }

    public static class nexCropOf{
        public int m_startPositionLeft = 0;
        public int m_startPositionBottom = 0;
        public int m_startPositionRight = 0;
        public int m_startPositionTop = 0;
        public int m_endPositionLeft = 0;
        public int m_endPositionBottom = 0;
        public int m_endPositionRight = 0;
        public int m_endPositionTop = 0;

        public int m_rotatedStartPositionLeft = 0;
        public int m_rotatedStartPositionBottom = 0;
        public int m_rotatedStartPositionRight = 0;
        public int m_rotatedStartPositionTop = 0;
        public int m_rotatedEndPositionLeft = 0;
        public int m_rotatedEndPositionBottom = 0;
        public int m_rotatedEndPositionRight = 0;
        public int m_rotatedEndPositionTop = 0;

        public int m_facePositionLeft = 0;
        public int m_facePositionTop = 0;
        public int m_facePositionRight = 0;
        public int m_facePositionBottom = 0;
        public int m_rotatedFacePositionLeft = 0;
        public int m_rotatedFacePositionTop = 0;
        public int m_rotatedFacePositionRight = 0;
        public int m_rotatedFacePositionBottom = 0;

        //private float[] m_startMatrix;
        //private float[] m_endMatrix;

        public int m_rotation = 0;
        public int m_width = 0;
        public int m_height = 0;
        //private String m_mediaPath; --> internal set

        public float m_faceBounds_left;
        public float m_faceBounds_top;
        public float m_faceBounds_right;
        public float m_faceBounds_bottom;
        public boolean m_faceBounds_set;


    }

    public static class nexColorEffectOf{
        //private ColorMatrix colorMatrix;
        public float brightness;
        public float contrast;
        public float saturation;
        public float hue;
        public int tintColor;
        public String presetName;
        public boolean lut_enabled_ = false;
        public int lut_resource_id_ = 0;
        public String kineMasterID;
        public String assetItemID;
    }

    public static class nexEffectOf{
        public String mID;
        public String mAutoID;
        public String mName;
        public int mType;
        public int mDuration;
        public nexAssetPackageManager.ItemMethodType itemMethodType;

        //private nexObserver mObserver; --> internal set
        //public boolean mUpdated;
        public String[] mTitles= null;

        public boolean mIsResolveOptions = false;

        public boolean mOptionsUpdate;
        public HashMap<String,String> m_effectOptions;
        //clip effect
        public int mShowStartTime = 0;
        public int mShowEndTime = 10000;
        //transition effect
        public int mMinDuration;
        public int mMaxDuration;
        public int mEffectOffset;
        public int mEffectOverlap;
    }

    public static class nexVideoClipEditOf{
        //private nexClip mClip; --> internal set

        public int mTrimStartDuration ;
        public int mTrimEndDuration;

        public int mMasterSpeedControl = 100;
        public int mKeepPitch = 1;
        //boolean mUpdated; --> internal false set

    }

    public static class nexAudioEditOf{
        public int VCfactor=0; //0~4
        public int mPitch = 0; //-12 ~ 12
        public int mCompressor = 0; //0~7
        public int mProcessorStrength = -1; //0~6
        public int mBassStrength = -1; //0~6
        public int mMusicEffect = 0; //0~3
        public int mPanLeft = -111; //-100 ~ 100
        public int mPanRight = -111; //-100 ~ 100
        //public String mEnhancedAudioFilter = null; -> todo;
        //public String Equalizer = null; -> todo;
        //private nexClip mClip; --> internal set
    }

    public static class nexAudioEnvelopOf{
        public ArrayList<Integer> m_volumeEnvelopeTime;
        public ArrayList<Integer> m_volumeEnvelopeLevel;

        //private int [] m_cachedTimeList; --> no set
        //private int [] m_cachedLevelList; --> no set

        //private boolean m_modify = true; --> internal true set

        public int m_totalTime;
        public int m_trimStartTime;
        public int m_trimEndTime;
    }

    public static class nexAudioItemOf{
        public int mId;
        public nexClipOf mClip;
        public int mTrimStartDuration ;
        public int mTrimEndDuration;
        public int mSpeedControl = 100;
    }
}
