/******************************************************************************
 * File Name        : nexProject.java
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

import android.content.Intent;
import android.graphics.Rect;
import android.util.Log;

import com.google.gson_nex.Gson;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.editorwrapper.KMIntentBuilder;
import com.nexstreaming.nexeditorsdk.exception.ProjectAlreadyAttachedException;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;

/**
 * This class manages clips to be edited with NexEditor&trade;&nbsp;SDK as projects. 
 * 
 * To edit with the NexEditor&trade;&nbsp;SDK, a project is created, and then clips (video, image, or audio)
 * are added to the project and further edited with the options available.
 * 
 * A finished project is then exported to become the final version of a NexEditor&trade;&nbsp; video.
 * 
 * The following sample code demonstrates how to create a project and add clips to the project:
 * <b>class sample code</b>
 * {@code nexProject project = new nexProject(mThemeID,"Open","Close");
    for(int i=m_listfilepath.size()-1; i>=0; i--) {
        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(i));
        if( clip != null )
            project.add(clip);
    }
    String transitionID = "nexstring..XXXXXX";
    project.getClip(0,true).getTransitionEffect().setTransitionEffect(transitionID);}
 </tt>
 * @since version 1.0.0
 */
public class nexProject extends nexObserver {
    private static String TAG="nexProject";
    private String mThemeId = EditorGlobal.DEFAULT_THEME_ID;
    private String mLetterbox  = nexApplicationConfig.getDefaultLetterboxEffect();
    private nexTheme mTheme = null;
    private boolean mEmptyTheme = false;
//    private String mBackGroundMusic = null;
    private int mProjectVolume = 100;
    private int mManualVolCtl = 0;
    private int mAudioFadeInTime = 200;
    private int mAudioFadeOutTime = 5000;

    private String mOpeningTitle = null;
    private String mEndingTitle = null;

    private float mBGMVolumeScale = 0.5f;
    private int mId;
    private static int sNextId = 1;

    boolean mUseThemeMusic2BGM = true;
    boolean mLoopBGM = true;
    int mStartTimeBGM = 0;
    private nexClip mBackGroundMusic = null;
    int mBGMTrimStartTime;
    int mBGMTrimEndTime;

    private int mTemplateApplyMode = 0;
    private boolean mTemplateOverlappedTransition = true;
    /**
     * The time to set as the automatic duration for image clips in a particular theme. 
     * @since version 1.0.0
     */
    final public static int kAutoThemeClipDuration = 6000;

    /**
     * The time to set as the automatic duration of a transition effect applied between project clips.
     * @since version 1.0.0
     */
    final public static int kAutoThemeTransitionDuration = 2000;

    /**
     * This method creates a project using the basic theme.
     *
     * <p>Example code :</p>
     *      {@code
                private nexProject project = new nexProject();
            }
     * @since version 1.0.0
     */
    public nexProject() {
        mThemeId = null;
        mEmptyTheme = true;
        mId = sNextId++;
        Log.d(TAG,"["+mId+"] nexProject create");
    }

    public String getLetterboxEffect(){

        return mLetterbox;
    }

    public void setLetterboxEffect(String effect_uid){

        mLetterbox = effect_uid;
    }

    /**
     * This method creates an instance of a project using a chosen <tt>Theme</tt>.
     * 
     * In NexEditor&trade;&nbsp;SDK, a theme includes background music as well as opening and closing title  
     * and related transition effects to create a visually cohesive video experience for a user who doesn't want to set 
     * each individual option when editing.
     *
     * <p>Example code :</p>
     * {@code mProject = new nexProject(EffectLibrary.getEffectLibrary(getApplicationContext()).findThemeById(com.nexstreaming.kinemaster.travel));}
     * @param Theme The theme to set for the project being created.
     * 
     * @since version 1.0.0
     */
    @Deprecated
    public nexProject(nexTheme Theme){
        mThemeId = null;
        mEmptyTheme = true;
        mId = sNextId++;
        Log.d(TAG,"["+mId+"] nexProject create");
    }

    /**
     * This method creates an instance of a project using the received theme ID from the <tt>ThemeId</tt> parameter.
     *
     * <p>Example code :</p>
     * {@code private nexProject project = new nexProject("com.nexstreaming.kinemaster.travel");}
     * @param ThemeId The ID of the theme to set for the project being created, as a <tt>String</tt>.
     * 
     * @since version 1.0.0
     */
    @Deprecated
    public nexProject(String ThemeId) {
        mThemeId = null;
        mEmptyTheme = true;
        mId = sNextId++;
        Log.d(TAG,"["+mId+"] nexProject create");
    }

    /**
     * The generator of NexEditor&trade;&nbsp; projects.
     *
     * <p>Example code :</p>
     * {@code nexProject project = new nexProject(mThemeID,"Open my Create","End my Close");}
     * @param ThemeId  The theme ID of the project as a <tt>String</tt>.
     * @param open The text title that appears at the opening of the video project.
     * @param end  The text title that appears at the end of the video project.
     * @since version 1.0.0
     */
    @Deprecated
    public nexProject(String ThemeId, String open, String end) {


        mThemeId = null;
        mEmptyTheme = true;
        mId = sNextId++;
        Log.d(TAG,"["+mId+"] nexProject create");
    }

    /**
     * This method sets the opening and ending titles of a project.
     *
     * <p>Example code :</p>
     * {@code project.setTitle("Open my Create","End my Close");}
     * @param open The text title that appears at the opening of the video.
     * @param end  The text title that appears at the end of the video.
     * @since version 1.0.0
     */
    public void setTitle(String open, String end ){
        mOpeningTitle = open;
        mEndingTitle = end;
        updateTimeLine(false);
        mNeedLoadList = true;
    }

    /**
     * This method gets the opening title of a project.
     *
     * <p>Example code :</p>
     * {@code strTitle = mProject.getOpeningTitle();}
     * @return The title set for the theme opening of a project.
     * @since version 1.0.0
     */
    public String getOpeningTitle(){
        return mOpeningTitle;
    }

    /**
     * This method get the ending title of a project.
     *
     * <p>Example code :</p>
     * {@code strTitle = mProject.getEndingTitle();}
     * @return The text set for the theme ending of a project.
     * @since version 1.0.0
     */
    public String getEndingTitle(){
        return mEndingTitle;
    }
/*
    public void save( OutputStream outputStream ) {

    }

    public static nexProject load( InputStream inputStream ) {
        return null;
    }
*/
    /**
     * This method gets a read-only list of clips, from the track with video clips and image clips only.  
     * A project contains two different kinds of tracks: one track for video and image clips and other tracks for audio clips.
     *
     * <p>Example code :</p>
     * {@code List<nexClip> list = mProject.getPrimaryItems();}
     * @return The list of <tt>nexClip</tt> clips, that are video or image clips.
     * @since version 1.0.0 
     */
    public List<nexClip> getPrimaryItems() {
        if( m_externalView_primaryItems==null )
            m_externalView_primaryItems = Collections.unmodifiableList(mPrimaryItems);
        return m_externalView_primaryItems;
    }

    /**
     * This method gets a read-only list from the track with audio clips only. 
     * A project contains two different kinds of tracks: one track for video and image clips and other tracks for audio clips.
     * @return The list of <tt> nexClip</tt> audio clips.
     * @since version 1.0.0
     *
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public List<nexClip> getSecondaryItems() {
        return null;
    }

    /** 
     * This method gets a list of the audio items that can be trimmed and have their volume adjusted.
     *
     * <p>Example code :</p>
     * {@code List<nexAudioItem> alist =  mProject.getAudioItems();}
     * @return A list of the audio items, <tt>nexAudioItem</tt>, that can be trimmed and volume-adjusted.
     * @since version 1.1.0
     */
    public List<nexAudioItem> getAudioItems() {
        if( m_externalView_secondaryItems==null )
            m_externalView_secondaryItems = Collections.unmodifiableList(mSecondaryItems);
        return m_externalView_secondaryItems;
    }

    private int isPrimaryItem(nexClip item) {
        if( item.getClipType() == nexClip.kCLIP_TYPE_AUDIO  ) {
            return 0;
        }

        if( item.getClipType() == nexClip.kCLIP_TYPE_VIDEO || item.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
            return 1;
        }

        return -1;
    }

    /**
     * This method gets the total number of clips saved in a specific track.
     *
     * <p>Example code :</p>
     * {@code for(int i = 0; i<mProject.getTotalClipCount(false); i++){
            if(duration < mProject.getAudioItem(i).getEndTime()){
                duration += mProject.getAudioItem(i).getEndTime();
            }
        }}
     * @param bPrimary Set to <tt>TRUE</tt> if the track is composed of video clips and image clips only; set to <tt>FALSE</tt> if the track is composed of audio clips only.
     * 
     * @return The number of clips in the selected track, as an <tt>integer</tt>.
     * @since version 1.0.0
     */
    public int getTotalClipCount(boolean bPrimary){
        if( bPrimary ) {
            return mPrimaryItems.size();
        }
        return mSecondaryItems.size();
    }

    /**
     * This method gets the total number of image clips and video clips in a project.
     * 
     * This includes the number of trimmed video clips. For example if there is one image clip 
     * and video clips that have been divided and trimmed into three parts, then 1 + 3 = 4 will be returned.
     * 
     * @return The total number of visual clips in the project, as an <tt>integer</tt>.
     * @since version 1.0.0
     * 
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public int getTotalVisualClipCount(){
        int clipcount = mPrimaryItems.size();
        int total = 0;
        for( int i = 0 ; i < clipcount ; i++ ){
            nexClip clip = mPrimaryItems.get(i);
            if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
                int count = clip.getVideoClipEdit().getTrimCount();
                if(count == 0) count = 1;
                total += count;
            }else if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ){
                total++;
            }
        }
        return total;
    }

    /**
     * This method adds a clip to a track. 
     *
     * <p>Example code :</p>
     * {@code nexClip clip = nexClip.getSolidClip(Color.RED);
        if( clip != null ) {
            project.add(clip);
        }}
     * @param item The <tt>nexClip</tt> instance to be added.
     * 
     * @return 0 if successful; otherwise a <tt> RuntimeException</tt>.
     * @since version 1.0.0
     */
    public int add( nexClip item ) {
        //updateTimeLine(true);
        int rval = isPrimaryItem(item);
        if( rval < 0  )
            return rval;
        if( rval == 1  ) {
            return add(mPrimaryItems.size(),true, item);
        }
        return add(mSecondaryItems.size(),false, item);
    }

