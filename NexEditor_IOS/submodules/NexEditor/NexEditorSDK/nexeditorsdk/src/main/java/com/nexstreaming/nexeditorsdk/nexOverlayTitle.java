/******************************************************************************
 * File Name        : nexOverlayTitle.java
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

import android.animation.TimeInterpolator;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.AccelerateInterpolator;
import android.view.animation.AnticipateInterpolator;
import android.view.animation.AnticipateOvershootInterpolator;
import android.view.animation.BounceInterpolator;
import android.view.animation.CycleInterpolator;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.LinearInterpolator;
import android.view.animation.OvershootInterpolator;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.app.common.util.utilityTextOverlay;
import com.nexstreaming.kminternal.kinemaster.config.LL;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

/**
 * Preparing...
 * @since version 1.0.0
 */
final class nexOverlayTitle {
    private static String TAG="nexOverlayTitle";

    private static final String TAG_OVERLAY = "overlay";
    private static final String TAG_OVERLAY_NAME = "overlay_name";
    private static final String TAG_OVERLAY_VERSION = "overlay_version";
    private static final String TAG_OVERLAY_DESCRIPTION = "overlay_desc";
    private static final String TAG_OVERLAY_PRIORITY = "overlay_priority";

    private static final String TAG_OVERLAY_X = "overlay_x";
    private static final String TAG_OVERLAY_Y = "overlay_y";
    private static final String TAG_OVERLAY_WIDTH = "overlay_width";
    private static final String TAG_OVERLAY_HEIGHT = "overlay_height";


    private static final String TAG_ID= "id";
    private static final String TAG_TYPE= "type";
    private static final String TAG_PRIORITY= "priority";
    private static final String TAG_TEXT = "text";
    private static final String TAG_TEXT_DESC = "text_desc";
    private static final String TAG_TEXT_MAX_LEN = "text_max_len";
    private static final String TAG_GROUP= "group";
    private static final String TAG_POSITION = "position";
    private static final String TAG_START_TIME = "start_time";
    private static final String TAG_DURATION = "duration";
    private static final String TAG_END_TIME = "end_time";
    private static final String TAG_FONT = "font";
    private static final String TAG_FONT_SIZE = "font_size";
    private static final String TAG_TEXT_COLOR = "text_color";
    private static final String TAG_SHADOW_COLOR = "shadow_color";
    private static final String TAG_SHADOW_VISIBLE = "shadow_visible";
    private static final String TAG_GLOW_COLOR = "glow_color";
    private static final String TAG_GLOW_VISIBLE = "glow_visible";
    private static final String TAG_OUTLINE_COLOR = "outline_color";
    private static final String TAG_OUTLINE_VISIBLE = "outline_visible";
    private static final String TAG_ALIGN = "align";
    private static final String TAG_ANIMATION = "animation";
    private static final String TAG_ROTATE = "rotate";
    private static final String TAG_SCALE = "scale";
    private static final String TAG_ADJUST_POS_NON_SUB = "adjust_pos_non_sub";
    private static final String TAG_ADJUST_ALIGN_NON_SUB = "adjust_align_non_sub";
    private static final String TAG_ADJUST_ANIMATION_NON_SUB = "adjust_animation_non_sub";

    private static final String TAG_IMAGE_RES = "image_res";

    private static final String TAG_VALUES = "values";
    private static final String TAG_MOTION_TYPE = "motion_type";
    private static final String TAG_START = "start";
    private static final String TAG_END = "end";
    private static final String TAG_START_X = "start_x";
    private static final String TAG_END_X = "end_x";
    private static final String TAG_START_Y = "start_y";
    private static final String TAG_END_Y = "end_y";
    private static final String TAG_CLOCKWISE = "clockwise";
    private static final String TAG_ROTATE_DEGREE = "rotatedegree";

    private enum Priority{
        NONE,
        START,
        END,
        START_END
    }

    public interface TitleInfoListener {
        public void OnTitleInfoListener(int id, String font, int font_size, String text, int maxLen, String desc, String group, int overlay_width, int overlay_height);
    }

    private String mOverlayName;
    private String mOverlayVersion;
    private String mOverlayDescription;
    private String mOverlayPriority;
    private Priority mPriority = Priority.NONE;

    private ArrayList<HashMap<String, String>> mOverlayList;

    private ArrayList<nexOverlayEDL> mEDLList = new ArrayList<>();

    private int mOverlayID = 0;

