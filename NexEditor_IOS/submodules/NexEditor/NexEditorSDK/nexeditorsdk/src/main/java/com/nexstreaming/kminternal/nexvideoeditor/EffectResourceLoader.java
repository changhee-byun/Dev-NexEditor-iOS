package com.nexstreaming.kminternal.nexvideoeditor;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.kminternal.kinemaster.fonts.Font.TypefaceLoadException;

import android.graphics.Typeface;

public interface EffectResourceLoader {
    public AssetPackageReader getReader(String effect_id) throws IOException;
	public File getAssociatedFile(String effect_id, String filename) throws IOException;
	public InputStream openAssociatedFile(String effect_id, String filename) throws IOException;
	public Typeface getTypeface( String typeface_id ) throws TypefaceLoadException;
    public Typeface getTypeface(String effect_id, String filename) throws TypefaceLoadException, IOException;
}
