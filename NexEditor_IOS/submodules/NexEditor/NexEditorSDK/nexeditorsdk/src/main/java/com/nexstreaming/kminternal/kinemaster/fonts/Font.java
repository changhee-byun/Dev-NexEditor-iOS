package com.nexstreaming.kminternal.kinemaster.fonts;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Typeface;
import android.support.annotation.NonNull;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.text.TextUtils;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class Font implements Comparable<Font> {

	private static final int SAMPLE_IMAGE_WIDTH = 1000;
	private static final int SAMPLE_IMAGE_HEIGHT = 100;
	
	private final String m_id;
	private final boolean m_asset;
	private final String m_androidAssetPath;
	private final File m_fontFile;
	private transient WeakReference<Bitmap> m_cachedBitmap;
	private transient WeakReference<Typeface> m_cachedTypeface;
	private final transient Typeface m_systemTypeface;
	private final String m_collectionId;
	private final String m_name;
	private FontName m_fname;

	public Font(String id, String collectionId, File fontFile, String name) {
		m_id = id;
		m_collectionId = collectionId;
		m_asset = false;
		m_systemTypeface = null;
		m_androidAssetPath = null;
		m_fontFile = fontFile;
		m_name = name;
	}
	
	public Font( String id, String collectionId, String assetPath, String name  ) {
		m_id = id;
		m_collectionId = collectionId;
		m_asset = assetPath != null;
		m_androidAssetPath = assetPath;
		m_fontFile = null;
		m_systemTypeface = null;
		m_name = name;
	}
	
	public Font(String id, String collectionId, Typeface systemTypeface, String name) {
		m_id = id;
		m_collectionId = collectionId;
		m_asset = systemTypeface != null;
		m_systemTypeface = systemTypeface;
		m_androidAssetPath = null;
		m_fontFile = null;
		m_name = name;
	}

//	@Deprecated
//	public Font(String id, String collectionId, File sampleFile, File fontFile, EffectLibrary library, String fontRef, String name ) {
//		m_id = id;
//		m_collectionId = collectionId;
//		m_asset = false;
//		m_androidAssetPath = null;
//		m_systemTypeface = null;
//		m_fontFile = fontFile;
//		m_name = name;
//	}

	public String getName( Context ctx ) {
		return m_name;
	}
	
	public static class TypefaceLoadException extends Exception {
		private static final long serialVersionUID = 1L;

		public TypefaceLoadException() {
			super();
		}

		public TypefaceLoadException(String detailMessage, Throwable throwable) {
			super(detailMessage, throwable);
		}

		public TypefaceLoadException(String detailMessage) {
			super(detailMessage);
		}

		public TypefaceLoadException(Throwable throwable) {
			super(throwable);
		}
		
	}
	
	public Typeface getTypeface(Context ctx) throws TypefaceLoadException {
		if( m_cachedTypeface!=null ) {
			Typeface typeface = m_cachedTypeface.get();
			if( typeface!=null ) {
				return typeface;
			}
		}
		if( m_asset ) {
			Typeface typeface = null;
			if( m_systemTypeface!=null ) {
				typeface = m_systemTypeface;
			} else {
				typeface = Typeface.createFromAsset(ctx.getAssets(), m_androidAssetPath);
			}
			m_cachedTypeface = new WeakReference<>(typeface);
			return typeface;
		} else if( m_fontFile!=null ) {
			Typeface typeface;
			try {
				typeface = Typeface.createFromFile(m_fontFile);
			} catch (RuntimeException e) {
				if( e.getMessage().contains("native typeface cannot be made") ) {
					throw new TypefaceLoadException(e);
				} else {
					throw e;
				}
			}
			m_cachedTypeface = new WeakReference<>(typeface);
			return typeface;
		}
		return null;
	}

	public Typeface getTypeface(Context ctx, Typeface defaultTypeface) {
		try {
			return getTypeface(ctx);
		} catch (TypefaceLoadException e) {
			return defaultTypeface;
		}
	}
	
	public Bitmap getSampleImage( Context ctx ) {
		if( m_cachedBitmap != null ) {
			Bitmap cachedBitmap = m_cachedBitmap.get();
			if( cachedBitmap!=null ) {
				return cachedBitmap;
			}
		}

		Bitmap bm = createSampleImage(ctx);
		if( bm==null ) {
			return null;
		}
		
		int pixels[] = new int[bm.getWidth() * bm.getHeight()];
		bm.getPixels(pixels, 0, bm.getWidth(), 0, 0, bm.getWidth(), bm.getHeight());
		for( int i=0; i<pixels.length; i++) {
			pixels[i] = pixels[i] & ((pixels[i] << 8)|0xff);
		}
		Bitmap abm = Bitmap.createBitmap(pixels, bm.getWidth(), bm.getHeight(), Config.ARGB_8888);
		abm.setHasAlpha(true);
		m_cachedBitmap = new WeakReference<>(abm);
		return abm;
	}

	private Bitmap createSampleImage(Context ctx) {
		TextPaint textPaint = new TextPaint();
		textPaint.setTypeface(getTypeface(ctx, Typeface.DEFAULT));
		textPaint.setAntiAlias(true);
		textPaint.setColor(Color.WHITE);
		textPaint.setTextSize(60);

		int margin = (int)(SAMPLE_IMAGE_WIDTH * 0.03f);
        int lineWidth = SAMPLE_IMAGE_WIDTH - margin * 2;
		CharSequence sampleText = TextUtils.ellipsize(m_name, textPaint, lineWidth, TextUtils.TruncateAt.END);
		StaticLayout layout = new StaticLayout(sampleText, textPaint, lineWidth,
				Layout.Alignment.ALIGN_NORMAL, 1.f, 0, true);

		Bitmap bitmap = Bitmap.createBitmap(SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, Config.ARGB_8888);
		Canvas canvas = new Canvas(bitmap);
		canvas.save();
		canvas.translate(margin, (SAMPLE_IMAGE_HEIGHT - layout.getHeight()) / 2);
		layout.draw(canvas);
		canvas.restore();

		return bitmap;
	}

	public boolean isInstalled() {
		if( m_asset ) {
			return true;
		} else {
			if( m_fontFile!=null && m_fontFile.exists() ) {
				return true;
			} else {
				return false;
			}
		}
	}

	public int getSize() {
		if( m_fontFile==null || !m_fontFile.exists() ) {
			return 0;
		} else {
			long size = m_fontFile.length();
			if( size > Integer.MAX_VALUE ) {
				return Integer.MAX_VALUE;
			} else {
				return (int)size;
			}
		}
	}
	
	public String getId() {
		return m_id;
	}

	public String getCollectionId() {
		return m_collectionId;
	}
	
	boolean matchTypefaceSpec( String typefaceSpec ) {
		if( typefaceSpec.startsWith("android:") && m_systemTypeface!=null ) {
			if( typefaceSpec.equalsIgnoreCase("android:Droid Sans") && m_id.equalsIgnoreCase("system.droidsans")) {
				return true;
			}
			if( typefaceSpec.equalsIgnoreCase("android:Droid Sans Bold") && m_id.equalsIgnoreCase("system.droidsansb")) {
				return true;
			}
			if( typefaceSpec.equalsIgnoreCase("android:Droid Serif") && m_id.equalsIgnoreCase("system.droidserif")) {
				return true;
			}
			if( typefaceSpec.equalsIgnoreCase("android:Droid Serif Bold") && m_id.equalsIgnoreCase("system.droidserifb")) {
				return true;
			}
			if( typefaceSpec.equalsIgnoreCase("android:Droid Serif Italic") && m_id.equalsIgnoreCase("system.droidserifi")) {
				return true;
			}
			if( typefaceSpec.equalsIgnoreCase("android:Droid Serif Bold Italic") && m_id.equalsIgnoreCase("system.droidserifbi")) {
				return true;
			}
		}
		if( m_asset && m_androidAssetPath !=null && typefaceSpec.startsWith("asset:")) {
			return typefaceSpec.substring(6).equals(m_androidAssetPath);
		}
		return false;
	}

	private static class Suffix {
		final String text;
		final int weight;

		private Suffix(String text, int weight) {
			this.text = text;
			this.weight = weight;
		}
	}

	private static final Suffix[] suffixes = {
			new Suffix(" thin", 1),
			new Suffix(" light", 2),
			new Suffix(" regular", 3),
			new Suffix(" medium", 4),
			new Suffix(" bold", 5),
			new Suffix(" black", 6),
			new Suffix(" italic", 100),
			new Suffix(" condensed", 1000)
	};

	private static class FontName {

		final String baseName;
		final int sortWeight;
		static final Map<String,FontName> cache = new HashMap<>();

		public static FontName get(String name) {
			FontName fname = cache.get(name);
			if( fname!=null )
				return fname;
			fname = new FontName(name);
			return fname;
		}

		private FontName(String name) {
			boolean found;
			name = name.toLowerCase(Locale.ENGLISH).trim();
			int weight = 0;
			do {
				found = false;
				for( Suffix s: suffixes ) {
					if( name.endsWith(s.text) ) {
						name = name.substring(0,name.length()-s.text.length());
						found = true;
						weight += s.weight;
					}
				}
			} while( found );
			baseName = name;
			sortWeight = weight;
		}
	}

	private FontName getFName() {
		if( m_fname==null ) m_fname = FontName.get(m_name);
		return m_fname;
	}

	@Override
	public int compareTo(@NonNull Font another) {
		FontName a = getFName();
		FontName b = another.getFName();
		int base = a.baseName.compareTo(b.baseName);
		if( base!=0 ) return base;
		return a.sortWeight - b.sortWeight;
	}
	
	@Override
	public String toString() {
		return "[font:" + m_id + ":" + System.identityHashCode(this) + "]";
	}

}
