/******************************************************************************
 * File Name        : nexEffect.java
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

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemParameterDef;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemParameterType;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.app.common.nexasset.assetpackage.XMLItemDef;
import com.nexstreaming.app.common.nexasset.assetpackage.XMLItemDefReader;
import com.nexstreaming.app.common.util.ColorUtil;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;


import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class defines a NexEditor&trade;&nbsp; effect that can be applied to clips in a project.
 * @since version 1.0.0
 */
public abstract class nexEffect {
    String mID;
    String mAutoID;
    String mName;
    int mType = kEFFECT_NONE;
    int mDuration;
    //private nexClip mClip;
    nexAssetPackageManager.ItemMethodType itemMethodType = nexAssetPackageManager.ItemMethodType.ItemExtra;

    private nexObserver mObserver;
    boolean mUpdated;
    private String[] mTitles= null;
    private static final int kMaxStringTrackCount = 8;
    boolean mIsResolveOptions = false;

    boolean mOptionsUpdate;
    HashMap<String,String> m_effectOptions;

    /**
     * This member indicates that no effect is set to a clip.
     * 
     * @see #getType()
     * @since version 1.0.0
     * 
     */
    public static final int kEFFECT_NONE = 0;

    /**
     * This member indicates that the clip effect is automatically set matching the theme currently set.
     * @see #getType()
     * @since version 1.0.0
     * 
     */
    public static final int kEFFECT_CLIP_AUTO = 1;

    /**
     * This member indicates that the clip effect is not part of any theme but can be set by the user. 
     * @see #getType()
     * @since version 1.0.0
     * 
     */
    public static final int kEFFECT_CLIP_USER = 2;

    /**
     * This member indicates that the transition effect is automatically set matching the theme currently set.
     * 
     * @see #getType()
     * @since version 1.0.0
     * 
     */
    public static final int kEFFECT_TRANSITION_AUTO = 3;

    /**
     * This member indicates that the transition effect is set by the user.
     * 
     * @see #getType()
     * @since version 1.0.0
     * 
     */
    public static final int kEFFECT_TRANSITION_USER = 4;
//    public static final int kEFFECT_CROP_RAND = 5;
//    public static final int kEFFECT_CROP_FACE = 6;


    public static final int kEFFECT_OVERLAY_FILTER = 5;

    /**
     * This method gets the type of an effect.
     * @return The type of clip effect.  This will be one of: 
     * <ul>
     * <li>{@link #kEFFECT_NONE} = {@value #kEFFECT_NONE}
     * <li>{@link #kEFFECT_CLIP_AUTO} = {@value #kEFFECT_CLIP_AUTO}
     * <li>{@link #kEFFECT_CLIP_USER} = {@value #kEFFECT_CLIP_USER}
     * <li>{@link #kEFFECT_TRANSITION_AUTO} = {@value #kEFFECT_TRANSITION_AUTO}
     * <li>{@link #kEFFECT_TRANSITION_USER} = {@value #kEFFECT_TRANSITION_USER}
     * </ul>
     * @since version 1.0.0
     */
    public int getType() {
        return mType;
    }

    /**
     * 
     * This method gets the ID of an effect.
     *
     * @return <tt>none</tt> if no effect was set, otherwise the ID of the effect as a <tt>String</tt>. 
     * @since version 1.0.0
     */
    public String getId() {
        if( mType == nexEffect.kEFFECT_NONE ){
            return "none";
        }

        if( mType == kEFFECT_CLIP_AUTO ||  mType == kEFFECT_TRANSITION_AUTO ){
            if(mAutoID == null) {
                Log.d("nexEffect", "mType="+mType+" mAutoID=null");
                return "none";
            } else {
            return mAutoID;
        }
        }

        if(mID == null) {
            Log.d("nexEffect", "mType="+mType+" mID=null");
            return "none";
        } else {
        return mID;
    }
    }

    /**
     * This method gets the duration of an effect.
     * @return The duration of the effect as an <tt>integer</tt> in <tt>msec</tt> (milliseconds).
     * @since version 1.0.0
     */
    public int getDuration() {
//        if( mType == nexEffect.kEFFECT_NONE ){
//            return 0;
//        }
        return mDuration;
    }

