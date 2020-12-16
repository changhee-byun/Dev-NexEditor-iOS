package com.nexstreaming.scmbsdk;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;

/**
 * SCMB (Speed Control Meta Box) in Mp4 container.<br>
 * You must 1st call {@link #createInstance(String)}.<br>
 * And finish job was call {@link #destroyInstance()}<br>
 *
 * <p>Example code : load SCMB</p>
 * <pre>
 * {@code
    nxSCMBSDK instance = nxSCMBSDK.createInstance(m_listfilepath.get(0));
        if( instance != null ){
            nxSCMBEntry[] entry = instance.getEntries();
            if( entry != null ){
                for(int i = 0; i < entry.length ; i++ ) {
                    long startTrimTime = entry[i].getStartMs();
                    long endTrimTime = entry[i].getEndMs();
                    int speed = entry[i].getSpeed();
                }
            }
            instance.destroyInstance();
        }
    }
    </pre>
 <br>
 * <p>Example code : save SCMB</p>
 * <pre>
 * {@code
    nxSCMBEntry[] entries  = new nxSCMBEntry[3];
        entries[0] = new nxSCMBEntry(100,0,startTrim);
        entries[1] = new nxSCMBEntry(speed,startTrim,endTrim);
        entries[2] = new nxSCMBEntry(100,endTrim,10000);

        nxSCMBSDK instance = nxSCMBSDK.createInstance(m_listfilepath.get(0));
        if (instance != null) {
            instance.setEntries(entries);
            instance.destroyInstance();
        }
    }
 </pre>
 <br>
 * <p>Example code : clear SCMB</p>
 * <pre>
 * {@code
    nxSCMBSDK instance = nxSCMBSDK.createInstance(m_listfilepath.get(0));
        if (instance != null) {
            instance.setEntries(null);
            instance.destroyInstance();
        }
    }
 </pre>

 */
public final class nxSCMBSDK {

    private native int checkValidate(Context context);
    private native int openFile(String path);
    private native int closeFile(int handle);
    private native Entry [] getEntries(int handle,int type);
    private native int setEntries(int handle, Entry [] entrys);
    private native int setImageEntries(int handle, Entry [] entrys);

    private static int countOfHandles = 0;
    private static final int kEntryType_SCMB = 0;
    private static final int kEntryType_IMAGE = 1;
    private int handle = -1;
    private ErrorCode errorCode;



    private nxSCMBSDK(){

    }

    private static class Entry{
        int mType;
        int mSpeed;
        long mStartMs;
        long mEndMs;
        byte[] mExtra;
    }

    /**
     * get version
     * @return version code.
     */
    public static int versionCode(){
        return 23;
    }

    public int init(Context context){
        //return checkValidate(context);
        return 0;
    }

    /**
     * create sdk instance
     * @param path MP4 format Media path.
     * @return error is null.
     * @see #getLastErrorCode()
     */
    public static nxSCMBSDK createInstance (String path){
        nxSCMBSDK ret = new nxSCMBSDK();
        int h = ret.openFile(path);
        if( h < 0 ){
            ret.errorCode = ErrorCode.fromValue(h);
            return null;
        }
        countOfHandles++;
        ret.handle = h;
        return ret;
    }

    /**
     * destroy instance
     */
    public void destroyInstance(){
        closeFile(handle);
        handle = -1;
        countOfHandles--;
    }

    /**
     * get loaded SCMB entries.
     * @return no entry or error is size 0 array.
     */
    public nxSCMBEntry[] getEntries(){
        if( handle < 0 ) {
            errorCode = ErrorCode.INVALID_Handle;
            return new nxSCMBEntry[0];
        }

        nxSCMBSDK.Entry[] entrys = getEntries( handle,kEntryType_SCMB);
        if( entrys == null ){
            return new nxSCMBEntry[0];
        }

        nxSCMBEntry[] ret = new nxSCMBEntry[entrys.length];
        for( int i = 0 ; i < entrys.length ; i++ ){
            ret[i] = new nxSCMBEntry(entrys[i].mSpeed,entrys[i].mStartMs,entrys[i].mEndMs,entrys[i].mExtra);
        }
        return ret;
    }

    /**
     * get loaded Image entries.
     * @return no entry or error is size 0 array.
     */
    public nxImageEntry[] getImageEntries(){
        if( handle < 0 ) {
            errorCode = ErrorCode.INVALID_Handle;
            return new nxImageEntry[0];
        }

        nxSCMBSDK.Entry[] entrys = getEntries( handle,kEntryType_IMAGE);
        if( entrys == null ){
            return new nxImageEntry[0];
        }

        nxImageEntry[] ret = new nxImageEntry[entrys.length];
        for( int i = 0 ; i < entrys.length ; i++ ){
            ret[i] = new nxImageEntry(entrys[i].mExtra);
        }
        return ret;
    }

    /**
     * save and write SCMB entries.
     * @param entrys null is all clear.
     * @return false - error.
     */
    public boolean setEntries( nxSCMBEntry[] entrys){
        int errno = 0;
        if( handle < 0 ) {
            errorCode = ErrorCode.INVALID_Handle;
            return false;
        }

        if( entrys == null ){
            errno = setEntries(handle,null);
            errorCode = ErrorCode.fromValue(errno);
            if( errno != 0 ){
                return false;
            }
            return true;
        }

        nxSCMBSDK.Entry[] set = new nxSCMBSDK.Entry[entrys.length];
        for( int i = 0 ; i < entrys.length ; i++ ){
            nxSCMBSDK.Entry ent = new nxSCMBSDK.Entry();
            ent.mType = kEntryType_SCMB;
            ent.mSpeed = entrys[i].getSpeed();
            ent.mStartMs = entrys[i].getStartMs();
            ent.mEndMs = entrys[i].getEndMs();
            ent.mExtra = entrys[i].getExtra();
            set[i] = ent;
        }
        errno = setEntries(handle,set);
        errorCode = ErrorCode.fromValue(errno);
        if( errno != 0 ){
            return false;
        }
        return true;
    }

    /**
     * save and write Image meta.
     * @param entrys
     * @return false - error.
     */
    public boolean setImageEntries( nxImageEntry[] entrys){
        int errno = 0;
        if( handle < 0 ) {
            errorCode = ErrorCode.INVALID_Handle;
            return false;
        }

        if( entrys == null ){
            return false;
        }

        for( int i = 0 ; i < entrys.length ; i++ ){
            if( entrys[i].getData() == null ){
                return false;
            }
        }

        nxSCMBSDK.Entry[] set = new nxSCMBSDK.Entry[entrys.length];
        for( int i = 0 ; i < entrys.length ; i++ ){
            nxSCMBSDK.Entry ent = new nxSCMBSDK.Entry();
            ent.mType = kEntryType_IMAGE;
            ent.mSpeed = 0;
            ent.mStartMs = 0;
            ent.mEndMs = 0;
            ent.mExtra = entrys[i].getData();
            set[i] = ent;
        }
        errno = setImageEntries(handle,set);
        errorCode = ErrorCode.fromValue(errno);
        if( errno != 0 ){
            return false;
        }
        return true;

    }

    /**
     * The API to see the cause of the error.
     * @return last error code.
     */
    public ErrorCode getLastErrorCode(){
        return errorCode;
    }

    static {
        System.loadLibrary("nxSCMBjni");
    }
}
