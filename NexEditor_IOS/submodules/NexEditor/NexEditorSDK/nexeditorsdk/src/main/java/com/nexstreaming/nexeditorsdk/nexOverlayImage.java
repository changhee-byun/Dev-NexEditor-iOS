/******************************************************************************
 * File Name        : nexOverlayImage.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdk;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Rect;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.overlay.OverlayAsset;
import com.nexstreaming.app.common.nexasset.overlay.OverlayAssetFactory;
import com.nexstreaming.app.common.util.FileType;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.nexeditorsdk.exception.ClipIsNotVideoException;
import com.nexstreaming.nexeditorsdk.exception.NotSupportedAPILevel;
import com.nexstreaming.nexeditorsdk.exception.nexSDKException;

import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

/**
 * This class is used to define and manage overlay images in the NexEditor&trade;&nbsp;SDK.
 * 
 * In the context of the NexEditor&trade;&nbsp;SDK, an overlay image is any image that will be overlaid
 * onto video or images in a NexEditor&trade;&nbsp; project, such as for example stickers.  This class 
 * can be used to add new overlay images to be used as stickers within the NexEditor&trade;&nbsp;SDK.
 * 
 * To use an overlay image:
 * <ol>
 *  <li> The overlay image item must first be created with either <tt> nexOverlayImage(String itemId, Bitmap bitmap)</tt> or <tt> nexOverlayImage(String itemId, int resourceId)</tt> for images that will be used more than once. </li>
 *  <li> The created overlay image instance must be registered by calling <tt> registerOverlayImage()</tt>. </li>
 *  <li> To use an overlay item, it can be called with the overlay image ID set when it is registered. </li>
 *  <li> When finished, the overlay image should be unregistered by calling <tt> unregisterOverlayImage()</tt>. </li>
 * </ol>
 * <p>Example code :</p>
   {@code
        nexOverlayImage overlay = new nexOverlayImage( "testOverlay",R.drawable.myavatar_17004763_1);
        nexOverlayImage.registerOverlayImage(overlay);
   }

 * @see {@link com.nexstreaming.nexeditorsdk.nexOverlayItem nexOverlayItem}
 * @since version 1.1.0
 */
public class nexOverlayImage implements Cloneable {
    private static final String TAG = "nexOverlayImage";
    private String mId;

    private String mBitmapPath;
    private runTimeMakeBitMap mMakeBitMap;
    private int mSolidColor;
    private VideoClipInfo mVideoClipInfo;
    protected int mResourceId;
    private int mWidth;
    private int mHeight;

    private int mType;

    private int mBitmapInSample = 1;
    private int mBitmapWidth;
    private int mBitmapHeight;

    private boolean mUpdateInfo;
    private static Map<String, nexOverlayImage> sOverlayImageItems;


    private boolean mAssetManager;
    private OverlayAsset cachedOverlayAsset;

    private int anchorPoint = nexOverlayItem.AnchorPoint_MiddleMiddle;

    public static final int kOverlayType_ResourceImage = 1;

    public static final int kOverlayType_UserImage = 2;

    public static final int kOverlayType_UserVideo = 3;

    public static final int kOverlayType_RunTimeImage = 4;

    public static final int kOverlayType_SolidColorImage = 5;

    public static final int kOverlayType_Asset = 6;

    protected boolean mUpdate = false;

    protected static nexOverlayImage clone(nexOverlayImage src) {
        //Log.d("clone", "clone work./nexOverlayImage");
        nexOverlayImage object = null;
        try {
            object = (nexOverlayImage) src.clone();
            if (src.mVideoClipInfo != null) {
                object.mVideoClipInfo = VideoClipInfo.clone(src.mVideoClipInfo);
            }
            object.mId = src.mId;
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }

        return object;
    }