    /**
     * @brief This method creates an instance of <tt>nexTemplateComposer</tt>.
     *
     * @since version 1.0.0
     */
    public nexOverlayTitle() {
        mOverlayList = new ArrayList<HashMap<String, String>>();
    }

    /**
     * @brief  This method clears the resources of <tt>nexTemplateComposer</tt>.
     * This method must be called when the activity is destroyed.
     *
     * @since version 1.0.0
     */
    public void release() {
        if(mOverlayList != null) {
            mOverlayList.clear();
        }

        if( mEDLList != null ) {
            mEDLList.clear();
        }
    }

    private boolean parseOverlayAssetInfo(JSONObject object) {
        HashMap<String, String> overlay = new HashMap<>();

        try {
            mOverlayName = object.getString(TAG_OVERLAY_NAME);
            mOverlayVersion = object.getString(TAG_OVERLAY_VERSION);
            mOverlayDescription = object.getString(TAG_OVERLAY_DESCRIPTION);

            if( mOverlayVersion.startsWith("1.") ) {
                mOverlayPriority = object.getString(TAG_OVERLAY_PRIORITY);
            }
            else if( mOverlayVersion.startsWith("2.") ) {
            }
        } catch (JSONException e) {
            e.printStackTrace();
            if(LL.D) Log.d(TAG, "pasrse Overlay info failed : " + e.getMessage());
            return false;
        }

        return true;
    }

    private String parseEDL(JSONObject object, TitleInfoListener titleInfoListener) {

        nexOverlayEDL edl = new nexOverlayEDL();

        if(LL.D) Log.d(TAG, "parseOverlay edl : " + object.toString());

        String ret = edl.parseOverlayTitleInfo(object);
        if( ret != null ) {
            return ret;
        }

        mEDLList.add(edl);
        return null;
    }

