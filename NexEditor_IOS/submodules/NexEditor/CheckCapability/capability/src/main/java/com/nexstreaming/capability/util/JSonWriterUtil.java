package com.nexstreaming.capability.util;

import android.os.Environment;
import android.util.JsonReader;
import android.util.JsonWriter;

import com.nexstreaming.capability.deviceinfo.CapabilityResult;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

/**
 * Created by Eric on 2015-12-02.
 */
public class JSonWriterUtil {
    /**
     * we'd like to encoder a stream of messages such as the following:
     {
        "deviceinfo": {
            "model":"string",
            "manufacture":"string",
            "chipset":"string",
            "os":"string",
        }

        "performance": {
            "codec_mem_size":"Integer",
            "max_fps":"Integer"
            "max_resolution":"Integer",
            "mpeg4v_supported":"boolean",

            {
                "use_encoder": {
                    "available_decoder":"Integer",
                    "realtime_decoder":"Integer"
                },
                "unuse_encoder": {
                    "available_decoder":"Integer",
                    "realtime_decoder":"Integer"
                }
            }

        }
     }
     */

    File outputFile;

    /**
     * 결과 파일이 만들어질 경로를 받고, 설정한 이름에 맞춰서 파일을 만들어준다.
     * 인자: 경로
     * 인자: 파일네임
     * 리턴: OutputStream
     */
    public OutputStream getOutputStream2WantedPath(String directoryPath, String fileName) {
        OutputStream out = null;
        try {
            outputFile = new File(directoryPath, fileName);
            out = new BufferedOutputStream(new FileOutputStream(outputFile));
        } catch ( NullPointerException |FileNotFoundException e ) {
            e.printStackTrace();
        }
        return out;
    }

    /**
     * 외부저장소/CodecCapcity/CapacityResult.txt 파일을 만들어준다.
     * 리턴: OutputStream
     */
    public OutputStream getOutputStream2DefaultPath() {
        OutputStream out = null;
        String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        File tmpDirectory = new File(sdcardPath+ File.separator+"CodecCapacity");

        if ( !tmpDirectory.exists() ) {
            if ( !tmpDirectory.mkdirs() ) {
                CBLog.d("make directory fail");
                return out;
            }
        }

        try {
            outputFile = new File(tmpDirectory, "CapabilityResult.txt");
            out = new BufferedOutputStream(new FileOutputStream(outputFile));
        } catch ( NullPointerException |FileNotFoundException e ) {
            e.printStackTrace();
        }
        return out;
    }

    public void writeJsonStream(OutputStream out, CapabilityResult capabilityResult) throws IOException {
        JsonWriter writer = new JsonWriter(new OutputStreamWriter(out, "UTF-8"));
        writer.setIndent(" ");
        writeMessage(writer, capabilityResult);
        writer.close();
    }

    void writeMessagesArray(JsonWriter writer, CapabilityResult capabilityResult) throws IOException {
        writer.beginArray();
        writeMessage(writer, capabilityResult);
        writer.endArray();
    }

    void writeMessage(JsonWriter writer, CapabilityResult capabilityResult) throws IOException {
        ArrayList<CapabilityResult.TestResult> testResultList = capabilityResult.getTestResultList();
        writer.beginObject();
        writer.name("deviceinfo");

        writer.beginObject();
        writer.name("model").value(capabilityResult.getModelName());
        writer.name("manufacture").value(capabilityResult.getManufacture());
        writer.name("chipset").value(capabilityResult.getChipset());
        writer.name("os").value(capabilityResult.getOS());
        writer.endObject();

        writer.name("performance");
        writer.beginObject();
        writer.name("codec_mem_size").value(capabilityResult.getHWCodecMemSize());
        writer.name("max_fps").value(capabilityResult.getMaxFPS());
        writer.name("max_resolution").value(capabilityResult.getMaxResolution());
        writer.name("mpeg4v_supported").value(capabilityResult.isSupportMPEGV4());

        writer.name("use_encoder");
        writeCapacity(writer, testResultList.get(0));
        writer.name("unuse_encoder");
        writeCapacity(writer, testResultList.get(1));

        writer.endObject();
        writer.endObject();
    }

    void writeCapacity(JsonWriter writer, CapabilityResult.TestResult testResult) throws IOException {
        writer.beginObject();
        writer.name("available_dec_count").value(testResult.getDecoderCount());
        writer.name("realtime_dec_count").value(testResult.getInTimeCount());
        writer.endObject();
    }

