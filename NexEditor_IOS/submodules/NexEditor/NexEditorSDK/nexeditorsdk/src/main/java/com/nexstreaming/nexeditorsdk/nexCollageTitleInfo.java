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

import android.graphics.Color;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;

class nexCollageTitleInfo implements nexCollageInfo, nexCollageInfoTitle {
    private static String TAG="nexCollageTitleInfo";

    private static final String TAG_START = "start";
    private static final String TAG_END = "end";
    private static final String TAG_POSITION = "position";
    private static final String TAG_TITLE_ID = "title_id";

    private static final String TAG_TITLE_TYPE = "title_type";
    private static final String TAG_TITLE_DEFAULT = "title_default";
    private static final String TAG_DRAW_ID = "draw_id";
    private static final String TAG_MAX_LENGTH = "max_length";
    private static final String TAG_RECOMMEND_LENGTH = "recommend_length";
    private static final String TAG_TITLE_MAX_LINES = "title_max_lines";

    enum Type {
        User, System
    }

    private float start;
    private float end;
    private RectF position;
    private List<PointF> polygon = new ArrayList<>();

    private String titleId;
    private Type titleType;
    private String drawId;
    private int maxLength = 20;
    private int recommendLength = 20;
    private int titleMaxLines = 1;

    private Map<String, String> defaultTitle = new HashMap<String, String>();

    private String userText = null;
    private String userFont = "";
    private String userFillColor = "";
    private String userStrokeColor = "";
    private String userDropShadowColor = "";

    protected nexCollage.CollageInfoChangedListener infoChanged;

    protected void setInfoChangedListener(nexCollage.CollageInfoChangedListener listener) {
        infoChanged = listener;
    }

    public String getId() {
        return titleId;
    }

    public RectF getRectangle() {
        return position;
    }

    public void setPosition(RectF position) {
        this.position = position;
    }

    public List<PointF> getPositions() {
        return polygon;
    }

    public String getTitle(String locale) {
        if( userText != null )//&& userText.length() > 0 )
            return userText;

        String defaultText = "";

        if( locale != null && defaultTitle.containsKey(locale))
        {
            defaultText = defaultTitle.get(locale);
        }
        else
        {
            locale = Locale.getDefault().getLanguage();

            if( defaultTitle.containsKey(locale) )
            {
                defaultText = defaultTitle.get(locale);
            }
            else if( defaultTitle.containsKey("en") ) {
                defaultText = defaultTitle.get("en");
            }
        }

        if( defaultText.startsWith("@content=") ) {
            if( infoChanged != null ) {
                String sub = defaultText.substring("@content=".length());
                defaultText = infoChanged.TitleInfoContentTime(drawId, sub);
            }
        }
        else if( defaultText.startsWith("@collage=") ) {
            if( infoChanged != null ) {
                String sub = defaultText.substring("@collage=".length());
                defaultText = infoChanged.CollageTime(sub);
            }
        }

        return defaultText;
    }

    public boolean setTitle(String text) {
        this.userText = text;
        if( infoChanged != null )
            infoChanged.TitleInfoChanged();
        return true;
    }

    public String getTitleFont() {
        return userFont;
    }

    public boolean setTitleFont(String id)
    {
        userFont = id;
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
        return true;
    }

    public int getTitleMaxLength() {
        return maxLength;
    }

    public int getTitleRecommendLength() {
        return recommendLength;
    }

    public int getTitleMaxLines() {
        return titleMaxLines;
    }

    public float getStart() {
        return start;
    }

    public float getEnd() {
        return end;
    }

    public RectF getPosition() {
        return position;
    }

    public boolean isUserTitle() {
        return titleType == Type.User;
    }

    public boolean isSystemTitle() {
        return titleType == Type.System;
    }

    public String getTitle() {
        StringBuilder b = new StringBuilder();

        try {
            if( userFont.length() > 0 ) {
                b.append(URLEncoder.encode(getId()+"_font", "UTF-8"));
                b.append("=");
                b.append(URLEncoder.encode("fontid:"+userFont, "UTF-8"));
                b.append("&");
            }

            // userFillColor = String.format("#%8X", Color.argb(155, 255, 0, 155));
            if( userFillColor.length() > 0 ) {
                b.append(URLEncoder.encode(getId()+"_fill_color", "UTF-8"));
                b.append("=");
                b.append(URLEncoder.encode(userFillColor, "UTF-8"));
                b.append("&");
            }
            if( userStrokeColor.length() > 0 ) {
                b.append(URLEncoder.encode(getId()+"_stroke_color", "UTF-8"));
                b.append("=");
                b.append(URLEncoder.encode(userStrokeColor, "UTF-8"));
                b.append("&");
            }
            if( userDropShadowColor.length() > 0 ) {
                b.append(URLEncoder.encode(getId()+"_dropshadow_color", "UTF-8"));
                b.append("=");
                b.append(URLEncoder.encode(userDropShadowColor, "UTF-8"));
                b.append("&");
            }

            b.append(URLEncoder.encode(getId(), "UTF-8"));
            b.append("=");

            String local = null;
            if( userText == null ){//|| userText.length() <= 0 ) {
                local = Locale.getDefault().getLanguage();
                String ds = null;

                if( defaultTitle.containsKey(local) ) {
                    ds = defaultTitle.get(local);
                }
                else {
                    ds = defaultTitle.get("en");
                }

                if( ds.startsWith("@content=") ) {
                    if( infoChanged != null ) {
                        String sub = ds.substring("@content=".length());
                        ds = infoChanged.TitleInfoContentTime(drawId, sub);
                    }
                    else {
                        ds = "";
                    }

                }
                else if( ds.startsWith("@collage=") ) {
                    if( infoChanged != null ) {
                        String sub = ds.substring("@collage=".length());
                        ds = infoChanged.CollageTime(sub);
                    }
                    else {
                        ds = "";
                    }
                }

                b.append(URLEncoder.encode(ds, "UTF-8"));
            }
            else {
                if( userText.length() <= 0 ){
                    b.append(URLEncoder.encode(" ", "UTF-8"));
                }else {
                    b.append(URLEncoder.encode(userText, "UTF-8"));
                }
            }
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }

        return b.toString();
    }