    private String parseOverlay(JSONObject object, TitleInfoListener titleInfoListener) {
        try {
            HashMap<String, String> overlay = new HashMap<String, String>();

            if(LL.D) Log.d(TAG, "parseOverlay Overlay : " + object.toString());

            overlay.put(TAG_ID, "" + mOverlayID);
            overlay.put(TAG_TYPE, object.getString(TAG_TYPE));
            // overlay.put(TAG_PRIORITY, object.getString(TAG_PRIORITY));

            if( object.has(TAG_TEXT) ) overlay.put(TAG_TEXT, object.getString(TAG_TEXT));
            if( object.has(TAG_TEXT_DESC) ) overlay.put(TAG_TEXT_DESC, object.getString(TAG_TEXT_DESC));
            if( object.has(TAG_TEXT_MAX_LEN) ) overlay.put(TAG_TEXT_MAX_LEN, object.getString(TAG_TEXT_MAX_LEN));
            if( object.has(TAG_POSITION) ) overlay.put(TAG_POSITION, object.getString(TAG_POSITION));
            if( object.has(TAG_START_TIME) ) overlay.put(TAG_START_TIME, object.getString(TAG_START_TIME));
            if( object.has(TAG_DURATION) ) overlay.put(TAG_DURATION, object.getString(TAG_DURATION));
            if( object.has(TAG_FONT) ) overlay.put(TAG_FONT, object.getString(TAG_FONT));
            if( object.has(TAG_FONT_SIZE) ) overlay.put(TAG_FONT_SIZE, object.getString(TAG_FONT_SIZE));
            if( object.has(TAG_TEXT_COLOR) ) overlay.put(TAG_TEXT_COLOR, object.getString(TAG_TEXT_COLOR));
            if( object.has(TAG_SHADOW_COLOR) ) overlay.put(TAG_SHADOW_COLOR, object.getString(TAG_SHADOW_COLOR));
            if( object.has(TAG_SHADOW_VISIBLE) ) overlay.put(TAG_SHADOW_VISIBLE, object.getString(TAG_SHADOW_VISIBLE));
            if( object.has(TAG_GLOW_COLOR) ) overlay.put(TAG_GLOW_COLOR, object.getString(TAG_GLOW_COLOR));
            if( object.has(TAG_GLOW_VISIBLE) ) overlay.put(TAG_GLOW_VISIBLE, object.getString(TAG_GLOW_VISIBLE));
            if( object.has(TAG_OUTLINE_COLOR) ) overlay.put(TAG_OUTLINE_COLOR, object.getString(TAG_OUTLINE_COLOR));
            if( object.has(TAG_OUTLINE_VISIBLE) ) overlay.put(TAG_OUTLINE_VISIBLE, object.getString(TAG_OUTLINE_VISIBLE));
            if( object.has(TAG_ALIGN) ) overlay.put(TAG_ALIGN, object.getString(TAG_ALIGN));
            if( object.has(TAG_ANIMATION) ) overlay.put(TAG_ANIMATION, object.getString(TAG_ANIMATION));
            if( object.has(TAG_ADJUST_ANIMATION_NON_SUB) ) overlay.put(TAG_ADJUST_ANIMATION_NON_SUB, object.getString(TAG_ADJUST_ANIMATION_NON_SUB));

            if( object.has(TAG_ROTATE) ) overlay.put(TAG_ROTATE, object.getString(TAG_ROTATE));
            if( object.has(TAG_SCALE) ) overlay.put(TAG_SCALE, object.getString(TAG_SCALE));

            if( object.has(TAG_GROUP) ) overlay.put(TAG_GROUP, object.getString(TAG_GROUP));
            if( object.has(TAG_ADJUST_POS_NON_SUB) ) overlay.put(TAG_ADJUST_POS_NON_SUB, object.getString(TAG_ADJUST_POS_NON_SUB));
            if( object.has(TAG_ADJUST_ALIGN_NON_SUB) ) overlay.put(TAG_ADJUST_ALIGN_NON_SUB, object.getString(TAG_ADJUST_ALIGN_NON_SUB));

            if( object.has(TAG_IMAGE_RES) ) overlay.put(TAG_IMAGE_RES, object.getString(TAG_IMAGE_RES));

            if( object.getString(TAG_START_TIME).equals("0") ) {
                if( mPriority == Priority.START_END ) {
                } else if( mPriority == Priority.END ) {
                    mPriority = Priority.START_END;
                } else {
                    mPriority = Priority.START;
                }
            }else if( object.getString(TAG_START_TIME).equals("-1") ) {
                if( mPriority == Priority.START_END ) {
                } else if( mPriority == Priority.START ) {
                    mPriority = Priority.START_END;
                } else {
                    mPriority = Priority.END;
                }
            }

            if( titleInfoListener != null && overlay.containsKey(TAG_TEXT) ) {
                String font = object.getString(TAG_FONT);
                String text = object.getString(TAG_TEXT);
                String desc = object.getString(TAG_TEXT_DESC);
                String group = object.has(TAG_GROUP) ? object.getString(TAG_GROUP) : "";
                int max_len = Integer.parseInt(object.getString(TAG_TEXT_MAX_LEN));
                int font_size = Integer.parseInt(object.getString(TAG_FONT_SIZE));

                int overlay_width = 0;
                int overlay_height = font_size;
                if( object.has(TAG_POSITION) )
                {
                    String pos = overlay.get(TAG_POSITION).replace(" ", "");
                    String[] position = pos.split(",");

                    if (position != null) {

                        int left, right, top, bottom;

                        left = Integer.parseInt(position[0]);
                        top = Integer.parseInt(position[1]);

                        right = Integer.parseInt(position[2]);
                        bottom = Integer.parseInt(position[3]);
                        overlay_width = right - left;
                        overlay_height = bottom - top;
                    }
                }

                titleInfoListener.OnTitleInfoListener(mOverlayID, font, font_size, text, max_len, desc, group, overlay_width,overlay_height);
            }

            mOverlayID++;
            mOverlayList.add(overlay);
        } catch (JSONException e) {
            e.printStackTrace();
            return e.getMessage();
        }

        return null;
    }

    public String parseOverlayAsset(JSONObject jsonObject, TitleInfoListener titleInfoListener) {

        if( parseOverlayAssetInfo(jsonObject) == false )
        {
            return "parseOverlayAssetInfo parse error";
        }

        try {
            String ret = null;
            JSONArray overlays = jsonObject.getJSONArray(TAG_OVERLAY);
            for (int i = 0; i < overlays.length(); i++) {

                JSONObject jsonItem =  overlays.getJSONObject(i);
                if( mOverlayVersion.startsWith("1.") ) {
                    ret = parseOverlay(jsonItem, titleInfoListener);
                }
                else if( mOverlayVersion.startsWith("2.") ) {
                    ret = parseEDL(jsonItem, titleInfoListener);
                }
                else {
                    release();
                    return "not supported version";
                }

                if( ret != null ) {
                    release();
                    if(LL.D) Log.d(TAG, "Overlay parse error : " + ret);
                    return ret;
                }
            }
        } catch (JSONException e) {
            release();
            e.printStackTrace();
            if(LL.D) Log.d(TAG, "parseOverlayAsset failed" + e.getMessage());
            return e.getMessage();
        }

        if(LL.D) Log.d(TAG, "parseOverlayAsset end");
        return null;
    }