    /**
     * This interface allows the developer to make bitmap in runtime. 
     * This interface is used by {@link #com.nexstreaming.nexeditorsdk.nexOverlayImage nexOverlayImage}(String, #runTimeMakeBitMap) as parameter <tt>runTimeMakeBitMap</tt>.
     * 
     * The following sample code demonstrates internal process :
     * <p>Example code :</p>
     * {@code if( isAniMate() ){
     * for( ; ; ){
     * getBitmapID();
     * makeBitmap()
     * }
     * } else {
     * getBitmapID();
     * makeBitmap()
     * }
     * }
     *
     * @see #nexOverlayImage(String, com.nexstreaming.nexeditorsdk.nexOverlayImage.runTimeMakeBitMap)
     * @since version 1.3.0
     */
    public interface runTimeMakeBitMap {

    
        /**
         * This method returns whether the runtime bitmap is for an animation or a still image by the return value, to the <tt>nexEngine</tt>.
         * <p/>
         * If returned value is <tt>FALSE</tt>, <tt>getBitmapID()</tt>and <tt>makeBitmap()</tt> will be called only once.
         *
         * @return <tt>TRUE</tt> if the bitmap is for an animation; <tt>FALSE</tt> if the bitmap is for a still image.
         * @since version 1.3.3
         */
        public boolean isAniMate();


        /**
         * This method gets the bitmap ID of an image to be used by the method <tt>makeBitmap()</tt>.
         * <p/>
         * The developer should assign IDs to every bitmaps and the IDs should not be repeated with other image IDs or android resource IDs.
         * When making bitmaps for an animation, every bitmaps of the animation should have different IDs.
         * Every ID should be a number bigger than 2.
         *
         * @return The ID of a bitmap to be used by <tt>makeBitmap()</tt>.
         * @since version 1.3.3
         */
        public int getBitmapID();

        /**
         * This method makes the bitmap with the ID from the method <tt>getBitmapID()</tt> to make an overlay image.
         *
         * @return The bitmap to be used as an overly image.
         * @since version 1.3.3
         */
        public Bitmap makeBitmap();
    }

    static boolean registerOverlayImage(nexOverlayImage overlayImage) {
        if( sOverlayImageItems == null ){
            sOverlayImageItems = new HashMap<>();
        }
        if (sOverlayImageItems.containsKey(overlayImage.getId())) {
            return false;
        }
        sOverlayImageItems.put(overlayImage.getId(), overlayImage);
        return true;
    }

    static boolean unregisterOverlayImage(String overlayImageId) {
        if( sOverlayImageItems == null ){
            sOverlayImageItems = new HashMap<>();
        }

        sOverlayImageItems.remove(overlayImageId);
        return true;
    }

    static void allClearRegisterOverlayImage() {
        if( sOverlayImageItems == null ){
            sOverlayImageItems = new HashMap<>();
        }

        sOverlayImageItems.clear();
    }

    static nexOverlayImage getOverlayImage(String overlayImageId) {
        if( sOverlayImageItems == null ){
            sOverlayImageItems = new HashMap<>();
        }

        return sOverlayImageItems.get(overlayImageId);
    }

    /**
     * This method gets the ID of an overlay image item, as a <tt> String</tt>.
     * <p/>
     * The ID of an overlay image item can be used to retrieve the image, or to unregister the overlay image item when finished with it.
     * <p/>
     * <p>Example code :</p>
     *      {@code    nexOverlayImage overlay = new nexOverlayImage( "testOverlay",this,R.drawable.ilove0001);
     * String overlayid = overlay.getId();
     * }
     * 
     * @return The ID of the overlay image item, set when the item was created, as a <tt> String</tt>.
     * @see #getOverlayImage
     * @see #unregisterOverlayImage
     * @since version 1.1.0
     */
    public String getId() {
        return mId;
    }

    private int getType() {
        if( mType == 0 ) {

            if( mAssetManager ) {
                mType = kOverlayType_Asset;
                return mType;
            }

            if (mMakeBitMap != null) {
                mType = kOverlayType_RunTimeImage;
                return mType;
            }

            if (mVideoClipInfo != null) {
                mType = kOverlayType_UserVideo;
                return mType;
            }

            if (mResourceId != 0) {
                mType = kOverlayType_ResourceImage;
                return mType;
            }

            if (mSolidColor != 0) {
                mType = kOverlayType_SolidColorImage;
                return mType;
            }

            if (mBitmapPath != null) {
                mType = kOverlayType_UserImage;
                return mType;
            }

        }
        return mType;
    }


