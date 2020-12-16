/******************************************************************************
 * File Name        : nexAudioEnvelop.java
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

import com.nexstreaming.app.common.util.Ints;
import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;

import java.lang.Cloneable;
import java.util.ArrayList;

/**
 * This class allows to control the audio volume for each divided sections of audio item. 
 *
 * <p>Example code:</p>
 *     {@code 
           setEnv.setOnClickListener(new View.OnClickListener() {
               @Override
               public void onClick(View v) {
                   mTime.add(envTime.getText().toString());
                   mVol.add(envVol.getText().toString());
                   mItemAdp.notifyDataSetChanged();
               }
           });
           for(int i=0; i<mTime.size(); i++){
               project.getClip(0,false).getAudioEnvelop().addVolumeEnvelope(Integer.parseInt(mTime.get(i)),Integer.parseInt(mVol.get(i)));
           }
       }
 * 
 */
public final class nexAudioEnvelop implements Cloneable {
//    nexClip mClip;
    private ArrayList<Integer> m_volumeEnvelopeTime;
    private ArrayList<Integer> m_volumeEnvelopeLevel;

    private int [] m_cachedTimeList;
    private int [] m_cachedLevelList;

    private boolean m_modify = true;

    private int m_totalTime;
    private int m_trimStartTime;
    private int m_trimEndTime;
    private static final int maxIndex = 65536;

    private nexAudioEnvelop(){

    }