    public String applyOverlayAsset(Context context, nexProject project, List<nexOverlayManager.nexTitleInfo> inputTextList) {

        clearOverlayAsset(project);

        Priority apply = getCheckPriority(project);

        if( mOverlayVersion.startsWith("1.") ) {
        }
        else if( mOverlayVersion.startsWith("2.") ) {

            int overlayWidth = nexApplicationConfig.getAspectProfile().getWidth();
            int overlayHeight = nexApplicationConfig.getAspectProfile().getHeight();

            for(nexOverlayEDL edl : mEDLList) {

                if( edl.display.startsWith("squar") ){
                    if( overlayWidth != overlayHeight ) {
                        edl.resetApplyInfo();
                        continue;
                    }
                }
                else if( edl.display.startsWith("landscap") ){
                    if( overlayWidth <= overlayHeight ) {
                        edl.resetApplyInfo();
                        continue;
                    }
                }
                else if( edl.display.startsWith("portrai") ){
                    if( overlayWidth >= overlayHeight ) {
                        edl.resetApplyInfo();
                        continue;
                    }
                }

                int width;
                int height;
                if( edl.width == 0 )
                {
                    height = (int)(overlayHeight * edl.height);
                    width = (int)(height * edl.ratio);

                }
                else {
                    width = (int)(overlayWidth * edl.width);
                    height = (int)(width/edl.ratio);
                }

                int x = edl.getPosX(overlayWidth, width);
                int y = edl.getPosY(overlayHeight, height);

                nexOverlayFilter filter = new nexOverlayFilter(edl.edl);

                // nexEffectLibrary.getEffectLibrary(getApplicationContext()).findOverlayEffectById(mSelectedOverlayFilterId);

                // filter.getEffectOption().getTextOptions()[0];

                int start = edl.start;
                int duration = edl.duration;

                if( start == -1 ) {

                    start = project.getTotalTime() - duration;
                }

                if( duration == -1 ) {
                    duration = project.getTotalTime() - start;
                }

                filter.setWidth(width);
                filter.setHeight(height);

                edl.setApplyInfo(filter, start, duration, x, y, width, height, overlayWidth, overlayHeight);

                nexOverlayItem textOverlayItem = new nexOverlayItem(filter, x, y, start, start + duration);

                textOverlayItem.setOverlayTitle(true);
                project.addOverlay(textOverlayItem);

                Log.d(TAG, String.format("Apply edl overlay(%s) (%d %d) (%d %d %d %d)", edl.edl, start, duration, x, y, width, height));
            }

            return null;
        }
        else {
            return "Not support version";
        }

        String group = null;
        boolean main_olay = false;
        int adjust_x = 0;
        int adjust_y = 0;

        for(HashMap<String, String> overlay : mOverlayList ) {

            nexOverlayItem textOverlayItem = null;

            String type = overlay.get(TAG_TYPE);

            if (type.equals("title")) {
                if (overlay.containsKey(TAG_GROUP) && overlay.get(TAG_GROUP).endsWith("_1")) {
                    if (group == null) {
                        group = overlay.get(TAG_GROUP);
                        main_olay = hasEmptyTitleInGroup(inputTextList,group);
                        if( main_olay && overlay.containsKey(TAG_ADJUST_POS_NON_SUB) ) {

                            String pos = overlay.get(TAG_ADJUST_POS_NON_SUB).replace(" ", "");
                            String[] position = pos.split(",");
                            adjust_x = Integer.parseInt(position[0]);
                            adjust_y = Integer.parseInt(position[1]);
                        }
                    } else if (!group.equals(overlay.get(TAG_GROUP))) {
                        group = overlay.get(TAG_GROUP);

                        adjust_x = 0;
                        adjust_y = 0;

                        main_olay = hasEmptyTitleInGroup(inputTextList,group);
                        if( main_olay && overlay.containsKey(TAG_ADJUST_POS_NON_SUB) ) {

                            String pos = overlay.get(TAG_ADJUST_POS_NON_SUB).replace(" ", "");
                            String[] position = pos.split(",");
                            adjust_x = Integer.parseInt(position[0]);
                            adjust_y = Integer.parseInt(position[1]);
                        }
                    }
                }
            }

            if( main_olay && overlay.containsKey(TAG_GROUP) && overlay.get(TAG_GROUP).endsWith("_2") )
                continue;

            switch(type) {
                case "title" :
                {
                    int id = Integer.parseInt(overlay.get(TAG_ID));
                    String text = overlay.get(TAG_TEXT);
                    int text_max_len = Integer.parseInt(overlay.get(TAG_TEXT_MAX_LEN));
                    String pos = overlay.get(TAG_POSITION).replace(" ", "");
                    String[] position = pos.split(",");

                    if (position == null) {
                        return "Wrong position data of title";
                    }

                    int left, top, right, bottom;

                    left = Integer.parseInt(position[0]);
                    top = Integer.parseInt(position[1]);

                    right = Integer.parseInt(position[2]);
                    bottom = Integer.parseInt(position[3]);

                    int x = (left + right) / 2;
                    int y = (top + bottom) / 2;

                    int w = right - left;
                    int h = bottom - top;

                    int start_time = Integer.parseInt(overlay.get(TAG_START_TIME));
                    int duration = Integer.parseInt(overlay.get(TAG_DURATION));

                    if ((apply == Priority.START && start_time < 0) ||
                            (apply == Priority.END && start_time >= 0)) {
                        if (LL.D)
                            Log.d(TAG, String.format("Apply title skip(%s %d)", apply.name(), start_time));
                        continue;
                    }

                    String font = overlay.get(TAG_FONT);

                    font = getFont(id, inputTextList, font);
                    //int font_size = Integer.parseInt(overlay.get(TAG_FONT_SIZE));
                    int font_size = getFontSize(id, inputTextList,Integer.parseInt(overlay.get(TAG_FONT_SIZE)));
                    int text_color = Color.parseColor(overlay.get(TAG_TEXT_COLOR));
                    if( font_size > h ){
                        font_size = h-4;
                    }
                    // font_size = 150;

                    boolean shadow_visible = Integer.parseInt(overlay.get(TAG_SHADOW_VISIBLE)) == 1;
                    int shadow_color = Color.parseColor(overlay.get(TAG_SHADOW_COLOR));

                    boolean glow_visible = Integer.parseInt(overlay.get(TAG_GLOW_VISIBLE)) == 1;
                    int glow_color = Color.parseColor(overlay.get(TAG_GLOW_COLOR));
                    boolean outline_visible = Integer.parseInt(overlay.get(TAG_OUTLINE_VISIBLE)) == 1;
                    int outline_color = Color.parseColor(overlay.get(TAG_OUTLINE_COLOR));

                    String align = overlay.get(TAG_ALIGN);

                    if( main_olay && overlay.containsKey(TAG_ADJUST_ALIGN_NON_SUB) )
                        align = overlay.get(TAG_ADJUST_ALIGN_NON_SUB);

                    Paint.Align textAlign = Paint.Align.CENTER; // LEFT, RIGHT, CENTER
                    if (align.contains("LEFT")) {
                        textAlign = Paint.Align.LEFT;
                    } else if (align.contains("RIGHT")) {
                        textAlign = Paint.Align.RIGHT;
                    }

                    int drawAlign = 1;

                    if( align.contains("MIDDLE") ) {
                        drawAlign = 2;
                    }
                    else if (align.contains("BOTTOM")) {
                        drawAlign = 3;
                    }

                    String userText = null;
                    if( main_olay )
                    {
                        userText = getText(group, inputTextList);
                        font_size = getTextSize(group, inputTextList, font_size);
                        x += adjust_x;
                        y += adjust_y;
                    }
                    else
                    {
                        userText = getText(id, inputTextList, text);
                    }

                    if( userText == null || userText.length() <= 0 )
                    {
                        continue;
                    }

                    if (start_time < 0) {
                        if (project.getTotalTime() < duration) {
                            start_time = 0;
                            duration = project.getTotalTime();
                        } else {
                            start_time = project.getTotalTime() - duration;
                        }
                    }

                    utilityTextOverlay textOverlay = new utilityTextOverlay(context, userText, w, h, 1);

                    textOverlayItem = new nexOverlayItem(new nexOverlayImage("" + id, textOverlay), x, y, start_time, start_time + duration);

                    textOverlay.setText(font_size, text_color, textAlign, font, drawAlign);

                    if (shadow_visible) {
                        float shadowRadius = 5f;
                        float shadowDx = 3f;
                        float shadowDy = 3f;
                        textOverlay.setTextShadow(true, shadow_color, shadowRadius, shadowDx, shadowDy);
                    }
                    if (glow_visible) {
                        float glowRadius = 8f;
                        textOverlay.setTextGlow(true, glow_color, glowRadius);
                    }
                    if (outline_visible) {
                        float outlineWidth = 1f;
                        textOverlay.setTextOutline(true, outline_color, outlineWidth);
                    }
                    textOverlay.setTextOverlayId(textOverlayItem.getId());

                    break;
                }
                case "image" :
                {
                    int id = Integer.parseInt(overlay.get(TAG_ID));
                    String pos = overlay.get(TAG_POSITION).replace(" ", "");
                    String[] position = pos.split(",");

                    if (position == null) {
                        return "Wrong position data of title";
                    }

                    int left, top, right, bottom;

                    left = Integer.parseInt(position[0]);
                    top = Integer.parseInt(position[1]);

                    right = Integer.parseInt(position[2]);
                    bottom = Integer.parseInt(position[3]);

                    int x = (left + right) / 2;
                    int y = (top + bottom) / 2;

                    int w = right - left;
                    int h = bottom - top;

                    int start_time = Integer.parseInt(overlay.get(TAG_START_TIME));
                    int duration = Integer.parseInt(overlay.get(TAG_DURATION));

                    if ((apply == Priority.START && start_time < 0) ||
                            (apply == Priority.END && start_time >= 0)) {
                        if (LL.D)
                            Log.d(TAG, String.format("Apply image res skip(%s %d)", apply.name(), start_time));
                        continue;
                    }

                    if (start_time < 0) {
                        if (project.getTotalTime() < duration) {
                            start_time = 0;
                            duration = project.getTotalTime();
                        } else {
                            start_time = project.getTotalTime() - duration;
                        }
                    }

                    String res = overlay.get(TAG_IMAGE_RES);

                    nexOverlayImage oli = getOverlayImage(res);

                    textOverlayItem = new nexOverlayItem(oli, x, y, start_time, start_time + duration);
                    // textOverlayItem.setRotate();
                    break;
                }
            }

            if( textOverlayItem == null )
                continue;

            if( overlay.containsKey(TAG_ROTATE) )
            {
                float rotate = Float.parseFloat(overlay.get(TAG_ROTATE));
                textOverlayItem.setRotate(rotate);
            }

            // textOverlayItem.setp

            if( overlay.containsKey(TAG_SCALE) )
            {
                String scales = overlay.get(TAG_SCALE).replace(" ", "");
                String[] scale = scales.split(",");

                if( scale != null ) {
                    float xScale = Float.parseFloat(scale[0]);
                    float yScale = Float.parseFloat(scale[1]);
                    float zScale = Float.parseFloat(scale[2]);

                    textOverlayItem.setScale(xScale, yScale, zScale);
                }
            }

            textOverlayItem.clearAnimate();

            String animation = overlay.get(TAG_ANIMATION);

            if( main_olay && overlay.containsKey(TAG_ADJUST_ANIMATION_NON_SUB) )
                animation = overlay.get(TAG_ADJUST_ANIMATION_NON_SUB);

            try {
                JSONArray jsonarray = new JSONArray(animation);

                for( int i = 0; i < jsonarray.length(); i++ ) {
                    JSONObject ani = jsonarray.getJSONObject(i);

                    String ani_type = ani.getString(TAG_TYPE);
                    JSONArray values = ani.getJSONArray(TAG_VALUES);

                    switch(ani_type) {
                        case "move":
                            for( int j = 0; j < values.length(); j++ ) {
                                JSONObject val = values.getJSONObject(j);

                                String ani_motion = val.getString(TAG_MOTION_TYPE);

                                TimeInterpolator interpolator = getInterpolation(ani_motion);

                                int ani_start = Integer.parseInt(val.getString(TAG_START_TIME));
                                int ani_end = Integer.parseInt(val.getString(TAG_END_TIME));

                                final float start_x = Float.parseFloat(val.getString(TAG_START_X));
                                final float end_x = Float.parseFloat(val.getString(TAG_END_X));
                                final float start_y = Float.parseFloat(val.getString(TAG_START_Y));
                                final float end_y = Float.parseFloat(val.getString(TAG_END_Y));

                                final nexOverlayItem over = textOverlayItem;

                                over.addAnimate(nexAnimate.getMove(ani_start, ani_end-ani_start, new nexAnimate.MoveTrackingPath() {
                                    @Override
                                    public float getTranslatePosition(int coordinate, float timeRatio) {

                                        if( coordinate == nexAnimate.kCoordinateX ) {

                                            float gap = Math.abs(start_x - end_x) * timeRatio;
                                            gap = start_x < end_x ? gap : -gap;

                                            return start_x + gap;
                                        }
                                        if( coordinate == nexAnimate.kCoordinateY ) {

                                            float gap = Math.abs(start_y - end_y) * timeRatio;
                                            gap = start_y < end_y ? gap : -gap;

                                            return start_y + gap;
                                        }
                                        return 0;
                                    }
                                }).setInterpolator(interpolator));

                            }
                            break;
                        case "alpha":
                            for( int j = 0; j < values.length(); j++ ) {
                                JSONObject val = values.getJSONObject(j);

                                String ani_motion = val.getString(TAG_MOTION_TYPE);
                                TimeInterpolator interpolator = getInterpolation(ani_motion);

                                int ani_start = Integer.parseInt(val.getString(TAG_START_TIME));
                                int ani_end = Integer.parseInt(val.getString(TAG_END_TIME));

                                float start = Float.parseFloat(val.getString(TAG_START));
                                float end = Float.parseFloat(val.getString(TAG_END));

                                textOverlayItem.addAnimate(nexAnimate.getAlpha(ani_start, ani_end - ani_start, start, end).setInterpolator(interpolator));
                            }
                            break;
                        case "scale":
                            for( int j = 0; j < values.length(); j++ ) {
                                JSONObject val = values.getJSONObject(j);

                                String ani_motion = val.getString(TAG_MOTION_TYPE);
                                TimeInterpolator interpolator = getInterpolation(ani_motion);

                                int ani_start = Integer.parseInt(val.getString(TAG_START_TIME));
                                int ani_end = Integer.parseInt(val.getString(TAG_END_TIME));

                                float start_x = Float.parseFloat(val.getString(TAG_START_X));
                                float end_x = Float.parseFloat(val.getString(TAG_END_X));
                                float start_y = Float.parseFloat(val.getString(TAG_START_Y));
                                float end_y = Float.parseFloat(val.getString(TAG_END_Y));

                                textOverlayItem.addAnimate(nexAnimate.getScale(ani_start, ani_end - ani_start, start_x, start_y, end_x, end_y).setInterpolator(interpolator));
                            }
                            break;
                        case "rotate":
                            for( int j = 0; j < values.length(); j++ ) {
                                JSONObject val = values.getJSONObject(j);

                                String ani_motion = val.getString(TAG_MOTION_TYPE);
                                TimeInterpolator interpolator = getInterpolation(ani_motion);
                                int ani_start = Integer.parseInt(val.getString(TAG_START_TIME));
                                int ani_end = Integer.parseInt(val.getString(TAG_END_TIME));

                                boolean clockwise = Integer.parseInt(val.getString(TAG_CLOCKWISE)) == 1;
                                float rotatedegree = Float.parseFloat(val.getString(TAG_ROTATE_DEGREE));

                                textOverlayItem.addAnimate(nexAnimate.getRotate(ani_start, ani_end - ani_start, clockwise, rotatedegree, null).setInterpolator(interpolator));
                            }
                            break;

                    }
                }
            } catch (JSONException e) {
                e.printStackTrace();

                if(LL.D) Log.d(TAG, "applyOverlayAsset failed" + e.getMessage());
                return e.getMessage();
            }

            textOverlayItem.setOverlayTitle(true);
            project.addOverlay(textOverlayItem);
        }

        return null;
    }

