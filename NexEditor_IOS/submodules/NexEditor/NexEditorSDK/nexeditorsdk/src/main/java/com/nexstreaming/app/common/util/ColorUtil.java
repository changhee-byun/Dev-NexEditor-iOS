package com.nexstreaming.app.common.util;

public class ColorUtil {

	public static String colorString( int color ) {
		return String.format("#%08X",(color >>> 24) | (color<<8));
	}
	
	public static int parseColor(String colorSpec) {
		
		colorSpec = colorSpec.trim();
		
		if( colorSpec.startsWith("#") ) {
			colorSpec = colorSpec.substring(1).trim();
			if( colorSpec.length()==3 ) {
				long parsed = Long.parseLong(colorSpec,16);
				return ((int)((parsed&0xF)<<4 | (parsed&0xF0)<<8 | (parsed&0xF00)<<12)) | 0xFF000000;
			} else if( colorSpec.length()==6 ) {
				return ((int)Long.parseLong(colorSpec,16)) | 0xFF000000;
			} else if( colorSpec.length()>=8 ) {
				long parsed = Long.parseLong(colorSpec,16);
				return (int)((parsed >> 8) | ((parsed&0xFF) << 24));
			} else {
				return 0;
			}
		}
		
		String[] part = colorSpec.trim().split(" ");
		int v = 0;
		int n = 2;
		for(int i=0; i<part.length; i++) {
			String s = part[i].trim();
			if( s.length()<1 )
				continue;
			int partValue = (int)(Float.parseFloat(s)*255f);
			if( partValue>255 )
				partValue = 255;
			if( partValue < 0 )
				partValue = 0;

			if( n>=0 ) {
				v |= (partValue<<n*8);
				n--;
			} else {
				v |= (partValue<<24);
				break;
			}
			
		}
		
		return v;

	}

}
