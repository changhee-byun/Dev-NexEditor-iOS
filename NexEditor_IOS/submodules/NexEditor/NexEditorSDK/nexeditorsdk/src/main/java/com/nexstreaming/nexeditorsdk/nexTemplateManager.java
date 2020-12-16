/******************************************************************************
 * File Name        : nexTemplateManager.java
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
import android.graphics.Rect;
import android.graphics.RectF;
import android.os.AsyncTask;
import android.util.Log;

import com.google.gson_nex.Gson;
import com.nexstreaming.app.common.util.Stopwatch;
import com.nexstreaming.kminternal.json.TemplateMetaData;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.utils.facedetect.FaceInfo;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.nexeditorsdk.exception.ExpiredTimeException;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * @brief This class provides an easy way to use items of the template Category in nexAssetPackageManager in nexTemplateManager.
 * If the developer uses only templates of nexEditorSDK or separates templates, they can choose not to use nexAssetPackageManager and compose an application with just nexTemplateManager.
 * @since 1.7.0
 */
public class nexTemplateManager {
    private static nexTemplateManager sSingleton = null;
    private Context mAppContext;
    private Context mResContext;
    private static final String TAG="nexTemplateManager";
    private static boolean isApplyTemplate;
    private String errorMsg="";
    private List<Template> templateEntries = new ArrayList<Template>();
    private List<Template> externalView_templateEntries = null;
    private static final ExecutorService sInstallThreadExcutor = Executors.newSingleThreadExecutor();
    private nexTemplateComposer composer;
    private int lastError = 0;
    private boolean mCancel;
    private boolean mUseClipSpeed = false;
    private boolean mUseVideoTrim = true;
    private int mVideoMemorySize = 0;
    private static boolean sAutoAspectSelect = true;
    private static nexAssetPackageManager.Category [] supportCategory = { nexAssetPackageManager.Category.template, nexAssetPackageManager.Category.beattemplate };

    /**
     * Item {@link nexAssetPackageManager.Item} interface of nexAssetPackageManager was inherited to make this class.
     *
     * This processes Template Items in an AssetPackage as a group.
     * Many Templates in an AssetPackage are of the same theme and different resolution, so the developer may choose a resolution via {@link Template#selectAspect(int)}.
     * The resolution set to the SDK is selected by default, but 16:89 is selected in case where there is no resolution supported.
     * @since 1.7.0
     */
    public static class Template extends nexAssetPackageManager.ItemEnt {
        private float aspect[];
        private int selectAspect;
        private int maxAspect;
        private String [] ids;
        private String bgmId;
        private boolean parsed = false;
        private int maxSourceCount;
        private int maxExtendCount = -1;
        private int maxRecommendCount;
        private int internalSourceCount = -1;

        Template(){

        }

        private Template(nexAssetPackageManager.Item item){
            super(item);
        }

        /**
         * If the category of {@link com.nexstreaming.nexeditorsdk.nexAssetPackageManager.Item} is Template, this API can create a Template.
         * @param item - Item received using {@link nexAssetPackageManager#getInstalledAssetItemById(String)}
         * @return - \c NULL if the Item category is not template.
         * @since 1.7.0
         */
        public static Template promote(nexAssetPackageManager.Item item){
            for( nexAssetPackageManager.Category c : supportCategory ){
                if( item.category() == c ){
                    return new Template(item);
                }
            }
            return null;
        }

        private void parseTemplate(){
            if( !parsed ){
                parsed = true;
                String json = nexTemplateComposer.AssetPackageTemplateJsonToString(id());
                if( json != null ) {
                    Gson gson = new Gson();
                    TemplateMetaData.Music meta = gson.fromJson(json, TemplateMetaData.Music.class);
                    if( meta != null ){
                        bgmId = meta.string_audio_id;
                        int count = -1;
                        int recommendCount = 1;
                        int extendsCount = 0;
                        int internalSource = 0;
                        for(TemplateMetaData.EffectEntry ent : meta.list_effectEntries){
                            count++;
                            if( ent.b_source_change || ent.int_priority > 0 ){
                                if( ent.int_priority > 0  ){
                                    extendsCount++;
                                }else {
                                    if( ent.internal_clip_id != null ){
                                        internalSource++;
                                    }else {
                                        recommendCount++;
                                    }
                                }
                            }
                        }
                        maxSourceCount = count;
                        maxRecommendCount = recommendCount;
                        maxExtendCount = extendsCount;
                        internalSourceCount = internalSource;
                    }
                }
            }
        }