    public String clearOverlayAsset(nexProject project) {
        if( project == null ) return "Null project";
        List<Integer> delIds = new ArrayList<>();
        List<nexOverlayItem> overlays = project.getOverlayItems();

        for( nexOverlayItem item : overlays )
        {
            if( item.getOverlayTitle() )
                delIds.add(item.getId());
        }

        for(int i :  delIds ){
            project.removeOverlay(i);
        }
        return null;
    }

    List<nexOverlayInfoTitle> getOverlayInfoTitle() {

        List<nexOverlayInfoTitle> infos = new ArrayList<>();
        for(nexOverlayEDL edl : mEDLList) {

            for( nexOverlayTitleInfo info : edl.title_infos ) {
                if( info.title_rect != null )
                    infos.add(info);
            }
        }
        return infos;
    }

    nexOverlayInfoTitle getOverlayInfoTitle(int time, float x, float y) {

        for(nexOverlayEDL edl : mEDLList) {

            nexOverlayTitleInfo info = edl.getInfoTitle(time, x, y);
            if( info != null ) return info;
        }
        return null;
    }

    int getFontSize(int id, List<nexOverlayManager.nexTitleInfo> titlelist, int defaultFontSize){
        for( nexOverlayManager.nexTitleInfo info : titlelist ) {
            if( info.getId() == id )
            {
                int size = info.getFontSize();
                Log.d(TAG,"getFontSize() id="+id+", FontSize="+size);
                if( size <= 0 ){
                    return defaultFontSize;
                }
                return size;
            }
        }
        Log.d(TAG,"getFontSize() id="+id+", defaultFontSize="+defaultFontSize);
        return defaultFontSize;
    }

