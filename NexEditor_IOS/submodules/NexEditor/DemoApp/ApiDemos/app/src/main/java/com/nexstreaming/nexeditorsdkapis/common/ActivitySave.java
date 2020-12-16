package com.nexstreaming.nexeditorsdkapis.common;

/**
 * Created by jeongwook.yoon on 2017-10-16.
 */

public class ActivitySave {
    private static String slabel;
    private static String sparm1;
    private static String sparm2;

    public static void setLabel(String label){
        if( slabel != null ){
            if( slabel.compareTo(label) != 0 ){
                sparm1 = sparm2 = null;
            }
        }
        slabel = label;

    }

    public static void setSaveData(String parm1 , String parm2){
        sparm1 = parm1;
        sparm2 = parm2;
    }

    public static String getLabel(){
        return slabel;
    }

    public static String getPram1(){
        return sparm1;
    }

    public static String getPram2(){
        return sparm2;
    }

    public static boolean existSaveData(String label){
        if( slabel != null ){
            if( slabel.compareTo(label) == 0 ){
                if( sparm1 != null || sparm2 != null ){
                    return true;
                }
            }
        }
        return false;
    }
}
