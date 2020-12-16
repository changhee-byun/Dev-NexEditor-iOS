/******************************************************************************
 * File Name        : nexOverlayEDL.java
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

import android.graphics.RectF;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

class nexOverlayEDL {
    private static String TAG="nexOverlayTitleEDL";

    private static final String TAG_EDL = "edl";
    private static final String TAG_EDL_RATIO = "edl_ratio";
    private static final String TAG_EDL_HORIZONTAL_ALIGN = "edl_horizontal_align";
    private static final String TAG_EDL_VERTICAL_ALIGN = "edl_vertical_align";
    private static final String TAG_EDL_X = "edl_x";
    private static final String TAG_EDL_Y = "edl_y";
    private static final String TAG_EDL_WIDTH = "edl_width";
    private static final String TAG_EDL_HEIGHT = "edl_height";
    private static final String TAG_EDL_DISPLAY = "edl_display";
    private static final String TAG_EDL_START = "edl_start";
    private static final String TAG_EDL_DURATION = "edl_duration";

    private static final String TAG_TITLE_INFO = "title_info";

    String edl;
    float ratio;
    int anchor = 4;
    float x;
    float y;
    float width;
    float height;
    String display = "";
    int start;
    int duration;

    nexOverlayFilter apply_filter = null;
    int apply_start = 0;
    int apply_duration = 0;
    RectF apply_rect;

    ArrayList<nexOverlayTitleInfo> title_infos = new ArrayList<>();

    protected int getPosX(int overlayWidth, int edlWidth) {
        int pos = (int)(overlayWidth * x);

        switch (anchor) {
            case nexOverlayItem.AnchorPoint_LeftTop:
            case nexOverlayItem.AnchorPoint_LeftMiddle:
            case nexOverlayItem.AnchorPoint_LeftBottom:
                pos += (edlWidth/2);
                break;
            case nexOverlayItem.AnchorPoint_RightTop:
            case nexOverlayItem.AnchorPoint_RightMiddle:
            case nexOverlayItem.AnchorPoint_RightBottom:
                pos -= (edlWidth/2);
                break;
        }

        return pos;
    }

    protected int getPosY(int overlayHeight, int edlHeight) {
        int pos = (int)(overlayHeight * y);

        switch (anchor) {
            case nexOverlayItem.AnchorPoint_LeftTop:
            case nexOverlayItem.AnchorPoint_MiddleTop:
            case nexOverlayItem.AnchorPoint_RightTop:
                pos += (edlHeight/2);
                break;
            case nexOverlayItem.AnchorPoint_LeftBottom:
            case nexOverlayItem.AnchorPoint_MiddleBottom:
            case nexOverlayItem.AnchorPoint_RightBottom:
                pos -= (edlHeight/2);
                break;
        }

        return pos;
    }

    protected void updateOption(nexEffectOptions options) {
        for(nexOverlayTitleInfo info : title_infos) {
            nexEffectOptions.TextOpt textOpt = (nexEffectOptions.TextOpt)options.getOptionEndWithId(info.getId());
            if( textOpt != null ) {
                textOpt.setText(info.getTitle(""));
            }

            String font = info.getTitleFont();
            if( font != null && font.length() > 0 ) {
                nexEffectOptions.TypefaceOpt fontOpt = (nexEffectOptions.TypefaceOpt)options.getOptionEndWithId(info.getId() + "_font");
                if( fontOpt != null ) {
                    fontOpt.setTypeface(font);
                }
            }

            int fill = info.getTitleFillColor();
            if( fill != 0 ) {
                nexEffectOptions.ColorOpt colorFillOpt = (nexEffectOptions.ColorOpt)options.getOptionEndWithId(info.getId() + "_fill_color");
                if (colorFillOpt != null) {
                    colorFillOpt.setARGBColor(fill);
                }
            }

            int stroke = info.getTitleStrokeColor();
            if( stroke != 0 ) {
                nexEffectOptions.ColorOpt colorStrokeOpt = (nexEffectOptions.ColorOpt)options.getOptionEndWithId(info.getId() + "_stroke_color");
                if (colorStrokeOpt != null) {
                    colorStrokeOpt.setARGBColor(stroke);
                }
            }

            int dropShadow = info.getTitleDropShadowColor();
            if( dropShadow != 0 ) {
                nexEffectOptions.ColorOpt colorDropShadowOpt = (nexEffectOptions.ColorOpt)options.getOptionEndWithId(info.getId() + "_dropshadow_color");
                if (colorDropShadowOpt != null) {
                    colorDropShadowOpt.setARGBColor(dropShadow);
                }
            }
        }
    }

    protected void setApplyInfo(nexOverlayFilter filter, int start, int duration, int x, int y, int width, int height, int displayWidth, int displayHeight) {
        apply_filter = filter;
        apply_start = start;
        apply_duration = duration;
        float x1 = x - (width / 2);
        float y1 = y - (height / 2);
        float x2 = (x1+width)/displayWidth;
        float y2 = (y1+height)/displayHeight;
        apply_rect = new RectF(x1/displayWidth, y1/displayHeight, x2, y2);

        updateOption(filter.getEffectOption());
        Log.d(TAG, "setApplyInfo EDL:" + apply_rect);
        for(nexOverlayTitleInfo info : title_infos) {
            info.setApplyInfo((int)x1, (int)y1, width, height, displayWidth, displayHeight);
        }
    }

    protected void resetApplyInfo() {
        apply_filter = null;
        apply_start = 0;
        apply_duration = 0;
        apply_rect = null;
        for(nexOverlayTitleInfo info : title_infos) {
            info.resetApplyInfo();
        }
    }

    protected nexOverlayTitleInfo getInfoTitle(int time, float x, float y) {

        Log.d(TAG, String.format("getInfoTitle(%d %f %f : %d %d)", time, x, y, apply_start, apply_duration));

        if( time < apply_start || time > (apply_start+apply_duration) ) return null;
        if( apply_rect == null || apply_rect.contains(x, y) == false ) return null;


        for(nexOverlayTitleInfo info : title_infos) {
            if( info.title_rect == null ) continue;

            if( info.title_rect.contains(x, y) )
                return info;
        }
        return null;
    }


    String parseOverlayTitleInfo(JSONObject object) {
        try {

            if( object.has(TAG_EDL) ) edl = object.getString(TAG_EDL);
            if( object.has(TAG_EDL_RATIO) ) ratio = Float.parseFloat(object.getString(TAG_EDL_RATIO));
            String horizontalAlign = null;
            String verticalAlign = null;
            if( object.has(TAG_EDL_HORIZONTAL_ALIGN) ) horizontalAlign = object.getString(TAG_EDL_HORIZONTAL_ALIGN);
            if( object.has(TAG_EDL_VERTICAL_ALIGN) ) verticalAlign = object.getString(TAG_EDL_VERTICAL_ALIGN);

            if( horizontalAlign != null && verticalAlign != null ) {
                anchor = 3;
                if( verticalAlign.compareTo("top") == 0 ) {
                    anchor = 0;
                }
                else if( verticalAlign.compareTo("bottom") == 0 ) {
                    anchor = 6;
                }

                if( horizontalAlign.compareTo("left") == 0 ) {
                }
                else if( horizontalAlign.compareTo("right") == 0 ) {
                    anchor += 2;
                }
                else {
                    anchor += 1;
                }
            }

            if( object.has(TAG_EDL_X) ) x = Float.parseFloat(object.getString(TAG_EDL_X));
            if( object.has(TAG_EDL_Y) ) y = Float.parseFloat(object.getString(TAG_EDL_Y));
            if( object.has(TAG_EDL_WIDTH) ) width = Float.parseFloat(object.getString(TAG_EDL_WIDTH));
            if( object.has(TAG_EDL_HEIGHT) ) height = Float.parseFloat(object.getString(TAG_EDL_HEIGHT));

            if( object.has(TAG_EDL_DISPLAY) ) display = object.getString(TAG_EDL_DISPLAY);

            if( object.has(TAG_EDL_START) ) start = Integer.parseInt(object.getString(TAG_EDL_START));
            if( object.has(TAG_EDL_DURATION) ) duration = Integer.parseInt(object.getString(TAG_EDL_DURATION));

            if( start == -1 && duration == -1 ) {
                return "Overlay time error";
            }

            if( object.has(TAG_TITLE_INFO) ) {
                JSONArray titles = object.getJSONArray(TAG_TITLE_INFO);
                for( int i = 0; i < titles.length(); i++ )
                {
                    JSONObject title_info = titles.getJSONObject(i);
                    nexOverlayTitleInfo info = new nexOverlayTitleInfo();
                    info.parseOverlayTitleInfo(title_info, new nexOverlayTitleInfo.InfoChangedListener() {
                        @Override
                        public void changeTitleInfo() {
                            if( apply_filter != null )
                                updateOption(apply_filter.getEffectOption());
                        }
                    });
                    title_infos.add(info);
                }
            }

        } catch (JSONException e) {
            e.printStackTrace();
            Log.d(TAG, "parse Collage failed : " + e.getMessage());
            return e.getMessage();
        }

        return null;
    }
}
