/******************************************************************************
 * File Name        : nexEffectOptions.java
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

import com.nexstreaming.app.common.util.ColorUtil;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * This class defines and sets the options available for an effect on a clip.
 *
 * Each effect in the NexEditor&trade;&nbsp;SDK has different characteristics and therefore different possible 
 * settings options with default values, but the default settings can be adjusted in detail using this class to customize the effects.
 * 
 * Instances of this class cannot be created as stand-alone instances but must be used with
 * {@link nexEffectLibrary#getEffectOptions(android.content.Context, String)}.
 * 
 * 
 * <p>For Example:</p>
 *     {@code     nexEffectOptions opt = (nexEffectOptions)data.getSerializableExtra("effectopt");
            String effect = data.getStringExtra("effect");
            mEngine.getProject().getClip(0,true).getClipEffect().setEffect(effect);
            mEngine.getProject().getClip(0,true).getClipEffect().updateEffectOptions(opt, true);
            mEngine.updateProject();
       }
 * 
 * @since version 1.0.1
 */
public final class nexEffectOptions implements Serializable {
    private static final long serialVersionUID = 1L;
    public static final String TAG_TYPEFACE_FONTID = "fontid:";
    String mEffectID;
    int mEffectType;
    private boolean updated;
    public static final int kType_Unknown = 0;
    public static final int kType_Text = 1;
    public static final int kType_Color = 2;
    public static final int kType_Select = 3;
    public static final int kType_Range = 4;
    public static final int kType_Switch = 5;
    public static final int kType_Typeface = 6;

    private List<TextOpt> mTextOptions = new ArrayList<TextOpt>();
    private List<TextOpt> m_externalView_texts = null;
    private List<ColorOpt> mColorOptions = new ArrayList<ColorOpt>();
    private List<ColorOpt> m_externalView_colors = null;
    private List<SelectOpt> mSelectOptions = new ArrayList<SelectOpt>();
    private List<SelectOpt> m_externalView_selects = null;
    private List<RangeOpt> mRangeOptions = new ArrayList<RangeOpt>();
    private List<RangeOpt> m_externalView_ranges = null;

    private List<SwitchOpt> mSwitchOptions = new ArrayList<SwitchOpt>();
    private List<SwitchOpt> m_externalView_switch = null;

    private List<TypefaceOpt> mTypefaceOptions = new ArrayList<TypefaceOpt>();
    private List<TypefaceOpt> m_externalView_Typeface = null;


    private Set<String> mOptionIds = new HashSet<>();

    private nexEffectOptions() {

    }

    /**
     * This method gets the ID of an effect, as a <tt>String</tt>.
     * 
     * <p>Example code:</p>
     *     {@code
                Effect effect = EditorGlobal.getEditor().getEffectLibrary().findEffectById(options.getEffectID());
            }
     * @return The effect ID as a <tt>String</tt>.
     * @since version 1.1.0
     */
    public String getEffectID() {
        return mEffectID;
    }

    protected nexEffectOptions(String effectID, int type){
        mEffectID = effectID;
        mEffectType = type;
    }

    void setEffectType(int type){
        mEffectType = type;
    }

    /**
     * This method gets the input text and other text-related options for an effect.
     * 
     * <p>Example code:</p>
     *     {@code
                mOptions.getTextOptions().get(0).setText(mLine1.getText().toString());
            }
     * @return All text options related to an effect, as a list.
     * 
     * @see #getColorOptions()
     * @see #getSelectOptions()
     * @since version 1.1.0
     */
    public List<TextOpt> getTextOptions(){
        if( m_externalView_texts==null )
            m_externalView_texts = Collections.unmodifiableList(mTextOptions);
        return m_externalView_texts;
    }

    /**
     * This method retrieves the color-related settings and options available on an effect, as a list.
     * 
     * Possible color-related options for an effect include text color, background color, or effect color but 
     * the options available depend on the specific effect.
     * 
     * <p>Example code:</p>
     *     {@code
                mOptions.getColorOptions().get(0).setARGBColor(mColor1Value);
            }
     * @return A list of the color-related options that can be set on the effect.
     * 
     * @see #getTextOptions()
     * @see #getSelectOptions()
     * @since version 1.0.1
     */
    public List<ColorOpt> getColorOptions(){
        if( m_externalView_colors==null )
            m_externalView_colors = Collections.unmodifiableList(mColorOptions);

        return m_externalView_colors;
    }

