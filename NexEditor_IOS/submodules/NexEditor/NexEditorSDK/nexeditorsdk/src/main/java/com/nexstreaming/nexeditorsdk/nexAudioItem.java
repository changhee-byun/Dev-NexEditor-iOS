/******************************************************************************
 * File Name        : nexAudioItem.java
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

import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;

/**
 * This class defines and manages audio items that are applied to a project with the NexEditor&trade;&nbsp;SDK.
 * 
 * The volume, start time, and end time of an audio item can be set and adjusted, and an audio item 
 * can be turned on and off.
 * 
 * <p>For Example, an audio item can be added to a NexEditor&trade;&nbsp; project with the following sample code:</p>
 * <tt>
   mSecondaryItems.add(new nexAudioItem(clip,startTime,endTime));
   </tt>
 * <p>Example code :</p>
 *  {@code
        bt.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    project.changeAudio(project.getAudioItem(0), mStartTime, project.getTotalTime());
                    project.getAudioItem(0).setTrim(mTrimTime, project.getTotalTime());
                    project.getAudioItem(0).getClip().setClipVolume(200);
                }
            }
    }
 *
 * @since version 1.1.0
 */
public final class nexAudioItem implements Cloneable{
    private int mId;
    protected nexClip mClip;
    protected int mTrimStartDuration ;
    protected int mTrimEndDuration;
    private static int sLastId = 1;
    private int mSpeedControl = 100;

