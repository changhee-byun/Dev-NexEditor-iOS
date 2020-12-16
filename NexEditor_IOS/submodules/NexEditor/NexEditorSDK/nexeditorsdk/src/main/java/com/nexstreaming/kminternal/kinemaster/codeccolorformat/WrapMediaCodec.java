package com.nexstreaming.kminternal.kinemaster.codeccolorformat;


import com.nexstreaming.kminternal.kinemaster.config.LL;

class WrapMediaCodec {

	public static final int INFO_FRAME_TYPE_VIDEO			= 0;
	public static final int INFO_FRAME_TYPE_AUDIO			= 1;
	public static final int INFO_FRAME_TYPE_CONFIG			= 2;
	
	public static final int INFO_OUTPUT_BUFFERS_CHANGED		= 1;
	public static final int INFO_OUTPUT_FORMAT_CHANGED		= 2;
	public static final int INFO_TRY_AGAIN_LATER			= 3;
	public static final int BUFFER_FLAG_END_OF_STREAM		= 4;
	public static final int BUFFER_FLAG_CODEC_CONFIG		= 5;
	
	public static String flagToString(int iFlag)
	{
		if( LL.I ) {
			switch(iFlag)
			{
				case INFO_OUTPUT_BUFFERS_CHANGED:
					return "INFO_OUTPUT_BUFFERS_CHANGED";
					
				case INFO_OUTPUT_FORMAT_CHANGED:
					return "INFO_OUTPUT_FORMAT_CHANGED";
					
				case INFO_TRY_AGAIN_LATER:
					return "INFO_TRY_AGAIN_LATER";
					
				case BUFFER_FLAG_END_OF_STREAM:
					return "BUFFER_FLAG_END_OF_STREAM";
					
				case BUFFER_FLAG_CODEC_CONFIG:
					return "BUFFER_FLAG_CODEC_CONFIG";
					
				default:
					break;
			}
			return String.format("UNKNOWN");
		} else {
			return null;
		}
	}

}