    /**
     * This method gets the designated text location and other select options available on an effect.
     * 
     * <p>Example code:</p>
     *     {@code
                mOptions.getSelectOptions().get(0).setSelectIndex(position);
            }
     * @return The list of select options that can be set on the effect.
     * 
     * @see #getTextOptions()
     * @see #getColorOptions()
     * @since version 1.0.1
     */
    public List<SelectOpt> getSelectOptions(){
        if( m_externalView_selects==null )
            m_externalView_selects = Collections.unmodifiableList(mSelectOptions);
        return m_externalView_selects;
    }

    /**
     * This gets the RangeOpt list from the options.
     * @return
     * @since 1.7.0
     */
    public List<RangeOpt> getRangeOptions(){
        if( m_externalView_ranges==null )
            m_externalView_ranges = Collections.unmodifiableList(mRangeOptions);
        return m_externalView_ranges;
    }

    /**
     * This gets the SwitchOpt list from the options.
     * @return
     * @since 1.7.0
     */
    public List<SwitchOpt> getSwitchOptions(){
        if( m_externalView_switch==null )
            m_externalView_switch = Collections.unmodifiableList(mSwitchOptions);
        return m_externalView_switch;
    }

    public List<TypefaceOpt> getTypefaceOptions(){
        if( m_externalView_Typeface==null )
            m_externalView_Typeface = Collections.unmodifiableList(mTypefaceOptions);
        return m_externalView_Typeface;
    }

    public int getType(String id){
        if( id.startsWith("text") ){
            return kType_Text;
        }

        if( id.startsWith("selection") ){
            return kType_Select;
        }

        if( id.startsWith("switch") ){
            return kType_Switch;
        }

        if( id.startsWith("typeface") ){
            return kType_Typeface;
        }

        if( id.startsWith("color") ){
            return kType_Color;
        }

        if( id.startsWith("range") ){
            return kType_Range;
        }

        return kType_Unknown;
    }

    public String[] getOptionIdsByGroup(int groupId){
        ArrayList<String> temp = new ArrayList<>();
        for( TextOpt opt : mTextOptions ){
            if( opt.groupId == groupId ){
                temp.add(opt.getId());
            }
        }

        for( ColorOpt opt : mColorOptions ){
            if( opt.groupId == groupId ){
                temp.add(opt.getId());
            }
        }

        for( SelectOpt opt : mSelectOptions ){
            if( opt.groupId == groupId ){
                temp.add(opt.getId());
            }
        }

        for( RangeOpt opt : mRangeOptions ){
            if( opt.groupId == groupId ){
                temp.add(opt.getId());
            }
        }

        for( SwitchOpt opt : mSwitchOptions ){
            if( opt.groupId == groupId ){
                temp.add(opt.getId());
            }
        }

        for( TypefaceOpt opt : mTypefaceOptions ){
            if( opt.groupId == groupId ){
                temp.add(opt.getId());
            }
        }
        String [] ret = new String[temp.size()];
        for( int i = 0 ; i < temp.size() ; i++){
            ret[i] = temp.get(i);
        }
        return ret;
    }

    void makeGroup(){
        int id = 1;
        for( TextOpt opt : mTextOptions ){
            int hitCount = 0;
            String baseId = opt.getId().substring(opt.getId().indexOf(':'));

            for( ColorOpt copt : mColorOptions){
                if( copt.getId().contains(baseId) ){
                    copt.groupId = id;
                    hitCount++;
                }
            }

            for( TypefaceOpt copt : mTypefaceOptions){
                if( copt.getId().contains(baseId) ){
                    copt.groupId = id;
                    hitCount++;
                }
            }

            for( SelectOpt copt : mSelectOptions){
                if( copt.getId().contains(baseId) ){
                    copt.groupId = id;
                    hitCount++;
                }
            }

            for( RangeOpt copt : mRangeOptions){
                if( copt.getId().contains(baseId) ){
                    copt.groupId = id;
                    hitCount++;
                }
            }

            for( SwitchOpt copt : mSwitchOptions){
                if( copt.getId().contains(baseId) ){
                    copt.groupId = id;
                    hitCount++;
                }
            }
            if( hitCount > 0){
                opt.groupId = id;
                id++;
            }
        }
    }