        @Override
        public String id(){
            if( sAutoAspectSelect ){
                selectAspect();
            }
            return ids[selectAspect];
        }

        @Override
        public String name(String locale){
            String name = packageInfo().assetName(locale);
            if( name != null)
                return name;
            return super.name(locale);
        }
        /**
         * The currently set Aspect ratio value.
         * @return 16/9 - 16:9 , 1 - 1:1 , 9/16 - 9:16 , 2 - 2:1
         * @since 1.7.0
         */
        public float aspect(){
            if( sAutoAspectSelect ){
                selectAspect();
            }
            return aspect[selectAspect];
        }

        /**
         * The supported Aspect ratio values.
         * @return 16/9 - 16:9 , 1 - 1:1 , 9/16 - 9:16 , 2 - 2:1
         * @since 1.7.0
         */
        public float[] getSupportedAspects(){
            float[] ret = new float[maxAspect];
            for(int i = 0 ; i < ret.length ; i++){
                ret[i] = aspect[i];
            }
            return ret;
        }

        /**
         * The Aspect ratio index to set.
         * @param index Value smaller than the length of getSupportedAspects().
         * @see #getSupportedAspects()
         * @since 1.7.0
         */
        public void selectAspect(int index){
            if( maxAspect <=  index )
                return;
            selectAspect = index;
        }

        public String getVersion() {
            if( id.contains(".ver_") ) {
                String ver = id.substring(id.indexOf(".ver_")+5, id.indexOf(".ver_")+8);
                return ver;
            }
            return "200";
        }

        private void selectAspect(){
            float ar = nexApplicationConfig.getAspectRatio();
            float aspectDim = 3f;
            for(int i = 0 ; i < maxAspect ; i++  ){
                float aspectDelta = ar - aspect[i];
                if( aspectDelta < 0 ) aspectDelta *= -1f;
                if( aspectDim > aspectDelta ){
                    aspectDim = aspectDelta;
                    selectAspect = i;
                }
            }
        }

        void setAspect(String id){
            if( maxAspect >= 5 ){
                return;
            }

            if( this.aspect == null ) {
                this.aspect = new float[5];
                this.ids = new String[5];
            }

            if( id.contains("9v16") ){
                this.aspect[maxAspect] = 9f/16f;
            }else if( id.contains("2v1") ){
                this.aspect[maxAspect] = 2;
            }else if( id.contains("1v2") ){
                this.aspect[maxAspect] = 1f/2f;
            }else if( id.contains("1v1") ){
                this.aspect[maxAspect] = 1;
            }else{
                this.aspect[maxAspect] = 16f/9f;
            }

            this.ids[maxAspect] = id;

            maxAspect++;

            selectAspect();
        }

        /**
         * This gets the ID of the default BGM of a Template.
         * @return ID of the audio category of an Asset.
         * @since 1.7.1
         */
        public String defaultBGMId(){
            if( category() == nexAssetPackageManager.Category.beattemplate ){
                parseTemplate();
                return bgmId;
            }

            if( bgmId == null ){
                String json = nexTemplateComposer.AssetPackageTemplateJsonToString(id());
                if( json != null ) {
                    try {
                        JSONObject jsonObject = new JSONObject(json);
                        bgmId = jsonObject.getString("template_bgm");
                    } catch (JSONException e) {
//                e.printStackTrace();
                    }
                }
            }
            return bgmId;
        }

        /**
         *
         * @return
         * @since 1.7.34
         */
        @Override
        public String[] getSupportedLocales(){
            if( packageInfo() == null){
                return new String[0];
            }
            return packageInfo().getSupportedLocales();
        }
    }

    /**
     * This gets the template information being managed in the form of TemplateEntry. 
     * @return The TemplateEntry of the templates.
     * @since 1.5.42
     */
    public List<Template> getTemplates() {
        if( externalView_templateEntries == null ){
            externalView_templateEntries = Collections.unmodifiableList(templateEntries);
        }
        return externalView_templateEntries;
    }

