/******************************************************************************
 * File Name        : nexTemplate.java
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
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.graphics.Rect;
import android.media.ExifInterface;
import android.os.Environment;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.util.Floats;
import com.nexstreaming.app.common.util.Stopwatch;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.nexeditorsdk.exception.nexSDKException;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.Stack;
import java.util.Vector;

final class nexTemplate {
    private static String TAG="nexTemplate";

    private static final String TAG_TEMPLATE = "template";
    private static final String TAG_TEMPLATE_NAME = "template_name";
    private static final String TAG_TEMPLATE_VERSION = "template_version";
    private static final String TAG_TEMPLATE_DESCRIPTION = "template_desc";
    private static final String TAG_TEMPLATE_MODE = "template_mode";
    private static final String TAG_TEMPLATE_INTRO = "template_intro";
    private static final String TAG_TEMPLATE_LOOP = "template_loop";
    private static final String TAG_TEMPLATE_OUTRO = "template_outro";
    private static final String TAG_TEMPLATE_SINGLE = "template_single";
    private static final String TAG_TEMPLATE_BGM = "template_bgm";
    private static final String TAG_TEMPLATE_SINGLE_BGM = "template_single_bgm";
    private static final String TAG_TEMPLATE_BGM_VOLUME = "template_bgm_volume";
    private static final String TAG_TEMPLATE_DEFAULT_IMG_DUR = "template_default_image_duration";

    private static final String TAG_TEMPLATE_LUT = "template_lut";
	private static final String TAG_TEMPLATE_PROJECT_VOL_FADE_IN_TIME = "template_project_vol_fade_in_time";
	private static final String TAG_TEMPLATE_PROJECT_VOL_FADE_OUT_TIME = "template_project_vol_fade_out_time";
    private static final String TAG_TEMPLATE_SINGLE_PROJECT_VOL_FADE_IN_TIME = "template_single_project_vol_fade_in_time";
    private static final String TAG_TEMPLATE_SINGLE_PROJECT_VOL_FADE_OUT_TIME = "template_single_project_vol_fade_out_time";

    private static final String TAG_TEMPLATE_SINGLE_VIDEO = "template_single_video";

    private static final String TAG_MIN_DURATION = "template_min_duration";
    private static final String TAG_DEFAULT_EFFECT = "template_default_effect";
    private static final String TAG_DEFAULT_EFFECT_SCALE = "template_default_effect_scale";

    private int mTemplateEffectID = 1;

    String template_name;
    String template_version;
    String template_desc;
    String template_mode;
    String template_bgm;
    String template_single_bgm;
    float template_bgm_volume = 1;
    int template_default_image_duration = 2500;

    int template_project_vol_fade_in_time = -1;
    int template_project_vol_fade_out_time = -1;

    int template_single_project_vol_fade_in_time = -1;
    int template_single_project_vol_fade_out_time = -1;

    int template_single_video = 0;

    int template_min_duration;
    String template_default_effect;
    boolean template_default_effect_scale = true;

    float template_ratio;

    boolean useClipSpeed = false;
    String lastEffectIdentifier = "";

    boolean overlappedTransition = true;

    private ArrayList<nexTemplateEffect> mIntroList;
    private ArrayList<nexTemplateEffect> mLoopList;
    private ArrayList<nexTemplateEffect> mOutroList;
    private ArrayList<nexTemplateEffect> mSingleList;

    private Map<String, ArrayList<nexTemplateEffect>> mAlternativeOutro;

    protected static boolean mUseVideoTrim = false;
    protected static int mVideoMemorySize = 1920*1080*5;
    private boolean mCancel = false;

    /**
     * This method creates an instance of <tt>nexTemplateComposer</tt>.
     *
     * @since version 1.0.0
     */
    public nexTemplate() {
        mIntroList = new ArrayList<>();
        mLoopList = new ArrayList<>();
        mOutroList = new ArrayList<>();
        mSingleList = new ArrayList<>();

        mAlternativeOutro = new HashMap<String, ArrayList<nexTemplateEffect>>();
    }

    /**
     * This method clears the resources of <tt>nexTemplateComposer</tt>.
     *
     * This method must be called when the activity is destroyed.
     *
     * @since version 1.0.0
     */
    public void release() {
        mCancel = false;

        if(mIntroList != null) {
            mIntroList.clear();
        }
        if(mLoopList != null) {
            mLoopList.clear();
        }
        if(mOutroList != null) {
            mOutroList.clear();
        }
        if(mSingleList != null) {
            mSingleList.clear();
        }
    }

    String parseTemplateInfo(JSONObject object) {
        try {
            template_name = object.getString(TAG_TEMPLATE_NAME);
            template_version = object.getString(TAG_TEMPLATE_VERSION);
            template_desc = object.getString(TAG_TEMPLATE_DESCRIPTION);
            template_mode = object.getString(TAG_TEMPLATE_MODE);

            template_ratio = 16 / 9.0f;
            if (template_mode.equals("16v9")) {
                template_ratio = 16 / 9.0f;
            } else if (template_mode.equals("9v16")) {
                template_ratio = 9 / 16.0f;
            }else if (template_mode.equals("1v1")) {
                template_ratio = 1.0f;
            }else if (template_mode.equals("2v1")) {
                template_ratio = 2.0f;
            }else if (template_mode.equals("1v2")) {
                template_ratio = 0.5f;
            }

            template_bgm = object.getString(TAG_TEMPLATE_BGM);
            if (object.has(TAG_TEMPLATE_SINGLE_BGM))
                template_single_bgm = object.getString(TAG_TEMPLATE_SINGLE_BGM);

            if (object.has(TAG_TEMPLATE_BGM_VOLUME))
                template_bgm_volume = Float.parseFloat(object.getString(TAG_TEMPLATE_BGM_VOLUME));

            if( object.has(TAG_MIN_DURATION) )
                template_min_duration = Integer.parseInt(object.getString(TAG_MIN_DURATION));
            if( object.has(TAG_DEFAULT_EFFECT) )
                template_default_effect = object.getString(TAG_DEFAULT_EFFECT);

            if( object.has(TAG_DEFAULT_EFFECT_SCALE) )
                template_default_effect_scale = object.getString(TAG_DEFAULT_EFFECT_SCALE).equals("0") ? false : true;

            if( object.has(TAG_TEMPLATE_DEFAULT_IMG_DUR) && !object.getString(TAG_TEMPLATE_DEFAULT_IMG_DUR).equals("default") )
                template_default_image_duration = Integer.parseInt(object.getString(TAG_TEMPLATE_DEFAULT_IMG_DUR));

            if( object.has(TAG_TEMPLATE_PROJECT_VOL_FADE_IN_TIME) && !object.getString(TAG_TEMPLATE_PROJECT_VOL_FADE_IN_TIME).equals("default") )
                template_project_vol_fade_in_time = Integer.parseInt(object.getString(TAG_TEMPLATE_PROJECT_VOL_FADE_IN_TIME));

            if( object.has(TAG_TEMPLATE_PROJECT_VOL_FADE_OUT_TIME) && !object.getString(TAG_TEMPLATE_PROJECT_VOL_FADE_OUT_TIME).equals("default") )
                template_project_vol_fade_out_time = Integer.parseInt(object.getString(TAG_TEMPLATE_PROJECT_VOL_FADE_OUT_TIME));

            if( object.has(TAG_TEMPLATE_SINGLE_PROJECT_VOL_FADE_IN_TIME) && !object.getString(TAG_TEMPLATE_SINGLE_PROJECT_VOL_FADE_IN_TIME).equals("default") )
                template_single_project_vol_fade_in_time = Integer.parseInt(object.getString(TAG_TEMPLATE_SINGLE_PROJECT_VOL_FADE_IN_TIME));

            if( object.has(TAG_TEMPLATE_SINGLE_PROJECT_VOL_FADE_OUT_TIME) && !object.getString(TAG_TEMPLATE_SINGLE_PROJECT_VOL_FADE_OUT_TIME).equals("default") )
                template_single_project_vol_fade_out_time = Integer.parseInt(object.getString(TAG_TEMPLATE_SINGLE_PROJECT_VOL_FADE_OUT_TIME));

            if( object.has(TAG_TEMPLATE_SINGLE_VIDEO)  )
                template_single_video = Integer.parseInt(object.getString(TAG_TEMPLATE_SINGLE_VIDEO));


        } catch (JSONException e) {
            e.printStackTrace();
            if(LL.D) Log.d(TAG, "parse Template failed : " + e.getMessage());

            Log.d(TAG,"case1 1");
            return e.getMessage();
        }

        return null;
    }

    String parseTemplate(String templateFile, JSONObject jsonObject) {

        mCancel = false;

        if( templateFile != null ) {
            File file = new File(templateFile);

            if (file != null) {
                Log.d(TAG, String.format("Template templateFile path(%s)", file.getAbsolutePath()));
                if( file.getAbsolutePath().endsWith("txt") ) {
                    // mTemplatePath = file.getAbsolutePath().substring(0, file.getAbsolutePath().lastIndexOf(File.separator));
                }
            }
        }

        mTemplateEffectID = 1;

        String ret = parseTemplateInfo(jsonObject);

        if( ret != null )
        {
            return "Template header parse error : " + ret;
        }

        JSONArray IntroTemplateArray = null;
        try {
            IntroTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_INTRO);
            for (int i = 0; i < IntroTemplateArray.length(); i++) {
                nexTemplateEffect te = nexTemplateEffect.getTemplateEffect(IntroTemplateArray.getJSONObject(i), mTemplateEffectID++);
                if( te != null ) {
                    mIntroList.add(te);
                    continue;
                }
                return "Template intro parse error";
            }
            JSONArray LoopTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_LOOP);
            for (int j = 0; j < LoopTemplateArray.length(); j++) {
                nexTemplateEffect te = nexTemplateEffect.getTemplateEffect(LoopTemplateArray.getJSONObject(j), mTemplateEffectID++);
                if( te != null ) {

                    mLoopList.add(te);
                    continue;
                }
                return "Template Loop parse error";
            }
            //
            JSONArray OutroTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_OUTRO);
            for (int j = 0; j < OutroTemplateArray.length(); j++) {
                JSONObject jsonElement = OutroTemplateArray.getJSONObject(j);
                if( jsonElement == null ) continue;

                if( jsonElement.has("alternative_id") && jsonElement.has("alternative_outro") )
                {
                    String alternative_id = jsonElement.getString("alternative_id");
                    JSONArray alternative_outro = jsonElement.getJSONArray("alternative_outro");

                    ArrayList<nexTemplateEffect> outros = new ArrayList<>();

                    for( int k = 0; k < alternative_outro.length(); k++ ) {

                        nexTemplateEffect te = nexTemplateEffect.getTemplateEffect(alternative_outro.getJSONObject(k), mTemplateEffectID++);
                        if (te == null) {
                            return "Template Outro parse error(alternative outro)";
                        }
                        outros.add(te);
                    }

                    mAlternativeOutro.put(alternative_id, outros);
                }
                else
                {
                    nexTemplateEffect te = nexTemplateEffect.getTemplateEffect(OutroTemplateArray.getJSONObject(j), mTemplateEffectID++);
                    if (te != null) {
                        mOutroList.add(te);
                        continue;
                    }
                    return "Template Outro parse error";
                }
            }

            if(jsonObject.has(TAG_TEMPLATE_SINGLE)) {
                JSONArray SingleTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_SINGLE);
                for (int j = 0; j < SingleTemplateArray.length(); j++) {
                    nexTemplateEffect te = nexTemplateEffect.getTemplateEffect(SingleTemplateArray.getJSONObject(j), mTemplateEffectID++);
                    if (te != null) {
                        mSingleList.add(te);
                        continue;
                    }
                    return "Template Single parse error";
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();

            mIntroList.clear();
            mLoopList.clear();
            mOutroList.clear();
            mSingleList.clear();

            if(LL.D) Log.d(TAG, "parseTemplate failed : " + e.getMessage());
            return "parseTemplate failed : " + e.getMessage();
        }

        if(LL.D) Log.d(TAG, "parseTemplate end");
        return null;
    }

    int getTemplateMinDuration()
    {
        return template_min_duration;
    }

    int getIntroDuration()
    {
        int duration = 0;

        if( mIntroList == null || mIntroList.isEmpty() ) return duration;

        for(nexTemplateEffect te : mIntroList )
        {
            if(     te.getSource_type().equals("res_video") ||
                    te.getSource_type().equals("res_image") ||
                    te.getSource_type().equals("res_solid") )
                continue;

            duration += te.getDuration();
        }
        return duration;
    }

    int getIntroEffectCount()
    {
        int count = 0;

        for( nexTemplateEffect te : mIntroList )
        {
            if(     te.getSource_type().equals("res_video") ||
                    te.getSource_type().equals("res_image") ||
                    te.getSource_type().equals("res_solid") )
                continue;
            count++;
        }
        return count;
    }

    int getLoopDuration()
    {
        int duration = 0;

        if( mLoopList == null || mLoopList.isEmpty() ) return duration;

        for(nexTemplateEffect te : mLoopList )
        {
            if(     te.getSource_type().equals("res_video") ||
                    te.getSource_type().equals("res_image") ||
                    te.getSource_type().equals("res_solid") )
                continue;

            duration += te.getDuration();
        }
        return duration;
    }

    int getOutroDuration()
    {
        int duration = 0;

        if( mAlternativeOutro != null && mAlternativeOutro.size() > 0 ) {
            Iterator<ArrayList<nexTemplateEffect>> iter = mAlternativeOutro.values().iterator();
            if( iter.hasNext() )
            {
                ArrayList<nexTemplateEffect> templateEffects = iter.next();
                for( nexTemplateEffect te : templateEffects )
                {
                    if(     te.getSource_type().equals("res_video") ||
                            te.getSource_type().equals("res_image") ||
                            te.getSource_type().equals("res_solid") )
                        continue;

                    duration += te.getDuration();
                }
                return duration;
            }
        }

        if( mOutroList == null || mOutroList.isEmpty() ) return duration;

        for( nexTemplateEffect te : mOutroList )
        {
            if(     te.getSource_type().equals("res_video") ||
                    te.getSource_type().equals("res_image") ||
                    te.getSource_type().equals("res_solid") )
                continue;

            duration += (te.getDuration() + te.getTransition_duration());
        }
        return duration;
    }

    int getSingleDuration()
    {
        int duration = 0;

        if( mSingleList == null || mSingleList.isEmpty() ) return duration;

        for( nexTemplateEffect te : mSingleList )
        {
            if(     te.getSource_type().equals("res_video") ||
                    te.getSource_type().equals("res_image") ||
                    te.getSource_type().equals("res_solid") )
                continue;

            duration += (te.getDuration() + te.getTransition_duration());
        }
        return duration;
    }

    int getOutroEffectCount()
    {
        int count = 0;

        for( nexTemplateEffect te : mOutroList )
        {
            if(     te.getSource_type().equals("res_video") ||
                    te.getSource_type().equals("res_image") ||
                    te.getSource_type().equals("res_solid") )
                continue;
            count++;
        }
        return count;
    }

    int getIntroNeedSourceCount()
    {
        int count = 0;

        for( nexTemplateEffect te : mIntroList )
        {
            count += te.getEffectDrawInfoCount(true);
        }
        return count;
    }

    int getOutroNeedSourceCount()
    {
        int count = 0;

        if( mAlternativeOutro != null && mAlternativeOutro.size() > 0 )
        {
            ArrayList<nexTemplateEffect> outroList = mAlternativeOutro.values().iterator().next();

            for( nexTemplateEffect te : outroList )
            {
                count += te.getEffectDrawInfoCount(true);
            }
            return count;
        }

        for( nexTemplateEffect te : mOutroList )
        {
            if(     te.getSource_type().equals("res_video") ||
                    te.getSource_type().equals("res_image") ||
                    te.getSource_type().equals("res_solid") )
                continue;

            count += te.getEffectDrawInfoCount(true);
        }
        return count;
    }

    nexTemplateEffect getTemplateEffectWithID(int id)
    {
        for( nexTemplateEffect te : mIntroList )
        {
            if( id == te.id )
                return te;
        }
        for( nexTemplateEffect te : mLoopList )
        {
            if( id == te.id )
                return te;
        }
        for( nexTemplateEffect te : mOutroList )
        {
            if( id == te.id )
                return te;
        }

        return null;
    }


    int getDefaultImageDurationOfTemplate()
    {
        return template_default_image_duration;
    }

    String checkAssetExist()
    {
        Stopwatch watch = new Stopwatch();
        watch.start();

        for( nexTemplateEffect te : mIntroList )
        {
            if( !te.getEffect_id().equals("none") )
            {
                if (AssetPackageManager.getInstance().getInstalledItemById(te.getEffect_id()) == null)
                    return "Into Effect not installed : " + te.getEffect_id();
            }

            if( !te.getTransition_id().equals("none") ) {
                if (AssetPackageManager.getInstance().getInstalledItemById(te.getTransition_id()) == null)
                    return "Intro Effect not installed : " + te.getTransition_id();
            }
        }

        for( nexTemplateEffect te : mLoopList )
        {
            if( !te.getEffect_id().equals("none") )
            {
                if (AssetPackageManager.getInstance().getInstalledItemById(te.getEffect_id()) == null)
                    return "Loop Effect not installed : " + te.getEffect_id();
            }

            if( !te.getTransition_id().equals("none") ) {
                if (AssetPackageManager.getInstance().getInstalledItemById(te.getTransition_id()) == null)
                    return "Loop Effect not installed : " + te.getTransition_id();
            }
        }

        for( nexTemplateEffect te : mOutroList )
        {
            if( !te.getEffect_id().equals("none") )
            {
                if (AssetPackageManager.getInstance().getInstalledItemById(te.getEffect_id()) == null)
                    return "Outro Effect not installed : " + te.getEffect_id();
            }

            if( !te.getTransition_id().equals("none") ) {
                if (AssetPackageManager.getInstance().getInstalledItemById(te.getTransition_id()) == null)
                    return "Outro Effect not installed : " + te.getTransition_id();
            }
        }
        for( nexTemplateEffect te : mSingleList )
        {
            if( !te.getEffect_id().equals("none") )
            {
                if (AssetPackageManager.getInstance().getInstalledItemById(te.getEffect_id()) == null)
                    return "Single Effect not installed : " + te.getEffect_id();
            }

            if( !te.getTransition_id().equals("none") ) {
                if (AssetPackageManager.getInstance().getInstalledItemById(te.getTransition_id()) == null)
                    return "Single Effect not installed : " + te.getTransition_id();
            }
        }
        watch.stop();
        Log.d(TAG,"checkAssetExist elapsed = "+watch.toString());
        return null;
    }

    String applyBGM(nexProject project, Context appContext, Context resContext) {
        project.setBackgroundMusicPath(template_bgm);
        project.setBGMMasterVolumeScale(template_bgm_volume);
        return null;
    }

    String applySingleBGM(nexProject project, Context appContext, Context resContext) {
        project.setBackgroundMusicPath(template_single_bgm);
        project.setBGMMasterVolumeScale(template_bgm_volume);
        return null;
    }

    String updateProjectWithTemplate(nexProject project, Context appContext, Context resContext, boolean useClipSpeed, boolean overlappedTransition) {

//        String retCheck = checkAssetExist();
//        if( retCheck != null )
//            return retCheck;

        this.useClipSpeed = useClipSpeed;
        this.overlappedTransition = overlappedTransition;

        Log.d(TAG, String.format("updateProjectWithTemplate(%b %b)", useClipSpeed, overlappedTransition));

        if( template_version.startsWith("2.0") ) {
            project.setTemplateApplyMode(2);
            project.clearDrawInfo();
            return applyTemplate20_Project(project, appContext, resContext);
        }
        else if( template_version.startsWith("3.0") ) {
            project.setTemplateApplyMode(3);

            String ret = applyTemplate30_Project(project, appContext, resContext);

            if( template_project_vol_fade_in_time >= 0 )
            {
                project.setProjectAudioFadeInTime(template_project_vol_fade_in_time);
            }

            if( template_project_vol_fade_out_time >= 0 )
            {
                project.setProjectAudioFadeOutTime(template_project_vol_fade_out_time);
            }

            project.updateProject();
            applyBGM(project, appContext, resContext);
            return ret;
        }

        return "Unsupported Template version : " + template_version;
    }

    nexTemplateUnitEDL templateUnitHead = null;

    boolean calcTemplateUnit(Context context, nexProject project)
    {
        templateUnitHead = null;

        nexProject srcProject = nexProject.clone(project);

        int effect_intro_idx = 0;
        int effect_loop_idx = 0;

        nexTemplateEffect effect = null;

        int top_start = 0;
        nexClip clip = null;
        int clip_start = 0;
        int clip_duration = 0;
        nexTemplateUnitEDL top = null;

        Stack<nexClip> stackClip = new Stack<>();

        ArrayList<nexTemplateEffect> outro = mOutroList;
        if( mAlternativeOutro != null ) {
            outro = mAlternativeOutro.values().iterator().next();
        }

        nexTemplateUnitEDL outroEDLList = null;
        for( int i = 0; i < outro.size(); i++ ) {
            effect = outro.get(i);
            top = effect.makeTemplateUnit(context, 0);

            if( outroEDLList == null ) {
                outroEDLList = top;
            }
            else {
                outroEDLList.addLastNext(top);
            }
        }

        nexClip keepClip = null;
        int keepClipDur = 0;
        while(top != null ) {
            nexTemplateUnitEDL child = top.child;
            while( child != null ) {

                if( keepClip == null && srcProject.getTotalClipCount(true) >= 1 ) {
                    keepClip = srcProject.getClip(srcProject.getTotalClipCount(true) - 1, true);
                    keepClipDur = keepClip.getProjectDuration();
                    srcProject.remove(keepClip);
                }

                if( keepClip == null )
                    break;

                if( keepClip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {

                    child.empty = false;
                    child.clip = keepClip;
                    child.clip_type = 0;
                    keepClip = null;
                    keepClipDur = 0;

                    child = child.next;
                    continue;
                }
                else
                {
                    if( top.hasVideoSourceChild() ) {
                        break;
                    }

                    child.empty = false;
                    child.clip = keepClip;
                    child.clip_type = 1;

                    if( top.duration >= keepClipDur ) {
                        keepClip = null;
                        child = child.next;
                        continue;
                    }

                    keepClipDur -= top.duration;
                }
                child = child.next;
            }
            top = top.prev;
        }

        effect = null;
        clip = null;
        clip_duration = 0;
        while( mCancel == false ) {

            if( clip == null ) {
                if( srcProject.getTotalClipCount(true) > 0 ) {
                    clip = srcProject.getClip(0, true);
                    clip_duration = clip.getProjectDuration();
                    clip_start = 0;
                    srcProject.remove(clip);
                }
                else {
                    if( keepClip != null ) {
                        clip = keepClip;
                        clip_duration = keepClipDur;
                        clip_start = 0;
                        keepClip = null;

                    }
                }
            }

            if( clip == null ) {
                break;
            }

            if( effect == null ) {
                if( effect_intro_idx < mIntroList.size() ) {
                    effect = mIntroList.get(effect_intro_idx);
                    effect_intro_idx++;
                }
                else if( effect_loop_idx < mLoopList.size() ) {
                    effect = mLoopList.get(effect_loop_idx);
                    effect_loop_idx = effect_loop_idx + 1 >= mLoopList.size() ? 0 : effect_loop_idx + 1;
                }

                top = effect.makeTemplateUnit(context, top_start);
                top.alternative = effect.alternative_id;

                if( templateUnitHead == null )
                    templateUnitHead = top;
                else
                    templateUnitHead.addLastNext(top);
            }

            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {

                nexTemplateUnitEDL emptySub = top.getFirstEmptyChildUnit();
                if( emptySub == null ) {
                    effect = null;
                    top_start += top.duration;
                    continue;
                }
                emptySub.empty = false;
                emptySub.clip_type = 0;
                emptySub.clip = clip;
                clip = null;
                continue;
            }
            else
            {
                nexTemplateUnitEDL emptySub = top.getEmptyChildUnit();
                if( emptySub == null || top.hasVideoSourceChild()) {
                    effect = null;
                    top_start += top.duration;
                    continue;
                }

                nexTemplateUnitEDL tmpTop = top.prev;
                int tmpTopDur = 0;
                while( tmpTop != null && tmpTopDur < (clip_duration / 2) ) {
                    if( tmpTop.hasVideoSourceChild() ) {
                        tmpTop =  tmpTop.next;
                        break;
                    }

                    tmpTopDur += tmpTop.duration;
                    nexTemplateUnitEDL sub = tmpTop.child;
                    while( sub != null ) {
                        if( sub.clip_type == 0 ) {
                            stackClip.push(sub.clip);
                            sub.clip = null;
                            sub.empty = true;
                        }
                        sub =  sub.next;
                    }

                    if( tmpTop.prev == null )
                        break;

                    tmpTop = tmpTop.prev;
                }

                while( tmpTop != top ) {
                    nexTemplateUnitEDL sub = tmpTop.child;

                    sub.empty = false;
                    sub.clip_type = 1;
                    sub.clip = clip;

                    sub.clip_start = clip_start;
                    sub.clip_end = clip_start + top.duration;

                    clip_duration -= tmpTop.duration;
                    clip_start += tmpTop.duration;

                    sub = sub.next;
                    while( sub != null ) {
                        if( sub.empty ) {
                            sub.clip = stackClip.pop();
                            sub.empty = false;
                        }
                        sub =  sub.next;
                    }
                    tmpTop = tmpTop.next;
                }

                nexTemplateUnitEDL sub = top.child;
                boolean bindClip = false;
                while( sub != null ) {
                    if( sub.empty ) {
                        if( bindClip == false ) {
                            sub.empty = false;
                            sub.clip_type = 1;
                            sub.clip = clip;

                            sub.clip_start = clip_start;
                            sub.clip_end = clip_start + top.duration;

                            bindClip = true;
                            sub =  sub.next;
                            continue;
                        }

                        if( stackClip.size() > 0 ) {
                            sub.clip = stackClip.pop();
                            sub.clip_type = 0;
                            sub.empty = false;
                        }
                        else {
                            sub.clip = clip;
                            sub.clip_type = 1;
                        }
                    }
                    sub =  sub.next;
                }

                if( top.duration >= clip_duration ) {
                    clip_duration = 0;
                    clip_start = 0;
                    clip = null;

                    effect = null;
                    top_start += top.duration;
                    continue;
                }

                clip_duration -= top.duration;
                clip_start += top.duration;

                effect = null;
                top_start += top.duration;
            }
        }

        templateUnitHead.addLastNext(outroEDLList);
        // templateUnitHead.calcTime(0);
        templateUnitHead.printInfo();

        return true;
    }

    boolean prepareTemplateUnit(Context context, nexProject project)
    {
        templateUnitHead = null;

        nexProject srcProject = nexProject.clone(project);

        int effect_intro_idx = 0;
        int effect_loop_idx = 0;

        nexTemplateEffect effect = null;

        int top_start = 0;
        nexClip clip = null;
        int clip_start = 0;
        int clip_duration = 0;
        nexTemplateUnitEDL top = null;

        Stack<nexClip> stackClip = new Stack<>();

        // Apply Intro and Loop
        effect = null;
        clip = null;
        clip_duration = 0;
        while( mCancel == false ) {

            if( clip == null ) {
                if( srcProject.getTotalClipCount(true) > 0 ) {
                    clip = srcProject.getClip(0, true);
                    clip_duration = clip.getProjectDuration();
                    clip_start = 0;
                    srcProject.remove(clip);
                }
            }

            if( clip == null ) {
                break;
            }

            if( effect == null ) {
                if( effect_intro_idx < mIntroList.size() ) {
                    effect = mIntroList.get(effect_intro_idx);
                    effect_intro_idx++;
                }
                else if( effect_loop_idx < mLoopList.size() ) {
                    effect = mLoopList.get(effect_loop_idx);
                    effect_loop_idx = effect_loop_idx + 1 >= mLoopList.size() ? 0 : effect_loop_idx + 1;
                }

                top = effect.makeTemplateUnit(context, top_start);

                if( templateUnitHead == null )
                    templateUnitHead = top;
                else
                    templateUnitHead.addLastNext(top);
            }

            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                nexTemplateUnitEDL emptySub = top.findEmptyChildUnit(clip.getProjectStartTime());
                if( emptySub == null ) {
                    effect = null;
                    top_start += (top.duration - top.transition_duration);
                    continue;
                }
                emptySub.empty = false;
                emptySub.clip_type = 0;
                emptySub.clip = clip;
                clip = null;
                continue;
            }
            else
            {
                if( top.hasEmptyChild() == false || top.isAvailableVideo2Top(clip) == false ) {
                    effect = null;
                    top_start += (top.duration - top.transition_duration);
                    continue;
                }

                nexTemplateUnitEDL tmpTop = top.prev;
                int tmpTopDur = 0;
                while( tmpTop != null && tmpTopDur < (clip.getProjectDuration() / 2) ) {
                    if( tmpTop.isAvailableVideo2Top(clip) == false ) {
                        tmpTop =  tmpTop.next;
                        break;
                    }

                    tmpTopDur += tmpTop.duration;
                    nexTemplateUnitEDL sub = tmpTop.child;
                    while( sub != null ) {
                        if( sub.clip_type == 0 ) {
                            stackClip.push(sub.clip);
                            sub.clip = null;
                            sub.empty = true;
                        }
                        sub =  sub.next;
                    }

                    if( tmpTop.prev == null || tmpTopDur > (clip.getProjectDuration() / 2))
                        break;

                    tmpTop = tmpTop.prev;
                }

                while( tmpTop != null && tmpTop != top ) {
                    nexTemplateUnitEDL sub = tmpTop.getEmptyChildUnit();

                    if( sub == null ) {
                        tmpTop = tmpTop.next;
                        continue;
                    }

                    sub.empty = false;
                    sub.clip_type = 1;
                    sub.clip = clip;

                    sub.clip_start = clip_start;
                    sub.clip_end = clip_start + top.duration;

                    clip_start += (tmpTop.duration - tmpTop.transition_duration);
                    clip_duration -= (tmpTop.duration - tmpTop.transition_duration);

                    sub = sub.next;
                    while( sub != null ) {
                        if( sub.empty ) {
                            if( stackClip.size() > 0 ) {
                                sub.empty = false;
                                sub.clip_type = 0;
                                sub.clip = stackClip.pop();
                            }
                            else {
                                sub.clip = clip;
                                sub.clip_type = 1;
                            }
                        }
                        sub =  sub.next;
                    }
                    tmpTop = tmpTop.next;
                }

                nexTemplateUnitEDL sub = top.getEmptyChildUnit();
                if( sub != null ) {
                    sub.empty = false;
                    sub.clip_type = 1;
                    sub.clip = clip;

                    sub.clip_start = clip_start;
                    sub.clip_end = clip_start + top.duration;

                    clip_start += (top.duration - top.transition_duration);
                    clip_duration -= (top.duration - top.transition_duration);

                    sub =  sub.next;
                    while( sub != null ) {
                        if( sub.empty ) {
                            if( stackClip.size() > 0 ) {
                                sub.empty = false;
                                sub.clip_type = 0;
                                sub.clip = stackClip.pop();
                            }
                            else {
                                sub.clip = clip;
                                sub.clip_type = 1;
                            }
                        }
                        sub =  sub.next;
                    }
                }

                if( clip_duration <= 500 ) {
                    clip_duration = 0;
                    clip_start = 0;
                    clip = null;
                }

                effect = null;
                top_start += (top.duration - top.transition_duration);
            }
        }

        // Apply Outro
        int outroSourceCount = getOutroNeedSourceCount();
        stackClip.clear();
        if( mCancel == false ) {
            nexTemplateUnitEDL tmpTop = top;
            while( tmpTop != null && stackClip.size() < outroSourceCount ) {

                nexTemplateUnitEDL sub = tmpTop.child;
                while( sub != null && stackClip.size() < outroSourceCount ) {
                    if( sub.empty == false && (sub.clip_type == 0 || sub.clip_type == 1) ) {
                        stackClip.push(sub.clip);
                        sub.clip = null;
                        sub.empty = true;
                    }
                    sub =  sub.next;
                }

                if( tmpTop.prev == null || tmpTop.isAllChildEmpty() == false )
                    break;

                top_start = tmpTop.start;
                tmpTop = tmpTop.prev;
                tmpTop.next = null;
            }

            ArrayList<nexTemplateEffect> outro = mOutroList;
            if( mAlternativeOutro != null ) {
                outro = mAlternativeOutro.get(tmpTop.alternative);
            }

            for(nexTemplateEffect e : outro ) {
                top = e.makeTemplateUnit(context, top_start);
                tmpTop.addLastNext(top);

                nexTemplateUnitEDL sub = top.getEmptyChildUnit();
                nexClip c = null;
                while( sub != null ) {
                    if (sub.empty ) {
                        if( stackClip.size() > 0 ) {
                            c = stackClip.pop();
                            sub.empty = false;
                            sub.clip = c;
                            sub.clip_type = c.getClipType() == nexClip.kCLIP_TYPE_IMAGE ? 0 : 1;
                        }
                        else {
                            sub.clip = c;
                            sub.clip_type = c.getClipType() == nexClip.kCLIP_TYPE_IMAGE ? 0 : 1;
                        }
                    }
                    sub = sub.next;
                }
                top_start += (top.duration - top.transition_duration);
            }
        }

        // templateUnitHead.addLastNext(outroEDLList);
        // templateUnitHead.calcTime(0);
        templateUnitHead.printInfo();

        return true;
    }

    boolean hasImageClip(nexProject tmpProject){
        for(int i =0; i<tmpProject.getTotalClipCount(true); i++ ) {
            nexClip clip = tmpProject.getClip(i, true);
            if (clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                return true;
            }
        }
        return false;
    }

    boolean loop_end = false;
    String applyTemplate20_Project(nexProject project, Context appContext, Context resContext) {

        int projectTotalTime = project.getTotalTime();

        Log.d(TAG, String.format("applyTemplate20_Project ( ProjectTime:%d TemplateMinDur:%d)", projectTotalTime, getTemplateMinDuration()));

        nexProject tmpProject = nexProject.clone(project);
        project.allClear(true);

        int outroCount = getOutroEffectCount();
        int outroDur = getOutroDuration();
        ArrayList<nexClip> clipList = new ArrayList<>();
        if(!mSingleList.isEmpty()) {
            if((tmpProject.getTotalClipCount(true) == 1 && tmpProject.getClip(0, true).getClipType() == nexClip.kCLIP_TYPE_IMAGE)
            ||((tmpProject.getTotalTime() < getSingleDuration()) && hasImageClip(tmpProject) == false)){
                Log.d(TAG, String.format("Template Apply Single Start(%d %d)", project.getTotalClipCount(true), tmpProject.getTotalClipCount(true)));
                String ret = setProjectWithEffects(tmpProject, project, appContext, mSingleList, true);
                if( ret != null )
                {
                    return ret;
                }
                project.updateTimeLine(false);

                if( template_single_project_vol_fade_in_time >= 0 )
                {
                    project.setProjectAudioFadeInTime(template_single_project_vol_fade_in_time);
                }

                if( template_single_project_vol_fade_out_time >= 0 )
                {
                    project.setProjectAudioFadeOutTime(template_single_project_vol_fade_out_time);
                }

                project.updateProject();
                if(template_single_bgm != null) {
                    applySingleBGM(project, appContext, resContext);
                }else{
                    applyBGM(project, appContext, resContext);
                }
                return ret;
            }
        }

        if( template_single_video == 1 ) {
        }
        else {
            for (int i = tmpProject.getTotalClipCount(true) - 1; i >= 0; i--) {
                if (outroDur < 800 || outroCount <= 0)
                    break;

                nexClip clip = tmpProject.getClip(i, true);
                if (clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                    clipList.add(0, clip);
                    tmpProject.remove(clip);
                    outroDur -= clip.getProjectDuration();
                    outroCount--;
                    continue;
                }

                if (clip.getProjectDuration() <= outroDur) {
                    clipList.add(0, clip);
                    tmpProject.remove(clip);
                    outroDur -= clip.getProjectDuration();
                    continue;
                }

                //NESA-999
                int remainTime = projectTotalTime - outroDur;
                if (mIntroList.get(0).getDuration_min() > remainTime) {
                    clipList.add(0, clip);
                    tmpProject.remove(clip);
                    continue;
                }

                int speed_clip = clip.getVideoClipEdit().getSpeedControl();
                nexClip clip2 = nexClip.dup(clip);
                tmpProject.add(i, true, clip2);
                clip2.getVideoClipEdit().setSpeedControl(speed_clip);
                clip2.setRotateDegree(clip.getRotateDegree());

                int realTime = outroDur;
                if (speed_clip != 100)
                    realTime = realTime * speed_clip / 100;

                int iStartTrim = clip.getVideoClipEdit().getStartTrimTime();
                int iEndTrim = clip.getVideoClipEdit().getEndTrimTime();

                clip2.getVideoClipEdit().setTrim(iStartTrim, iEndTrim - realTime);
                clip.getVideoClipEdit().setTrim(iEndTrim - realTime, iEndTrim);

                Log.d(TAG, String.format("Template Apply 1(%d %d %d) 2(%d %d %d)",
                        clip2.getProjectDuration(),
                        clip2.getVideoClipEdit().getStartTrimTime(),
                        clip2.getVideoClipEdit().getEndTrimTime(),
                        clip.getProjectDuration(),
                        clip.getVideoClipEdit().getStartTrimTime(),
                        clip.getVideoClipEdit().getEndTrimTime()));

                clipList.add(0, clip);
                tmpProject.remove(clip);
                break;
            }
        }

        // update intro
        Log.d(TAG, String.format("Template Apply Intro Start(%d %d)", project.getTotalClipCount(true), tmpProject.getTotalClipCount(true)));

        String ret = setProjectWithEffects(tmpProject, project, appContext, mIntroList, false);
        if( ret != null )
        {
            return ret;
        }

        Log.d(TAG, String.format("Template Apply Intro End(%d %d)", project.getTotalClipCount(true), tmpProject.getTotalClipCount(true)));

        while( tmpProject.getTotalClipCount(true) > 0 && loop_end == false )
        {
            ret = setProjectWithEffects(tmpProject, project, appContext, mLoopList, false);
            if( ret != null )
            {
                return ret;
            }
            if( mCancel ){
                Log.d(TAG,"cancel template");
                return "cancel template";
            }
        }

        Log.d(TAG, String.format("Template Apply Loop End(%d %d)", project.getTotalClipCount(true), tmpProject.getTotalClipCount(true)));

        while( clipList.size() > 0 ) {
            nexClip clip = clipList.get(0);
            tmpProject.add(clip);
            clipList.remove(0);
        }

        Log.d(TAG, String.format("Template Apply Outpro Start(%d %d)", project.getTotalClipCount(true), tmpProject.getTotalClipCount(true)));
        ret = setProjectWithEffects(tmpProject, project, appContext, mOutroList, true);
        if( ret != null )
        {
            return ret;
        }

        Log.d(TAG, String.format("Template Apply Outro End(%d %d)", project.getTotalClipCount(true), tmpProject.getTotalClipCount(true)));

        project.updateTimeLine(false);

        if( template_project_vol_fade_in_time >= 0 )
        {
            project.setProjectAudioFadeInTime(template_project_vol_fade_in_time);
        }

        if( template_project_vol_fade_out_time >= 0 )
        {
            project.setProjectAudioFadeOutTime(template_project_vol_fade_out_time);
        }

        project.updateProject();
        if( !overlappedTransition ) {
            int clipTotal = project.getTotalClipCount(true);
            for(int i = 0; i < clipTotal; i++) {
                if(project.getClip(i, true).getClipEffect().getId().endsWith(".force_effect")){
                    Rect rect = new Rect();
                    project.getClip(i-1, true).getCrop().getEndPositionRaw(rect);
                    project.getClip(i, true).getCrop().setStartPositionRaw(rect);

                    if( i+1 < clipTotal ) {
                        project.getClip(i + 1, true).getCrop().getStartPositionRaw(rect);
                        project.getClip(i, true).getCrop().setEndPositionRaw(rect);
                    }
                }
            }
        }
        applyBGM(project, appContext, resContext);

        return ret;
    }

    String setProjectWithEffects(nexProject srcProject, nexProject dstProject, Context appContext, ArrayList<nexTemplateEffect> templateEffects, boolean end)
    {
        //Log.d(TAG, String.format("Template setProjectWithEffects remain_cnt(%d)", srcProject.getTotalClipCount(true)));

        int outroDuration = getOutroDuration();

        int iEffectSize = templateEffects.size();
        for(int i = 0; i < iEffectSize; i++ ) {

            nexTemplateEffect te = templateEffects.get(i);

            int transition_dur  = te.getTransition_duration();
            int effect_dur      = te.getDuration();
            int effect_min_dur  = te.getDuration_min();
            int effect_max_dur  = te.getDuration_max();

            int effect_transition_time = effect_dur + transition_dur;
            int effect_transition_min_time = effect_min_dur + transition_dur;
            int effect_transition_max_time = effect_max_dur + transition_dur;

            effect_transition_max_time = effect_transition_max_time <= effect_transition_time ? effect_transition_time : effect_transition_max_time;

            boolean support_resources_on_effect = te.isApply_effect_on_res();
            int spd = te.crop_speed;
            int clip_list_size = srcProject.getTotalClipCount(true);
            if (clip_list_size <= 0 && !end) {
                Log.d(TAG, String.format("setProjectWithEffects clip apply end"));
                break;
            }

            if (te.addResource(dstProject, appContext, template_ratio)) {
                if (!end) {
                    lastEffectIdentifier = te.alternative_id;
                    // Log.d(TAG, "Template save identifier " + lastEffectIdentifier);
                }
                else {
                    nexClip c = dstProject.getClip(dstProject.getTotalClipCount(true) - 1, true);
                    if( c != null ) {
                        te.alternativeEffect(dstProject, c, lastEffectIdentifier, end);
                    }
                }
                continue;
            }

            if (clip_list_size <= 0) {
                Log.d(TAG, String.format("setProjectWithEffects clip apply end"));
                break;
            }

            if( template_single_video == 1 && end == false )
            {
                if( (srcProject.getClip(0, true).getProjectDuration() - effect_transition_time) < outroDuration )
                {
                    loop_end = true;
                    Log.d(TAG, String.format("setProjectWithEffects clip apply end for outro on single video"));
                    break;
                }
            }

            if (!end) {
                lastEffectIdentifier = te.alternative_id;
                // Log.d(TAG, "Template save identifier " + lastEffectIdentifier);
            }

            //Log.d(TAG, String.format("Template setProjectWithEffects clip size(%d)", clip_list_size));

            int effect_remain_time = effect_transition_time;

            while( srcProject.getTotalClipCount(true) > 0 ) {

                nexClip clip = srcProject.getClip(0, true);
                int clip_dur = clip.getProjectDuration();

                if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                    clip.getCrop().setFaceDetectSpeed(spd);
                    if(clip.getCrop().getEndPosionLock()){
                        Rect startRect = new Rect();
                        Rect endRect = new Rect();
                        clip.getCrop().getStartPosition(startRect);
                        clip.getCrop().getEndPosition(endRect);

                        clip.getCrop().applyCropSpeed(startRect, endRect, clip.getWidth(), clip.getHeight(), spd, clip_dur);
                        clip.getCrop().shrinkToAspect(startRect, nexApplicationConfig.getAspectRatio());
                        clip.getCrop().shrinkToAspect(endRect, nexApplicationConfig.getAspectRatio());

                        clip.getCrop().setStartPosition(startRect);
                        clip.getCrop().setEndPosition(endRect);
                    }
                    if( clip_dur <= effect_transition_time ) {
                        if( clip_dur >= effect_transition_min_time ) {
                            srcProject.remove(clip);
                            dstProject.add(clip);

                            te.applyEffect("0", clip, true, template_ratio, useClipSpeed);
                            te.applySoundEffect2Clip(dstProject, clip);
                            te.alternativeEffect(dstProject, clip, lastEffectIdentifier, end);
                            validateTransition(dstProject);
                            break;
                        }

                        if(     effect_transition_time <= 5000 ||
                                srcProject.getTotalClipCount(true) == 1 ||
                                support_resources_on_effect == false )
                        {
                            clip.setImageClipDuration(effect_remain_time);
                            if(clip.getCrop().getEndPosionLock()){
                                Rect startRect = new Rect();
                                Rect endRect = new Rect();
                                clip.getCrop().getStartPosition(startRect);
                                clip.getCrop().getEndPosition(endRect);

                                clip.getCrop().applyCropSpeed(startRect, endRect, clip.getWidth(), clip.getHeight(), spd, clip.getImageClipDuration());
                                clip.getCrop().shrinkToAspect(startRect, nexApplicationConfig.getAspectRatio());
                                clip.getCrop().shrinkToAspect(endRect, nexApplicationConfig.getAspectRatio());

                                clip.getCrop().setStartPosition(startRect);
                                clip.getCrop().setEndPosition(endRect);
                            }
                            srcProject.remove(clip);
                            dstProject.add(clip);

                            te.applyEffect("0", clip, true, template_ratio, useClipSpeed);
                            te.applySoundEffect2Clip(dstProject, clip);
                            te.alternativeEffect(dstProject, clip, lastEffectIdentifier, end);
                            validateTransition(dstProject);
                            break;
                        }

                        effect_remain_time -= clip_dur;

                        srcProject.remove(clip);
                        dstProject.add(clip);

                        if( effect_remain_time <= 0 ) {
                            te.applyEffect("1", clip, true, template_ratio, useClipSpeed);
                            te.applySoundEffect2Clip(dstProject, clip);
                            te.alternativeEffect(dstProject, clip, lastEffectIdentifier, end);
                            validateTransition(dstProject);
                            break;
                        }

                        te.applyEffect("1", clip, false, template_ratio, useClipSpeed);
                        te.applySoundEffect2Clip(dstProject, clip);
                        te.alternativeEffect(dstProject, clip, lastEffectIdentifier, end);

                        continue;
                    }

                    clip.setImageClipDuration(effect_transition_time);
                    if(clip.getCrop().getEndPosionLock()){
                        Rect startRect = new Rect();
                        Rect endRect = new Rect();
                        clip.getCrop().getStartPosition(startRect);
                        clip.getCrop().getEndPosition(endRect);

                        clip.getCrop().applyCropSpeed(startRect, endRect, clip.getWidth(), clip.getHeight(), spd, clip.getImageClipDuration());
                        clip.getCrop().shrinkToAspect(startRect, nexApplicationConfig.getAspectRatio());
                        clip.getCrop().shrinkToAspect(endRect, nexApplicationConfig.getAspectRatio());

                        clip.getCrop().setStartPosition(startRect);
                        clip.getCrop().setEndPosition(endRect);
                    }
                    srcProject.remove(clip);
                    dstProject.add(clip);

                    te.applyEffect("0", clip, true, template_ratio, useClipSpeed);
                    te.applySoundEffect2Clip(dstProject, clip);
                    te.alternativeEffect(dstProject, clip, lastEffectIdentifier, end);
                    validateTransition(dstProject);
                    break;
                }

                if( clip_dur < effect_transition_min_time ) {

                    if( te.useParticleVideo() == false ) {
                        srcProject.remove(clip);
                        continue;
                    }
                    srcProject.remove(clip);
                    dstProject.add(clip);

                    te.applyEffect("2", clip, false, template_ratio, useClipSpeed);
                    te.applySoundEffect2Clip(dstProject, clip);
                    te.alternativeEffect(dstProject, clip, lastEffectIdentifier, end);
                    // validateTransition(dstProject);

                    if( !overlappedTransition ) {
                        clip.getVideoClipEdit().setSpeedControl(100);
                        if( srcProject.getTotalClipCount(true) <= 0 ) {
                            clip.setTemplateEffectID(clip.getTemplateEffectID() & 0x01000000);
                        }
                    }

                    effect_transition_min_time -= clip_dur;
                    effect_transition_max_time -= clip_dur;
                    effect_transition_time -= clip_dur;
                    continue;
                }

                if( clip_dur <= effect_transition_max_time ) {

                    srcProject.remove(clip);
                    dstProject.add(clip);

                    te.applyEffect("3", clip, true, template_ratio, useClipSpeed);
                    te.applySoundEffect2Clip(dstProject, clip);
                    te.alternativeEffect(dstProject, clip, lastEffectIdentifier, end);
                    validateTransition(dstProject);

                    if (!overlappedTransition) {
                        clip.getVideoClipEdit().setSpeedControl(100);
                        changeTransition2Effect(dstProject);
                    }
                    break;
                }

                int j = i+1 >= iEffectSize ? 0 : i+1;
                nexTemplateEffect nexEffect = templateEffects.get(j);

                int realTime = effect_transition_time;

                int remainTime = clip_dur - effect_transition_time;
                int remainMinTime = clip_dur - effect_transition_min_time;
                int nextEffectMinTime = nexEffect.getDuration_min() + nexEffect.getTransition_duration();

                if( remainTime <= nextEffectMinTime && !nexEffect.isResource() && te.useParticleVideo() ) {

                    if( (remainMinTime >= nextEffectMinTime ) ) {
                        realTime = effect_transition_min_time;
                    }
                    else
                    {
                        srcProject.remove(clip);
                        dstProject.add(clip);

                        te.applyEffect("3", clip, true, template_ratio, useClipSpeed);
                        te.applySoundEffect2Clip(dstProject, clip);
                        te.alternativeEffect(dstProject, clip, lastEffectIdentifier, end);
                        validateTransition(dstProject);

                        if (!overlappedTransition) {
                            clip.getVideoClipEdit().setSpeedControl(100);
                            changeTransition2Effect(dstProject);
                        }
                        break;
                    }
                }

                int speed_clip = clip.getVideoClipEdit().getSpeedControl();
                nexClip clip2 = nexClip.dup(clip);
                dstProject.add(clip2);

                clip2.setRotateDegree(clip.getRotateDegree());
                clip2.getVideoClipEdit().setSpeedControl(speed_clip);

                if (speed_clip != 100)
                    realTime = realTime * speed_clip / 100;

                clip2.getVideoClipEdit().setTrim(clip.getVideoClipEdit().getStartTrimTime(), clip.getVideoClipEdit().getStartTrimTime() + realTime);
                clip.getVideoClipEdit().setTrim(clip.getVideoClipEdit().getStartTrimTime() + realTime, clip.getVideoClipEdit().getEndTrimTime());
                te.applyEffect("4", clip2, true, template_ratio, useClipSpeed);
                te.applySoundEffect2Clip(dstProject, clip2);
                te.alternativeEffect(dstProject, clip2, lastEffectIdentifier, end);

                if( !overlappedTransition ) {
                    clip.getVideoClipEdit().setSpeedControl(100);
                    clip2.getVideoClipEdit().setSpeedControl(100);
                    changeTransition2Effect(dstProject);
                }
                else {
                    validateTransition(dstProject);
                }
                break;
            }
        }
        return null;
    }

    String applyTemplate30_Project(nexProject project, Context appContext, Context resContext) {

        int projectTotalTime = project.getTotalTime();

        Log.d(TAG, String.format("applyTemplate30_Project ( ProjectTime:%d TemplateMinDur:%d)", projectTotalTime, getTemplateMinDuration()));

        int imageCount = 0;
        int videoCount = 0;
        int videoMaxDuration = 0;
        int videoMemorySize = 0;

        for(int i = 0; i < project.getTotalClipCount(true); i++ ) {
            nexClip clip = project.getClip(i, true);
            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                imageCount++;
            }
            else {
                videoMaxDuration = videoMaxDuration < clip.getProjectDuration() ? clip.getProjectDuration() : videoMaxDuration;
                videoMemorySize += (clip.getWidth()*clip.getHeight());
                videoCount++;
            }
        }

        Log.d(TAG, String.format("applyTemplate30_Project(I:%d V:%d)", imageCount, videoCount));

        if( imageCount <= 0 && videoCount <= 0 ) {
            return "Not support project on 3.0 template";
        }

        if( videoCount <= 0 ) {
            nexProject imageProject = nexProject.clone(project);

            project.allClear(true);
            project.getTopDrawInfo().clear();

            String ret = makeProject30WithOnlyImages(imageProject, project, appContext, resContext);

            project.updateTimeLine(false);

            Log.d(TAG, "Template3.0 make Project end with only images: " + ret);
            return ret;
        }

        nexProject srcProject = nexProject.clone(project);

        project.allClear(true);
        project.getTopDrawInfo().clear();

        int srcCount = srcProject.getTotalClipCount(true);
        if( !mSingleList.isEmpty() && mVideoMemorySize >= videoMemorySize ) {

            for( nexTemplateEffect te : mSingleList ) {
                if( te.getDuration() < videoMaxDuration && mUseVideoTrim == false )
                    break;

                if( te.getEffectDrawInfoCount(true) >= srcCount ) {

                    nexClip lastClip = null;
                    List<nexDrawInfo> te_top = te.makeTopDrawInfos(0, 0, null);
                    List<nexTemplateDrawInfo> te_sub = te.getSubEffects();

                    for (nexTemplateDrawInfo sub : te_sub) {
                        if( sub.applyResouceClip(project, appContext, te, 0, template_ratio) ) {
                            continue;
                        }
                        else if( srcProject.getTotalClipCount(true) > 0 ) {
                            lastClip = srcProject.getClip(0, true);
                            srcProject.remove(lastClip);
                            project.add(lastClip);
                            lastClip.clearDrawInfos();

                            lastClip.mStartTime = Integer.MAX_VALUE;
                            lastClip.mEndTime = Integer.MIN_VALUE;
                        }
                        sub.setDrawInfo2Clip(lastClip, te.id, te.getTopEffectDuration(), 0, template_ratio, null, true);
                    }

                    setTopDrawInfo2Project(project, te_top);
                    project.updateProject();
                    te.applySoundEffect2Project(project, appContext, 0, null);
                    applySingleBGM(project, appContext, resContext);
                    Log.d(TAG, String.format("Template videoProject Apply single effect(%d %d)", project.getTotalClipCount(true), srcProject.getTotalClipCount(true)));
                    return null;
                }
            }
        }

        prepareTemplateUnit(appContext, srcProject);
        if( templateUnitHead != null )
        {
            boolean result = templateUnitHead.applyTemplateUnit2Project(project, appContext, 0, template_ratio);
            if( result ) {
                Log.d(TAG, String.format("Template Apply end"));
                return null;
            }
            else {
                Log.d(TAG, String.format("Template Applying failed"));
                return "Template Applying failed";
            }

        }

        return "Template applying failed(variable content) with preprocessing fail";







