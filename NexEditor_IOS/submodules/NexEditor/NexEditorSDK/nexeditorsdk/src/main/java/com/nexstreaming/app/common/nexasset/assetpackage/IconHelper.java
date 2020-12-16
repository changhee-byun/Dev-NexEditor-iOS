package com.nexstreaming.app.common.nexasset.assetpackage;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Picture;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.RectF;
import android.util.Log;

import com.larvalabs.svgandroid.SVG;
import com.larvalabs.svgandroid.SVGParser;
import com.nexstreaming.app.common.util.PathUtil;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Locale;

public  class IconHelper {
    private static final String LOG_TAG = "IconHelper";

    public static Bitmap getIconBitmap(Context context, ItemInfo itemInfo, int iconWidth, int iconHeight) throws IOException {
        return loadBitmap(context,itemInfo,itemInfo.getIconPath(), iconWidth, iconHeight);
    }

    public static Bitmap getThumbnailBitmap(Context context, ItemInfo itemInfo, int iconWidth, int iconHeight) throws IOException {
        return loadBitmap(context,itemInfo,itemInfo.getThumbPath(), iconWidth, iconHeight);
    }

    public static Bitmap getAssetItemBitmap(Context context, ItemInfo itemInfo) throws IOException {
        return loadBitmap(context,itemInfo,itemInfo.getFilePath(), 0, 0);
    }

    public static Bitmap getAssetItemBitmap(Context context, ItemInfo itemInfo, String path) throws IOException {
        return loadBitmap(context,itemInfo, PathUtil.relativePath(itemInfo.getFilePath(),path), 0, 0);
    }

    private static Bitmap loadBitmap(Context context, ItemInfo itemInfo, String path, int iconWidth, int iconHeight) throws IOException {
        AssetInfo assetInfo = itemInfo.getAssetPackage();
        AssetPackageReader reader = AssetPackageReader.readerForPackageURI(context, itemInfo.getPackageURI(), assetInfo.getAssetId());
        InputStream iconInStream;
        try {
            iconInStream = reader.openFile(path);
        }catch (FileNotFoundException e){
            return null;
        }
        Bitmap iconBitmap = null;
        Canvas canvas = null;
        try {
            if(path.toLowerCase(Locale.ENGLISH).endsWith(".svg")) {
                SVG svg = SVGParser.getSVGFromInputStream(iconInStream);
                Picture picture = svg.getPicture();
                if(iconWidth > 0 && iconHeight > 0) {
                    float scaleFactor = (float)iconWidth / (float)picture.getWidth();
                    int adjustedHeight = (int)(scaleFactor * picture.getHeight());
                    iconBitmap = Bitmap.createBitmap(iconWidth, adjustedHeight, Bitmap.Config.ARGB_8888);
                    canvas = new Canvas(iconBitmap);
                    canvas.drawPicture(picture, new RectF(0, 0, iconWidth, adjustedHeight));
                }else {
                    iconBitmap = Bitmap.createBitmap(picture.getWidth(), picture.getHeight(), Bitmap.Config.ARGB_8888);
                    canvas = new Canvas(iconBitmap);
                    picture.draw(canvas);
                }
            } else {
                Bitmap bitmap = BitmapFactory.decodeStream(iconInStream);
                iconBitmap = bitmap.copy(Bitmap.Config.ARGB_8888, true);
            }
        // 24 Nov 2016, mark.lee, KM-4963
        } catch (Exception e) {
            Log.e(LOG_TAG, e.getMessage(), e);
        }finally {
            iconInStream.close();
            if(itemInfo.getCategory() == ItemCategory.transition) {
                Paint paint = new Paint();
                if(canvas == null) {
                    canvas = new Canvas(iconBitmap);
                }
                paint.setColor(0xffffffff);
                paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_IN));
                canvas.drawRect(0, 0, iconBitmap.getWidth(), iconBitmap.getHeight(), paint);
            }
        }
        return iconBitmap;
    }

    private static Bitmap loadBitmap(String path, InputStream iconInStream) throws IOException {
        Bitmap iconBitmap = null;
        try {
            if(path.toLowerCase(Locale.ENGLISH).endsWith(".svg")) {
                SVG svg = SVGParser.getSVGFromInputStream(iconInStream);
                Picture picture = svg.getPicture();
                iconBitmap = Bitmap.createBitmap(picture.getWidth(),picture.getHeight(), Bitmap.Config.ARGB_8888);
                Canvas canvas = new Canvas(iconBitmap);
                picture.draw(canvas);
            } else {
                iconBitmap = BitmapFactory.decodeStream(iconInStream);
            }
        }finally {
            iconInStream.close();
        }
        return iconBitmap;
    }

}
