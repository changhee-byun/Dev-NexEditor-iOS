package com.nexstreaming.scmbsdk;

/**
 * SCMB Entry Class.
 * @see nxSCMBSDK#getEntries()
 * @see nxSCMBSDK#setEntries(nxSCMBEntry[])
 */
public final class nxSCMBEntry {
    private int mSpeed;
    private long mStartMs;
    private long mEndMs;
    private byte[] mExtra;
    /**
     * SCMB Entry
     * Entry duration is (endMs - startMs)*100/speed.
     * @param speed unit is %(percent). normal speed is 100. 2X speed is 200. 0.5X speed is 50.
     * @param startMs unit is ms. Trim start point Time.
     * @param endMs unit is ms. Trim end point Time
     */
    public nxSCMBEntry(int speed, long startMs , long endMs) {
        mSpeed = speed;
        mStartMs = startMs;
        mEndMs = endMs;
    }

    public nxSCMBEntry(int speed, long startMs , long endMs, byte[] extra) {
        mSpeed = speed;
        mStartMs = startMs;
        mEndMs = endMs;
        mExtra = extra;
    }

    /**
     * get speed
     * @return unit is %(percent). speed value was return.
     */
    public int getSpeed(){
        return mSpeed;
    }

    /**
     * Get Trim start point Time
     * @return unit is ms. Trim start point Time value was return.
     */
    public long getStartMs(){
        return mStartMs;
    }

    /**
     * Get Trim end point Time
     * @return unit is ms. Trim end point Time was return.
     */
    public long getEndMs(){
        return mEndMs;
    }

    public byte[] getExtra() {
        return mExtra;
    }

    /**
     * Get Entry duration
     * @return unit is ms. (endMs - startMs)*100/speed.
     */
    public long getDuration(){
        return (mEndMs - mStartMs)*100/mSpeed;
    }

    /**
     *
     * @param speed unit is %(percent)
     */
    public void setSpeed(int speed){
        mSpeed = speed;
    }

    /**
     *
     * @param start unit is ms
     */
    public void setStartMs(long start){
        mStartMs = start;
    }

    /**
     *
     * @param end unit is ms
     */
    public void setEndMs(long end){
        mEndMs = end;
    }


    public void setExtra(byte[] data){
        mExtra = data;
    }
}
