package com.nexstreaming.kminternal.kinemaster.mediainfo;

public class PCMLevels {

	private final byte[] pcmLevels;
	
	PCMLevels( byte[] pcmLevels ) {
		this.pcmLevels = pcmLevels;
	}
	
	public byte[] getLevels(){
		return pcmLevels;
	}
	
}