    /**
     * This method adds a clip to a track.
     *
     * <p>Example code :</p>
     * {@code nexClip clip = nexClip.getSolidClip(Color.RED);
        if( clip != null ) {
            project.add(index, true, clip);
        }}
     * @param index  The index of the clip to be added.
     * @param bPrimary Set to <tt>TRUE</tt> if the track is composed of video clips and image clips only; set to <tt>FALSE</tt> if the track is composed of audio clips only. 
     * @param item The <tt>nexClip</tt> instance to be added.
     * 
     * @return 0 if successful; otherwise a <tt> RuntimeException</tt>.
     * @since version 1.0.0
     */
    public int add( int index, boolean bPrimary, nexClip item ) {
        if( item.getAttachmentState() ) {
            Log.e(TAG,"["+mId+"] add() ProjectAlreadyAttachedException index="+index);
            throw new ProjectAlreadyAttachedException();
        }
        //mClipTimeUpdated = true;
        updateTimeLine(false);
        item.setAttachmentState(true, this);

        if( bPrimary ) {
            //Rect rc;
            //item.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_RAND);
            if( mEmptyTheme ){
                if( !item.isFaceDetectProcessed()) {
                    //item.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                }
                item.getClipEffect(true).setEffectNone();
                item.getTransitionEffect(true).setEffectNone();
                item.getTransitionEffect(true).setDuration(0);
            }
            mPrimaryItems.add(index, item);
        }else{
            addAudio(item, index, index + item.getTotalTime());
        }
        return 0;
    }

    private int addClone( int index, nexClip item ) {
        updateTimeLine(false);
        item.setAttachmentState(true, this);

        mPrimaryItems.add(index, item);
        return 0;
    }

    private int addCloneAudio(int index, nexAudioItem item){
        updateTimeLine(false);
        item.getClip().setAttachmentState(true,this);
        mSecondaryItems.add(index,item);
        return 0;
    }

    /** 
     * This method adds an audio clip to a project.
     *
     * <p>Example code :</p>
     * {@code project.addAudio(clip2, 0, clip2.getTotalTime());}
     * @param clip  The type of clip to add: in this case <tt>audio</tt>.
     * @param startTime  The start time of the audio clip to be added (relative to the timeline of the project).
     * @param endTime  The end time of the audio clip to be added.
     * 
     * @return Always 0.
     * 
     * @since version 1.0.0
     */
    public int addAudio(nexClip clip, int startTime, int endTime){
        if( !checkAudioTime(startTime,endTime) ){
            return -1;
        }
        int id;
        clip.setAttachmentState(true, this);
        nexAudioItem ai = new nexAudioItem(clip, startTime, endTime);
        id = ai.getId();
        mSecondaryItems.add(ai);
        Collections.sort(mSecondaryItems, new AudioStartTimeDesCompare());
        mNeedLoadList = true;
        return id;
    }
  
    /**     
     * This method allows to change the start time and the end time of an audio item. 
     *
     * <p>Example code :</p>
     * {@code project.changeAudio(project.getAudioItem(0), mStartTime, project.getTotalTime());}
     * @param audioItem An audio item to change the start time and end time. 
     * @param startTime The new start time of the audio item. 
     * @param endTime  The new end time of the audio item. 
     * 
     * @return -1 after the sorting if the audio item needs to be sorted, otherwise 0. 
     * @see #addAudio(nexClip, int, int)
     * @since version 1.3.52
     */
    public int changeAudio(nexAudioItem audioItem, int startTime, int endTime){
        remove(audioItem.getClip());
        if( !checkAudioTime(startTime,endTime) ){
            mSecondaryItems.add(audioItem);
            audioItem.getClip().setAttachmentState(true,this);
            Collections.sort(mSecondaryItems, new AudioStartTimeDesCompare());
            return -1;
        }
        audioItem.setProjectTime(startTime,endTime);
        mSecondaryItems.add(audioItem);
        audioItem.getClip().setAttachmentState(true,this);
        Collections.sort(mSecondaryItems, new AudioStartTimeDesCompare());
        mNeedLoadList = true;
        return 0;
    }

    private boolean checkAudioTime(int startTime, int endTime){
        ArrayList<nexAudioItem> localArrayList = new ArrayList();
        Iterator localIterator = this.mSecondaryItems.iterator();
        while (localIterator.hasNext())
        {
            nexAudioItem localnexAudioItem = (nexAudioItem)localIterator.next();
            if ((localnexAudioItem.getEndTime() > startTime) && (localnexAudioItem.getStartTime() < endTime)) {
                localArrayList.add(localnexAudioItem);
            }
        }
        int Count = 0;

        for (int i=0; i < localArrayList.size() - 1; i++)
        {
            if ((localArrayList.get(i)).getEndTime() > (localArrayList.get(i + 1)).getStartTime())
            {
                if ((localArrayList.get(i)).getStartTime() < (localArrayList.get(i + 1)).getEndTime()) {
                    Count++;
                }
            }
        }
        return Count < 2;
    }
  
    /**
     * This method gets an audio item, from the audio item list, by the parameter <tt>index</tt>.  
     *
     * <p>Example code :</p>
     * {@code project.changeAudio(project.getAudioItem(0), mStartTime, project.getTotalTime());}
     * @param index index of an audio item. 
     * @return nexAudioItem
     * @since version 1.3.52
     */
    public nexAudioItem getAudioItem(int index){
        return mSecondaryItems.get(index);
    }
  
    /**
     * This method gets an audio item using the ID set with the parameter <tt>id</tt>.
     *
     * <p>Example code :</p>
     * {@code project.findAudioItem(id).setTrim(mStartTrim, mEndTrim);}
     * @param id  ID of an audio item. 
     * @return nexAudioItem
     * @since version 1.3.52
     */
    public nexAudioItem findAudioItem(int id ){
        for( nexAudioItem item: mSecondaryItems ){
            if(item.getId() == id ){
                return item;
            }
        }
        return null;
    }

    /** 
     * This method changes the theme of a project.
     *
     * <p>Example code :</p>
     * {@code project.setThemeId("com.nexstreaming.kinemaster.travel");}
     * @param ThemeId The theme ID of the new theme to be changed, as a <tt>String</tt>. 
     * 
     * @since version 1.0.0
     */
    @Deprecated
    public void setThemeId(String ThemeId){
        mEmptyTheme = true;
        mThemeId = null;
    }
    /**
     * This method sets the background music of a project.
     * 
     * Background music for a project plays throughout the entire project.
     *
     * <p>Example code :</p>
     * {@code project.setBackgroundMusicPath(m_soundfilepath);}
     * @param strMusicPath The location where the background music file is saved. 
     * 
     * @return <tt>TRUE</tt> if the audio file is supported by NexEditor&trade;&nbsp;, otherwise <tt>FALSE</tt>.
     * @since version 1.0.0
     */
    public boolean setBackgroundMusicPath(String strMusicPath){
        if( strMusicPath == null ){
            if( mBackGroundMusic != null ){
                mNeedLoadList = true;
            }
            mBackGroundMusic = null;
            return true;
        }
        nexClip BGMusic = nexClip.getSupportedClip(strMusicPath);
        if (BGMusic != null) {
            if (BGMusic.getClipType() == nexClip.kCLIP_TYPE_AUDIO) {
                mBackGroundMusic = BGMusic;
                mNeedLoadList = true;
                return true;
            }
        }
        return false;
    }

    /**
     * This method gets the background music for a project.
     *
     * <p>Example code :</p>
     * {@code nexClip bgm = mProject.getBackgroundMusic();}
     * @return The <tt>nexClip</tt> instance of the background music.
     * @since version 1.0.0
     */
    public nexClip getBackgroundMusic() {
        return mBackGroundMusic;
    }

    /**
     * This method gets the path of the background music for a project.
     *
     * <p>Example code :</p>
     * {@code path = project.getBackgroundMusicPath();}
     * @return The location where the background music audio file is saved.
     * @since version 1.0.0
     */
    public String getBackgroundMusicPath() {
        if( mBackGroundMusic != null ) {
            return mBackGroundMusic.getPath();
        }
        return null;
    }

    /**
     * This method sets additional settings on the background music. 
     * This method should be called first before calling {@link nexEngine#updateProject()} to successfully apply the settings value.
     *
     * <p>Example code :</p>
     * {@code project.setBackgroundConfig(mStartTime, false, loop.isChecked());}
     * @param startTime  The start time of the background music within the project as an <tt>integer</tt>.
     * @param useThemeMusic  Set to <tt>TRUE</tt> to enable theme music (if a theme was set to the project) when the background music (set from 
     * the user's own music file) is not set or unavailable; otherwise <tt>FALSE</tt>.
     * @param loop  Set to <tt>TRUE</tt> to loop the background music; otherwise <tt>FALSE</tt>.
     *
     * @since version 1.3.4
     */
    public void setBackgroundConfig(int startTime ,boolean useThemeMusic , boolean loop){
        mUseThemeMusic2BGM = useThemeMusic;
        mLoopBGM = loop;
        mStartTimeBGM = startTime;
        mNeedLoadList = true;
    }

    /**
     * This method set the BGM trim
     * @param startTime BGM trim start time
     * @param endTime BGM trim end time
     * @since version 1.5.15
     */
    public void setBackgroundTrim(int startTime ,int endTime){
        mBGMTrimStartTime = startTime;
        mBGMTrimEndTime = endTime;
        mNeedLoadList = true;
    }

    /**
     * This method gets the ID of the theme that is set for a project.
     *
     * <p>Example code :</p>
     * {@code bgm = nexClip.getSupportedClip(mEffectLibrary.getMusicForTheme(mProject.getThemeId()).getAbsolutePath());}
     * @return The theme ID currently set for the project.
     * @since version 1.0.0
     */
    public String getThemeId(){
        return mThemeId;
    }

    /**
     * This method gets a clip in a project. 
     * 
     * This method can be used to get any kind of clip included in a project, including a video, an image, or an audio clip.
     *
     * <p>Example code :</p>
     * {@code project.getClip(0, false).setClipVolume(mBGMVol);}
     * @param location The location of the clip. 
     * @param bPrimary Set to <tt>TRUE</tt> if the track is composed of video and image clips; otherwise set to <tt>FALSE</tt>. 
     * 
     * @return The <tt>nexClip</tt> instance from the project.
     * @since version 1.0.0
     */
    public nexClip getClip(int location, boolean bPrimary) {
        if(bPrimary ) {
            return mPrimaryItems.get(location);
        }
        return mSecondaryItems.get(location).mClip;
    }

    /**
     * @brief This method returns the last clip of a video clip track.     
     * @return The last clip
     * @since 1.7.0
     */
    public nexClip getLastPrimaryClip(){
        return mPrimaryItems.get(mPrimaryItems.size()-1);
    }

    /**
     * Given a clip already in a project, this method returns the clip that immediately follows that reference clip.
     *
     * <p>Example code :</p>
     * {@code nextClip = project.getNextClip(project.getClip(0, false));}
     * @param clip The reference clip from the project.
     * 
     * @return The clip following the reference clip passed with the parameter <tt>clip</tt>.
     * @since version 1.0.0
     */
    public nexClip getNextClip(nexClip clip){
        int index = mPrimaryItems.indexOf(clip);
        if( index == mPrimaryItems.size() || index == -1 ){
            return null;
        }
        return mPrimaryItems.get(index);
    }

    public int indexOf(nexClip clip){
        int index = mPrimaryItems.indexOf(clip);
        return index;
    }

    /**
     * This method moves a clip in a project.
     *
     * <p>Example code :</p>
     * {@code mProject.move(pos - 1, mProject.getClip(pos, true));}
     * @param index  The location where the clip will be moved. 
     * @param item  The <tt> nexClip</tt> to be moved.
     * 
     * @since version 1.0.0
     */
    public void move( int index, nexClip item ) {
        int remove_idx = mPrimaryItems.indexOf(item);
        if( remove_idx < 0 ) {
            //TODO: Throw exception
            return;
        }

        if( index==remove_idx )
            return;

        if( mPrimaryItems.remove(item) ) {
            updateTimeLine(false);
            mPrimaryItems.add(index, item);
        }
    }

    public void swap(nexClip clip1 , nexClip clip2 ){
        int index1 = mPrimaryItems.indexOf(clip1);
        int index2 = mPrimaryItems.indexOf(clip2);
        if( index1 < 0 || index2 < 0 ){
            return;
        }
        Collections.swap(mPrimaryItems,index1,index2);
        updateTimeLine(false);
    }

    /**
     * This method removes a clip from a project. 
     *
     * <p>Example code :</p>
     * {@code project1.remove(clip);}
     * @param item The <tt>nexClip</tt> to be removed.
     * @since version 1.0.0
     */
    public int remove( nexClip item ) {
        int rval = isPrimaryItem(item);
        if( rval < 0  ){
            return rval;
        }

        if(  rval == 1 ) {
            mPrimaryItems.remove(item);
        }else {
            for( int i = 0 ; i < mSecondaryItems.size() ; i++ ){
                if( mSecondaryItems.get(i).mClip == item ){
                    mSecondaryItems.remove(i);
                    break;
                }
            }
        }
        item.setAttachmentState(false,null);
        updateTimeLine(false);
        return 0;
    }

    public List<nexDrawInfo> getTopDrawInfo()
    {
        return mTopEffectInfo;
    }

    protected void clearDrawInfo() {
        if( mTopEffectInfo != null )
            mTopEffectInfo.clear();

        if( mSubEffectInfo != null )
            mSubEffectInfo.clear();

        for(nexClip c : mPrimaryItems ) {
            c.getDrawInfos().clear();
        }
    }

    /**
     * This method clears all of the clips in a track in a project.
     *
     * <p>Example code :</p>
     * {@code mProject.allClear(true);}
     * @param primary  Set to <tt>TRUE</tt> if the track is composed of video clips or image clips only; 
     *                 Otherwise set to <tt>FALSE</tt> if the track is composed of audio clips only. 
     * @since version 1.0.0
     */
    public void allClear(boolean primary ){

        int total = 0;
        int i = 0;
        if( primary ) {
            total = mPrimaryItems.size();
            for ( i = 0; i < total; i++) {
                nexClip item = mPrimaryItems.get(i);
                item.setAttachmentState(false,null);
//                mPrimaryItems.remove(0);
            }
            mPrimaryItems.clear();
        } else {
            total = mSecondaryItems.size();
            for ( i = 0; i < total; i++) {
                mSecondaryItems.get(i).mClip.setAttachmentState(false,null);
            }
            mSecondaryItems.clear();
        }
        mClipTimeUpdated = false;

        mTopEffectInfo.clear();
        mSubEffectInfo.clear();
    }

    /**
     * This method sets the master volume of the background music set to a project.
     *
     * <p>Example code :</p>
     * {@code project.setBGMMasterVolumeScale(mBGMVol / 100f);}
     * @param scale   The master volume in scale from 0 to 1 where 1 is the original volume. 
     * @since version 1.3.4
     */
    public void setBGMMasterVolumeScale(float scale) {
        if( scale < 0 ) {
            mBGMVolumeScale = 0;
        }else if( scale > 1 ){
            mBGMVolumeScale = 1;
        }else{
            mBGMVolumeScale = scale;
        }
        mNeedLoadList = true;
    }

    /**
     * This method gets the master volume of the background music set to a project.
     *
     * <p>Example code :</p>
     * {@code item.mClipVolume = (int) (mProject.getBGMMasterVolumeScale() * 200);}
     * @return  The value of the background music master volume. 
     * @since version 1.3.4
     */
    public float getBGMMasterVolumeScale() {
        return mBGMVolumeScale;
    }
/*
    public void setProjectListener() {

    }
*/
    /**
     * This method gets the total playtime of a project.
     * This method can be used instead of {@link nexEngine#getDuration()} of <tt>nexEngine</tt>.
     *
     * <p>Example code :</p>
     * {@code progressBar_playtime.setMax(project.getTotalTime() / 1000);}
     * @return The total playtime of the project in <tt>msec</tt> (milliseconds).
     * 
     * @see nexEngine#getDuration()
     * @since version 1.0.0
     */
    public int getTotalTime() {

        if( mTemplateApplyMode == 3 ) {
            int end = 0;
            for(int i = 0; i < mPrimaryItems.size(); i++ )
            {
                nexClip clip = mPrimaryItems.get(i);
                end = end < clip.mEndTime ? clip.mEndTime : end;
            }
            return end;
        }

        int size = mPrimaryItems.size();
        if( size == 0 )
            return 0;
        updateProject();
        nexClip clip = mPrimaryItems.get(size-1);
        return clip.mEndTime;
    }

    /**
     * @brief This method is called to change the template apply mode flag.
     * This method is used internally in nexTemplate.
     *
     * <p>Example code :</p>
     * {@code project.setTemplateApplyMode();}
     * @param flag   template apply mode flag.
     * @since version 1.5.50
     */
    public void setTemplateApplyMode(int flag) {
        mTemplateApplyMode = flag;
    }

    /**
     * @brief This method is called to get the template apply mode flag.
     * This method is used internally in SDK.
     *
     * <p>Example code :</p>
     * {@code project.getTemplateApplyMode();}
     * @since version 1.8
     */
    public int getTemplateApplyMode() {
        return mTemplateApplyMode;
    }

    /**
     * @brief This method is called to get templateOverlappedTransitionMode flag.
     * This method is used internally in nexTemplate.
     *
     * <p>Example code :</p>
     * {@code project.setTemplateApplyMode();}
     * @since version 1.7.51
     */
    public boolean getTemplageOverlappedTransitionMode() {
        return mTemplateOverlappedTransition;
    }

    /**
     * @brief This method is called to change the template overlapped transition mode flag.
     * This method is used internally in nexTemplate.
     *
     * <p>Example code :</p>
     * {@code project.setTemplateApplyMode();}
     * @param flag   template apply mode flag.
     * @since version 1.7.51
     */
    public void setTemplageOverlappedTransitionMode(boolean flag) {
        mTemplateOverlappedTransition = flag;
    }

    /**
     * This method is called to get an update of the total playtime of a project when changes are made on a track.
     * This method is used internally by {@link #getTotalTime()} , {@link #getClipPositionTime(int)}, and {@link nexEngine#updateProject()}.
     *
     * <p>Example code :</p>
     * {@code project.updateProject();}
     * @see #getTotalTime()
     * @see #getClipPositionTime(int)
     * @see nexEngine#updateProject()
     * @since version 1.0.0
     */
    public boolean updateProject(){
        int ntotal = getTotalClipCount(true);

        if (mClipTimeUpdated == false) {
            return false;
        }

        mClipTimeUpdated = false;

        if (ntotal == 0) {
            return false;
        }

        if( getTemplateApplyMode() == 3 ) {
            return true;
        }

        int startTime = 0;
        int endTime = 0;
        int totalTime = 0;
        int nexClipduration = 0;
        int effectMargin = 0;
        int titleStartTime = 0;
        int titleEndTime = 0;
        nexClip clipItem;

        for (int i = 0; i < ntotal; i++) {
            clipItem = getClip(i, true);
            clipItem.mStartTime = startTime;

            switch (clipItem.getClipType()) {
                case nexClip.kCLIP_TYPE_IMAGE:
                    nexClipduration = clipItem.getImageClipDuration();
                    break;
                case nexClip.kCLIP_TYPE_VIDEO:
                    if (!clipItem.getTemplateOverlappedTransition()) {
                        nexClipduration = clipItem.getVideoClipEdit().getDuration() + clipItem.getTransitionEffect().getDuration();
                    }
                    else {
                        nexClipduration = clipItem.getVideoClipEdit().getDuration();
                    }
                    break;
            }
            endTime = startTime + nexClipduration;

            if (clipItem.getTransitionEffect(true).getType() == nexEffect.kEFFECT_NONE) {
                effectMargin = 0;
            } else {
                if (clipItem.getTransitionEffect(true).getOverlap() != 0) {
                    effectMargin = clipItem.getTransitionEffect(true).getDuration() * 100 / clipItem.getTransitionEffect(true).getOverlap();
                } else {
                    effectMargin = 0;
                }
            }

            if( getTemplateApplyMode() == 2) {
                clipItem.mTitleEffectStartTime = clipItem.getClipEffect().getShowStartTime();
                clipItem.mTitleEffectEndTime = clipItem.getClipEffect().getShowEndTime();
                titleEndTime = endTime - effectMargin;
                titleStartTime = titleEndTime + clipItem.getTransitionEffect(true).getDuration();

                // Log.d(TAG, String.format("1 TitleStart:%d TitleEnd:%d", clipItem.mTitleEffectStartTime, clipItem.mTitleEffectEndTime));
            } else {
                //Log.d(TAG,"["+mId+"]["+index+"]titleStartTime ="+titleStartTime);
                clipItem.mTitleEffectStartTime = titleStartTime;
                titleEndTime = endTime - effectMargin;
                //Log.d(TAG,"["+mId+"]["+index+"]titleEndTime ="+titleEndTime);
                clipItem.mTitleEffectEndTime = titleEndTime;
                titleStartTime = titleEndTime + clipItem.getTransitionEffect(true).getDuration();
            }

            totalTime = endTime;
            clipItem.mEndTime = endTime;
            startTime = totalTime - effectMargin;
        }

        if( getTemplateApplyMode() == 2 ) {
            int effectStart = 0;
            int clip_idx = 0;

            while (clip_idx < ntotal) {
                clipItem = getClip(clip_idx, true);

                if (clipItem == null) {
                    clip_idx++;
                    continue;
                }

                int lastIdx = getLastClipIndexWithEffectID(clip_idx, clipItem.getTemplateEffectID());

                effectStart = adjustmentEffectTime(clip_idx, lastIdx, effectStart, clipItem.getTemplateEffectID());

                clip_idx = lastIdx + 1;
            }
        }
        return true;
    }


    int getLastClipIndexWithEffectID(int start, int effectID)
    {
        if( (effectID & 0x01000000) == 0x01000000 )
        {
            return start;
        }

        for( int i = start; i < getTotalClipCount(true); i++) {
            nexClip clip = getClip(i, true);

            if( (clip.getTemplateEffectID() & 0xF0FFFFFF) == effectID )
                continue;

            return i-1;
        }
        return getTotalClipCount(true) - 1;
    }

    int adjustmentEffectTime(int start, int end, int startTime, int effectID )
    {
        nexClip clip = getClip(end, true);
        int endTime = clip.getProjectEndTime() - (clip.getTransitionEffect().getDuration() * clip.getTransitionEffect().getOffset() / 100);
        int nextTime = endTime + clip.getTransitionEffect().getDuration();

        int clipStart = getClip(start, true).getProjectStartTime();
        int clipEnd = getClip(end, true).getProjectEndTime();

        for(int i = 0; i < getTotalClipCount(false); i++ )
        {
            nexClip audioClip = getClip(i, false);

            // Log.d(TAG, String.format("updateProject Audio2 Clip adjustment id(%d %d) cs(%d) ce(%d) as(%d)", (effectID & 0xF0FFFFFF), audioClip.getTemplateEffectID(), clipStart, clipEnd, audioClip.getProjectStartTime()));

            if( audioClip.getTemplateEffectID() == (effectID & 0xF0FFFFFF) &&
                    clipStart <= audioClip.getProjectStartTime() &&
                    clipEnd > audioClip.getProjectStartTime() )
            {
                remove(audioClip);
                int audioStart = startTime + ((endTime - startTime) * audioClip.getTemplateAudioPos() / 100);
                addAudio(audioClip, audioStart, audioStart + audioClip.getTotalTime());

                // Log.d(TAG, String.format("updateProject Audio Clip adjustment as(%d) ap(%d)", audioStart, audioClip.getTemplateAudioPos()));

                // Log.d(TAG, String.format("updateProject Audio Clip adjustment cs(%d) as(%d) ap(%d)", startTime, audioStart, audioClip.getTemplateAudioPos()));
            }
        }

        for (int i = start; i <= end; i++) {
            clip = getClip(i, true);

            clip.mTitleEffectStartTime = startTime;
            clip.mTitleEffectEndTime = endTime;

            // clip.getClipEffect().setEffectShowTime(0, endTime - startTime);

            // Log.d(TAG, String.format("updateProject Clip adjustment title effect time(%d) (%d)", startTime, endTime));
            clip.getClipEffect().setEffectShowTime(0, 0);
        }

        return nextTime;
    }

    /**
     * This method sets the fade-in time for the audio at the beginning of a project.  
     * 
     * This method allows the volume of audio to gradually increase at the beginning of the project,
     * making the addition of any types of audio less jarring and smoother.
     * 
     * The default fade-in time for project audio is 200 milliseconds.
     *
     * <p>Example code :</p>
     * {@code project.setProjectAudioFadeInTime(Integer.parseInt(fadeIn.getText().toString()));
        project.setProjectAudioFadeOutTime(Integer.parseInt(fadeOut.getText().toString()));}
     * @param fadeIn The time it takes for the audio to fade-in, in <tt>msec</tt> (milliseconds).
     * 
     * \see setProjectAudioFadeOutTime
     * @since version 1.0.0
     */
    public void setProjectAudioFadeInTime(int fadeIn){
        mAudioFadeInTime = fadeIn;
        mNeedLoadList = true;
    }

    /**
     * This method sets the overall audio volume. Once set, previous volume setting for Clip or BGM is ignored. 
     * @param volume (0~99 , 100 ~200)- volume level to set, 100- Normal Mode (keep current Clip or BGM volume).
     * @since 1.5.21
     */
    public void setProjectVolume(int volume) {
        mProjectVolume = volume;
    }

    /**
     * This method returns current Project Auido Volume value.
     * @return (0~99 , 100 ~200)- volume level to set, 100- Normal Mode (keep current Clip or BGM volume).
     * @since 1.5.21
     */
    public int getProjectVolume() {
        return mProjectVolume;
    }

    /**
     * This method sets whether to keep the mixed volume level or not.
     * @param manual_vol_ctl (0 - Default Mode (lower the mixed audio volume level.) , 1- Manual Mode (keep the mixed audio volume level.)
     * @since 1.5.21
     */
    public void setManualVolumeControl(int manual_vol_ctl) {
        mManualVolCtl = manual_vol_ctl;
    }

    /**
     * This method returns the current mode of mixed audio volume level.
     * @return (0 - Default Mode (lower the mixed audio volume level.) , 1- Manual Mode (keep the mixed audio volume level.)
     * @since 1.5.21
     */
    public int getManualVolumeControl() {
        return mManualVolCtl;
    }

    /**
     * This method gradually decreases the volume of audio at the end of a playing project.
     * 
     * The default fade-out time is 5000 milliseconds.
     *
     * <p>Example code :</p>
     * {@code project.setProjectAudioFadeInTime(Integer.parseInt(fadeIn.getText().toString()));
        project.setProjectAudioFadeOutTime(Integer.parseInt(fadeOut.getText().toString()));}
     * @param fadeOut The time it takes for the audio to fade-out, in <tt>msec</tt> (milliseconds).
     * 
     * \see setProjectAudioFadeInTime
     * @since version 1.0.0
     */
    public void setProjectAudioFadeOutTime(int fadeOut){
        mAudioFadeOutTime = fadeOut;
        mNeedLoadList = true;
    }

    /**
     * This method gets the time to take for project audio to fade-in at the beginning of a project.
     *
     * <p>Example code :</p>
     * {@code int fadeIn = mProject.getProjectAudioFadeInTime();}
     * @return The time it takes for the audio to fade-in, in <tt>msec</tt> (milliseconds).
     * 
     * @see #setProjectAudioFadeInTime(int)
     * @since version 1.0.0
     * 
     */
    public int getProjectAudioFadeInTime(){
        //if(m_isProjectAudioFadeInOn)
        return mAudioFadeInTime;
        //else
        //	return 0;
    }

    /**
     * This method gets the time over which the volume fades out at the end of a playing project.
     *
     * <p>Example code :</p>
     * {@code int fadeOut = mProject.getProjectAudioFadeOutTime();}
     * @return The time it takes for the audio to fade out at the end of a project, in <tt>msec</tt> (milliseconds).
     * 
     * @see #setProjectAudioFadeOutTime(int)
     * @since version 1.0.0
     * 
     */
    public int getProjectAudioFadeOutTime(){
        //if(m_isProjectAudioFadeOutOn)
        return mAudioFadeOutTime;
        //else
        //	return 0;
    }

    /** 
     * This method adds an overlay item to a project.
     *
     * <p>Example code :</p>
     * {@code nexAnimate ani = nexAnimate.getAnimateImages(
            0, 5000, R.drawable.ilove0001
            , R.drawable.ilove0002
            , R.drawable.ilove0003
            , R.drawable.ilove0004
            , R.drawable.ilove0005
            , R.drawable.ilove0006
            , R.drawable.ilove0007
            , R.drawable.ilove0008
            , R.drawable.ilove0009
            , R.drawable.ilove0010
            );
        overlayItem.addAnimate(ani);
        mProject.addOverlay(overlayItem);}
     * @param overlay  The overlay item to add to a project.
     * 
     * @return  <tt>TRUE</tt> if the overlay ID exists and the overlay item is added; otherwise <tt>FALSE</tt>.
     * 
     * @see #removeOverlay(int)
     * @see #getOverlay(int)
     * @since version 1.1.0
     */
    public boolean addOverlay(nexOverlayItem overlay){
        if( overlay.getId() == 0 ){
            return false;
        }
        mOverlayItems.add(overlay);

        Collections.sort(mOverlayItems, new StartTimeDesCompare());
        return true;
    }

    /** 
     * This method gets an overlay item from its corresponding overlay ID.
     *
     * <p>Example code :</p>
     * {@code mEngine.getProject().getOverlay(mOverlayId).setPosition(lastProgress, mEngine.getProject().getOverlay(mOverlayId).getPositionY());}
     * @param overlayId The ID of the overlay item to retrieve.
     * 
     * @return  The corresponding overlay item if the overlay ID passed exists; otherwise <tt>null</tt>.
     * 
     * @see #addOverlay(nexOverlayItem)
     * @since version 1.1.0
     */
    public nexOverlayItem getOverlay(int overlayId){
        for( nexOverlayItem temp : mOverlayItems ){
            if( temp.getId() == overlayId )
                return temp;
        }
        return null;
    }

    /** 
     * This method removes an overlay item from a project.
     *
     * <p>Example code :</p>
     * {@code mExportProject.removeOverlay(mSelectedPIP2.get(2).getId());}
     * @note To remove all overlay items in a project, please call the <tt>clearOverlay()</tt> method instead.
     * 
     * @param overlayId  The overlay ID of the overlay item to remove, as an <tt>integer</tt>.
     * 
     * @return <tt>TRUE</tt> if the overlay item is successfully removed; otherwise <tt>FALSE</tt>.
     * 
     * @see #clearOverlay()
     * @since version 1.1.0
     */
    public boolean removeOverlay(int overlayId){
        boolean rval = false;
        for( nexOverlayItem temp : mOverlayItems ){
            if( temp.getId() == overlayId ){
                rval =  true;
                mOverlayItems.remove(temp);

//                temp.clearCache();
                break;
            }
        }
        return rval;
    }

    /** 
     * This method removes all of the overlay items from a project.
     * 
     * To remove a specific overlay item from a project, please call the <tt>removeOverlay()</tt> method 
     * with the overlay ID of the item instead.
     *
     * <p>Example code :</p>
     * {@code mProject.clearOverlay();}
     * @see #removeOverlay(int)
     * @since version 1.1.0
     */
    public void clearOverlay(){
//        for( nexOverlayItem temp : mOverlayItems ){
//            temp.clearCache();
//        }
        mOverlayItems.clear();
        //NexCache.getInstance().releaseCache();
    }

    /** 
     * This method returns a list of all of the overlay items in a project.
     *
     * <p>Example code :</p>
     * {@code mSelectedPIP2.addAll(mExportProject.getOverlayItems());}
     * @return The overlay items in a project, as a list of <tt>nexOverlayItem</tt> instances.
     * @since version 1.1.0
     */
    public List<nexOverlayItem> getOverlayItems() {
        if( m_externalView_overlayItems==null )
            m_externalView_overlayItems = Collections.unmodifiableList(mOverlayItems);
        return m_externalView_overlayItems;
    }

    private class StartTimeDesCompare implements Comparator<nexOverlayItem> {
        @Override
        public int compare(nexOverlayItem arg0, nexOverlayItem arg1) {
            return arg0.mStartTime > arg1.mStartTime ? -1 : arg0.mStartTime < arg1.mStartTime ? 1:0;
        }
    }

    private static class AudioStartTimeDesCompare implements Comparator<nexAudioItem> {
        @Override
        public int compare(nexAudioItem arg0, nexAudioItem arg1) {
            return arg0.getStartTime() > arg1.getStartTime() ? -1 : arg0.getStartTime() < arg1.getStartTime() ? 1:0;
        }
    }

    private List<nexOverlayItem> mOverlayItems = new ArrayList<nexOverlayItem>();
    private List<nexOverlayItem> m_externalView_overlayItems = null;

    private List<nexClip> mPrimaryItems = new ArrayList<nexClip>();
    private List<nexClip> m_externalView_primaryItems = null;
    private List<nexAudioItem> mSecondaryItems = new ArrayList<nexAudioItem>();
    private List<nexAudioItem> m_externalView_secondaryItems = null;

    private List<nexDrawInfo> mTopEffectInfo = new ArrayList<>();
    private List<nexDrawInfo> mSubEffectInfo = new ArrayList<>();

    private int getClipTimeGuideLine(int clipIndex ){
        int clipDuration = 0;
        int prevIndex = clipIndex - 1;
        int offset;
        int overlap;
        int duration;

        if (clipIndex >= mPrimaryItems.size() || clipIndex < 0 ) {
            return 0;
        }

        if( prevIndex >= 0 ){
            offset = mPrimaryItems.get(prevIndex).getTransitionEffect(true).getOffset();
            overlap = mPrimaryItems.get(prevIndex).getTransitionEffect(true).getOverlap();
            duration = mPrimaryItems.get(prevIndex).getTransitionEffect(true).getDuration();
            clipDuration = (duration - duration*offset/100) + (duration*overlap/100);
        }

        offset = mPrimaryItems.get(clipIndex).getTransitionEffect(true).getOffset();
        duration = mPrimaryItems.get(clipIndex).getTransitionEffect(true).getDuration();
        clipDuration = clipDuration + (duration - duration*offset/100);

        int min = clipDuration / 500;
        return (min*500);
    }

    private int getTransitionTimeGuideLine(int clipIndex , int showClipDurationTime ){
        int clipDuration = 0;
        int prevIndex = clipIndex - 1;
        int nextIndex = clipIndex  + 1;
        int offset;
        int overlap;
        int duration;
        int max = 0;

        if (nextIndex >= mPrimaryItems.size()) {
            return 0;
        }

        if( showClipDurationTime < 0 ){
            showClipDurationTime = 500;
        }

        clipDuration = mPrimaryItems.get(clipIndex).getProjectDuration();
        if( prevIndex >= 0 ){
            offset = mPrimaryItems.get(prevIndex).getTransitionEffect(true).getOffset();
            overlap = mPrimaryItems.get(prevIndex).getTransitionEffect(true).getOverlap();
            duration = mPrimaryItems.get(prevIndex).getTransitionEffect(true).getDuration();
            clipDuration = clipDuration - (duration - duration*offset/100);
            clipDuration = clipDuration - duration*overlap/100;
        }

        clipDuration -= showClipDurationTime;

        if( nextIndex > 0 ){
            duration = mPrimaryItems.get(nextIndex).getProjectDuration()/3;
            if( clipDuration >  duration ){
                clipDuration = duration;
            }
        }

        offset = mPrimaryItems.get(clipIndex).getTransitionEffect(true).getOffset();

        max = clipDuration*2;
        if( offset > 0 ){
            max = clipDuration * 100 / offset;
        }
        int margin = max / 500;

        return (margin *500);
    }

    /** 
     * This method guides for a clip's best time range to be added in the primary track of a project. 
     * 
     * For example, if a transition effect takes 5 seconds whilst a clip(video and image) takes 3 seconds, the clip can't be viewed properly because the transition effect is longer.
     * To avoid this, this method guides for the best duration of a clip to be played properly in the project. 
     *
     * <p>Example code :</p>
     * {@code int[] timeline = mProject.getClipDurationTimeGuideLine(index);}
     * @param clipIndex   The index of a clip from the primary track of the project. 
     * @return The minimum clip duration on index 0 and maximum clip duration on index 1 as <tt>msec</tt> (milliseconds), in <tt>array</tt>. 
     * @since version 1.3.0
     */
    public int[] getClipDurationTimeGuideLine( int clipIndex ){
        int min = getClipTimeGuideLine(clipIndex);
        int max = 10000;
        if( mPrimaryItems.get(clipIndex).getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
            max = mPrimaryItems.get(clipIndex).getProjectDuration();
            if( min > max ){
                return new int[]{0,0};
            }
        }else{
            if( min > max ){
                return new int[]{min,min+1000};
            }
        }

        return new int[]{min,max};
    }

    /** 
     * This method guides for the best duration of a transition effect, when applied to the clips in a project. 
     *
     * <p>Example code :</p>
     * {@code int[] TrTimeLine = mProject.getTransitionDurationTimeGuideLine(clipindex, 2000);}
     * @param clipIndex    The index of the clips on the primary track of the project. 
     * @param showClipDurationTime   The minimum duration of a transition effect in <tt>msec</tt> (milliseconds).
     *
     * @return The minimum effect duration on index 0 and maximum effect duration on index 1 as <tt>msec</tt> (milliseconds), in <tt>array</tt>. 
     * @since version 1.3.0
     */
    public int[] getTransitionDurationTimeGuideLine( int clipIndex , int showClipDurationTime){
        int min = 500;
        int max = getTransitionTimeGuideLine(clipIndex, showClipDurationTime);
        if( min > max ){
            return new int[]{0,0};
        }
        return new int[]{min,max};
    }

    public int[] getTransitionDurationTimeGuideLine( nexClip clip , int showClipDurationTime){
        int i = 0;
        boolean find = false;
        for(nexClip c: mPrimaryItems ){
            if( c == clip ){
                find = true;
                break;
            }
            i++;
        }
        if( find ){
            return getTransitionDurationTimeGuideLine( i ,  showClipDurationTime);
        }
        return new int[]{0,0};
    }

    /** 
     * This method gets the location, on the timeline, of a clip(video and image) on the primary track of a project.
     *
     * <p>Example code :</p>
     * {@code int[] clipPositionTime = mProject.getClipPositionTime(clipindex);}
     * {@link com.nexstreaming.nexeditorsdk.nexClip#getProjectStartTime()} of the class <tt>nexClip</tt> can be used instead of this method.
     *
     * @param clipIndex   The index of a clip on the primary track. 
     *
     * @return The start time on index 0 and end time on index 1 as <tt>msec</tt> (milliseconds), in <tt>array</tt>. 
     * @since version 1.3.0
     */
    public int[] getClipPositionTime( int clipIndex ){
        updateProject();
        int start = mPrimaryItems.get(clipIndex).getProjectStartTime();
        int end = mPrimaryItems.get(clipIndex).getProjectEndTime();
        return new int[]{start,end};
    }

    /**
     * This method duplicates a project.
     * The duplicated project is an independent project from the original project it was copied from.
     *
     * <p>Example code :</p>
     * {@code mExportProject = nexProject.clone(mDisplayProject);}
     * @param src     A project to duplicate.
     *
     * @return  A duplicated new project.
     *
     * @since version 1.3.43
     */
    public static nexProject clone(nexProject src) {
        nexProject object = null;

        if(src.mEmptyTheme) {
            object = new nexProject();
        } else {
            object = new nexProject(src.mThemeId);
        }
        Log.d(TAG,"clone src project["+src.mId+"] to dest project["+object.mId+"]");
        object.mThemeId = src.mThemeId;
        object.mTheme = src.mTheme;
        object.mEmptyTheme = src.mEmptyTheme;
        object.mAudioFadeInTime = src.mAudioFadeInTime;
        object.mAudioFadeOutTime = src.mAudioFadeOutTime;
        object.mOpeningTitle = src.mOpeningTitle;
        object.mEndingTitle = src.mEndingTitle;
        object.mBGMVolumeScale = src.mBGMVolumeScale;
        object.mUseThemeMusic2BGM = src.mUseThemeMusic2BGM;
        object.mLoopBGM = src.mLoopBGM;
        object.mStartTimeBGM = src.mStartTimeBGM;
//        object.mBackGroundMusic = src.mBackGroundMusic;
        object.mClipTimeUpdated = src.mClipTimeUpdated;
        object.mNeedLoadList = src.mNeedLoadList;
        // copy Overlay
        for(nexOverlayItem overlayItem: src.getOverlayItems()) {
            object.addOverlay(nexOverlayItem.clone(overlayItem));
        }
        // copy backgroundmusic
        if(src.mBackGroundMusic!=null) {
            object.mBackGroundMusic = nexClip.clone(src.mBackGroundMusic);
        }
        // copy clips
        for(nexClip clip: src.getPrimaryItems()) {
            object.addClone(object.getTotalClipCount(true), nexClip.clone(clip));
        }

        // copy audio clips
        /*
        List<nexAudioItem> audioItemList = src.getAudioItems();
        for(nexAudioItem audioItem: audioItemList) {
            object.add(nexClip.dup(audioItem.getClip()));
        }

        int idx2=0;
        List<nexAudioItem> tmp_audioItemList = object.getAudioItems();
        for(nexAudioItem audioItem: tmp_audioItemList) {
            audioItem.getClip().setAudioOnOff(audioItemList.get(idx2).getClip().getAudioOnOff());
            audioItem.setTrim(audioItemList.get(idx2).getStartTrimTime(), audioItemList.get(idx2).getEndTrimTime());
            audioItem.getClip().setClipVolume(audioItemList.get(idx2).getClip().getClipVolume());
            audioItem.setProjectTime(audioItemList.get(idx2).getStartTime() , audioItemList.get(idx2).getEndTime() );
            idx2++;
        }
*/
        int i = 0;
        for(nexAudioItem audioClip : src.getAudioItems()){
            object.addCloneAudio(i,nexAudioItem.clone(audioClip));
            i++;
        }
        return object;
    }

    public void clearFaceDetectInfo(){

        int clipcount = mPrimaryItems.size();
        for( int i = 0 ; i < clipcount ; i++ ){

            nexClip clip = mPrimaryItems.get(i);
            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ){

                clip.resetFaceDetectProcessed();
            }
        }
    }
    /**
     * This method gets the index of a clip on the specific location of project's timeline.  
     *
     * <p>Example code :</p>
     * {@code int position = project.getClipPosition(2000);}
     * @param projectCTS    The specific location of the project's timeline in <tt>msec</tt> (milliseconds) as <tt>integer</tt>.
     * @return The index of a clip located on the project's timeline specified by parameter <tt>projectCTS</tt>. -1 if the index can not be found.
     * @see #getProjectTime2ClipTimePosition(int, int)
     * @since version 1.3.43
     */
    public int getClipPosition(int projectCTS){
        int ClipPosition = -1;
        for( int i = 0 ; i < getTotalClipCount(true) ; i++ ){
            if( mPrimaryItems.get(i).getProjectStartTime() <= projectCTS && projectCTS < mPrimaryItems.get(i).getProjectEndTime() ){
                ClipPosition = i;
                break;
            }
        }
        return ClipPosition;
    }