    /**
     * This gets the package assetIdx of the installed Templates. 
     * @return The package assetIdx of the installed Templates. 
     * @since 1.7.0
     */
    public int[] getTemplateAssetIdxs() {
        synchronized (m_templateEntryLock) {
            int count = 0;
            List<Integer> list = new ArrayList<>();

            for (Template template : templateEntries) {
                if (list.size() == 0) {
                    list.add(template.packageInfo().assetIdx());
                    continue;
                }
                for (int assetidx : list) {
                    if (assetidx == template.packageInfo().assetIdx()) {
                        continue;
                    }
                }
                list.add(template.packageInfo().assetIdx());
            }

            int[] ret = new int[list.size()];
            for (int i = 0; i < ret.length; i++) {
                ret[i] = list.get(i);
            }
            return ret;
        }
    }

    /**
     * This gets the Asset Package IDs of the Templates.
     * {@link #loadTemplate()} has to be run at least once to get normal values.
         * @return
     * @see #loadTemplate()
     * @since 1.7.0
     */
    public String[] getTemplateAssetIds() {
        synchronized (m_templateEntryLock) {
            int count = 0;
            List<String> list = new ArrayList<>();

            for (Template template : templateEntries) {
                if (list.size() == 0) {
                    list.add(template.packageInfo().assetId());
                    continue;
                }
                for (String assetid : list) {
                    if (assetid.compareTo(template.packageInfo().assetId()) == 0) {
                        continue;
                    }
                }
                list.add(template.packageInfo().assetId());
            }

            String[] ret = new String[list.size()];
            for (int i = 0; i < ret.length; i++) {
                ret[i] = list.get(i);
            }
            return ret;
        }
    }

    /**
     * This gets the detailed information of the Template to the Template ID.
     * @param id id - nexAssetPackageManager.ItemEnt.id() or   nexAssetPackageManager.Item.id() of Template Category.
     * @return Template information.
     */
    public Template getTemplateById(String id){
        synchronized (m_templateEntryLock) {
            for (Template template : templateEntries) {
                if (template.id().compareTo(id) == 0)
                    return template;
            }
            return null;
        }
    }

    private nexTemplateManager(Context appContext, Context resContext ){
        mAppContext = appContext;
        mResContext = resContext;
    }

    /**
     * Creates and gets the instance of nexTemplateManager. Call this API first to create an instance of nexTemplateManager.
     *
     * Instances created by this API are saved as a \c static and shared with all.
     * @param appContext The application context.
     * @param resContext The context with Template resources.
     * @return The singleton instance of nexTemplateManager.
     * @see #getTemplateManager()
     * @since 1.5.42
     */
    public static nexTemplateManager getTemplateManager( Context appContext, Context resContext ) {

        if( sSingleton!=null && !sSingleton.mAppContext.getPackageName().equals(appContext.getPackageName()) ) {
            sSingleton = null;
        }
        if( sSingleton==null ) {
            sSingleton = new nexTemplateManager(appContext,resContext);
        }
        return sSingleton;
    }

    /**
     * Gets the instances of nexTemplateManager that have been created.
     * @note Do not call this API first. For it to work properly, {@link #getTemplateManager(Context, Context)} should have been executed.
     * @return The singleton instance of nexTemplateManager. If {@link #getTemplateManager(Context, Context)} was never called before, the return value will be \c NULL.
     * @see #getTemplateManager(Context, Context)
     * @since 1.5.42
     */
    public static nexTemplateManager getTemplateManager(  ) {
        return sSingleton;
    }

