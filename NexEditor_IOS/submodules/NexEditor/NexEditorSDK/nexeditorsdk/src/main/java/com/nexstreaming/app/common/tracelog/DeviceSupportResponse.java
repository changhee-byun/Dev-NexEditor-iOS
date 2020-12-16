package com.nexstreaming.app.common.tracelog;

import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

public class DeviceSupportResponse implements TLP.BaseResponse {

    public int result;							// (REQUIRED) response code from server
    public int next;							// (REQUIRED) device support information check interval
    public MatchInfo match_info;				// (OPTIONAL) - required if response code is SUCCESS
    public DeviceInfo device_info;				// (OPTIONAL) - required if response code is SUCCESS
    
    @Override
    public String toString() {
    	StringBuilder sb = new StringBuilder();
    	sb.append("DeviceSupportResponse:\n");
    	
    	sb.append("  result:");
    	sb.append(result);
    	sb.append('\n');
    	
    	sb.append("  next:");
    	sb.append(next);
    	sb.append('\n');

    	if( match_info==null ) {
	    	sb.append("  match_info: null\n");
    	} else {
	    	sb.append("  match_info:\n");
	    	sb.append(match_info.toString().replaceAll("(?m)^", "    "));
    	}
    	
    	if( device_info==null ) {
	    	sb.append("  device_info: null\n");
    	} else {
	    	sb.append("  device_info:\n");
	    	sb.append(device_info.toString().replaceAll("(?m)^", "    "));
    	}
    	
    	return sb.toString();
    }
    
    public static class MatchInfo {
    	public Integer record_idx;				// Identifies the matched record (for debugging)
    	public String build_device;				// Matching build_device or 'null' for "don't care" match
    	public String build_model;				// Matching build_model or 'null' for "don't care" match
    	public String board_platform;			// Matching board_platform or 'null' for "don't care" match
    	public String manufacturer;				// Matching manufacturer or 'null' for "don't care" match
    	public int os_api_level_min;			// Matching minimum API level (0 for "don't care" match)
    	public int os_api_level_max;			// Matching maximum API level (>=10000 for "don't care" match)

    	@Override
    	public String toString() {
        	StringBuilder sb = new StringBuilder();
        	sb.append("MatchInfo:\n");
        	
        	sb.append("  record_idx:");
        	sb.append(record_idx);
        	sb.append('\n');
        	
        	sb.append("  build_device:");
        	sb.append(build_device);
        	sb.append('\n');
        	
        	sb.append("  build_model:");
        	sb.append(build_model);
        	sb.append('\n');
        	
        	sb.append("  board_platform:");
        	sb.append(board_platform);
        	sb.append('\n');
        	
        	sb.append("  manufacturer:");
        	sb.append(manufacturer);
        	sb.append('\n');
        	
        	sb.append("  os_api_level_min:");
        	sb.append(os_api_level_min);
        	sb.append('\n');
        	
        	sb.append("  os_api_level_max:");
        	sb.append(os_api_level_max);
        	sb.append('\n');
        	return sb.toString();
        	
    	}
    }
    
    public static class DeviceInfo {
    	
        public int supported;					// Overall device support: 0=not supported, 1=supported (0 overrides all other settings and prevents KineMaster running on this device)
        public int support_avc;					// AVC support level:  0=none, 1=enc, 2=dec, 3=enc+dec
        public int support_mpeg4v;				// MPEG4V support level:  0=none, 1=enc, 2=dec, 3=enc+dec
        public int max_fps;						// maximum supported video frames per second
        public int max_codec_mem_size;			// maximum memory available for all video codec instances combined
        public int max_dec_count;				// maximum number of video decoder instances
        public int max_enc_count;				// maximum number of video encoder instances
        public int max_fhd_trans_time;			// maximum full HD (>=1080p) transition duration in milliseconds, 0x7fffffff/-1 for unlimited
        public int rec_image_mode;				// 0 = Disable, 1 = Native, 2 = KineMaster
        public int rec_video_mode;				// 0 = Disable, 1 = Native, 2 = KineMaster
        public int audio_codec_count;			// max number of simultaneous audio codec instances supported; 0=no audio support; 0x7fffffff/-1 for unlimited
        public int max_sw_import_res;			// max import resolution when using software codec (width * height)
        public int max_hw_import_res;			// max import resolution when using hardware codec (width * height)
        public int max_dec_res_nexsw_b;			// max decoder res for BASELINE profile content when using NEX SW codec (width * height)
        public int max_dec_res_nexsw_m;			// max decoder res for MAIN profile content when using NEX SW codec (width * height)
        public int max_dec_res_nexsw_h;			// max decoder res for HIGH profile content when using NEX SW codec (width * height)
        public int max_dec_res_sw_b;			// max decoder res for BASELINE profile content when using GOOGLE SW codec (width * height)
        public int max_dec_res_sw_m;			// max decoder res for MAIN profile content when using GOOGLE SW codec (width * height)
        public int max_dec_res_sw_h;			// max decoder res for HIGH profile content when using GOOGLE SW codec (width * height)
        public int max_dec_res_hw_b;			// max decoder res for BASELINE profile content when using GOOGLE HW codec (width * height)
        public int max_dec_res_hw_m;			// max decoder res for MAIN profile content when using GOOGLE HW codec (width * height)
        public int max_dec_res_hw_h;			// max decoder res for HIGH profile content when using GOOGLE HW codec (width * height)