    nexOverlayImage(String itemId, int width, int height, runTimeMakeBitMap makeBitmap) {
        mId = itemId;
        mResourceId = 0;
        mMakeBitMap = makeBitmap;
        mBitmapPath = null;
        //TODO: internal
        mWidth = width;
        mHeight = height;
        mUpdateInfo = true;

        mBitmapWidth = mWidth;
        mBitmapHeight = mHeight;
        mVideoClipInfo = null;
        mSolidColor = 0;
    }

    nexOverlayImage(String itemId, int width, int height, String path) {
        mId = itemId;
        mResourceId = 0;
        mMakeBitMap = null;
        mBitmapPath = path;
        //TODO: internal
        mWidth = width;
        mHeight = height;
        mUpdateInfo = true;

        mBitmapWidth = mWidth;
        mBitmapHeight = mHeight;
        mVideoClipInfo = null;
        mSolidColor = 0;
        initSample();
    }

    nexOverlayImage(String itemId) {
        mId = itemId;
        mResourceId = 0;
        mMakeBitMap = null;
        mBitmapPath = null;
        //TODO: internal
        mUpdateInfo = false;

        mBitmapWidth = mWidth;
        mBitmapHeight = mHeight;
        mVideoClipInfo = null;
        mSolidColor = 0;
    }

    nexOverlayImage(String itemId, boolean assetManager) {
        mId = itemId;
        mResourceId = 0;
        mMakeBitMap = null;
        mBitmapPath = null;
        //TODO: internal
        mUpdateInfo = false;

        mBitmapWidth = mWidth;
        mBitmapHeight = mHeight;
        mVideoClipInfo = null;
        mSolidColor = 0;
        mAssetManager = assetManager;
    }

    /**
     * This method creates an instance of an overlay image from a bitmap path.
     * <p/>
     * Because this method of creating an overlay image requires a lot of memory,
     * this method should be used to create overlay images that will only be used once.
     * <p/>
     * <p>Example code :</p>
     *      {@code    nexOverlayImage makeKmOverlayImage(String imageId, String bitmapPath, int width, int height)
     * {
     * nexOverlayImage image = new nexOverlayImage(imageId, bitmapPath);
     * image.setCrop(width, height);
     * return image;
     * }
     * }
     *
     * @param itemId
     * @param bitmapPath
     * @throws NotSupportedAPILevel nexEditorSDK API Limited Level is {@value EditorGlobal#kOverlayImageLimited}
     * @see #nexOverlayImage(String itemId, String bitmapPath)
     * @since version 1.1.0
     */

