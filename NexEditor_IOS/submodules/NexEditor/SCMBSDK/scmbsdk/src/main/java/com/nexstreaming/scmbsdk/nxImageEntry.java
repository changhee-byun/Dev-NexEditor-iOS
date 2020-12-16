package com.nexstreaming.scmbsdk;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Locale;

/**
 * Image Entry Class
 * @see nxSCMBSDK#getImageEntries()
 * @see nxSCMBSDK#setImageEntries(nxImageEntry[])
 */
public class nxImageEntry extends nxBytesEntry {
    private static final String TAG="nxImageEntry";
    /**
     * make Image Entry
     *
     * @param imageFilePath Jpeg Image path.
     * @param sampleWidth sampling width ( 0 value was set FHD resolution )
     * @param sampleHeight sampling height ( 0 value was set FHD resolution )
     * @return nxImageEntry
     * @throws FileNotFoundException imageFilePath was not exist or was not image file.
     */
    public static nxImageEntry createEntry(String imageFilePath,int sampleWidth, int sampleHeight) throws FileNotFoundException {
        int maxSize = 2073600;

        int maxWidth = sampleWidth <= 0 ? 1920: sampleWidth;
        int maxHeight = sampleHeight <= 0 ? 1080 : sampleHeight;

        maxSize = maxWidth * sampleHeight;

        File f = new File(imageFilePath);
        if( !f.isFile() ){
            throw new FileNotFoundException(imageFilePath+" was not exist file.");
        }

        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inJustDecodeBounds = true;

        BitmapFactory.decodeFile(imageFilePath,opt);
        opt.inJustDecodeBounds = false;

        if( opt.outWidth <= 0 || opt.outHeight <= 0 ){
            throw new FileNotFoundException(imageFilePath+" was not image file.");
        }

        if( opt.outWidth < sampleWidth && opt.outHeight < sampleHeight ){
            byte[] bytes = new byte[(int)f.length()];
            try {
                FileInputStream is = new FileInputStream(f);
                is.read(bytes);
                is.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            return new nxImageEntry(bytes);
        }

        String lcpath = imageFilePath.toLowerCase(Locale.US);
        int orientation = ExifInterface.ORIENTATION_UNDEFINED;
        if( lcpath.endsWith(".jpeg") || lcpath.endsWith(".jpg") ) {
            try {
                ExifInterface exif = new ExifInterface(imageFilePath);
                orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_UNDEFINED);
            } catch (IOException e) {
                orientation = ExifInterface.ORIENTATION_UNDEFINED;
            }
        }


        if( opt.outWidth > opt.outHeight ){
            ;
        }else{
            int tmp = maxWidth;
            maxWidth = maxHeight;
            maxHeight = tmp;
        }

        int sampleSize = 1;
        while( 	sampleSize < 8 &&
                (( opt.outWidth / sampleSize > maxWidth && opt.outHeight / sampleSize > maxHeight) ||
                        (( opt.outWidth / sampleSize * opt.outHeight / sampleSize > (maxSize>0?maxSize:1500000)) )))
        {
            sampleSize *= 2;
        }

        opt.inSampleSize = sampleSize;
        Bitmap bm = BitmapFactory.decodeFile(imageFilePath,opt);
        if( bm == null ){
            return null;
        }
        Bitmap original = bm;
        Log.d(TAG,"resample bitmap="+original.getWidth()+"X"+original.getHeight()+", sample="+sampleSize);
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

        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        bm.compress(Bitmap.CompressFormat.JPEG, 100, stream);
        return new nxImageEntry(stream.toByteArray());
    }

    nxImageEntry(byte[] data){
        super(data);
    }

    /**
     * replace input image.
     * @param bmp input image
     * @return false - update fail!
     */
    public boolean setImage(Bitmap bmp){
        if( bmp == null ){
            return false;
        }

        if( bmp.isRecycled() ){
            return false;
        }

        if( bmp.getWidth() <= 0 || bmp.getHeight() <= 0 ){
            return false;
        }
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        bmp.compress(Bitmap.CompressFormat.JPEG, 100, stream);
        mData = stream.toByteArray();
        return true;
    }


