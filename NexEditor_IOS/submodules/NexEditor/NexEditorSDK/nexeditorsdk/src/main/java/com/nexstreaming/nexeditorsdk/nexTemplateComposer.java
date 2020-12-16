/******************************************************************************
 * File Name        : nexTemplateComposer.java
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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Rect;
import android.os.Environment;
import android.util.Log;
import android.util.SparseArray;
import android.view.animation.AccelerateDecelerateInterpolator;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.store.AssetLocalInstallDB;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.nexeditorsdk.exception.nexSDKException;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.zip.ZipInputStream;

/**
 * This class newly composes clips of a project that has been created from a pre-registered template file.
 * @since version 1.0.0
 */
public final class nexTemplateComposer {
    private static final String TAG="nexTemplateComposer";
    private static final String cancelMassage = "cancel template";
    private nexProject mProject;
    private String mTemplateFilePath;
    private String mTemplateID;
    private InputStream mInputStream;
    private Context mAppContext;
    private Context mResContext;
    private String mBGMPath = null;
    private boolean mBGMTempFile = false;
    private ArrayList<HashMap<String, String>> mTemplateList;
    private JSONArray mTemplateArray = null;

    private ArrayList<HashMap<String, String>> mIntroTemplateList;
    private ArrayList<HashMap<String, String>> mLoopTemplateList;
    private ArrayList<HashMap<String, String>> mOutroTemplateList;
    private SparseArray<Integer> mTemplateTypeCountList;
    private JSONArray mIntroTemplateArray = null;
    private JSONArray mLoopTemplateArray = null;
    private JSONArray mOutroTemplateArray = null;


    private nexTemplate mTemplate = null;

    /*
    *   1: 1st version / initial
    *   2: 2nd version / intro, loop, out-ro support
    **/
    private String mTemplateVersion = null;

    private static final String TAG_TEMPLATE = "template";
    private static final String TAG_TEMPLATE_VERSION = "template_version";
    private static final String TAG_TEMPLATE_DESCRIPTION = "template_desc";
    private static final String TAG_TEMPLATE_INTRO = "template_intro";
    private static final String TAG_TEMPLATE_LOOP = "template_loop";
    private static final String TAG_TEMPLATE_OUTRO = "template_outro";
    private static final String TAG_TEMPLATE_NAME = "template_name";
    private static final String TAG_TEMPLATE_BGM = "template_bgm";
    private static final String TAG_TEMPLATE_BGM_VOLUME = "template_bgm_volume";
    private static final String TAG_TEMPLATE_LETTERBOX = "template_letterbox";

    // SCENE, TRANSITION
    private static final String TAG_TYPE = "type";
    private static final String TAG_ID = "id";
    // ALL, VIDEO, IMAGE, SOLID, EXTERNAL_VIDEO, EXTERNAL_IMAGE
    private static final String TAG_SOURCE_TYPE = "source_type";
    private static final String TAG_DURATION = "duration";
    private static final String TAG_VOLUME = "volume";
    private static final String TAG_EFFECTS = "effects";
    private static final String TAG_BRIGHTNESS = "brightness";
    private static final String TAG_CONTRAST = "contrast";
    private static final String TAG_SATURATION = "saturation";
    private static final String TAG_COLOR_FILTER = "color_filter";
    private static final String TAG_SPEED_CONTROL = "speed_control";
    private static final String TAG_CROP_MODE = "crop_mode";
    private static final String TAG_CROP_SPEED = "image_crop_speed";
    private static final String TAG_VIDEO_CROP_MODE = "video_crop_mode";
    private static final String TAG_IMAGE_CROP_MODE = "image_crop_mode";
    private static final String TAG_LUT = "lut";
    private static final String TAG_EXTERNAL_VIDEO_PATH = "external_video_path";
    private static final String TAG_EXTERNAL_IMAGE_PATH = "external_image_path";
    private static final String TAG_SOLID_COLOR = "solid_color";
    private static final String TAG_VIGNETTE = "vignette";

    private ArrayList<TemplateOverlay> mOverlayList;
    private boolean mCancel = false;

    private class TemplateOverlay {
        private int clipID;
        private int overlayDuration;
        private int overlayVariation;
        private boolean overlayAnimation;
        private int inAnimationStartTime;
        private int inAnimationTime;
        private int outAnimationStartTime;
        private int outAnimationTime;

        public TemplateOverlay(int clipID, int overlayDuration, int overlayVariation, boolean overlayAnimation, int inAnimationStartTime, int inAnimationTime, int outAnimationStartTime, int outAnimationTime) {
            this.clipID = clipID;
            this.overlayDuration = overlayDuration;
            this.overlayVariation = overlayVariation;
            this.overlayAnimation = overlayAnimation;
            this.inAnimationStartTime = inAnimationStartTime;
            this.inAnimationTime = inAnimationTime;
            this.outAnimationStartTime = outAnimationStartTime;
            this.outAnimationTime = outAnimationTime;
        }

        public int getClipID() {
            return this.clipID;
        }

        public int getOverlayDuration() {
            return this.overlayDuration;
        }

        public int getOverlayVariation() {
            return this.overlayVariation;
        }

        public boolean getOverlayAnimation() {
            return this.overlayAnimation;
        }

        public int getInAnimationStartTime() {
            return  this.inAnimationStartTime;
        }

        public int getInAnimationTime() {
            return this.inAnimationTime;
        }

        public int getOutAnimationStartTime() {
            return this.outAnimationStartTime;
        }

        public int getOutAnimationTime() {
            return this.outAnimationTime;
        }
    }

    /**
     * This method creates an instance of <tt>nexTemplateComposer</tt>.
     *
     * @since version 1.0.0
     */
    public nexTemplateComposer() {
        mTemplateList = new ArrayList<>();
        mIntroTemplateList = new ArrayList<>();
        mLoopTemplateList = new ArrayList<>();
        mOutroTemplateList = new ArrayList<>();
        mTemplateTypeCountList = new SparseArray<>();
        //
        mOverlayList = new ArrayList<>();
    }

    /**
     * This method returns an instance after creating an empty project.
     *
     * @since version 1.0.0
     */
    public nexProject createProject() {
        return new nexProject();
    }

    /**
     * This method clears the resources of <tt>nexTemplateComposer</tt>.
     * This method must be called when the activity is destroyed.
     *
     * @since version 1.0.0
     */
    public void release() {
        introCount = 0;
        loopCount = 0;
        tempClipID = 0;
        mCancel = false;

        if( mTemplate != null ){
            mTemplate.release();
        }
        //
        if(mTemplateList != null) {
            mTemplateList.clear();
        }
        if(mIntroTemplateList != null) {
            mIntroTemplateList.clear();
        }
        if(mLoopTemplateList != null) {
            mLoopTemplateList.clear();
        }
        if(mOutroTemplateList != null) {
            mOutroTemplateList.clear();
        }
        if(mTemplateTypeCountList != null) {
            mTemplateTypeCountList.clear();
        }
        if(mOverlayList != null) {
            mOverlayList.clear();
        }
        if(mBGMPath != null) {
            if( mBGMTempFile ) {
                File file = new File(mBGMPath);
                file.delete();
            }
        }
        templateListID = 0;
        mTemplateVersion = null;
    }

    /**
     * This method creates a new project with template files stated by the parameter <tt>templateFilePath</tt>.
     *
     * <p>Example code :</p>
     * {@code nexTemplateComposer composer = new nexTemplateComposer();
    nexProject project = composer.createProject();
    // add clips in the project
    ...
    composer.setTemplateEffects2Project(project, getApplicationContext(), null, tempalte_file_path, false, titleInfo);}
     * @param project           The nexProject instance.
     * @param appContext        The application context instance.
     * @param resContext        The context instance that can access the asset.
     * @param templateFilePath  The directory where the template file is saved.
     * @param updateState       Set to <tt>true</tt> and a project will be created with previous template list.
     *                          Set to <tt>false</tt> and a project will be created with new template list.
     *                          Default value is <tt>false</tt>.
     * @since version 1.0.0
     */
    public String setTemplateEffects2Project(nexProject project, Context appContext, Context resContext, String templateFilePath, boolean updateState) throws nexSDKException {
        initTemplateComposer(project, appContext, resContext, templateFilePath);
        return setTemplateEffect();
    }

