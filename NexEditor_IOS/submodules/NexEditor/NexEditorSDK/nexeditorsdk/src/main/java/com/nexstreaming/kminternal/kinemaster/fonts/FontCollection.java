package com.nexstreaming.kminternal.kinemaster.fonts;

import java.util.List;

import android.content.Context;

public interface FontCollection {
	
	String getName( Context ctx );
	List<Font> getFonts();
	String getId();
	
}
