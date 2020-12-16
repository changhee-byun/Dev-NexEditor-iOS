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
import android.graphics.Rect;
import android.media.ExifInterface;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.nexeditorsdk.exception.nexSDKException;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

final class nexTemplateDrawInfo {
    private static String TAG="nexTemplateDrawInfo";

    // DRAW_INFOS

    private static final String TAG_DRAWINFOS = "draw_infos";
    private static final String TAG_SOURCE_TYPE = "source_type";
    private static final String TAG_SOURCE_PATH = "source_path";
    private static final String TAG_START = "start";
    private static final String TAG_END = "end";
    private static final String TAG_CROP_MODE = "crop_mode";
    private static final String TAG_VIDEO_CROP_MODE = "video_crop_mode";
    private static final String TAG_IMAGE_CROP_SPEED = "image_crop_speed";
    private static final String TAG_LUT = "lut";
    private static final String TAG_ALTERNATIVE_LUT = "alternative_lut";
    private static final String TAG_DRAW_WIDTH = "draw_width";
    private static final String TAG_DRAW_HEIGHT = "draw_height";
    private static final String TAG_SYSTEM_SOURCE_WIDTH = "system_source_width";
    private static final String TAG_SYSTEM_SOURCE_HEIGHT = "system_source_height";
    private static final String TAG_VOLUME = "volume";
    private static final String TAG_AUDIO_RES = "audio_res";
    private static final String TAG_AUDIO_RES_POS = "audio_res_pos";

    int subId = 0;
    String source_type = "";
    String res_path = "";
    float start;
    float end;
    nexColorEffect lut = null;
    Map<String, String> alternative_lut = new HashMap<String, String>();
    String cropMode = "default";
    String videoCropMode = "";
    String imageCropSpeed = "";
    float drawWidth;
    float drawHeight;
    int systemSourceWidth = 0;
    int systemSourceHeight = 0;
    int volume = 0;
    String audio_res = "none";
    float audio_res_pos;

    static String getValue(JSONObject object, String tag) {
        try {
            return object.getString(tag);
        } catch (JSONException e) {
            if( tag.equals(TAG_SOURCE_TYPE) )
            {
                return "ALL";
            }
            else if( tag.equals(TAG_SOURCE_PATH) ) {
                return "default";
            }
            else if( tag.equals(TAG_START) )
            {
                return "0";
            }
            else if( tag.equals(TAG_END) )
            {
                return "1";
            }
            else if(  tag.equals(TAG_CROP_MODE) ||
                    tag.equals(TAG_VIDEO_CROP_MODE) )
            {
                return "default";
            }
            else if( tag.equals(TAG_IMAGE_CROP_SPEED) )
            {
                return "default";
            }
            else if(tag.equals(TAG_LUT))
            {
                return "null";
            }
            else if(  tag.equals(TAG_DRAW_WIDTH) ||
                    tag.equals(TAG_DRAW_HEIGHT) )
            {
                return "0";
            }
            else if(  tag.equals(TAG_VOLUME) )
            {
                return "0";
            }
            else if(  tag.equals(TAG_SYSTEM_SOURCE_WIDTH) || tag.equals(TAG_SYSTEM_SOURCE_HEIGHT) )
            {
                return "0";
            }
            else if(  tag.equals(TAG_AUDIO_RES) )
            {
                return "none";
            }
            else if(  tag.equals(TAG_AUDIO_RES_POS) )
            {
                return "0";
            }
            else
            {
                return "default";
            }
        }
    }

