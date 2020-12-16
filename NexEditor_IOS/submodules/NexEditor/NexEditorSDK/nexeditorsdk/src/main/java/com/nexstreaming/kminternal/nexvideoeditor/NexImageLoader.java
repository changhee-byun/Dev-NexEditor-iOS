package com.nexstreaming.kminternal.nexvideoeditor;

import android.content.ContentUris;
import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BlurMaskFilter;
import android.graphics.BlurMaskFilter.Blur;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Paint.FontMetrics;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.media.ExifInterface;
import android.net.Uri;
import android.provider.MediaStore;
import android.text.Layout.Alignment;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.text.TextUtils;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.fonts.Font;
import com.nexstreaming.kminternal.kinemaster.fonts.Font.TypefaceLoadException;
import com.nexstreaming.kminternal.kinemaster.fonts.FontManager;
import java.io.BufferedInputStream;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.WeakHashMap;

/**
 * Provides information on image.
 * method for details.
 * 
 * @author Nextreaming Corporation
 *
 */

public final class NexImageLoader
{
	private static final String LOG_TAG = "NexImageLoader";
//	private Bitmap mBitmap;
//    private int mWidth;
//    private int mHeight;
//    private boolean m_premult = false;
    private AssetManager m_assetManager;
//    private Resources m_rsrc; yoon delete
//    private NexThemeManagerWrapper m_themeManagerWrapper;
    private OverlayPathResolver m_overlayPathResolver;
    private EffectResourceLoader m_effectResourceLoader;
    private int m_jpegMaxWidth;
    private int m_jpegMaxHeight;
    private int m_jpegMaxSize;
    
    private static final String TAG_Text = "[Text]";
    private static final String TAG_Overlay = "[Overlay]";
    private static final String TAG_ThemeImage = "[ThemeImage]";
    private static final String TAG_PreviewThemeImage = "[PvwThImage]";
    private static final String TYPEFACE_SYSTEM = "android:";
    private static final String TYPEFACE_ASSET = "asset:";
    private static final String TYPEFACE_THEME = "theme:";
    private static final String TYPEFACE_FONTID = "fontid:";
    private static final String TYPEFACE_FONTFILE = "fontfile:";
    private static final String TYPEFACE_FILE = "file:";
    
    private static final int NXT_LONGTEXT_CROP_END           = 0;
    private static final int NXT_LONGTEXT_WRAP               = 1;
    private static final int NXT_LONGTEXT_ELLIPSIZE_START    = 2;
    private static final int NXT_LONGTEXT_ELLIPSIZE_MIDDLE   = 3;
    private static final int NXT_LONGTEXT_ELLIPSIZE_END      = 4;
    
	private static final int NXT_TEXTFLAG_BOLD       = 0x00000001;
	private static final int NXT_TEXTFLAG_ITALIC     = 0x00000002;
	private static final int NXT_TEXTFLAG_FILL       = 0x00000004;
	private static final int NXT_TEXTFLAG_STROKE     = 0x00000008;
	private static final int NXT_TEXTFLAG_UNDERLINE  = 0x00000010;
	private static final int NXT_TEXTFLAG_STRIKE     = 0x00000020;
	//private static final int NXT_TEXTFLAG_HINTING    = 0x00000040;
	private static final int NXT_TEXTFLAG_SUBPIXEL   = 0x00000080;
	private static final int NXT_TEXTFLAG_SHADOW     = 0x00000100;
	private static final int NXT_TEXTFLAG_LINEAR     = 0x00000200;
	private static final int NXT_TEXTFLAG_AUTOSIZE   = 0x00000400;
	private static final int NXT_TEXTFLAG_CUTOUT     = 0x00000800;
	private static final int NXT_TEXTFLAG_STROKEBACK = 0x00001000;

	private static final int NXT_ALIGN_LEFT          = 0x00;
	private static final int NXT_ALIGN_CENTER        = 0x01;
	private static final int NXT_ALIGN_RIGHT         = 0x02;
	private static final int NXT_ALIGNMASK       	 = 0x0F;
	private static final int NXT_VALIGN_TOP          = 0x00;
	private static final int NXT_VALIGN_CENTER       = 0x10;
	private static final int NXT_VALIGN_BOTTOM       = 0x20;
	private static final int NXT_VALIGNMASK		 	 = 0xF0;
	
	private static final int NXT_BLUR_NORMAL		 = 0;
	private static final int NXT_BLUR_SOLID			 = 1;
	private static final int NXT_BLUR_INNER			 = 2;
	private static final int NXT_BLUR_OUTER			 = 3;

	private static final int MAX_USERSTRINGS = 3;
	
	public static abstract class OverlayPathResolver {
		public abstract String resolveOverlayPath( String overlayPath );
	}
	
	public NexImageLoader( Resources res, EffectResourceLoader effectResourceLoader, OverlayPathResolver overlayPathResolver, int jpegMaxWidth, int jpegMaxHeight, int jpegMaxSize ) {
		if( res==null ) {
//			m_rsrc = null; yoon delete
			m_assetManager = null;
		} else {
//			m_rsrc = res; yoon delete
            m_assetManager = res.getAssets();
		}
		m_effectResourceLoader = effectResourceLoader;
		m_overlayPathResolver = overlayPathResolver;
		m_jpegMaxWidth = jpegMaxWidth;
		m_jpegMaxHeight = jpegMaxHeight;
		m_jpegMaxSize = jpegMaxSize;
	}

//	public NexImage( Resources res, final NexThemeManagerWrapper themeManagerWrapper, OverlayPathResolver overlayPathResolver ) {
//		if( res==null ) {
//			m_rsrc = null;
//			m_assetManager = null;
//		} else {
//			m_rsrc = res;
//			m_assetManager = res.getAssets();
//		}
//		m_effectResourceLoader = new EffectResourceLoader() {
//			
//			@Override
//			public InputStream openAssociatedFile(String effect_id, String filename)
//					throws IOException {
//
//        		if( themeManagerWrapper!=null && themeManagerWrapper.getThemeManager()!=null ) {
//            		NexThemeManager.NexFXItem item = null;
//            		
//            		if( item==null )
//            			item = themeManagerWrapper.getThemeManager().getEffect(effect_id);
//            		if( item==null )
//            			item = themeManagerWrapper.getThemeManager().getTransition(effect_id);
//            		if( item==null )
//            			item = themeManagerWrapper.getThemeManager().getTheme(effect_id);
//            		
//            		if( item!=null && m_assetManager!=null) {
//                		return item.openAssociatedFile(m_assetManager, filename);
//            		}
//        		} 
//
//        		return null;
//			}
//			
//			@Override
//			public File getAssociatedFile(String effect_id, String filename)
//					throws IOException {
//
//        		if( themeManagerWrapper!=null && themeManagerWrapper.getThemeManager()!=null ) {
//            		NexThemeManager.NexFXItem item = null;
//            		
//            		if( item==null )
//            			item = themeManagerWrapper.getThemeManager().getEffect(effect_id);
//            		if( item==null )
//            			item = themeManagerWrapper.getThemeManager().getTransition(effect_id);
//            		if( item==null )
//            			item = themeManagerWrapper.getThemeManager().getTheme(effect_id);
//            		
//            		if( item!=null && m_assetManager!=null) {
//                		return item.getAssociatedFile(m_assetManager, filename);
//            		}
//        		} 
//				
//				return null;
//			}
//		};
//		m_overlayPathResolver = overlayPathResolver;
//	}
	
	public void setResources( Resources res ) {
		if( res==null ) {
//			m_rsrc = null; yoon delete
			m_assetManager = null;
		} else {
//			m_rsrc = res; yoon delete
            m_assetManager = res.getAssets();
		}
	}
	
	private String pdecode(String s) {
		final String hexchars = "0123456789ABCDEF";
		int findpos = -1;
		StringBuilder b = new StringBuilder(s);
		while( (findpos=b.indexOf("%",findpos+1)) != -1 ) {
			if( findpos+2 >= b.length() )
				break;
			int c1 = hexchars.indexOf(s.charAt(findpos+1));
			int c2 = hexchars.indexOf(s.charAt(findpos+2));
			if( c1 != -1 && c2 != -1 ) {
				b.setCharAt(findpos, (char)((c1<<4)|c2));
				b.delete(findpos+1, findpos+3);
				s = b.toString();
			}
		}
		return b.toString();
	}

	public byte[] openThemeFile(String path){

		String untagged_path = path;
		String base_id;
		String rel_path;

		int delim_idx = untagged_path.indexOf('/');
		if( delim_idx>=0 ) {
			base_id = untagged_path.substring(0,delim_idx);
			rel_path = untagged_path.substring(delim_idx+1);
		} else {
			base_id = "";
			rel_path = untagged_path;
		}

		if( m_effectResourceLoader!=null ) {

			try {
                AssetPackageReader reader = m_effectResourceLoader.getReader(base_id);
				File f = m_effectResourceLoader.getAssociatedFile(base_id, rel_path);
				if(f.exists()){

					InputStream in = m_effectResourceLoader.openAssociatedFile(base_id, rel_path);
					Log.e(LOG_TAG, "get size begin loading bitmap for effect(" + base_id + ") : " + rel_path);
					int size = 0;
					int tmp_read = 0;
					while((tmp_read = (int)in.skip(Integer.MAX_VALUE)) > 0){

						size += tmp_read;
					}
					in.close();
					in = m_effectResourceLoader.openAssociatedFile(base_id, rel_path);
					Log.e(LOG_TAG, "get size end loading bitmap for effect(" + base_id + ") : " + rel_path + " size=" + size);
					byte[] temp_result = new byte[size];
					int read_size = 0;
					int read_point = 0;
					int to_read = size;
					while(-1 != (read_size = in.read(temp_result, read_point, to_read))) {
						read_point += read_size;
						to_read = size - read_point;
						if(to_read <= 0)
							break;
					}

					return temp_result;
				}
			} catch (IOException e) {

				if(LL.E) Log.e(LOG_TAG,"Error loading bitmap for effect(" + base_id + ") : " + rel_path);
				e.printStackTrace();
			}

			return null;
		}

		return null;
	}


