package com.nexstreaming.app.common.util;

/**
 * Created by damian.lee on 2016-04-11.
 */
public class CapabilityInfo {

    /* Device Information */
    String model;
    String manufacture;
    String chipset;
    String os;

    /* Device Performance */
    int codecMemSize;
    int maxFPS;
    int maxResolution;
    boolean bSupportMPEGV4;

    /* encoder operates in the background */
    int availableDecCount1;
    int realDecCount1;
    /* encoder does not operate in the background */
    int availableDecCount2;
    int realDecCount2;

    public CapabilityInfo()
    {

    }

    public CapabilityInfo(String model, String manufacture, String chipset, String os, int memSize, int maxFPS, int maxResolution, boolean supportMPEGV4,
                          int availableDecCount1, int realDecCount1, int availableDecCount2, int realDecCount2)
    {
        this.model = model;
        this.manufacture = manufacture;
        this.chipset = chipset;
        this.os = os;

        this.codecMemSize = memSize;
        this.maxFPS = maxFPS;
        this.maxResolution = maxResolution;
        this.bSupportMPEGV4 = supportMPEGV4;

        this.availableDecCount1 = availableDecCount1;
        this.realDecCount1 = realDecCount1;
        this.availableDecCount2 = availableDecCount2;
        this.realDecCount2 = realDecCount2;
    }

    public String getModel(){
        return model;
    }
    public String getManufacture(){
        return manufacture;
    }
    public String getChipset(){
        return chipset;
    }
    public String getOS(){
        return os;
    }
    public int getCodecMemSize(){
        return codecMemSize;
    }
    public int getMaxFPS(){
        return maxFPS;
    }
    public int getMaxResolution(){
        return maxResolution;
    }
    public boolean isbSupportMPEGV4(){
        return bSupportMPEGV4;
    }
    public int getAvailableDecCount1(){
        return availableDecCount1;
    }
    public int getRealDecCount1(){
        return realDecCount1;
    }
    public int getAvailableDecCount2(){
        return availableDecCount2;
    }
    public int getRealDecCount2(){
        return realDecCount2;
    }

    public void setModel(String model) {
        this.model = model;
    }
    public void setManufacture(String manufacture) {
        this.manufacture = manufacture;
    }
    public void setChipset(String chipset) {
        this.chipset = chipset;
    }
    public void setOS(String os) {
        this.os = os;
    }
    public void setCodecMemSize(int size) {
        this.codecMemSize = size;
    }
    public void setMaxFPS(int size) {
        this.maxFPS = size;
    }
    public void setMaxResolution(int size) {
        this.maxResolution = size;
    }
    public void setSupportMPEGV4(boolean value)
    {
        this.bSupportMPEGV4 = value;
    }
    public void setAvailableDecCount1(int count) {
        this.availableDecCount1 = count;
    }
    public void setRealDecCount1(int count) {
        this.realDecCount1 = count;
    }
    public void setAvailableDecCount2(int count) {
        this.availableDecCount2 = count;
    }
    public void setRealDecCount2(int count) {
        this.realDecCount2 = count;
    }
}

