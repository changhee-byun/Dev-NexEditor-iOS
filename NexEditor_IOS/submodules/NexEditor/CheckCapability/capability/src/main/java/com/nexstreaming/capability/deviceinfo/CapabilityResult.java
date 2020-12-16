package com.nexstreaming.capability.deviceinfo;

import com.nexstreaming.capability.util.CBLog;

import java.util.ArrayList;

/**
 * Created by damian.lee on 2016-06-23.
 */
public class CapabilityResult {
    String model_name;
    String manufacture;
    String chipset;
    String os_verson;

    int HWCodecMemSize;
    int MaxFPS;
    int MaxResolution;
    boolean supportMPEGV4;

    ArrayList<TestResult> testResultList  = new ArrayList<>();

    public class TestResult {
        int decoder_count;
        int in_time_count;

        public TestResult(int decoder_count, int in_time_count) {
            this.decoder_count = decoder_count;
            this.in_time_count = in_time_count;
        }

        public void setDecoderCount(int value) {
            decoder_count = value;
        }
        public void setInTimeCount(int value) {
            in_time_count = value;
        }
        public int getDecoderCount() {
            return decoder_count;
        }
        public int getInTimeCount() {
            return in_time_count;
        }
    }

    public CapabilityResult() {

        DeviceInformation deviceInformation = new DeviceInformation();
        deviceInformation.init();

        model_name = deviceInformation.getModelName();
        manufacture = deviceInformation.getManufacture();
        chipset = deviceInformation.getChipset();
        os_verson = deviceInformation.getOSVersion();

        HWCodecMemSize = 0;
        MaxFPS = 0;
        MaxResolution = 0;
        supportMPEGV4 = true;

        for(int i=0; i<2; i++) {
            testResultList.add(new TestResult(0,0));
        }
    }

    public ArrayList<TestResult> getTestResultList() {
        return testResultList;
    }

    public void releaseResource() {
        testResultList.clear();
        testResultList = null;
    }

    public String getModelName() {
        return model_name;
    }

    public String getManufacture() {
        return manufacture;
    }

    public String getChipset() {
        return chipset;
    }

    public String getOS() {
        return os_verson;
    }


    public void setDecoderCount2TestList(int idx, int value) {
        testResultList.get(idx).setDecoderCount(value);
    }

    public void setInTimeCount2TestList(int idx, int value) {
        testResultList.get(idx).setInTimeCount(value);
    }

    public int getDecoderCount2TestList(int idx) {
        return testResultList.get(idx).getDecoderCount();
    }

    public int getInTimeCount2TestList(int idx) {
        return testResultList.get(idx).getInTimeCount();
    }

    public void setHWCodecMemSize(int value) {
        HWCodecMemSize = value;
    }

    public int getHWCodecMemSize() {
        return HWCodecMemSize;
    }

    public void setMaxFPS(int value) {
        MaxFPS = value;
    }

    public int getMaxFPS() {
        return MaxFPS;
    }

    public void setMaxResolution(int value) {
        MaxResolution = value;
    }

    public int getMaxResolution() {
        return MaxResolution;
    }

    public void setSupportMPEGV4(boolean bValue) {
        supportMPEGV4 = bValue;
    }

    public boolean isSupportMPEGV4() {
        return supportMPEGV4;
    }
}