    public nexOverlayImage(String itemId, String bitmapPath) {
        if (EditorGlobal.ApiLevel < EditorGlobal.kOverlayImageLimited) {
            mId = itemId;

            mResourceId = 0;
            mMakeBitMap = null;
            mBitmapPath = bitmapPath;

            mUpdateInfo = true;

            mBitmapWidth = mWidth;
            mBitmapHeight = mHeight;
            mVideoClipInfo = null;
            mSolidColor = 0;

            FileType fileType = FileType.fromFile(bitmapPath);
            if( fileType!=null && fileType.isImage()  ) {
                mBitmapPath = bitmapPath;
                initSample();
            }else{
                mBitmapPath = null;
                nexClip clip = nexClip.getSupportedClip(bitmapPath);
                if( clip == null ){
                    throw new ClipIsNotVideoException();
                }

                if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
                    mVideoClipInfo = new VideoClipInfo();
                    mVideoClipInfo.mHasAudio = clip.hasAudio();
                    mVideoClipInfo.mHasVideo = clip.hasVideo();
                    mVideoClipInfo.mPath = clip.getRealPath();
                    mVideoClipInfo.mTotalTime = clip.getTotalTime();
                    mVideoClipInfo.mWidth = clip.getWidth();
                    mVideoClipInfo.mHeight = clip.getHeight();
                }else{
                    throw new ClipIsNotVideoException();
                }
            }

        } else {
            throw new NotSupportedAPILevel();
        }
    }

    /**
     * This method creates an overlay image from an Android image resource ID.
     * <p/>
     * This method should be used to create overlay images such as stickers and other images created from the registered presets.
     * <p/>
     * <p>Example code :</p>
     *      {@code
     * nexOverlayImage overlay = new nexOverlayImage( "testOverlay",this,R.drawable.myavatar_17004763_1);
     * }
     *
     * @param itemId     The designated ID to use for the instance of the overlay image item to be created, as a <tt> String</tt>.
     * @param context    The Android Application Context.
     * @param resourceId The Android image resource ID of the overlay image to be created, as an <tt>integer</tt>.
     * @throws NotSupportedAPILevel nexEditorSDK API Limited Level is {@value EditorGlobal#kOverlayImageLimited}
     * @see #nexOverlayImage(String, Context, int)
     * @since version 1.1.0
     */
    public nexOverlayImage(String itemId, Context context, int resourceId) {
        if (EditorGlobal.ApiLevel < EditorGlobal.kOverlayImageLimited) {
            mId = itemId;
            mResourceId = resourceId;

            mUpdateInfo = true;

            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeResource(context.getApplicationContext().getResources(),mResourceId,options);
            mWidth = options.outWidth;
            mHeight = options.outHeight;

            mMakeBitMap = null;

            mBitmapPath = null;
            mBitmapWidth = 0;
            mBitmapHeight = 0;
            mVideoClipInfo = null;
            mSolidColor = 0;
        } else {
            throw new NotSupportedAPILevel();
        }
    }

    /**
     * This method creates an overlay image from the bitmap added by the developer, using the interface <tt>runTimeMakeBitMap</tt>.
     * This method should be used to create overlay images such as stickers and other images created from the registered presets.
     *
     * @param itemId     The designated ID to use for the instance of the overlay image item to be created, as a <tt> String</tt>.
     * @param makeBitmap The bitmap image made by the developer with <tt>runTimeMakeBitMap</tt>.
     *                   <p>Example code :</p>
     *                    {@code public class TextOverlay implements nexOverlayImage.runTimeMakeBitMap {
     *                   private String mText;
     *                   private Context mContext;
     *                   public TextOverlay(Context context, String text){
     *                   mText = text;
     *                   mContext = context;
     *                   }
     *
     *                   public boolean isAniMate() {
     *                   return false;
     *                   }
     *
     *                   public int getBitmapID() {
     *                   return 2;
     *                   }
     *
     *                   public Bitmap makeBitmap() {
     *                   myView vm = new myView(mContext);
     *
     *                   Bitmap bitmap = Bitmap.createBitmap(200, 200, Bitmap.Config.ARGB_8888);
     *                   Canvas c = new Canvas(bitmap);
     *                   vm.draw(c);
     *                   return bitmap;
     *                   }
     *                   ......
     *                   }
     *
     *                   nexOverlayImage textOverlay = new nexOverlayImage( "text", new TextOverlay(this,"Nexstreaming"));
     *
     *                   }
     * @throws NotSupportedAPILevel nexEditorSDK API Limited Level is {@value EditorGlobal#kOverlayImageLimited}
     * @since version 1.3.0
     */
    public nexOverlayImage(String itemId, runTimeMakeBitMap makeBitmap) {
        if (EditorGlobal.ApiLevel < EditorGlobal.kOverlayImageLimited) {
            mId = itemId;
            mResourceId = 0;
            mMakeBitMap = makeBitmap;

            mBitmapPath = null;
            mBitmapWidth = 0;
            mBitmapHeight = 0;
            mVideoClipInfo = null;
            mSolidColor = 0;
        } else {
            throw new NotSupportedAPILevel();
        }
    }

    /**
     * Constructor of the <tt>nexOverlayImage</tt>.
     * <p/>
     * <p>Example code :</p>
     *      {@code    nexClip clip = nexClip.getSupportedClip(filepath);
     * id = mSelectedPIP.size();
     * nexOverlayImage ov = new nexOverlayImage("video"+id,clip);
     * }
     *
     * @param itemId The designated ID of an overlay image instance to be created, as a <tt>String</tt>.
     * @param clip   The media content to be added as an overlay image.
     * @throws ClipIsNotVideoException
     * @throws NotSupportedAPILevel    nexEditorSDK API Limited Level is {@value EditorGlobal#kOverlayVideoLimited}
     * @since version 1.3.52
     */
    public nexOverlayImage(String itemId, nexClip clip) {
        if (EditorGlobal.ApiLevel < EditorGlobal.kOverlayVideoLimited) {
            mId = itemId;
            mResourceId = 0;
            mMakeBitMap = null;
            mBitmapWidth = 0;
            mBitmapHeight = 0;

            if (clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                mVideoClipInfo = new VideoClipInfo();
                mVideoClipInfo.mHasAudio = clip.hasAudio();
                mVideoClipInfo.mHasVideo = clip.hasVideo();
                mVideoClipInfo.mPath = clip.getRealPath();
                mVideoClipInfo.mTotalTime = clip.getTotalTime();
                mVideoClipInfo.mWidth = clip.getWidth();
                mVideoClipInfo.mHeight = clip.getHeight();

                mSolidColor = 0;
                mBitmapPath = null;

            }else if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ){
                if( clip.isSolid() ){
                    mVideoClipInfo = null;
                    mSolidColor = clip.getSolidColor();
                    mBitmapPath = null;
                }else{
                    mSolidColor = 0;
                    mVideoClipInfo = null;
                    mBitmapPath = clip.getRealPath();
                    initSample();
                }
            }else{

                mVideoClipInfo = null;
                mSolidColor = 0;
                mBitmapPath = null;
                throw new nexSDKException("Audio not supported!");
            }

        } else {
            mVideoClipInfo = null;
            mSolidColor = 0;
            mBitmapPath = null;
            throw new NotSupportedAPILevel();
        }
    }

    /**
     *
     * @param itemId
     * @param argbColor
     * @since 1.7.43
     */
    public nexOverlayImage(String itemId, int argbColor){
        mId = itemId;
        mResourceId = 0;
        mMakeBitMap = null;
        mWidth = 32;
        mHeight = 18;
        mBitmapPath = null;
        mBitmapWidth = 32;
        mBitmapHeight = 18;
        mSolidColor = argbColor;
        mVideoClipInfo = null;
        mUpdateInfo = true;
    }

    /**
     * This method gets the resource ID of an overlay image, if the overlay image was made with a resource ID.
     * <p/>
     * <p>Example code :</p>
     *      {@code    nexOverlayImage overlay = new nexOverlayImage("testOverlay",DirectExportTestActivity.this, R.drawable.ilove0001);
     * int rID = overlay.getResourceId();
     * }
     *
     * @return The resource ID if the overlay image was made with {@link #nexOverlayImage(String, Context, int)}; otherwise 0.
     * @since version 1.3.0
     */
    public int getResourceId() {
        return mResourceId;
    }

    boolean isAniMate() {
        if (mMakeBitMap != null) {
            if (mMakeBitMap.isAniMate()) {
                return true;
            }
        }
        return false;
    }

    private void updateInfo() {
        if (!mUpdateInfo) {
            int type = getType();
            Bitmap ret = null;
            switch (type){
                case kOverlayType_Asset:
                    getUserBitmap();

                    mWidth = cachedOverlayAsset.getIntrinsicWidth();
                    mHeight = cachedOverlayAsset.getIntrinsicHeight();
                    break;
                case kOverlayType_ResourceImage:
                    break;
                case kOverlayType_SolidColorImage:
                    break;
                case kOverlayType_UserVideo:
                    //TODO: thumbnail?
                    mWidth = mVideoClipInfo.mWidth;
                    mHeight = mVideoClipInfo.mHeight;
                    mUpdateInfo = true;
                    break;
                case kOverlayType_RunTimeImage:
                case kOverlayType_UserImage:
                default:
                    Bitmap bm = getUserBitmap();
                    mWidth = bm.getWidth();
                    mHeight = bm.getHeight();
                    break;
            }
            mUpdateInfo = true;
        }
    }

    /**
     * This method gets the width of a clip.
     *
     * @return clip width
     * @see #getHeight()
     * @since version 1.5.15
     */
    public int getWidth() {
        updateInfo();
        return mWidth;
    }

    /**
     * This method gets the height of a clip.
     *
     * @return clip height
     * @see #getWidth()
     * @since version 1.5.15
     */
    public int getHeight() {
        updateInfo();
        return mHeight;
    }

    void getBound(Rect bound){
        updateInfo();
        bound.left = 0 - mWidth/2;
        bound.top = 0 - mHeight/2;
        bound.right = 0 + mWidth/2;
        bound.bottom = 0 + mHeight/2;
    }

    OverlayAsset getOverlayAssetBitmap() throws IOException, XmlPullParserException {
        if( mAssetManager ) {
            if (cachedOverlayAsset == null) {
                cachedOverlayAsset = OverlayAssetFactory.forItem(mId);
            }
            return cachedOverlayAsset;
        }
        return  null;
    }

    protected Bitmap getUserBitmap() {
        int type = getType();
        Bitmap ret = null;
        switch (type){
            case kOverlayType_Asset:
                try {
                    getOverlayAssetBitmap();

                } catch (IOException e) {
                    e.printStackTrace();

                } catch (XmlPullParserException e) {
                    e.printStackTrace();

                }
                break;
            case kOverlayType_ResourceImage:
                ret = BitmapFactory.decodeResource(KineMasterSingleTon.getApplicationInstance().getApplicationContext().getResources(), mResourceId);
                break;
            case kOverlayType_SolidColorImage:
                ret = getSolidRect();
                break;
            case kOverlayType_UserVideo:
                //TODO: thumbnail?
                break;
            case kOverlayType_RunTimeImage:
                ret = mMakeBitMap.makeBitmap();
                break;
            case kOverlayType_UserImage:
            default:
                if(mBitmapWidth == 0)
                    mBitmapWidth = 1280;

                if(mBitmapHeight == 0)
                    mBitmapHeight = 720;

                ret = decodeSampledBitmapFromFile(mBitmapPath, mBitmapWidth, mBitmapHeight);
            break;
        }
        return ret;
    }

    /**
     * This method crops to resize an overlay image.
     * <p/>
     * <p>Example code :</p>
     * <pre>
     *      {@code    nexOverlayImage image = new nexOverlayImage(imageId, bitmapPath);
     * image.setCrop(width, height);
     * }
     * </pre>
     *
     * @param width  The width of the overlay image in <tt>bitmap</tt> as an <tt>integer</tt>.
     * @param height The height of the overlay image in <tt>bitmap</tt> as an <tt>integer</tt>.
     * @since version 1.3.38
     */
    @Deprecated
    public void setCrop(int width, int height) {
        if( getType() == kOverlayType_UserImage ) {
            mBitmapWidth = width;
            mBitmapHeight = height;
        }
    }

    @Deprecated
    public void resizeBitmap(int width, int height){
        if( getType() == kOverlayType_UserImage ) {
            mBitmapWidth = width;
            mBitmapHeight = height;
        }
    }

    protected String getUserBitmapID() {
        if( getType() == kOverlayType_RunTimeImage ) {
            if (mMakeBitMap != null)
                return mId + mMakeBitMap.getBitmapID();
        }

        if( getType() == kOverlayType_UserImage ) {
            if (mBitmapPath != null) {
                return mId + mBitmapPath.hashCode();
            }
        }
        return mId;
    }

    private int calculateInSampleSize(BitmapFactory.Options options, int reqWidth, int reqHeight) {
        // Raw height and width of image
        final int height = options.outHeight;
        final int width = options.outWidth;
        int inSampleSize = 1;

        if (height > reqHeight || width > reqWidth) {

            // Calculate ratios of height and width to requested height and width
            final int heightRatio = Math.round((float) height / (float) reqHeight);
            final int widthRatio = Math.round((float) width / (float) reqWidth);

            // Choose the smallest ratio as inSampleSize value, this will guarantee
            // a final image with both dimensions larger than or equal to the
            // requested height and width.
            inSampleSize = heightRatio < widthRatio ? heightRatio : widthRatio;
        }

        return inSampleSize;
    }

    private Bitmap decodeSampledBitmapFromFile(String path, int reqWidth, int reqHeight) {
        // First decode with inJustDecodeBounds=true to check dimensions
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = mBitmapInSample;
        options.inJustDecodeBounds = false;
        Bitmap org = BitmapFactory.decodeFile(path, options);
        if( org != null ){
            if( org.getWidth() != reqWidth && org.getHeight() != reqHeight ){
                int w,h;
                float heightRatio = (float) org.getWidth() / (float) reqHeight;
                float widthRatio = (float) org.getHeight() / (float) reqWidth;

                if( widthRatio > heightRatio ){
                    w = Math.round(widthRatio*org.getWidth());
                    h = Math.round(widthRatio*org.getHeight());
                }else{
                    w = Math.round(heightRatio*org.getWidth());
                    h = Math.round(heightRatio*org.getHeight());
                }

                Bitmap ret =  Bitmap.createScaledBitmap(org, w, h, true);
                return  ret;
            }else{
                return org;
            }
        }
        return null;
    }

    /**
     * This method clears the cache, where the bitmap images made with {@link #nexOverlayImage(String, com.nexstreaming.nexeditorsdk.nexOverlayImage.runTimeMakeBitMap)}
     * are saved, to free up storage space.
     * <p/>
     * <p>Example code :</p>
     *      {@code    nexOverlayImage image = new nexOverlayImage(imageId, bitmapPath);
     * ...
     * image.releaseBitmap();
     * }
     *
     * @since version 1.3.0
     */
    public void releaseBitmap() {

    }

    boolean isVideo() {

        if( getType() == kOverlayType_UserVideo ) {
            return true;
        }
        return false;
    }

    boolean isUpdated() {
        return mUpdate;
    }


    /**
     * This method gets information of a video clip.
     * The information includes details on the video content such as width, height, size and etc.
     * <p/>
     * <p>Example code :</p>
     *      {@code
     * visualClip.mTotalTime = item.getOverlayImage().getVideoClipInfo().getTotalTime();
     * }
     *
     * @return VideoClipInfo
     * @since version 1.5.0
     */
    public VideoClipInfo getVideoClipInfo() {
        return mVideoClipInfo;
    }

    /**
     * The information of a video clip.
     * The information includes details on the video content such as width, height, size and etc.
     * <p/>
     * <p>Example code :</p>
     *      {@code    mVideoClipInfo = new VideoClipInfo();
     * mVideoClipInfo.mHasAudio = clip.hasAudio();
     * mVideoClipInfo.mHasVideo = clip.hasVideo();
     * mVideoClipInfo.mPath = clip.getPath();
     * mVideoClipInfo.mTotalTime = clip.getTotalTime();
     * mVideoClipInfo.mWidth = clip.getWidth();
     * mVideoClipInfo.mHeight = clip.getHeight();
     * }
     *
     * @since version 1.5.0
     */
    public static class VideoClipInfo implements Cloneable {
        private boolean mHasVideo;
        private boolean mHasAudio;
        private String mPath;
        private int mTotalTime;
        private int mWidth;
        private int mHeight;


        /**
         * This method checks whether a video content is assigned to a specific clip.
         *
         * @return <tt>TRUE</tt> if a video exists, otherwise <tt>FALSE</tt>.
         * @see #hasAudio()
         * @since version 1.5.0
         */
        public boolean hasVideo() {
            return mHasVideo;
        }

        /**
         * This method checks whether an audio content is assigned to a specific clip.
         *
         * @return <tt>TRUE</tt> if an audio exists, otherwise <tt>FALSE</tt>.
         * @see #hasVideo()
         * @since version 1.5.0
         */
        public boolean hasAudio() {
            return mHasAudio;
        }

        /**
         * This method gets the absolute path of the original video content of a video clip.
         *
         * @return The absolute path of the video content.
         * @since version 1.5.0
         */
        public String getPath() {
            return mPath;
        }

        /**
         * This method gets the duration of a video clip.
         *
         * @return The duration of the video clip.
         * @since version 1.5.0
         */
        public int getTotalTime() {
            return mTotalTime;
        }

        /**
         * This method gets the width of a video clip.
         *
         * @return The width of the video clip.
         * @see #getHeight()
         * @since version 1.5.0
         */
        public int getWidth() {
            return mWidth;
        }

        /**
         * This method gets the height of a video clip.
         *
         * @return The height of the video clip.
         * @see #getWidth()
         * @since version 1.5.0
         */
        public int getHeight() {
            return mHeight;
        }

        protected static VideoClipInfo clone(VideoClipInfo src) {
            //Log.d("clone", "clone work./VideoClipInfo");
            VideoClipInfo object = null;
            try {
                object = (VideoClipInfo) src.clone();
                object.mPath = src.mPath;
            } catch (CloneNotSupportedException e) {
                e.printStackTrace();
            }
            return object;
        }
    }

    boolean isAssetManager() {
        if( getType() == kOverlayType_Asset ){
            return true;
        }
        return false;
    }

    private Bitmap getSolidRect() {
        int color = mSolidColor;
        int w = 32;
        int h = 18;
        int[] pixels = new int[w * h];
        for (int i = 0; i < pixels.length; i++)
            pixels[i] = color;
        return Bitmap.createBitmap(pixels, w, h, Bitmap.Config.ARGB_8888);
    }

    private void initSample(){
        if( mBitmapPath != null ) {
            final BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;

            BitmapFactory.decodeFile(mBitmapPath, options);
            int w = 1280;
            int h = 720;
            if( options.outHeight > options.outWidth ){
                w = 720;
                h = 1280;
            }
            //Log.d(TAG, "resizeFitUserImage1 input=" + width + "X" + height + ", output=" + options.outWidth + "X" + options.outHeight);
            // Calculate inSampleSize
            options.inSampleSize = calculateInSampleSize(options, w, h);
            mBitmapInSample = options.inSampleSize;
            BitmapFactory.decodeFile(mBitmapPath, options);
            //Log.d(TAG, "resizeFitUserImage2 inSampleSize=" + options.inSampleSize + ", output=" + options.outWidth + "X" + options.outHeight);
            mWidth = options.outWidth;
            mHeight = options.outHeight;
            mBitmapWidth = mWidth;
            mBitmapHeight = mHeight;
        }
    }

    public void setAnchorPoint(int point){
        anchorPoint = point;
    }

    public int getAnchorPoint(){
        return anchorPoint;
    }

    public int mRecommandDuration = -1;

    /**
     *
     * @return 0 - static overlay ( only one frame ) , others animation overlay ( ms )
     * @since 2.0.14
     */
    public int getDefaultDuration(){
        if( mRecommandDuration < 0 ) {
            int type = getType();
            int duration = 0;
            switch (type) {
                case kOverlayType_Asset:
                    try {
                        OverlayAsset asset = getOverlayAssetBitmap();
                        duration = asset.getDefaultDuration();
                    } catch (IOException e) {
                        e.printStackTrace();
                    } catch (XmlPullParserException e) {
                        e.printStackTrace();
                    }
                    break;
            }
            mRecommandDuration = duration;
        }
        return mRecommandDuration;
    }

}
