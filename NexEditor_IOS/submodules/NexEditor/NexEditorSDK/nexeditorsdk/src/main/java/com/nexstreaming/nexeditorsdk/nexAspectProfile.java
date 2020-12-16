/******************************************************************************
 * File Name        : nexAspectProfile.java
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

import android.util.Log;

import java.util.Arrays;
import java.util.List;

/**
 * This class manages Aspect Profile of a canvas which is used for editing with the NexEditor&trade;&nbsp;SDK.
 * Use this class when there is no preset Aspect Ratio Modes for you to use.
 * The recomended Aspect Ratio is 16:9, because all effects in the NexEditor&trade;&nbsp;SDK are created based on 16:9 ratio.
 * Therefore, some effects may not be seen as normal if they are applied to other aspect ratios.
 * It is recommended to use nexAspectProfile, when applying Resize, Trim, Speed, Audio editing to original video without any effects being used.
 * Note that the resolution set in this class becomes the coordinates of an overlay.
 * @see nexApplicationConfig#getAspectProfile()
 * @see nexApplicationConfig#setAspectProfile(nexAspectProfile)
 * @since 1.7.7
 */
public class nexAspectProfile {
    private int width;
    private int height;
    private float aspectRatio ;
    private int aspectMode = nexApplicationConfig.kAspectRatio_Mode_free;
    private final static  int[] export_land_heights = { 1080, 720, 480 ,360, 240, 160};
    private final static  int[] export_port_heights = { 1920,1280, 640 ,640, 320, 240};
    private final static int export_max = 1920*1080;
    private final static float[] aspects = {16f/9f,1f,9f/16f,2f,1f/2f,4f/3f,3f/4f};

    public static final nexAspectProfile ardef = new nexAspectProfile(1280,720,nexApplicationConfig.kAspectRatio_Mode_free);
    public static final nexAspectProfile ar16v9 = new nexAspectProfile(1280,720,nexApplicationConfig.kAspectRatio_Mode_16v9);
    public static final nexAspectProfile ar1v1 = new nexAspectProfile(720,720,nexApplicationConfig.kAspectRatio_Mode_1v1);
    public static final nexAspectProfile ar9v16 = new nexAspectProfile(720,1280,nexApplicationConfig.kAspectRatio_Mode_9v16);
    public static final nexAspectProfile ar2v1 = new nexAspectProfile(1440,720,nexApplicationConfig.kAspectRatio_Mode_2v1);
    public static final nexAspectProfile ar1v2 = new nexAspectProfile(720,1440,nexApplicationConfig.kAspectRatio_Mode_1v2);
    public static final nexAspectProfile ar4v3 = new nexAspectProfile(960,720,nexApplicationConfig.kAspectRatio_Mode_4v3);
    public static final nexAspectProfile ar3v4 = new nexAspectProfile(720,960,nexApplicationConfig.kAspectRatio_Mode_3v4);



    /**
     *
     * @since 1.7.7
     */
    public static final int ExportProfileLevel_NONE = 0;

    /**
     *
     * @since 1.7.7
     */
    public static final int ExportProfileLevel_LOW = 3;

    /**
     *
     * @since 1.7.7
     */
    public static final int ExportProfileLevel_MID = 2;

    /**
     *
     * @since 1.7.7
     */
    public static final int ExportProfileLevel_HIGH = 1;

    /**
     * This class manages Export Profile such as recommended resolution, bitrate, etc.
     * @since 1.7.7
     * @see #getExportProfiles()
     */
    public static class ExportProfile{
        private int level;
        private int width;
        private int height;
        private int bitrate;

        private ExportProfile(int level, int width, int height, int bitrate) {
            this.bitrate = bitrate;
            this.width = width;
            this.level = level;
            this.height = height;
        }

        /**
         * This method returns the recommended bitrate
         * @return bitrate
         * @since 1.7.7
         */
        public int getBitrate() {
            return bitrate;
        }

        /**
         * This method returns the width of recommended resolution
         * @return width of resolution
         * @since 1.7.7
         */
        public int getWidth() {
            return width;
        }

        /**
         * This method returns the recommended export level
         * @return export level
         * @since 1.7.7
         */
        public int getLevel() {
            return level;
        }

