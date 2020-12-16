package com.nexstreaming.kminternal.nexvideoeditor;

import java.util.Vector;

import android.media.CamcorderProfile;

import com.nexstreaming.kminternal.kinemaster.config.NexEditorDeviceProfile;

public class NexVisualClipChecker {
	private static final String LOG_TAG = "NexAVCChecker.java";

    private int m_MCHWAVCDecBaselineLevel = 0;
    private int m_MCHWAVCDecMainLevel = 0;
    private int m_MCHWAVCDecHighLevel = 0;

    private int m_MCHWAVCDecBaselineSize = 0;
    private int m_MCHWAVCDecMainSize = 0;
    private int m_MCHWAVCDecHighSize = 0;

    private int m_MCHWAVCEncBaselineSize = 0;
    private int m_MCHWAVCEncMainSize = 0;
    private int m_MCHWAVCEncHighSize = 0;

    private boolean m_canUseMCSWCodec = false;
    
    private int m_MCSWAVCDecBaselineSize = 0;
    private int m_MCSWAVCDecMainSize = 0;
    private int m_MCSWAVCDecHighSize = 0;
    
    private boolean m_canUseNXSWCodec = false;
    
    private int m_NXSWAVCDecBaselineSize = 0;
    private int m_NXSWAVCDecMainSize = 0;
    private int m_NXSWAVCDecHighSize = 0;
    
    private int m_iMaxFPS = 55;
    private int m_iMaxSWImportSize = 0;
    private int m_iMaxHWImportSize = 0;
    
    private int m_iVideoBitRate = 0;
    private int m_iAudioSamplingRate = 0;
    private int m_iAudioChannels = 0;

    private Vector<CamcorderProfile> m_vecCamcorderProfile;

    private static int minNotZero(int a, int b) {
        if( a == 0 )
            return b;
        if( b == 0 )
            return a;
        if( a < b )
            return a;
        else
            return b;
    }

    NexVisualClipChecker(NexEditor editor)
    {
    	if( editor == null ) return;
    	
    	NexEditorDeviceProfile dp = NexEditorDeviceProfile.getDeviceProfile();
    	int maxSizeForUnknownDevice = dp.getMaxCamcorderProfileSizeForUnknownDevice();
    	if( maxSizeForUnknownDevice<=0 )
    		maxSizeForUnknownDevice = Integer.MAX_VALUE;

        m_MCHWAVCDecBaselineLevel = dp.getIntProperty("Device_Support_BaselineMaxLevel", 0);
        m_MCHWAVCDecMainLevel = dp.getIntProperty("Device_Support_MainMaxLevel", 0);
        m_MCHWAVCDecHighLevel = dp.getIntProperty("Device_Support_HighMaxLevel", 0);
    	
	    m_MCHWAVCDecBaselineSize = dp.getMCHWAVCDecBaselineSize();// editor.getPropertyInt("MCHWAVCDecBaselineLevelSize", 0);
	    m_MCHWAVCDecMainSize = dp.getMCHWAVCDecMainSize(); //editor.getPropertyInt("MCHWAVCDecMainLevelSize", 0);
	    m_MCHWAVCDecHighSize = dp.getMCHWAVCDecHighSize(); //editor.getPropertyInt("MCHWAVCDecHighLevelSize", 0);

	    if( dp.isUnknownDevice() ) {
            m_MCHWAVCEncBaselineSize = minNotZero(maxSizeForUnknownDevice, editor.getPropertyInt("MCHWAVCEncBaselineLevelSize", 0));
		    m_MCHWAVCEncMainSize = Math.min(maxSizeForUnknownDevice,editor.getPropertyInt("MCHWAVCEncMainLevelSize", 0));
		    m_MCHWAVCEncHighSize = Math.min(maxSizeForUnknownDevice,editor.getPropertyInt("MCHWAVCEncHighLevelSize", 0));
	    } else {
		    m_MCHWAVCEncBaselineSize = editor.getPropertyInt("MCHWAVCEncBaselineLevelSize", 0);
		    m_MCHWAVCEncMainSize = editor.getPropertyInt("MCHWAVCEncMainLevelSize", 0);
		    m_MCHWAVCEncHighSize = editor.getPropertyInt("MCHWAVCEncHighLevelSize", 0);
	    }
	    
	    m_canUseMCSWCodec = editor.getPropertyBool("canUseMCSoftwareCodec", false);
	    if( m_canUseMCSWCodec )
	    {
		    m_MCSWAVCDecBaselineSize = dp.getMCSWAVCDecBaselineSize();// editor.getPropertyInt("MCSWAVCDecBaselineLevelSize", 0);
		    m_MCSWAVCDecMainSize = dp.getMCSWAVCDecMainSize(); //editor.getPropertyInt("MCSWAVCDecMainLevelSize", 0);
		    m_MCSWAVCDecHighSize = dp.getMCSWAVCDecHighSize(); //editor.getPropertyInt("MCSWAVCDecHighLevelSize", 0);
	    }
	    
	    m_canUseNXSWCodec = editor.getPropertyBool("canUseSoftwareCodec", false);
	    if( m_canUseNXSWCodec )
	    {
	    	m_NXSWAVCDecBaselineSize = dp.getNXSWAVCDecBaselineSize();// editor.getPropertyInt("NXSWAVCDecBaselineLevelSize", 0);
	    	m_NXSWAVCDecMainSize = dp.getNXSWAVCDecMainSize();// editor.getPropertyInt("NXSWAVCDecMainLevelSize", 0);
	    	m_NXSWAVCDecHighSize = dp.getNXSWAVCDecHighSize();//editor.getPropertyInt("NXSWAVCDecHighLevelSize", 0);
	    }
	    
	    m_iMaxFPS = dp.getMaxSupportedFPS();
	    
        m_iVideoBitRate = dp.getMaxSupportedVideoBitrate(0);
        m_iAudioSamplingRate = dp.getMaxSupportedAudioSamplingRate(0);
        m_iAudioChannels = dp.getMaxSupportedAudioChannels(0);

	    if( dp.isUnknownDevice())
	    {
	    	m_iMaxSWImportSize = m_MCHWAVCDecBaselineSize;
	    	m_iMaxHWImportSize = m_MCHWAVCDecBaselineSize;
	    }
	    else
	    {
		    m_iMaxSWImportSize = dp.getMaxImportSize(true);
		    m_iMaxHWImportSize = dp.getMaxImportSize(false);;
	    }
	    
	    m_vecCamcorderProfile = new Vector<CamcorderProfile>();
		int quality[] = {CamcorderProfile.QUALITY_1080P, CamcorderProfile.QUALITY_720P, CamcorderProfile.QUALITY_480P, CamcorderProfile.QUALITY_CIF, CamcorderProfile.QUALITY_QVGA};
		for(int i = 0; i < quality.length; i++ )
		{
			if( CamcorderProfile.hasProfile(quality[i]) )
			{
				m_vecCamcorderProfile.add( CamcorderProfile.get(quality[i]) );
			}
		}
    }

