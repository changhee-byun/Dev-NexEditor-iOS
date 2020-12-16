package com.nexstreaming.app.common.util;

import com.google.gson_nex.stream.JsonReader;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 * Created by damian.lee on 2016-04-11.
 */
public class JsonReaderUtil {

    public CapabilityInfo readCapabilityJson(InputStream in) throws IOException{
        JsonReader reader = new JsonReader(new InputStreamReader(in, "UTF-8"));
        return readCategory(reader);
    }

    public CapabilityInfo readCategory(JsonReader reader) throws IOException  {
        CapabilityInfo capabilityInfo = new CapabilityInfo();
        reader.beginObject();

        while(reader.hasNext()) {
            String name = reader.nextName();

            if(name.equals("deviceinfo")) {
                readDeviceInfo(reader, capabilityInfo);
            } else if(name.equals("performance")) {
                readPerformance(reader, capabilityInfo);
            } else {
                reader.skipValue();
            }
        }
        reader.endObject();

        return capabilityInfo;
    }

    public void readDeviceInfo(JsonReader reader, CapabilityInfo capabilityInfo) throws IOException {

        reader.beginObject();
        while (reader.hasNext()) {
            String name = reader.nextName();

            if (name.equals("model")) {

                capabilityInfo.setModel(reader.nextString());
            } else if (name.equals("manufacture")) {
                capabilityInfo.setManufacture(reader.nextString());
            } else if (name.equals("chipset")) {
                capabilityInfo.setChipset(reader.nextString());
            } else if (name.equals("os")) {
                capabilityInfo.setOS(reader.nextString());
            } else {
                reader.skipValue();
            }
        }
        reader.endObject();
    }

    public void readPerformance(JsonReader reader, CapabilityInfo capabilityInfo) throws IOException {

        reader.beginObject();
        while (reader.hasNext()) {
            String name = reader.nextName();

            if (name.equals("codec_mem_size")) {
                capabilityInfo.setCodecMemSize(reader.nextInt());
            } else if (name.equals("max_fps")) {
                capabilityInfo.setMaxFPS(reader.nextInt());
            } else if (name.equals("max_resolution")) {
                capabilityInfo.setMaxResolution(reader.nextInt());
            } else if (name.equals("mpeg4v_supported")) {
                capabilityInfo.setSupportMPEGV4(reader.nextBoolean());
            } else if (name.equals("use_encoder") || name.equals("unuse_encoder")) {
                readCodecCount(reader, capabilityInfo, name);
            } else {
                reader.skipValue();
            }
        }
        reader.endObject();
    }

    public void readCodecCount(JsonReader reader, CapabilityInfo capabilityInfo, String mode) throws IOException {

        reader.beginObject();
        while (reader.hasNext()) {
            String name = reader.nextName();

            if (name.equals("available_dec_count")) {
                if(mode.equals("use_encoder")) {
                    capabilityInfo.setAvailableDecCount1(reader.nextInt());
                } else {
                    capabilityInfo.setAvailableDecCount2(reader.nextInt());
                }
            } else if (name.equals("realtime_dec_count")) {
                if(mode.equals("use_encoder")) {
                    capabilityInfo.setRealDecCount1(reader.nextInt());
                } else {
                    capabilityInfo.setRealDecCount2(reader.nextInt());
                }
            } else {
                reader.skipValue();
            }
        }
        reader.endObject();
    }
}