        /**
         * This method returns the height of recommended resolution
         * @return height of resolution
         * @since 1.7.7
         */
        public int getHeight() {
            return height;
        }
    }

    /**
     * This method returns a list of preset Aspect Profile.
     * @return {free, 16:9, 9:16, 2:1, 1:2, 4:3, 3:4 }
     * @since 1.7.7
     */
    public static List<nexAspectProfile> getPresetList() {
        return Arrays.asList(new nexAspectProfile[]{
                ardef, ar16v9, ar1v1, ar9v16, ar2v1, ar1v2, ar4v3, ar3v4,
        });
    }

    /**
     * This method returns an Aspect Profile, which is nexApplicationConfig.kAspectRatio_Mode_XvX.
     * @param mode nexApplicationConfig.kAspectRatio_Mode_XvX
     * @return Aspect Profile set by mode
     * @since 1.7.7
     */
    public static nexAspectProfile getAspectProfile(int mode){
        if( mode <= 0 ){
            return null;
        }
        List<nexAspectProfile> list= getPresetList();

        if( list.size() <= mode ){
            return null;
        }

        return list.get(mode);
    }

    private nexAspectProfile(int width, int height, int mode){
        this.width = width;
        this.height = height;
        aspectRatio = (float) width/(float)height;
        aspectMode = mode;
    }

    /**
     * This method sets the resolution of an Aspect Profile.
     * Input parameters are resolution values to be set to an overlay.
     * @param width width of an overlay
     * @param height height of an overlay
     * @since 1.7.7
     */
    public nexAspectProfile(int width, int height){
        int w = 1280;
        int h = 720;

        if(width > height){
            w = Math.round(720.0f * (float)width / (float)height);
            h = 720;
        }
        else{
            w = 720;
            h = Math.round(720.0f * (float)height / (float)width);
        }

        this.width = w;
        this.height = h;
        aspectRatio = (float) w/(float)h;
        aspectMode = getAspectMode(true);
    }

    /**
     * This method returns the width of an overlay resolution.
     * The return value is the width of an overlay.
     * @return width
     * @since 1.7.7
     */
    public int getWidth(){
        return width;
    }

    /**
     * This method returns the height of an overlay resolution.
     * The return value is the height of an overlay.
     * @return height
     * @since 1.7.7
     */
    public int getHeight(){
        return height;
    }

    /**
     * This method returns an aspect ratio value.
     * @return Aspect ratio value, which is width/height.
     * @since 1.7.7
     */
    public float getAspectRatio(){
        return aspectRatio;
    }

    int getAspectMode(boolean force){
        float ratio = getAspectRatio();
        if( ratio == 16f/9f ){
            return nexApplicationConfig.kAspectRatio_Mode_16v9;
        }else if( ratio == 9f/16f ){
            return nexApplicationConfig.kAspectRatio_Mode_9v16;
        }else if( ratio == 2f/1f ){
            return nexApplicationConfig.kAspectRatio_Mode_2v1;
        }else if( ratio == 1 ){
            return nexApplicationConfig.kAspectRatio_Mode_1v1;
        }else if( ratio == 1f/2f ){
            return nexApplicationConfig.kAspectRatio_Mode_1v2;
        }else if( ratio == 3f/4f ){
            return nexApplicationConfig.kAspectRatio_Mode_3v4;
        }else if( ratio == 4f/3f ){
            return nexApplicationConfig.kAspectRatio_Mode_4v3;
        }
        return nexApplicationConfig.kAspectRatio_Mode_free;
    }