    /**
     *  This method sets the duration of an effect.
     * @param duration The duration of the effect as an <tt>integer</tt> in <tt>msec</tt> (milliseconds).
     * @since version 1.0.0
     */
    public void setDuration(int duration) {
        if(mDuration != duration){
            mUpdated = true;
            setModified(false);
        }
        mDuration = duration;
    }

    /**
     * This method removes an effect that has been set on a clip.
     * @since version 1.0.0
     */
    public void setEffectNone() {
        if(mType != kEFFECT_NONE ){
            mUpdated = true;
            mOptionsUpdate = false;
            setModified(false);
            resetOptions();
            mIsResolveOptions = false;
        }
        mType = kEFFECT_NONE;
    }

    boolean setEffect(String ID,int Type ){
        mType = Type;
        if( mID != null && ID.compareTo(mID) == 0 ){
            return false;
        }
        if( isValidId(ID) ){
            mUpdated = true;
            mID = ID;
            mOptionsUpdate = false;
            resetOptions();
            mIsResolveOptions = false;
            return true;
        }
        return false;
    }

    void setModified(boolean onlyLoadList){
        if( mObserver != null ){
            //mObserver.mClipTimeUpdated = true;
            mObserver.updateTimeLine(onlyLoadList);
        }
    }
/*
    protected void setClip(nexClip clip){
        mClip = clip;

    }
*/

    void setObserver(nexObserver project){
        mObserver = project;
    }

    boolean isValidId( String Id ) {
            return true; //todo :
    }

    /**
     * This method sets the input text for effects with text.
     * Depending on the effects, the number of text tracks may vary.
     *  
     * @param TrackNum The index of text tracks(maximum of 4 tracks) from 0 ~ 3, within a field.
     * @param str The input text.
     * 
     * @since version 1.0.0
     * @see #getTitle(int)
     */
    public void setTitle(int TrackNum, String str) {
        if( mTitles == null ){
            mTitles = new String[kMaxStringTrackCount];
        }

        if( TrackNum < kMaxStringTrackCount ){
            mTitles[TrackNum] = str;
        }
    }

    /**
     * This method gets the input text.
     * @param TrackNum The index of text tracks(maximum of 4 tracks) from 0 ~ 3, within a field.
     * @return The text from the current track.
     * @since version 1.0.0
     * @see #setTitle(int, String)
     */
    public String getTitle(int TrackNum) {
        if( mTitles == null ){
            return null;
        }
        if( TrackNum < kMaxStringTrackCount ){
            return mTitles[TrackNum];
        }
        return null;
    }

    String[] getTitles(){
        return mTitles;
    }

    private boolean isAllEmptyTitle(){
        if( mTitles == null ){
            return true;
        }

        for( int i= 0 ; i < kMaxStringTrackCount ; i++ ){
            if( mTitles[i] != null ){
                return false;
            }
        }
        return true;
    }