    public static nexTemplateDrawInfo getTemplateDrawInfo(JSONObject object, int id)
    {
        nexTemplateDrawInfo info = new nexTemplateDrawInfo();

        info.subId = id;

        info.source_type = getValue(object, TAG_SOURCE_TYPE);
        if( !info.source_type.equals("user") )
        {
            info.res_path = getValue(object, TAG_SOURCE_PATH);

            if( object.has(TAG_SYSTEM_SOURCE_WIDTH) ) {
                info.systemSourceWidth = Integer.parseInt(getValue(object, TAG_SYSTEM_SOURCE_WIDTH));
            }
            if( object.has(TAG_SYSTEM_SOURCE_HEIGHT) ) {
                info.systemSourceHeight = Integer.parseInt(getValue(object, TAG_SYSTEM_SOURCE_HEIGHT));
            }
        }

        info.start = Float.parseFloat(getValue(object, TAG_START));
        info.end = Float.parseFloat(getValue(object, TAG_END));

        String lut = getValue(object, TAG_LUT);
        if(lut != null && lut.compareTo("null") != 0 && lut.compareTo("none") != 0 ) {
            info.lut = nexColorEffect.getLutColorEffect(lut);
        }

        if( object.has(TAG_ALTERNATIVE_LUT) ) {
            try {
                info.alternative_lut.clear();
                JSONObject at = object.getJSONObject(TAG_ALTERNATIVE_LUT);
                Iterator<String> keys = at.keys();
                while( keys.hasNext() ) {
                    String key = keys.next();
                    info.alternative_lut.put(key, at.getString(key));
                    // Log.d(TAG, "Alternative transition " + key + " " + at.getString(key));
                }
            } catch (JSONException e) {
                e.printStackTrace();
                info.alternative_lut.clear();
            }
        }

        info.cropMode = getValue(object, TAG_CROP_MODE);
        info.videoCropMode = getValue(object, TAG_VIDEO_CROP_MODE);
        info.imageCropSpeed = getValue(object, TAG_IMAGE_CROP_SPEED);

        info.drawWidth = Float.parseFloat(getValue(object, TAG_DRAW_WIDTH));
        info.drawHeight = Float.parseFloat(getValue(object, TAG_DRAW_HEIGHT));

        info.volume = Integer.parseInt(getValue(object, TAG_VOLUME));

        if( object.has(TAG_AUDIO_RES) ) {
            String audio_res = getValue(object, TAG_AUDIO_RES);
            if( !audio_res.equals("none") ) {
                String audio_res_pos = getValue(object, TAG_AUDIO_RES_POS);
                info.audio_res = audio_res;
                info.audio_res_pos = Float.parseFloat(audio_res_pos);
            }
        }

        return info;
    }

    public nexDrawInfo makeDrawInfo(int duration, int startTime) {
        nexDrawInfo info = new nexDrawInfo();

        int sTime = startTime + (int)((float)duration * start);
        int eTime = startTime + (int)((float)duration * end);

        info.setStartTime(sTime);
        info.setEndTime(eTime);

        info.setLUT(lut == null ? 0 : lut.getLUTId());
        return info;
    }

    public boolean applyResouceClip(nexProject project, Context context, nexTemplateEffect templateEffect, int topEffectStart, float ratio) {
        if( (source_type.compareTo("system") == 0 || source_type.compareTo("system_mt") == 0 )&& res_path != null && res_path.length() > 0 ) {
            nexClip tmpClip = null;

            String res = nexAssetPackageManager.getAssetPackageMediaPath(context, res_path);
            if (res != null) {
                tmpClip = nexClip.getSupportedClip(res);
            }

            if (tmpClip != null) {
                tmpClip.setAssetResource(true);

                if( source_type.compareTo("system_mt") == 0 ) {
                    tmpClip.setMotionTrackedVideo(true);
                }

                project.add(tmpClip);
                tmpClip.clearDrawInfos();

                tmpClip.mStartTime = topEffectStart;
                tmpClip.mEndTime = topEffectStart + tmpClip.getTotalTime();

                setDrawInfo2Clip(tmpClip, templateEffect.id, templateEffect.getTopEffectDuration(), topEffectStart, ratio, null, false);
            }
            return true;
        }
        return false;
    }

    public void setDrawInfo2Clip(nexClip clip, int topID, int duration, int startTime, float ratio, String identifier, boolean trim) {
        if( clip == null ) return;

        nexDrawInfo info = new nexDrawInfo();
        info.setTopEffectID(topID);
        info.setID(subId);
        info.setSubEffectID(subId);

        int sTime = startTime + (int)((float)duration * start);
        int eTime = startTime + (int)((float)duration * end);

        Log.d(TAG, String.format("Template setDrawInfo2Clip(dur:%d start:%d %d %d)", duration, startTime, sTime, eTime));


        if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
            clip.mStartTime = sTime < clip.mStartTime ? sTime : clip.mStartTime;
            clip.mEndTime = eTime > clip.mEndTime ? eTime : clip.mEndTime;

            clip.setImageClipDuration(clip.mEndTime - clip.mStartTime);
        }else {
//            sTime = sTime < clip.getProjectStartTime() ? clip.getProjectStartTime() : sTime;
//            eTime = eTime > clip.getProjectEndTime() ? clip.getProjectEndTime() : eTime;

            clip.mStartTime = sTime < clip.mStartTime ? sTime : clip.mStartTime;
            clip.mEndTime = eTime > clip.mEndTime ? eTime : clip.mEndTime;

            // clip.getVideoClipEdit().setTrim();
            if( trim ) {
                int drawInfoTime = (int)((float)duration * (end-start));
                drawInfoTime = drawInfoTime < clip.getVideoDuration() ? drawInfoTime : clip.getVideoDuration();
                clip.getVideoClipEdit().setTrim(0, drawInfoTime);
            }
            clip.setClipVolume(volume);
        }