//        else if( imageCount <= 0 ) {
//            // return "Not support with only videos project";
//
//            nexProject videoProject = nexProject.clone(project);
//
//            String ret = makeProject30WithOnlyVideos(videoProject, project, appContext, resContext);
//
//            project.updateTimeLine(false);
//
//            Log.d(TAG, "Template3.0 make Project end with only videos: " + ret);
//            return ret;
//        }

//        String ret = makeProject30(project);
//
//        return ret;
        // return "Not support with videos and images project";
    }

    String makeProject30WithOnlyImages(nexProject srcProject, nexProject dstProject, Context appContext, Context resContext) {

        Log.d(TAG, String.format("Template imageProject Apply Start(%d %d)", dstProject.getTotalClipCount(true), srcProject.getTotalClipCount(true)));

        if(!mSingleList.isEmpty() ) {
            int srcCount = srcProject.getTotalClipCount(true);
            for( nexTemplateEffect te : mSingleList ) {
                if( te.getEffectDrawInfoCount(true) >= srcCount ) {

                    nexClip lastClip = null;
                    List<nexDrawInfo> te_top = te.makeTopDrawInfos(0, 0, null);
                    List<nexTemplateDrawInfo> te_sub = te.getSubEffects();

                    for (nexTemplateDrawInfo sub : te_sub) {
                        if( sub.applyResouceClip(dstProject, appContext, te, 0, template_ratio) ) {
                            continue;
                        }
                        else if( srcProject.getTotalClipCount(true) > 0 ) {
                            lastClip = srcProject.getClip(0, true);
                            srcProject.remove(lastClip);
                            dstProject.add(lastClip);
                            lastClip.clearDrawInfos();

                            lastClip.mStartTime = Integer.MAX_VALUE;
                            lastClip.mEndTime = Integer.MIN_VALUE;
                        }
                        sub.setDrawInfo2Clip(lastClip, te.id, te.getTopEffectDuration(), 0, template_ratio, null, false);
                        sub.applySoundEffect(dstProject, 0, te.getTopEffectDuration());
                    }

                    setTopDrawInfo2Project(dstProject, te_top);
                    dstProject.updateProject();
                    // te.applySoundEffect2Project(dstProject, appContext, 0, null);
                    applySingleBGM(dstProject, appContext, resContext);
                    Log.d(TAG, String.format("Template imageProject Apply single effect(%d %d)", dstProject.getTotalClipCount(true), srcProject.getTotalClipCount(true)));
                    return null;
                }
            }
        }

        boolean needApplyIntro = true;
        boolean needApplyLoop = true;
        int outroNeedCount = getOutroNeedSourceCount();

        int topEffectStart = 0;

        nexClip lastClip = null;

        lastEffectIdentifier = "";

        while( srcProject.getTotalClipCount(true) > 0 && mCancel == false ) {

            if( needApplyIntro ) {

                for(int i = 0; i < mIntroList.size(); i++ ) {

                    nexTemplateEffect te = mIntroList.get(i);
                    List<nexDrawInfo> te_top = te.makeTopDrawInfos(topEffectStart, 0, null);
                    List<nexTemplateDrawInfo> te_sub = te.getSubEffects();

                    lastEffectIdentifier = te.alternative_id;

                    for (nexTemplateDrawInfo sub : te_sub) {
                        if( sub.applyResouceClip(dstProject, appContext, te, topEffectStart, template_ratio) ) {
                            continue;
                        }
                        else if (srcProject.getTotalClipCount(true) > outroNeedCount) {
                            lastClip = srcProject.getClip(0, true);
                            srcProject.remove(lastClip);
                            dstProject.add(lastClip);
                            lastClip.clearDrawInfos();

                            lastClip.mStartTime = Integer.MAX_VALUE;
                            lastClip.mEndTime = Integer.MIN_VALUE;
                        }
                        sub.setDrawInfo2Clip(lastClip, te.id, te.getTopEffectDuration(), topEffectStart, template_ratio, null, false);
                        sub.applySoundEffect(dstProject, topEffectStart, te.getTopEffectDuration());
                    }

                    // te.applySoundEffect2Project(dstProject, appContext, topEffectStart, null);
                    topEffectStart = setTopDrawInfo2Project(dstProject, te_top);
                }

                needApplyIntro = false;
                if( srcProject.getTotalClipCount(true) <= outroNeedCount ) {
                    needApplyLoop = false;
                }
                Log.d(TAG, String.format("Template imageProject Apply Intro End(%d %d) (%d)", dstProject.getTotalClipCount(true), srcProject.getTotalClipCount(true), outroNeedCount));
                continue;
            }

            if( needApplyLoop ) {

                for(int i = 0; i < mLoopList.size(); i++ ) {

                    nexTemplateEffect te = mLoopList.get(i);
                    List<nexDrawInfo> te_top = te.makeTopDrawInfos(topEffectStart, 0, null);
                    List<nexTemplateDrawInfo> te_sub = te.getSubEffects();

                    lastEffectIdentifier = te.alternative_id;

                    for (nexTemplateDrawInfo sub : te_sub) {
                        if( sub.applyResouceClip(dstProject, appContext, te, topEffectStart, template_ratio) ) {
                            continue;
                        }
                        else if( srcProject.getTotalClipCount(true) > outroNeedCount ) {
                            lastClip = srcProject.getClip(0, true);
                            srcProject.remove(lastClip);
                            dstProject.add(lastClip);
                            lastClip.clearDrawInfos();

                            lastClip.mStartTime = Integer.MAX_VALUE;
                            lastClip.mEndTime = Integer.MIN_VALUE;
                        }
                        sub.setDrawInfo2Clip(lastClip, te.id, te.getTopEffectDuration(), topEffectStart, template_ratio, null, false);
                        sub.applySoundEffect(dstProject, topEffectStart, te.getTopEffectDuration());
                    }

                    // te.applySoundEffect2Project(dstProject, appContext, topEffectStart, null);
                    topEffectStart = setTopDrawInfo2Project(dstProject, te_top);
                    if (srcProject.getTotalClipCount(true) <= outroNeedCount) {
                        needApplyLoop = false;
                        Log.d(TAG, String.format("Template imageProject Apply Loop End(%d %d) (%d)", dstProject.getTotalClipCount(true), srcProject.getTotalClipCount(true), outroNeedCount));
                        break;
                    }
                }
                continue;
            }

            ArrayList<nexTemplateEffect> outroList = mOutroList;

            if( mAlternativeOutro != null && mAlternativeOutro.size() > 0 )
            {
                if( mAlternativeOutro.containsKey(lastEffectIdentifier) ) {
                    outroList = mAlternativeOutro.get(lastEffectIdentifier);
                    Log.d(TAG, String.format("Template imageProject select Outro(%s)", lastEffectIdentifier));
                }
                else {
                    outroList = mAlternativeOutro.values().iterator().next();
                }
            }

            for(int i = 0; i < outroList.size(); i++ ) {

                nexTemplateEffect te = outroList.get(i);
                List<nexDrawInfo> te_top = te.makeTopDrawInfos(topEffectStart, 0, lastEffectIdentifier);
                List<nexTemplateDrawInfo> te_sub = te.getSubEffects();

                lastClip = null;
                for (nexTemplateDrawInfo sub : te_sub) {
                    if( sub.applyResouceClip(dstProject, appContext, te, topEffectStart, template_ratio) ) {
                        continue;
                    }
                    else if( srcProject.getTotalClipCount(true) > 0 ) {
                        lastClip = srcProject.getClip(0, true);
                        srcProject.remove(lastClip);
                        dstProject.add(lastClip);
                        lastClip.clearDrawInfos();

                        lastClip.mStartTime = Integer.MAX_VALUE;
                        lastClip.mEndTime = Integer.MIN_VALUE;
                    }
                    sub.setDrawInfo2Clip(lastClip, te.id, te.getTopEffectDuration(), topEffectStart, template_ratio, lastEffectIdentifier, false);
                    sub.applySoundEffect(dstProject, topEffectStart, te.getTopEffectDuration());
                }

                // te.applySoundEffect2Project(dstProject, appContext, topEffectStart, lastEffectIdentifier);
                topEffectStart = setTopDrawInfo2Project(dstProject, te_top);
            }
            Log.d(TAG, String.format("Template imageProject Apply Outro End(%d %d) (%d)", dstProject.getTotalClipCount(true), srcProject.getTotalClipCount(true), outroNeedCount));
            break;
        }

        if( mCancel ) {
            return "apply Template user canceled";
        }

        dstProject.updateProject();
        applyBGM(dstProject, appContext, resContext);
        return null;
    }

    List<nexDrawInfo> makeTopEffects(nexProject project) {

        int duration = project.getTotalTime();

        Log.d(TAG, String.format("Template makeTopEffects(%d)", duration));

        int topEffectStart = 0;

        boolean needApplyIntro = true;
        boolean needApplyLoop = true;

        List<nexDrawInfo> newTopEffects = new ArrayList<>();
        int outroDuration = getOutroDuration();

        if( duration <= outroDuration ) {
            needApplyIntro = false;
            needApplyLoop = false;
        }

        // make top effect;
        int idx = 0;
        int videoDur = 0;
        nexClip videoClip = null;

        List<nexDrawInfo> te_top = null;
        while( duration > 0 && mCancel == false ) {

            if( needApplyIntro ) {

                for (int i = 0; i < mIntroList.size(); i++) {

                    nexTemplateEffect te = mIntroList.get(i);

                    te_top = null;
                    while( mCancel == false ) {
                        if( videoClip == null && idx < project.getTotalClipCount(true) ) {

                            videoClip = project.getClip(idx++, true);
                            videoDur += videoClip.getProjectDuration();
                        }

                        if( videoClip == null ) break;

                        if(  videoDur > te.getDuration_max() ) {
                            if( (videoDur - te.getDuration()) < 5000  ) {
                                te_top = te.makeTopDrawInfos(topEffectStart, videoDur, null);
                                videoClip = null;
                                videoDur = 0;
                                break;
                            }

                            te_top = te.makeTopDrawInfos(topEffectStart, 0, null);
                            videoDur -= te.getDuration();
                            break;
                        }
                        else {
                            if( videoDur >= te.getDuration_min() ) {
                                te_top = te.makeTopDrawInfos(topEffectStart, videoDur, null);
                                videoClip = null;
                                videoDur = 0;
                                break;
                            }
                            videoClip = null;
                        }
                    }

                    if( te_top == null ) {
                        needApplyIntro = false;
                        continue;
                    }

                    for (nexDrawInfo in : te_top) {
                        newTopEffects.add(in);
                        if (in.getIsTransition() == 1) {
                            topEffectStart = in.getStartTime();
                        }
                        else {
                            topEffectStart = in.getEndTime();
                        }
                    }

                    duration -= (topEffectStart - te_top.get(0).getStartTime());
                }
                needApplyIntro = false;
                continue;
            }

            if (needApplyLoop) {

                for (int i = 0; i < mLoopList.size() && needApplyLoop; i++) {

                    nexTemplateEffect te = mLoopList.get(i);

                    if( duration <= outroDuration * 1.4f || (duration - te.getTopEffectDuration()) < getOutroDuration()) {
                        Log.d(TAG, String.format("Template videoProject Applying 0(%d)", duration));
                        needApplyLoop = false;
                        break;
                    }

                    te_top = null;
                    while( mCancel == false ) {
                        if( videoClip == null && idx < project.getTotalClipCount(true) ) {

                            videoClip = project.getClip(idx++, true);
                            videoDur += videoClip.getProjectDuration();
                        }

                        if( videoClip == null ) break;

                        if(  videoDur > te.getDuration_max() ) {
                            if( (videoDur - te.getDuration()) < 5000  ) {
                                te_top = te.makeTopDrawInfos(topEffectStart, videoDur, null);
                                videoClip = null;
                                videoDur = 0;
                                break;
                            }

                            te_top = te.makeTopDrawInfos(topEffectStart, 0, null);
                            videoDur -= te.getDuration();
                            break;
                        }
                        else {
                            if( videoDur >= te.getDuration_min() ) {
                                te_top = te.makeTopDrawInfos(topEffectStart, videoDur, null);
                                videoClip = null;
                                videoDur = 0;
                                break;
                            }
                            videoClip = null;
                        }
                    }

                    if( te_top == null ) {
                        needApplyLoop = false;
                        continue;
                    }

                    for( nexDrawInfo in : te_top ) {
                        newTopEffects.add(in);
                        if (in.getIsTransition() == 1) {
                            topEffectStart = in.getStartTime();
                        }
                        else {
                            topEffectStart = in.getEndTime();
                        }
                    }
                    duration -= (topEffectStart - te_top.get(0).getStartTime());
                }
                continue;
            }

            Log.d(TAG, String.format("Template videoProject Applying 2(%d)", duration));
            int outroCount = mOutroList.size();
            for (int i = 0; i < outroCount; i++) {

                nexTemplateEffect te = mOutroList.get(i);

                te_top = null;
                while( mCancel == false ) {

                    if( videoClip == null && idx < project.getTotalClipCount(true) ) {

                        videoClip = project.getClip(idx++, true);
                        videoDur += videoClip.getProjectDuration();
                    }

                    if( i == outroCount - 1 && idx < project.getTotalClipCount(true)) {
                        videoClip = null;
                        continue;
                    }

                    if( i == outroCount - 1 ) {
                        te_top = te.makeTopDrawInfos(topEffectStart, videoDur, lastEffectIdentifier);
                        videoClip = null;
                        videoDur = 0;
                        break;
                    }
                    else {
                        if(  videoDur > te.getDuration_max() ) {
                            if( (videoDur - te.getDuration()) < 5000  ) {
                                te_top = te.makeTopDrawInfos(topEffectStart, videoDur, lastEffectIdentifier);
                                videoClip = null;
                                videoDur = 0;
                                break;
                            }

                            te_top = te.makeTopDrawInfos(topEffectStart, 0, lastEffectIdentifier);
                            videoDur -= (te_top.get(0).getEndTime() - te_top.get(0).getStartTime());
                            break;
                        }
                        else {
                            if( videoDur >= te.getDuration_min() ) {
                                te_top = te.makeTopDrawInfos(topEffectStart, videoDur, lastEffectIdentifier);
                                videoClip = null;
                                videoDur = 0;
                                break;
                            }
                            videoClip = null;
                        }
                    }
                }

                if( te_top == null ) {

                    break;
                }

                for( nexDrawInfo in : te_top ) {
                    newTopEffects.add(in);
                    if (in.getIsTransition() == 1)
                        topEffectStart = in.getStartTime();
                    else
                        topEffectStart = in.getEndTime();
                }
                Log.d(TAG, String.format("Template videoProject Applying(%d %d %d)", duration, te_top.get(0).getStartTime(), te_top.get(0).getEndTime()));
                duration -= (topEffectStart - te_top.get(0).getStartTime());
            }
        }

        return newTopEffects;
    }

    String makeProject30WithOnlyVideos(nexProject srcProject, nexProject dstProject, Context appContext, Context resContext) {

        Log.d(TAG, String.format("Template videoProject Apply Start(%d)", dstProject.getTotalClipCount(true)));

        if(!mSingleList.isEmpty() ) {
            int projectVideoMemSize = 0;
            int maxVideoDuration = 0;

            for(int i = 0; i < dstProject.getTotalClipCount(true); i++ ) {
                nexClip c = dstProject.getClip(i, true);
                projectVideoMemSize += c.getWidth()*c.getHeight();
                maxVideoDuration = maxVideoDuration < c.getProjectDuration() ? c.getProjectDuration() : maxVideoDuration;
            }

            int srcCount = dstProject.getTotalClipCount(true);
            for( nexTemplateEffect te : mSingleList ) {
                if( te.getDuration() < maxVideoDuration && mUseVideoTrim == false )
                    break;

                if( mVideoMemorySize < projectVideoMemSize )
                    break;

                if( te.getEffectDrawInfoCount(true) >= srcCount ) {

                    nexClip lastClip = null;
                    List<nexDrawInfo> te_top = te.makeTopDrawInfos(0, 0, null);
                    List<nexTemplateDrawInfo> te_sub = te.getSubEffects();

                    for (nexTemplateDrawInfo sub : te_sub) {
                        if( sub.applyResouceClip(dstProject, appContext, te, 0, template_ratio) ) {
                            continue;
                        }
                        else if( srcProject.getTotalClipCount(true) > 0 ) {
                            lastClip = srcProject.getClip(0, true);
                            srcProject.remove(lastClip);
                            dstProject.add(lastClip);
                            lastClip.clearDrawInfos();

                            lastClip.mStartTime = Integer.MAX_VALUE;
                            lastClip.mEndTime = Integer.MIN_VALUE;
                        }
                        sub.setDrawInfo2Clip(lastClip, te.id, te.getTopEffectDuration(), 0, template_ratio, null, true);
                    }

                    setTopDrawInfo2Project(dstProject, te_top);
                    dstProject.updateProject();
                    te.applySoundEffect2Project(dstProject, appContext, 0, null);
                    applySingleBGM(dstProject, appContext, resContext);
                    Log.d(TAG, String.format("Template videoProject Apply single effect(%d %d)", dstProject.getTotalClipCount(true), srcProject.getTotalClipCount(true)));
                    return null;
                }
            }
        }

        nexClip lastClip = null;
        int lastClipStart = 0;
        int lastClipEnd = 0;

        for( int i = 0; i < dstProject.getTotalClipCount(true); i++ ) {
            dstProject.getClip(i, true).clearDrawInfos();
        }

        // make top effect;
        List<nexDrawInfo> topEffects = makeTopEffects(dstProject);
        for( nexDrawInfo info : topEffects )
            info.print();

        dstProject.getTopDrawInfo().clear();

        if( mCancel ) {
            return "apply Template user canceled";
        }

        for( nexDrawInfo top : topEffects ) {
            dstProject.getTopDrawInfo().add(top);

            if( top.getIsTransition() == 1 ) continue;

            if( mCancel ) {
                dstProject.allClear(true);
                dstProject.getTopDrawInfo().clear();
                return "apply Template user canceled";
            }

            nexTemplateEffect te = getTemplateEffectWithID(top.getID());
            if( te != null ) {

                for( int i = 0; i < dstProject.getTotalClipCount(true); i++ ) {
                    lastClip = dstProject.getClip(i, true);

                    lastClipStart = lastClip.getProjectStartTime();
                    lastClipEnd = lastClip.getProjectEndTime();

                    if( lastClipEnd <= top.getStartTime() || lastClipStart >= top.getEndTime() )
                        continue;

                    List<nexDrawInfo> infos = te.makeSubDrawInfos(lastClip, top.getID(), top.getEndTime() - top.getStartTime(), top.getStartTime(), template_ratio);
                    lastClip.addDrawInfos(infos);
                }
            }
        }

        if( mCancel ) {
            dstProject.allClear(true);
            dstProject.getTopDrawInfo().clear();
            return "apply Template user canceled";
        }
        Log.d(TAG, String.format("Template videoProject Apply End(%d)", dstProject.getTotalClipCount(true)));

        return null;
    }

    void copyTemplateDrawInfos(List<nexTemplateDrawInfo> src, List<nexTemplateDrawInfo> dst) {
        dst.clear();
        for( nexTemplateDrawInfo info : src ) {
            dst.add(info);
        }
        Collections.sort(dst, new Comparator<nexTemplateDrawInfo>() {
            @Override
            public int compare(nexTemplateDrawInfo s1, nexTemplateDrawInfo s2) {
                float a1 = s1.end - s1.start;
                float a2 = s2.end - s2.start;

                if( a1 < a2 ) {
                    return -1;
                }
                else if( a1 > a2 ) {
                    return 1;
                }
                return 0;
            }
        });
    }

    int setTopDrawInfo2Project(nexProject project, List<nexDrawInfo> tops) {

        int start = 0;
        for (nexDrawInfo in : tops) {
            project.getTopDrawInfo().add(in);
            if (in.getIsTransition() == 1)
                start = in.getStartTime();
            else {
                start = in.getEndTime();
            }
        }
        return start;
    }

    int setSubDrawInfo2VideoClip(int videoIdx, List<nexClip> clips, nexDrawInfo top, nexTemplateDrawInfo sub) {

        int topStart = top.getStartTime();
        int topEnd = top.getEndTime();

        while( videoIdx < clips.size() ) {
            nexClip clip = clips.get(videoIdx);

            if (clip.getProjectEndTime() <= topStart) {
                videoIdx++;
                continue;
            }

            sub.setDrawInfo2Clip(clip, top.getID(), topEnd - topStart, topStart, template_ratio, null, false);

            if (clip.getProjectEndTime() >= topEnd) {
                break;
            }

            if (videoIdx + 1 < clips.size()) {
                sub.setDrawInfo2Clip(clips.get(videoIdx + 1), top.getID(), topEnd - topStart, topStart, template_ratio, null, false);
            }
            break;
        }
        return videoIdx;
    }

    int getLastVideoIdxForSubDrawInfo(int videoIdx, List<nexClip> clips, nexDrawInfo top) {

        int topStart = top.getStartTime();

        while( videoIdx < clips.size() ) {
            nexClip clip = clips.get(videoIdx);

            if (clip.getProjectEndTime() <= topStart) {
                videoIdx++;
                continue;
            }
            break;
        }
        return videoIdx;
    }

    String makeProject30(nexProject dstProject) {

        nexProject imageProject = new nexProject();
        nexProject videoProject = new nexProject();

        int videoClipStart = 0;

        while(dstProject.getTotalClipCount(true) > 0 ) {
            nexClip clip = dstProject.getClip(0, true);
            dstProject.remove(clip);
            clip.clearDrawInfos();

            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                imageProject.add(clip);
            }
            else if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
                videoProject.add(clip);

                clip.mStartTime = videoClipStart;
                videoClipStart += clip.getProjectDuration();
                clip.mEndTime = videoClipStart;
            }
            else {
                return "Tamplate 3.0 unknown clip was included";
            }
        }

        dstProject.allClear(true);
        dstProject.getTopDrawInfo().clear();

        imageProject.updateProject();
        videoProject.updateProject();

        nexClip imageClip = null;
        nexClip videoClip = null;

        List<nexClip> tempClips = new ArrayList<>();
        int imageTotal = imageProject.getTotalClipCount(true);

        List<nexDrawInfo> topEffects = makeTopEffects(videoProject);
        dstProject.getTopDrawInfo().clear();

        int subTotal = 0;
        for( nexDrawInfo top : topEffects ) {

            if (top.getIsTransition() == 1) continue;
            subTotal += (top.getSubTemplateDrawInfos().size() - 1);
        }

        while( videoProject.getTotalClipCount(true) > 0 && mCancel == false ) {
            videoClip = videoProject.getClip(0, true);
            videoProject.remove(videoClip);
            dstProject.add(videoClip);
            tempClips.add(videoClip);
        }

        List<nexTemplateDrawInfo> te_sub = new ArrayList<>();

        if( subTotal >= imageTotal ) {

            int idx = 1;
            int skip = (subTotal / imageTotal);
            skip = skip > 1 ? skip - 1 : 1;

            int videoIdx = 0;

            for( nexDrawInfo top : topEffects ) {
                dstProject.getTopDrawInfo().add(top);

                if( top.getIsTransition() == 1 ) continue;

                if( mCancel ) {
                    dstProject.allClear(true);
                    dstProject.getTopDrawInfo().clear();
                    return "apply Template user canceled";
                }

                int topStart = top.getStartTime();
                int topEnd = top.getEndTime();

                copyTemplateDrawInfos(top.getSubTemplateDrawInfos(), te_sub);

                int te_sub_size = te_sub.size();
                for(int i = 0; i < te_sub_size; i++ ) {

                    nexTemplateDrawInfo sub = te_sub.get(i);

                    if( idx % skip == 0 && te_sub_size > 1 && i < te_sub_size -1 && imageProject.getTotalClipCount(true) > 0 ) {

                        imageClip = imageProject.getClip(0, true);
                        imageProject.remove(imageClip);
                        dstProject.add(imageClip);

                        imageClip.mStartTime = Integer.MAX_VALUE;
                        imageClip.mEndTime = Integer.MIN_VALUE;

                        sub.setDrawInfo2Clip(imageClip, top.getID(), topEnd - topStart, topStart, template_ratio, null, false);
                        idx++;
                        continue;
                    }

                    videoIdx = setSubDrawInfo2VideoClip(videoIdx, tempClips, top, sub);
                    if( i < te_sub_size -1 )
                        idx++;
                }
                te_sub.clear();
            }
            tempClips.clear();
        }
        else {
            boolean needApplyIntro = true;
            boolean needApplyLoop = true;

            int outroNeedCount = getOutroNeedSourceCount();

            if( imageProject.getTotalClipCount(true) <= outroNeedCount ) {
                needApplyIntro = false;
                needApplyLoop = false;
            }

            int topEffectStart = 0;
            int topStart = 0;
            int topEnd = 0;
            int videoIdx = 0;

            while( imageProject.getTotalClipCount(true) > 0 && mCancel == false ) {

                if( needApplyIntro ) {
                    for(int i = 0; i < mIntroList.size(); i++ ) {

                        nexTemplateEffect te = mIntroList.get(i);

                        List<nexDrawInfo> te_top = te.makeTopDrawInfos(topEffectStart, 0, null);
                        nexDrawInfo top = te_top.get(0);
                        topStart = top.getStartTime();
                        topEnd = top.getEndTime();

                        copyTemplateDrawInfos(te.getSubEffects(), te_sub);

                        int te_sub_size = te_sub.size();
                        for(int j = 0; j < te_sub_size; j++ ) {

                            nexTemplateDrawInfo sub = te_sub.get(j);

                            if( (j < te_sub_size -1 || videoIdx >= tempClips.size()) && imageProject.getTotalClipCount(true) > outroNeedCount ) {

                                imageClip = imageProject.getClip(0, true);
                                imageProject.remove(imageClip);
                                dstProject.add(imageClip);

                                imageClip.mStartTime = Integer.MAX_VALUE;
                                imageClip.mEndTime = Integer.MIN_VALUE;

                                sub.setDrawInfo2Clip(imageClip, top.getID(), topEnd - topStart, topStart, template_ratio, null, true);
                                continue;
                            }

                            if( videoIdx < tempClips.size() ) {
                                videoIdx = setSubDrawInfo2VideoClip(videoIdx, tempClips, top, sub);
                            }
                            else {
                                sub.setDrawInfo2Clip(imageClip, top.getID(), topEnd - topStart, topStart, template_ratio, null, false);
                            }
                        }
                        te_sub.clear();

                        topEffectStart = setTopDrawInfo2Project(dstProject, te_top);
                    }

                    needApplyIntro = false;
                    if (imageProject.getTotalClipCount(true) <= outroNeedCount) {
                        needApplyLoop = false;
                    }
                    Log.d(TAG, String.format("Template makeProject(image video) Apply Intro End(%d %d) (%d)", dstProject.getTotalClipCount(true), imageProject.getTotalClipCount(true), outroNeedCount));
                    continue;
                }

                lastEffectIdentifier = "";
                if( needApplyLoop ) {
                    for(int i = 0; i < mLoopList.size() && imageProject.getTotalClipCount(true) > outroNeedCount; i++ ) {

                        nexTemplateEffect te = mLoopList.get(i);

                        List<nexDrawInfo> te_top = te.makeTopDrawInfos(topEffectStart, 0, null);
                        nexDrawInfo top = te_top.get(0);
                        topStart = top.getStartTime();
                        topEnd = top.getEndTime();
                        lastEffectIdentifier = te.alternative_id;

                        copyTemplateDrawInfos(te.getSubEffects(), te_sub);

                        int te_sub_size = te_sub.size();
                        for( int j = 0; j < te_sub_size; j++ ) {

                            nexTemplateDrawInfo sub = te_sub.get(j);

                            int lastIdx= getLastVideoIdxForSubDrawInfo(videoIdx, tempClips, top);
                            if( (j < te_sub_size -1 || lastIdx >= tempClips.size()) && imageProject.getTotalClipCount(true) > outroNeedCount ) {

                                imageClip = imageProject.getClip(0, true);
                                imageProject.remove(imageClip);
                                dstProject.add(imageClip);
                                imageClip.getDrawInfos().clear();

                                imageClip.mStartTime = Integer.MAX_VALUE;
                                imageClip.mEndTime = Integer.MIN_VALUE;

                                sub.setDrawInfo2Clip(imageClip, top.getID(), topEnd - topStart, topStart, template_ratio, null, false);
                                continue;
                            }

                            if( videoIdx < tempClips.size() ) {
                                videoIdx = setSubDrawInfo2VideoClip(videoIdx, tempClips, top, sub);
                            }
                            else {
                                sub.setDrawInfo2Clip(imageClip, top.getID(), topEnd - topStart, topStart, template_ratio, null, false);
                            }
                        }
                        te_sub.clear();

                        topEffectStart = setTopDrawInfo2Project(dstProject, te_top);
                    }

                    if (imageProject.getTotalClipCount(true) <= outroNeedCount) {
                        needApplyLoop = false;
                        Log.d(TAG, String.format("Template makeProject(image video) Apply Loop End(%d %d) (%d)", dstProject.getTotalClipCount(true), imageProject.getTotalClipCount(true), outroNeedCount));
                    }
                    continue;
                }

                for(int i = 0; i < mOutroList.size(); i++ ) {

                    nexTemplateEffect te = mOutroList.get(i);

                    List<nexDrawInfo> te_top = te.makeTopDrawInfos(topEffectStart, 0, null);
                    nexDrawInfo top = te_top.get(0);
                    topStart = top.getStartTime();
                    topEnd = top.getEndTime();

                    copyTemplateDrawInfos(te.getSubEffects(), te_sub);

                    int te_sub_size = te_sub.size();
                    for(int j = 0; j < te_sub_size; j++ ) {

                        nexTemplateDrawInfo sub = te_sub.get(j);

                        int lastIdx= getLastVideoIdxForSubDrawInfo(videoIdx, tempClips, top);
                        if( (j < te_sub_size -1 || lastIdx >= tempClips.size()) && imageProject.getTotalClipCount(true) > 0 ) {

                            imageClip = imageProject.getClip(0, true);
                            imageProject.remove(imageClip);
                            dstProject.add(imageClip);

                            imageClip.mStartTime = Integer.MAX_VALUE;
                            imageClip.mEndTime = Integer.MIN_VALUE;

                            sub.setDrawInfo2Clip(imageClip, top.getID(), topEnd - topStart, topStart, template_ratio, lastEffectIdentifier, false);
                            continue;
                        }

                        if( videoIdx < tempClips.size() ) {
                            videoIdx = setSubDrawInfo2VideoClip(videoIdx, tempClips, top, sub);
                        }
                        else {
                            sub.setDrawInfo2Clip(imageClip, top.getID(), topEnd - topStart, topStart, template_ratio, lastEffectIdentifier, false);
                        }
                    }
                    te_sub.clear();

                    topEffectStart = setTopDrawInfo2Project(dstProject, te_top);
                }

                Log.d(TAG, String.format("Template makeProject(image video) Apply Outro End(%d %d) (%d)", dstProject.getTotalClipCount(true), imageProject.getTotalClipCount(true), outroNeedCount));
                break;
            }
        }

        if( mCancel ) {
            return "apply Template user canceled";
        }

        return null;
    }

    void validateTransition(nexProject dstProject) {
        int count = dstProject.getTotalClipCount(true);
        if( count <= 1 ) return;

        nexClip preClip = dstProject.getClip(count - 2, true);
        int preClipEffectID = preClip.getTemplateEffectID() & 0xF0FFFFFF;
        nexClip nextClip = dstProject.getClip(count - 1, true);
        int nextClipEffectID = nextClip.getTemplateEffectID() & 0xF0FFFFFF;

        if(     preClipEffectID != nextClipEffectID &&
                preClip.getTransitionEffect().getDuration() >= nextClip.getProjectDuration() - nextClip.getTransitionEffect().getDuration() ) {
            preClip.getTransitionEffect(true).setEffectNone();
            preClip.getTransitionEffect(true).setDuration(0);

        }

        if(     preClipEffectID != nextClipEffectID &&
                nextClip.getProjectDuration() <= nextClip.getTransitionEffect().getDuration() ) {
            nextClip.getTransitionEffect(true).setEffectNone();
            nextClip.getTransitionEffect(true).setDuration(0);
        }
    }

    void changeTransition2Effect(nexProject dstProject) {
        int count = dstProject.getTotalClipCount(true);
        if( count <= 0 ) return;

        nexClip clip = dstProject.getClip(count - 1, true);
        if( clip == null ) return;

        if( clip.getTransitionEffect().getId().equals("none") || clip.getTransitionEffect().getDuration() <= 0 ) return;

        int duration = clip.getTransitionEffect().getDuration();
        if( clip.getVideoClipEdit().getSpeedControl() != 100 )
            duration = duration * clip.getVideoClipEdit().getSpeedControl() / 100;

        nexClip dupClip = nexClip.dup(clip);
        dstProject.add(dupClip);

        dupClip.setRotateDegree(clip.getRotateDegree());
        dupClip.getVideoClipEdit().setSpeedControl(clip.getVideoClipEdit().getSpeedControl());
        dupClip.setColorEffect(clip.getColorEffect());
        dupClip.setBrightness(clip.getBrightness());
        dupClip.setContrast(clip.getContrast());
        dupClip.setSaturation(clip.getSaturation());
        dupClip.setTemplateEffectID(clip.getTemplateEffectID());
        dupClip.setClipVolume(clip.getClipVolume());
        dupClip.setAudioOnOff(clip.getAudioOnOff());
//        Rect rect = new Rect();
//        clip.getCrop().getStartPositionRaw(rect);
//        //dupClip.getCrop().setStartPositionRaw(rect);
//        clip.getCrop().getEndPositionRaw(rect);
//        dupClip.getCrop().setStartPositionRaw(rect);
//        dupClip.getCrop().setEndPositionRaw(rect);

        dupClip.getClipEffect().setEffect(clip.getTransitionEffect().getId()+".force_effect");

        dupClip.getTransitionEffect().setTransitionEffect("none");
        dupClip.getTransitionEffect().setDuration(0);

        clip.getTransitionEffect().setTransitionEffect("none");
        clip.getTransitionEffect().setDuration(0);

        dupClip.getVideoClipEdit().setTrim(clip.getVideoClipEdit().getEndTrimTime() - duration, clip.getVideoClipEdit().getEndTrimTime());
        clip.getVideoClipEdit().setTrim(clip.getVideoClipEdit().getStartTrimTime(), clip.getVideoClipEdit().getEndTrimTime() - duration);

        Log.d(TAG, String.format("Template split clip(%d %d) next(%d %d %s)",
                clip.getProjectStartTime(), clip.getVideoClipEdit().getStartTrimTime() + duration, clip.getVideoClipEdit().getStartTrimTime() + duration,
                dupClip.getProjectEndTime(), dupClip.getTransitionEffect().getId() ));

    }

    void setCancel(){
        Log.d(TAG,"setCancel");
        mCancel = true;
    }

    public void setUseVideoTrim(boolean useVideoTrim) {
        this.mUseVideoTrim = useVideoTrim;
    }
}