    private static void encodeEffectOptions( StringBuilder b, nexEffectOptions options ) {

            boolean first = true;

            //ItemInfo info = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItemById(options.getEffectID());
            //if( info != null ) {
                XMLItemDef itemDef = null;
                try {
                    itemDef = XMLItemDefReader.getItemDef(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), options.getEffectID());
                } catch (XmlPullParserException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (itemDef != null) {
                    List<ItemParameterDef> itemDefs = itemDef.getParameterDefinitions();
                    int index = 0;
                    try {
                        for (ItemParameterDef def : itemDefs) {
                            if (first) {
                                first = false;
                            } else {
                                b.append('&');
                            }

                            if (def.getType() == ItemParameterType.TEXT) {
                                List<nexEffectOptions.TextOpt> text = options.getTextOptions();
                                for (nexEffectOptions.TextOpt opt : text) {
                                    if (def.getId().compareTo(opt.getId()) == 0) {
                                        if (opt.getText() != null) {
                                            b.append(URLEncoder.encode(opt.getId(), "UTF-8"));
                                            b.append("=");
                                            b.append(URLEncoder.encode(opt.getText(), "UTF-8"));
                                        }
                                    }
                                }
                            }else if( def.getType() == ItemParameterType.RGBA || def.getType() == ItemParameterType.RGB ){
                                if (options != null) {
                                    List<nexEffectOptions.ColorOpt> color = options.getColorOptions();
                                    for (nexEffectOptions.ColorOpt opt : color) {
                                        if (def.getId().compareTo(opt.getId()) == 0) {
                                            b.append(URLEncoder.encode(opt.getId(), "UTF-8"));
                                            b.append("=");
                                            b.append(URLEncoder.encode(ColorUtil.colorString(opt.getARGBformat()), "UTF-8"));
                                        }
                                    }
                                }
                            }else if( def.getType() == ItemParameterType.CHOICE){
                                if (options != null) {
                                    List<nexEffectOptions.SelectOpt> select = options.getSelectOptions();
                                    for (nexEffectOptions.SelectOpt opt : select) {
                                        if (def.getId().compareTo(opt.getId()) == 0) {
                                            //int val = opt.getSelectIndex();
                                            //List<UserField.FieldOption> list = f.getOptions();
                                            b.append(URLEncoder.encode(opt.getId(), "UTF-8"));
                                            b.append("=");
                                            b.append(URLEncoder.encode(opt.getSelectValue(), "UTF-8"));
                                        }
                                    }
                                }
                            }else if(def.getType() == ItemParameterType.RANGE){
                                if (options != null) {
                                    List<nexEffectOptions.RangeOpt> ranges = options.getRangeOptions();
                                    for(nexEffectOptions.RangeOpt opt : ranges){
                                        if (def.getId().compareTo(opt.getId()) == 0) {
                                            b.append(URLEncoder.encode(opt.getId(), "UTF-8"));
                                            b.append("=");
                                            b.append(URLEncoder.encode(""+opt.getValue(), "UTF-8"));
                                        }
                                    }
                                }
                            }else if( def.getType() == ItemParameterType.SWITCH ){
                                if (options != null) {
                                    List<nexEffectOptions.SwitchOpt> switchs = options.getSwitchOptions();
                                    for(nexEffectOptions.SwitchOpt opt : switchs){
                                        if (def.getId().compareTo(opt.getId()) == 0) {
                                            b.append(URLEncoder.encode(opt.getId(), "UTF-8"));
                                            b.append("=");
                                            if( opt.getValue() ) {
                                                b.append(URLEncoder.encode(def.getOnValue(), "UTF-8"));
                                            }else{
                                                b.append(URLEncoder.encode(def.getOffValue(), "UTF-8"));
                                            }
                                        }
                                    }
                                }
                            }else if( def.getType() == ItemParameterType.TYPEFACE ){
                                if (options != null) {
                                    List<nexEffectOptions.TypefaceOpt> typefaces = options.getTypefaceOptions();
                                    for(nexEffectOptions.TypefaceOpt opt : typefaces){
                                        if (def.getId().compareTo(opt.getId()) == 0) {
                                            b.append(URLEncoder.encode(opt.getId(), "UTF-8"));
                                            b.append("=");
                                            b.append(URLEncoder.encode(opt.getTypeface(), "UTF-8"));
                                        }
                                    }
                                }
                            }
                        }
                    }catch (UnsupportedEncodingException e) {
                        throw new IllegalStateException(e);
                    }
                }
            }

    /** 
     * This method gets the title effect options as <tt>Strings</tt>, from the serialized effect option values. 
     *
     * @param options The value of the serialized effect options. 
     * @return  The value of the effect options as <tt>Strings</tt>. 
     * <p>Example code :</p>
     *     {@code  m_previewView.setEffectOptions(nexEffect.getTitleOptions(mOptions));
        m_previewView.setEffect(mSelectedEffectId );
        m_previewView.setEffectTime(3000);
        }
     * @see com.nexstreaming.nexeditorsdk.nexEffectPreviewView#setEffectOptions(String)
     * @see com.nexstreaming.nexeditorsdk.nexEffectLibrary#getEffectOptions(android.content.Context, String)
     * @since version 1.1.0
     */
    public static String getTitleOptions( nexEffectOptions options ) {
        StringBuilder b = new StringBuilder();
        if (options.mEffectType == nexEffect.kEFFECT_OVERLAY_FILTER) {
            encodeEffectOptions(b, options);
        }else {

            b.append(0);
            b.append(',');
            b.append(10000);
            b.append('?');
            if (options.mEffectType == nexEffect.kEFFECT_TRANSITION_USER || options.mEffectType == nexEffect.kEFFECT_TRANSITION_AUTO) {
                encodeEffectOptions(b, options);
            }
            b.append('?');
            if (options.mEffectType == nexEffect.kEFFECT_CLIP_USER || options.mEffectType == nexEffect.kEFFECT_CLIP_AUTO) {
                encodeEffectOptions(b, options);
            }
        }
        return b.toString();
    }