    protected static nexAudioItem clone(nexAudioItem src) {
        nexAudioItem object = null;
        try {
            object = (nexAudioItem)src.clone();
            object.mClip = nexClip.clone(src.mClip);

        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return object;
    }

    protected nexAudioItem(nexClip clip, int startTime, int endTime){
        if( endTime <=  startTime ){
            throw new InvalidRangeException(startTime, endTime);
        }

        mClip = clip;
        mClip.mStartTime = startTime;
        mClip.mEndTime = endTime;
        mId = sLastId;
        sLastId++;
    }
  
    /**
     * This method gets an audio item as an audio clip into the project. 
     *
     * <p>Example code :</p>
     *  {@code project.getAudioItem(0).getClip().setClipVolume(200);}
     * @return An audio clip. 
     * @since version 1.3.37
     * @see #getId()
     */
    public nexClip getClip() {
        return mClip;
    }
  
    /**
     * This method gets the ID of an audio item.  
     * 
     * <p>Example code :</p>
     * {@code for(int j = 0; j<m_listfilepath.size(); j++) {
            if(m_listfilepath.get(j).getID() == id) {
                m_listfilepath.get(j).setTimeline(2);
            }
        }}
     * @return The ID of an audio item.
     * @see #getClip()
     * @since version 1.5.0
     */
    public int getId(){
        return mId;
    }

    protected void setProjectTime(int startTime ,int endTime){
        if( endTime <=  startTime){
            throw new InvalidRangeException(startTime, endTime);
        }
        if(startTime < 0){
            throw new InvalidRangeException(0, mClip.mEndTime, startTime);
        }
        mClip.mStartTime = startTime;
        mClip.mEndTime =  endTime;
        mClip.setProjectUpdateSignal(true);
    }

    /**
     * This method gets the start time of an audio item in a project.
     * 
     * <p>Example code :</p>
     * {@code    TextView textView2 = new TextView(AudioTrackActivity.this);
           weight = project.getAudioItem(i).getStartTime()/1000;
           textView2.setLayoutParams(new LinearLayout.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT, weight));}
     * @return The start time of the audio item, in <tt>msec</tt> (milliseconds).
     * 
     * @see #getEndTime()
     * @since version 1.1.0
     */
    public int getStartTime(){
        return mClip.mStartTime;
    }

    /**
     * This method gets the end time of an audio item in a project.
     * 
     * <p>Example code :</p>
     *  {@code for(int i = 0; i<mProject.getTotalClipCount(false); i++){
             if(duration < mProject.getAudioItem(i).getEndTime()){
                 duration += mProject.getAudioItem(i).getEndTime();
             }
         }
        }
     * @return The end time of the audio item, in <tt>msec</tt> (milliseconds).
     * 
     * @see #getStartTime()
     * @since version 1.1.0
     */
    public int getEndTime(){
        return mClip.mEndTime;
    }

    /**
     * This method trims an existing audio item in a project, setting new start and end times for the item.
     * 
     * <p>Example code :</p>
     *  {@code
            project.getAudioItem(0).setTrim(mTrimTime, project.getTotalTime());
        }
     * @param startTime The start time of the trimmed audio item, in <tt>msec</tt> (milliseconds).
     * @param endTime The end time of the trimmed audio item, in <tt>msec</tt> (milliseconds).
     * 
     * @see #getStartTrimTime()
     * @see #getEndTrimTime()
     * @since version 1.1.0
     */
    public void setTrim(int startTime, int endTime){
        if( endTime <=  startTime ){
            throw new InvalidRangeException(startTime, endTime);
        }
        if(startTime > mClip.getTotalTime()){
            throw new InvalidRangeException(0, mClip.getTotalTime(), startTime);
        }
        if(endTime > mClip.getTotalTime()){
            throw new InvalidRangeException(0, mClip.getTotalTime(), endTime);
        }
        mTrimStartDuration = startTime;
        mTrimEndDuration = mClip.getTotalTime() - endTime;
        mClip.getAudioEnvelop().updateTrimTime(startTime, endTime);

        // for KMSA-328
        /*
        if(mClip.mEndTime > endTime) {
            mClip.mEndTime = mClip.mStartTime + endTime - startTime;
        }
        */
        mClip.setProjectUpdateSignal(true);
    }
  
    /**
     * This method removes the trim time set to an audio item. 
     *
     * <p>Example code :</p>
     *  {@code
            project.getAudioItem(0).removeTrim();
        }
     * @see #setTrim(int, int)
     * @since version 1.3.52
     */
    public void removeTrim(){
        mTrimStartDuration = 0;
        mTrimEndDuration = 0;
        mClip.getAudioEnvelop().updateTrimTime(0,mClip.getTotalTime());
        mClip.mEndTime = mClip.mStartTime + mClip.getTotalTime();
        mClip.setProjectUpdateSignal(true);
    }

    /**
     * This method gets the new start time of an audio item in a project after it has been trimmed.
     * 
     * <p>Example code :</p>
     *  {@code
            int starttrimTime = project.getAudioItem(0).getStartTrimTime();
        }
     * @return  The start time of the trimmed audio item, in <tt>msec</tt> (milliseconds).
     * 
     * @see #setTrim(int, int)
     * @see #getEndTrimTime()
     * @since version 1.1.0
     */
    public int getStartTrimTime(){
        return mTrimStartDuration;
    }

    /**
     * This method gets the new end time of an audio item in a project after it has been trimmed.
     * 
     * <p>Example code :</p>
     *  {@code
            int endtrimTime = project.getAudioItem(0).getEndTrimTime();
        }
     * @return  The end time of the trimmed audio item, in <tt>msec</tt> (milliseconds).
     * 
     * @see #setTrim(int, int)
     * @see #getStartTrimTime()
     * @since version 1.1.0
     */
    public int getEndTrimTime(){
        return mClip.getTotalTime() - mTrimEndDuration;
    }

    public void setSpeedControl(int speedFactor){
        mSpeedControl = speedFactor;
    }

    public int getSpeedControl(){
        return mSpeedControl;
    }

    nexSaveDataFormat.nexAudioItemOf getSaveData(){
        nexSaveDataFormat.nexAudioItemOf data = new nexSaveDataFormat.nexAudioItemOf();
        data.mId = mId;
        data.mClip = mClip.getSaveData();
        data.mTrimStartDuration = mTrimStartDuration;
        data.mTrimEndDuration = mTrimEndDuration;
        data.mSpeedControl = mSpeedControl;
        return data;
    }

    nexAudioItem( nexObserver project, nexSaveDataFormat.nexAudioItemOf data){
        mId = data.mId;
        mClip = new nexClip(project,data.mClip);
        mTrimStartDuration = data.mTrimStartDuration;
        mTrimEndDuration = data.mTrimEndDuration;
        mSpeedControl = data.mSpeedControl;
    }
}