    /**
     * This method gets time of a clip when the clip is on the specific location on the timeline specified by the parameter <tt>position</tt>.
     * For example, when there are three(3) videos with duration of 4 seconds each in a project, 
     * and the input value for <tt>position</tt> is 5, this method will return 1 because 5 second on the timeline will be on the 1 second on the second video. 
     *
     * <p>Example code :</p>
     * {@code int time = project.getProjectTime2ClipTimePosition(position, 2000);}
     * @param position   The index of a clip. Returned value from the method <tt>getClipPosition()</tt>. 
     * @param projectCTS  The specific location of the project's timeline in <tt>msec</tt> (milliseconds) as <tt>integer</tt>.
     * @return position The time of a clip corresponding the specified time by the parameter <tt>projectCTS</tt> on the timeline.
     * @see #getClipPosition(int)
     * @since version 1.3.43
     */
    public int getProjectTime2ClipTimePosition(int position, int projectCTS){
        int start = 0;
        int end = 0;
        int ClipPosition = -1;

        if( position <0 || position >= mPrimaryItems.size() ) {
            return -1;
        }

        start = mPrimaryItems.get(position).getProjectStartTime();
        end = mPrimaryItems.get(position).getProjectEndTime();
        if(  start <= projectCTS && projectCTS < end ){
            int trim = 0;
            int speed = 0;
            ClipPosition = projectCTS - start;
            if( mPrimaryItems.get(position).getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
                trim = mPrimaryItems.get(position).getVideoClipEdit().getStartTrimTime();
                speed = mPrimaryItems.get(position).getVideoClipEdit().getSpeedControl();
                if( speed != 100 ) {
                    if( speed == 3 ) {
                        ClipPosition = Math.round((float)(ClipPosition/32));
                    }else if( speed == 6 ) {
                        ClipPosition = Math.round((float)(ClipPosition/16));
                    }else if( speed == 13 ){
                        ClipPosition = Math.round((float)(ClipPosition/8));
                    }else {
                        ClipPosition = Math.round((float)(ClipPosition*speed/100));
                    }
                }
                ClipPosition += trim;
            }
        }
        return ClipPosition;
    }