	public enum Profile {
		BASELINE, MAIN, HIGH
	}

	public int getMaxDecSize(Profile profile) {

		int iBaseSize = 0;
		int iMainSize = 0;
		int iHighSize = 0;

		if( m_canUseNXSWCodec )
		{
			iBaseSize = m_NXSWAVCDecBaselineSize;
			iMainSize = m_NXSWAVCDecMainSize;
			iHighSize = m_NXSWAVCDecHighSize;
		}
		else
		{
			iBaseSize = m_MCHWAVCDecBaselineSize;
			iMainSize = m_MCHWAVCDecMainSize;
			iHighSize = m_MCHWAVCDecHighSize;
		}

		int iCodecSupportSize = 0;
		switch(profile)
		{
			case BASELINE: // Baseline profile
				iCodecSupportSize = iBaseSize;
				break;
			case MAIN: // Main profile
				iCodecSupportSize = iMainSize;
				break;
			case HIGH: // High profile
				iCodecSupportSize = iHighSize;
				break;
		};

		if( m_canUseMCSWCodec ) {
			if (profile == Profile.BASELINE && iCodecSupportSize < m_MCSWAVCDecBaselineSize) {
				iCodecSupportSize = m_MCSWAVCDecBaselineSize;
			}
		}

		return iCodecSupportSize;
	}
    
