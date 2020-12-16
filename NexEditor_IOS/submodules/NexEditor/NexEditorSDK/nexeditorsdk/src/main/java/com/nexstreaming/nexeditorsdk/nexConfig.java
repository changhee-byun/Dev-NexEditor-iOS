/******************************************************************************
 * File Name        : nexConfig.java
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

import android.os.Environment;
import com.nexstreaming.app.common.util.CapabilityInfo;
import com.nexstreaming.app.common.util.JsonReaderUtil;
import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;


/**
 * This class manages configuration to set additional settings for the NexEditor&trade;&nbsp;SDK. 
 * 
 * This class should be called before @link com.nexstreaming.nexeditorsdk.nexApplicationConfig#init init@endlink(android.content.Context, String) is called when starting NexEditor SDK. 
 * 
 * @since version 1.3.0
 */
public final class nexConfig {
    /**
     * This key decides whether the NexEditor&trade;&nbsp;SDK should operate with the <tt>nexConfig</tt> settings or default settings of the SDK.
     *
     * Values :<ul>
     *     <li>0 - Operates NexEditor&trade;&nbsp;SDK with default settings.</li>
     *     <li>1 - Operates NexEditor&trade;&nbsp;SDK with settings set with <tt>nexConfig.setProperty</tt>. </li>
     * </ul>
     *
     * @see #setProperty(int, int)
     * @see #getProperty(int)
     * @since version 1.3.0
     */
    public static final int	 kSetUserConfig = 0;

    /**
     * This key sets the memory size of the hardware codec. 
     * 
     * Values : Input memory size value as an <tt>integer</tt>. (ex. 3840*2160*3/2)
     * 
     * Default : 1920*1088*3/2
     *
     * @see #setProperty(int, int)
     * @see #getProperty(int)
     * @since version 1.3.0
     */
    public static final int	 kHardwareCodecMemSize = 1;

    /**
     * This key sets the number of decoders, available at the same time, for the hardware codec.
     * 
     * Values : The number of decoders available at the same time. The maximum value is 2.
     *
     * Default : 2
     *
     * @see #setProperty(int, int)
     * @see #getProperty(int)
     * @since version 1.3.0
     */
    public static final int	 kHardwareDecMaxCount = 2;

    /**
     * This sets the number of FPS available for decoding.
     * 
     * Values: The number of FPS available as an <tt>integer</tt>.
     * 
     * Default : 120
     *
     * @see #setProperty(int, int)
     * @see #getProperty(int)
     * @since version 1.3.0
     */
    public static final int	 kMaxSupportedFPS = 3;

    /**
     * This decides whether the NexEditor&trade;&nbsp;SDK should support MPEG4 if the hardware codec supports MPEG4. 
     *
     * values :<ul>
     *     <li>0 - Do not support MPEG4.</li>
     *     <li>1 - Support MPEG4.</li>
     *         </ul>
     * Default : 0
     *
     * @see #setProperty(int, int)
     * @see #getProperty(int)
     * @since version 1.3.0
     */
    public static final int	 kSupportMPEGV4 = 4;

    /**
     * This sets the number of Max resolution of content.
     *
     * Values: The number of Max resolution available as an <tt>integer</tt>.
     *
     * Default : UHD
     *
     * @see #setProperty(int, int)
     * @see #getProperty(int)
     * @since version 1.5.15
     */
    public static final int kMaxResolution = 5;


    /**
     * This key is decalred to forcefully set the return value of checkDirectExport().
     * values :<ul>
     *     <li>0 - check Direct Export.</li>
     *     <li>1 - must true called .</li>
     *         </ul>
     * Default : 0
     * @since 1.7.7
     */
    public static final int kForceDirectExport = 6;


    /**
     * This key is decalred to print log level
     * Default : 2
     * @since 1.7.57
     */
    public static final int kNativeLogLevel = 7; //0 ~ 5


    /**
     * This key is decalred to TimeChecker
     *
     * values :<ul>
     *     <li>0 - Do not support TimeChecker.</li>
     *     <li>1 - Support TimeChecker.</li>
     *         </ul>
     * Default : 1

     * @since 2.0.0
     */
    public static final int kUseTimeChecker = 8; // 0 or 1

    /**
     * This key is decalred to Device Max Light Level (unit : cd/m2)
     * Default : 550
     * @since 2.0.0
     */
    public static final int kDeviceMaxLightLevel = 9; // 0 or 1

    /**
     * This key is decalred to Device Max Gamma (unit : typeof float Gamma value X 1000 )<br>
     * ex) Gamma = 2.4 --> 2.4 X 1000 = 2400
     * Default : 2400
     * @since 2.0.0
     */
    public static final int kDeviceMaxGamma = 10; // 0 or 1

    /**
     * This key is decalred to device preview tool
     * Default : false
     * @since 2.15.0
     */
    public static final int kIsDevicePreview = 11; // 0 or 1