    String getFont(int id, List<nexOverlayManager.nexTitleInfo> titlelist, String defaultFont)
    {
        for( nexOverlayManager.nexTitleInfo info : titlelist ) {
            if( info.getId() == id )
            {
                return info.getFontID();
            }
        }
        return defaultFont;
    }

    String getText(int id, List<nexOverlayManager.nexTitleInfo> titlelist, String defaultText)
    {
        for( nexOverlayManager.nexTitleInfo info : titlelist ) {
            if( info.getId() == id )
            {
                return info.getText();
            }
        }
        return defaultText;
    }

    String getText(String group, List<nexOverlayManager.nexTitleInfo> titlelist)
    {
        for( nexOverlayManager.nexTitleInfo info : titlelist ) {
            if( info.getGroup().startsWith(group.substring(0, 1)) )
            {
                if( (info.getText() == null || info.getText().length() <= 0) )
                    continue;

                return info.getText();
            }
        }
        return null;
    }

    int getTextSize(String group, List<nexOverlayManager.nexTitleInfo> titlelist,int default_size)
    {
        for( nexOverlayManager.nexTitleInfo info : titlelist ) {
            if( info.getGroup().startsWith(group.substring(0, 1)) )
            {
                if( (info.getText() == null || info.getText().length() <= 0) )
                    continue;

                return info.getFontSize();
            }
        }
        return default_size;
    }