	/** Check support or not
	 *  return values
	 *   0 : Supported
	 *   1 : Not supported but supported after resolution transcoding.
	 *   2 : Not supported but supported after FPS transcoding.
	 *   3 : Not supported (profile not supported)
	 *   4 : Not supported (resolution over HW codec max resolution)
	 *   5 : Not supported (other) 
     *   6 : Not supported audio samplingrate
     *   7 : Not supported but supported after bitrate transcoding.
	 */
    public int checkSupportedClip(int iProfile, int iLevel, int iWidth, int iHeight, int iFPS, int iVideoBitrate, int iAudioSamplingRate, int iAudioChannels)
    {
    	// TODO: Check for iProfile -1, iLevel -1 -> return 6 and show popup in MediaBrowser (cannot determine if this clip is supported, download anyway?)

        if( m_iAudioSamplingRate > 0 && m_iAudioSamplingRate < iAudioSamplingRate )
            return 6;

    	int iMaxImportSize = m_canUseNXSWCodec ? m_iMaxSWImportSize : m_iMaxHWImportSize;
    	if( iProfile == 255 )
    	{
    		if( iWidth * iHeight > iMaxImportSize )
    		{
                	int iMaxDecSize = m_canUseNXSWCodec ? m_NXSWAVCDecBaselineSize : m_MCHWAVCDecBaselineSize;

                	if( iWidth * iHeight > iMaxDecSize )
                	{
                	    return 4;
                	}
    			return 1;
    		}
    		
    		if( iFPS > m_iMaxFPS )
    			return 2;

            /*
            if( m_iVideoBitRate > 0 && m_iVideoBitRate < iVideoBitrate )
            {
                return 7;
            }
            */
    		return 0;
    	}
    	
    	int iBaseSize = 0;
    	int iMainSize = 0;
    	int iHighSize = 0;
    	
    	if( m_canUseNXSWCodec )
    	{
        	iBaseSize = m_NXSWAVCDecBaselineSize;
        	iMainSize = m_NXSWAVCDecMainSize;
        	iHighSize = m_NXSWAVCDecHighSize;
    	}
    	else
    	{
        	iBaseSize = m_MCHWAVCDecBaselineSize;
        	iMainSize = m_MCHWAVCDecMainSize;
        	iHighSize = m_MCHWAVCDecHighSize;
    	}

    	int iCodecSupportSize = 0;
    	switch(iProfile)
    	{
	    	case 66: // Baseline profile
                if( m_MCHWAVCDecBaselineLevel != 0 && m_MCHWAVCDecBaselineLevel < iLevel )
                    return 5;
	    		iCodecSupportSize = iBaseSize;
	    		break;
	    	case 77: // Main profile
	    		if( iMainSize == 0 )
	    			return 3;
                if( m_MCHWAVCDecMainLevel != 0 && m_MCHWAVCDecMainLevel < iLevel )
                    return 5;
	    		iCodecSupportSize = iMainSize;
	    		break;
	    	case 100: // High profile
	    		if( iHighSize == 0 )
	    			return 3;
                if( m_MCHWAVCDecHighLevel != 0 && m_MCHWAVCDecHighLevel < iLevel )
                    return 5;
	    		iCodecSupportSize = iHighSize;
	    		break;
	    	default:
	    		return 3;
    	};
    	
    	if( iWidth * iHeight > iCodecSupportSize )
		{
        	if( m_canUseMCSWCodec )
        	{
        		if( iProfile == 66 && iWidth*iHeight <= m_MCSWAVCDecBaselineSize )
        		{
        			return 1;
        		}
        		else
        		{
        			return 3;
        		}
        		
        	}    		
    		return 4;
		}
    	
    	if( iWidth * iHeight > iMaxImportSize )
		{
    		return 1;
		}
    	
		if( iFPS > m_iMaxFPS )
    	{
    		/*
    		if( iWidth * iHeight < 720 * 480 )
    			return 0;
    		*/
			return 2;
    	}

        /*
        if( m_iVideoBitRate > 0 && m_iVideoBitRate < iVideoBitrate )
        {
            return 7;
        }
        */
		return 0;    	
    }
  
    public int getMaxImportSizeWithHWDecoder()
    {
    	int importSize = m_MCHWAVCDecBaselineSize;
    	importSize = m_MCHWAVCDecMainSize > 0 ? Math.min(importSize, m_MCHWAVCDecMainSize) : importSize;
    	importSize = m_MCHWAVCDecHighSize > 0 ? Math.min(importSize, m_MCHWAVCDecHighSize) : importSize;
    	return importSize;
    }    
    
    public int getMaxImportSize(boolean bSoft)
    {
    	return bSoft ? m_iMaxSWImportSize : m_iMaxHWImportSize;
    }        
  
    public int getMaxHardwareAVCDecBaselineSize()
    {
    	return m_MCHWAVCDecBaselineSize;
    }
    
    public int getMaxHardwareAVCDecMainSize()
    {
    	return m_MCHWAVCDecMainSize;
    }
    
    public int getMaxHardwareAVCDecHighSize()
    {
    	return m_MCHWAVCDecHighSize;
    }
    
    public int getMaxHardwareAVCEncBaselineSize()
    {
    	return m_MCHWAVCEncBaselineSize;
    }
    
    public int getMaxHardwareAVCEncMainSize()
    {
    	return m_MCHWAVCEncMainSize;
    }
    
    public int getMaxHardwareAVCEncHighSize()
    {
    	return m_MCHWAVCEncHighSize;
    }    
    
    public int getMaxMCSoftwareAVCDecBaselineSize()
    {
    	return m_MCSWAVCDecBaselineSize;
    }
    
    public int getMaxMCSoftwareAVCDecMainSize()
    {
    	return m_MCSWAVCDecMainSize;
    }
    
    public int getMaxMCSoftwareAVCDecHighSize()
    {
    	return m_MCSWAVCDecHighSize;
    }    
    
    public int getMaxNXSoftwareAVCDecBaselineSize()
    {
    	return m_NXSWAVCDecBaselineSize;
    }
    
    public int getMaxNXSoftwareAVCDecMainSize()
    {
    	return m_NXSWAVCDecMainSize;
    }
    
    public int getMaxNXSoftwareAVCDecHighSize()
    {
    	return m_NXSWAVCDecHighSize;
    }       
}