    /**
     *
     * @return
     * @since 2.0.7
     */
    public String[] getOptionIds(){
        return mOptionIds.toArray(new String[mOptionIds.size()]);
    }

    /**
     *
     * @param optionId
     * @return
     * @since 2.0.7
     */
    public Option getOptionById(String optionId){
        for( TextOpt opt : mTextOptions ){
            if( opt.getId().compareTo(optionId) == 0 ){
                return opt;
            }
        }

        for( ColorOpt opt : mColorOptions ){
            if( opt.getId().compareTo(optionId) == 0 ){
                return opt;
            }
        }

        for( SelectOpt opt : mSelectOptions ){
            if( opt.getId().compareTo(optionId) == 0 ){
                return opt;
            }
        }

        for( RangeOpt opt : mRangeOptions ){
            if( opt.getId().compareTo(optionId) == 0 ){
                return opt;
            }
        }

        for( SwitchOpt opt : mSwitchOptions ){
            if( opt.getId().compareTo(optionId) == 0 ){
                return opt;
            }
        }

        for( TypefaceOpt opt : mTypefaceOptions ){
            if( opt.getId().compareTo(optionId) == 0 ){
                return opt;
            }
        }
        return null;
    }

    protected Option getOptionEndWithId(String optionId){
        for( TextOpt opt : mTextOptions ){
            if( opt.getId().endsWith(optionId)  ){
                return opt;
            }
        }

        for( ColorOpt opt : mColorOptions ){
            if( opt.getId().endsWith(optionId)  ){
                return opt;
            }
        }

        for( SelectOpt opt : mSelectOptions ){
            if( opt.getId().endsWith(optionId)  ){
                return opt;
            }
        }

        for( RangeOpt opt : mRangeOptions ){
            if( opt.getId().endsWith(optionId)  ){
                return opt;
            }
        }

        for( SwitchOpt opt : mSwitchOptions ){
            if( opt.getId().endsWith(optionId)  ){
                return opt;
            }
        }

        for( TypefaceOpt opt : mTypefaceOptions ){
            if( opt.getId().endsWith(optionId)  ){
                return opt;
            }
        }

        return null;
    }

    /**
     * This method resets the color and select options that can be set on an effect back to their original default values.
     * 
     * <p>Example code:</p>
     *     {@code
                mOptions.setDefaultValue();
            }
     * @since version 1.1.0
     */
    public void setDefaultValue(){
        if( mColorOptions != null ){
            for (ColorOpt opt : mColorOptions){
                opt.argb_color = opt.default_argb_color;
            }
        }

        if( mSelectOptions != null ){
            for( SelectOpt opt : mSelectOptions){
                opt.setSelectIndex(opt.default_select_index);
            }
        }

        if( mRangeOptions != null ){
            for( RangeOpt opt : mRangeOptions){
                opt.setValue(opt.default_value);
            }
        }

        if( mSwitchOptions != null ){
            for( SwitchOpt opt :  mSwitchOptions ){
                opt.setValue(opt.default_on);
            }
        }

        if( mTypefaceOptions != null ){
            for( TypefaceOpt opt : mTypefaceOptions){
                opt.setTypeface(opt.default_typeface);
            }
        }
    }

    protected void addTextOpt(String id, String label, int lineNum){
        mOptionIds.add(id);
        mTextOptions.add(new TextOpt(id,label,lineNum));
    }

    protected void addColorOpt(String id, String label, String color){
        mOptionIds.add(id);
        mColorOptions.add(new ColorOpt(id,label,color));
    }

    protected void addSelectOpt(String id, String label, String[] items, String[] values, int select){
        mOptionIds.add(id);
        mSelectOptions.add(new SelectOpt(id,label,items,values,select));
    }