    /**
     * Gets the maximum speed control value applicable to the video clip in the project. 
     * @param position The clip position in the project.
     * @return The maximum speed control value that can be applied(13 ~ 400).
     * @since 1.5.42
     */
    public int getClipMaxSpeedControlList(int position){

        if( mPrimaryItems.get(position).getClipType() != nexClip.kCLIP_TYPE_VIDEO ) {
            return 0;
        }

        int duration = mPrimaryItems.get(position).getTotalTime();
        if(  mPrimaryItems.get(position).getVideoClipEdit().mTrimStartDuration != 0 ||  mPrimaryItems.get(position).getVideoClipEdit().mTrimEndDuration != 0){
            duration = (mPrimaryItems.get(position).getTotalTime() -  mPrimaryItems.get(position).getVideoClipEdit().mTrimStartDuration - mPrimaryItems.get(position).getVideoClipEdit().mTrimEndDuration);
        }

        int preOverlapTime = 0;
        int postOverlapTime = 0;

        int preIndex =  position -1;
        int postIndex =  position +1;
        if( preIndex >= 0 ){
            preOverlapTime = mPrimaryItems.get(preIndex).getProjectEndTime() - mPrimaryItems.get(position).getProjectStartTime();
        }

        if( postIndex < mPrimaryItems.size() ){
            postOverlapTime = mPrimaryItems.get(position).getProjectEndTime() - mPrimaryItems.get(postIndex).getProjectStartTime();
        }

        int limitDuration = preOverlapTime + postOverlapTime + 500;
        int tab[] = {13,25,50,75,100,125,150,175,200,400};
        int reval = 0;
        for( int i = 0 ; i < tab.length ; i++ ){
            int newDuration = Math.round((float)(duration*100/tab[i]));
            if( limitDuration <= newDuration ){
                reval = tab[i];
            }else{
                break;
            }
        }
        return reval;
    }