    private void resetOptions(){
        if(m_effectOptions != null) {
            m_effectOptions.clear();
        }
    }
    /** 
     * This method updates the effect options in a project. 
     * 
     * @param options The effect option to update to the project. 
     * @param up2Effect  <tt>TRUE</tt> to get updated effect option, or set to <tt>FALSE</tt> to get the default effect option.
     *
     * @return <tt>TRUE</tt> if the ID of the current effect and the effect that the option is updated from are the same,  otherwise <tt>FALSE</tt>.
     * <p>Example code :</p>
     *     {@code
     *     protected void onActivityResult(int requestCode, int resultCode, Intent data) {
                super.onActivityResult(requestCode, resultCode, data);
                if(requestCode == 100 && resultCode == Activity.RESULT_OK) {
                    nexEffectOptions opt = (nexEffectOptions)data.getSerializableExtra("effectopt");
                    String effect = data.getStringExtra("effect");
                    mEngine.getProject().getClip(0,true).getClipEffect().setEffect(effect);
                    mEngine.getProject().getClip(0,true).getClipEffect().updateEffectOptions(opt, true);
                    mEngine.updateProject();
                }
            }
    }
     * @see com.nexstreaming.nexeditorsdk.nexEffectLibrary#getEffectOptions(android.content.Context, String)
     * @since version 1.0.1
     */
    public boolean updateEffectOptions(nexEffectOptions options, boolean up2Effect){
        if(m_effectOptions == null) {
            m_effectOptions = new HashMap<String, String>();
        }
        if( options.mEffectID.compareTo(mID) == 0 ){

            mOptionsUpdate = true;

            if( up2Effect )
                m_effectOptions.clear();

            List<nexEffectOptions.TextOpt> text = options.getTextOptions();
            if (text != null) {
                int index = 0;
                for (nexEffectOptions.TextOpt opt : text) {
                    if( up2Effect ) {
                        if (opt.getText() != null) {
                            setTitle(index, opt.getText());
                            m_effectOptions.put(opt.getId(), getTitle(index));
                        }
                    }else{
                        if( getTitle(index) != null ){
                            opt.setText(getTitle(index));
                        }
                    }
                    index++;
                }
            }

            List<nexEffectOptions.ColorOpt> color = options.getColorOptions();
            if (color != null) {
                for (nexEffectOptions.ColorOpt opt : color) {
                    if( up2Effect ) {
                        if (opt.default_argb_color != opt.argb_color) {
//                            mEffectColorOption.put(opt.getId(), opt.getRGBAformat());
                            m_effectOptions.put(opt.getId(),ColorUtil.colorString(opt.getARGBformat()));
                        }
                    }else{
                        String val =(String)m_effectOptions.get((String) opt.getId());
                        if( val != null ){
                            //opt.rgba_color = Integer.parseInt(val.substring(1),16);
                            opt.argb_color = ColorUtil.parseColor(val);
                        }
                                //opt.rgba_color = val;
                    }
                }
            }

            List<nexEffectOptions.SelectOpt> select = options.getSelectOptions();
            if (select != null) {
                for (nexEffectOptions.SelectOpt opt : select) {
                    if( up2Effect ) {
                        if (opt.default_select_index != opt.select_index) {
//                                mEffectSelectOption.put(opt.getId(), opt.getSelectValue());
                            m_effectOptions.put(opt.getId(), opt.getSelectValue());
                        }
                    }else{
                        String val = (String)m_effectOptions.get((String) opt.getId());
                        if( val != null ) {
                            opt.setValue(val);
                        }
                    }
                }
            }

            List<nexEffectOptions.RangeOpt> ranges = options.getRangeOptions();
            if (ranges != null) {
                for (nexEffectOptions.RangeOpt opt : ranges) {
                    if( up2Effect ) {
                        if (opt.default_value != opt.mValue) {
//                                mEffectSelectOption.put(opt.getId(), opt.getSelectValue());
                            m_effectOptions.put(opt.getId(), ""+opt.getValue());
                        }
                    }else{
                        int val = Integer.parseInt(m_effectOptions.get(opt.getId()));
                        opt.setValue(val);
                    }
                }
            }

            List<nexEffectOptions.SwitchOpt> switchOpts = options.getSwitchOptions();
            if (switchOpts != null) {
                for (nexEffectOptions.SwitchOpt opt : switchOpts) {
                    if( up2Effect ) {
                        if (opt.default_on != opt.on) {
//                                mEffectSelectOption.put(opt.getId(), opt.getSelectValue());
                            if( opt.on ) {
                                m_effectOptions.put(opt.getId(), "on");
                            }else{
                                m_effectOptions.put(opt.getId(), "off");
                            }
                        }
                    }else{
                        if( m_effectOptions.get(opt.getId()).compareTo("on") == 0 ) {
                            opt.setValue(true);
                        }else{
                            opt.setValue(false);
                        }
                    }
                }
            }

            List<nexEffectOptions.TypefaceOpt> typeface = options.getTypefaceOptions();
            if (typeface != null) {
                int index = 0;
                for (nexEffectOptions.TypefaceOpt opt : typeface) {
                    if( up2Effect ) {
                        if( opt.mTypefaceName != null ) {
                            if (opt.default_typeface.compareTo(opt.mTypefaceName) != 0 ) {
                                m_effectOptions.put(opt.getId(), opt.getTypeface());
                            }
                        }
                    }else{
                        String val = (String)m_effectOptions.get((String) opt.getId());
                        if( val != null ) {
                            opt.setTypeface(val);
                        }
                    }
                    index++;
                }
            }

            mIsResolveOptions = true;
            return true;

        }
        return false;
    }