    boolean hasEmptyTitleInGroup(List<nexOverlayManager.nexTitleInfo> titlelist, String group)
    {
        boolean adjust = false;
        for( nexOverlayManager.nexTitleInfo info : titlelist ) {
            if( info.getGroup().startsWith(group.substring(0, 1)) && (info.getText() == null || info.getText().length() <= 0) )
            {
                adjust = true;
            }
        }
        return adjust;
    }

    int getOverlayTime(boolean start)
    {
        int time = 0;
        for(HashMap<String, String> overlay : mOverlayList ) {
            int start_time = Integer.parseInt(overlay.get(TAG_START_TIME));
            int duration = Integer.parseInt(overlay.get(TAG_DURATION));

            if( start ) {
                if( start_time < 0 )
                    continue;

                time = time < duration ? duration : time;
                continue;
            }

            if( start_time >= 0 )
                continue;

            time = time < duration ? duration : time;
        }
        return time;
    }


    Priority getCheckPriority(nexProject project) {
        int projectTime = project.getTotalTime();

        int iStart = getOverlayTime(true);
        int iEnd = getOverlayTime(false);

        if (projectTime >= iStart + iEnd)
            return Priority.START_END;

        if (mOverlayPriority.equals("start"))
            return Priority.START;

        return Priority.END;
    }