    private Template getAssetTemplate(String AssetId){
        for( Template t : templateEntries ){
            if( t.packageInfo().assetId().compareTo(AssetId) == 0 ){
                return t;
            }
        }
        return null;
    }
/*
    private String getDefaultBGMId(String templateId){
        if(templateId == null || getNameById(templateId) == null ){
            return null;
        }
        String template_bgm = null;
        String json = nexTemplateComposer.AssetPackageTemplateJsonToString(templateId);
        if( json != null ) {
            try {
                JSONObject jsonObject = new JSONObject(json);
                template_bgm = jsonObject.getString("template_bgm");
            } catch (JSONException e) {
//                e.printStackTrace();
            }
        }
        return template_bgm;
    }
*/
    private Object m_templateEntryLock = new Object();
    private void resolveTemplate(boolean ignoreExpiredAsset){
        synchronized (m_templateEntryLock) {
            templateEntries.clear();

            for(nexAssetPackageManager.Category c : supportCategory ) {

                List<nexAssetPackageManager.Item> templates = nexAssetPackageManager.getAssetPackageManager(mAppContext).getInstalledAssetItems(c);
                for (nexAssetPackageManager.Item info : templates) {
                    if (info.hidden() || (ignoreExpiredAsset && nexAssetPackageManager.getAssetPackageManager(mAppContext).checkExpireAsset(info.packageInfo()))) {
                        continue;
                    }
                    Template t = getAssetTemplate(info.packageInfo().assetId());
                    if (t == null) {
                        t = Template.promote(info);
                        templateEntries.add(t);
                    }
                    t.setAspect(info.id());
                }
            }
        }
    }

    /**
     * Loads the template resources from the assets and external drive.
     *
     * This API works through blocking.
     * For faster loading, it does not check template effects.
     * @since 1.5.42
     */
    public void loadTemplate(final Runnable onCompleteListener){

        new Thread(new Runnable(){

            @Override
            public void run(){

                resolveTemplate(false);
                onCompleteListener.run();
            }
        }).start();
    }

    public void loadTemplate(){
        resolveTemplate(false);
    }

    /**
     * Loads the template resources from the assets and external drive.
     *
     * This API works through blocking.
     * For faster loading, it does not check template effects.
     * @param ignoreExpired false - load expired asset, true - don't load expired asset
     * @since 1.7.51
     */
    public void loadTemplate(boolean ignoreExpired){
        resolveTemplate(ignoreExpired);
    }

    private String getNameById(String id){
        synchronized (m_templateEntryLock) {
            for (Template ent : templateEntries) {
                if (ent.id() != null && ent.id().compareTo(id) == 0) {
                    return ent.name(null);
                }
            }
            return null;
        }
    }

    /**
     * This applies a Template to a nexProject.
     *
     * This method automatically applies effects to clips in a nexProject.
     * @param project The project to which Template will be applied to; there must be at least one video clip. 
     * @param templateId nexAssetPackageManager.ItemEnt.id() or nexAssetPackageManager.Item.id() of Template Category.
     * @return \c TRUE - Template is properly applied to the \c FALSE - Applying Template failed.
     * @since 1.7.0
     */
    public boolean applyTemplateToProjectById(nexProject project , String templateId) throws ExpiredTimeException{
        if( isApplyTemplate ){
            Log.d(TAG,"applyTemplateToProjectById errorMsg= already run applyTemplate");
            lastError = -3;
            return false;
        }
        isApplyTemplate = true;
        boolean ret = false;
        try {
            ret = internalApplyTemplateToProjectById(project, templateId, true, nexEngine.nexUndetectedFaceCrop.NONE.getValue());
        } catch (ExpiredTimeException e) {
            isApplyTemplate = false;
            throw e;
        }
        isApplyTemplate = false;
        return ret;
    }

    /**
     * This applies a Template to a nexProject.
     *
     * This method automatically applies effects to clips in a nexProject.
     * @param project The project to which Template will be applied to; there must be at least one video clip.
     * @param templateId nexAssetPackageManager.ItemEnt.id() or nexAssetPackageManager.Item.id() of Template Category.
     * @param overlappedTransitionEffect If true, overlapped transition effect
     * @return \c TRUE - Template is properly applied to the \c FALSE - Applying Template failed.
     * @since 1.7.0
     */
    public boolean applyTemplateToProjectById(nexProject project , String templateId, boolean overlappedTransitionEffect) throws ExpiredTimeException{
        if( isApplyTemplate ){
            Log.d(TAG,"applyTemplateToProjectById errorMsg= already run applyTemplate");
            lastError = -3;
            return false;
        }
        isApplyTemplate = true;

        boolean ret = false;
        try {
            ret = internalApplyTemplateToProjectById(project,templateId, overlappedTransitionEffect, nexEngine.nexUndetectedFaceCrop.NONE.getValue());
        }catch (ExpiredTimeException e ){
            isApplyTemplate = false;
            throw e;
        }

        isApplyTemplate = false;
        return ret;
    }