    /**
     * This method returns an Aspect Mode, which is nexApplicationConfig.kAspectRatio_Mode_XvX.
     * @return
     * <ul>
     *     <li>kAspectRatio_Mode_free={@value nexApplicationConfig#kAspectRatio_Mode_free}</li>
     *     <li>kAspectRatio_Mode_16v9={@value nexApplicationConfig#kAspectRatio_Mode_16v9}</li>
     *     <li>kAspectRatio_Mode_9v16={@value nexApplicationConfig#kAspectRatio_Mode_9v16}</li>
     *     <li>kAspectRatio_Mode_1v1={@value nexApplicationConfig#kAspectRatio_Mode_1v1}</li>
     *     <li>kAspectRatio_Mode_2v1={@value nexApplicationConfig#kAspectRatio_Mode_2v1}</li>
     *     <li>kAspectRatio_Mode_1v2={@value nexApplicationConfig#kAspectRatio_Mode_1v2}</li>
     *     <li>kAspectRatio_Mode_4v3={@value nexApplicationConfig#kAspectRatio_Mode_4v3}</li>
     *     <li>kAspectRatio_Mode_3v4={@value nexApplicationConfig#kAspectRatio_Mode_3v4}</li>
     * </ul>
     * @since 1.7.7
     */
    public int getAspectMode(){
        return aspectMode;
    }


    /**
     * This method returns a list of recommended export profile.
     * @return list of export profile
     * @since 1.7.7
     */
    public ExportProfile[] getExportProfiles(){
        ExportProfile[] profiles = new ExportProfile[3];
        int[] export_heights = export_land_heights;
        int index = 0;
        if( !isLandscapeMode() ){
            export_heights = export_port_heights;
        }
        for (int h : export_heights) {
            if (export_max >= h * h * getAspectRatio()) {
                if (h * getAspectRatio() % 2 == 0) {
                    profiles[index] = new ExportProfile(index + 1, (int) (h * getAspectRatio()), h, 6 * 1024 * 1024);
                    index++;
                    if (index == 3) {
                        break;
                    }
                }
            }
        }

        if(  index < 3 ) {
            for (int i = index; i < 3; i++) {
                profiles[index] = new ExportProfile(0, getWidth(), getHeight(), 6 * 1024 * 1024);
            }
        }

        return profiles;
    }

    /**
     * This method checks whether the resolution is in landscape or portrait.
     * @return ture - Landscape, false - Portrait
     * @since 1.7.7
     */
    public boolean isLandscapeMode(){
        return (width >= height);
    }

    /**
     * This method returns an Aspect Mode which is the closest to the resolution of an overlay.
     * @return
     *<ul>
     *     <li>kAspectRatio_Mode_free={@value nexApplicationConfig#kAspectRatio_Mode_free}</li>
     *     <li>kAspectRatio_Mode_16v9={@value nexApplicationConfig#kAspectRatio_Mode_16v9}</li>
     *     <li>kAspectRatio_Mode_9v16={@value nexApplicationConfig#kAspectRatio_Mode_9v16}</li>
     *     <li>kAspectRatio_Mode_1v1={@value nexApplicationConfig#kAspectRatio_Mode_1v1}</li>
     *     <li>kAspectRatio_Mode_2v1={@value nexApplicationConfig#kAspectRatio_Mode_2v1}</li>
     *     <li>kAspectRatio_Mode_1v2={@value nexApplicationConfig#kAspectRatio_Mode_1v2}</li>
     *     <li>kAspectRatio_Mode_4v3={@value nexApplicationConfig#kAspectRatio_Mode_4v3}</li>
     *     <li>kAspectRatio_Mode_3v4={@value nexApplicationConfig#kAspectRatio_Mode_3v4}</li>
     * </ul>
     * @since 1.7.7
     */
    public int getSimilarAspectMode(){
        return getSimilarAspectMode(getAspectRatio());
    }

    /**
     * This method returns the lowest common numerator and denominator of an aspect ratio.
     * @return [0] - numerator, [1] - denominator
     */
    public int[] aspect2ReduceFraction(){
        return aspect2ReduceFraction(width,height,0);
    }