    /**
     * This method shares the project that had been edited in NexEditor&trade;&nbsp;.
     * 
     * @deprecated For internal use only. Please do not use.
     *
     * @warning Use this method only when KineMaster&trade;&nbsp; has been installed, which is compatible with NexEditor&trade;&nbsp;.
     * 
     * <p>Example code :</p>
     * {@code startActivity(mEngin.getProject().makeKineMasterIntent());}
     * @return The intent that can call KineMaster&trade;&nbsp;.
     * @since 1.5.24
     */
    @Deprecated
    public Intent makeKineMasterIntent(){
        updateProject();
        Log.d(TAG,"["+mId+"] make intent start");
        KMIntentBuilder builder = new KMIntentBuilder();
        for( nexClip clipItem : mPrimaryItems ) {
            Rect startRect = new Rect();
            Rect endRect = new Rect();

            clipItem.getCrop().getStartPositionRaw(startRect);
            clipItem.getCrop().getEndPositionRaw(endRect);

            switch (clipItem.getClipType()) {
                case nexClip.kCLIP_TYPE_IMAGE:
                    if( clipItem.isSolid() ){
                        builder.startSolidClip()
                                .solidColor(clipItem.getSolidColor())
                                .duration(clipItem.getImageClipDuration())
                                .brightness(clipItem.getBrightness())
                                .rotation(clipItem.getRotateDegree())
                                .contrast(clipItem.getContrast())
                                .saturation(clipItem.getSaturation())
                                .colorFilterId(clipItem.getColorEffectID())
                                .clipEffectId(clipItem.getClipEffect(true).getId()) //TODO : it is null value by clip effect none.
                                .transitionEffectId(clipItem.getTransitionEffect(true).getId())
                                .transitionDuration(clipItem.getTransitionEffect(true).getDuration())
                                .vignette(clipItem.getVignetteEffect())
                                .endSolidClip();
                    }else {
                        builder.startImageClip()
                                .path(clipItem.getPath())
                                .crop(startRect.left, startRect.top, startRect.right, startRect.bottom, endRect.left, endRect.top, endRect.right, endRect.bottom)
                                .duration(clipItem.getImageClipDuration())
                                .brightness(clipItem.getBrightness())
                                .rotation(clipItem.getRotateDegree())
                                .contrast(clipItem.getContrast())
                                .saturation(clipItem.getSaturation())
                                .colorFilterId(clipItem.getColorEffectID())
                                .clipEffectId(clipItem.getClipEffect(true).getId()) //TODO : it is null value by clip effect none.
                                .transitionEffectId(clipItem.getTransitionEffect(true).getId())
                                .transitionDuration(clipItem.getTransitionEffect(true).getDuration())
                                .vignette(clipItem.getVignetteEffect())
                                .endImageClip();
                    }
                    break;
                case nexClip.kCLIP_TYPE_VIDEO:
                    builder.startVideoClip()
                            .path(clipItem.getPath())
                            .duration(clipItem.getVideoClipEdit().getDuration()) //TODO : apply speedcontrol time???
                            .crop(startRect.left, startRect.top, startRect.right, startRect.bottom, endRect.left, endRect.top, endRect.right, endRect.bottom)
                            .playbackSpeed(clipItem.getVideoClipEdit().getSpeedControl())
                            .startTrim(clipItem.getVideoClipEdit().getStartTrimTime())
                            .rotation(clipItem.getRotateDegree())
                            .mute(!clipItem.getAudioOnOff())
                            .volume(clipItem.getClipVolume())
                            .brightness(clipItem.getBrightness())
                            .contrast(clipItem.getContrast())
                            .saturation(clipItem.getSaturation())
                            .colorFilterId(clipItem.getColorEffectID())
                            .clipEffectId(clipItem.getClipEffect(true).getId())
                            .transitionEffectId(clipItem.getTransitionEffect(true).getId())
                            .transitionDuration(clipItem.getTransitionEffect(true).getDuration())
                            .vignette(clipItem.getVignetteEffect())
                    .endVideoClip();
                    break;
            }

        }

        for( nexAudioItem audioItem : mSecondaryItems ){
            builder.startAudioClip()
                    .path(audioItem.getClip().getPath())
                    .startTime(audioItem.getStartTime())
                    .endTime(audioItem.getEndTime())
                    .background(false)
                    .loop(false)
                    .startTrim(audioItem.getStartTrimTime())
                    .endTrim(audioItem.getEndTrimTime())
                    .mute(!audioItem.getClip().getAudioOnOff())
                    .volume(audioItem.getClip().getClipVolume())
                    .endAudioClip();
        }

        if( mBackGroundMusic != null ){
            builder.startAudioClip()
                    .path(mBackGroundMusic.getPath())
                    .startTime(mStartTimeBGM)
                    .endTime(getTotalTime())
                    .background(true)
                    .loop(mLoopBGM)
                    .startTrim(mBGMTrimStartTime)
                    .endTrim(mBGMTrimEndTime)
                    .volume((int)(getBGMMasterVolumeScale()*200))
                    .endAudioClip();
        }

        //TODO : Overlay
        for( nexOverlayItem overlayItem : mOverlayItems ){
            if( overlayItem.getOverlayImage() instanceof nexOverlayKineMasterText  ){
                nexOverlayKineMasterText text =  (nexOverlayKineMasterText)overlayItem.getOverlayImage();

                int x = overlayItem.getPositionX() *1280 / nexApplicationConfig.getAspectProfile().getWidth() ;
                int y = overlayItem.getPositionY() * 720 / nexApplicationConfig.getAspectProfile().getHeight() ;
/*
                if( nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v1 ){
                    x = 1280 * x /720;
                }else if( nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16 ){
                    x = 1280 * x /720;
                    y = 720 * y /1280;
                }else if( nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_2v1 ){
                    x = 1280 * x /1440;
                    y = 720 * y /720;
                }else if( nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v2 ){
                    x = 1280 * x /720;
                    y = 720 * y /1440;
                }
*/
                builder.startTextLayer()
                        .text(text.getText())
                        .textSize(text.getTextSize())
                        .textColor(text.getTextColor())
                        .glowColor(text.getGlowColor(false))
                        .outlineColor(text.getOutlineColor(false))
                        .shadowColor(text.getShadowColor(false))
                        .fontId(text.getFontId())
                        .startTime(overlayItem.getStartTime())
                        .endTime(overlayItem.getEndTime())
                        .startKeyframe()
                        .time(0)
                        .position(x, y)
                        .alpha(overlayItem.getAlpha())
                        .angle(overlayItem.getRotate())
                        .scale(overlayItem.getScaledX())
                        .endKeyframe()
                        .animationIn(overlayItem.getLayerExpression().getNames(0))
                        .animationInDuration(overlayItem.getLayerExpressionDuration())
                        .animationOut(overlayItem.getLayerExpression().getNames(1))
                        .animationOutDuration(overlayItem.getLayerExpressionDuration())
                        .animationOverall(overlayItem.getLayerExpression().getNames(2))
                        .endTextLayer();
            }
        }

        return builder.build();
    }


