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

import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public final class nexExportFormat {
    private static String TAG="nexExportFormat";

    public static final String TAG_FORMAT_TYPE = "type";
    public static final String TAG_FORMAT_PATH = "path";
    public static final String TAG_FORMAT_WIDTH = "width";
    public static final String TAG_FORMAT_HEIGHT = "height";

    public static final String TAG_FORMAT_QUALITY = "quality";

    public static final String TAG_FORMAT_VIDEO_CODEC = "videocodec";
    public static final String TAG_FORMAT_VIDEO_BITRATE = "videobitrate";
    public static final String TAG_FORMAT_VIDEO_PROFILE = "videoprofile";
    public static final String TAG_FORMAT_VIDEO_LEVEL = "videolevel";
    public static final String TAG_FORMAT_VIDEO_ROTATE = "videorotate";
    public static final String TAG_FORMAT_VIDEO_FPS = "videofps";

    public static final String TAG_FORMAT_AUDIO_CODEC = "audiocodec";
    public static final String TAG_FORMAT_AUDIO_BITRATE = "audiobitrate";
    public static final String TAG_FORMAT_AUDIO_SAMPLERATE = "audiosamplerate";

    public static final String TAG_FORMAT_MAX_FILESIZE = "maxfilesize";

    public static final String TAG_FORMAT_INTERVAL_TIME = "intervaltime";
    public static final String TAG_FORMAT_START_TIME = "starttime";
    public static final String TAG_FORMAT_END_TIME = "endtime";
    public static final String TAG_FORMAT_UUID = "uuid";

    public static final String TAG_FORMAT_TAG = "tag";

    Map<String, String> formats = new HashMap<String, String>();

    public final float getFloat(String key) {
        if( formats.containsKey(key) )
            return Float.parseFloat(formats.get(key));
        return 0.0f;
    }

    public final int getInteger(String key) {
        if( formats.containsKey(key) )
            return Integer.parseInt(formats.get(key));
        return 0;
    }

    public final long getLong(String key) {
        if( formats.containsKey(key) )
            return Long.parseLong(formats.get(key));
        return 0;
    }

    public final String getString(String key) {
        if( formats.containsKey(key) )
            return formats.get(key);
        return "";
    }

    public final void setFloat(String key, float value) {
        formats.put(key, ""+value);
    }

    public final void setInteger(String key, int value) {
        formats.put(key, ""+value);
    }

    public final void setLong(String key, long value) {
        formats.put(key, ""+value);
    }

    public final void setString(String key, String value) {
        formats.put(key, value);
    }
}
