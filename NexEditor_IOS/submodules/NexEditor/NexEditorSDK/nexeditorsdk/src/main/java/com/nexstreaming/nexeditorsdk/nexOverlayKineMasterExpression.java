/******************************************************************************
 * File Name        : nexOverlayKineMasterExpression.java
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

import java.util.Arrays;
import java.util.List;

/**
 * This class is a list of predefined animations of the KineMaster&tm;. 
 *
 * The expressions of predefined animations follow the KineMaster&tm;. 
 *
 * <p>Example code :</p>
 {@code
    mProject.getOverlay(mOverlayItemId).setLayerExpression(nexOverlayKineMasterExpression.getExpression(position));
 }

 * @since version 1.5.23
 */
@Deprecated
public final class nexOverlayKineMasterExpression implements Cloneable {
    private String name;
    private int KineMasterID;
    private String KMinID;
    private String KMoutID;
    private String KMoverallID;
    public static final nexOverlayKineMasterExpression NONE = new nexOverlayKineMasterExpression("None",0,"none","none","none");
    public static final nexOverlayKineMasterExpression FADE = new nexOverlayKineMasterExpression("Fade",1,"FadeIn","FadeOut","none");
    public static final nexOverlayKineMasterExpression POP = new nexOverlayKineMasterExpression("Pop",2,"PopIn","ScaleUpOut","none");
    public static final nexOverlayKineMasterExpression SLIDE = new nexOverlayKineMasterExpression("Slide",3,"SlideRightIn","SlideRightOut","none");
    public static final nexOverlayKineMasterExpression SPIN = new nexOverlayKineMasterExpression("Spin",4,"SpinCCWIn","SpinCWOut","none");
    public static final nexOverlayKineMasterExpression DROP = new nexOverlayKineMasterExpression("Drop",5,"DropIn","FadeOut","none");
    public static final nexOverlayKineMasterExpression SCALE = new nexOverlayKineMasterExpression("Scale",6,"ScaleUpIn","ScaleDownOut","none");
    public static final nexOverlayKineMasterExpression FLOATING = new nexOverlayKineMasterExpression("Floating",7,"FadeIn","FadeOut","FloatingOverall");
    public static final nexOverlayKineMasterExpression DRIFTING = new nexOverlayKineMasterExpression("Drifting",8,"FadeIn","FadeOut","DriftingOverall");
    public static final nexOverlayKineMasterExpression SQUISHING = new nexOverlayKineMasterExpression("Squishing",9,"FadeIn","FadeOut","SquishingOverall");

    private static final nexOverlayKineMasterExpression[] values = {
            NONE, FADE,  POP, SLIDE, SPIN, DROP, SCALE,FLOATING, DRIFTING, SQUISHING
    };

    /**
     * This method gets the animation presets that are compatible with KineMaster&trade;&nbsp; as a list.
     * @return A list of predefined animations. 
     * @since version 1.5.23
     */
    @Deprecated
    public static List<nexOverlayKineMasterExpression> getPresetList() {
        return Arrays.asList(values);
    }

    /**
     * This method gets the animation presets that are compatible with KineMaster&trade;&nbsp; as an <tt>array</tt>.
     * @return An <tt>array</tt> of predefined animations. 
     * @since version 1.5.23
     */
    @Deprecated
    public static nexOverlayKineMasterExpression[] values() {
        return values;
    }

    /**
     * This method gets the ID value of <tt>nexOverlayKineMasterExpression</tt>.
     *
     * @param id The ID of the predefined nexOverlayKineMasterExpression which can be found out by <tt>getID()</tt>.
     *
     * @return The animation that corresponds to the parameter <tt>id</tt>. 
     * @throws InvalidRangeException will occur when the ID does not exist.
     *
     * @since version 1.5.23
     */
    @Deprecated
    public static nexOverlayKineMasterExpression getExpression(int id){
        if( id >= values.length ){
            throw new InvalidRangeException(id,values.length);
        }

        if( id < 0 ){
            throw new InvalidRangeException(id);
        }
        return values[id];
    }

    /**
     * This method gets the names of the animation presets that are compatible with KineMaster&trade;&nbsp; as an <tt>array</tt>.
     * @return The names of predefined animations. 
     * @since version 1.5.23
     */
    @Deprecated
    public static String[] getNames(){
        String[] ret = new String[values.length];
        for( int i = 0 ; i < values.length ; i++ ){
            ret[i] = values[i].name;
        }
        return ret;
    }

    private nexOverlayKineMasterExpression(String name, int id, String KMinID, String KMoutID,  String KMoverallID){
        this.name =name;
        KineMasterID = id;
        this.KMinID = KMinID;
        this.KMoutID = KMoutID;
        this.KMoverallID = KMoverallID;
    }

    /**
     * This method gets the ID value.
     * 
     * @return The ID value.
     */
    @Deprecated
    public int getID(){
        return KineMasterID;
    }

    String getNames(int mode){
        if( mode == 0 ){
            return KMinID;
        }else if( mode == 1 ){
            return KMoutID;
        }else if( mode == 2 ){
            return KMoverallID;
        }
        return null;
    }
}