    public static InputStream loadResultJsonFile() throws IOException {
        File fResult;
        InputStream in;

        // Load result file
        String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        File tmpDirectory = new File(sdcardPath+ File.separator+"CodecCapacity");
        if ( !tmpDirectory.exists() ) {
            return null;
        }

        try {
            fResult = new File(tmpDirectory, "CapabilityResult.txt");
            in = new BufferedInputStream(new FileInputStream(fResult));
            if ( in == null ) return null;
        } catch ( NullPointerException |FileNotFoundException e ) {
            return null;
        }

        // Json Reader
        JsonReader reader=null;
        try {
            reader = new JsonReader(new InputStreamReader(in, "UTF-8"));
        } catch ( UnsupportedEncodingException e ) {
            return null;
        }

        // Read & Parse Json file.
        String name1=null, name2=null, name3=null, svalue=null;
        int ivalue=0;
        boolean bvalue=false;
        int[] count = new int[4];

        reader.beginObject();
        while ( reader.hasNext() ) {
            name1 = reader.nextName();
            if ( name1.equals("deviceinfo") ) {
                reader.beginObject();
                while ( reader.hasNext() ) {
                    name2 = reader.nextName();
                    if ( name2.equals("model") ) {
                        svalue = reader.nextString();
                        CBLog.d("model="+svalue);
                    } else if ( name2.equals("manufacture") ) {
                        svalue = reader.nextString();
                        CBLog.d("manufacture="+svalue);
                    } else if ( name2.equals("chipset") ) {
                        svalue = reader.nextString();
                        CBLog.d("chipset="+svalue);
                    } else if ( name2.equals("os") ) {
                        svalue = reader.nextString();
                        CBLog.d("os="+svalue);
                    }
                }
                reader.endObject();
            } else if ( name1.equals("performance") ) {
                reader.beginObject();
                while ( reader.hasNext() ) {
                    name2 = reader.nextName();

                    if ( name2.equals("codec_mem_size") ) {
                        ivalue = reader.nextInt();
                        CBLog.d("codec_mem_size="+ivalue);
                    } else if ( name2.equals("max_fps") ) {
                        ivalue = reader.nextInt();
                        CBLog.d("max_fps="+ivalue);
                    } else if ( name2.equals("max_resolution") ) {
                        ivalue = reader.nextInt();
                        CBLog.d("max_resolution="+ivalue);
                    } else if ( name2.equals("mpeg4v_supported") ) {
                        bvalue = reader.nextBoolean();
                        CBLog.d("mpeg4v_supported="+bvalue);
                    } else if ( name2.equals("use_encoder") ) {
                        reader.beginObject();
                        while ( reader.hasNext() ) {
                            name3 = reader.nextName();
                            if ( name3.equals("available_dec_count") ) {
                                count[0] = reader.nextInt();
                                CBLog.d("use_encoder : codec_mem_size="+count[0]);
                            } else if ( name3.equals("realtime_dec_count") ) {
                                count[1] = reader.nextInt();
                                CBLog.d("use_encoder : codec_mem_size="+count[1]);
                            }
                        }
                        reader.endObject();
                    } else if ( name2.equals("unuse_encoder") ) {
                        reader.beginObject();
                        while ( reader.hasNext() ) {
                            name3 = reader.nextName();
                            if ( name3.equals("available_dec_count") ) {
                                count[2] = reader.nextInt();
                                CBLog.d("unuse_encoder : codec_mem_size="+count[2]);
                            } else if ( name3.equals("realtime_dec_count") ) {
                                count[3] = reader.nextInt();
                                CBLog.d("unuse_encoder : codec_mem_size="+count[3]);
                            }
                        }
                        reader.endObject();
                    }
                }
                reader.endObject();
            }
        }
        reader.endObject();

        StringBuffer sb = new StringBuffer();
        sb.append("{\n");
        sb.append("\"value1_in_type1\"" + ": " + count[0] + ",\n");
        sb.append("\"value2_in_type1\"" + ": " + count[1] + ",\n");
        sb.append("\"value1_in_type2\"" + ": " + count[2] + ",\n");
        sb.append("\"value2_in_type2\"" + ": " + count[3] + "\n");
        sb.append("}");
        CBLog.d("Read result file="+sb.toString());
        return new ByteArrayInputStream(sb.toString().getBytes());
    }

    static InputStream readResultFile() {
        String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        File tmpDirectory = new File(sdcardPath+ File.separator+"CodecCapacity");
        if ( !tmpDirectory.exists() ) {
            return null;
        }

        try {
            File fResult = new File(tmpDirectory, "CapabilityResult.txt");
            InputStream in = new BufferedInputStream(new FileInputStream(fResult));
            return in;
        } catch ( NullPointerException |FileNotFoundException e ) {
            e.printStackTrace();
        }
        return null;
    }

    static public void removeResultFile() {
        String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        File tmpDirectory = new File(sdcardPath+ File.separator+"CodecCapacity");
        if ( tmpDirectory.exists() ) {
            File fResult = new File(tmpDirectory, "CapabilityResult.txt");
            if ( fResult!=null ) fResult.delete();
        }
        return;
    }
}
