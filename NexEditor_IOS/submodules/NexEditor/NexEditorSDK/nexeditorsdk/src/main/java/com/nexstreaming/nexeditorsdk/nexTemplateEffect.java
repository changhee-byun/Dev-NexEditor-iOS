/******************************************************************************
 * File Name        : nexTemplateEffect.java
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
import android.graphics.Color;
import android.media.ExifInterface;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.util.Stopwatch;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.nexeditorsdk.exception.nexSDKException;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Vector;

final class nexTemplateEffect {
    private static String TAG="nexTemplateEffect";

    // SCENE, TRANSITION
    private static final String TAG_TYPE = "type";
    private static final String TAG_IDENTIFIER = "identifier";
    private static final String TAG_ALTERNATIVE_ID = "alternative_id";
    private static final String TAG_GROUP = "group";
    private static final String TAG_ID = "id";
    // ALL, VIDEO, IMAGE, SOLID, EXTERNAL_VIDEO, EXTERNAL_IMAGE
    private static final String TAG_SOURCE_TYPE = "source_type";
    private static final String TAG_RES_PATH = "res_path";
    private static final String TAG_DURATION = "duration";
    private static final String TAG_DURATION_MIN = "duration_min";
    private static final String TAG_DURATION_MAX = "duration_max";
    private static final String TAG_VOLUME = "volume";
    private static final String TAG_EFFECT = "effect";
    private static final String TAG_EFFECTS = "effects";
    private static final String TAG_DRAWINFOS = "draw_infos";
    private static final String TAG_BRIGHTNESS = "brightness";
    private static final String TAG_CONTRAST = "contrast";
    private static final String TAG_SATURATION = "saturation";
    private static final String TAG_COLOR_FILTER = "color_filter";
    private static final String TAG_SPEED_CONTROL = "speed_control";
    private static final String TAG_FREEZE_DURATION = "freeze_duration";
    private static final String TAG_CROP_MODE = "crop_mode";
    private static final String TAG_VIDEO_CROP_MODE = "video_crop_mode";
    private static final String TAG_IMAGE_CROP_MODE = "image_crop_mode";
    private static final String TAG_IMAGE_CROP_SPEED = "image_crop_speed";
    private static final String TAG_LUT = "lut";
    private static final String TAG_EXTERNAL_VIDEO_PATH = "external_video_path";
    private static final String TAG_EXTERNAL_IMAGE_PATH = "external_image_path";
    private static final String TAG_SOLID_COLOR = "solid_color";
    private static final String TAG_VIGNETTE = "vignette";

    private static final String TAG_POSITION = "position";
    private static final String TAG_FADE_IN_DUR = "fade_in_dur";
    private static final String TAG_FADE_OUT_DUR = "fade_out_dur";

    private static final String TAG_EFFECT_ON_RES = "apply_effect_on_res";
    private static final String TAG_AUDIO_RES = "audio_res";
    private static final String TAG_AUDIO_RES_POS = "audio_res_pos";

    private static final String TAG_TEXT_EFFECT_TITLE = "text_effect_title";
    private static final String TAG_TEXT_EFFECT_TITLE_MAX_LENGTH = "text_effect_title_max_length";
    private static final String TAG_TEXT_EFFECT_SUBTITLE = "text_effect_subtitle";
    private static final String TAG_TEXT_EFFECT_SUBTITLE_MAX_LENGTH = "text_effect_subtitle_max_length";
    private static final String TAG_TEXT_EFFECT_EXTRA = "text_effect_extra";
    private static final String TAG_TEXT_EFFECT_EXTRA_MAX_LENGTH = "text_effect_extra_max_length";
    private static final String TAG_TEXT_EFFECT_DESC = "text_effect_desc";

    private static final String TAG_TRANSITION_NAME = "transition_name";
    private static final String TAG_TRANSITION_DURATION = "transition_duration";

    private static final String TAG_USE_PATICLE_VIDEO = "use_particle_video";

    private static final String TAG_ALTERNATIVE_EFFECT = "alternative_effect";
    private static final String TAG_ALTERNATIVE_TRANSITION = "alternative_transition";
    private static final String TAG_ALTERNATIVE_LUT = "alternative_lut";
    private static final String TAG_ALTERNATIVE_AUDIO = "alternative_audio";
    private static final String TAG_ALTERNATIVE_AUDIO_POS = "alternative_audio_pos";

    int id;

    String type;
    String alternative_id;
    String source_type;
    String res_path;
    int duration;
    int duration_min;
    int duration_max;
    int volume;
    String effect_id;
    int brightness = -1;
    int contrast = -1;
    int saturation = -1;
    int color_filter = -1;
    int speed_control = 100;
    int freeze_duration = 0;
    String crop_mode = "default";
    int crop_speed = 101;
    String video_crop_mode = "";
    String image_crop_mode = "";
    String lut;
    String external_video_path;
    String external_image_path;
    String solid_color;
    String vignette;

    Map<String, String> alternative_effect = new HashMap<String, String>();
    Map<String, String> alternative_transition = new HashMap<String, String>();
    Map<String, String> alternative_lut = new HashMap<String, String>();
    Map<String, String> alternative_audio = new HashMap<String, String>();
    Map<String, String> alternative_audio_pos = new HashMap<String, String>();

    boolean apply_effect_on_res = true;
    String audio_res = "none";
    int audio_res_pos;

    String transition_id = "none";
    int transition_duration;

    int use_particle_video = 1;

    List<nexTemplateDrawInfo> effectDrawInfos = new ArrayList<>();

    public static nexTemplateEffect getTemplateEffect(JSONObject object, int id)
    {
        String type = getValue(object, TAG_TYPE);

        if( !type.equals("scene") ) return null;

        nexTemplateEffect effect = new nexTemplateEffect();

        effect.id = id;
        effect.type = type;

        effect.alternative_id = getValue(object, TAG_IDENTIFIER);

        if( object.has(TAG_ALTERNATIVE_ID) )
            effect.alternative_id = getValue(object, TAG_ALTERNATIVE_ID);

        effect.source_type = getValue(object, TAG_SOURCE_TYPE);
        if( !effect.source_type.equals("res_default") )
        {
            effect.res_path = getValue(object, TAG_RES_PATH);
        }

        effect.duration = Integer.parseInt(getValue(object, TAG_DURATION));
        effect.duration_max = Integer.parseInt(getValue(object, TAG_DURATION_MAX));
        effect.duration_min = Integer.parseInt(getValue(object, TAG_DURATION_MIN));
        effect.volume = Integer.parseInt(getValue(object, TAG_VOLUME));

        if( object.has(TAG_EFFECT) )
        {
            effect.effect_id = getValue(object, TAG_EFFECT);
        }
        else {
            effect.effect_id = getValue(object, TAG_EFFECTS);
        }

        if( object.has(TAG_DRAWINFOS)) {
            try {
                Log.d(TAG, String.format("Effect : %s", effect.effect_id));
                JSONArray jsonArray = object.getJSONArray(TAG_DRAWINFOS);
                int idx = 1;
                for(int i = 0; i < jsonArray.length(); i++ ) {
                    nexTemplateDrawInfo info = nexTemplateDrawInfo.getTemplateDrawInfo(jsonArray.getJSONObject(i), (id << 16) | idx);
                    if( info != null ) {
                        effect.effectDrawInfos.add(info);
                        info.print();
                    }
                    idx++;
                }
            } catch (JSONException e) {
                e.printStackTrace();
                effect.effectDrawInfos.clear();
            }
            if( getValue(object, TAG_EFFECT_ON_RES).equals("0") )
                effect.apply_effect_on_res = false;
        }

        String val = getValue(object, TAG_SPEED_CONTROL);
        if( !val.equals("default") )
            effect.speed_control = Integer.parseInt(val);

        val = getValue(object, TAG_BRIGHTNESS);
        if( !val.equals("default") )
            effect.brightness = Integer.parseInt(val);

        val = getValue(object, TAG_CONTRAST);
        if( !val.equals("default") )
            effect.contrast = Integer.parseInt(val);

        val = getValue(object, TAG_SATURATION);
        if( !val.equals("default") )
            effect.saturation = Integer.parseInt(val);

        val = getValue(object, TAG_COLOR_FILTER);
        if( !val.equals("default") )
            effect.color_filter = Integer.parseInt(val);

        effect.vignette = getValue(object, TAG_VIGNETTE);
        effect.lut = getValue(object, TAG_LUT);

        effect.external_video_path = getValue(object, TAG_EXTERNAL_VIDEO_PATH);
        effect.external_image_path = getValue(object, TAG_EXTERNAL_IMAGE_PATH);
        effect.solid_color = getValue(object, TAG_SOLID_COLOR);

        if( object.has(TAG_EFFECT_ON_RES)) {
            if( getValue(object, TAG_EFFECT_ON_RES).equals("0") )
                effect.apply_effect_on_res = false;
        }

        if( object.has(TAG_AUDIO_RES) ) {
            String audio_res = getValue(object, TAG_AUDIO_RES);
            if( !audio_res.equals("none") ) {
                String audio_res_pos = getValue(object, TAG_AUDIO_RES_POS);
                effect.audio_res = audio_res;
                effect.audio_res_pos = Integer.parseInt(audio_res_pos);
            }
        }

        effect.crop_mode = getValue(object, TAG_CROP_MODE);
        if( object.has(TAG_VIDEO_CROP_MODE) )
        {
            effect.video_crop_mode = getValue(object, TAG_VIDEO_CROP_MODE);
        }
        if( object.has(TAG_IMAGE_CROP_MODE) )
        {
            effect.image_crop_mode = getValue(object, TAG_IMAGE_CROP_MODE);
        }

        if( object.has(TAG_IMAGE_CROP_SPEED) ) {
            effect.crop_speed = Integer.parseInt(getValue(object, TAG_IMAGE_CROP_SPEED));
        }

        if( object.has(TAG_FREEZE_DURATION) ) {
            effect.freeze_duration = Integer.parseInt(getValue(object, TAG_FREEZE_DURATION));
        }

        effect.transition_id = getValue(object, TAG_TRANSITION_NAME);
        if( effect.transition_id == null || effect.transition_id.compareTo("none") == 0 )
            effect.transition_duration = 0;
        else
            effect.transition_duration = Integer.parseInt(getValue(object, TAG_TRANSITION_DURATION));


        if( object.has(TAG_USE_PATICLE_VIDEO) )
            effect.use_particle_video = Integer.parseInt(getValue(object, TAG_USE_PATICLE_VIDEO));
        if( object.has(TAG_ALTERNATIVE_EFFECT) ) {
            try {
                effect.alternative_effect.clear();
                JSONObject at = object.getJSONObject(TAG_ALTERNATIVE_EFFECT);
                Iterator<String> keys = at.keys();
                while( keys.hasNext() ) {
                    String key = keys.next();
                    effect.alternative_effect.put(key, at.getString(key));
                    // Log.d(TAG, "Alternative transition " + key + " " + at.getString(key));
                }
            } catch (JSONException e) {
                e.printStackTrace();
                effect.alternative_effect.clear();
            }

        }

        if( object.has(TAG_ALTERNATIVE_TRANSITION) ) {
            try {
                effect.alternative_transition.clear();
                JSONObject at = object.getJSONObject(TAG_ALTERNATIVE_TRANSITION);
                Iterator<String> keys = at.keys();
                while( keys.hasNext() ) {
                    String key = keys.next();
                    effect.alternative_transition.put(key, at.getString(key));
                    // Log.d(TAG, "Alternative transition " + key + " " + at.getString(key));
                }
            } catch (JSONException e) {
                e.printStackTrace();
                effect.alternative_transition.clear();
            }
        }

        if( object.has(TAG_ALTERNATIVE_LUT) ) {
            try {
                effect.alternative_lut.clear();
                JSONObject at = object.getJSONObject(TAG_ALTERNATIVE_LUT);
                Iterator<String> keys = at.keys();
                while( keys.hasNext() ) {
                    String key = keys.next();
                    effect.alternative_lut.put(key, at.getString(key));
                    // Log.d(TAG, "Alternative transition " + key + " " + at.getString(key));
                }
            } catch (JSONException e) {
                e.printStackTrace();
                effect.alternative_lut.clear();
            }
        }

        if( object.has(TAG_ALTERNATIVE_AUDIO) ) {
            try {
                effect.alternative_audio.clear();
                JSONObject at = object.getJSONObject(TAG_ALTERNATIVE_AUDIO);
                Iterator<String> keys = at.keys();
                while( keys.hasNext() ) {
                    String key = keys.next();
                    effect.alternative_audio.put(key, at.getString(key));
                    // Log.d(TAG, "Alternative transition " + key + " " + at.getString(key));
                }
            } catch (JSONException e) {
                e.printStackTrace();
                effect.alternative_audio.clear();
            }
        }

        if( object.has(TAG_ALTERNATIVE_AUDIO_POS) ) {
            try {
                effect.alternative_audio_pos.clear();
                JSONObject at = object.getJSONObject(TAG_ALTERNATIVE_AUDIO_POS);
                Iterator<String> keys = at.keys();
                while( keys.hasNext() ) {
                    String key = keys.next();
                    effect.alternative_audio_pos.put(key, at.getString(key));
                    // Log.d(TAG, "Alternative transition " + key + " " + at.getString(key));
                }
            } catch (JSONException e) {
                e.printStackTrace();
                effect.alternative_audio_pos.clear();
            }
        }

        return effect;
    }

    public String applyEffect(String tag, nexClip clip, boolean apply_transition, float ratio, boolean useClipSpeed)
    {

         Log.d(TAG, String.format("Template applyEffect %s  clip(%d %d) effect(%s) apply_transition(%b) transition(%s, %d)",
                 tag, clip.getProjectStartTime(), clip.getProjectDuration(), effect_id, apply_transition, transition_id, transition_duration));

        int effect_idx = id;
        if( apply_transition )
        {
            effect_idx = effect_idx | 0x01000000;
        }

        clip.setTemplateEffectID(effect_idx);

        try {

            if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO )
            {
                if(volume != -1) {
                    if( volume == 0 ) {
                        clip.setAudioOnOff(false);
                    }
                    else {
                        clip.setAudioOnOff(true);
                        clip.setClipVolume(volume);
                    }
                }
                if( useClipSpeed == false && speed_control != -1 ) {
                    clip.getVideoClipEdit().setSpeedControl(speed_control);
                }

                Log.d(TAG, String.format("freeze_duration(%d)", freeze_duration));
                if( freeze_duration != 0 ) {
                    clip.getVideoClipEdit().setFreezeDuration(freeze_duration);
                }
            }

            if(effect_id.equals("none")) {
                clip.getClipEffect(true).setEffectNone();
            } else {
                clip.getClipEffect(true).setEffect(effect_id);
            }

            clip.getClipEffect(true).setEffectShowTime(0, 0);

            applyColor2Clip(clip);
            applyCrop2Clip(clip, ratio);
            applyLUT2Clip(clip);

            if( !apply_transition || transition_id.equals("none") || transition_duration <= 0 )
            {
                clip.getTransitionEffect(true).setEffectNone();
                clip.getTransitionEffect(true).setDuration(0);
                return null;
            }

            if( clip.getProjectDuration() <= transition_duration )
            {
                Log.d(TAG, String.format("Template Apply transition was ignored on short clip"));
                clip.getTransitionEffect(true).setEffectNone();
                clip.getTransitionEffect(true).setDuration(0);
                return null;
            }

            clip.getTransitionEffect(true).setTransitionEffect(transition_id);
            clip.getTransitionEffect(true).setDuration(transition_duration);
        } catch(nexSDKException kde) {
            return kde.getMessage();
        }

        return null;
    }

    public void alternativeEffect(nexProject project, nexClip clip, String lastEffectIdentifier, boolean end) {
        // Log.d(TAG, "Template alternativeEffect " + lastEffectIdentifier + " " + end);
        if( !end ) return;

        if( lastEffectIdentifier != null && lastEffectIdentifier.length() > 0 ) {
            if( alternative_effect.containsKey(lastEffectIdentifier) ) {
                clip.getClipEffect(true).setEffect(alternative_effect.get(lastEffectIdentifier));
                // Log.d(TAG, "Template alternativeEffect effect " + lastEffectIdentifier + " " + end);
            }
            if( alternative_transition.containsKey(lastEffectIdentifier) ) {
                clip.getTransitionEffect(true).setTransitionEffect(alternative_transition.get(lastEffectIdentifier));
                // Log.d(TAG, "Template alternativeEffect transition " + lastEffectIdentifier + " " + end);
            }

            if( alternative_lut.containsKey(lastEffectIdentifier) ) {
                if (!alternative_lut.get(lastEffectIdentifier).equals("null")) {
                    nexColorEffect colorEffect = nexColorEffect.getLutColorEffect(alternative_lut.get(lastEffectIdentifier));
                    if( colorEffect != null ) {
                        clip.setColorEffect(colorEffect);
                    }
                }
                // Log.d(TAG, "Template alternativeEffect lut " + lastEffectIdentifier + " " + end);
            }

            if( alternative_audio.containsKey(lastEffectIdentifier) ) {
                String audio = alternative_audio.get(lastEffectIdentifier);

                if( audio != null && audio.length() > 0 ) {
                    if( !audio.equals("none") ) {
                        String file = nexAssetPackageManager.getAssetPackageMediaPath(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), audio);
                        nexClip tmpClip = nexClip.getSupportedClip(file);

                        if (tmpClip != null) {
                            int audio_res_dur = tmpClip.getTotalTime();
                            int pos = Integer.parseInt(alternative_audio_pos.get(lastEffectIdentifier));
                            tmpClip.setTemplateEffectID(id);
                            tmpClip.setTemplateAudioPos(pos);

                            for(int i =0; i < project.getTotalClipCount(false); i++) {
                                nexClip ac = project.getClip(0, false);
                                if( ac == null ) continue;

                                if( ac.getTemplateEffectID() == id ) {
                                    project.remove(ac);
                                }
                            }
                            project.addAudio(tmpClip, clip.getProjectStartTime(), clip.getProjectStartTime() + audio_res_dur);
                            // project.updateProject();
                        }
                    }
                }
            }

        }
    }


    public boolean addResource(nexProject project, Context appContext, float ratio)
    {
        // Log.d(TAG, String.format("Template applyResource2Project : ") + effect.get(TAG_SOURCE_TYPE) + " " + effect.get(TAG_RES_PATH) );
        nexClip tmpClip = null;
        String file = null;

        if( source_type.equals("res_video") ) {
            file = nexAssetPackageManager.getAssetPackageMediaPath(appContext, res_path);
            if(file != null) {
                tmpClip = nexClip.getSupportedClip(file);
            }

            Log.d(TAG, String.format("Template applyResource2Project file : ") + file);

            if (tmpClip != null) {
                project.add(tmpClip);
                project.updateProject();

                tmpClip.setAssetResource(true);
                applyEffect("res : ", tmpClip, true, ratio, false);
                applySoundEffect2Clip(project, tmpClip);
            }
            return true;
        }
        else if( source_type.equals("res_image") )
        {
            file = nexAssetPackageManager.getAssetPackageMediaPath(appContext, res_path);
            if(file != null) {
                tmpClip = nexClip.getSupportedClip(file);
            }

            if (tmpClip != null) {
                tmpClip.setImageClipDuration(duration + transition_duration);
                project.add(tmpClip);
                project.updateProject();

                tmpClip.setAssetResource(true);
                applyEffect("res : ", tmpClip, true, ratio, false);
                applySoundEffect2Clip(project, tmpClip);
            }
            return true;
        }
        else if( source_type.equals("res_solid") )
        {
            int solidColor = Color.parseColor(res_path);
            tmpClip = nexClip.getSolidClip(solidColor);
            if(tmpClip != null) {
                tmpClip.setImageClipDuration(duration + transition_duration);
                project.add(tmpClip);
                project.updateProject();

                tmpClip.setAssetResource(true);
                applyEffect("res : ", tmpClip, true, ratio, false);
                applySoundEffect2Clip(project, tmpClip);
            }
            return true;
        }
        return false;
    }

    public boolean isResource()
    {
        if( source_type.equals("res_video") ) {

            return true;
        }
        else if( source_type.equals("res_image") )
        {
               return true;
        }
        else if( source_type.equals("res_solid") )
        {
             return true;
        }
        return false;
    }

    public String getSource_type() {
        return source_type;
    }

    public int getDuration() {
        return duration;
    }

    public int getDuration_min() {
        return duration_min;
    }

    public int getDuration_max() {
        return duration_max;
    }

    public boolean isApply_effect_on_res() {
        return apply_effect_on_res;
    }

    public boolean useParticleVideo() {
        return use_particle_video == 1 ? true : false;
    }

    public int getTransition_duration() {
        return transition_duration;
    }

    public String getEffect_id() {
        return effect_id;
    }

    public String getTransition_id() {
        return transition_id;
    }

    public int getEffectDrawInfoCount(boolean onlyUser) {
        if( effectDrawInfos != null ) {
            int src_count = 0;

            for(nexTemplateDrawInfo info : effectDrawInfos ) {
                if( onlyUser ) {
                    if( info.source_type.compareTo("res_default") == 0 || info.source_type.compareTo("user") == 0)
                        src_count++;
                }
                else {
                    src_count++;
                }
            }
            return src_count;
        }
        return 0;
    }

    public List<nexDrawInfo> makeTopDrawInfos(int startTime, int adjustDuration, String identifier)
    {
        List<nexDrawInfo> top = new ArrayList<>();

        adjustDuration = adjustDuration == 0 ? duration : adjustDuration;

        nexDrawInfo infoEffect = new nexDrawInfo();
        infoEffect.setID(id);
        infoEffect.setSubEffectID((id << 16) | 1);
        if( identifier != null && alternative_effect.containsKey(identifier) ) {
            infoEffect.setEffectID(alternative_effect.get(identifier));
        }
        else {
            infoEffect.setEffectID(effect_id);
        }
        infoEffect.setStartTime(startTime);
        infoEffect.setEndTime(startTime + adjustDuration);
        infoEffect.setSubTemplateDrawInfos(effectDrawInfos);

        top.add(infoEffect);

        if( transition_id.compareTo("none") != 0 && transition_duration > 0 )
        {
            nexDrawInfo infoTransition = new nexDrawInfo();
            if( identifier != null && alternative_transition.containsKey(identifier) ) {
                infoTransition.setEffectID(alternative_transition.get(identifier));
            }
            else {
                infoTransition.setEffectID(transition_id);
            }
            infoTransition.setIsTransition(1);
            infoTransition.setStartTime(infoEffect.getEndTime() - transition_duration);
            infoTransition.setEndTime(infoEffect.getEndTime());

            // infoEffect.setEndTime(startTime + adjustDuration + transition_duration);

            top.add(infoTransition);
            return top;
        }

        return top;
    }

    public int getTopEffectDuration()
    {
        return duration;
    }

    public int getTopEffectMinDuration()
    {
        return duration_min;
    }

    public int getTopEffectMaxDuration()
    {
        return duration_max;
    }


    public List<nexTemplateDrawInfo> getSubEffects()
    {
        return effectDrawInfos;
    }

    public List<nexDrawInfo> makeSubDrawInfos(nexClip clip, int topID, int duration, int startTime, float ratio) {
        List<nexDrawInfo> drawInfos = new ArrayList<>();

        for( nexTemplateDrawInfo info : effectDrawInfos )
        {
            nexDrawInfo i = info.getDrawInfo(clip, topID, duration, startTime, ratio);
            drawInfos.add(i);
        }
        return drawInfos;
    }

    public void copySubEffects(List<nexTemplateDrawInfo> subEffects)
    {
        subEffects.clear();
        for(nexTemplateDrawInfo e : effectDrawInfos ) {
            if( subEffects.size() <= 0 ) {
                subEffects.add(e);
                continue;
            }

            for(int i = 0; i < subEffects.size(); i++) {
                if( (e.end - e.start) < (subEffects.get(i).end - subEffects.get(i).start) ) {
                    subEffects.add(i, e);
                    break;
                }
                if( i+1 == subEffects.size()) {
                    subEffects.add(e);
                    break;
                }
            }
        }
    }

    static String getValue(JSONObject object, String tag) {
        try {
            return object.getString(tag);
        } catch (JSONException e) {
            if( tag.equals(TAG_EFFECTS) ) {
                return "none";
            } else if(  tag.equals(TAG_ID) ||
                    tag.equals(TAG_TRANSITION_DURATION) ||
                    tag.equals(TAG_AUDIO_RES_POS) ) {
                return "0";
            } else if(  tag.equals(TAG_VOLUME) ||
                    tag.equals(TAG_SPEED_CONTROL) ) {
                return "100";
            } else if(  tag.equals(TAG_DURATION) ) {
                return "3000";
            } else if(  tag.equals(TAG_DURATION_MAX) ) {
                return "5000";
            } else if(  tag.equals(TAG_DURATION_MIN) ) {
                return "2000";
            } else if(  tag.equals(TAG_BRIGHTNESS) ||
                    tag.equals(TAG_CONTRAST) ||
                    tag.equals(TAG_SATURATION) ||
                    tag.equals(TAG_COLOR_FILTER) ) {
                return "-1";
            } else if(tag.equals(TAG_SOURCE_TYPE)) {
                return "ALL";
            } else if(tag.equals(TAG_EXTERNAL_VIDEO_PATH)
                    || tag.equals(TAG_EXTERNAL_IMAGE_PATH)
                    || tag.equals(TAG_SOLID_COLOR)
                    || tag.equals(TAG_LUT)) {
                return null;
            } else if(tag.equals(TAG_VIGNETTE)) {
                return "clip,no";
            } else if(tag.equals(TAG_CROP_MODE)) {
                return "default";
            } else if(tag.equals(TAG_TRANSITION_NAME)) {
                return "none";
            } else if(tag.equals(TAG_IDENTIFIER)) {
                return "";
            } else if( tag.equals(TAG_FREEZE_DURATION) ) {
                return "0";
            } else if( tag.equals(TAG_USE_PATICLE_VIDEO) ) {
                return "1";
            } else {
                return "default";
            }
        }
    }

    nexColorEffect getColorEffect(int index) {
        List<nexColorEffect> presetList = nexColorEffect.getPresetList();
        return presetList.get(index);
    }

    String applyColor2Clip(nexClip clip)
    {
        if(brightness != -1) {
            clip.setBrightness(brightness);
        }
        if(contrast != -1) {
            clip.setContrast(contrast);
        }
        if(saturation != -1) {
            clip.setSaturation(saturation);
        }
        if(color_filter != -1) {
            clip.setColorEffect(getColorEffect(color_filter - 1));
        }
        return null;
    }

    String applySoundEffect2Clip(nexProject project, nexClip clip)
    {
        if( audio_res != null && audio_res.length() > 0 ) {
            if( !audio_res.equals("none") ) {
                project.updateProject();
                String file = nexAssetPackageManager.getAssetPackageMediaPath(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), audio_res);
                nexClip tmpClip = nexClip.getSupportedClip(file);

                if (tmpClip != null) {
                    int audio_res_dur = tmpClip.getTotalTime();
                    tmpClip.setTemplateEffectID(id);
                    tmpClip.setTemplateAudioPos(audio_res_pos);
                    project.addAudio(tmpClip, clip.getProjectStartTime(), clip.getProjectStartTime() + audio_res_dur);
                    // project.updateProject();
                }
            }
        }

        return null;
    }

    void applySoundEffect2Project(nexProject project, Context context, int topEffectStart, String identifier)
    {
        if( audio_res != null && audio_res.length() > 0 ) {
            if( !audio_res.equals("none") ) {
                project.updateProject();

                String audio = audio_res;
                int pos = audio_res_pos;

                if( identifier != null && alternative_audio.containsKey(identifier) ) {

                    audio = alternative_audio.get(identifier);
                    pos = Integer.parseInt(alternative_audio_pos.get(identifier));
                }


                String file = nexAssetPackageManager.getAssetPackageMediaPath(context, audio);
                nexClip tmpClip = nexClip.getSupportedClip(file);

                if (tmpClip != null) {
                    int audio_res_dur = tmpClip.getTotalTime();
                    tmpClip.setAssetResource(true);
                    tmpClip.setTemplateEffectID(id);
                    tmpClip.setTemplateAudioPos(pos);
                    project.addAudio(tmpClip, topEffectStart, topEffectStart + audio_res_dur);
                }
            }
        }
    }

    String applyCrop2Clip(nexClip clip, float ratio)
    {
        String cropMode = crop_mode;
        if(clip.getCrop().getEndPosionLock()){
            return null;
        }
        if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE )
        {
            cropMode = image_crop_mode.length() > 0 ? image_crop_mode : cropMode;
        }
        if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO )
        {
            cropMode = video_crop_mode.length() > 0 ? video_crop_mode : cropMode;
        }

        // Log.d(TAG, String.format("Apply default crop mode(%s)", cropMode));
        switch(cropMode) {
            case "fill": {
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FILL);
                break;
            }
            case "pan_rand": {
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_RAND);
                break;
            }
            case "pan_face": {
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_FACE);
                break;
            }
            case "fit": {
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                break;
            }
            default: {
                if( ratio != 0 ){
                    float clip_ratio = clip.getWidth() / (clip.getHeight() * 1.0f);
                    int rotate = clip.getRotateDegree();

                    if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
                        if( rotate == 90 || rotate == 270) {
                            clip_ratio = clip.getHeight() / (clip.getWidth() * 1.0f);
                        }
                    }
                    else if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                        ExifInterface exif = null;
                        try {
                            exif = new ExifInterface(clip.getRealPath());
                        } catch (IOException e) {
                            //Error
                            e.printStackTrace();
                        }

                        if( exif != null ) {
                            String orientString = exif.getAttribute(ExifInterface.TAG_ORIENTATION);
                            int orientation = orientString != null ? Integer.parseInt(orientString) : ExifInterface.ORIENTATION_NORMAL;

                            if(orientation == ExifInterface.ORIENTATION_ROTATE_90 || orientation == ExifInterface.ORIENTATION_ROTATE_270) {
                                if( orientation == ExifInterface.ORIENTATION_ROTATE_90 )
                                    rotate = 90;
                                if( orientation == ExifInterface.ORIENTATION_ROTATE_270 )
                                    rotate = 270;

                                clip_ratio = clip.getHeight() / (clip.getWidth() * 1.0f);
                            }
                        }
                    }

                    if( Math.abs(ratio - clip_ratio) > 0.05 )
                    {
                        clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_RAND);
                    }
                    else
                    {
                        clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                    }

                    Log.d(TAG, String.format("Apply default crop mode(%f %f) (%d)", ratio, clip_ratio, rotate));
                    break;
                }
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                break;
            }
        }
        return null;
    }

    String applyLUT2Clip(nexClip clip) {
        if(lut != null) {
            if (!lut.equals("null")) {
                nexColorEffect colorEffect = nexColorEffect.getLutColorEffect(lut);
                if( colorEffect != null ) {
                    Log.d(TAG,"Template applyLUT2Clip effect set =xte"+colorEffect.getPresetName());
                    clip.setColorEffect(colorEffect);
                }
            }
        }
        return null;
    }

    nexTemplateUnitEDL makeTemplateUnit(int start, int master_idx)
    {
        nexTemplateUnitEDL master = new nexTemplateUnitEDL();
        master.type = 0;
        master.start = start;
        master.end = start + duration;
        master.duration = duration;
        master.master_idx = master_idx;
        master.alternative = alternative_id;
        master.template_effect = this;

        int idx = 0;
        for( nexTemplateDrawInfo info : effectDrawInfos )
        {
            nexTemplateUnitEDL sub = new nexTemplateUnitEDL();
            sub.type = 1;
            sub.start = start + (int)(duration * info.start);
            sub.end = start + (int)(duration * info.end);
            sub.master_idx = master_idx;
            sub.sub_idx = idx++;

            master.addChild(sub);
        }

        return master;
    }

    nexTemplateUnitEDL makeTemplateUnit(Context appContext, int start)
    {
        nexTemplateUnitEDL master = new nexTemplateUnitEDL();
        master.type = 0;
        master.start = start;
        master.end = start + duration;
        master.duration = duration;
        master.transition_duration = transition_duration;
        master.alternative = alternative_id;
        master.template_effect = this;

        int idx = 0;
        for( nexTemplateDrawInfo info : effectDrawInfos )
        {
            nexTemplateUnitEDL sub = new nexTemplateUnitEDL();
            sub.type = 1;
            sub.template_drawinfo = info;
            sub.sub_idx = idx++;

            if( info.source_type.compareTo("user") != 0  ) {
                String file = nexAssetPackageManager.getAssetPackageMediaPath(appContext, info.res_path);
                if(file != null) {
                    nexClip tmpClip = nexClip.getSupportedClip(file);
                    tmpClip.setAssetResource(true);
                    if( info.source_type.compareTo("system_mt") == 0 )
                        tmpClip.setMotionTrackedVideo(true);
                    sub.empty = false;
                    sub.clip = tmpClip;
                    sub.clip_type = 2;
                }
            }

            master.addChild(sub);
        }

        return master;
    }





}