    void initTemplateComposer(nexProject project, Context appContext, Context resContext, String templateFilePath) throws nexSDKException {
        mProject = project;
        mAppContext = appContext;
        mResContext = resContext;
        mCancel = false;
        mTemplateID = null;
        mTemplateFilePath = null;
        mInputStream = null;
        mTemplateID = templateFilePath;

        release();

        // Set initial values.
        //
        mTemplateTypeCountList.append(0, 0);
        mTemplateTypeCountList.append(1, 0);
        mTemplateTypeCountList.append(2, 0);
    }

    void addTemplateOverlay(int clipID, String tagValue) {
        String[] splitArrary = tagValue.split(",");

        // 
        //
        String type = splitArrary[0];
        String duration = splitArrary[1];
        String variation = splitArrary[2];
        String activeAnimation = splitArrary[3];
        String inAnimationStartTime = splitArrary[4];
        String inAnimationTime = splitArrary[5];
        String outAnimationStartTime = splitArrary[6];
        String outAnimationTime = splitArrary[7];
        //
        if(LL.D) Log.d(TAG, "clipID="+clipID
                + " /type="+type
                + " /duration="+duration
                + " /variation="+variation
                + " /activeAnimation="+activeAnimation
                + " /inAnimationStartTime="+inAnimationStartTime
                + " /inAnimationTime="+inAnimationTime
                + " /outAnimationStartTime="+outAnimationStartTime
                + " /outAnimationTime="+outAnimationTime);

        if(type.equals("overlay")) {
            TemplateOverlay overlay = new TemplateOverlay(clipID,
                    Integer.parseInt(duration),
                    Integer.parseInt(variation),
                    Boolean.parseBoolean(activeAnimation),
                    Integer.parseInt(inAnimationStartTime),
                    Integer.parseInt(inAnimationTime),
                    Integer.parseInt(outAnimationStartTime),
                    Integer.parseInt(outAnimationTime));
            mOverlayList.add(overlay);
        }
    }

    nexOverlayItem vignetteOverlayViaRatioMode(final String imageNaming, final int width, final int height, int startTime2overlay, int duration2overlay) {
        return new nexOverlayItem(new nexOverlayImage("template_overlay", width, height, new nexOverlayImage.runTimeMakeBitMap() {
            @Override
            public boolean isAniMate() {
                return false;
            }

            @Override
            public int getBitmapID() {
                return 100;
                /*
                if(nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_16v9) {
                    return 100;
                } else {
                    return 200;
                }
                */
            }

            @Override
            public Bitmap makeBitmap() {
                try {
                    Bitmap bm = BitmapFactory.decodeStream(mResContext.getResources().getAssets().open(imageNaming), null, null);
                    Bitmap res = Bitmap.createScaledBitmap(bm, width, height, true);
                    return res;
                } catch (IOException e) {
                    e.printStackTrace();
                    return null;
                }
            }
        }), width/2, height/2, startTime2overlay, startTime2overlay+duration2overlay);
    }

