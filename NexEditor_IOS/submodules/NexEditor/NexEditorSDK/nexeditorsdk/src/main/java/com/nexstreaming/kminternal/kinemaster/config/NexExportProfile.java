package com.nexstreaming.kminternal.kinemaster.config;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import android.content.res.Resources;
import android.media.CamcorderProfile;

public class NexExportProfile implements Serializable
{
	private static final long	serialVersionUID	= 1L;

	private static final String LOG_TAG = "NexExportProfile";

	public static NexExportProfile[] supportedProfiles() {
		return new NexExportProfile[]{ EXPORT_1080P, EXPORT_720P, EXPORT_960_540, EXPORT_640_360};
	}

	private final int m_width;
	private final int m_height;
	private final int m_displayHeight;
	private final int m_bitrate;
	private final int m_labelResource;

    private final String m_full_hd = "Full HD";
    private final String m_hd = "High Definition";
    private final String m_med_quality = "Medium Quality";
    private final String m_low_quality = "Low Quality";
	// Full HD
//	public static final NexExportProfile EXPORT_1440P 	= new NexExportProfile(2704, 1440, 1440, 20*1024*1024, R.string.dlg_encode_res_full_hd);	//?
	public static final NexExportProfile EXPORT_1080P 	= new NexExportProfile(1920, 1080, 1080, 12*1024*1024, 1);
	public static final NexExportProfile EXPORT_1088P 	= new NexExportProfile(1920, 1088, 1080, 12*1024*1024, 1);
	
	// HD
	public static final NexExportProfile EXPORT_720P 	= new NexExportProfile(1280, 720,  720,  6*1024*1024, 2);
	public static final NexExportProfile EXPORT_736P 	= new NexExportProfile(1280, 736,  720,  6*1024*1024, 2);
	
	// Medium Quality
	public static final NexExportProfile EXPORT_960_540 = new NexExportProfile(960,  540,  540,  3*1024*1024, 3);
	public static final NexExportProfile EXPORT_960_544 = new NexExportProfile(960,  544,  540,  3*1024*1024, 3);
	public static final NexExportProfile EXPORT_800_480 = new NexExportProfile(800,  480,  480,  3*1024*1024/2, 3);
	public static final NexExportProfile EXPORT_864_480 = new NexExportProfile(864,  480,  480,  2*1024*1024, 3);
	public static final NexExportProfile EXPORT_640_480 = new NexExportProfile(640,  480,  480,  3*1024*1024/2, 4);

	// Low Quality
	public static final NexExportProfile EXPORT_640_360 = new NexExportProfile(640,  360,  360,  2*1024*1024, 4);
	public static final NexExportProfile EXPORT_640_368 = new NexExportProfile(640,  368,  360,  2*1024*1024, 4);
	public static final NexExportProfile EXPORT_640_352 = new NexExportProfile(640,  352,  360,  2*1024*1024, 4);
	public static final NexExportProfile EXPORT_400_240 = new NexExportProfile(400,  240,  240,  512*1024, 4);

	// Thumbnail quality
	public static final NexExportProfile EXPORT_320_180 = new NexExportProfile(320,  180,  180,  512*1024, 4);
	public static final NexExportProfile EXPORT_320_192 = new NexExportProfile(320,  192,  180,  512*1024, 4);

	public NexExportProfile( int width, int height, int displayHeight, int bitrate,int labelResource  ) {
		m_width = width;
		m_height = height;
		m_bitrate = bitrate;
		m_displayHeight = displayHeight;
		m_labelResource = labelResource;
	}
	
	public int width() {
		return m_width;
	}
	public int height() {
		return m_height;
	}
	public int displayHeight() {
		return m_displayHeight;
	}
	public int bitrate() {
		return m_bitrate;
	}
	public int labelResource() {
		return m_labelResource;
	}
	public String label( Resources rsrc ) {
		if( rsrc!=null && m_labelResource!=0 ) {
			switch ( m_labelResource  ){
                case 1:
                    return m_full_hd;
                case 2:
                    return m_hd;
                case 3:
                    return m_med_quality;
                case 4:
                    return m_low_quality;

            }
		}
		return m_width + " x " + m_height;
	}

	public static int getLabelResource(int width, int height)
	{
		if( height > 1000 ) {
			return 1;
		} else if( height > 700 ) {
			return 2;
		} else if( height > 450 ) {
			return 3;
		} else {
			return 4;
		}
	}
	
	public static List<NexExportProfile> getExportProfiles( int max_size ) {
		int quality[] = {CamcorderProfile.QUALITY_1080P, CamcorderProfile.QUALITY_720P, CamcorderProfile.QUALITY_480P};
		
		List<NexExportProfile> listProfile = new ArrayList<NexExportProfile>(); 
		
		for( int i = 0; i < quality.length; i++ )
		{
			if( CamcorderProfile.hasProfile(quality[i]) )
			{
				CamcorderProfile profile = CamcorderProfile.get(quality[i]);
				if( profile != null && profile.videoFrameHeight*profile.videoFrameWidth <= max_size )
				{
					NexExportProfile newProfile = new NexExportProfile(		
							profile.videoFrameWidth,
							profile.videoFrameHeight,
							profile.videoFrameHeight,
							profile.videoBitRate,
							getLabelResource(profile.videoFrameWidth, profile.videoFrameHeight));
					listProfile.add(newProfile);
				}
			}
		}
		return listProfile;
	}
	
	public static NexExportProfile getExportProfiles( int iWidth, int iHeight ) {
		NexExportProfile newProfile = null;
		int quality[] = {CamcorderProfile.QUALITY_1080P, CamcorderProfile.QUALITY_720P, CamcorderProfile.QUALITY_480P, CamcorderProfile.QUALITY_CIF, CamcorderProfile.QUALITY_QVGA};
		
		for( int i = 0; i < quality.length; i++ )
		{
			if( CamcorderProfile.hasProfile(quality[i]) )
			{
				CamcorderProfile profile = CamcorderProfile.get(quality[i]);
				if( profile == null ) continue;
				
				int iWidthGap = Math.abs(profile.videoFrameWidth - iWidth);
				int iHeightGap = Math.abs(profile.videoFrameHeight - iHeight);
				
				if( iWidthGap < 32 && iHeightGap < 32 )
				{
					newProfile = new NexExportProfile(		
							profile.videoFrameWidth,
							profile.videoFrameHeight,
							profile.videoFrameHeight,
							profile.videoBitRate,
							getLabelResource(profile.videoFrameWidth, profile.videoFrameHeight));
					break;
				}
			}
		}
		
		if( newProfile == null )
		{
			newProfile = new NexExportProfile(		
					iWidth,
					iHeight,
					iHeight,
					iWidth*iHeight*6,
					getLabelResource(iWidth, iHeight));
		}
		return newProfile;
	}	
}