    protected static nexAudioEnvelop clone(nexAudioEnvelop src) {
        //Log.d("clone", "clone work./nexAudioEnvelop");
        nexAudioEnvelop object = null;
        try {
            object = (nexAudioEnvelop)src.clone();
            object.m_volumeEnvelopeTime = src.m_volumeEnvelopeTime;
            object.m_volumeEnvelopeLevel = src.m_volumeEnvelopeLevel;
            object.m_cachedTimeList = src.m_cachedTimeList;
            object.m_cachedLevelList = src.m_cachedLevelList;
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return object;
    }

    nexAudioEnvelop(nexClip clip){
        m_totalTime = clip.getTotalTime();
        m_trimStartTime = 0;
        m_trimEndTime = clip.getTotalTime();
    }
 
    /**
     * This method updates the trim time of an audio item. 
     * 
     * <p>Example code:</p>
     *     {@code
               mClip.getAudioEnvelop().updateTrimTime(0,mClip.getTotalTime());
           }
     * @param start The start time of the trimmed section on the audio item. 
     * @param end The end time of the trimmed section on the audio item. 
     *
     * @since version 1.3.52
     */
    public void updateTrimTime(int start , int end){
        if( end <=  start){
            throw new InvalidRangeException(start, end);
        }
        if(start < 0){
            throw new InvalidRangeException(0, m_totalTime, start);
        }
        m_trimStartTime = start;
        m_trimEndTime = end;
        m_modify = true;
    }
     
    /**
     * This method adds a volume envelope by setting the time and the volume values. 
     * The added volume envelopes are automatically listed by time. 
     *
     * <p>Example code:</p>
	 *     {@code
	             int index = project.getClip(0,false).getAudioEnvelop().addVolumeEnvelope(Integer.parseInt(mTime.get(i)),Integer.parseInt(mVol.get(i)));
	       }
     * @param time  The time to add the volume envelop. 
     * @param level The audio volume to set to the envelop.
     * 
     * @return Index of the volume envelop list.
     * @see #removeVolumeEnvelop(int)
     * @since version 1.3.52
     */
    public int addVolumeEnvelope( int time, int level){
        if( time < 0){
            throw new InvalidRangeException(0, m_totalTime, time);
        }
        if( level > 200 || level < 0){
            throw new InvalidRangeException(0, 200, level);
        }
        int index = findTime2Index(time, level);
        m_modify = true;
        if( index >= maxIndex )
            return index - maxIndex;
        m_volumeEnvelopeTime.add(index, time);
        m_volumeEnvelopeLevel.add(index, level);
        return index;
    }

    private int findTime2Index(int time, int level){

        if(m_volumeEnvelopeLevel == null) {
            m_volumeEnvelopeLevel = new ArrayList<Integer>();
            m_volumeEnvelopeLevel.add(0,100);
            m_volumeEnvelopeLevel.add(1,100);
        }

        if(m_volumeEnvelopeTime ==null) {
            m_volumeEnvelopeTime = new ArrayList<Integer>();
            m_volumeEnvelopeTime.add(0,0);
            m_volumeEnvelopeTime.add(1,m_totalTime);
            return 1;
        }

        int index =  m_volumeEnvelopeTime.size();
        for( int i = 0; i < m_volumeEnvelopeTime.size() ; i++) {
            int set = m_volumeEnvelopeTime.get(i);
            if( set == time ){
                index = maxIndex+i;
                m_volumeEnvelopeLevel.set(i,level);
                break;
            }else if( set > time ){
                index = i;
                break;
            }
        }
        return index;
    }

    /**
     * This method changes the volume of an envelope on the location set with the parameter <tt>index</tt>.
     * 
     * <p>Example code:</p>
	 *     {@code
	             mClip.getAudioEnvelop().changeVolumeLevelValue(0, 200);
	       }
     * @param index The location of a volume envelop. 
     * @param level The new volume value to set to the envelop.
     * 
     * @see #addVolumeEnvelope( int, int)
     * @see #removeVolumeEnvelop(int)
     * @since version 1.1.0     
     */
    public void changeVolumeLevelValue(int index, int level){
        if(index > m_volumeEnvelopeLevel.size()-1) {
            throw new InvalidRangeException(0, m_volumeEnvelopeLevel.size()-1, index);
        }
        if( level > 200 || level < 0){
            throw new InvalidRangeException(0, 200, level);
        }
        m_volumeEnvelopeLevel.set(index, level);
        m_modify = true;
    }

     /**                        
     * This method removes all the volume envelopes set on a clip. 
     * 
     * <p>Example code:</p>
     *     {@code
                 mClip.getAudioEnvelop().removeVolumeEnvelop();
           }
     *
     * @see #addVolumeEnvelope( int, int)
     * @see #changeVolumeLevelValue(int, int)
     * @see #removeVolumeEnvelop(int)
     * @since version 1.1.0     
     */
    public void removeVolumeEnvelop(){
        if( m_volumeEnvelopeTime != null )
            m_volumeEnvelopeTime.clear();

        if( m_volumeEnvelopeLevel != null )
            m_volumeEnvelopeLevel.clear();
        m_modify = true;
    }    
 
    /**                        
     * This method removes a volume envelope that's passed through <tt>index</tt> from the clip.
     * 
     * <p>Example code:</p>
	 *     {@code
	             mClip.getAudioEnvelop().removeVolumeEnvelop(0);
	       }
     * @param index The location of a volume envelop. 
     *
     * @see #addVolumeEnvelope( int, int)
     * @see #changeVolumeLevelValue(int, int)
     * @since version 1.1.0     
     */
    public void removeVolumeEnvelop(int index){
        if(index > m_volumeEnvelopeLevel.size()-1) {
            throw new InvalidRangeException(0, m_volumeEnvelopeLevel.size()-1, index);
        }
        m_volumeEnvelopeTime.remove(index);
        m_volumeEnvelopeLevel.remove(index);
        m_modify = true;
    }
  
    /**
     * This method gets the timestamp of an envelope that's on the location set with the parameter <tt>index</tt>.
     *
     * <p>Example code:</p>
	 *     {@code
                 int time = mClip.getAudioEnvelop().getVolumeEnvelopeTime(0);
	       }
     * @param index  The location of a volume envelop.  
     *
     * @return The timestamp of the envelop or -1 if there is no envelop list. 
     * @see #getVolumeEnvelopeLevel(int)
     * @since version 1.1.0     
     */
    public int getVolumeEnvelopeTime(int index){
        if(index > m_volumeEnvelopeTime.size()-1) {
            throw new InvalidRangeException(0, m_volumeEnvelopeTime.size()-1, index);
        }
        if(m_volumeEnvelopeTime == null)
            return -1;
        return m_volumeEnvelopeTime.get(index);
    }
    /**
     * This method gets the volume of an envelope that's on the location set with the parameter <tt>index</tt>.
     * 
     * <p>Example code:</p>
	 *     {@code
	             int level = mClip.getAudioEnvelop().getVolumeEnvelopelevel(0);
	       }
     * @param index The location of a volume envelop.   
     *
     * @return The volume level of the envelop or -1 if there is no envelop list.
     * @since version 1.1.0
     * @see #getVolumeEnvelopeTime(int)
     */
    public int getVolumeEnvelopeLevel(int index){
        if(index > m_volumeEnvelopeLevel.size()-1) {
            throw new InvalidRangeException(0, m_volumeEnvelopeLevel.size()-1, index);
        }
        if(m_volumeEnvelopeLevel == null)
            return -1;
        return m_volumeEnvelopeLevel.get(index);
    }
 
    /** 
     * This method gets the duration between the start time of a trimmed section and the time of an envelope's location set with the parameter <tt>index</tt>.
     *
     * <p>Example code:</p>
	 *     {@code
	             int adjtime = mClip.getAudioEnvelop().getVolumeEnvelopeTimeAdj(0);
	       }
     * @param index The location of a volume envelop.
     *
     * @return The duration between the start trim time and the envelope's location, otherwise -1 if there is no envelop list. 
     * @since version 1.1.0
     * @see #getVolumeEnvelopeTime(int)
     */
    public int getVolumeEnvelopeTimeAdj(int index){
        if(index > m_volumeEnvelopeTime.size()-1) {
            throw new InvalidRangeException(0, m_volumeEnvelopeTime.size()-1, index);
        }
        if(m_volumeEnvelopeTime == null)
            return -1;
        return m_volumeEnvelopeTime.get(index) - m_trimStartTime;
    }

    /**
     * This method gets the total number of envelopes set to an audio item. 
     * 
     * <p>Example code:</p>
	 *     {@code
	             int totalcount = mClip.getAudioEnvelop().getVolumeEnvelopeLength();
	       }
     * @return The number of envelops. 
     * @since version 1.1.0
     */
    public int getVolumeEnvelopeLength(){
        return m_volumeEnvelopeTime.size();
    }

    private int getProjectDuration(){
        return m_trimEndTime -m_trimStartTime;
    }
    /**
     * This method gets the time list of the envelopes set to an audio item. 
     * 
     * <p>Example code:</p>
	 *     {@code
	             int[] timelist = mClip.getAudioEnvelop().getVolumeEnvelopeTimeList();
	       }
     * @return The time list of envelops, VolumeEnvelopeTimeList(int[]). 
     * @see #getVolumeEnvelopeLevelList()
     *
     * @since version 1.3.52
     */
    public int[] getVolumeEnvelopeTimeList(){
        return getVolumeEnvelopeList(true);
    }

    /**
     * This method gets the volume level list of the envelopes set to an audio item. 
     * 
     * <p>Example code:</p>
	 *     {@code
	             int volumelists = mClip.getAudioEnvelop().getVolumeEnvelopeLevelList();
	       }
     * @return The volume level list of the envelops, VolumeEnvelopeLevelList(int[])
     * @see #getVolumeEnvelopeTimeList()
     * 
     * @since version 1.3.52
     */
    public int[] getVolumeEnvelopeLevelList(){
        return getVolumeEnvelopeList(false);
    }

    private int[] getVolumeEnvelopeList(boolean time ){
        if( m_modify ) {
            m_modify = false;
            if (m_volumeEnvelopeLevel != null) {
                ArrayList<Integer> volumeEnvelopeTime = new ArrayList<Integer>(m_volumeEnvelopeLevel.size() + 2);
                ArrayList<Integer> volumeEnvelopeLevel = new ArrayList<Integer>(m_volumeEnvelopeLevel.size() + 2);

                int prevTime = 0;
                int prevLevel = 0;

                for (int i = 0; i < m_volumeEnvelopeLevel.size(); i++) {
                    int t = getVolumeEnvelopeTimeAdj(i);
                    int l = getVolumeEnvelopeLevel(i);
                    int duration = getProjectDuration();
                    boolean bEnd = (m_totalTime == m_trimEndTime);
                    if (t <= duration && t > 0) {
//                        if (volumeEnvelopeTime.isEmpty()) {
//                            int level = (int) (((m_trimStartTime) - (prevTime + m_trimStartTime)) / (float) ((t + m_trimStartTime) - (prevTime + m_trimStartTime)) * (float) (l - prevLevel) + (float) prevLevel);
//                            volumeEnvelopeTime.add(0);
//                            volumeEnvelopeLevel.add(level);
//                        }
                        volumeEnvelopeTime.add(t);
                        volumeEnvelopeLevel.add(l);
                    } else if (t <= duration && l > 0 && t == 0) {
                        volumeEnvelopeTime.add(t);
                        volumeEnvelopeLevel.add(l);
                    } else if (t > duration && !bEnd  ) {
//                        if (volumeEnvelopeTime.isEmpty()) {
//                            int level = (int) (((m_trimStartTime) - (prevTime + m_trimStartTime)) / (float) ((t + m_trimStartTime) - (prevTime + m_trimStartTime)) * (float) (l - prevLevel) + (float) prevLevel);
//                            volumeEnvelopeTime.add(0);
//                            volumeEnvelopeLevel.add(level);
//                        }
                        int level = (int) ((duration - prevTime) / (float) (t - prevTime) * (float) (l - prevLevel) + (float) prevLevel);
                        volumeEnvelopeTime.add(duration);
                        volumeEnvelopeLevel.add(level);
                        break;
                    }
                    prevTime = t;
                    prevLevel = l;
                }
                m_cachedTimeList = Ints.toInts(volumeEnvelopeTime);
                m_cachedLevelList = Ints.toInts(volumeEnvelopeLevel);
            }else{
                m_cachedTimeList = null;
                m_cachedLevelList = null;
            }
        }
        if( time ){
            return m_cachedTimeList;
        }
        return m_cachedLevelList;
    }

    nexSaveDataFormat.nexAudioEnvelopOf getSaveData(){
        nexSaveDataFormat.nexAudioEnvelopOf data = new nexSaveDataFormat.nexAudioEnvelopOf();
        data.m_volumeEnvelopeLevel = m_volumeEnvelopeLevel;
        data.m_volumeEnvelopeTime = m_volumeEnvelopeTime;
        data.m_totalTime = m_totalTime;
        data.m_trimStartTime = m_trimStartTime;
        data.m_trimEndTime = m_trimEndTime;

        return data;
    }

    nexAudioEnvelop(nexSaveDataFormat.nexAudioEnvelopOf data){
        m_volumeEnvelopeLevel = data.m_volumeEnvelopeLevel;
        m_volumeEnvelopeTime = data.m_volumeEnvelopeTime;
        m_totalTime = data.m_totalTime;
        m_trimStartTime = data.m_trimStartTime;
        m_trimEndTime = data.m_trimEndTime;
    }

}