    /**
     * This applies a Template to a nexProject.
     *
     * This method automatically applies effects to clips in a nexProject.
     * @param project The project to which Template will be applied to; there must be at least one video clip.
     * @param templateId nexAssetPackageManager.ItemEnt.id() or nexAssetPackageManager.Item.id() of Template Category.
     * @param overlappedTransitionEffect If true, overlapped transition effect
     * @param facemode 
     * @return \c TRUE - Template is properly applied to the \c FALSE - Applying Template failed.
     * @since 1.7.0
     */
    public boolean applyTemplateToProjectById(nexProject project , String templateId, boolean overlappedTransitionEffect, int facemode) throws ExpiredTimeException{
        if( isApplyTemplate ){
            Log.d(TAG,"applyTemplateToProjectById errorMsg= already run applyTemplate");
            lastError = -3;
            return false;
        }
        isApplyTemplate = true;
        boolean ret = false;
        try {
            ret = internalApplyTemplateToProjectById(project, templateId, overlappedTransitionEffect, facemode);
        }catch (ExpiredTimeException e ){
            isApplyTemplate = false;
            throw e;
        }

        isApplyTemplate = false;
        return ret;
    }

    private boolean internalApplyTemplateToProjectById(nexProject project , String templateId, boolean overlappedTransitionEffect, int facemode) throws ExpiredTimeException{
        Stopwatch watch = new Stopwatch();
        watch.start();
        lastError = 0;
        if( getNameById(templateId) == null ){
            lastError = -1;
            return false;
        }

        Template t = getTemplateById(templateId);
        if( t == null ){
            lastError = -1;
            return false;
        }

        if( nexAssetPackageManager.checkExpireAsset(t.packageInfo()) ){
            lastError = -2;
            throw new ExpiredTimeException(templateId);
        }

        if( t.category() == nexAssetPackageManager.Category.beattemplate ){
            return nexBeatTemplateManager.internalApplyTemplateToProjectById(project,templateId);
        }

        if( composer == null ){
            composer = new nexTemplateComposer();
        }

        EditorGlobal.getEditor().setFirstFaceThread(null);


        if(facemode != 0){

            final nexClip clip = project.getClip(0, true);
            final int inner_facemode = facemode;

            if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE){

                EditorGlobal.getEditor().waitFaceThread();
                Thread check = new Thread(new Runnable(){

                    @Override
                    public void run(){

                            getFirstFace(clip, inner_facemode);
                            }
                        }
                    );

                check.start();
                EditorGlobal.getEditor().setFirstFaceThread(check);
            }
            
        }