    int getId(){
        return mId;
    }

    nexSaveDataFormat.nexProjectOf getSaveData(){
        nexSaveDataFormat.nexProjectOf data = new nexSaveDataFormat.nexProjectOf();
        data.mProjectVolume = mProjectVolume;
        data.mManualVolCtl = mManualVolCtl;
        data.mAudioFadeInTime = mAudioFadeInTime;
        data.mAudioFadeOutTime = mAudioFadeOutTime;

        data.mOpeningTitle = mOpeningTitle;
        data.mEndingTitle = mEndingTitle;

        data.mBGMVolumeScale = mBGMVolumeScale;
        //private int mId;
        //private static int sNextId = 1;

        data.mUseThemeMusic2BGM = mUseThemeMusic2BGM;
        data.mLoopBGM = mLoopBGM;
        data.mStartTimeBGM = mStartTimeBGM;
        if( mBackGroundMusic == null ){
            data.mBackGroundMusic = null;
        }else {
            data.mBackGroundMusic = mBackGroundMusic.getSaveData();
        }
        data.mBGMTrimStartTime = mBGMTrimStartTime;
        data.mBGMTrimEndTime = mBGMTrimEndTime;

        //OverlayAsset mOverlayTemplate;

        //private int mExportMode;

        data.mTemplateApplyMode = mTemplateApplyMode;
        data.mTemplateOverlappedTransition = mTemplateOverlappedTransition;

        //private List<nexOverlayItem> mOverlayItems = new ArrayList<nexOverlayItem>(); --> TODO:
        //private List<nexOverlayItem> m_externalView_overlayItems = null;
        if( mPrimaryItems.size() == 0 ) {
            data.mPrimaryItems = null;
        }else{
            data.mPrimaryItems = new ArrayList<>();
            for( nexClip clip : mPrimaryItems ){
                if( clip.isMotionTrackedVideo() ) continue;
                data.mPrimaryItems.add(clip.getSaveData());
            }
        }
        //private List<nexClip> m_externalView_primaryItems = null;
        if( mSecondaryItems.size() == 0) {
            data.mSecondaryItems = null;
        }else{
            data.mSecondaryItems = new ArrayList<>();
            for( nexAudioItem item : mSecondaryItems){
                data.mSecondaryItems.add(item.getSaveData());
            }
        }
        //private List<nexAudioItem> m_externalView_secondaryItems = null;

        if( mTopEffectInfo.size() == 0 ){
            data.mTopEffectInfo = null;
        }else{
            data.mTopEffectInfo = mTopEffectInfo;
        }

        if( mSubEffectInfo.size() == 0 ){
            data.mSubEffectInfo = null;
        }else{
            data.mSubEffectInfo = mSubEffectInfo;
        }

        return data;
    }