    /**
     *
     * @param imageFilePath
     * @param sampleWidth
     * @param sampleHeight
     * @return
     */
    public boolean setImage(String imageFilePath, int sampleWidth, int sampleHeight )  {
        int maxSize = 2073600;


        int maxWidth = sampleWidth <= 0 ? 1920: sampleWidth;
        int maxHeight = sampleHeight <= 0 ? 1080 : sampleHeight;

        maxSize = maxWidth * sampleHeight;

        File f = new File(imageFilePath);
        if( !f.isFile() ){
            return false;
        }

        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inJustDecodeBounds = true;

        BitmapFactory.decodeFile(imageFilePath,opt);
        opt.inJustDecodeBounds = false;

        if( opt.outWidth <= 0 || opt.outHeight <= 0 ){
            return false;
        }

        String lcpath = imageFilePath.toLowerCase(Locale.US);
        int orientation = ExifInterface.ORIENTATION_UNDEFINED;
        if( lcpath.endsWith(".jpeg") || lcpath.endsWith(".jpg") ) {
            try {
                ExifInterface exif = new ExifInterface(imageFilePath);
                orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_UNDEFINED);
            } catch (IOException e) {
                orientation = ExifInterface.ORIENTATION_UNDEFINED;
            }
        }

        int sampleSize = 1;
        while( 	sampleSize < 8 &&
                (( opt.outWidth / sampleSize > maxWidth && opt.outHeight / sampleSize > maxHeight) ||
                        (( opt.outWidth / sampleSize * opt.outHeight / sampleSize > (maxSize>0?maxSize:1500000)) )))
        {
            sampleSize *= 2;
        }

        opt.inSampleSize = sampleSize;
        Bitmap bm = BitmapFactory.decodeFile(imageFilePath,opt);

        if( bm == null ){
            return false;
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

        setImage(bm);
        return true;
    }

    /**
     *
     * @return
     */
    public Bitmap getImage(){
        if( mData == null ){
            return null;
        }
        return BitmapFactory.decodeByteArray(mData,0,mData.length);
    }

    public Bitmap rotateImage(Bitmap src, int orientation){
        Bitmap bm = null;
        Bitmap original = src;
        switch( orientation ) {
            case 180:
            {
                bm = Bitmap.createBitmap(original.getWidth(), original.getHeight(), Bitmap.Config.ARGB_8888);
                Canvas c = new Canvas(bm);
                Matrix m = new Matrix();
                m.setScale(-1, -1);
                m.postTranslate(original.getWidth(), original.getHeight());
                c.drawBitmap(original, m, null);
                break;
            }
            case 90:
            {
                bm = Bitmap.createBitmap(original.getHeight(), original.getWidth(), Bitmap.Config.ARGB_8888);
                Canvas c = new Canvas(bm);
                Matrix m = new Matrix();
                m.setRotate(90);
                m.postTranslate(original.getHeight(), 0);
                c.drawBitmap(original, m, null);
                break;
            }
            case 270:
            {
                bm = Bitmap.createBitmap(original.getHeight(), original.getWidth(), Bitmap.Config.ARGB_8888);
                Canvas c = new Canvas(bm);
                Matrix m = new Matrix();
                m.setRotate(270);
                m.postTranslate(0, original.getWidth());
                c.drawBitmap(original, m, null);
                break;
            }
        }
        if( bm == null )
            return src;
        return bm;
    }

    /**
     *
     * @return
     */
    public byte[] getEncodedData(){
        return mData;
    }

    /**
     *
     * @param path
     * @return
     */
    public boolean saveFile(String path){
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(path);
            out.write(mData,0,mData.length);

        } catch (FileNotFoundException e) {
            //e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if( out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return true;
    }
}
