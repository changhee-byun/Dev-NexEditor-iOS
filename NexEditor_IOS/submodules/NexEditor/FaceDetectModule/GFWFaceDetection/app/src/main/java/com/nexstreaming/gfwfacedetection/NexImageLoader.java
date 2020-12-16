package com.nexstreaming.gfwfacedetection;

/**
 * Created by yoosunghyun on 16/05/2017.
 */

import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.media.ExifInterface;

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

    public NexImageLoader( Resources res, OverlayPathResolver overlayPathResolver, int jpegMaxWidth, int jpegMaxHeight, int jpegMaxSize ) {
        if( res==null ) {
//			m_rsrc = null; yoon delete
            m_assetManager = null;
        } else {
//			m_rsrc = res; yoon delete
            m_assetManager = res.getAssets();
        }
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
            }
        }
        return b.toString();
    }

    public static class LoadedBitmapInfo{

        private int originalWidth;
        private int originalHeight;
        private LoadedBitmapInfo(int originalWidth, int originalHeight) {
            this.originalWidth = originalWidth;
            this.originalHeight = originalHeight;
        }
        public int getOriginalWidth() {
            return originalWidth;
        }
        public int getOriginalHeight() {
            return originalHeight;
        }
    }

    public static class LoadedBitmap {
        private Bitmap bitmap;
        private int originalWidth;
        private int originalHeight;
        private LoadedBitmap( Bitmap bitmap, int originalWidth, int originalHeight ) {
            this.originalWidth = originalWidth;
            this.originalHeight = originalHeight;
            this.bitmap = bitmap;
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
        return loadBitmap(path, maxWidth, maxHeight, Integer.MAX_VALUE);
    }

    public static LoadedBitmap loadBitmap( InputStream stream, int maxWidth, int maxHeight, int maxSize ) {
        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inJustDecodeBounds = true;
        stream.mark(Integer.MAX_VALUE);
        BitmapFactory.decodeStream(stream, null, opt);
        try {
            stream.reset();
        } catch (IOException e) {
            throw new RuntimeException("Failed to reset stream",e);
        }
        opt.inJustDecodeBounds = false;
        opt.inPreferredConfig = Bitmap.Config.RGB_565;
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


        Bitmap bm = BitmapFactory.decodeStream(stream, null, opt);
        if( bm==null ) {
            return new LoadedBitmap(null, 0, 0);
        }

        return new LoadedBitmap(bm,originalWidth,originalHeight);
    }

    public static LoadedBitmap loadBitmap( String path, int maxWidth, int maxHeight, int maxSize ) {
        return loadBitmap(path,maxWidth,maxHeight,maxSize,null);
    }

    private static class CachedBitmapInfo {
        final String path;
        final int maxWidth;
        final int maxHeight;
        final int maxSize;
        final Bitmap.Config preferredConfig;

        private CachedBitmapInfo(String path, int maxWidth, int maxHeight, int maxSize, Bitmap.Config preferredConfig) {
            this.path = path==null?"":path;
            this.maxWidth = maxWidth;
            this.maxHeight = maxHeight;
            this.maxSize = maxSize;
            this.preferredConfig = preferredConfig;
        }

        @Override
        public boolean equals(Object o) {
            if( o==null || !(o instanceof CachedBitmapInfo) )
                return false;
            if( o==this )
                return true;
            CachedBitmapInfo other = (CachedBitmapInfo)o;
            return maxWidth == other.maxWidth && maxHeight == other.maxHeight && maxSize == other.maxSize && preferredConfig == other.preferredConfig && path.equals(other.path);
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

    public static LoadedBitmap loadBitmap( String path, int maxWidth, int maxHeight, int maxSize, Bitmap.Config preferredConfig ) {

        CachedBitmapInfo bci = new CachedBitmapInfo(path, maxWidth, maxHeight, maxSize, preferredConfig);
        synchronized (sBitmapCacheLock) {

            LoadedBitmap ret = null;
            WeakReference<Bitmap> refExistingBitmap = sBitmapCache.get(bci);
            if (refExistingBitmap != null) {
                Bitmap existingBitmap = refExistingBitmap.get();
                if (existingBitmap != null) {
                    LoadedBitmapInfo existingLoadedBitmapInfo = sLoadedBitmapCache.get(existingBitmap);
                    if (existingLoadedBitmapInfo != null) {

                        ret = new LoadedBitmap(existingBitmap, existingLoadedBitmapInfo.getOriginalWidth(), existingLoadedBitmapInfo.getOriginalHeight());
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
    public static void clearBitmapCache(){
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