    float isLeft( PointF P0, PointF P1, PointF P2 )
    {
        return ( (P1.x - P0.x) * (P2.y - P0.y) - (P2.x -  P0.x) * (P1.y - P0.y) );
    }

    int wn_PnPoly( PointF P, List<PointF> poligons)
    {
        int    wn = 0;    // the  winding number counter

        for( int i = 0; i < poligons.size() - 1; i++) {
            if (poligons.get(i).y <= P.y) {          // start y <= P.y
                if (poligons.get(i+1).y  > P.y)      // an upward crossing
                    if (isLeft( poligons.get(i), poligons.get(i+1), P) > 0)  // P left of  edge
                        ++wn;            // have  a valid up intersect
            }
            else {                        // start y > P.y (no test needed)
                if (poligons.get(i+1).y  <= P.y)     // a downward crossing
                    if (isLeft( poligons.get(i), poligons.get(i+1), P) < 0)  // P right of  edge
                        --wn;            // have  a valid down intersect
            }
        }
        return wn;
    }

    public boolean isContains(float x, float y) {

        if( polygon != null && polygon.size() > 0 ) {

            PointF p = new PointF(x, y);
            int wn = wn_PnPoly(p, polygon);
            if( wn > 0 )
                return true;
        }

        if( position != null )
            return position.contains(x, y);

        return false;
    }

    public String getDrawId() {
        return drawId;
    }

    String parseTitleInfo(JSONObject object) {
        try {
            start = Float.parseFloat(object.getString(TAG_START));
            end = Float.parseFloat(object.getString(TAG_END));
            if( object.has(TAG_POSITION) ) {
                String pos = object.getString(TAG_POSITION).replace(" ", "");
                String[] position = pos.split(",");

                if (position == null) {
                    return "Wrong position data of titleinfo";
                }

                polygon.clear();
                if( position.length == 4 ) {
                    float left = Float.parseFloat(position[0]);
                    float top = Float.parseFloat(position[1]);

                    float right = Float.parseFloat(position[2]);
                    float bottom = Float.parseFloat(position[3]);

                    PointF lt = new PointF(left, top);
                    PointF rt = new PointF(right, top);
                    PointF rb = new PointF(right, bottom);
                    PointF lb = new PointF(left, bottom);

                    polygon.add(lt);
                    polygon.add(rt);
                    polygon.add(rb);
                    polygon.add(lb);
                    polygon.add(lt);

                    this.position = new RectF(left, top, right, bottom);
                }
                else {
                    float left = Float.MAX_VALUE;
                    float top = Float.MAX_VALUE;
                    float right = Float.MIN_VALUE;
                    float bottom = Float.MIN_VALUE;
                    for(int i = 0; i < position.length; i+=2 ) {
                        float x = Float.parseFloat(position[i]);
                        float y = Float.parseFloat(position[i+1]);

                        left = left > x ? x : left;
                        right = right < x ? x : right;
                        top = top > y ? y : top;
                        bottom = bottom < y ? y : bottom;

                        PointF pointF = new PointF(x, y);
                        // Log.d(TAG, "Position : " + pointF.toString());
                        polygon.add(pointF);
                    }

                    this.position = new RectF(left, top, right, bottom);
                }
            }

            titleId = object.getString(TAG_TITLE_ID);

            titleType = object.getString(TAG_TITLE_TYPE).compareTo("user") == 0 ? Type.User : Type.System;

            if( object.has(TAG_DRAW_ID) ) {
                drawId = object.getString(TAG_DRAW_ID);
            }

            JSONObject title_defaults = object.getJSONObject(TAG_TITLE_DEFAULT);
            Iterator<String> keys = title_defaults.keys();
            while( keys.hasNext() ) {
                String key = keys.next();
                defaultTitle.put(key, title_defaults.getString(key));
            }

            if( object.has(TAG_MAX_LENGTH) ) {
                maxLength = Integer.parseInt(object.getString(TAG_MAX_LENGTH));
            }

            if( object.has(TAG_RECOMMEND_LENGTH) ) {
                recommendLength = Integer.parseInt(object.getString(TAG_RECOMMEND_LENGTH));
            }

            if( object.has(TAG_TITLE_MAX_LINES) ) {
                titleMaxLines = Integer.parseInt(object.getString(TAG_TITLE_MAX_LINES));
            }

            // Locale.getDefault().getDisplayLanguage();
        } catch (JSONException e) {
            e.printStackTrace();
            Log.d(TAG, "parse Collage failed : " + e.getMessage());
            return e.getMessage();
        }

        return null;
    }

    nexSaveDataFormat.nexCollageTitleInfoOf getSaveData(){
        nexSaveDataFormat.nexCollageTitleInfoOf data = new nexSaveDataFormat.nexCollageTitleInfoOf();
        data.userDropShadowColor = userDropShadowColor;
        data.userFillColor = userFillColor;
        data.userFont = userFont;
        data.userStrokeColor = userStrokeColor;
        data.userText = userText;
        return data;
    }

    void loadSaveData(nexSaveDataFormat.nexCollageTitleInfoOf data){
        userDropShadowColor = data.userDropShadowColor;
        userFillColor = data.userFillColor;
        userFont = data.userFont;
        userStrokeColor = data.userStrokeColor;
        userText = data.userText;
    }
}