    protected void addRangeOpt(String id, String label, int default_value, int min_value, int max_value ){
        mOptionIds.add(id);
        mRangeOptions.add(new RangeOpt(id,label,default_value,min_value,max_value));
    }

    protected void addSwitchOpt(String id, String label, boolean on){
        mOptionIds.add(id);
        mSwitchOptions.add(new SwitchOpt(id,label,on));
    }

    protected void addTypefaceOpt(String id, String label, String typeface){
        mOptionIds.add(id);
        mTypefaceOptions.add(new TypefaceOpt(id,label,typeface));
    }


    /**
     * This method gets the number of text fields in an effect where text can be entered.
     * 
     * <p>Example code:</p>
     *     {@code
                for( int line = 0; line < mOptions.getTextFieldCount() ; line++ ){}
            }
     * @return The number of text fields in the effect, as an <tt>integer</tt>.
     * @since version 1.0.1
     */
    public int getTextFieldCount(){
        if( mTextOptions ==  null )
            return 0;
        return mTextOptions.size();
    }

    /**
     * This class defines and saves the possible text, color, and select options for an effect.
     * 
     * The effect options, <tt>Options</tt>, defined by this class create a serialization, which allows the values
     * to be passed to other applications or activities.
     * 
     * @since version 1.0.1
     */
    public class Option implements Serializable {
        private static final long serialVersionUID = 1L;
        private String mLabel;
        private String mId;
        protected int groupId;
        private Option(){}
        protected Option(String id, String label){

            mLabel = label;
            mId = id;
        }

        /**
         * This method gets the saved ID of an effect option.
         * 
         * @return The ID of the effect option, as a <tt>String</tt>.
         * @since version 1.0.1
         */
        public String getId(){
            return mId;
        }

        /**
         * This method gets the label of an effect option.
         * 
         * Based on the label returned, it is possible to guess which option is in use.
         * 
         * @return  The option label, as a <tt> String</tt>.
         * @since version 1.0.1
         */
        public String getLabel(){
            return mLabel;
        }


        public int getGroupId() {
            return groupId;
        }

    }

    /**
     * This class defines the text-related options possible in an effect.
     * 
     * Text-related options may include text position or if the the text in the effect appears on multiple lines among others.
     * 
     * <p>Example code:</p>
     *     {@code     for( nexEffectOptions.TextOpt opt : text ) {
                    if( f.getId().compareTo(opt.getId()) == 0 ){
                        if( opt.getText() != null ){
                            b.append(URLEncoder.encode(opt.getId(), "UTF-8"));
                            b.append("=");
                            b.append(URLEncoder.encode(opt.getText(),"UTF-8"));
                        }
                    }
                }
            }
     * @see #getTextOptions()
     * @since version 1.1.0
     */
    public class TextOpt extends Option{
        private static final long serialVersionUID = 1L;
        private int mMaxLine;
        private String mTitle;
        private TextOpt(){}
        protected TextOpt(String id, String label, int lineNum ){
            super(id,label);
            mMaxLine = lineNum;
        }

        /**
         * This method sets the text input added to an effect.
         * 
         * @param text  The text to be added to the effect, as a <tt>String</tt>.
         * 
         * @see #getText()
         * @since version 1.1.0
         */
        public void setText(String text){
            mTitle = text;
            updated = true;
        }

        /**
         * This method gets the text saved for an effect.
         * 
         * @return  The text saved on an effect, as a <tt>String</tt>.
         * 
         * @see #setText(String)
         * @since version 1.1.0
         */
        public String getText(){
            return mTitle;
        }

        public String toString() {
            return getClass().getName() + " id:" + getId() + ", label:" + getLabel() + ", title:" + mTitle;
        }

    }

    public class TypefaceOpt extends Option{
        private static final long serialVersionUID = 1L;
        protected String default_typeface;
        protected String mTypefaceName;
        private TypefaceOpt(){}
        protected TypefaceOpt(String id, String label, String typeface){
            super(id,label);
            default_typeface = typeface;
            mTypefaceName = typeface;
        }