    static int[] sConfigProperty = {
            0,
            8912896,//hardwareCodecMemSize, 4096*2176
            4,//hardwareDecMaxCount
            120,//maxSupportedFPS
            0,//supportMPEGV4
            8912896,//maxSupportedResolution, 4096*2176
            0,
            2, //loglevel
            1, //timechecker
            550,
            2400,
            0,
    0};

    /**
     * This method sets the basic settings for the NexEditor&trade;&nbsp;SDK.  
     *
     * This should be called before @link com.nexstreaming.nexeditorsdk.nexApplicationConfig#init init@endlink(android.content.Context, String) is called. 
     *
     * <p>Example code:</p>
     *      {@code
                nexConfig.set(3840*2176+1920*1088, 4, 120, false, 3840*2176);
            }
     * @param HWCodecMemSize   The memory size of the hardware codec. The default value is 3840*2176.
     * @param HWDecCount       The number of decoders available at the same time. The default value is 2.
     * @param MaxFPS           The value of FPS. The default value is 120. 
     * @param supportMPEGV4    Whether or not MPEG4 is supported. The default value is 0. 
     * @param MaxResolution    The maximum resolution of the content to be added to the project. The Default setting is UHD(3840x2176). 
     *
     * @see #setProperty(int, int)
     * @see #getProperty(int)
     *
     * @since version 1.3.0
     */
    public static void set( int HWCodecMemSize, int HWDecCount, int MaxFPS, boolean supportMPEGV4, int MaxResolution ){
        sConfigProperty[kSetUserConfig] = 1;
        sConfigProperty[kHardwareCodecMemSize] = HWCodecMemSize;
        sConfigProperty[kHardwareDecMaxCount] = HWDecCount;
        sConfigProperty[kMaxSupportedFPS] = MaxFPS;
        sConfigProperty[kSupportMPEGV4] = supportMPEGV4?1:0;
        sConfigProperty[kMaxResolution] = MaxResolution;
    }

    public static void setCapability(InputStream inputStream)
    {
        String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        File tmpDirectory = new File(sdcardPath+ File.separator+"CodecCapacity");

        File inputFile = new File(tmpDirectory, "CapabilityResult.txt");
        InputStream in = null;
        try {
            in = new BufferedInputStream(new FileInputStream(inputFile));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        JsonReaderUtil jsonReaderUtil = new JsonReaderUtil();
        CapabilityInfo capabilityInfo = null;
        try {
            capabilityInfo = jsonReaderUtil.readCapabilityJson(in);
        } catch (IOException e) {
            e.printStackTrace();
        }


        int HWDecodeMaxCount = (capabilityInfo.getAvailableDecCount1() > capabilityInfo.getRealDecCount2()) ? capabilityInfo.getRealDecCount2() : capabilityInfo.getAvailableDecCount1();

        sConfigProperty[kSetUserConfig] = 1;
        sConfigProperty[kHardwareCodecMemSize] = capabilityInfo.getCodecMemSize();
        sConfigProperty[kHardwareDecMaxCount] = HWDecodeMaxCount;
        sConfigProperty[kMaxSupportedFPS] = capabilityInfo.getMaxFPS();
        sConfigProperty[kSupportMPEGV4] = capabilityInfo.isbSupportMPEGV4()?1:0;
        sConfigProperty[kMaxResolution] = capabilityInfo.getMaxResolution();

    }

    /**
     * This method gets the setting values of the NexEditor&trade;&nbsp;SDK. 
     *
     * <p>Example code:</p>
     *      {@code
                int value = nexConfig.getProperty(1);
            }
     * @param key The value indicating each of the keys ( 1 ~ 5 ).
     * @return The value set to the key.
     * @see #setProperty(int, int)
     * @since version 1.3.0
     */
    public static int getProperty(int key){
        if( sConfigProperty.length <=  key ){
            throw new InvalidRangeException(0,sConfigProperty.length-1,key);
        }
        return sConfigProperty[key];
    }

    /**
     * This method changes the setting value of the NexEditor&trade;&nbsp;SDK. 
     *
     * This method should be called before @link com.nexstreaming.nexeditorsdk.nexApplicationConfig#init init@endlink(android.content.Context, String) is called. 
     * 
     * <p>Example code:</p>
     *      {@code
                nexConfig.setProperty(0,0);
            }
     * @param key    The value indicating each of the keys ( 1 ~ 5 ). 
     * @param value The changed setting value of the key.
     * @return if key value is over property length return <tt>FALSE</tt>, else return <tt>TRUE</tt>.
     * @see #getProperty(int)
     * @since version 1.3.0
     */
    public static boolean setProperty(int key, int value){
        if( sConfigProperty.length <=  key ){
            return false;
        }
        sConfigProperty[kSetUserConfig] = 1;
        sConfigProperty[key] = value;
        return true;
    }
}