        composer.setUseProjectSpeed(mUseClipSpeed);
        if( nexConfig.getProperty(nexConfig.kHardwareDecMaxCount) == 1 ){
            composer.setOverlappedTransitionFlag(false);
        }else {
            composer.setOverlappedTransitionFlag(overlappedTransitionEffect);
        }
        //nexTemplateComposer tc = new nexTemplateComposer();
        errorMsg = composer.setTemplateEffects2Project(project, mAppContext, mResContext, templateId, false);
        if( errorMsg != null ){
            Log.d(TAG,"internalApplyTemplateToProjectById errorMsg="+errorMsg);
            composer.release();
            watch.stop();
            Log.d(TAG,"internalApplyTemplateToProjectById error elapsed="+watch.toString());
            lastError = -2;
            return false;
        }
        composer.release();
        watch.stop();
        Log.d(TAG,"internalApplyTemplateToProjectById elapsed="+watch.toString());
        return true;
    }

    private final class applyTemplateTask extends AsyncTask<Void, Integer, Void>{
        nexProject project;
        String templateId;
        Runnable onCompleteListener;
        boolean overlappedTransitionEffect;
        private boolean ret;

        applyTemplateTask(nexProject project, String templateId,boolean overlappedTransitionEffect, Runnable onCompleteListener){
            this.project = project;
            this.templateId = templateId;
            this.onCompleteListener = onCompleteListener;
            this.overlappedTransitionEffect= overlappedTransitionEffect;
        }

        @Override
        protected Void doInBackground(Void... params) {
            try {
                ret = internalApplyTemplateToProjectById(project,templateId, overlappedTransitionEffect, nexEngine.nexUndetectedFaceCrop.NONE.getValue());
            } catch (ExpiredTimeException e) {
                e.printStackTrace();
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( !mCancel ) {
                if (onCompleteListener != null) {
                    onCompleteListener.run();
                }
            }
            isApplyTemplate = false;
        }
    }

    /**
     *
     * <pre>
     * {@code
     * if( progressDialog == null)
        progressDialog = new ProgressDialog(this);
        progressDialog.show();
    nexTemplateManager.getTemplateManager(context,context).applyTemplateAsyncToProjectById( mProject, mId, new Runnable() {

            public void run() {
                progressDialog.dismiss();
            }
      });

     * }</pre>
     * @param project
     * @param templateId
     * @param onCompleteListener
     * @return
     * @since 1.7.8
     */
    public boolean applyTemplateAsyncToProjectById( nexProject project ,  String templateId,  Runnable onCompleteListener) throws ExpiredTimeException{
        if( isApplyTemplate ){
            lastError = -3;
            return false;
        }

        Template t = getTemplateById(templateId);
        if( t == null ){
            lastError = -1;
            return false;
        }

        if( nexAssetPackageManager.checkExpireAsset(t.packageInfo()) ){
            lastError = -2;
            throw new ExpiredTimeException(templateId);
        }

        isApplyTemplate = true;
        mCancel = false;
        applyTemplateTask task = new applyTemplateTask(project,templateId,true,onCompleteListener);
        task.executeOnExecutor(sInstallThreadExcutor);

        return true;
    }

    /**
     *
     * @param project
     * @param templateId
     * @param overlappedTransitionEffect
     * @param onCompleteListener
     * @return
     * @since 1.7.58
     */
    public boolean applyTemplateAsyncToProjectById( nexProject project ,  String templateId, boolean overlappedTransitionEffect,  Runnable onCompleteListener)throws ExpiredTimeException{
        if( isApplyTemplate ){
            lastError = -3;
            return false;
        }

        Template t = getTemplateById(templateId);
        if( t == null ){
            lastError = -1;
            return false;
        }

        if( nexAssetPackageManager.checkExpireAsset(t.packageInfo()) ){
            lastError = -2;
            throw new ExpiredTimeException(templateId);
        }


        isApplyTemplate = true;
        mCancel = false;
        applyTemplateTask task = new applyTemplateTask(project,templateId,overlappedTransitionEffect,onCompleteListener);
        task.executeOnExecutor(sInstallThreadExcutor);

        return true;
    }



    /**
     * If {@link #applyTemplateToProjectById(nexProject, String)} failed, this gets the error message which contains the cause.
     * @return \c NULL - OK; others - a message about exceptions that occurred.
     * @since 1.7.0
     */
    public String getLastErrorMessage(){
        return errorMsg;
    }

    /**
     * This gets the IDs of the installed Templates.
     * @return The IDs of the installed Templates.
     * @since 1.7.0
     */
    public String[] getTemplateIds(){
        String[] ret = new String[templateEntries.size()];
        for (int i = 0; i < ret.length ; i++ ) {
            ret[i] = templateEntries.get(i).id();
        }
        return ret;
    }

    /**
     * This uninstalls a Template.
     *
     * This API is same as nexAssetPackageManager.uninstallPackageById(String).
     * @param templateId nexAssetPackageManager.ItemEnt.id() or nexAssetPackageManager.Item.id() of Template Category.
     * @since 1.7.0
     */
    public void uninstallPackageById(String templateId){
        nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).uninstallPackageById(templateId);
    }

    /**
     * This checks if there is any new Template package.
     *
     * This API is same as {@link nexAssetPackageManager#uninstallPackageById(String)}.
     * @return The number of new packages to install. 
     * @since 1.7.0
     */
    public int findNewPackages(){
        return nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).findNewPackages();
    }

    /**
     * This API checks if installPackagesAsync is running or not.
     *
     * This API is same as {@link nexAssetPackageManager#isInstallingPackages()}.
     * @return
     * @since 1.7.0
     */
    public boolean isInstallingPackages(){
        return nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).isInstallingPackages();
    }

    /**
     * This API installs Templates received from the Asset Store.
     *
     * As a pre-requisite, run findNewPackages() first. 
     * If findNewPackages() finds a new package, then the developer should use it. 
     * installPackagesAsync is a single task, so check if it is running using isInstallingPackages() before using it. 
     * This API is same as {@link nexAssetPackageManager#installPackagesAsync(nexAssetPackageManager.OnInstallPackageListener)}.
     * @param listener
     * @since 1.7.0
     */
    public void installPackagesAsync(final nexAssetPackageManager.OnInstallPackageListener listener){
        nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).installPackagesAsync(listener);
    }

    public void installPackagesAsync(int downloadedAssetIdx, final nexAssetPackageManager.OnInstallPackageListener listener){
        nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).installPackagesAsync(downloadedAssetIdx,listener);
    }


    /**
     * @since 1.7.8
     */
    public void cancel(){
        mCancel = true;
        //Log.d(TAG,"cancel");
        if( !isApplyTemplate ){
            return;
        }
        if( composer != null ) {
            composer.setCancel();
        }
    }

    /**
     *
     * @return
     * @since 1.7.8
     */
    public int getLastError(){
        return lastError;
    }

    /**
     *
     * @param reload
     * @param assetIdx
     * @return
     */
    public String getTemplateItemId(boolean reload , int assetIdx){
        if( reload ){
            loadTemplate();
        }

        if( assetIdx < 2 ){
            return null;
        }
        String id = null;
        synchronized (m_templateEntryLock) {
            for (Template t : templateEntries) {
                if (t.packageInfo().assetIdx() == assetIdx) {
                    id = t.id();
                    break;
                }
            }
        }
        return id;
    }

    @Deprecated
    public void getFirstFace(nexClip clip, int facemode){

        Log.d(TAG,"getFirstFace In");
        String path = clip.getPath();
        FaceInfo face = FaceInfo.getFaceInfo(path);
        if(null == face){

            Log.d(TAG,"getFirstFace Proc");
            face = new FaceInfo(new File(path), true, mAppContext);
            FaceInfo.putFaceInfo(path, face);
        }
        // onFirstFaceDone(clip, face, facemode);
        Log.d(TAG,"getFirstFace Out");
    }

    void onFirstFaceDone(nexClip clip, FaceInfo faceInfo, int facemode) {

        int innerWidth = clip.getCrop().getWidth();
        int innerHeight = clip.getCrop().getHeight();

        boolean face_detected =true; 
        RectF _sbounds = new RectF(); 
        Rect sbounds = new Rect();
        Rect facebounds = new Rect();
        Rect fullbounds = new Rect(0, 0, innerWidth, innerHeight);
        faceInfo.getBounds(_sbounds);

        sbounds.set( (int)(_sbounds.left*innerWidth), (int)(_sbounds.top*innerHeight), (int)(_sbounds.right*innerWidth), (int)(_sbounds.bottom*innerHeight) );
        facebounds.set( (int)(_sbounds.left*innerWidth), (int)(_sbounds.top*innerHeight), (int)(_sbounds.right*innerWidth), (int)(_sbounds.bottom*innerHeight) );

        if( sbounds.isEmpty() ) {
            sbounds.set(0, 0, innerWidth*3/4, innerHeight*3/4);
            sbounds.offset((int)(innerWidth*Math.random()/4), (int)(innerHeight*Math.random()/4));
            Log.d(TAG,"Face Detection Empty ");
            face_detected =false;

        } else {

            int addSpace;
            addSpace = (innerWidth/4)-sbounds.width();
            if( addSpace>=2 ) {
                sbounds.left -= addSpace/2;
                sbounds.right += addSpace/2;
                Log.d(TAG,"Face Detection width addSpace>0 ");
            }
            addSpace = (innerHeight/4)-(int)sbounds.height();
            if( addSpace>=2 ) {
                sbounds.top -= addSpace/2;
                sbounds.bottom += addSpace/2;
                Log.d(TAG,"Face Detection height addSpace>0 ");
            }

            clip.getCrop().growToAspect(sbounds, nexApplicationConfig.getAspectRatio());

            if(!sbounds.intersect(0, 0, innerWidth, innerHeight)) {
                sbounds.set(0, 0, innerWidth*2/3, innerHeight*2/3);
                Log.d(TAG,"Face Detection insect not ");
                sbounds.offset((int)(innerWidth*Math.random()/3), (int)(innerHeight*Math.random()/3));
            }
            fullbounds.set(0, 0, innerWidth*3/4, innerHeight*3/4);
            fullbounds.offset((int)(innerWidth*Math.random()/4), (int)(innerHeight*Math.random()/4));
        }

        if(face_detected == true){

            clip.getCrop().shrinkToAspect(sbounds, nexApplicationConfig.getAspectRatio());                       
            clip.getCrop().shrinkToAspect(fullbounds, nexApplicationConfig.getAspectRatio());      
            Log.d(TAG,"Face Detection aync true  ");

            int addspace = sbounds.right - sbounds.left;
            sbounds.left -= addspace / 4;
            sbounds.right += addspace / 4;
            addspace = sbounds.bottom - sbounds.top;
            sbounds.top -= addspace / 4;
            sbounds.bottom += addspace / 4;
            clip.getCrop().growToAspect(sbounds, nexApplicationConfig.getAspectRatio());

            clip.getCrop().setStartPosition(fullbounds);
            clip.getCrop().setEndPosition(sbounds);
            clip.getCrop().setFacePosition(facebounds);
            clip.getCrop().getStartPositionRaw(fullbounds);
            clip.getCrop().getEndPositionRaw(sbounds);
            clip.getCrop().getFacePositionRaw(facebounds);
    
            EditorGlobal.getEditor().updateRenderInfo(1, 1, fullbounds, sbounds, facebounds);

        }else{

            clip.getCrop().shrinkToAspect(sbounds, nexApplicationConfig.getAspectRatio());
            if(facemode == 1){
                clip.getCrop().shrinkToAspect(fullbounds, nexApplicationConfig.getAspectRatio());      
            }else clip.getCrop().growToAspect(fullbounds, nexApplicationConfig.getAspectRatio());
            
            clip.getCrop().setStartPosition(sbounds);
            clip.getCrop().setEndPosition(fullbounds);
            clip.getCrop().setFacePosition(facebounds);
            clip.getCrop().getStartPositionRaw(sbounds);
            clip.getCrop().getEndPositionRaw(fullbounds);
            clip.getCrop().getFacePositionRaw(facebounds);
            
            Log.d(TAG,"Face Detection aync false  ");

            EditorGlobal.getEditor().updateRenderInfo(1, 0, fullbounds, sbounds, facebounds);
        }
        clip.setFaceDetectProcessed(face_detected, facebounds);
        Log.d(TAG,"Face Detection aync thread end ="+clip.getPath());
    }

    /**
     *
     * @param on
     * @since 1.7.27
     */
    public void setUseClipSpeed(boolean on){
        mUseClipSpeed = on;
    }

    /**
     *
     * This method sets a trim mode while applying template.
     * @param on  default is true.
     *            true is to use video clip trim mode otherwise do not use trim mode.
     * @since 2.0.10
     */
    public void setVideoClipTrimMode(boolean on){
        mUseVideoTrim = on;
    }

    /**
     *
     * This method sets available memory size of device for video decode.
     * @param memorySize Available memory size of device.
     *                   The memory size have to be setted 1920*1080*2 if the device only support two 1080P content.
     * @since 2.0.10
     */
    public void setVideoMemorySize(int memorySize){
        mVideoMemorySize = memorySize;
    }

    /**
     *
     * @param on
     * @since 1.7.47
     */
    public static void setAutoSelectFromAspect(boolean on){
        sAutoAspectSelect = on;
    }

    boolean updateTemplate(boolean installed , nexAssetPackageManager.Item item){
        synchronized (m_templateEntryLock) {
            boolean update = false;
            if (installed) {
                if (item.hidden()) {
                    return update;
                }

                Template t = getAssetTemplate(item.packageInfo().assetId());
                if (t == null) {
                    t = Template.promote(item);
                    //t.bgmId = getDefaultBGMId(item.id());
                    templateEntries.add(t);
                }
                t.setAspect(item.id());
                update = true;
            } else {
                for (Template t : templateEntries) {
                    if (t.packageInfo().assetIdx() == item.packageInfo().assetIdx()) {
                        templateEntries.remove(t);
                        update = true;
                        break;
                    }
                }
            }
            return update;
        }
    }
}
