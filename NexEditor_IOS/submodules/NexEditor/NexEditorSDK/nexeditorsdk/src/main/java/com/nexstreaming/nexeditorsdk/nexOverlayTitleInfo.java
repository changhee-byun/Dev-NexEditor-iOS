/******************************************************************************
 * File Name        : nexOverlayTitleInfo.java
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

import android.graphics.Color;
import android.graphics.RectF;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Locale;

class nexOverlayTitleInfo implements nexOverlayInfoTitle {
    private static String TAG="nexOverlayTitleInfo";

    private static final String TAG_TITLE_ID = "title_id";
    private static final String TAG_TITLE_DEFAULT = "title_default";
    private static final String TAG_TITLE_MAX_LENGTH = "title_max_length";
    private static final String TAG_TITLE_RECOMMEND_LENGTH = "title_recommend_length";
    private static final String TAG_TITLE_MAX_LINE = "title_max_line";
    private static final String TAG_TITLE_X = "title_x";
    private static final String TAG_TITLE_Y = "title_y";
    private static final String TAG_TITLE_WIDTH = "title_width";
    private static final String TAG_TITLE_HEIGHT = "title_height";

    String title_id;
    HashMap<String, String> title_default = new HashMap<>();
    int title_max_length;
    int title_recommend_length;
    int title_max_line;
    float title_x;
    float title_y;
    float title_width;
    float title_height;

    private String userText = null;
    private String userFont = "";
    private String userFillColor = "";
    private String userStrokeColor = "";
    private String userDropShadowColor = "";

    InfoChangedListener changeLinstener = null;
    RectF title_rect;

    protected interface InfoChangedListener {
        public void changeTitleInfo();
    }

    public RectF getRect() {
        return title_rect;
    }

    public String getId() {
        return title_id;
    }

    public String getTitle() {
        if( userText != null && userText.length() > 0 )
            return userText;

        String defaultText = "";

        String locale = Locale.getDefault().getLanguage();

        if( title_default.containsKey(locale) )
        {
            defaultText = title_default.get(locale);
        }
        else if( title_default.containsKey("en") ) {
            defaultText = title_default.get("en");
        }

        return defaultText;
    }

    public String getTitle(String locale) {
        if( userText != null && userText.length() > 0 )
            return userText;

        String defaultText = "";

        if( locale != null && title_default.containsKey(locale))
        {
            defaultText = title_default.get(locale);
        }
        else
        {
            locale = Locale.getDefault().getLanguage();

            Log.d(TAG, "default locale" + locale);

            if( title_default.containsKey(locale) )
            {
                defaultText = title_default.get(locale);
            }
            else if( title_default.containsKey("en") ) {
                defaultText = title_default.get("en");
            }
        }

        return defaultText;
    }

    public boolean setTitle(String text) {
        this.userText = text;
        if( changeLinstener != null)
            changeLinstener.changeTitleInfo();

        return true;
    }

    public String getTitleFont() {
        return userFont;
    }

    public boolean setTitleFont(String id)
    {
        this.userFont = id;
        if( changeLinstener != null)
            changeLinstener.changeTitleInfo();
        return true;
    }

    public int getTitleFillColor() {
        if( userFillColor != null && userFillColor.length() > 0 ) {
            String color = userFillColor.substring(1, userFillColor.length());
            int c = Integer.parseInt(color);
            int red = Color.alpha(c);
            int green = Color.red(c);
            int blue = Color.green(c);
            int alpha = Color.blue(c);

            return Color.argb(alpha, red, green, blue);
        }
        return 0;
    }

    public boolean setTitleFillColor(int color)
    {
        int alpha = Color.alpha(color);
        int red = Color.red(color);
        int green = Color.green(color);
        int blue = Color.blue(color);

        if( Color.alpha(color) == 0 ) {
            userFillColor = "";
        }
        else {
            userFillColor = String.format("#%8X", Color.argb(red, green, blue, alpha));
        }

        if( changeLinstener != null)
            changeLinstener.changeTitleInfo();

        return true;
    }

    public int getTitleStrokeColor() {
        if( userFillColor != null && userFillColor.length() > 0 ) {
            String color = userFillColor.substring(1, userFillColor.length());
            int c = Integer.parseInt(color);
            int red = Color.alpha(c);
            int green = Color.red(c);
            int blue = Color.green(c);
            int alpha = Color.blue(c);

            return Color.argb(alpha, red, green, blue);
        }
        return 0;
    }

    public boolean setTitleStrokeColor(int color)
    {
        int alpha = Color.alpha(color);
        int red = Color.red(color);
        int green = Color.green(color);
        int blue = Color.blue(color);

        if( Color.alpha(color) == 0 ) {
            userStrokeColor = "";
        }
        else {
            userStrokeColor = String.format("#%8X", Color.argb(red, green, blue, alpha));
        }

        if( changeLinstener != null)
            changeLinstener.changeTitleInfo();

        return true;
    }

    public int getTitleDropShadowColor() {
        if( userFillColor != null && userFillColor.length() > 0 ) {
            String color = userFillColor.substring(1, userFillColor.length());
            int c = Integer.parseInt(color);

            int red = Color.alpha(c);
            int green = Color.red(c);
            int blue = Color.green(c);
            int alpha = Color.blue(c);

            return Color.argb(alpha, red, green, blue);
        }
        return 0;
    }

    public boolean setTitleDropShadowColor(int color)
    {
        int alpha = Color.alpha(color);
        int red = Color.red(color);
        int green = Color.green(color);
        int blue = Color.blue(color);

        if( Color.alpha(color) == 0 ) {
            userDropShadowColor = "";
        }
        else {
            userDropShadowColor = String.format("#%8X", Color.argb(red, green, blue, alpha));
        }

        if( changeLinstener != null)
            changeLinstener.changeTitleInfo();
        return true;
    }

    public int getTitleMaxLength() {
        return title_max_length;
    }

    public int getTitleRecommendLength() {
        return title_recommend_length;
    }

    public int getTitleMaxLines() {
        return title_max_line;
    }

    String parseOverlayTitleInfo(JSONObject object, InfoChangedListener listener) {
        try {
            if( object.has(TAG_TITLE_ID) ) title_id = object.getString(TAG_TITLE_ID);
            if( object.has(TAG_TITLE_DEFAULT) ) {
                JSONObject title_defaults = object.getJSONObject(TAG_TITLE_DEFAULT);
                Iterator<String> keys = title_defaults.keys();
                while( keys.hasNext() ) {
                    String key = keys.next();
                    title_default.put(key, title_defaults.getString(key));
                }
            }

            if( object.has(TAG_TITLE_MAX_LENGTH) ) title_max_length = Integer.parseInt(object.getString(TAG_TITLE_MAX_LENGTH));
            if( object.has(TAG_TITLE_RECOMMEND_LENGTH) ) title_recommend_length = Integer.parseInt(object.getString(TAG_TITLE_RECOMMEND_LENGTH));
            if( object.has(TAG_TITLE_MAX_LINE) ) title_max_line = Integer.parseInt(object.getString(TAG_TITLE_MAX_LINE));

            if( object.has(TAG_TITLE_X) ) title_x = Float.parseFloat(object.getString(TAG_TITLE_X));
            if( object.has(TAG_TITLE_Y) ) title_y = Float.parseFloat(object.getString(TAG_TITLE_Y));
            if( object.has(TAG_TITLE_WIDTH) ) title_width = Float.parseFloat(object.getString(TAG_TITLE_WIDTH));
            if( object.has(TAG_TITLE_HEIGHT) ) title_height = Float.parseFloat(object.getString(TAG_TITLE_HEIGHT));

        } catch (JSONException e) {
            e.printStackTrace();
            Log.d(TAG, "parse Collage failed : " + e.getMessage());
            return e.getMessage();
        }

        changeLinstener = listener;
        return null;
    }

    protected void setApplyInfo(int x, int y, int width, int height, int displayWidth, int displayHeight) {
        float t_x = (width*title_x) + x;
        float t_w = (width*title_width);
        float t_y = (height*title_y) + y;
        float t_h = (height*title_height);

        title_rect = new RectF(t_x/displayWidth, t_y/displayHeight, (t_x+t_w)/displayWidth, (t_y+t_h)/displayHeight);

        // Log.d(TAG, "setApplyInfo title_info : " + title_rect);
    }

    protected void resetApplyInfo() {
        title_rect = null;
    }
}