    public NexImage openThemeImage(String path)
    {
    	Bitmap resultBitmap = null;
    	int resultWidth = 0;
    	int resultHeight = 0;

        if( path.contains(".force_effect/"))
            path = path.replace(".force_effect/", "/");

    	if ( path.startsWith(TAG_ThemeImage + "/@special:") || path.startsWith(TAG_PreviewThemeImage + "/@special:")) {
    		/*if(path.equals( TAG_ThemeImage + "/@special:watermark.jpg") || path.equals( TAG_PreviewThemeImage + "/@special:watermark.jpg")) {
    			if(!EditorGlobal.WATERMARK_LOGO) {
    				mBitmap = null;
    				return 1;
    			}
    			mBitmap = BitmapFactory.decodeResource(m_rsrc, R.drawable.km_watermark);
	            mWidth	= mBitmap.getWidth();
	            mWidth	= mWidth & 0xFFFFFFFE;
	            mHeight	= mBitmap.getHeight();
	            mHeight	= mHeight & 0xFFFFFFFE;
    	    	if(LL.D) Log.d(LOG_TAG, "decoded watermark image('" + mBitmap.getWidth() + " x " + mBitmap.getHeight() + "')");
    			if( mBitmap==null ) {
    				return 1;
    			}
        		return 0;
    		} else {*/
    	    return null;
//    		}

    	} else if( path.startsWith(TAG_Text) ) {
    		
    		String parseStr = path.substring(TAG_Text.length());
    		int parseStrLen = parseStr.indexOf(";;");
    		String wholeUserText = parseStr.substring(parseStrLen+2);//.trim();
    		/*if(wholeUserText.contains("@")) {
    			wholeUserText = wholeUserText.replace("@", "@");
    			if(LL.D) Log.d(LOG_TAG, "ASCII value of @ : " + (int)wholeUserText.charAt(0) + (int)wholeUserText.charAt(1)); 
    		//	wholeUserText.replace("\", ", "\",");
    		}*/
    		Typeface userFont = null; 
    		int escidx = wholeUserText.indexOf('\u001b');
    		if( escidx > -1 ) {
    			if( m_effectResourceLoader!=null ) {
    				try {
						userFont = m_effectResourceLoader.getTypeface(wholeUserText.substring(0, escidx));
					} catch (TypefaceLoadException e) {
						if( LL.E ) Log.e( LOG_TAG, "typeface error", e);
					}
    			}
    			wholeUserText = wholeUserText.substring(escidx+1);
    		}
    		if( wholeUserText.length() < 1 ) {
/*                yoon delete
    			if( m_rsrc==null ) {
    				wholeUserText = "";
    			} else {
    				wholeUserText = m_rsrc.getString(R.string.title_effect_preview_sampletext);
    			}

            */
                wholeUserText = "Title Text Goes Here";
    		}
    		String[] userText = new String[]{wholeUserText}; // wholeUserText.split("\n", MAX_USERSTRINGS);
    		if(LL.D) Log.d(LOG_TAG, "==== User Text: " + wholeUserText); 
    		parseStr = parseStr.substring(0,parseStrLen);
    		String[] arg_array = parseStr.split(";");
    		Map<String,String> args = new HashMap<String,String>();
    		for( int i=0; i<arg_array.length; i++ ) {
    			String[] kv = arg_array[i].split("=");
    			if( kv.length<2 ) {
    				args.put(kv[0], "");
    			} else {
    				args.put(kv[0], kv[1]);
    			}
    		}

			if(args.containsKey("shadowblurtype") == false)
				args.put("shadowblurtype", "1");
    		
    		int flags = (int)Long.parseLong(args.get("flags"),16);
    		int align = Integer.parseInt(args.get("align"));
    		int longtext = Integer.parseInt(args.get("longtext"));
    		int width = Integer.parseInt(args.get("width"));
    		int height = Integer.parseInt(args.get("height"));
    		int bgcolor = (int)Long.parseLong(args.get("bgcolor"),16);
    		int fillcolor = (int)Long.parseLong(args.get("fillcolor"),16);
    		int strokecolor = (int)Long.parseLong(args.get("strokecolor"),16);
    		int shadowcolor = (int)Long.parseLong(args.get("shadowcolor"),16);
    		int maxlines = (int)Long.parseLong(args.get("maxlines"),16);
    		float skewx = Float.parseFloat(args.get("skewx"));
    		float scalex = Float.parseFloat(args.get("scalex"));
    		float size = Float.parseFloat(args.get("size"));
    		float strokewidth = Float.parseFloat(args.get("strokewidth"));
    		float spacingmult = Float.parseFloat(args.get("spacingmult"));
    		float spacingadd = Float.parseFloat(args.get("spacingadd"));
    		float shadowradius = Float.parseFloat(args.get("shadowradius"));
    		float textblur = Float.parseFloat(args.get("textblur"));
    		int blurtype = Integer.parseInt(args.get("blurtype"));
			int shadowblurtype = Integer.parseInt(args.get("shadowblurtype"));
    		int margin = Integer.parseInt(args.get("margin"));
    		float shadowoffsx = Float.parseFloat(args.get("shadowoffsx"));
    		float shadowoffsy = Float.parseFloat(args.get("shadowoffsy"));
    		String fmtstr = args.get("text");
    		String base_id = args.get("baseid");
    		fmtstr = pdecode(fmtstr);
    		int originalWidth = width;
    		int originalHeight = height;
    		int valign = align & NXT_VALIGNMASK;
    		align &= NXT_ALIGNMASK;
    		
    		for( int i=0; i<MAX_USERSTRINGS; i++ ) {
    			if( i<userText.length ) {
    				String t = userText[i];
    				int len = t.length();
    				
    				String front = t;
    				String back = "";
    				int dist = len;
    				int newdist;
    				int mid = 0;
    				for( int j=0; j<len; j++ ) {
    					if( t.charAt(j) == ' ' ) {
    						newdist = Math.abs(len/2 - j);
    						if( newdist < dist ) {
    							dist = newdist;
    							mid = j;
    						}
    					}
    				}
    				if( mid>0 && mid+1<len ) {
    					front = t.substring(0, mid);
    					back = t.substring(mid+1);
    				}
    				
    				
    				fmtstr = fmtstr.replace((CharSequence)("%" + (i+1)), (CharSequence)t);
    				fmtstr = fmtstr.replace((CharSequence)("%f" + (i+1)), (CharSequence)front);
    				fmtstr = fmtstr.replace((CharSequence)("%b" + (i+1)), (CharSequence)back);
    				if( t.length() == 1 ) {
	    				fmtstr = fmtstr.replace((CharSequence)("%s" + (i+1)), (CharSequence)(t.substring(0, 1)));
	    				fmtstr = fmtstr.replace((CharSequence)("%!s" + (i+1)), "");
	    				fmtstr = fmtstr.replace((CharSequence)("%e" + (i+1)), "");
	    				fmtstr = fmtstr.replace((CharSequence)("%!e" + (i+1)), "");
	    				fmtstr = fmtstr.replace((CharSequence)("%m" + (i+1)), "");
    				} else if( t.length() < 1 ) {
	    				fmtstr = fmtstr.replace((CharSequence)("%s" + (i+1)), "");
	    				fmtstr = fmtstr.replace((CharSequence)("%!s" + (i+1)), "");
	    				fmtstr = fmtstr.replace((CharSequence)("%e" + (i+1)), "");
	    				fmtstr = fmtstr.replace((CharSequence)("%!e" + (i+1)), "");
	    				fmtstr = fmtstr.replace((CharSequence)("%m" + (i+1)), "");
    				} else if( t.length() > 1 ) {
	    				fmtstr = fmtstr.replace((CharSequence)("%s" + (i+1)), (CharSequence)(t.substring(0, 1)));
	    				fmtstr = fmtstr.replace((CharSequence)("%!s" + (i+1)), (CharSequence)(t.substring(1, len)));
	    				fmtstr = fmtstr.replace((CharSequence)("%e" + (i+1)), (CharSequence)(t.substring(len-1, len)));
	    				fmtstr = fmtstr.replace((CharSequence)("%!e" + (i+1)), (CharSequence)(t.substring(0, len-1)));
	    				fmtstr = fmtstr.replace((CharSequence)("%m" + (i+1)), (CharSequence)(t.substring(1, len-1)));
    				}
    			} else {
    				fmtstr = fmtstr.replace((CharSequence)("%" + (i+1)), "");
    				fmtstr = fmtstr.replace((CharSequence)("%s" + (i+1)), "");
    				fmtstr = fmtstr.replace((CharSequence)("%!s" + (i+1)), "");
    				fmtstr = fmtstr.replace((CharSequence)("%e" + (i+1)), "");
    				fmtstr = fmtstr.replace((CharSequence)("%!e" + (i+1)), "");
    				fmtstr = fmtstr.replace((CharSequence)("%m" + (i+1)), "");
    			}
    		}
    		
    		
    		 /*   		Cyrillic U+0400�U+04FF
    		Cyrillic Supplement U+0500�U+052F
    		Cyrillic Extended-A U+2DE0�U+2DFF
    		Cyrillic Extended-B U+A640�U+A69F*/

    		boolean useSystemFont = false;
    		if( fmtstr.matches(".*[[\\u0400-\\u052F][\\u2DE0-\\u2DFF][\\uA640-\\uA69F]].*") ) {
    			useSystemFont = true;
    		}
    		
    		// This could just be Paint, but StaticLayout requires TextPaint; not sure why (Android documentation is lacking on this point) --Matthew
    		TextPaint paint = new TextPaint();
    		paint.setAntiAlias(true);
    		    		
    		String typeface = args.get("typeface");
    		if( userFont !=null ) {
    			paint.setTypeface(userFont);
    		} else if( useSystemFont ) {
    			int style = 0;
    			if( (flags & NXT_TEXTFLAG_BOLD)!=0 )
    				style |= Typeface.BOLD;
    			if( (flags & NXT_TEXTFLAG_ITALIC)!=0 )
    				style |= Typeface.ITALIC;
    			paint.setTypeface(Typeface.defaultFromStyle(style));
    		} else if( typeface.startsWith(TYPEFACE_SYSTEM) ) {
    			typeface = typeface.substring(TYPEFACE_SYSTEM.length());
    			int style = 0;
    			if( (flags & NXT_TEXTFLAG_BOLD)!=0 )
    				style |= Typeface.BOLD;
    			if( (flags & NXT_TEXTFLAG_ITALIC)!=0 )
    				style |= Typeface.ITALIC;
    			paint.setTypeface(Typeface.create(typeface, style));
    		} else if( typeface.startsWith(TYPEFACE_FILE) ) {
    			typeface = typeface.substring(TYPEFACE_FILE.length());
    			paint.setTypeface(Typeface.createFromFile(typeface));        			
    		} else if( typeface.startsWith(TYPEFACE_ASSET) ) {
    			typeface = typeface.substring(TYPEFACE_ASSET.length());
                //yoon
                if (m_assetManager != null)
                    paint.setTypeface(Typeface.createFromAsset(m_assetManager, typeface));

    		} else if( typeface.startsWith(TYPEFACE_THEME) ) {
    			// For now, theme typefaces come from assets as well
    			// (we'll need to change this when we support external
    			// themes)
    			typeface = typeface.substring(TYPEFACE_THEME.length());
                if( m_effectResourceLoader!=null ) {

                    try{

                        paint.setTypeface(m_effectResourceLoader.getTypeface(typeface));
                    }
                    catch(Font.TypefaceLoadException e){

                        e.printStackTrace();
                    }
                } 
    			// if( m_assetManager!=null )
    			// 	paint.setTypeface(Typeface.createFromAsset(m_assetManager, typeface));
            } else if( typeface.startsWith(TYPEFACE_FONTID) ) {

                typeface = typeface.substring(TYPEFACE_FONTID.length());

                paint.setTypeface(FontManager.getInstance().getTypeface(typeface));

            } else if( typeface.startsWith(TYPEFACE_FONTFILE) ) {

                Typeface tf = null;
                typeface = typeface.substring(TYPEFACE_FONTFILE.length());
                if( m_effectResourceLoader!=null ) {
                    try {
                        tf = m_effectResourceLoader.getTypeface(base_id, typeface);
                    } catch (IOException e) {
                        tf = null;
                        e.printStackTrace();
                    } catch (TypefaceLoadException e) {
                        tf = null;
                        e.printStackTrace();
                    }

                }
                if( tf != null ) {
                    paint.setTypeface(tf);
                } else {
                    Log.d(LOG_TAG, String.format("TYPEFACE NOT FOUND : base_id=%s, font=%s", base_id, typeface));
                }
            } else {
                File f = null;
    			if( m_effectResourceLoader!=null ) {
            		try {
						f = m_effectResourceLoader.getAssociatedFile(base_id, typeface);
					} catch (IOException e) {
						e.printStackTrace();
					}
    			} 
    			
    			if( f!=null ) {
    				if( !f.exists() ) {
        				if(LL.I) Log.i(LOG_TAG,"TYPEFACE FILE DOES NOT EXIST : base_id=" + base_id + "; f=" + f);
    				} else {
    					paint.setTypeface(Typeface.createFromFile(f));
    				}
    			} else {
    				if(LL.I) Log.i(LOG_TAG,"TYPEFACE NOT FOUND : base_id=" + base_id);
    			}
    		}
    		
    		if( (flags & NXT_TEXTFLAG_UNDERLINE)!=0 ) {
    			paint.setUnderlineText(true);
    		}
    		if( (flags & NXT_TEXTFLAG_STRIKE)!=0 ) {
    			paint.setStrikeThruText(true);
    		}
    		if( (flags & NXT_TEXTFLAG_SUBPIXEL)!=0 ) {
    			paint.setSubpixelText(true);
    		}
    		if( (flags & NXT_TEXTFLAG_LINEAR)!=0 ) {
    			paint.setLinearText(true);
    		}
    		if( (flags & NXT_TEXTFLAG_SHADOW)!=0 ) {
    		//	paint.setShadowLayer(shadowradius, shadowoffsx, shadowoffsy, shadowcolor);
    		}
    		if( scalex > 0.0 )
    			paint.setTextScaleX(scalex);
    		if( skewx > 0.0 )
    			paint.setTextSkewX(skewx);
    		if( size > 0.0 )
    			paint.setTextSize(size);
    		else
    			size = paint.getTextSize();
    		
    		if((flags & (NXT_TEXTFLAG_STROKE|NXT_TEXTFLAG_FILL))==0) {
    			flags |= NXT_TEXTFLAG_FILL;
    		}
    		
    		float tx, ty;
    		
    		
    		if( width>0 ) {
    			TextUtils.TruncateAt truncateAt = null;
    			switch( longtext ) {
    			case NXT_LONGTEXT_CROP_END:
    				// TODO
    				break;
    			case NXT_LONGTEXT_ELLIPSIZE_START:
    				truncateAt = TextUtils.TruncateAt.START;
    				break;
    			case NXT_LONGTEXT_ELLIPSIZE_MIDDLE:
    				truncateAt = TextUtils.TruncateAt.MIDDLE;
    				break;
    			case NXT_LONGTEXT_ELLIPSIZE_END:
    				truncateAt = TextUtils.TruncateAt.END;
    				break;
    			case NXT_LONGTEXT_WRAP:
    			default:
    				// TODO
    				break;
    			}
    			
        		Alignment reqTextAlign = Alignment.ALIGN_NORMAL;
        		switch( align ) {
    			case NXT_ALIGN_RIGHT:
    				reqTextAlign = Alignment.ALIGN_OPPOSITE;
    				break;
    			case NXT_ALIGN_CENTER:
    				reqTextAlign = Alignment.ALIGN_CENTER;
    				break;
    			case NXT_ALIGN_LEFT:
    			default:
    				reqTextAlign = Alignment.ALIGN_NORMAL;
    				break;
        		}
        		
        		int maxWidth = width>0?width:2048;
        		int maxHeight = height>0?height:2048;
    			
    			StaticLayout textLayout = null;
    			float workSize = size;
    			
    			for(;;) {
    				textLayout = new StaticLayout(
    					fmtstr, 0, fmtstr.length(), 
    					paint, width-(margin*2), reqTextAlign, 
    					spacingmult, spacingadd, 
    					true /*includepad*/, 
    					truncateAt, width-(margin*2));
    			
	    			width = textLayout.getWidth()+(margin*2);
	    			height = textLayout.getHeight()+(margin*2);
	    			
	    			/*if( truncateAt!=null ) {
		    			width = textLayout.getEllipsizedWidth()+(margin*2);
	    			}*/
	    			
	    			if( width <= maxWidth && height <= maxHeight && (maxlines<1 || textLayout.getLineCount()<=maxlines) )
	    				break;
	    			
	    			if( workSize > size/5.0 && workSize > 6.0 ) {
	    				workSize = workSize - Math.max(1.0f, size/12.0f); 
	    				paint.setTextSize(workSize);
	    			} else {
	    				if( width > maxWidth )
	    					width = maxWidth;
	    				if( height > maxHeight )
	    					height = maxHeight;
	    				break;
	    			}
	    			
    			}

                float xoffs = 0;
                float yoffs = 0;

                if( (flags & NXT_TEXTFLAG_STROKE)!=0 ) {
					switch( align ) {
						case NXT_ALIGN_RIGHT:
							xoffs = -strokewidth;
							break;
						case NXT_ALIGN_CENTER:
							xoffs = 0;
							break;
						case NXT_ALIGN_LEFT:
						default:
							xoffs = strokewidth;
							break;
					}
				}

    			if( (flags & NXT_TEXTFLAG_AUTOSIZE)==0 && originalWidth>0 && originalHeight>0 ) {
    				if(LL.D) Log.d(LOG_TAG, "VAlign=" + valign + " originalWidth=" + originalWidth + " originalHeight=" + originalHeight + " width=" + width + " height=" + height);
    				switch( valign ) {
    					case NXT_VALIGN_BOTTOM:
    						yoffs = originalHeight - height;
    						break;
    					case NXT_VALIGN_CENTER:
    						yoffs = (originalHeight - height)/2;
    						break;
    					case NXT_VALIGN_TOP:
    					default:
    						break;
    				}
    				width = originalWidth;
    				height = originalHeight;
    			}
    			
    			if( width > 2048 )
    				width = 2048;
    			if( height > 2048 )
    				height = 2048;
    			
	    		width = width < 1 ? 1 : width;
	    		height = height < 1 ? 1 : height;
    			
	    		resultBitmap = Bitmap.createBitmap( width, height, Bitmap.Config.ARGB_8888);
	    		
	            resultWidth	= resultBitmap.getWidth();
	            resultWidth	= resultWidth & 0xFFFFFFFE;
	            resultHeight	= resultBitmap.getHeight();
	            resultHeight	= resultHeight & 0xFFFFFFFE;
	    		
	    		Canvas canvas = new Canvas(resultBitmap);
	    		
	    		canvas.drawColor(bgcolor);
	    		/*paint.setColor(bgcolor);
	    		Xfermode oldmode = paint.getXfermode();
	    		if(LL.D) Log.d(LOG_TAG,"clearing to [A] : " + bgcolor);
	    		paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
	    		canvas.drawPaint(paint);
	    		paint.setXfermode(oldmode);*/
	    		
	    		canvas.save();
	    		canvas.translate(margin,margin);
    			canvas.translate(xoffs,yoffs);

	    		//	paint.setShadowLayer(shadowradius, shadowoffsx, shadowoffsy, shadowcolor);

	    		if( (flags & NXT_TEXTFLAG_SHADOW)!=0 ) {

                    if(shadowradius > 0){

						BlurMaskFilter.Blur blur = Blur.NORMAL;
						switch( shadowblurtype ) {
						case NXT_BLUR_NORMAL:
							blur = Blur.NORMAL;
							break;
						case NXT_BLUR_INNER:
							blur = Blur.INNER;
							break;
						case NXT_BLUR_OUTER:
							blur = Blur.OUTER;
							break;
						case NXT_BLUR_SOLID:
							blur = Blur.SOLID;
							break;
						}

                        canvas.save();
                        canvas.translate(shadowoffsx, shadowoffsy);
                        paint.setMaskFilter(new BlurMaskFilter(shadowradius, blur));
                        paint.setStyle(Paint.Style.FILL);
                        paint.setColor(shadowcolor);
                        textLayout.draw(canvas);
                        paint.setMaskFilter(null);
                        canvas.restore();                        
                    }
	    			
	    		}

	    		if( (flags & NXT_TEXTFLAG_STROKEBACK)!=0 ) {
	        		/*if( (flags & NXT_TEXTFLAG_FILL)!=0 && (flags & NXT_TEXTFLAG_SHADOW)!=0 ) {
	            		paint.setShadowLayer(0, 0, 0, 0);
	        		}*/
	    			paint.setStyle(Paint.Style.STROKE);
	    			paint.setColor(strokecolor);
	    			paint.setStrokeWidth(strokewidth);
	    			//Path textPath = new Path();
	    			
	    			textLayout.draw(canvas);
	    			
	    			//paint.getTextPath(fmtstr, 0, fmtstr.length(), tx, ty, textPath);
	    			//canvas.drawPath(textPath, paint);
	    			//canvas.drawText(fmtstr, tx, ty, paint);
	    		}

	    		if( textblur > 0.00001 ) {
	    			BlurMaskFilter.Blur blur = Blur.NORMAL;
	    			switch( blurtype ) {
	    			case NXT_BLUR_NORMAL:
	    				blur = Blur.NORMAL;
	    				break;
	    			case NXT_BLUR_INNER:
	    				blur = Blur.INNER;
	    				break;
	    			case NXT_BLUR_OUTER:
	    				blur = Blur.OUTER;
	    				break;
	    			case NXT_BLUR_SOLID:
	    				blur = Blur.SOLID;
	    				break;
	    			}
	    			paint.setMaskFilter(new BlurMaskFilter(textblur, blur));
	    		}
	    		
	    		if( (flags & NXT_TEXTFLAG_FILL)!=0 ) {
	    			if( (flags & NXT_TEXTFLAG_CUTOUT)!=0 ) {
	    				paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_IN));
	    			}
	    			paint.setStyle(Paint.Style.FILL);
	    			paint.setColor(fillcolor);
	    			textLayout.draw(canvas);
	    			paint.setXfermode(null);
	    		}
	    		
	    		if( (flags & NXT_TEXTFLAG_STROKE)!=0 ) {
	        		/*if( (flags & NXT_TEXTFLAG_FILL)!=0 && (flags & NXT_TEXTFLAG_SHADOW)!=0 ) {
	            		paint.setShadowLayer(0, 0, 0, 0);
	        		}*/
	    			paint.setStyle(Paint.Style.STROKE);
	    			paint.setColor(strokecolor);
	    			paint.setStrokeWidth(strokewidth);
	    			//Path textPath = new Path();
	    			
	    			textLayout.draw(canvas);
	    			
	    			//paint.getTextPath(fmtstr, 0, fmtstr.length(), tx, ty, textPath);
	    			//canvas.drawPath(textPath, paint);
	    			//canvas.drawText(fmtstr, tx, ty, paint);
	    		}
    			paint.setMaskFilter(null);
	    		
	    		canvas.restore();
    			
    		} else {
    			
	    		Rect textBounds = new Rect();
	    		paint.getTextBounds(fmtstr, 0, fmtstr.length(), textBounds);
	    		textBounds.right += 20;
	    		int textMargin = (int)Math.ceil(shadowradius);
	    		
				FontMetrics metrics = paint.getFontMetrics();
				int textHeight = (int)Math.max(metrics.bottom * 2 - metrics.top, textBounds.height());
	
	    		if( width < 1 || height < 1 ) {
	
	    			width = Math.max(textBounds.width(),textBounds.right)+(textMargin*2)+(int)Math.abs(shadowoffsx);
	    			height = textHeight+(textMargin*2)+(int)Math.abs(shadowoffsy);
	    			if( width > 2048 )
	    				width = 2048;
	    			if( height > 2048 )
	    				height = 2048;
	    		}
	    		
	    		width = width < 1 ? 1 : width;
	    		height = height < 1 ? 1 : height;
	    		
	    		switch( align ) {
    			case NXT_ALIGN_RIGHT:
    				paint.setTextAlign(Paint.Align.RIGHT);
    				tx = width;
    				break;
    			case NXT_ALIGN_CENTER:
    				paint.setTextAlign(Paint.Align.CENTER);
    				tx = width/2;
    				break;
    			case NXT_ALIGN_LEFT:
    			default:
    				paint.setTextAlign(Paint.Align.LEFT);
    				tx = 20;
    				break;
        		}
	    		
	    		ty = (height - metrics.ascent)/2;
	    		//ty = height - textMargin - textBounds.bottom - Math.max(0,shadowoffsy);
	    		tx += textMargin - Math.min(0,shadowoffsx);
	    		
	    		resultBitmap = Bitmap.createBitmap( width, height, Bitmap.Config.ARGB_8888);
	    		
	            resultWidth	= resultBitmap.getWidth();
	            resultWidth	= resultWidth & 0xFFFFFFFE;
	            resultHeight	= resultBitmap.getHeight();
	            resultHeight	= resultHeight & 0xFFFFFFFE;
	    		
	    		Canvas canvas = new Canvas(resultBitmap);
	    		
	    		/*paint.setColor(bgcolor);
	    		Xfermode oldmode = paint.getXfermode();
	    		if(LL.D) Log.d(LOG_TAG,"clearing to [A] : " + bgcolor);
	    		paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
	    		canvas.drawPaint(paint);
	    		paint.setXfermode(oldmode);*/
	    		canvas.drawColor(bgcolor);
	    		
	    		if( (flags & NXT_TEXTFLAG_FILL)!=0 ) {
	    			paint.setStyle(Paint.Style.FILL);
	    			paint.setColor(fillcolor);
	    			canvas.drawText(fmtstr, tx, ty, paint);
	    		}
	    		
	    		if( (flags & NXT_TEXTFLAG_STROKE)!=0 ) {
	        		if( (flags & NXT_TEXTFLAG_FILL)!=0 && (flags & NXT_TEXTFLAG_SHADOW)!=0 ) {
	            		paint.setShadowLayer(0, 0, 0, 0);
	        		}
	    			paint.setStyle(Paint.Style.STROKE);
	    			paint.setColor(strokecolor);
	    			paint.setStrokeWidth(strokewidth);
	    			Path textPath = new Path();
	    			paint.getTextPath(fmtstr, 0, fmtstr.length(), tx, ty, textPath);
	    			canvas.drawPath(textPath, paint);
	    			//canvas.drawText(fmtstr, tx, ty, paint);
	    		}
    		}
    		
    	} else if (path.startsWith(TAG_ThemeImage)) {
    		
    		String untagged_path = path.substring(TAG_ThemeImage.length());
    		String base_id;
    		String rel_path;
    		
    		int delim_idx = untagged_path.indexOf('/');
    		if( delim_idx>=0 ) {
    			base_id = untagged_path.substring(0,delim_idx);
    			rel_path = untagged_path.substring(delim_idx+1);
    		} else {
    			base_id = "";
    			rel_path = untagged_path;
    		}
    		
    		resultBitmap = null;
    		
    		if( m_effectResourceLoader!=null ) {
    		
	        	try {
                    AssetPackageReader reader = m_effectResourceLoader.getReader(base_id);
                    resultBitmap    = BitmapFactory.decodeStream(m_effectResourceLoader.openAssociatedFile(base_id, rel_path));
				} catch (IOException e) {
					resultBitmap = null;
					if(LL.E) Log.e(LOG_TAG,"Error loading bitmap for effect(" + base_id + ") : " + rel_path);
					e.printStackTrace();
				}

    		}
    		
    		
//    		if( m_themeManagerWrapper!=null && m_themeManagerWrapper.getThemeManager()!=null ) {
//        		NexThemeManager.NexFXItem item = null;
//        		
//        		if( item==null )
//        			item = m_themeManagerWrapper.getThemeManager().getEffect(base_id);
//        		if( item==null )
//        			item = m_themeManagerWrapper.getThemeManager().getTransition(base_id);
//        		if( item==null )
//        			item = m_themeManagerWrapper.getThemeManager().getTheme(base_id);
//        		
//        		if( item!=null && m_assetManager!=null ) {
//            		if(LL.D) Log.d(LOG_TAG,"Found item for base_id=" + base_id);
//		        	try {
//						mBitmap	= BitmapFactory.decodeStream(item.openAssociatedFile(m_assetManager, rel_path));
//					} catch (IOException e) {
//						mBitmap = null;
//						if(LL.E) Log.e(LOG_TAG,"Error loading bitmap for effect(" + base_id + ") : " + rel_path);
//						e.printStackTrace();
//					}
//        		} else {
//            		if(LL.D) Log.d(LOG_TAG,"NOT FOUND : base_id=" + base_id);
//        		}
//    		} 
    		
    		if (resultBitmap==null && m_assetManager!=null) {
	        	try {
					resultBitmap	= BitmapFactory.decodeStream(m_assetManager.open(rel_path));
				} catch (IOException e) {
					resultBitmap = null;
					if(LL.E) Log.e(LOG_TAG,"Error loading bitmap (general asset mode) for effect(" + base_id + ") : " + rel_path);
					e.printStackTrace();
				}
    		}
        	if( resultBitmap != null) {
	            resultWidth	= resultBitmap.getWidth();
	            resultHeight	= resultBitmap.getHeight();
				return new NexImage(resultBitmap, resultWidth, resultHeight);
        	} else {
				if(LL.E) Log.e(LOG_TAG,"Bitmap failed to load for effect(" + base_id + ") : " + rel_path);
        	}
    	} else if (path.startsWith(TAG_Overlay)) {
    		if( m_overlayPathResolver==null ) {
    			resultBitmap = Bitmap.createBitmap(8, 8, Bitmap.Config.ARGB_8888);
	            resultWidth	= resultBitmap.getWidth();
	            resultHeight	= resultBitmap.getHeight();
				return new NexImage(resultBitmap, resultWidth, resultHeight);
    		}
    		String untagged_path = path.substring(TAG_Overlay.length());
    		String resolved_path = m_overlayPathResolver.resolveOverlayPath(untagged_path);
    		if( resolved_path==null ) {
				resultBitmap = Bitmap.createBitmap(8, 8, Bitmap.Config.ARGB_8888);
	            resultWidth	= resultBitmap.getWidth();
	            resultHeight	= resultBitmap.getHeight();
				return new NexImage(resultBitmap, resultWidth, resultHeight);
    		} 
    		//if(LL.D) Log.d(LOG_TAG,"openThemeImage -> resolved overlay path : " + resolved_path);
    		
    		BitmapFactory.Options opts = new BitmapFactory.Options();
    		opts.inJustDecodeBounds = true;
			BitmapFactory.decodeFile(resolved_path,opts );
			opts.inJustDecodeBounds = false;
			opts.inSampleSize = 1; 
			resultBitmap	= BitmapFactory.decodeFile(resolved_path, opts);
//    		if(LL.D) Log.d(LOG_TAG,"  openThemeImage -> overlay bitmap : " + mBitmap);
        	if( resultBitmap != null) {
	            resultWidth	= resultBitmap.getWidth();
	            resultHeight	= resultBitmap.getHeight();
//	    		if(LL.D) Log.d(LOG_TAG,"  openThemeImage -> overlay bitmap w=" + mWidth + " h=" + mHeight );
	    		
	    		/** TESTING CODE - CHECK EMPTY **/
				/*int pixels[] = new int[mBitmap.getWidth()];
				boolean empty = true;
				for( int y=0; y<mBitmap.getHeight(); y++ ) {
					mBitmap.getPixels(pixels, 0, pixels.length, 0, y, pixels.length, 1);
					for( int x =0; x<pixels.length; x++ ) {
						if( (pixels[x] & 0x00FFFFFF)!=0 ) {
							empty = false;
							break;
						}
					}
					if(!empty)
						break;
				}
	    		if(LL.D) Log.d(LOG_TAG,"  openThemeImage -> overlay bitmap empty=" + empty );*/
	    		/********************************/
	    		
				return new NexImage(resultBitmap, resultWidth, resultHeight);
        	}
        } else if (path.startsWith(TAG_PreviewThemeImage)) {
        	// Loads the image at a lower resolution for better performance during preview
        	// Only used if ALTERNATE_PREVIEW_RESOLUTION is defined when building the theme
        	// renderer in the engine.
        	//
        	// (Generally disabled because in actual testing, this didn't give much performance benefit
        	// unless the resolution was reduced significantly, which looked really bad).
    		
    		String untagged_path = path.substring(TAG_ThemeImage.length());
    		String base_id;
    		String rel_path;
    		
    		int delim_idx = untagged_path.indexOf('/');
    		if( delim_idx>=0 ) {
    			base_id = untagged_path.substring(0,delim_idx);
    			rel_path = untagged_path.substring(delim_idx+1);
    		} else {
    			base_id = "";
    			rel_path = untagged_path;
    		}
    		
    		resultBitmap = null;
    		
			BitmapFactory.Options opts = new BitmapFactory.Options();
//			opts.inSampleSize = 1;
    		
    		if( m_effectResourceLoader!=null ) {
    		
	        	try {
                    AssetPackageReader reader = m_effectResourceLoader.getReader(base_id);
                    resultBitmap	= BitmapFactory.decodeStream(m_effectResourceLoader.openAssociatedFile(base_id, rel_path),null,opts);
				} catch (IOException e) {
					resultBitmap = null;
					if(LL.E) Log.e(LOG_TAG,"Error loading bitmap for effect(" + base_id + ") : " + rel_path);
					e.printStackTrace();
				}

    		}
    		
    		
//    		if( m_themeManagerWrapper!=null && m_themeManagerWrapper.getThemeManager()!=null ) {
//        		NexThemeManager.NexFXItem item = null;
//        		
//        		if( item==null )
//        			item = m_themeManagerWrapper.getThemeManager().getEffect(base_id);
//        		if( item==null )
//        			item = m_themeManagerWrapper.getThemeManager().getTransition(base_id);
//        		if( item==null )
//        			item = m_themeManagerWrapper.getThemeManager().getTheme(base_id);
//        		
//        		if( item!=null && m_assetManager!=null ) {
//            		if(LL.D) Log.d(LOG_TAG,"Found item for base_id=" + base_id);
//		        	try {
//						mBitmap	= BitmapFactory.decodeStream(item.openAssociatedFile(m_assetManager, rel_path));
//					} catch (IOException e) {
//						mBitmap = null;
//						if(LL.E) Log.e(LOG_TAG,"Error loading bitmap for effect(" + base_id + ") : " + rel_path);
//						e.printStackTrace();
//					}
//        		} else {
//            		if(LL.D) Log.d(LOG_TAG,"NOT FOUND : base_id=" + base_id);
//        		}
//    		} 
    		
    		if (resultBitmap==null && m_assetManager!=null) {
	        	try {
					resultBitmap	= BitmapFactory.decodeStream(m_assetManager.open(rel_path),null,opts);
				} catch (IOException e) {
					resultBitmap = null;
					if(LL.E) Log.e(LOG_TAG,"Error loading bitmap (general asset mode) for effect(" + base_id + ") : " + rel_path);
					e.printStackTrace();
				}
    		}
        	if( resultBitmap != null) {
	            resultWidth	= resultBitmap.getWidth();
	            resultHeight	= resultBitmap.getHeight();
				return new NexImage(resultBitmap, resultWidth, resultHeight);
        	} else {
				if(LL.E) Log.e(LOG_TAG,"Bitmap failed to load for effect(" + base_id + ") : " + rel_path);
        	}
    	} else if (path.startsWith(TAG_Overlay)) {
    		if( m_overlayPathResolver==null ) {
    			resultBitmap = Bitmap.createBitmap(8, 8, Bitmap.Config.ARGB_8888);
	            resultWidth	= resultBitmap.getWidth();
	            resultHeight	= resultBitmap.getHeight();
				return new NexImage(resultBitmap, resultWidth, resultHeight);
    		}
    		String untagged_path = path.substring(TAG_Overlay.length());
    		String resolved_path = m_overlayPathResolver.resolveOverlayPath(untagged_path);
    		if( resolved_path==null ) {
				resultBitmap = Bitmap.createBitmap(8, 8, Bitmap.Config.ARGB_8888);
	            resultWidth	= resultBitmap.getWidth();
	            resultHeight	= resultBitmap.getHeight();
				return new NexImage(resultBitmap, resultWidth, resultHeight);
    		} 
    		//if(LL.D) Log.d(LOG_TAG,"openThemeImage -> resolved overlay path : " + resolved_path);
    		
    		BitmapFactory.Options opts = new BitmapFactory.Options();
    		opts.inJustDecodeBounds = true;
			BitmapFactory.decodeFile(resolved_path,opts );
			opts.inJustDecodeBounds = false;
			opts.inSampleSize = 1; 
			resultBitmap	= BitmapFactory.decodeFile(resolved_path, opts);
//    		if(LL.D) Log.d(LOG_TAG,"  openThemeImage -> overlay bitmap : " + mBitmap);
        	if( resultBitmap != null) {
	            resultWidth	= resultBitmap.getWidth();
	            resultHeight	= resultBitmap.getHeight();
//	    		if(LL.D) Log.d(LOG_TAG,"  openThemeImage -> overlay bitmap w=" + mWidth + " h=" + mHeight );
	    		
	    		/** TESTING CODE - CHECK EMPTY **/
				/*int pixels[] = new int[mBitmap.getWidth()];
				boolean empty = true;
				for( int y=0; y<mBitmap.getHeight(); y++ ) {
					mBitmap.getPixels(pixels, 0, pixels.length, 0, y, pixels.length, 1);
					for( int x =0; x<pixels.length; x++ ) {
						if( (pixels[x] & 0x00FFFFFF)!=0 ) {
							empty = false;
							break;
						}
					}
					if(!empty)
						break;
				}
	    		if(LL.D) Log.d(LOG_TAG,"  openThemeImage -> overlay bitmap empty=" + empty );*/
	    		/********************************/
	    		
				return new NexImage(resultBitmap, resultWidth, resultHeight);
        	}
    	} else if (!path.startsWith("[")) {
			resultBitmap = null;
			if( m_assetManager!=null ) {
	        	try {
                    //yoon
                    if( path.compareTo("placeholder1.jpg") == 0 || path.compareTo("placeholder2.jpg") == 0  ){
                        resultBitmap = BitmapFactory.decodeStream(m_assetManager.open(path));
                    }else {
                        resultBitmap = BitmapFactory.decodeStream(m_assetManager.open(path));
                    }
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
        	if( resultBitmap != null) {
	            resultWidth	= resultBitmap.getWidth();
	            resultHeight	= resultBitmap.getHeight();
				return new NexImage(resultBitmap, resultWidth, resultHeight);
        	}
    	}
    	if (resultBitmap !=null && resultWidth > 0 && resultHeight > 0 ) {
			return new NexImage(resultBitmap, resultWidth, resultHeight);
    	}
        return null;
    }
    
	/*public static BitmapFactory.Options s_opt = new BitmapFactory.Options();
    
	public static long getImageDirect( String path, Buffer buffer ) {
    	if( path.startsWith("@solid:") && path.endsWith(".jpg")) {
//    		int color = (int)Long.parseLong(path.substring(7, 15),16);
//    		m_premult = true;
//    		mWidth = 32;
//    		mHeight = 18;
//    		int[] pixels = new int[mWidth*mHeight];
//    		for( int i=0; i<pixels.length; i++ )
//    			pixels[i] = color;
//    		mBitmap = Bitmap.createBitmap(pixels, mWidth, mHeight, Bitmap.Config.ARGB_8888);
//    		return 0;
    	}
        try
        {
        	s_opt.inSampleSize = 1;
        	s_opt.inJustDecodeBounds = true;
			BitmapFactory.decodeFile(path, s_opt);
			s_opt.inJustDecodeBounds = false;
			if(LL.D) Log.d(LOG_TAG,"Image width=" + s_opt.outWidth + " height=" + s_opt.outHeight + " name='" + path + "'");
			
			final int MAX_WIDTH = 810; // 720 * 1.125
			final int MAX_HEIGHT = 1440; // 1280 * 1.125
			final int MAX_SIZE = 1500000; // 1280 * 1.125
			
			if( s_opt.outHeight > MAX_WIDTH && s_opt.outWidth > MAX_HEIGHT ) {
				s_opt.inSampleSize = Math.min(s_opt.outHeight/MAX_HEIGHT, s_opt.outWidth/MAX_WIDTH) + 1;
				if(LL.D) Log.d(LOG_TAG,"Downsampling image on w,h (inSampleSize=" + s_opt.inSampleSize + ")");
			} else if( (s_opt.outHeight * s_opt.outWidth > MAX_SIZE ) ) {
				s_opt.inSampleSize = (s_opt.outHeight * s_opt.outWidth)/MAX_SIZE + 1;
				if(LL.D) Log.d(LOG_TAG,"Downsampling image on w*h>1500000 (inSampleSize=" + s_opt.inSampleSize + ")");
			}
			
			int actualWidth = s_opt.outWidth/s_opt.inSampleSize;
			int actualHeight = s_opt.outHeight/s_opt.inSampleSize;
			
			if( buffer==null )
			{
				return 
			}
			
			mBitmap = BitmapFactory.decodeFile(path, opt);
        	if( mBitmap != null) {
    			if(LL.D) Log.d(LOG_TAG,"Actual bitmap width=" + mBitmap.getWidth() + " height=" + mBitmap.getHeight() );
	            mWidth	= mBitmap.getWidth();
	            mWidth	= mWidth & 0xFFFFFFFE;
	            mHeight	= mBitmap.getHeight();
	            mHeight	= mHeight & 0xFFFFFFFE;
        		m_premult = true;
	        	return 0;
        	}
        	else
        	{
        		mWidth = 0;
        		mHeight = 0;
        		m_premult = true;
        	}
        }
        catch (Exception e) { 
        	
        }
        return 1;
	}*/

	public static class LoadedBitmapInfo{

		private int originalWidth;
		private int originalHeight;
		private int loadedType;
		private LoadedBitmapInfo(int originalWidth, int originalHeight, int loadedType) {
			this.originalWidth = originalWidth;
			this.originalHeight = originalHeight;
			this.loadedType = loadedType;
		}
		private LoadedBitmapInfo(int originalWidth, int originalHeight) {
			this.originalWidth = originalWidth;
			this.originalHeight = originalHeight;
			this.loadedType = 1;
		}
		public int getOriginalWidth() {
			return originalWidth;
		}
		public int getOriginalHeight() {
			return originalHeight;
		}
		public int getLoadedType() {
			return loadedType;
		}
	}
    
    public static class LoadedBitmap {
    	private Bitmap bitmap;
    	private int originalWidth;
    	private int originalHeight;
		private int loadedType;
    	private LoadedBitmap( Bitmap bitmap, int originalWidth, int originalHeight, int loadedType) {
    		this.originalWidth = originalWidth;
    		this.originalHeight = originalHeight;
    		this.bitmap = bitmap;
    		this.loadedType = loadedType;
    	}
		private LoadedBitmap( Bitmap bitmap, int originalWidth, int originalHeight) {
			this.originalWidth = originalWidth;
			this.originalHeight = originalHeight;
			this.bitmap = bitmap;
			this.loadedType = 1;
		}
    	public Bitmap getBitmap() {
    		return bitmap;
    	}
    	public int getOriginalWidth() {
    		return originalWidth;
    	}
    	public int getOriginalHeight() {
    		return originalHeight;
    	}
		public int getLoadedType() {
			return loadedType;
		}
    }
    
//    public static LoadedBitmap loadBitmap( String path ) {
////		final int JPEG_MAX_WIDTH = 1440; // 720 * 1.125
////		final int JPEG_MAX_HEIGHT = 810; // 1280 * 1.125
////		final int JPEG_MAX_SIZE = 1500000; // 1280 * 1.125
//		final int JPEG_MAX_WIDTH = 2048; // 720 * 1.125
//		final int JPEG_MAX_HEIGHT = 2048; // 1280 * 1.125
//		final int JPEG_MAX_SIZE = (2048*2048); // 1280 * 1.125
//    	return loadBitmap(path,JPEG_MAX_WIDTH,JPEG_MAX_HEIGHT,JPEG_MAX_SIZE);
//    }
    
    public static LoadedBitmap loadBitmap( String path, int maxWidth, int maxHeight ) {
    	return loadBitmap(path, maxWidth, maxHeight, Integer.MAX_VALUE, 0);
    }

    public static LoadedBitmap loadBitmap( InputStream stream, int maxWidth, int maxHeight, int maxSize ) {
		BufferedInputStream inputStream = new BufferedInputStream(stream);
        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inJustDecodeBounds = true;

		try {
			inputStream.mark(stream.available());
		} catch (IOException e) {
			e.printStackTrace();
		}
		BitmapFactory.decodeStream(inputStream, null, opt);
        try {
			inputStream.reset();
        } catch (IOException e) {
            throw new RuntimeException("Failed to reset stream",e);
        }
        opt.inJustDecodeBounds = false;
        int originalWidth = opt.outWidth;
        int originalHeight = opt.outHeight;

        int sampleSize = 1;
        while( 	sampleSize < 8 &&
                (( opt.outWidth / sampleSize > maxWidth && opt.outHeight / sampleSize > maxHeight) ||
                        (( opt.outWidth / sampleSize * opt.outHeight / sampleSize > (maxSize>0?maxSize:1500000)) )))
        {
            sampleSize *= 2;
        }

        opt.inSampleSize = sampleSize;

        if(LL.D) Log.d(LOG_TAG,"loadBitmap from stream width=" + opt.outWidth + " height=" + opt.outHeight + " sampleSize=" + sampleSize );

		Bitmap bm = BitmapFactory.decodeStream(inputStream, null, opt);
		try {
			inputStream.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		if( bm==null ) {
            return new LoadedBitmap(null, 0, 0);
        }

        return new LoadedBitmap(bm,originalWidth,originalHeight);
    }

    public static LoadedBitmap loadBitmap( String path, int maxWidth, int maxHeight, int maxSize , int iThumb) {
        return loadBitmap(path,maxWidth,maxHeight,maxSize,null,iThumb);
    }

	private static class CachedBitmapInfo {
		final String path;
		final int maxWidth;
		final int maxHeight;
		final int maxSize;
		final Bitmap.Config preferredConfig;
		final boolean isThumbnail;

		private CachedBitmapInfo(String path, int maxWidth, int maxHeight, int maxSize, Bitmap.Config preferredConfig, boolean bThumb) {
			this.path = path==null?"":path;
			this.maxWidth = maxWidth;
			this.maxHeight = maxHeight;
			this.maxSize = maxSize;
			this.preferredConfig = preferredConfig;
			this.isThumbnail = bThumb;
		}

		@Override
		public boolean equals(Object o) {
			if( o==null || !(o instanceof CachedBitmapInfo) )
				return false;
			if( o==this )
				return true;
			CachedBitmapInfo other = (CachedBitmapInfo)o;
			return maxWidth == other.maxWidth && maxHeight == other.maxHeight && maxSize == other.maxSize && preferredConfig == other.preferredConfig && path.equals(other.path) && isThumbnail == other.isThumbnail;
		}

		@Override
		public int hashCode() {
			return path.hashCode() + maxWidth*191 + maxHeight*61 + maxSize*199 + (preferredConfig==null?0:preferredConfig.hashCode());
		}
	}

	private static Map<CachedBitmapInfo,WeakReference<Bitmap>> sBitmapCache = new HashMap<>();
	private static WeakHashMap<Bitmap, LoadedBitmapInfo> sLoadedBitmapCache = new WeakHashMap<>();
	private static int sCleanCacheCount = 0;
	private static final Object sBitmapCacheLock = new Object();

    public static LoadedBitmap loadBitmap( String path, int maxWidth, int maxHeight, int maxSize, Bitmap.Config preferredConfig, int iThumb ) {
		if(iThumb ==1) {
			return loadBitmapThumb(path, maxWidth, maxHeight, maxSize, preferredConfig);
		}
		Log.d(LOG_TAG, "loadBitmap");
		CachedBitmapInfo bci = new CachedBitmapInfo(path, maxWidth, maxHeight, maxSize, preferredConfig, false);
		synchronized (sBitmapCacheLock) {

			LoadedBitmap ret = null;
			WeakReference<Bitmap> refExistingBitmap = sBitmapCache.get(bci);
			if (refExistingBitmap != null) {
				Bitmap existingBitmap = refExistingBitmap.get();
				if (existingBitmap != null) {
					LoadedBitmapInfo existingLoadedBitmapInfo = sLoadedBitmapCache.get(existingBitmap);
					if (existingLoadedBitmapInfo != null) {

						ret = new LoadedBitmap(existingBitmap, existingLoadedBitmapInfo.getOriginalWidth(), existingLoadedBitmapInfo.getOriginalHeight(), existingLoadedBitmapInfo.getLoadedType());
					}
				}
			}

			sCleanCacheCount++;
			if (sCleanCacheCount > 30) {
				sCleanCacheCount = 0;
				List<CachedBitmapInfo> toRemove = null;
				for (Map.Entry<CachedBitmapInfo, WeakReference<Bitmap>> entry : sBitmapCache.entrySet()) {
					if( entry.getValue().get()==null ) {
						if( toRemove==null ) {
							toRemove = new ArrayList<>();
						}
						toRemove.add(entry.getKey());
					}
				}
				if( toRemove!=null ) {
					for( CachedBitmapInfo e: toRemove ) {
						sBitmapCache.remove(e);
					}
				}
			}

			if(ret != null)
				return ret;
		}

    	String lcpath = path.toLowerCase(Locale.US);
    	int orientation = ExifInterface.ORIENTATION_UNDEFINED;
    	if( lcpath.endsWith(".jpeg") || lcpath.endsWith(".jpg") ) {
	    	try {
				ExifInterface exif = new ExifInterface(path);
				orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_UNDEFINED);
			} catch (IOException e) {
				orientation = ExifInterface.ORIENTATION_UNDEFINED;
			}
    	}
    	
		BitmapFactory.Options opt = new BitmapFactory.Options();
		opt.inJustDecodeBounds = true;
		BitmapFactory.decodeFile(path, opt);
		opt.inJustDecodeBounds = false;
        opt.inPreferredConfig = preferredConfig;
    	int originalWidth = opt.outWidth;
    	int originalHeight = opt.outHeight;
		
    	int sampleSize = 1;
    	while( 	sampleSize < 8 && 
    			(( opt.outWidth / sampleSize > maxWidth && opt.outHeight / sampleSize > maxHeight) || 
    			(( opt.outWidth / sampleSize * opt.outHeight / sampleSize > (maxSize>0?maxSize:1500000)) )))
    	{
    		sampleSize *= 2;
    	}	

    	opt.inSampleSize = sampleSize;
		
		if(LL.D) Log.d(LOG_TAG,"loadBitmap width=" + opt.outWidth + " height=" + opt.outHeight + " sampleSize=" + sampleSize + " name='" + path + "'");
    	
		Bitmap bm = BitmapFactory.decodeFile(path, opt);
		if( bm==null ) {
			return new LoadedBitmap(null, 0, 0);
		}
		Bitmap original = bm;
		
		switch( orientation ) {
		
			case ExifInterface.ORIENTATION_FLIP_HORIZONTAL:
			{
				bm = Bitmap.createBitmap(original.getWidth(), original.getHeight(), Bitmap.Config.ARGB_8888);
				Canvas c = new Canvas(bm);
				Matrix m = new Matrix();
				m.setScale(-1, 1);
				m.postTranslate(original.getWidth(), 0);
				c.drawBitmap(original, m, null);
			}
			case ExifInterface.ORIENTATION_FLIP_VERTICAL:
			{
				bm = Bitmap.createBitmap(original.getWidth(), original.getHeight(), Bitmap.Config.ARGB_8888);
				Canvas c = new Canvas(bm);
				Matrix m = new Matrix();
				m.setScale(1, -1);
				m.postTranslate(0, original.getHeight());
				c.drawBitmap(original, m, null);
				break;
			}
			case ExifInterface.ORIENTATION_NORMAL:
			{
				// Nothing to do; already in the normal orientation
				break;
			}
			case ExifInterface.ORIENTATION_ROTATE_180:
			{
				bm = Bitmap.createBitmap(original.getWidth(), original.getHeight(), Bitmap.Config.ARGB_8888);
				Canvas c = new Canvas(bm);
				Matrix m = new Matrix();
				m.setScale(-1, -1);
				m.postTranslate(original.getWidth(), original.getHeight());
				c.drawBitmap(original, m, null);
				break;
			}
			case ExifInterface.ORIENTATION_ROTATE_90:
			{
				bm = Bitmap.createBitmap(original.getHeight(), original.getWidth(), Bitmap.Config.ARGB_8888);
				Canvas c = new Canvas(bm);
				Matrix m = new Matrix();
				m.setRotate(90);
				m.postTranslate(original.getHeight(), 0);
				c.drawBitmap(original, m, null);
				break;
			}
			case ExifInterface.ORIENTATION_ROTATE_270:
			{
				bm = Bitmap.createBitmap(original.getHeight(), original.getWidth(), Bitmap.Config.ARGB_8888);
				Canvas c = new Canvas(bm);
				Matrix m = new Matrix();
				m.setRotate(270);
				m.postTranslate(0, original.getWidth());
				c.drawBitmap(original, m, null);
				break;
			}
			case ExifInterface.ORIENTATION_TRANSPOSE:
			{
				// TODO: Support this orientation (it's the same as ORIENTATION_ROTATE_90, but flipped horizontally)
				break;
			}
			case ExifInterface.ORIENTATION_TRANSVERSE:
			{
				// TODO: Support this orientation (it's the same as ORIENTATION_ROTATE_270, but flipped horizontally)
				break;
			}
			case ExifInterface.ORIENTATION_UNDEFINED:
			{
				// Nothing to do; cannot correct the orientation if we don't know it
				break;
			}
			default:
			{
				break;
			}
		}

        if( preferredConfig!=null && bm.getConfig() != preferredConfig ) {
            Bitmap bm2 = Bitmap.createBitmap(bm.getWidth(), bm.getHeight(), preferredConfig);
            Canvas cvs = new Canvas(bm2);
            cvs.drawBitmap(bm, 0, 0, null);
            bm.recycle();
            bm = bm2;
        }

        LoadedBitmap result;
		
		switch( orientation ) {
			case ExifInterface.ORIENTATION_ROTATE_90:
			case ExifInterface.ORIENTATION_ROTATE_270:
			case ExifInterface.ORIENTATION_TRANSPOSE:
			case ExifInterface.ORIENTATION_TRANSVERSE:
				result = new LoadedBitmap(bm,originalHeight,originalWidth);
				break;
			default:
				result = new LoadedBitmap(bm,originalWidth,originalHeight);
				break;

		}
		sBitmapCache.put(bci, new WeakReference<Bitmap>(bm));
		LoadedBitmapInfo result_info = new LoadedBitmapInfo(result.getOriginalWidth(), result.getOriginalHeight());
		sLoadedBitmapCache.put(bm, result_info);
		return result;
    }
	public static LoadedBitmap loadBitmapThumb( String path, int maxWidth, int maxHeight, int maxSize, Bitmap.Config preferredConfig ) {
		Log.d(LOG_TAG, "loadBitmapThumb");
		CachedBitmapInfo bci = new CachedBitmapInfo(path, maxWidth, maxHeight, maxSize, preferredConfig, true);
		synchronized (sBitmapCacheLock) {

			LoadedBitmap ret = null;
			WeakReference<Bitmap> refExistingBitmap = sBitmapCache.get(bci);
			if (refExistingBitmap != null) {
				Bitmap existingBitmap = refExistingBitmap.get();
				if (existingBitmap != null) {
					LoadedBitmapInfo existingLoadedBitmapInfo = sLoadedBitmapCache.get(existingBitmap);
					if (existingLoadedBitmapInfo != null) {
						ret = new LoadedBitmap(existingBitmap, existingLoadedBitmapInfo.getOriginalWidth(), existingLoadedBitmapInfo.getOriginalHeight(), existingLoadedBitmapInfo.getLoadedType());
					}
				}
			}

			sCleanCacheCount++;
			if (sCleanCacheCount > 30) {
				sCleanCacheCount = 0;
				List<CachedBitmapInfo> toRemove = null;
				for (Map.Entry<CachedBitmapInfo, WeakReference<Bitmap>> entry : sBitmapCache.entrySet()) {
					if( entry.getValue().get()==null ) {
						if( toRemove==null ) {
							toRemove = new ArrayList<>();
						}
						toRemove.add(entry.getKey());
					}
				}
				if( toRemove!=null ) {
					for( CachedBitmapInfo e: toRemove ) {
						sBitmapCache.remove(e);
					}
				}
			}

			if(ret != null)
				return ret;
		}

		Bitmap bm = getThumbnail(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), path);
		if( bm==null ) {
			return new LoadedBitmap(null, 0, 0, 0);
		}
		int orientation = getThumbnailOrientation(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), path);

		Bitmap rotatedBitmap;
		LoadedBitmap result;

		switch( orientation ) {
			case 90:
				rotatedBitmap = rotateImage(bm, 90);
				result = new LoadedBitmap(rotatedBitmap,720,1280, 2);
				break;
			case 180:
				rotatedBitmap = rotateImage(bm, 180);
				result = new LoadedBitmap(rotatedBitmap,1280,720, 2);
				break;
			case 270:
				rotatedBitmap = rotateImage(bm, 270);
				result = new LoadedBitmap(rotatedBitmap,720,1280, 2);
				break;
			default:
				result = new LoadedBitmap(bm,1280,720, 2);
				break;
		}
		sBitmapCache.put(bci, new WeakReference<Bitmap>(bm));
		LoadedBitmapInfo result_info = new LoadedBitmapInfo(result.getOriginalWidth(), result.getOriginalHeight(), result.getLoadedType());
		sLoadedBitmapCache.put(bm, result_info);
		return result;
	}
	public static Bitmap rotateImage(Bitmap bitmap, int degrees) {
		if (degrees != 0 && bitmap != null) {
			Matrix m = new Matrix();
			m.setRotate(degrees, (float) bitmap.getWidth() / 2, (float) bitmap.getHeight() / 2);
			try {
				Bitmap temp = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), m, true);
				if (bitmap != temp) {
//					bitmap.recycle();		// XXX 
					bitmap = temp;
				}
			} catch (OutOfMemoryError e) {
				// We have no memory to rotate. Return the original bitmap.
				if(LL.E) Log.e(LOG_TAG, "rotateImage Error : " + e);
			}
		}
		return bitmap;
	}
    
	public static Bitmap rotateAndFlipImage(Bitmap bitmap, int degrees, boolean fliph, boolean flipv) {
		if ((degrees != 0 || fliph || flipv) && bitmap != null) {
			Matrix m = new Matrix();
			m.preRotate(degrees, (float) bitmap.getWidth() / 2, (float) bitmap.getHeight() / 2);
			m.preScale(fliph?-1:1, flipv?-1:1, (float) bitmap.getWidth() / 2, (float) bitmap.getHeight() / 2);
			try {
				Bitmap temp = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), m, true);
				if (bitmap != temp) {
//					bitmap.recycle();		// XXX 
					bitmap = temp;
				}
			} catch (OutOfMemoryError e) {
				// We have no memory to rotate. Return the original bitmap.
				if(LL.E) Log.e(LOG_TAG, "rotateImage Error : " + e);
			}
		}
		return bitmap;
	}
	
    
    public static void calcSampleSize(BitmapFactory.Options opt) {
    	
		final int JPEG_MAX_WIDTH = 1440; // 720 * 1.125
		final int JPEG_MAX_HEIGHT = 810; // 1280 * 1.125
		final int JPEG_MAX_SIZE = 1500000; // 1280 * 1.125
    	
    	int sampleSize = 1;
    	while( 	sampleSize < 8 && 
    			(( opt.outWidth / sampleSize > JPEG_MAX_WIDTH && opt.outHeight / sampleSize > JPEG_MAX_HEIGHT) || 
    			(( opt.outWidth / sampleSize * opt.outHeight / sampleSize > JPEG_MAX_SIZE) )))
    	{
    		sampleSize *= 2;
    	}	

    	opt.inSampleSize = sampleSize;
    }
    
    public static void calcSampleSize(BitmapFactory.Options opt, int maxWidth, int maxHeight, int maxSize) {
    	
    	int sampleSize = 1;
    	while( 	sampleSize < 8 && 
    			(( opt.outWidth / sampleSize > maxWidth && opt.outHeight / sampleSize > maxHeight) || 
    			(( opt.outWidth / sampleSize * opt.outHeight / sampleSize > (maxSize>0?maxSize:1500000)) )))
    	{
    		sampleSize *= 2;
    	}	

    	opt.inSampleSize = sampleSize;
    }
    
    
    public NexImage openFile(String path, int iThumb)
    {
    	if( path.startsWith("@solid:") && path.endsWith(".jpg")) {
    		int color = (int)Long.parseLong(path.substring(7, 15),16);
    		int w = 32;
    		int h = 18;
    		int[] pixels = new int[w*h];
    		for( int i=0; i<pixels.length; i++ )
    			pixels[i] = color;
    		return new NexImage(Bitmap.createBitmap(pixels, w, h, Bitmap.Config.ARGB_8888),w,h);
    	}else if( path.startsWith("@assetItem:") ){
			String ItemId = path.substring(11);
			if( m_effectResourceLoader!=null ) {
				try {

					Bitmap bm = NexImageLoader.loadBitmap(m_effectResourceLoader.openAssociatedFile(ItemId, null),m_jpegMaxWidth, m_jpegMaxHeight, m_jpegMaxSize).getBitmap();
					if( bm != null) {
						if(LL.D) Log.d(LOG_TAG,"@assetItem bitmap width=" + bm.getWidth() + " height=" + bm.getHeight() );
						int w	= bm.getWidth();
						w	= w & 0xFFFFFFFE;
						int h	= bm.getHeight();
						h	= h & 0xFFFFFFFE;
						return new NexImage(bm,w,h);
					}
					else
					{
						return null;
					}
				} catch (IOException e) {
				}

			}
			return null;
		}
        try
        {
//			BitmapFactory.Options opt = new BitmapFactory.Options();
//			opt.inJustDecodeBounds = true;
//			BitmapFactory.decodeFile(path, opt);
//			opt.inJustDecodeBounds = false;
//			if(LL.D) Log.d(LOG_TAG,"Image width=" + opt.outWidth + " height=" + opt.outHeight + " name='" + path + "'");
//			
//			NexImage.calcSampleSize(opt);
//			
//			/*final int MAX_WIDTH = 810; // 720 * 1.125
//			final int MAX_HEIGHT = 1440; // 1280 * 1.125
//			final int MAX_SIZE = 1500000; // 1280 * 1.125
//			
//			if( opt.outHeight > MAX_WIDTH && opt.outWidth > MAX_HEIGHT ) {
//				opt.inSampleSize = Math.min(opt.outHeight/MAX_HEIGHT, opt.outWidth/MAX_WIDTH) + 1;
//				if(LL.D) Log.d(LOG_TAG,"Downsampling image on w,h (inSampleSize=" + opt.inSampleSize + ")");
//			} else if( (opt.outHeight * opt.outWidth > MAX_SIZE ) ) {
//				opt.inSampleSize = (opt.outHeight * opt.outWidth)/MAX_SIZE + 1;
//				if(LL.D) Log.d(LOG_TAG,"Downsampling image on w*h>1500000 (inSampleSize=" + opt.inSampleSize + ")");
//			}*/
//			
//			mBitmap = BitmapFactory.decodeFile(path, opt);
        	LoadedBitmap lb = NexImageLoader.loadBitmap(path, m_jpegMaxWidth, m_jpegMaxHeight, m_jpegMaxSize, iThumb);
        	Bitmap bm = lb.getBitmap();
        	int loadedtype = lb.getLoadedType();
        	if( bm != null) {
    			if(LL.D) Log.d(LOG_TAG,"Actual bitmap width=" + bm.getWidth() + " height=" + bm.getHeight() + ", loadedtype=" + loadedtype);
	            int w	= bm.getWidth();
	            w	= w & 0xFFFFFFFE;
	            int h	= bm.getHeight();
	            h	= h & 0xFFFFFFFE;
	        	return new NexImage(bm,w,h,loadedtype);
        	}
        	else
        	{
        		return null;
        	}
        }
        catch (Exception e) { 
        	
        }
        return null;
    }

	public byte[] callbackReadAssetItemFile(String itemId, String path) {
		ItemInfo assetItem =  AssetPackageManager.getInstance().getInstalledItemById(itemId);
		if( assetItem==null ) {
			if( LL.D ) Log.d(LOG_TAG, "Error get assetItem id="+ itemId);
			return null;
		}
		AssetPackageReader reader;
		try {
			reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),assetItem.getPackageURI(),assetItem.getAssetPackage().getAssetId());
		} catch (IOException e) {
			if( LL.D ) Log.d(LOG_TAG, "Error making reader", e);
			return null;
		}
		String fullPath;
		if( path==null || path.length()<1 ) {
			fullPath = assetItem.getFilePath();
		} else {
			fullPath = relativePath(assetItem.getFilePath(),path);
		}
		ByteArrayOutputStream buf = new ByteArrayOutputStream();
		try {
			InputStream in = reader.openFile(fullPath);
			try {
				copy(in,buf);
			} finally {
				in.close();
			}
			return buf.toByteArray();
		} catch (IOException e) {
			if( LL.D ) Log.d(LOG_TAG, "Error reading file", e);
			return null;
		}
	}

	private static String relativePath( String a, String b ) {
		if( b.startsWith("..") || b.contains( "/..") )
			throw new SecurityException("Parent Path References Not Allowed");
		if(a.endsWith("/")) {
			return a + b;
		} else {
			int lastSlash = a.lastIndexOf('/');
			if( lastSlash<0 )
				return b;
			return a.substring(0,lastSlash+1) + b;
		}
	}

	private static void copy(InputStream input, OutputStream output) throws IOException {
		byte[] copybuf = new byte[1024*4];
		long count = 0;
		int n = 0;
		while (-1 != (n = input.read(copybuf))) {
			output.write(copybuf, 0, n);
			count += n;
		}
	}
	private static Bitmap getThumbnail(Context context, String path)
	{
		Cursor cursor = context.getContentResolver().query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, new String[] { MediaStore.MediaColumns._ID }, MediaStore.MediaColumns.DATA + "=?",
				new String[] { path }, null);
		if (cursor != null && cursor.moveToFirst())
		{
			int id = cursor.getInt(cursor.getColumnIndex(MediaStore.MediaColumns._ID));
			cursor.close();
			return MediaStore.Images.Thumbnails.getThumbnail(context.getContentResolver(), id, MediaStore.Images.Thumbnails.MINI_KIND, null);
		}

		cursor.close();
		return null;
	}
	private static int getThumbnailOrientation(Context context, String path)
	{
		Uri uri = null;
		int orientation = -1;
		Cursor cursor = context.getContentResolver().query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, new String[] { MediaStore.MediaColumns._ID }, MediaStore.MediaColumns.DATA + "=?",
				new String[] { path }, null);
		if (cursor != null && cursor.moveToFirst())
		{
			int id = cursor.getInt(cursor.getColumnIndex(MediaStore.MediaColumns._ID));
			uri = ContentUris.withAppendedId( MediaStore.Images.Media.EXTERNAL_CONTENT_URI, id );
			cursor.close();
		}
		if(uri != null) {
			String[] orientationColumn = {MediaStore.Images.Media.ORIENTATION};
			Cursor cur = context.getContentResolver().query(uri, orientationColumn, null, null, null);
			if (cur != null && cur.moveToFirst()) {
				orientation = cur.getInt(cur.getColumnIndex(orientationColumn[0]));
			}
		}
		return orientation;
	}


//    public int createImage(String path)
//    {
//        try
//        {
//        	if( mBitmap != null)
//        		mBitmap.recycle();
//        	// mBitmap = BitmapFactory.decodeStream(mAssetManager.open(path));
//        	return 0;
//        }
//        catch (Exception e) { }
//        return 1;
//    }    



//    public void close()
//    {
//    	if( mBitmap != null )
//    		mBitmap.recycle();
//    	mBitmap = null;
//    }

}

