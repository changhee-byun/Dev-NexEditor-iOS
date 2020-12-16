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

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

public final class nexExportFormatBuilder {
    private static String TAG="nexExportFormatBuilder";

    Map<String, String> formats = new HashMap<String, String>();

    public nexExportFormatBuilder setType(String type) {
        formats.put(nexExportFormat.TAG_FORMAT_TYPE, type);
        return this;
    }

    public nexExportFormatBuilder setPath(String path) {
        formats.put(nexExportFormat.TAG_FORMAT_PATH, path);
        return this;
    }

    public nexExportFormatBuilder setWidth(int width) {
        formats.put(nexExportFormat.TAG_FORMAT_WIDTH, "" + width);
        return this;
    }

    public nexExportFormatBuilder setHeight(int height) {
        formats.put(nexExportFormat.TAG_FORMAT_HEIGHT, "" + height);
        return this;
    }

    public nexExportFormatBuilder setQuality(int quality) {
        formats.put(nexExportFormat.TAG_FORMAT_QUALITY, "" + quality);
        return this;
    }

    public nexExportFormatBuilder setVideoCodec(int codec) {
        formats.put(nexExportFormat.TAG_FORMAT_VIDEO_CODEC, "" + codec);
        return this;
    }

    public nexExportFormatBuilder setVideoBitrate(int bitrate) {
        formats.put(nexExportFormat.TAG_FORMAT_VIDEO_BITRATE, "" + bitrate);
        return this;
    }

    public nexExportFormatBuilder setVideoProfile(int profile) {
        formats.put(nexExportFormat.TAG_FORMAT_VIDEO_PROFILE, "" + profile);
        return this;
    }

    public nexExportFormatBuilder setVideoLevel(int level) {
        formats.put(nexExportFormat.TAG_FORMAT_VIDEO_LEVEL, "" + level);
        return this;
    }

    public nexExportFormatBuilder setVideoRotate(int rotate) {
        formats.put(nexExportFormat.TAG_FORMAT_VIDEO_ROTATE, "" + rotate);
        return this;
    }

    public nexExportFormatBuilder setVideoFPS(int fps) {
        formats.put(nexExportFormat.TAG_FORMAT_VIDEO_FPS, "" + fps);
        return this;
    }

    public nexExportFormatBuilder setAudioCodec(int codec) {
        formats.put(nexExportFormat.TAG_FORMAT_AUDIO_CODEC, "" + codec);
        return this;
    }

    public nexExportFormatBuilder setAudioBitrate(int bitrate) {
        formats.put(nexExportFormat.TAG_FORMAT_AUDIO_BITRATE, "" + bitrate);
        return this;
    }

    public nexExportFormatBuilder setAudioSampleRate(int samplerate) {
        formats.put(nexExportFormat.TAG_FORMAT_AUDIO_SAMPLERATE, "" + samplerate);
        return this;
    }

    public nexExportFormatBuilder setMaxFileSize(long filesize) {
        formats.put(nexExportFormat.TAG_FORMAT_MAX_FILESIZE, "" + filesize);
        return this;
    }

    public nexExportFormatBuilder setTagID(int id) {
        formats.put(nexExportFormat.TAG_FORMAT_TAG, "" + id);
        return this;
    }

    public nexExportFormatBuilder setIntervalTime(int ms){
        formats.put(nexExportFormat.TAG_FORMAT_INTERVAL_TIME, "" + ms);
        return this;
    }

    public nexExportFormatBuilder setStartTime(int ms){
        formats.put(nexExportFormat.TAG_FORMAT_START_TIME, "" + ms);
        return this;
    }

    public nexExportFormatBuilder setEndTime(int ms){
        formats.put(nexExportFormat.TAG_FORMAT_END_TIME, "" + ms);
        return this;
    }

    public nexExportFormatBuilder setUUID(String uuid){
        formats.put(nexExportFormat.TAG_FORMAT_UUID, uuid);
        return this;
    }

    public nexExportFormatBuilder setUserField(String key, String value){
        formats.put(key, value);
        return this;
    }

    public nexExportFormat build() {
        nexExportFormat format = new nexExportFormat();

        Set<String> keys = formats.keySet();
        Iterator<String> k = keys.iterator();
        while(k.hasNext()) {
            String t = k.next();
            format.setString(t, formats.get(t));
        }
        return format;
    }

    public static nexExportFormatBuilder Builder(){
        return new nexExportFormatBuilder();
    }
}