    void setOverlay2Project() {
        for(TemplateOverlay overlay : mOverlayList) {
            // 1. 
            // 2. 
            // 3. 
            // 4. 
            //
            nexClip clip = mProject.getClip(overlay.getClipID(), true);

            int startTime2clip = clip.mStartTime;
            int startTime2overlay = startTime2clip + overlay.getOverlayVariation();

            nexOverlayItem overlayItem = null;
/*
            if(nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_16v9) {
                overlayItem = vignetteOverlayViaRatioMode("vignette.png", 1280, 720, startTime2overlay, overlay.getOverlayDuration());
            } else if(nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16) {
                overlayItem = vignetteOverlayViaRatioMode("vignette_9v16.png", 720, 1280, startTime2overlay, overlay.getOverlayDuration());
            }
*/
            overlayItem = vignetteOverlayViaRatioMode("vignette.png",nexApplicationConfig.getAspectProfile().getWidth() , nexApplicationConfig.getAspectProfile().getHeight(), startTime2overlay, overlay.getOverlayDuration());

            mProject.addOverlay(overlayItem);

            if(overlay.getOverlayAnimation()) {
                overlayItem.clearAnimate();
                overlayItem.addAnimate(nexAnimate.getAlpha(overlay.getInAnimationStartTime(), overlay.getInAnimationTime(), 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                overlayItem.addAnimate(nexAnimate.getAlpha(overlay.getOutAnimationStartTime(), overlay.getOutAnimationTime(), 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
            }

//
//            File file = new File("path, defined by Design Team");
//            if(file.isFile()) {
//                final String path = file.getAbsolutePath();
//                nexOverlayItem overlayItem = new nexOverlayItem(new nexOverlayImage("template_overlay", 1280, 720, path), 640, 360, startTime2overlay, startTime2overlay+overlay.getOverlayDuration());
//                mProject.addOverlay(overlayItem);
//                if(overlay.getOverlayAnimation()) {
//                    overlayItem.clearAnimate();
//                    overlayItem.addAnimate(nexAnimate.getAlpha(overlay.getInAnimationStartTime(), overlay.getInAnimationTime(), 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
//                    overlayItem.addAnimate(nexAnimate.getAlpha(overlay.getOutAnimationStartTime(), overlay.getOutAnimationTime(), 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
//                }
//            }
        }
    }

    String setTemplateEffect() {
        String ret = parsingJSONFile(mProject);
        if(ret != null) {
            return ret;
        }
        if(mTemplateVersion.equals("template 1.0")) {
            consistProjectViaVer1(mProject);
            if(  mCancel ){
                return cancelMassage;
            }
            ret = setProperty2Clips(mProject, mTemplateVersion);
        } else if(mTemplateVersion.equals("template 1.x")) {
            consistProjectViaVer2(mProject);
            if(  mCancel ){
                return cancelMassage;
            }

            ret = setProperty2Clips(mProject, mTemplateVersion);
        }
        else
        {
            ret = applyTemplateOnProject(mProject);
        }

        if(ret != null) {
            //
            return ret;
        } else {
            setOverlay2Project();
            return null;
        }
    }

    String getValue(JSONObject object, String tag) {
        try {
            return object.getString(tag);
        } catch (JSONException e) {
            if(tag.equals(TAG_EFFECTS)) {
                return "none";
            } else if(tag.equals(TAG_ID)
                    || tag.equals(TAG_VOLUME)) {
                return "0";
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
                return "";
            } else if(tag.equals(TAG_CROP_SPEED)) {
                return "0";
            } else {
                return "default";
            }
        }
    }

    HashMap<String, String> setParameter2List(JSONObject object) {
        HashMap<String, String> template = new HashMap<>();

        String type = getValue(object, TAG_TYPE);
        if (type.equals("scene")) {
            String id = getValue(object, TAG_ID);
            String source_type = getValue(object, TAG_SOURCE_TYPE);
            String duration = getValue(object, TAG_DURATION);
            String volume = getValue(object, TAG_VOLUME);
            String effect = getValue(object, TAG_EFFECTS);
            String brightness = getValue(object, TAG_BRIGHTNESS);
            String contrast = getValue(object, TAG_CONTRAST);
            String saturation = getValue(object, TAG_SATURATION);
            String color_filter = getValue(object, TAG_COLOR_FILTER);
            String speed_control = getValue(object, TAG_SPEED_CONTROL);
            String vignette = getValue(object, TAG_VIGNETTE);
            String lut = getValue(object, TAG_LUT);
            String external_video_path = getValue(object, TAG_EXTERNAL_VIDEO_PATH);
            String external_image_path = getValue(object, TAG_EXTERNAL_IMAGE_PATH);
            String solid_color = getValue(object, TAG_SOLID_COLOR);
            String crop_mode = getValue(object, TAG_CROP_MODE);
            String crop_speed = getValue(object, TAG_CROP_SPEED);
            String video_crop_mode = getValue(object, TAG_VIDEO_CROP_MODE);
            String image_crop_mode = getValue(object, TAG_IMAGE_CROP_MODE);

            template.put(TAG_TYPE, type);
            template.put(TAG_ID, id);
            template.put(TAG_SOURCE_TYPE, source_type);
            template.put(TAG_DURATION, duration);
            template.put(TAG_VOLUME, volume);
            template.put(TAG_EFFECTS, effect);
            template.put(TAG_BRIGHTNESS, brightness);
            template.put(TAG_CONTRAST, contrast);
            template.put(TAG_SATURATION, saturation);
            template.put(TAG_COLOR_FILTER, color_filter);
            template.put(TAG_SPEED_CONTROL, speed_control);
            template.put(TAG_LUT, lut);
            template.put(TAG_CROP_MODE, crop_mode);
            template.put(TAG_CROP_SPEED, crop_speed);
            template.put(TAG_VIDEO_CROP_MODE, video_crop_mode);
            template.put(TAG_IMAGE_CROP_MODE, image_crop_mode);

            //
            template.put(TAG_EXTERNAL_VIDEO_PATH, external_video_path);
            template.put(TAG_EXTERNAL_IMAGE_PATH, external_image_path);
            template.put(TAG_SOLID_COLOR, solid_color);
            template.put(TAG_VIGNETTE, vignette);
        } else if (type.equals("transition")) {
            String effect = getValue(object, TAG_EFFECTS);
            String duration = getValue(object, TAG_DURATION);

            template.put(TAG_TYPE, type);
            template.put(TAG_EFFECTS, effect);
            template.put(TAG_DURATION, duration);
        }

        return template;
    }

    static boolean checkEffectId(nexEffectLibrary fxlib, String Id){
        if( Id == null ){
            return true;
        }

        if( Id.compareToIgnoreCase("none") == 0 ){
            return true;
        }

        if( Id.compareToIgnoreCase("null") == 0 ){
            return true;
        }

        if( fxlib.checkEffectID(Id)  ){
            return true;
        }
        if(LL.D) Log.d(TAG, "missing effect: ("+Id+"))");
        return false;
    }

    static String[] checkMissEffects(nexEffectLibrary fxlib,InputStream templateStream){
        String jsonStr = null;
        String version = null;
        jsonStr = readFromFile(templateStream);
        ArrayList<String> missEffects = new ArrayList<>();
        String effect = null;
        if(jsonStr != null) {
            try {
                JSONObject jsonObject = new JSONObject(jsonStr);

                String template_name = jsonObject.getString(TAG_TEMPLATE_NAME);

                try {
                    String template_version = jsonObject.getString(TAG_TEMPLATE_VERSION);
                    String template_letterbox = nexApplicationConfig.getDefaultLetterboxEffect();
                    if(jsonObject.has(TAG_TEMPLATE_LETTERBOX))
                        template_letterbox = jsonObject.getString(TAG_TEMPLATE_LETTERBOX);
                    String template_description = jsonObject.getString(TAG_TEMPLATE_DESCRIPTION);
                    //
                    if(template_version.equals("v1.x") || template_version.equals("v2.0.0")) {
                        version = "template 1.x";
                    } else if( template_version.equals("2.0.0") ){
                        version = "template 2.0";
                    } else {
                        version = "template 1.0";
                    }
                    //
                    if(LL.D) Log.d(TAG, "Info(name, version, description): (" + template_name + " ," + version + " ," + template_description + "))");

                    JSONArray introTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_INTRO);

                    for (int i = 0; i < introTemplateArray.length(); i++) {
                        effect = introTemplateArray.getJSONObject(i).getString(TAG_EFFECTS);
                        if( !checkEffectId(fxlib,effect) ){
                            missEffects.add(effect);
                        }
                    }
                    //
                    JSONArray loopTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_LOOP);
                    for (int j = 0; j < loopTemplateArray.length(); j++) {
                        effect = loopTemplateArray.getJSONObject(j).getString(TAG_EFFECTS);
                        if( !checkEffectId(fxlib,effect) ){
                            missEffects.add(effect);
                        }
                    }
                    //
                    JSONArray outroTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_OUTRO);
                    for (int k = 0; k < outroTemplateArray.length(); k++) {
                        effect = outroTemplateArray.getJSONObject(k).getString(TAG_EFFECTS);
                        if( !checkEffectId(fxlib,effect) ){
                            missEffects.add(effect);
                        }
                    }
                } catch (JSONException e) {
                    if(LL.D) Log.d(TAG, "Info(name: ("+template_name+"))");
                    version = "template 1.0";
                }

                if(version.equals("template 1.0")) {
                    //
                    JSONArray templateArray = jsonObject.getJSONArray(TAG_TEMPLATE);

                    for(int i=0; i< templateArray.length(); i++) {
                        effect = templateArray.getJSONObject(i).getString(TAG_EFFECTS);
                        if( !checkEffectId(fxlib,effect) ){
                            missEffects.add(effect);
                        }
                    }
                }
            } catch (JSONException e) {
                //e.getMessage();
            } catch (Exception e) {
                //return e.getMessage();
            }
        }

        if( missEffects.size() > 0 ){
            String ret[] = new String[missEffects.size()];
            for(int i = 0 ; i < missEffects.size() ; i++ ){
                ret[i] = missEffects.get(i).toString();
            }
            return ret;
        }
        return null;
    }

    String parsingJSONFile(nexProject project) {
        String jsonStr = null;

        if( mTemplateID != null )
        {
            jsonStr = AssetPackageTemplateJsonToString(mTemplateID);
        }
        else {
            // TemplateList consist.
            if (mTemplateFilePath != null) {
                jsonStr = readFromFile(mTemplateFilePath);
            } else if (mInputStream != null) {
                jsonStr = readFromFile(mInputStream);
            }
        }

        if(jsonStr != null) {
            try {
                JSONObject jsonObject = new JSONObject(jsonStr);

                String template_name = jsonObject.getString(TAG_TEMPLATE_NAME);

                try {
                    String template_version = jsonObject.getString(TAG_TEMPLATE_VERSION);
                    String template_letterbox = nexApplicationConfig.getDefaultLetterboxEffect();
                    if(jsonObject.has(TAG_TEMPLATE_LETTERBOX))
                        template_letterbox = jsonObject.getString(TAG_TEMPLATE_LETTERBOX);
                    project.setLetterboxEffect(template_letterbox);

                    String template_description = jsonObject.getString(TAG_TEMPLATE_DESCRIPTION);
                    //
                    if(template_version.equals("v1.x")||template_version.equals("v2.0.0")) {

                        mTemplateVersion = "template 1.x";

                        if(LL.D) Log.d(TAG, "Info(name, version, description): (" + template_name + " ," + mTemplateVersion + " ," + template_description + "))");

                        applyBGM2Project(project, jsonObject.getString(TAG_TEMPLATE_BGM));

                        //
                        float bgm_volume = Float.parseFloat(jsonObject.getString(TAG_TEMPLATE_BGM_VOLUME));
                        project.setBGMMasterVolumeScale(bgm_volume);
                        //
                        mIntroTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_INTRO);
                        for (int i = 0; i < mIntroTemplateArray.length(); i++) {
                            mIntroTemplateList.add(setParameter2List(mIntroTemplateArray.getJSONObject(i)));
                        }
                        //
                        mLoopTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_LOOP);
                        for (int j = 0; j < mLoopTemplateArray.length(); j++) {
                            mLoopTemplateList.add(setParameter2List(mLoopTemplateArray.getJSONObject(j)));
                        }
                        //
                        mOutroTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE_OUTRO);
                        for (int j = 0; j < mOutroTemplateArray.length(); j++) {
                            mOutroTemplateList.add(setParameter2List(mOutroTemplateArray.getJSONObject(j)));
                        }
                    } else {
                        mTemplateVersion = "template 2.0";

                        mTemplate = new nexTemplate();
                        String ret = mTemplate.parseTemplate(mTemplateFilePath, jsonObject);
                        if( ret != null )
                            return ret;
                    }
                } catch (JSONException e) {
                    if(LL.D) Log.d(TAG, "Info(name: ("+template_name+"))");

                    mIntroTemplateList.clear();
                    mLoopTemplateList.clear();
                    mOutroTemplateList.clear();
                    //
                    mTemplateVersion = "template 1.0";
                }

                if(mTemplateVersion.equals("template 1.0")) {
                    //
                    mTemplateArray = jsonObject.getJSONArray(TAG_TEMPLATE);

                    for(int i=0; i< mTemplateArray.length(); i++) {
                        mTemplateList.add(setParameter2List(mTemplateArray.getJSONObject(i)));
                    }

                    applyBGM2Project(project, jsonObject.getString(TAG_TEMPLATE_BGM));
                }
            } catch (JSONException e) {
                return e.getMessage();
            } catch (Exception e) {
                return e.getMessage();
            }
        } else {
            return "template parsing fail!";
        }
        return null;
    }

    void applyBGM2Project(nexProject project, String bgm)
    {
        if( bgm == null ) return;

        Log.d(TAG, "bgm path1=" + bgm);

        if( mTemplateID != null ) {
            if (bgm.equalsIgnoreCase("null")) {
                project.setBackgroundMusicPath(null);
            }else {
                project.setBackgroundMusicPath(nexAssetPackageManager.getAssetPackageMediaPath(mAppContext, bgm));
            }
        }else {
            if (bgm.equalsIgnoreCase("null")) {
                // in case of not being set
                project.setBackgroundMusicPath(null);
            } else if (bgm.regionMatches(true, 0, "template", 0, 8)) {
                // in case of existing files in assets
                try {
                    project.setBackgroundMusicPath(raw2file(mAppContext, mResContext, bgm));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                // in case of existing files in external storage
                if (bgm.contains("/storage/")) {
                    String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
                    if (bgm.contains("/storage/emulated/0")) {
                        bgm = bgm.replace("/storage/emulated/0", sdCardPath);
                    } else {
                        bgm = bgm.replace("/storage", sdCardPath);
                    }
                }
                project.setBackgroundMusicPath(bgm);
            }
        }
    }

    int templateListID = 0;

    ArrayList<HashMap<String, String>> templateList;

    private boolean mUseProjectSpeed;

    void setUseProjectSpeed( boolean on){
        mUseProjectSpeed = on;
    }

    private boolean mOverlappedTransition = true;
    void setOverlappedTransitionFlag(boolean overlapped) {
        mOverlappedTransition = overlapped;
    }

    void consistProjectViaVer1(nexProject project) {
        int tempClipID;
        int speedValue = 100;
        int rotateDegree;
        int startTime;
        int endTime;
        int remainTime2Project;

        int clipID = 0;

        templateList = mTemplateList;

        nexProject source = nexProject.clone(project);
        project.allClear(true);
        templateListID = 0;
        do {
            //

            if(source.getClip(clipID, true).getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                rotateDegree = source.getClip(clipID, true).getRotateDegree();
                if( mUseProjectSpeed ) {
                    speedValue = source.getClip(clipID, true).getVideoClipEdit().getSpeedControl();
                }

                startTime = source.getClip(clipID, true).getVideoClipEdit().getStartTrimTime();

                endTime = source.getClip(clipID, true).getVideoClipEdit().getEndTrimTime() == 0 ?
                        source.getClip(clipID, true).getTotalTime() : source.getClip(clipID, true).getVideoClipEdit().getEndTrimTime();

                if( mUseProjectSpeed ) {
                    remainTime2Project = source.getClip(clipID, true).getVideoClipEdit().getDuration();
                }else {
                    remainTime2Project = endTime - startTime;
                }


                //nexClip tmpClip = nexClip.getSupportedClip(source.getClip(clipID, true).getPath());
                nexClip tmpClip = nexClip.dup(source.getClip(clipID, true));
                // we don't know how the project consists after executing below codes
                //tempClipID = project.getTotalClipCount(true) - 1;
                //if(tempClipID < 0) tempClipID = 0;
                boolean loop = true;
                int index = 0;
                do {
                    if( mCancel ){
                        return ;
                    }

                    int defined_duration = Integer.parseInt(templateList.get(templateListID).get(TAG_DURATION));
                    if(defined_duration < 0) defined_duration = 0;
                    Log.d(TAG,"remainTime2Project="+remainTime2Project+", defined_duration="+defined_duration+", templateListID="+templateListID);
                    remainTime2Project -= defined_duration;

                    if (remainTime2Project <= 500/* why 500? -> guarantee value to be able to express effects */) {
                        defined_duration += remainTime2Project;
                        loop = false;
                    }
                    Log.d(TAG,"index="+index+", defined_duration="+defined_duration+", loop="+loop);
                    if( mUseProjectSpeed ) {
                        defined_duration *= ((float) speedValue / 100f);
                    }

                    project.add(nexClip.dup(tmpClip));

                    if (templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("ALL") ||
                            templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("VIDEO")) {
                        project.getLastPrimaryClip().setRotateDegree(rotateDegree);
                        if( mUseProjectSpeed ) {
                            project.getLastPrimaryClip().getVideoClipEdit().setSpeedControl(speedValue);
                        }
                        project.getLastPrimaryClip().getVideoClipEdit().setTrim(startTime, startTime + defined_duration);
                    }

                    Rect crop = new Rect();
                    source.getClip(clipID, true).getCrop().getStartPositionRaw(crop);
                    project.getLastPrimaryClip().getCrop().setStartPositionRaw(crop);
                    source.getClip(clipID, true).getCrop().getEndPositionRaw(crop);
                    project.getLastPrimaryClip().getCrop().setEndPositionRaw(crop);

                    startTime += defined_duration;

                    templateListID += 2/* why +2? -> templateList is consisted with clip effect and transition effect */;
                    //
                    Log.d(TAG,"templateListID="+templateListID+", templateList.size()="+templateList.size());
                    if (templateListID >= templateList.size()) {
                        templateListID = 0;
                    }
                //} while (startTime < endTime);
                } while (loop);
            } else if(source.getClip(clipID, true).getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                int defined_duration = Integer.parseInt(templateList.get(templateListID).get(TAG_DURATION));

                rotateDegree = source.getClip(clipID, true).getRotateDegree();
                //nexClip tmpClip = nexClip.getSupportedClip(source.getClip(clipID, true).getPath());
                nexClip tmpClip = nexClip.dup(source.getClip(clipID, true));
                project.add(tmpClip);

                tmpClip.setRotateDegree(rotateDegree);
                tmpClip.setImageClipDuration(defined_duration);

                Rect crop = new Rect();
                source.getClip(clipID, true).getCrop().getStartPositionRaw(crop);
                tmpClip.getCrop().setStartPositionRaw(crop);
                source.getClip(clipID, true).getCrop().getEndPositionRaw(crop);
                tmpClip.getCrop().setEndPositionRaw(crop);

                templateListID += 2/* why +2? -> templateList is consisted with clip effect and transition effect */;
                //
                if (templateListID >= templateList.size()) {
                    templateListID = 0;
                }
            } else {
                if(LL.D) Log.d(TAG, "not support_type in template:"+source.getClip(clipID, true).getClipType());
            }
            clipID++;
        } while(clipID < source.getTotalClipCount(true));

        project.updateProject();
    }

    int introCount = 0;
    int loopCount = 0;
    int tempClipID = 0;

    boolean addSpecialClip2Project(nexProject project) {
        if(templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("EXTERNAL_VIDEO")
                || templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("EXTERNAL_IMAGE")
                || templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("SOLID")) {
            if(templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("EXTERNAL_VIDEO")) {
                nexClip tmpClip = nexClip.getSupportedClip(templateList.get(templateListID).get(TAG_EXTERNAL_VIDEO_PATH));
                if(tmpClip != null) {
                    project.add(tmpClip);
                    //
                    if(templateList == mIntroTemplateList) {
                        introCount++;
                    } else if(templateList == mLoopTemplateList) {
                        loopCount++;
                    }
                    tempClipID++;
                }
            } else {
                int imageclip_duration = Integer.parseInt(templateList.get(templateListID).get(TAG_DURATION));
                if(templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("EXTERNAL_IMAGE")) {
                    nexClip tmpClip = nexClip.getSupportedClip(templateList.get(templateListID).get(TAG_EXTERNAL_IMAGE_PATH));
                    if(tmpClip != null) {
                        tmpClip.setImageClipDuration(imageclip_duration);
                        project.add(tmpClip);
                        //
                        if(templateList == mIntroTemplateList) {
                            introCount++;
                        } else if(templateList == mLoopTemplateList) {
                            loopCount++;
                        }
                        tempClipID++;
                    }
                } else {
                    int solidColor = Color.parseColor(templateList.get(templateListID).get(TAG_SOLID_COLOR));
                    nexClip tmpClip = nexClip.getSolidClip(solidColor);
                    if(tmpClip != null) {
                        tmpClip.setImageClipDuration(imageclip_duration);
                        project.add(tmpClip);
                        //
                        if(templateList == mIntroTemplateList) {
                            introCount++;
                        } else if(templateList == mLoopTemplateList) {
                            loopCount++;
                        }
                        tempClipID++;
                    }
                }
            }
            manageTemplateList(true);
            return true;
        }
        return false;
    }

    void manageTemplateList(boolean bLoop) {

        if(templateList == mIntroTemplateList) {
            //
            // 0: intro, 1: loop, 2: outro
            mTemplateTypeCountList.append(0, introCount);
        } else if(templateList == mLoopTemplateList) {
            //
            // 0: intro, 1: loop, 2: outro
            mTemplateTypeCountList.append(1, loopCount);
        }

        templateListID += 2;
        //
        if(LL.D) Log.d(TAG, "templateListID/templateList.size:"+templateListID+"/"+templateList.size());
        //

        if(bLoop) {
            if(templateListID >= templateList.size()) {
                if(templateList == mIntroTemplateList) {
                    templateListID = 0;
                    templateList = mLoopTemplateList;
                    //
                    if(LL.D) Log.d(TAG, "intro -> loop");
                    //
                } else if(templateList == mLoopTemplateList) {
                    templateListID = 0;
                    //
                    if(LL.D) Log.d(TAG, "loop -> loop");
                    //
                }
            }
        } else {
            if(templateList == mIntroTemplateList) {
                templateListID = 0;
                templateList = mOutroTemplateList;
                //
                // 0: intro, 1: loop, 2: outro
                mTemplateTypeCountList.append(2, 1/* it fixes the value in the case of outro. */);
                //
                if(LL.D) Log.d(TAG, "intro -> outro");
                //
            } else if(templateList == mLoopTemplateList) {
                templateListID = 0;
                templateList = mOutroTemplateList;
                //
                // 0: intro, 1: loop, 2: outro
                mTemplateTypeCountList.append(2, 1/* it fixes the value in the case of outro. */);
                //
                if(LL.D) Log.d(TAG, "loop -> outro");
                //
            }
        }
    }

    void consistProjectViaVer2(nexProject project) {
        int speedValue;
        int rotateDegree;

        int startTime;
        int endTime;
        int remainTime2Project;

        int clipID = 0;
        int introDuration = 0;
        int outroDuration = 0;

        boolean bCompleteWork = false;
        int nextSourceClipID = 0;

        for(int i=0; i<mIntroTemplateList.size(); i+=2) {
            introDuration += Integer.parseInt(mIntroTemplateList.get(i).get(TAG_DURATION));
        }
        for(int j=0; j<mOutroTemplateList.size(); j+=2) {
            outroDuration += Integer.parseInt(mOutroTemplateList.get(j).get(TAG_DURATION));
        }

        templateList = mIntroTemplateList;

        nexProject source = nexProject.clone(project);
        project.allClear(true);

        do {
            if(source.getClip(clipID, true).getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                rotateDegree = source.getClip(clipID, true).getRotateDegree();
                speedValue = source.getClip(clipID, true).getVideoClipEdit().getSpeedControl();
                startTime = source.getClip(clipID, true).getVideoClipEdit().getStartTrimTime();
                endTime = source.getClip(clipID, true).getVideoClipEdit().getEndTrimTime() == 0 ?
                        source.getClip(clipID, true).getTotalTime() : source.getClip(clipID, true).getVideoClipEdit().getEndTrimTime();

                //nexClip tmpClip = nexClip.getSupportedClip(source.getClip(clipID, true).getPath());
                nexClip tmpClip = nexClip.dup(source.getClip(clipID, true));

                // we don't know how the project consists after executing below codes
                tempClipID = project.getTotalClipCount(true);
                if(tempClipID < 0) tempClipID = 0;

                int source_duration = (endTime - startTime);
                //
                if((introDuration + outroDuration) >= source_duration) {
                    remainTime2Project = source_duration;
                    if(LL.D) Log.d(TAG, "OVER durationOfSourceClip/ introDuration:"+introDuration+" outroDuration:"+outroDuration+" sourceDuration:"+source_duration);
                } else {
                    remainTime2Project = source_duration - outroDuration;
                    if(LL.D) Log.d(TAG, "UNDER durationOfSourceClip/ introDuration:"+introDuration+" outroDuration:"+outroDuration+" remainTime2Project:"+remainTime2Project);
                }

                do {
                    if(  mCancel ){
                        return;
                    }
                    addSpecialClip2Project(project);

                    int defined_duration = Integer.parseInt(templateList.get(templateListID).get(TAG_DURATION)) < 0 ? 0 : Integer.parseInt(templateList.get(templateListID).get(TAG_DURATION));
                    int remainTime/* remain duration(excluding current clip) */ = remainTime2Project - defined_duration;
                    int next_defined_duration = 0;
                    int nextListID = templateListID;

                    do {
                        nextListID += 2;
                        if(nextListID >= templateList.size()) {
                            nextListID = 0;
                        }
                        if(templateList.get(nextListID).get(TAG_SOURCE_TYPE).equals("EXTERNAL_VIDEO")
                                || templateList.get(nextListID).get(TAG_SOURCE_TYPE).equals("EXTERNAL_IMAGE")
                                || templateList.get(nextListID).get(TAG_SOURCE_TYPE).equals("SOLID")) {
                            continue;
                        } else {
                            break;
                        }
                    } while(true);

                    next_defined_duration = Integer.parseInt(templateList.get(nextListID).get(TAG_DURATION));

                    if(templateList == mOutroTemplateList || bCompleteWork) {
                        // a duplicated clip adds for being available to trim normally.
                        project.add(nexClip.dup(tmpClip));

                        nexClip tmpClip_ = project.getClip(tempClipID, true);

                        if(templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("ALL") ||
                                templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("VIDEO")) {
                            if(tmpClip_.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                                tmpClip_.setRotateDegree(rotateDegree);
                                tmpClip_.getVideoClipEdit().setSpeedControl(speedValue);
                                tmpClip_.getVideoClipEdit().setTrim(startTime, endTime);
                            }
                        }
                        //
                        startTime += endTime;

                        if(bCompleteWork) {
                            bCompleteWork = false;
                            //
                            if(templateList == mIntroTemplateList) {
                                introCount++;
                            } else if(templateList == mLoopTemplateList) {
                                loopCount++;
                            }
                            manageTemplateList(true);
                        }
                    } else {
                        if(LL.D) Log.d(TAG, "id:"+Integer.parseInt(templateList.get(templateListID).get(TAG_ID))
                                +"/ current defined duration:"+defined_duration
                                +"/ next defined duration:"+next_defined_duration
                                +"/ remain duration(based on source_duration - outro_duration)):"+remainTime);

                        /**
                         *  case 1.  the value, added both introDuration and outroDuration is bigger than sourceDuration.
                         *          it applies the value of the half of sourceDuration at both introDuration and outroDuration.
                         *          we ignore pre-defined both values.
                         */
                        if ((introDuration + outroDuration) >= source_duration) {
                            // a duplicated clip adds for being available to trim normally.
                            project.add(nexClip.dup(tmpClip));

                            nexClip tmpClip_ = project.getClip(tempClipID++, true);

                            if(templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("ALL") ||
                                    templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("VIDEO")) {
                                if(tmpClip_.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                                    tmpClip_.setRotateDegree(rotateDegree);
                                    tmpClip_.getVideoClipEdit().setSpeedControl(speedValue);
                                    tmpClip_.getVideoClipEdit().setTrim(startTime, startTime + remainTime2Project / 2);
                                }
                            }
                            //
                            startTime += (remainTime2Project / 2);

                            if (templateList == mIntroTemplateList) {
                                introCount++;
                            } else {
                                loopCount++;
                            }

                            nextSourceClipID++;
                            if(nextSourceClipID < source.getTotalClipCount(true)) {
                                if(LL.D) Log.d(TAG, "video,case 1] some clips exist in the source project.");
                                bCompleteWork = true;
                                manageTemplateList(true);
                            } else {
                                introCount++;
                                manageTemplateList(false);
                            }
                        }

                        /**
                         *  case 2
                         */
                        else if (remainTime/* remain duration(excluding current clip) */ > next_defined_duration) {
                            //
                            if (remainTime < 0) {
                                continue;
                            } else {
                                remainTime2Project -= defined_duration;
                            }

                            // a duplicated clip adds for being available to trim normally.
                            project.add(nexClip.dup(tmpClip));

                            nexClip tmpClip_ = project.getClip(tempClipID++, true);

                            if(templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("ALL") ||
                                    templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("VIDEO")) {
                                if(tmpClip_.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                                    tmpClip_.setRotateDegree(rotateDegree);
                                    tmpClip_.getVideoClipEdit().setSpeedControl(speedValue);
                                    tmpClip_.getVideoClipEdit().setTrim(startTime, startTime + defined_duration);
                                }
                            }
                            //
                            startTime += defined_duration;

                            if (templateList == mIntroTemplateList) {
                                introCount++;
                            } else {
                                loopCount++;
                            }
                            manageTemplateList(true);
                        }

                        /**
                         *  case 3
                         */
                        else {
                            // a duplicated clip adds for being available to trim normally.
                            project.add(nexClip.dup(tmpClip));

                            nexClip tmpClip_ = project.getClip(tempClipID++, true);

                            if(templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("ALL") ||
                                    templateList.get(templateListID).get(TAG_SOURCE_TYPE).equals("VIDEO")) {
                                if(tmpClip_.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                                    tmpClip_.setRotateDegree(rotateDegree);
                                    tmpClip_.getVideoClipEdit().setSpeedControl(speedValue);
                                    tmpClip_.getVideoClipEdit().setTrim(startTime, startTime + (defined_duration + remainTime / 2));
                                }
                            }
                            //
                            startTime += (defined_duration + remainTime / 2);

                            if (templateList == mIntroTemplateList) {
                                introCount++;
                            } else {
                                loopCount++;
                            }

                            nextSourceClipID++;
                            if(nextSourceClipID < source.getTotalClipCount(true)) {
                                if(LL.D) Log.d(TAG, "video,case 3] the certain clip exist in the source project.");
                                bCompleteWork = true;
                                manageTemplateList(true);
                            } else {
                                manageTemplateList(false);
                            }
                        }
                    }
                } while(startTime < endTime);
            } else if(source.getClip(clipID, true).getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                //
                addSpecialClip2Project(project);
                //
                int defined_duration = Integer.parseInt(templateList.get(templateListID).get(TAG_DURATION));
                rotateDegree = source.getClip(clipID, true).getRotateDegree();
                //
                //nexClip tmpClip = nexClip.getSupportedClip(source.getClip(clipID, true).getPath());
                nexClip tmpClip = nexClip.dup(source.getClip(clipID, true));
                tmpClip.setRotateDegree(rotateDegree);
                tmpClip.setImageClipDuration(defined_duration);
                project.add(tmpClip);
                //
                nextSourceClipID++;
                if(nextSourceClipID < source.getTotalClipCount(true)) {
                    //
                    if (templateList == mIntroTemplateList) {
                        introCount++;
                    } else {
                        loopCount++;
                    }

                    if(nextSourceClipID == source.getTotalClipCount(true)-1) {
                        if(source.getClip(nextSourceClipID, true).getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                            if(LL.D) Log.d(TAG, "image] the clipType of last clip is video, and go continually");
                            manageTemplateList(true);
                        } else {
                            if(LL.D) Log.d(TAG, "image] the clipType of last clip is image, and goto OutTroTemplate");
                            manageTemplateList(false);
                        }
                    } else {
                        if(LL.D) Log.d(TAG, "image] some clips exist in the source project.");
                        manageTemplateList(true);
                    }
                } else {
                    if(LL.D) Log.d(TAG, "image] No more any clip after this image clip.");
                    // 0: intro, 1: loop, 2: outro
                    mTemplateTypeCountList.append(2, 1/* it fixes the value in the case of outro. */);
                }
            } else {
                Log.d(TAG, "not support_type in template:"+source.getClip(clipID, true).getClipType());
            }
            clipID++;
        } while(clipID < source.getTotalClipCount(true));

        if(LL.D) Log.d(TAG, "intro:"+mTemplateTypeCountList.get(0)
                +" loop:"+mTemplateTypeCountList.get(1)
                +" out-ro:"+mTemplateTypeCountList.get(2));

        project.updateProject();
    }

    String setProperty2VideoClip(nexProject project, nexClip clip, int clipID) {
        int volume;
        int contrast;
        int brightness;
        int saturation;
        int colorFilter;
        int speedControl;
        int transitionDuration;

        String type;
        String cropMode;
        String clipEffect;
        String transitionEffect;
        String lut;
        String vignette;

        volume = Integer.parseInt((templateList.get(templateListID).get(TAG_VOLUME).equals("default")) ? "-1" : templateList.get(templateListID).get(TAG_VOLUME));
        contrast = Integer.parseInt((templateList.get(templateListID).get(TAG_CONTRAST).equals("default"))?"-1":templateList.get(templateListID).get(TAG_CONTRAST));
        brightness = Integer.parseInt((templateList.get(templateListID).get(TAG_BRIGHTNESS).equals("default"))?"-1":templateList.get(templateListID).get(TAG_BRIGHTNESS));
        saturation = Integer.parseInt((templateList.get(templateListID).get(TAG_SATURATION).equals("default")) ? "-1" : templateList.get(templateListID).get(TAG_SATURATION));
        colorFilter = Integer.parseInt((templateList.get(templateListID).get(TAG_COLOR_FILTER).equals("default")) ? "-1" : templateList.get(templateListID).get(TAG_COLOR_FILTER));
        speedControl = Integer.parseInt((templateList.get(templateListID).get(TAG_SPEED_CONTROL).equals("default")) ? "-1" : templateList.get(templateListID).get(TAG_SPEED_CONTROL));
        clipEffect = templateList.get(templateListID).get(TAG_EFFECTS);
        lut = templateList.get(templateListID).get(TAG_LUT);
        vignette = templateList.get(templateListID).get(TAG_VIGNETTE);
        cropMode = templateList.get(templateListID).get(TAG_CROP_MODE);
        if(cropMode.equals("")) {
            cropMode = templateList.get(templateListID).get(TAG_VIDEO_CROP_MODE);
        }

        try {
            if(clipEffect.equals("none")) {
                clip.getClipEffect(true).setEffectNone();
            } else {
                clip.getClipEffect(true).setEffect(clipEffect);

                if( mUseProjectSpeed ) {
                    if (clip.getVideoClipEdit().getSpeedControl() != 100) {
                        speedControl = clip.getVideoClipEdit().getSpeedControl();
                    }
                }
                clip.getClipEffect(true).setEffectShowTime(0, 0);

                /*
                // mj_: request from PM1. ->
                //
                int clipShowTime = clip.getProjectEndTime() - clip.getProjectStartTime();
                if (speedControl != -1) {
                    clipShowTime = Math.round((float) (clipShowTime * 100 / speedControl));
                }
                clip.getClipEffect(true).setEffectShowTime(0, clipShowTime);
                // <-
                */
                {// Theme.
                    int position = clipEffect.lastIndexOf(".");
                    String effect_type =  clipEffect.substring(position);
                    if(effect_type.equals(".opening")||effect_type.equals(".middle")||effect_type.equals(".ending")) {
                        clip.getClipEffect(true).setTitle(0, " ");
                        clip.getClipEffect(true).setTitle(1, " ");
                    }
                }
            }
            if(volume != -1) {
                clip.setClipVolume(volume);
            }
            if(brightness != -1) {
                clip.setBrightness(brightness);
            }
            if(contrast != -1) {
                clip.setContrast(contrast);
            }
            if(saturation != -1) {
                clip.setSaturation(saturation);
            }
            if(colorFilter != -1) {
                clip.setColorEffect(getColorEffect(colorFilter - 1));
            }
            if(speedControl != -1) {
                clip.getVideoClipEdit().setSpeedControl(speedControl);
            }
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
                case "fit":                {
                    clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                    break;
                }
                default: {
                    //if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO )
                    //    clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                    break;
                }
            }
            if(lut != null) {
                if (!lut.equals("null")) {
                    if( mTemplateID != null ) {
                        nexColorEffect colorEffect = nexColorEffect.getLutColorEffect(lut);
                        if( colorEffect != null ) {
                            Log.d(TAG,"lut color effect set ="+colorEffect.getPresetName());
                            clip.setColorEffect(colorEffect);
                        }
                    }else {
                        List<nexColorEffect> colorEffects = nexColorEffect.getPresetList();
                        for (nexColorEffect colorEffect : colorEffects) {
                            if (colorEffect.getPresetName().equals(lut)) {
                                clip.setColorEffect(colorEffect);
                                break;
                            }
                        }
                    }
                }
            }
            String[] splitArrary = vignette.split(",");
            if(splitArrary[0].equals("clip")) {
                if(splitArrary[1].equals("yes")) {
                    clip.setVignetteEffect(true);
                } else {
                    clip.setVignetteEffect(false);
                }
            } else if(splitArrary[0].equals("overlay")) {
                addTemplateOverlay(clipID, vignette);
            }

            templateListID += 1;
            type = templateList.get(templateListID).get(TAG_TYPE);
            if(type.equals("transition")) {
                transitionEffect = templateList.get(templateListID).get(TAG_EFFECTS);
                transitionDuration = Integer.parseInt((templateList.get(templateListID).get(TAG_DURATION).equals("default")) ? "-1" : templateList.get(templateListID).get(TAG_DURATION));
                if( mOverlappedTransition ) {
                    int[] guideTimes = project.getTransitionDurationTimeGuideLine(clipID, transitionDuration);
                    if (guideTimes[0] == 0 && guideTimes[1] == 0/*in case both max and min are same.*/) {
                        clip.getTransitionEffect(true).setEffectNone();
                        clip.getTransitionEffect(true).setDuration(0);
                    } else {
                        if (transitionEffect.equals("none")) {
                            clip.getTransitionEffect(true).setEffectNone();
                            clip.getTransitionEffect(true).setDuration(0);
                        } else {
                            clip.getTransitionEffect(true).setTransitionEffect(transitionEffect);
                            if (transitionDuration != -1) {
                                clip.getTransitionEffect(true).setDuration(transitionDuration);
                            }
                        }
                    }
                }else{
                    clip.getTransitionEffect(true).setEffectNone();
                    clip.getTransitionEffect(true).setDuration(0);
                }
            }
        } catch(nexSDKException kde) {
            return kde.getMessage();
        }

        return null;
    }

    String setProperty2ImageClip(nexClip clip, int clipID) {
        int contrast;
        int brightness;
        int saturation;
        int colorFilter;
        int transitionDuration;

        String type;
        String cropMode;
        String clipEffect;
        String transitionEffect;
        String lut;
        String vignette;

        type = templateList.get(templateListID).get(TAG_TYPE);
        if(type.equals("scene")) {
            contrast = Integer.parseInt((templateList.get(templateListID).get(TAG_CONTRAST).equals("default"))?"-1":templateList.get(templateListID).get(TAG_CONTRAST));
            brightness = Integer.parseInt((templateList.get(templateListID).get(TAG_BRIGHTNESS).equals("default"))?"-1":templateList.get(templateListID).get(TAG_BRIGHTNESS));
            saturation = Integer.parseInt((templateList.get(templateListID).get(TAG_SATURATION).equals("default"))?"-1":templateList.get(templateListID).get(TAG_SATURATION));
            colorFilter = Integer.parseInt((templateList.get(templateListID).get(TAG_COLOR_FILTER).equals("default"))?"-1":templateList.get(templateListID).get(TAG_COLOR_FILTER));
            clipEffect = templateList.get(templateListID).get(TAG_EFFECTS);
            lut = templateList.get(templateListID).get(TAG_LUT);
            vignette = templateList.get(templateListID).get(TAG_VIGNETTE);
            cropMode = templateList.get(templateListID).get(TAG_CROP_MODE);
            if(cropMode.equals("")) {
                cropMode = templateList.get(templateListID).get(TAG_IMAGE_CROP_MODE);
            }

            try {
                clip.setClipVolume(0);
                //
                if(clipEffect.equals("none")) {
                    clip.getClipEffect(true).setEffectNone();
                } else {
                    clip.getClipEffect(true).setEffect(clipEffect);
                    clip.getClipEffect(true).setEffectShowTime(0, clip.getProjectEndTime() - clip.getProjectStartTime());
                }
                if(brightness != -1) {
                    clip.setBrightness(brightness);
                }
                if(contrast != -1) {
                    clip.setContrast(contrast);
                }
                if(saturation != -1) {
                    clip.setSaturation(saturation);
                }
                if(colorFilter != -1) {
                    clip.setColorEffect(getColorEffect(colorFilter - 1));
                }
                switch (cropMode) {
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
                    case "fit":{
                        clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                        break;
                    }
                    default: {
                        //if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO )
                        //    clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                        break;
                    }
                }
                if(lut != null) {
                    if (!lut.equals("null")) {
                        List<nexColorEffect> colorEffects = nexColorEffect.getPresetList();
                        for (nexColorEffect colorEffect : colorEffects) {
                            if (colorEffect.getPresetName().equals(lut)) {
                                clip.setColorEffect(colorEffect);
                                break;
                            }
                        }
                    }
                }
                String[] splitArrary = vignette.split(",");
                if(splitArrary[0].equals("clip")) {
                    if(splitArrary[1].equals("yes")) {
                        clip.setVignetteEffect(true);
                    } else {
                        clip.setVignetteEffect(false);
                    }
                } else if(splitArrary[0].equals("overlay")) {
                    addTemplateOverlay(clipID, vignette);
                }

                templateListID++;
                type = templateList.get(templateListID).get(TAG_TYPE);
                if(type.equals("transition")) {
                    transitionEffect = templateList.get(templateListID).get(TAG_EFFECTS);
                    transitionDuration = Integer.parseInt((templateList.get(templateListID).get(TAG_DURATION).equals("default"))?"-1":templateList.get(templateListID).get(TAG_DURATION));
                    if( mOverlappedTransition ) {
                        if (transitionEffect.equals("none")) {
                            clip.getTransitionEffect(true).setEffectNone();
                            clip.getTransitionEffect(true).setDuration(0);
                        } else {
                            clip.getTransitionEffect(true).setTransitionEffect(transitionEffect);
                            if (transitionDuration != -1) {
                                clip.getTransitionEffect(true).setDuration(transitionDuration);
                            }
                        }
                    }else{
                        clip.getTransitionEffect(true).setEffectNone();
                        clip.getTransitionEffect(true).setDuration(0);
                    }
                }
            } catch(nexSDKException kde) {
                return kde.getMessage();
            }
        }
        return null;
    }

    String setProperty2Clips(nexProject project, String templateVersion) {
        int loopId = 0;

        if(project.getTotalClipCount(true) == 0) {
            return "no clip in the project";
        }

        if(templateVersion.equals("template 1.0")) {
            templateList = mTemplateList;
        } else {
            templateList = mIntroTemplateList;
        }
        //
        templateListID = 0;
        //
        do {
            if( mCancel ){
                mCancel = false;
                return cancelMassage;
            }
            nexClip clip = project.getClip(loopId, true);
            //
            if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                String error = setProperty2VideoClip(project, clip, loopId);
                if(error != null) {
                    return error;
                }
            } else if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                String error = setProperty2ImageClip(clip, loopId);
                if(error != null) {
                    return error;
                }
            }
            //
            templateListID++;
            if(templateListID == templateList.size()) {
                templateListID = 0;
            }
            //
            loopId++;
            //
            if(!templateVersion.equals("template 1.0")) {
                if(loopId == mTemplateTypeCountList.get(0/*intro*/)) {
                    if(mTemplateTypeCountList.get(1/*loop*/) == 0) {
                        templateList = mOutroTemplateList;
                    } else {
                        templateList = mLoopTemplateList;
                    }
                    templateListID = 0;
                }
                if(templateList == mLoopTemplateList) {
                    if(loopId == (mTemplateTypeCountList.get(0/*intro*/) + mTemplateTypeCountList.get(1/*loop*/))) {
                        templateList = mOutroTemplateList;
                        templateListID = 0;
                    }
                }
            }
            // We explicitly set the transition effect of the last clip as "NONE".
            if(loopId == project.getTotalClipCount(true)) {
                clip.getTransitionEffect(true).setEffectNone();
                clip.getTransitionEffect(true).setDuration(0);
            }
        } while(loopId < project.getTotalClipCount(true));

        project.updateProject();
        return null;
    }

    String applyTemplateOnProject(nexProject project) {

        if( mTemplate == null ) return "Template did not exist while apply template 2.0";

        // project.updateProject();

        int iDefaultImageDur = mTemplate.getDefaultImageDurationOfTemplate();

        for(int i = 0; i < project.getTotalClipCount(true); i++)
        {
            nexClip clip = project.getClip(i, true);
            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE )
            {
                clip.setImageClipDuration(iDefaultImageDur);
            }

            // Log.d(TAG, String.format("applyTemplateOnProject idx (%d) path(%s) dur(%d)", i, clip.getPath(), clip.getProjectDuration()));
        }
        project.updateProject();

        project.setTemplageOverlappedTransitionMode(mOverlappedTransition);

        String ret2 = mTemplate.updateProjectWithTemplate(project, mAppContext, mResContext, mUseProjectSpeed, mOverlappedTransition);
        if (ret2 != null )
            return ret2;

        return null;
    }

    nexColorEffect getColorEffect(int index) {
        List<nexColorEffect> presetList = nexColorEffect.getPresetList();
        return presetList.get(index);
    }

    static String readFromFile(String filePath) {
        String content;
        File file = new File(filePath);
        try {
            StringBuilder stringBuilder;
            try (InputStream inputStream = new BufferedInputStream(new FileInputStream(file))) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferReader = new BufferedReader(inputStreamReader);
                stringBuilder = new StringBuilder();
                String receiveString;

                while ((receiveString = bufferReader.readLine()) != null) {
                    stringBuilder.append(receiveString);
                    stringBuilder.append("\n");
                }

                inputStream.close();
            }
            content = stringBuilder.toString();
        } catch (FileNotFoundException e) {
            return e.getMessage();
        } catch (IOException e) {
            return e.getMessage();
        }
        return content;
    }

    static String readFromFile(InputStream inputStream) {
        String content = null;

        try {
            if(inputStream != null) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferReader = new BufferedReader(inputStreamReader);
                StringBuilder stringBuilder = new StringBuilder();
                String receiveString;

                while((receiveString = bufferReader.readLine()) != null) {
                    stringBuilder.append(receiveString);
                    stringBuilder.append("\n");
                }

                inputStream.close();
                content = stringBuilder.toString();
            }
        } catch (IOException e) {
            return e.getMessage();
        }
        return content;
    }

    String raw2file(Context appContext, Context resContext, String assetFileName) throws Exception {
        int index = assetFileName.lastIndexOf("/");
        String destFileName = EditorGlobal.getEditorRoot().getAbsolutePath()+File.separator+"template"+File.separator+assetFileName.substring(index + 1, assetFileName.length());
        File bgmfile = new File(destFileName);
        AssetManager am = resContext.getAssets();

        if( bgmfile.isFile() ){
            try {
                AssetFileDescriptor fd = am.openFd(assetFileName);
                if (bgmfile.length() == fd.getLength()) {
                    fd.close();
                    Log.d(TAG, "bgm file found in sdcard.");
                    mBGMPath = destFileName;
                    return mBGMPath;
                }
                fd.close();
            }catch (IOException e){
                Log.d(TAG, "bgm file found in sdcard.");
                mBGMPath = destFileName;
                return mBGMPath;
            }
        }

        InputStream inputStream = am.open(assetFileName);

        if(inputStream != null) {
            //1st sdcard
            try {

                File dir = new File(EditorGlobal.getEditorRoot().getAbsolutePath()+File.separator+"template");
                if( !dir.exists() ){
                    dir.mkdirs();
                }

                in2sdcard(inputStream, bgmfile);
                Log.d(TAG, "bgm file copy assete to sdcard.");
                inputStream.close();
                mBGMPath = destFileName;
                return mBGMPath;
            }catch ( Exception e) {

                destFileName = assetFileName.substring(index + 1, assetFileName.length());
                try {
                    in2file(appContext, inputStream, destFileName);

                    Log.d(TAG, "bgm file copy assete to temp data.");
                    mBGMTempFile = true;
                    mBGMPath = appContext.getFilesDir().getAbsolutePath() + File.separator + destFileName;
                    return mBGMPath;
                }catch (Exception ioe ) {
                    ioe.printStackTrace();
                }
            }
        }
        return null;
    }

    static void  in2sdcard(InputStream in, File destFile) throws IOException {
        int size;
        byte[] w = new byte[1024];
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(destFile);
            while(true) {
                size = in.read(w);
                if (size <= 0)
                    break;
                fos.write(w, 0, size);
            }
        } catch (IOException e) {
            throw e;
        } finally {
            if (fos != null) {
                fos.close();
            }
        }
    }

    void in2file(Context context, InputStream in, String destFileName) throws Exception {
        int size;
        byte[] w = new byte[1024];
        OutputStream out = null;
        try {
            out = context.openFileOutput(destFileName, Context.MODE_WORLD_READABLE);
            while (true) {
                size = in.read(w);
                if (size <= 0)
                    break;
                out.write(w, 0, size);
            }
        } catch (Exception e) {
            throw e;
        } finally{
            if (in != null) {
                in.close();
            }
            if (out != null) {
                out.close();
            }
        }
    }

    static String AssetPackageTemplateJsonToString(String ItemId){
        ItemInfo itemInfo = AssetPackageManager.getInstance().getInstalledItemById(ItemId);
        String content = null;
        if( itemInfo == null){
            Log.e(TAG,"AssetPackageTemplateJsonToString info fail="+ItemId);
            return null;
        }

        if( AssetPackageManager.getInstance().checkExpireAsset(itemInfo.getAssetPackage()) ){
            Log.e(TAG,"AssetPackageTemplateJsonToString expire item id="+ItemId);
            return null;
        }

        AssetPackageReader reader = null;
        InputStream inputStream = null;

        try {
            reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),itemInfo.getPackageURI(),itemInfo.getAssetPackage().getAssetId());
        }catch (IOException e) {
            e.printStackTrace();
            return null;
        }

        Log.d(TAG,"Template("+ItemId+") Asset("+itemInfo.getAssetPackage().getAssetIdx()+") version(In DB)="+itemInfo.getAssetPackage().getPackageVersion()+", version(In reader)="+ reader.getPackageFormatVersion());

        try {
            inputStream = reader.openFile(itemInfo.getFilePath());
            if(inputStream != null) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferReader = new BufferedReader(inputStreamReader);
                StringBuilder stringBuilder = new StringBuilder();
                String receiveString;

                while ((receiveString = bufferReader.readLine()) != null) {
                    stringBuilder.append(receiveString);
                    stringBuilder.append("\n");
                }

                inputStream.close();
                content = stringBuilder.toString();
                return content;
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            CloseUtil.closeSilently(reader);
        }
        return null;
    }

    void setCancel(){
        mCancel = true;
        if( mTemplate != null){
            Log.d(TAG,"setCancel");
            mTemplate.setCancel();
        }else{
            Log.d(TAG,"setCancel mTemplate is null");
        }
    }
}