    Map<String,String> getEffectOptions( String Title ) {
        if(m_effectOptions == null) {
            m_effectOptions = new HashMap<String, String>();
        }
        if( mType == kEFFECT_CLIP_AUTO || mType == kEFFECT_TRANSITION_AUTO ){
            m_effectOptions.clear();
        }

        if( (Title == null) && isAllEmptyTitle() ){
            return m_effectOptions;
        }
//        Log.d("Effect","getEffectOptions set Title");
        //ItemInfo info = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItemById(getId());
        //if (info != null) {
            XMLItemDef itemDef = null;
            try {
                itemDef = XMLItemDefReader.getItemDef(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), getId());
            } catch (XmlPullParserException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
            if (itemDef != null) {
                List<ItemParameterDef> itemDefs = itemDef.getParameterDefinitions();
                int index = 0;
                for (ItemParameterDef def : itemDefs) {
                    if (def.getType() == ItemParameterType.TEXT) {
                        if (Title != null) {
                            m_effectOptions.put(def.getId(), Title);
                        } else {
                            if (getTitle(index) != null)
                                m_effectOptions.put(def.getId(), getTitle(index));
                            index++;
                        }
                    }
                }
            }
        //}

        return m_effectOptions;
    }

    int getTitleCount(){
        int count =0;
        if( mTitles == null )
            return 0;
        for( int i = 0 ; i < mTitles.length ;i++ ){
            if( mTitles[i] != null && !mTitles[i].isEmpty() ){
                count++;
            }
        }
        return count;
    }

    /**
     *
     * @return
     * @since 1.7.57
     */
    public nexAssetPackageManager.ItemMethodType getMethodType(){
        if( itemMethodType == nexAssetPackageManager.ItemMethodType.ItemExtra ) {
            if (mID.compareToIgnoreCase("none") == 0) {
                itemMethodType = nexAssetPackageManager.ItemMethodType.ItemKedl;
            }else {
                ItemInfo item = AssetPackageManager.getInstance().getInstalledItemById(mID);
                if (item != null) {
                    itemMethodType = nexAssetPackageManager.getMethodType(item.getType());
                }
            }
        }
        return itemMethodType;
    }
}