        /**
         * This method sets the text input added to an effect.
         *
         * @param text  The text to be added to the effect, as a <tt>String</tt>.
         *
         * @see #getTypeface()
         * @since version 1.1.0
         */
        public void setTypeface(String text){
            if( text == null ){
                mTypefaceName = default_typeface;
                return;
            }
            if((text.indexOf('/') >= 0) || (text.indexOf(':') >= 0 )) {
                mTypefaceName = text;
            }else{
                mTypefaceName = TAG_TYPEFACE_FONTID+text;
            }
            updated = true;
        }

        /**
         * This method gets the text saved for an effect.
         *
         * @return  The text saved on an effect, as a <tt>String</tt>.
         *
         * @see #setTypeface(String)
         * @since version 1.1.0
         */
        public String getTypeface(){
            return mTypefaceName;
        }

        public void reset(){
            if( mTypefaceName != null ){
                if( mTypefaceName.compareTo(default_typeface) != 0 ){
                    updated = true;
                }
            }
            mTypefaceName = default_typeface;
        }

        public String toString() {
            return getClass().getName() + " id:" + getId() + ", label:" + getLabel() + ", title:" + mTypefaceName;
        }
    }

    /**
     * 
     * This class saves the possible color-related options for an effect.
     * 
     * Possible color-related options for an effect may include text or background colors 
     * among others, depending on the specific effect.
     * 
     * <p>Example code:</p>
     *     {@code     for( nexEffectOptions.ColorOpt opt : color){
                    if( f.getId().compareTo(opt.getId()) == 0 ){
                        b.append(URLEncoder.encode(opt.getId(), "UTF-8"));
                        b.append("=");
                        b.append(URLEncoder.encode(ColorUtil.colorString(opt.getARGBformat()) ,"UTF-8"));
                    }
                }
            }
     * @see #getColorOptions()
     * @since version 1.0.1
     */
    public class ColorOpt extends Option{
        protected int default_argb_color;
        protected int argb_color;
        private ColorOpt(){}
        protected ColorOpt(String id, String label, String color ){
            super(id,label);
            argb_color = ColorUtil.parseColor(color);
            default_argb_color = argb_color;
        }

        /**
         * This method gets the saved color for a color-related option, as an ARGB 32-bit value.
         * 
         * @return The option color in ARGB format, as an <tt>integer</tt>.
         * @since version 1.0.1
         */
        public int getARGBformat(){
            //int alpha = argb_color >> 24;
            return argb_color;
        }
/*
        public int getRGBAformat(){
            return rgba_color;
        }
*/
        /**
         * This method sets the color for a color-related option as an ARGB 32-bit value.
         * 
         * @param ARGBformat  The color to set in ARGB format, as an <tt>integer</tt>.
         * 
         * @since version 1.0.1
         */
        public void setARGBColor(int ARGBformat ){
            //argb_color = ARGBformat;
            if(  argb_color != ARGBformat) {
                updated = true;
            }
            argb_color = ARGBformat;
        }

        public void reset(){
            if(  argb_color != default_argb_color) {
                updated = true;
            }
            argb_color = default_argb_color;
        }
    }

    /**
     * This class defines options that must be selected for an effect, such as text position and radio select options (allows the user to choose only one of a predefined set of options).
     * 
     * <p>Example code:</p>
     *     {@code     for( nexEffectOptions.SelectOpt opt : select){
                    if( f.getId().compareTo(opt.getId()) == 0 ){
                        int val = opt.getSelectIndex();
                        List<UserField.FieldOption> list = f.getOptions();
                        b.append(URLEncoder.encode(opt.getId(), "UTF-8"));
                        b.append("=");
                        b.append(URLEncoder.encode(opt.getSelectValue(),"UTF-8"));
                    }
                }
            }
     * @see #getSelectOptions()
     * @since version 1.0.1
     */
    public class SelectOpt extends Option{
        private static final long serialVersionUID = 1L;
        protected int default_select_index;
        protected int select_index;
        private String[] mItems;
        private String[] mValues;

        private SelectOpt(){}
        protected SelectOpt(String id, String label, String[] items, String[] values,  int select ){
            super(id,label);
            mItems = items;
            mValues = values;
            select_index = select;
            default_select_index = select_index;
        }

