/******************************************************************************
 * File Name        : nexCollage.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdk;

import android.content.Context;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.utils.facedetect.FaceInfo;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

final class nexCollage {

    private static String TAG="nexCollage";

    private static final String TAG_COLLAGE_NAME = "collage_name";
    private static final String TAG_COLLAGE_VERSION = "collage_version";
    private static final String TAG_COLLAGE_DESCRIPTION = "collage_desc";
    private static final String TAG_COLLAGE_BGM = "collage_bgm";
    private static final String TAG_COLLAGE_BGM_VOLUME = "collage_bgm_volume";

    private static final String TAG_COLLAGE_TYPE = "collage_type";
    private static final String TAG_COLLAGE_DURATION = "collage_duration";
    private static final String TAG_COLLAGE_EDIT_TIME = "collage_edit_time";

    private static final String TAG_COLLAGE_RATIO = "collage_ratio";
    private static final String TAG_COLLAGE_DIMENSION = "collage_dimensions";

    private static final String TAG_COLLAGE_SOURCE_COUNT = "collage_source_count";

    private static final String TAG_EFFECT = "effect";

    private static final String TAG_PROJECT_FADE_IN = "collage_project_vol_fade_in_time";
    private static final String TAG_PROJECT_FADE_OUT = "collage_project_vol_fade_out_time";

    private static final String TAG_FRAME_COLLAGE = "frame_collage";

    private static final String TAG_DRAW_INFOS = "draw_infos";
    private static final String TAG_TITLE_INFOS = "title_infos";

    protected interface CollageInfoChangedListener {
        public void SourceChanged(nexClip remove, nexClip add);
        public void DrawInfoChanged(nexDrawInfo info);
        public String CollageTime(String format);
        public String TitleInfoContentTime(String drawId, String format);
        public void TitleInfoChanged();
        public RectF FaceRect(String path);
    }

    private String name;
    private String version;
    private String desc;
    private String bgm;
    private float bgm_volume = 1;

    private nexCollageManager.CollageType type;
    private int duration;
    private float editTime;

    private String ratio;
    private String dimension;
    private int sourceCount;
    private String effect;
    private boolean frameCollage;

    private List<nexCollageDrawInfo> drawInfos = new ArrayList<>();
    private List<nexCollageTitleInfo> titleInfos = new ArrayList<>();

    private float width;
    private float height;

    private int fadeInTime = 200;
    private int fadeOutTime = 1000;

    private nexProject project;
    private nexEngine engine;
    private Context context;

    private Date collageTime;

    private boolean isParsed = false;

    public float getEditTime() {
        return editTime;
    }

    public int getSourceCount() {
        return sourceCount;
    }

    public int getDuration() {
        return duration;
    }

    public String getDimension() {
        return dimension;
    }

    public nexCollageManager.CollageType getType() {
        return type;
    }

    public float getCollageRatio() {
        return width / height;
    }

    public List<nexCollageDrawInfo> getDrawInfos() {
        return drawInfos;
    }
    public List<nexCollageTitleInfo> getTitleInfos() {
        return titleInfos;
    }

    public static void restoreDimension( Rect rect, int width, int height ) {
        rect.left	= rect.left * width / nexCrop.ABSTRACT_DIMENSION ;
        rect.top	= rect.top * height / nexCrop.ABSTRACT_DIMENSION;
        rect.right	= rect.right * width / nexCrop.ABSTRACT_DIMENSION;
        rect.bottom = rect.bottom * height / nexCrop.ABSTRACT_DIMENSION;
    }

    public static void updateDimension( Rect rect, int width, int height ) {
        rect.left	= rect.left * nexCrop.ABSTRACT_DIMENSION / width;
        rect.top	= rect.top * nexCrop.ABSTRACT_DIMENSION / height ;
        rect.right	= rect.right * nexCrop.ABSTRACT_DIMENSION / width ;
        rect.bottom = rect.bottom * nexCrop.ABSTRACT_DIMENSION / height;
    }

    public static void shrinkToAspect( Rect rc, float aspect ) {
        float originalAspect = ((float)rc.width()) / (float)(rc.height());

        Log.d(TAG, String.format("shrinkToAspect(%f %f)", originalAspect, aspect));
        if( originalAspect < aspect ) {
            // Original too tall
            int newHeight = (int)(((float)rc.width())/aspect);
            int ctry = rc.centerY();
            rc.top = ctry - newHeight/2;
            rc.bottom = rc.top + newHeight;
        } else {
            // Original too wide
            int newWidth = (int)(((float)rc.height())*aspect);
            int ctrx = rc.centerX();
            rc.left = ctrx - newWidth/2;
            rc.right = rc.left + newWidth;
        }

        Log.d(TAG, String.format("shrinkToAspect: ") + rc.toString());
    }

    private String getCollageTitle(int duration) {
        StringBuilder b = new StringBuilder();
        b.append(0);
        b.append(',');
        b.append(duration);
        b.append('?');
        b.append('?');

        Iterator<nexCollageTitleInfo> infos = titleInfos.iterator();
        while( infos.hasNext() ) {
            nexCollageTitleInfo info = infos.next();
            b.append(info.getTitle());

            if( infos.hasNext() )
                b.append('&');
        }

        return b.toString();
    }

    private RectF detectFace(String path) {
        RectF _sbounds = new RectF();
        FaceInfo face = FaceInfo.getFaceInfo(path);
        if(null == face){
            face = new FaceInfo(new File(path), true, context);
            FaceInfo.putFaceInfo(path, face);
        }

        face.getBounds(_sbounds);
        return _sbounds;
    }

    protected String apply2Project(nexProject project, nexEngine engine, int recommendDuration, Context appContext, boolean isLoad) {

        Log.d(TAG, String.format("apply2Project sources(%d %d %d)", sourceCount, project.getTotalClipCount(true), recommendDuration));

        this.context = appContext;

        if( recommendDuration == 0 ) {
            recommendDuration = duration;
        }

        for(int i = 0; i < project.getTotalClipCount(true); i++) {
            nexClip c = project.getClip(i, true);
            if( c.isMotionTrackedVideo() || c.isAssetResource() )
                project.remove(c);
        }

        for(int i = 0; i < project.getTotalClipCount(false); i++) {
            nexClip c = project.getClip(i, false);
            if( c.isAssetResource() )
                project.remove(c);
        }

        boolean videoExist = false;
        int maxDur = 0;
        List<nexClip> clips = new ArrayList<>();
        while( project.getTotalClipCount(true) > 0 ) {
            nexClip clip = project.getClip(0, true);
            if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
                clip.setAudioOnOff(false);
                videoExist = true;
            }
            clips.add(clip);
            maxDur = maxDur > clip.getProjectDuration() ? maxDur : clip.getProjectDuration();
            project.remove(clip);
        }

        recommendDuration = recommendDuration == 0 ? maxDur : recommendDuration;

//        if( type == nexCollageManager.CollageType.StaticCollage && videoExist == false ) {
//            recommendDuration = 1000;
//        }

        project.getTopDrawInfo().clear();
        project.setTemplateApplyMode(3);

        nexDrawInfo topEffect = new nexDrawInfo();
        topEffect.setID(1);
        topEffect.setSubEffectID((1 << 16) | 1);
        topEffect.setEffectID(effect);
        topEffect.setStartTime(0);
        topEffect.setEndTime(recommendDuration);

        project.getTopDrawInfo().add(topEffect);

        collageTime = new Date();

        int subEffectIdx = 1;
        nexClip clip = null;
        for( nexCollageDrawInfo drawInfo : drawInfos ) {
            if( drawInfo.isSystemSource() ) {
                String path = drawInfo.getSystemResource(appContext);
                clip = nexClip.getSupportedClip(path);
                clip.setAssetResource(true);
                if( drawInfo.isMotionTrackedSource() ) {
                    clip.setMotionTrackedVideo(true);
                }
            }
            else {
                if (clip == null && clips.size() > 0) {
                    clip = clips.get(0);
                    clips.remove(clip);
                }
            }

            if( clip == null )
            {
                continue;
            }

            project.add(clip);

            nexDrawInfo oldDrawInfo = null;
            if( clip.getDrawInfos().size() > 0 ) {
                oldDrawInfo = clip.getDrawInfos().get(0);
            }

            clip.clearDrawInfos();
            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                clip.setImageClipDuration(recommendDuration);
            }
            else {
                clip.getVideoClipEdit().clearTrim();
                if( clip.getTotalTime() > recommendDuration )
                {
                    clip.getVideoClipEdit().setTrim(0, recommendDuration);
                }
                // clip.getVideoClipEdit().setTrim(0, recommendDuration);

            }
            clip.mStartTime = 0;
            clip.mEndTime = recommendDuration;

            nexDrawInfo info = new nexDrawInfo();
            info.setTopEffectID(topEffect.getID());
            info.setID((1 << 16) | subEffectIdx);
            info.setSubEffectID((1 << 16) | subEffectIdx);
            subEffectIdx++;

            int sTime = (int)((float)recommendDuration * drawInfo.getStart());
            int eTime = (int)((float)recommendDuration * drawInfo.getEnd());

            Log.d(TAG, String.format("Collage setDrawInfo2Clip(dur:%d %d %d)", recommendDuration, sTime, eTime));

            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                clip.mStartTime = sTime < clip.mStartTime ? sTime : clip.mStartTime;
                clip.mEndTime = eTime > clip.mEndTime ? eTime : clip.mEndTime;

                clip.setImageClipDuration(clip.mEndTime - clip.mStartTime);
            }else {
                sTime = sTime < clip.getProjectStartTime() ? clip.getProjectStartTime() : sTime;
                eTime = eTime > clip.getProjectEndTime() ? clip.getProjectEndTime() : eTime;
            }

            info.setStartTime(sTime);
            info.setEndTime(eTime);

            if( drawInfo.getDefaultLut() != null && drawInfo.getDefaultLut().compareTo("none") != 0 ) {
                nexColorEffect colorEffect = nexColorEffect.getLutColorEffect(drawInfo.getDefaultLut());
                if( colorEffect != null)
                    info.setLUT( colorEffect.getLUTId() );
            }

            int iWidth = clip.getWidth();
            int iHeight = clip.getHeight();

            if( clip.getRotateInMeta() == 90 || clip.getRotateInMeta() == 270 ) {
                iWidth = clip.getHeight();
                iHeight = clip.getWidth();
                if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO )
                    info.setRotateState(clip.getRotateInMeta());
            }

            Rect sr = new Rect(0, 0, iWidth, iHeight);
            Rect fr = new Rect(0, 0, 1, 1);

            if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE)
            {
                RectF _sbounds = detectFace(clip.getPath());
                sr.set((int)(_sbounds.left*iWidth), (int)(_sbounds.top*iHeight), (int)(_sbounds.right*iWidth), (int)(_sbounds.bottom*iHeight));

                if(sr.isEmpty()){
                    sr = new Rect(0, 0, iWidth, iHeight);
                }else {

                    fr.set((int)(_sbounds.left*iWidth), (int)(_sbounds.top*iHeight), (int)(_sbounds.right*iWidth), (int)(_sbounds.bottom*iHeight));

                    int addspace = (sr.right - sr.left)/2;
                    sr.left -= addspace;
                    sr.right += addspace;
                    if (sr.left < 0) {
                        sr.left = 0;
                    }
                    if (sr.right > iWidth) {
                        sr.right = iWidth;
                    }
                    addspace = (sr.bottom - sr.top)/2;
                    sr.top -= addspace;
                    sr.bottom += addspace;
                    if (sr.top < 0) {
                        sr.top = 0;
                    }
                    if (sr.bottom > iHeight) {
                        sr.bottom = iHeight;
                    }
                    //clip.getCrop().growToAspect(sr, nexApplicationConfig.getAspectRatio());
                }
            }

            Log.d(TAG, String.format("Collage setDrawInfo2Clip") + drawInfo.getRectangle().toString());

            float displayRatio = (drawInfo.getRectangle().width() * width) / (drawInfo.getRectangle().height() * height);

            float w = drawInfo.getDrawWidth();
            float h = drawInfo.getDrawHeight();
            if( w != 0 && h != 0 ) {
                displayRatio = w / h;
            }

            shrinkToAspect(sr, displayRatio);

            Log.d(TAG, String.format("Collage setDrawInfo2Clip sr : ") + sr.toString());

            updateDimension(sr, iWidth, iHeight);
            updateDimension(fr, iWidth, iHeight);

            Log.d(TAG, String.format("Collage setDrawInfo2Clip1 sr : ") + sr.toString());
            Log.d(TAG, String.format("Collage setDrawInfo2Clip clip size(%d %d)", iWidth, iHeight));

            info.setStartRect(sr);
            info.setEndRect(sr);

            info.setFaceRect(fr);

            clip.addDrawInfo(info);
            drawInfo.setBaseClip(clip);
            drawInfo.setDisplayRatio(displayRatio);
            drawInfo.applySoundEffect2Clip(project, recommendDuration);
            

            boolean calcscale = false;

            if( oldDrawInfo != null )
            {
                info.setRotateState(oldDrawInfo.getRotateState());
                info.setUserTranslate(oldDrawInfo.getUserTranslateX(), oldDrawInfo.getUserTranslateY());
                info.setUserRotateState(oldDrawInfo.getUserRotateState());
                info.setRealScale(oldDrawInfo.getRealScale());
                info.setBrightness(oldDrawInfo.getBrightness());
                info.setUserLUT(oldDrawInfo.getUserLUT());

                if( isLoad ) {
                    String userLUT = oldDrawInfo.getUserLUT();
                    if( userLUT != null && userLUT.compareTo("none") != 0 && userLUT.compareTo("null") != 0 ) {
                        nexColorEffect colorEffect = nexColorEffect.getLutColorEffect(userLUT);
                        if( colorEffect != null && colorEffect.getLUTId() == oldDrawInfo.getLUT() ) {
                            info.setLUT(oldDrawInfo.getLUT());
                        }
                    }
                    info.setStartRect(oldDrawInfo.getStartRect());
                    info.setEndRect(oldDrawInfo.getEndRect());
                    info.setFaceRect(oldDrawInfo.getFaceRect());
                    
                    // Log.d(TAG, String.format("Collage setDrawInfo2Clip1 old sr : ") + oldDrawInfo.getStartRect().toString());

                }
            }
            else{
                calcscale = true;
            }
            drawInfo.setScaleReferenceRect(null);
            drawInfo.updatePosition(calcscale);
            Log.d(TAG, String.format("Collage apply rect(%s) %f", info.getStartRect().toString(), info.getRealScale()));
            clip = null;
        }

        topEffect.setTitle(getCollageTitle(recommendDuration));

        if( bgm != null && bgm.length() > 0 && bgm.compareTo("none") != 0 ) {
            project.setBackgroundMusicPath(bgm);
            project.setBGMMasterVolumeScale(bgm_volume);
        }
        else {
            project.setBackgroundMusicPath(null);
            project.setBGMMasterVolumeScale(0);
        }

        project.setProjectAudioFadeInTime(fadeInTime);
        project.setProjectAudioFadeOutTime(fadeOutTime);

        engine.setProperty("PreviewFPS", "30");
        engine.setProject(project);
        engine.updateProject();
        this.project = project;
        this.engine = engine;

        return null;
    }

    protected boolean setBGM(String path) {
        if( project == null || engine == null )
            return false;
        if( path == null ) {
            if( bgm != null ) {
                project.setBackgroundMusicPath(bgm);
                project.setBGMMasterVolumeScale(bgm_volume);
                return true;
            }
            else {
                project.setBackgroundMusicPath(null);
            }

        }
        else {
            project.setBackgroundMusicPath(path);
        }
        engine.updateProject();
        return true;
    }

    protected boolean isFrameCollage() {
        return frameCollage;
    }

    public boolean swapDrawInfoClip(nexCollageDrawInfo info1, nexCollageDrawInfo info2) {
        if( info1 == null || info2 == null ) return false;

        nexClip clip1 = info1.getBindSource();
        nexClip clip2 = info2.getBindSource();
        if( clip1 == null || clip2 == null ) return false;

        nexClip clone1 = nexClip.dup(clip1);
        nexClip clone2 = nexClip.dup(clip2);

        if( clone1.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
            clone1.setAudioOnOff(false);
        }

        if( clone2.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
            clone2.setAudioOnOff(false);
        }

        nexDrawInfo di1 = clip1.getDrawInfos().get(0);
        nexDrawInfo di2 = clip2.getDrawInfos().get(0);

        int tmpRotate = di1.getRotateState();
        int tmpX = di1.getUserTranslateX();
        int tmpY = di1.getUserTranslateY();
        int tmpUserRotate = di1.getUserRotateState();
        float tmpRealScale = di1.getRealScale();
        int tmpLUT = di1.getLUT();
        int tmpCustomLUTA = di1.getCustomLUTA();
        int tmpCustomLUTB = di1.getCustomLUTB();
        int tmpCustomLUTPower = di1.getCustomLUTPower();
        String tmpUserLUT = di1.getUserLUT();
        int tmpBrightness = di1.getBrightness();

        di1.setRotateState(di2.getRotateState());
        di1.setUserTranslate(di2.getUserTranslateX(), di2.getUserTranslateY());
        di1.setUserRotateState(di2.getUserRotateState());
        di1.setRealScale(di2.getRealScale());

        di1.setLUT(0);
        String userLUT = di2.getUserLUT();
        if( userLUT != null && userLUT.compareTo("none") != 0 && userLUT.compareTo("null") != 0 ) {
            di1.setLUT(di2.getLUT());
        }
        else if( info1.getDefaultLut() != null && (info1.getDefaultLut().compareTo("none") != 0 && info1.getDefaultLut().compareTo("null") != 0) ) {
            nexColorEffect colorEffect = nexColorEffect.getLutColorEffect(info1.getDefaultLut());
            if( colorEffect != null ) {
                di1.setLUT(colorEffect.getLUTId());
            }
        }

        di1.setCustomLUTA(di2.getCustomLUTA());
        di1.setCustomLUTB(di2.getCustomLUTB());
        di1.setCustomLUTPower(di2.getCustomLUTPower());

        di1.setUserLUT(di2.getUserLUT());
        di1.setBrightness(di2.getBrightness());

        info1.updateSource(clone2, di1);

        di2.setRotateState(tmpRotate);
        di2.setUserTranslate(tmpX, tmpY);
        di2.setUserRotateState(tmpUserRotate);
        di2.setRealScale(tmpRealScale);

        di2.setLUT(0);
        if( tmpUserLUT != null && tmpUserLUT.compareTo("none") != 0 && tmpUserLUT.compareTo("null") != 0 ) {
            di2.setLUT(tmpLUT);
        }
        else if( info2.getDefaultLut() != null && (info2.getDefaultLut().compareTo("none") != 0 && info2.getDefaultLut().compareTo("null") != 0) ) {
            nexColorEffect colorEffect = nexColorEffect.getLutColorEffect(info2.getDefaultLut());
            if( colorEffect != null ) {
                di2.setLUT(colorEffect.getLUTId());
            }
        }

        di2.setCustomLUTA(tmpCustomLUTA);
        di2.setCustomLUTB(tmpCustomLUTB);
        di2.setCustomLUTPower(tmpCustomLUTPower);

        di2.setUserLUT(tmpUserLUT);
        di2.setBrightness(tmpBrightness);

        info2.updateSource(clone1, di2);

        int id = 1;
        for (int i = 0; i < project.getTotalClipCount(true); i++) {
            if (project.getClip(i, true).equals(clip1)) {
                project.add(id-1, true, clone2);
                project.remove(clip1);
                engine.removeClip(id);
            }

            if (project.getClip(i, true).equals(clip2)) {
                project.add(id-1, true, clone1);
                project.remove(clip2);
                engine.removeClip(id);
            }
            id++;
        }

        engine.updateProject();
        engine.setThumbnailRoutine(2);
        engine.seek((int) (duration * editTime));
        return true;
    }

    protected boolean isParsed() { return isParsed; }

    public String parseCollage(JSONObject object) {
        if( object == null ) {
            return "Collage parsing error with invalid handle";
        }

        try {
            name = object.getString(TAG_COLLAGE_NAME);
            version = object.getString(TAG_COLLAGE_VERSION);
            desc = object.getString(TAG_COLLAGE_DESCRIPTION);
            bgm = object.getString(TAG_COLLAGE_BGM);
            bgm_volume = Float.parseFloat(object.getString(TAG_COLLAGE_BGM_VOLUME));

            if( object.getString(TAG_COLLAGE_TYPE).compareTo("static") == 0 ) {
                type = nexCollageManager.CollageType.StaticCollage;

            }
            else if( object.getString(TAG_COLLAGE_TYPE).compareTo("dynamic") == 0 ) {
                type = nexCollageManager.CollageType.DynamicCollage;
            }

            duration = Integer.parseInt(object.getString(TAG_COLLAGE_DURATION));
            editTime = Float.parseFloat(object.getString(TAG_COLLAGE_EDIT_TIME));

            ratio = object.getString(TAG_COLLAGE_RATIO);
            String[] rat = ratio.toLowerCase().split("v");
            if (rat == null) {
                return "Wrong ratio info was included";
            }

            width = Float.parseFloat(rat[0]);
            height = Float.parseFloat(rat[1]);

            // dimension = object.getString(TAG_COLLAGE_DIMENSION);

            sourceCount = Integer.parseInt(object.getString(TAG_COLLAGE_SOURCE_COUNT));

            if( object.has(TAG_PROJECT_FADE_IN) ) {
                fadeInTime = Integer.parseInt(object.getString(TAG_PROJECT_FADE_IN));
            }

            if( object.has(TAG_PROJECT_FADE_OUT) ) {
                fadeOutTime = Integer.parseInt(object.getString(TAG_PROJECT_FADE_OUT));
            }

            frameCollage = false;
            if( object.has(TAG_FRAME_COLLAGE) ) {
                frameCollage = object.getString(TAG_FRAME_COLLAGE).compareTo("1") == 0;
            }

            effect = object.getString(TAG_EFFECT);

            if(object.has(TAG_DRAW_INFOS)) {
                JSONArray draw_infos = object.getJSONArray(TAG_DRAW_INFOS);
                for (int i = 0; i < draw_infos.length(); i++) {
                    nexCollageDrawInfo drawInfo = new nexCollageDrawInfo();
                    String ret = drawInfo.parseDrawInfo(draw_infos.getJSONObject(i));
                    if (ret != null) {
                        drawInfos.clear();
                        titleInfos.clear();
                        return "Collage drawinfo parse error";
                    }
                    drawInfo.setInfoChangedListener(new com.nexstreaming.nexeditorsdk.nexCollage.CollageInfoChangedListener() {
                        @Override
                        public void SourceChanged(nexClip remove, nexClip add) {
                            if (engine != null) {

                                int id = 1;
                                for (int i = 0; i < project.getTotalClipCount(true); i++) {
                                    if (project.getClip(i, true).equals(remove)) {
                                        break;
                                    }
                                    id++;
                                }

                                project.add(id-1, true, add);
                                project.remove(remove);

                                engine.removeClip(id);

                                engine.updateProject();
                                // engine.seek((int) (duration * editTime));
                            }
                        }

                        @Override
                        public void DrawInfoChanged(nexDrawInfo info) {
                            if (engine != null) {
                                engine.updateDrawInfo(info);
                            }
                        }

                        @Override
                        public String CollageTime(String format) {
                            return "";
                        }

                        @Override
                        public String TitleInfoContentTime(String drawId, String format) {
                            return "";
                        }

                        @Override
                        public void TitleInfoChanged() {

                        }

                        @Override
                        public RectF FaceRect(String path) {
                            return detectFace(path);
                        }
                    });
                    drawInfos.add(drawInfo);
                    continue;
                }
            }

            if(object.has(TAG_TITLE_INFOS)){
                JSONArray title_infos = object.getJSONArray(TAG_TITLE_INFOS);
                for (int i = 0; i < title_infos.length(); i++) {
                    nexCollageTitleInfo titleInfo = new nexCollageTitleInfo();
                    String ret = titleInfo.parseTitleInfo(title_infos.getJSONObject(i));
                    if( ret != null ) {
                        drawInfos.clear();
                        titleInfos.clear();
                        return "Collage title info parse error";
                    }

                    titleInfo.setInfoChangedListener(new com.nexstreaming.nexeditorsdk.nexCollage.CollageInfoChangedListener() {
                        @Override
                        public void SourceChanged(nexClip remove, nexClip add) {
                        }

                        @Override
                        public void DrawInfoChanged(nexDrawInfo info) {
                        }

                        @Override
                        public String CollageTime(String format)
                        {
                            if( collageTime == null )
                                collageTime = new Date();
                            SimpleDateFormat df = new SimpleDateFormat(format);
                            return df.format(collageTime);
                        }

                        @Override
                        public String TitleInfoContentTime(String drawId, String format)
                        {
                            for( nexCollageDrawInfo info : drawInfos ) {
                                if( info.getId().compareTo(drawId) == 0 ) {
                                    return info.getContentTime(format);
                                }
                            }
                            return "";
                        }

                        @Override
                        public void TitleInfoChanged() {
                            if( project != null ) {
                                project.getTopDrawInfo().get(0).setTitle(getCollageTitle(project.getTotalTime()));
                                if (engine != null) {
                                    engine.updateDrawInfo(project.getTopDrawInfo().get(0));
                                }
                            }

                        }

                        @Override
                        public RectF FaceRect(String path) {
                            return detectFace(path);
                        }
                    });

                    if( titleInfo.getDrawId() != null ) {

                    }

                    titleInfos.add(titleInfo);
                    continue;
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
            if(LL.D) Log.d(TAG, "parse Collage header failed : " + e.getMessage());
            return e.getMessage();
        }

        isParsed = true;
        return null;
    }

    nexSaveDataFormat getSaveData() {
        if( project == null ) return null;

        nexSaveDataFormat data = new nexSaveDataFormat();
        data.project = project.getSaveData();
        data.collage = new nexSaveDataFormat.nexCollageOf();

        if (titleInfos.size() == 0) {
            data.collage.titleInfos = null;
        } else{
            data.collage.titleInfos = new ArrayList<>();
            for (nexCollageTitleInfo info : titleInfos) {
                data.collage.titleInfos.add( info.getSaveData());
            }
        }

        return data;
    }

    void loadSaveData(nexSaveDataFormat.nexCollageOf data){
        if( data.titleInfos != null ){
            int i = 0;
            for( nexCollageTitleInfo info : titleInfos){
                if( i >= data.titleInfos.size() )
                    break;
                info.loadSaveData(data.titleInfos.get(i));
                i++;
            }
        }
    }
}