    TimeInterpolator getInterpolation(String motion)
    {
        TimeInterpolator interpolator = null;
        switch( motion )
        {
            case "AccelerateInterpolator":
                interpolator = new AccelerateInterpolator();
                break;
            case "AnticipateInterpolator":
                interpolator = new AnticipateInterpolator();
                break;
            case "AnticipateOvershootInterpolator":
                interpolator = new AnticipateOvershootInterpolator();
                break;
            case "BounceInterpolator":
                interpolator = new BounceInterpolator();
                break;
            case "CycleInterpolator":
                interpolator = new CycleInterpolator(1);
                break;
            case "DecelerateInterpolator":
                interpolator = new DecelerateInterpolator();
                break;
            case "LinearInterpolator":
                interpolator = new LinearInterpolator();
                break;
            case "OvershootInterpolator":
                interpolator = new OvershootInterpolator();
                break;

            // case "AccelerateDecelerateInterpolator":
            default:
                interpolator = new AccelerateDecelerateInterpolator();
                break;

        };
        return interpolator;
    }

    private nexOverlayImage getOverlayImage(String overlayImageId ){
        ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(overlayImageId);
        if( info == null ){
            return null;
        }

        if( info.getCategory() == ItemCategory.overlay && info.getType() == ItemType.overlay ) {
            return new nexOverlayImage(overlayImageId, true);
        }
        return null;
    }

}