    /**
     * project instance to string.
     * @return save data is format of string.
     * @since 2.0.0
     * @see #createFromSaveString(String)
     */
    public String saveToString(){
        Gson gson = new Gson();
        nexSaveDataFormat save = new nexSaveDataFormat();
        save.project = getSaveData();
        return gson.toJson(save);
    }

    nexProject (nexSaveDataFormat.nexProjectOf data){
        mProjectVolume = data.mProjectVolume;
        mManualVolCtl = data.mManualVolCtl;
        mAudioFadeInTime = data.mAudioFadeInTime;
        mAudioFadeOutTime = data.mAudioFadeOutTime;

        mOpeningTitle = data.mOpeningTitle;
        mEndingTitle = data.mEndingTitle;

        mBGMVolumeScale = data.mBGMVolumeScale;
        mId = sNextId++;

        mUseThemeMusic2BGM = data.mUseThemeMusic2BGM;
        mLoopBGM = data.mLoopBGM;
        mStartTimeBGM = data.mStartTimeBGM;
        if( data.mBackGroundMusic == null ){
            mBackGroundMusic = null;
        }else {
            mBackGroundMusic = new nexClip(null,data.mBackGroundMusic);
        }
        mBGMTrimStartTime = data.mBGMTrimStartTime;
        mBGMTrimEndTime = data.mBGMTrimEndTime;

        //OverlayAsset mOverlayTemplate;

        //private int mExportMode;

        mTemplateApplyMode = data.mTemplateApplyMode;
        mTemplateOverlappedTransition = data.mTemplateOverlappedTransition;

        //private List<nexOverlayItem> mOverlayItems = new ArrayList<nexOverlayItem>(); --> TODO:
        //private List<nexOverlayItem> m_externalView_overlayItems = null;
        if( data.mPrimaryItems != null ) {
            for (nexSaveDataFormat.nexClipOf ofData : data.mPrimaryItems) {
                mPrimaryItems.add(new nexClip(this, ofData));
            }
        }

        //private List<nexClip> m_externalView_primaryItems = null;
        if( data.mSecondaryItems != null ) {
            for (nexSaveDataFormat.nexAudioItemOf ofData : data.mSecondaryItems){
                mSecondaryItems.add(new nexAudioItem(this,ofData));
            }
        }
        //private List<nexAudioItem> m_externalView_secondaryItems = null;

        if( data.mTopEffectInfo != null ){
            mTopEffectInfo = data.mTopEffectInfo;
        }

        if( data.mSubEffectInfo != null ){
            mSubEffectInfo = data.mSubEffectInfo;
        }

        mClipTimeUpdated = true;
    }

    /**
     * load project from {@link #saveToString()}.
     * <p>Example code :</p>
     * <pre>
     *     {@code
     *     //save
     *     String saveData = mEngine.getProject().saveToString();
     *
     *     //load
     *     nexProject mProject = nexProject.createFromSaveString(saveData);
     *
     *     }
     *     </pre>
     * @param save {@link #saveToString()} return value.
     * @return nexProject loaded from save data.
     * @since 2.0.0
     * @see #saveToString()
     */
    public static nexProject createFromSaveString(String save){
        Gson gson = new Gson();
        nexSaveDataFormat saved = gson.fromJson(save,nexSaveDataFormat.class);
        return new nexProject(saved.project);
    }