        protected  int setValue(String value){
            for( int i = 0 ; i < mValues.length ; i++ ){
                if( mValues[i].compareTo(value) == 0 ){
                    if(  select_index != i) {
                        updated = true;
                    }
                    select_index = i;
                    break;
                }
            }
            return 0;
        }

        /**
         * This method gets the option index number of the currently selected option.
         * 
         * @return The index number of the selected option, as an <tt>integer</tt>.
         * @since version 1.0.1
         */
        public int getSelectIndex(){
            return select_index;
        }

        /**
         * This method sets the index number of the currently selected option item.
         * 
         * @param index The index number to set, as an <tt>integer</tt>.
         * 
         * @since version 1.0.1
         */
        public void setSelectIndex(int index){
            if( index < 0 )
                return;
            if( mItems == null )
                return;

            if( index >= mItems.length ){
                return;
            }
            select_index = index;
        }

        /**
         * This method gets items that can be selected for a particular select effect option, <tt>SelectOpt</tt>.
         * 
         * The sequence of the items determines their index number.
         * 
         * @return item  The items that can be selected for the select effect option, as an array of <tt>Strings</tt>.
         * @since version 1.0.1
         */
        public String[] getItems(){
            return mItems;
        }

        /**
         * This method gets the value selected for a particular select effect option, <tt>SelectOpt</tt>.
         * 
         * @return The selected value of the option, as a <tt>String</tt>.
         * @since version 1.1.0
         */
        public String getSelectValue() {
            return mValues[select_index];
        }
    }

    /**
     * This is an option that receives input similar to a slider.
     * @since 1.7.0
     */
    public class RangeOpt extends Option{
        private static final long serialVersionUID = 1L;
        protected int default_value;
        protected int min_value;
        protected int max_value;
        protected int mValue;
        private RangeOpt(){}
        protected RangeOpt(String id, String label, int default_value, int min_value, int max_value  ){
            super(id,label);
            this.default_value = default_value;
            this.max_value = max_value;
            this.min_value = min_value;
            mValue = default_value;
        }

        /**
         * Input setting values. Value should be between the min and max values.
         * If the number is smaller than min(), it will be substitued by min() value; vice versa.
         * @param value min() ~ max(),
         * @since 1.7.0
         */
        public void setValue(int value){
            if( value < min_value){
                value = min_value;
            }else if( value > max_value  ){
                value = max_value;
            }
            if( mValue != value ) {
                updated = true;
            }
            mValue = value;
        }

        /**
         * Returns the currently set value.
         * @return The set value.
         * @since 1.7.0
         */
        public int getValue(){
            return mValue;
        }

        /**
         * Returns the minimum value that can be set.
         * @return min value
         * @since 1.7.0
         */
        public int min(){
            return min_value;
        }

        /**
         * Returns the maximum value that can be set.
         * @return max value
         * @since 1.7.0
         */
        public int max(){
            return max_value;
        }

        public void reset(){
            if( mValue != default_value ){
                updated = true;
            }
            mValue = default_value;
        }

    }

    /**
     * An option that sets \c ON/OFF.
     * @since 1.7.0
     */
    public class SwitchOpt extends Option{
        protected boolean default_on;
        protected boolean on;
        private SwitchOpt(){}
        protected SwitchOpt(String id, String label, boolean on ){
            super(id,label);
            this.default_on = on;
            this.on = on;
        }

        /**
         * This gets the set value.
         * @return \c TRUE - on, \c FALSE - off.
         * @since 1.7.0
         */
        public boolean getValue(){
            return on;
        }

        /**
         * This inputs values to set.
         * @param on \c TRUE - on , \c FALSE - off
         * @since 1.7.0
         */
        public void setValue(boolean on){
            if( this.on != on ){
                updated = true;
            }
            this.on = on;
        }

        public void reset(){
            if( on != default_on ){
                updated = true;
            }
            on = default_on;
        }

    }

    boolean isUpdated(){
        return updated;
    }

    void clearUpadted(){
        updated = false;
    }
}