    /**
     * This method returns an Aspect Mode which is the closest to the aspect ratio of an overlay.
     * @param aspectRatio width/height
     * @return
     * <ul>
     *     <li>kAspectRatio_Mode_free={@value nexApplicationConfig#kAspectRatio_Mode_free}</li>
     *     <li>kAspectRatio_Mode_16v9={@value nexApplicationConfig#kAspectRatio_Mode_16v9}</li>
     *     <li>kAspectRatio_Mode_9v16={@value nexApplicationConfig#kAspectRatio_Mode_9v16}</li>
     *     <li>kAspectRatio_Mode_1v1={@value nexApplicationConfig#kAspectRatio_Mode_1v1}</li>
     *     <li>kAspectRatio_Mode_2v1={@value nexApplicationConfig#kAspectRatio_Mode_2v1}</li>
     *     <li>kAspectRatio_Mode_1v2={@value nexApplicationConfig#kAspectRatio_Mode_1v2}</li>
     *     <li>kAspectRatio_Mode_4v3={@value nexApplicationConfig#kAspectRatio_Mode_4v3}</li>
     *     <li>kAspectRatio_Mode_3v4={@value nexApplicationConfig#kAspectRatio_Mode_3v4}</li>
     * </ul>
     * @since 1.7.7
     */
    public static int getSimilarAspectMode(float aspectRatio){

        float aspectDim = 3f;
        int selectAspect = 1;
        for(int i = 0 ; i < aspects.length ; i++  ){
            float aspectDelta = aspectRatio - aspects[i];
            if( aspectDelta < 0 ) aspectDelta *= -1f;
            if( aspectDim > aspectDelta ){
                aspectDim = aspectDelta;
                selectAspect = i;
            }
        }
        return selectAspect+1;
    }

    /**
     * This method returns an Aspect Mode which is the closest to the resolution of an overlay.
     * @param width width of an overlay
     * @param height height of an overlay
     * @param rotate aspect ratio of an overlay
     * @return
     * <ul>
     *     <li>kAspectRatio_Mode_free={@value nexApplicationConfig#kAspectRatio_Mode_free}</li>
     *     <li>kAspectRatio_Mode_16v9={@value nexApplicationConfig#kAspectRatio_Mode_16v9}</li>
     *     <li>kAspectRatio_Mode_9v16={@value nexApplicationConfig#kAspectRatio_Mode_9v16}</li>
     *     <li>kAspectRatio_Mode_1v1={@value nexApplicationConfig#kAspectRatio_Mode_1v1}</li>
     *     <li>kAspectRatio_Mode_2v1={@value nexApplicationConfig#kAspectRatio_Mode_2v1}</li>
     *     <li>kAspectRatio_Mode_1v2={@value nexApplicationConfig#kAspectRatio_Mode_1v2}</li>
     *     <li>kAspectRatio_Mode_4v3={@value nexApplicationConfig#kAspectRatio_Mode_4v3}</li>
     *     <li>kAspectRatio_Mode_3v4={@value nexApplicationConfig#kAspectRatio_Mode_3v4}</li>
     * </ul>
     * @since 1.7.7
     */
    public static int getSimilarAspectMode(int width , int height, int rotate){

        float aspectRatio = (float)width/(float)height;
        if( rotate == 90 || rotate == 270 ){
            aspectRatio = (float)height/(float)width;
        }

        float aspectDim = 3f;
        int selectAspect = 1;
        for(int i = 0 ; i < aspects.length ; i++  ){
            float aspectDelta = aspectRatio - aspects[i];
            if( aspectDelta < 0 ) aspectDelta *= -1f;
            if( aspectDim > aspectDelta ){
                aspectDim = aspectDelta;
                selectAspect = i;
            }
        }
        return selectAspect+1;
    }

    /**
     * This method returns the lowest common numerator and denominator of a resolution.
     * @param width width of an overlay
     * @param height height of an overlay
     * @param rotate aspect ratio of an overlay
     * @return [0] - numerator, [1] - denominator
     */
    public static int[] aspect2ReduceFraction(int width , int height, int rotate){
        if( rotate == 90 || rotate == 270 ){
            return reduceFraction(height,width);
        }
        return reduceFraction(width,height);
    }

    private static int[] reduceFraction(int bunja, int bunmo) {
        int[] frac = new int[2];
        frac[0] = bunja;
        frac[1] = bunmo;

        if (frac[1] == 0) {
            frac[0] = 0;
            frac[1] = 0;
            return frac;
        }

        int gcd_result = gcd(frac[0], frac[1]);

        frac[0] = frac[0] / gcd_result;
        frac[1] = frac[1] / gcd_result;

        return frac;
    }

    private static int gcd(int a, int b) {

        while (b != 0) {
            int temp = a % b;
            a = b;
            b = temp;
        }

        return Math.abs(a);
    }

}