        info.setStartTime(sTime);
        info.setEndTime(eTime);

        if( identifier != null && alternative_lut.containsKey(identifier) ) {

            nexColorEffect tmpLut = nexColorEffect.getLutColorEffect(alternative_lut.get(identifier));
            info.setLUT(tmpLut == null ? 0 : tmpLut.getLUTId());
        }
        else {
            info.setLUT(lut == null ? 0 : lut.getLUTId());
        }

        if( drawWidth != 0 && drawHeight != 0 ) {
            ratio = drawWidth/drawHeight;
        }

        info.setRatio(ratio);

        applyCrop2Clip(clip, ratio);
        Rect sr = new Rect();
        Rect er = new Rect();
        clip.getCrop().getStartPositionRaw(sr);
        clip.getCrop().getEndPositionRaw(er);

        info.setRotateState(clip.getRotateDegree());
        info.setStartRect(sr);
        info.setEndRect(er);

        clip.addDrawInfo(info);
    }

    public nexDrawInfo getDrawInfo(nexClip clip, int topID, int duration, int startTime, float ratio) {
        if( clip == null ) return null;

        nexDrawInfo info = new nexDrawInfo();
        info.setTopEffectID(topID);
        info.setID(topID);
        info.setSubEffectID(subId);

        int sTime = startTime + (int)((float)duration * start);
        int eTime = startTime + (int)((float)duration * end);

        Log.d(TAG, String.format("Template setDrawInfo2Clip(dur:%d start:%d %d %d)", duration, startTime, sTime, eTime));


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

        info.setLUT(lut == null ? 0 : lut.getLUTId());

        applyCrop2Clip(clip, ratio);
        Rect sr = new Rect();
        Rect er = new Rect();
        clip.getCrop().getStartPositionRaw(sr);
        clip.getCrop().getEndPositionRaw(er);

        info.setRotateState(clip.getRotateDegree());
        info.setStartRect(sr);
        info.setEndRect(er);

        return info;
    }

    String applyCrop2Clip(nexClip clip, float ratio)
    {
        String cropMode = this.cropMode;

        if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO )
        {
            cropMode = videoCropMode.length() > 0 ? videoCropMode : cropMode;
        }

        // Log.d(TAG, String.format("Apply default crop mode(%s)", cropMode));
        switch(cropMode) {
            case "fill": {
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FILL, ratio);
                break;
            }
            case "pan_rand": {
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_RAND, ratio);
                break;
            }
            case "pan_face": {
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_FACE, ratio);
                break;
            }
            case "fit": {
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT, ratio);
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
                        clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_RAND, ratio);
                    }
                    else
                    {
                        clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT, ratio);
                    }

                    Log.d(TAG, String.format("Apply default crop mode(%f %f) (%d)", ratio, clip_ratio, rotate));
                    break;
                }
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT, ratio);
                break;
            }
        }
        return null;
    }

    protected String applySoundEffect(nexProject project, int start_time, int duration) {
        if( audio_res != null && audio_res.length() > 0 ) {
            if( !audio_res.equals("none") ) {
                project.updateProject();

                String file = nexAssetPackageManager.getAssetPackageMediaPath(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), audio_res);
                nexClip tmpClip = nexClip.getSupportedClip(file);
                if (tmpClip != null) {
                    int audio_res_dur = tmpClip.getTotalTime();
                    int start_pos = (int)(duration * audio_res_pos);
                    tmpClip.setAssetResource(true);
                    project.addAudio(tmpClip, start_time + start_pos, start_time + start_pos + audio_res_dur);
                }
            }
        }
        return null;
    }

    void print() {
        Log.d(TAG, String.format("subId : %d", subId));
        Log.d(TAG, String.format("start : %f", start));
        Log.d(TAG, String.format("end : %f", end));
        Log.d(TAG, String.format("lut : %d", lut == null ? 0 : lut.getLUTId()));
        Log.d(TAG, String.format("cropMode : %s", cropMode));
        Log.d(TAG, String.format("videoCropMode : %s", videoCropMode));
        Log.d(TAG, String.format("draw size : %f %f", drawWidth, drawHeight));
        Log.d(TAG, String.format("volume : %d", volume));
        Log.d(TAG, "---------------------------------------------------");
    }
}