        public List<ExportResInfo> export_res_sw;
        public List<ExportResInfo> export_res_hw;
        public List<ExportResInfo> export_res_extra;
        
        public Map<String,String> properties;	// custom properties for this device
        
        @Override
        public String toString() {
        	StringBuilder sb = new StringBuilder();
        	sb.append("DeviceSupportResponse:\n");
        	
        	sb.append("  support_avc:");
        	sb.append(support_avc);
        	sb.append('\n');
        	
        	sb.append("  support_mpeg4v:");
        	sb.append(support_mpeg4v);
        	sb.append('\n');
        	
        	sb.append("  max_fps:");
        	sb.append(max_fps);
        	sb.append('\n');
        	
        	sb.append("  max_codec_mem_size:");
        	sb.append(max_codec_mem_size);
        	sb.append('\n');
        	
        	sb.append("  max_dec_count:");
        	sb.append(max_dec_count);
        	sb.append('\n');
        	
        	sb.append("  max_enc_count:");
        	sb.append(max_enc_count);
        	sb.append('\n');
        	
        	sb.append("  max_fhd_trans_time:");
        	sb.append(max_fhd_trans_time);
        	sb.append('\n');
        	
        	sb.append("  rec_image_mode:");
        	sb.append(rec_image_mode);
        	sb.append('\n');
        	
        	sb.append("  rec_video_mode:");
        	sb.append(rec_video_mode);
        	sb.append('\n');
        	
        	sb.append("  audio_codec_count:");
        	sb.append(audio_codec_count);
        	sb.append('\n');
        	
        	sb.append("  max_sw_import_res:");
        	sb.append(max_sw_import_res);
        	sb.append('\n');
        	
        	sb.append("  max_hw_import_res:");
        	sb.append(max_hw_import_res);
        	sb.append('\n');
        	
        	sb.append("  max_dec_res_nexsw_b:");
        	sb.append(max_dec_res_nexsw_b);
        	sb.append('\n');

        	sb.append("  max_dec_res_nexsw_b:");
        	sb.append(max_dec_res_nexsw_b);
        	sb.append('\n');

        	sb.append("  max_dec_res_nexsw_h:");
        	sb.append(max_dec_res_nexsw_h);
        	sb.append('\n');

        	sb.append("  max_dec_res_sw_b:");
        	sb.append(max_dec_res_sw_b);
        	sb.append('\n');

        	sb.append("  max_dec_res_sw_m:");
        	sb.append(max_dec_res_sw_m);
        	sb.append('\n');

        	sb.append("  max_dec_res_sw_h:");
        	sb.append(max_dec_res_sw_h);
        	sb.append('\n');

        	sb.append("  max_dec_res_hw_b:");
        	sb.append(max_dec_res_hw_b);
        	sb.append('\n');

        	sb.append("  max_dec_res_hw_m:");
        	sb.append(max_dec_res_hw_m);
        	sb.append('\n');

        	sb.append("  max_dec_res_hw_h:");
        	sb.append(max_dec_res_hw_h);
        	sb.append('\n');
        	
        	int n;
        	
        	if( export_res_sw==null ) {
	        	sb.append("  export_res_sw: null\n");
        	} else {
	        	sb.append("  export_res_sw:\n");
	        	n = 0;
	        	for( ExportResInfo resinfo: export_res_sw ) {
	        		sb.append("    [" + (n++) + "] " + resinfo.toString() + "\n");
	        	}
        	}
        	
        	if( export_res_hw==null ) {
	        	sb.append("  export_res_hw: null\n");
        	} else {
	        	sb.append("  export_res_hw:\n");
	        	n = 0;
	        	for( ExportResInfo resinfo: export_res_hw ) {
	        		sb.append("    [" + (n++) + "] " + resinfo.toString() + "\n");
	        	}
        	}
	        	
        	if( export_res_extra==null ) {
	        	sb.append("  export_res_extra: null\n");
        	} else {
	        	sb.append("  export_res_extra:\n");
	        	n = 0;
	        	for( ExportResInfo resinfo: export_res_extra ) {
	        		sb.append("    [" + (n++) + "] " + resinfo.toString() + "\n");
	        	}
        	}
        	
        	if( properties==null ) {
        		sb.append("  properties: null\n");
        	} else {
        		sb.append("  properties:\n");
        		for( Entry<String, String> kv: properties.entrySet() ) {
        			sb.append("    " + kv.getKey() + "=" + kv.getValue());
        		}
        	}
        	
        	
        	return sb.toString();
        }
        

    }
    
    public static class ExportResInfo {
        public int width; 						// actual codec export width
        public int height; 						// actual codec export height
        public int display_height; 				// export display high (may be different from height if the video needs to be cropped)
        public int bitrate; 					// export bitrate
        
    	@Override
    	public String toString() {
        	return "<ExportResInfo " + width + "x" + height + " disp=" + display_height + " bitrate=" + bitrate + ">";
        	
    	}
        
    }
    
	public DeviceSupportResponse() {
	}

	@Override
	public int getResult() {
		return result;
	}

}