    private static void logClipOf( int index, nexSaveDataFormat.nexClipOf data ){
        Log.d(TAG,"["+index+"]nexClipOf.m_BGMVolume="+data.m_BGMVolume);
        Log.d(TAG,"["+index+"]nexClipOf.mCollageDrawInfoID="+data.mCollageDrawInfoID);
        Log.d(TAG,"["+index+"]nexClipOf.mPath="+data.mPath);
        Log.d(TAG,"["+index+"]nexClipOf.mTransCodingPath="+data.mTransCodingPath);
        Log.d(TAG,"["+index+"]nexClipOf.m_Brightness="+data.m_Brightness);
        Log.d(TAG,"["+index+"]nexClipOf.m_ClipVolume="+data.m_ClipVolume);
        Log.d(TAG,"["+index+"]nexClipOf.m_Contrast="+data.m_Contrast);
        Log.d(TAG,"["+index+"]nexClipOf.m_Saturation="+data.m_Saturation);
        Log.d(TAG,"["+index+"]nexClipOf.mAudioOnOff="+data.mAudioOnOff);
        Log.d(TAG,"["+index+"]nexClipOf.mAVSyncAudioStartTime="+data.mAVSyncAudioStartTime);
        Log.d(TAG,"["+index+"]nexClipOf.mCustomLUT_A="+data.mCustomLUT_A);
        Log.d(TAG,"["+index+"]nexClipOf.mCustomLUT_B="+data.mCustomLUT_B);
        Log.d(TAG,"["+index+"]nexClipOf.mCustomLUT_Power="+data.mCustomLUT_Power);
        Log.d(TAG,"["+index+"]nexClipOf.mDuration="+data.mDuration);
        Log.d(TAG,"["+index+"]nexClipOf.mEndTime="+data.mEndTime);
        Log.d(TAG,"["+index+"]nexClipOf.mFaceDetected="+data.mFaceDetected);
        Log.d(TAG,"["+index+"]nexClipOf.mFacedetectProcessed="+data.mFacedetectProcessed);
        Log.d(TAG,"["+index+"]nexClipOf.misMustDownSize="+data.misMustDownSize);
        Log.d(TAG,"["+index+"]nexClipOf.mMediaInfoUseCache="+data.mMediaInfoUseCache);
        Log.d(TAG,"["+index+"]nexClipOf.mOverlappedTransition="+data.mOverlappedTransition);
        Log.d(TAG,"["+index+"]nexClipOf.mPropertySlowVideoMode="+data.mPropertySlowVideoMode);
        Log.d(TAG,"["+index+"]nexClipOf.mStartTime="+data.mStartTime);
        Log.d(TAG,"["+index+"]nexClipOf.mTemplateAudioPos="+data.mTemplateAudioPos);
        Log.d(TAG,"["+index+"]nexClipOf.mTemplateEffectID="+data.mTemplateEffectID);
        Log.d(TAG,"["+index+"]nexClipOf.mTitleEffectEndTime="+data.mTitleEffectEndTime);
        Log.d(TAG,"["+index+"]nexClipOf.mTitleEffectStartTime="+data.mTitleEffectStartTime);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mExistAudio="+data.mInfo.mExistAudio);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mVideoTotalTime="+data.mInfo.mVideoTotalTime);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mSeekPointCount="+data.mInfo.mSeekPointCount);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mFramesPerSecond="+data.mInfo.mFramesPerSecond);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mAudioCodecType="+data.mInfo.mAudioCodecType);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mMimeType="+data.mInfo.mMimeType);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mVideoCodecType="+data.mInfo.mVideoCodecType);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mAudioBitrate="+data.mInfo.mAudioBitrate);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mAudioTotalTime="+data.mInfo.mAudioTotalTime);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mClipType="+data.mInfo.mClipType);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mDisplayHeight="+data.mInfo.mDisplayHeight);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mDisplayWidth="+data.mInfo.mDisplayWidth);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mExistVideo="+data.mInfo.mExistVideo);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mH264Level="+data.mInfo.mH264Level);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mH264Profile="+data.mInfo.mH264Profile);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mHeight="+data.mInfo.mHeight);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mRotateDegreeInMeta="+data.mInfo.mRotateDegreeInMeta);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mSuppertedResult="+data.mInfo.mSuppertedResult);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mTotalTime="+data.mInfo.mTotalTime);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mVideoBitrate="+data.mInfo.mVideoBitrate);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mVideoRenderMode="+data.mInfo.mVideoRenderMode);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mVideoHDRType="+data.mInfo.mVideoHDRType);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mVideoTotalTime="+data.mInfo.mVideoTotalTime);
        Log.d(TAG,"["+index+"]nexClipOf.mInfo.mWidth="+data.mInfo.mWidth);
        if( data.mClipEffect == null ) {
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect= null");
        }else{
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mAutoID=" + data.mClipEffect.mAutoID);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mID=" + data.mClipEffect.mID);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mName=" + data.mClipEffect.mName);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mID=" + data.mClipEffect.mDuration);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mEffectOffset=" + data.mClipEffect.mEffectOffset);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mEffectOverlap=" + data.mClipEffect.mEffectOverlap);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mIsResolveOptions=" + data.mClipEffect.mIsResolveOptions);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mMaxDuration=" + data.mClipEffect.mMaxDuration);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mMinDuration=" + data.mClipEffect.mMinDuration);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mOptionsUpdate=" + data.mClipEffect.mOptionsUpdate);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mShowEndTime=" + data.mClipEffect.mShowEndTime);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mShowStartTime=" + data.mClipEffect.mShowStartTime);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.mType=" + data.mClipEffect.mType);
            Log.d(TAG, "[" + index + "]nexClipOf.mClipEffect.itemMethodType=" + data.mClipEffect.itemMethodType);
        }

        if( data.mTransitionEffect == null ) {
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect= null");
        }else{
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mAutoID=" + data.mClipEffect.mAutoID);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mID=" + data.mClipEffect.mID);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mName=" + data.mClipEffect.mName);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mID=" + data.mClipEffect.mDuration);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mEffectOffset=" + data.mClipEffect.mEffectOffset);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mEffectOverlap=" + data.mClipEffect.mEffectOverlap);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mIsResolveOptions=" + data.mClipEffect.mIsResolveOptions);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mMaxDuration=" + data.mClipEffect.mMaxDuration);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mMinDuration=" + data.mClipEffect.mMinDuration);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mOptionsUpdate=" + data.mClipEffect.mOptionsUpdate);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mShowEndTime=" + data.mClipEffect.mShowEndTime);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mShowStartTime=" + data.mClipEffect.mShowStartTime);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.mType=" + data.mClipEffect.mType);
            Log.d(TAG, "[" + index + "]nexClipOf.mTransitionEffect.itemMethodType=" + data.mClipEffect.itemMethodType);
        }

        if( data.mCrop == null ){
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop= null");
        }else{

            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_endPositionBottom=" + data.mCrop.m_endPositionBottom);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_endPositionLeft=" + data.mCrop.m_endPositionLeft);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_endPositionRight=" + data.mCrop.m_endPositionRight);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_endPositionTop=" + data.mCrop.m_endPositionTop);

            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_startPositionBottom=" + data.mCrop.m_startPositionBottom);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_startPositionLeft=" + data.mCrop.m_startPositionLeft);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_startPositionLeft=" + data.mCrop.m_startPositionRight);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_startPositionLeft=" + data.mCrop.m_startPositionTop);

            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_faceBounds_bottom=" + data.mCrop.m_faceBounds_bottom);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_faceBounds_left=" + data.mCrop.m_faceBounds_left);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_faceBounds_right=" + data.mCrop.m_faceBounds_right);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_faceBounds_set=" + data.mCrop.m_faceBounds_set);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_faceBounds_top=" + data.mCrop.m_faceBounds_top);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_facePositionBottom=" + data.mCrop.m_facePositionBottom);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_facePositionLeft=" + data.mCrop.m_facePositionLeft);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_facePositionRight=" + data.mCrop.m_facePositionRight);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_facePositionTop=" + data.mCrop.m_facePositionTop);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_height=" + data.mCrop.m_height);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_height=" + data.mCrop.m_width);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_rotatedEndPositionBottom=" + data.mCrop.m_rotatedEndPositionBottom);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_rotatedEndPositionLeft=" + data.mCrop.m_rotatedEndPositionLeft);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_rotatedEndPositionRight=" + data.mCrop.m_rotatedEndPositionRight);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_rotatedEndPositionTop=" + data.mCrop.m_rotatedEndPositionTop);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_rotation=" + data.mCrop.m_rotation);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_rotatedStartPositionBottom=" + data.mCrop.m_rotatedStartPositionBottom);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_rotatedStartPositionBottom=" + data.mCrop.m_rotatedStartPositionLeft);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_rotatedStartPositionBottom=" + data.mCrop.m_rotatedStartPositionRight);
            Log.d(TAG, "[" + index + "]nexClipOf.mCrop.m_rotatedStartPositionBottom=" + data.mCrop.m_rotatedStartPositionTop);
        }

        if( data.mDrawInfos != null ){
            for( nexDrawInfo info : data.mDrawInfos ){
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getCustomLUTA=" +info.getCustomLUTA());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getCustomLUTB=" +info.getCustomLUTB());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getEffectID=" +info.getEffectID());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getTitle=" +info.getTitle());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getBrightness=" +info.getBrightness());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getClipID=" +info.getClipID());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getEndRect=" +info.getEndRect().toString());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getStartRect=" +info.getStartRect().toString());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getContrast=" +info.getContrast());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getSubEffectID=" +info.getSubEffectID());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getTranslateX=" +info.getUserTranslateX());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getTranslateX=" +info.getUserTranslateY());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getRotateState=" +info.getRotateState());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getUserRotateState=" +info.getUserRotateState());
                Log.d(TAG, "[" + index + "]nexClipOf.mDrawInfos.getEndTime=" +info.getEndTime());
            }
        }
    }


    static int logFromString(String save){
        Gson gson = new Gson();
        nexSaveDataFormat saved = gson.fromJson(save,nexSaveDataFormat.class);

        Log.d(TAG,"nexSaveDataFormatVersion="+saved.nexSaveDataFormatVersion);
        Log.d(TAG,"project.mEndingTitle="+saved.project.mEndingTitle);
        Log.d(TAG,"project.mOpeningTitle="+saved.project.mOpeningTitle);
        Log.d(TAG,"project.mAudioFadeInTime="+saved.project.mAudioFadeInTime);
        Log.d(TAG,"project.mAudioFadeOutTime="+saved.project.mAudioFadeOutTime);
        if( saved.project.mBackGroundMusic == null ) {
            Log.d(TAG, "project.mBackGroundMusic= null");
        }else{
            Log.d(TAG, "project.mBackGroundMusic= set");
        }
        Log.d(TAG,"project.mBGMTrimEndTime="+saved.project.mBGMTrimEndTime);
        Log.d(TAG,"project.mBGMTrimStartTime="+saved.project.mBGMTrimStartTime);
        Log.d(TAG,"project.mBGMVolumeScale="+saved.project.mBGMVolumeScale);
        Log.d(TAG,"project.mLoopBGM="+saved.project.mLoopBGM);
        Log.d(TAG,"project.mManualVolCtl="+saved.project.mManualVolCtl);
        Log.d(TAG,"project.mProjectVolume="+saved.project.mProjectVolume);
        Log.d(TAG,"project.mStartTimeBGM="+saved.project.mStartTimeBGM);
        Log.d(TAG,"project.mTemplateOverlappedTransition="+saved.project.mTemplateOverlappedTransition);
        Log.d(TAG,"project.mUseThemeMusic2BGM="+saved.project.mUseThemeMusic2BGM);

        if( saved.project.mPrimaryItems == null ){
            Log.d(TAG,"project.mPrimaryItems=null");
        }else{
            int index = 1;
            for( nexSaveDataFormat.nexClipOf data : saved.project.mPrimaryItems ){
                logClipOf(index,data);
                index++;
            }
        }

        return 0;
    }
}
