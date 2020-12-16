package com.nexstreaming.kminternal.kinemaster.editorwrapper;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;
import android.util.LruCache;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

/**
 * Created by jeongwook.yoon on 2015-11-23.
 */
public class LookUpTable {

    private static int max_lut_entry = 100;
    private static LookUpTable sSingleton = null;
    private static final String TAG="LookUpTable";
    private LruCache<String, LUT> mLUTCache;
    private Map<String, CustomLUT> custom_lut_ = new HashMap<String, CustomLUT>();
    private List<LUTEntry> mEntrys;
    private Context mAppContext;
    private transient WeakReference<Bitmap> m_cachedVignette;
    private boolean needUpdate;
    private static int CustomLUT_UID = 100;
    private Object m_cacheLock = new Object();
    public class CustomLUT{

        private int uid_;
        private LUT lut_;

        public CustomLUT(int uid, LUT lut){

            uid_ = uid;
            lut_ = lut;
        };

        public int getUID(){

            return uid_;
        }

        public LUT getLUT(){

            return lut_;
        }
    }

    public class LUTEntry{
        private int idx;
        private String itemId;
        private String name;
        private String km_id;
        private boolean hidden;

        private LUTEntry(){}
        private LUTEntry(int idx, String itemId, long time, boolean hidden ){
            this.idx = idx;
            this.itemId = itemId;
            int start = itemId.lastIndexOf(".");
            if( start < 0 ){
                start = 0;
            }else{
                start++;
            }
            name = itemId.substring(start);
            this.hidden = hidden;
            setKineMasterID();
        }

        private void setKineMasterID(){
            km_id = "LUT_"+name.toUpperCase();
            if( name.compareTo("disney") == 0 ){
                km_id = "LUT_DBRIGHT";
            }
        }

        public String getName(){
            return name;
        }

        public int getIdx(){
            return idx;
        }

        public boolean isHidden(){
            return hidden;
        }

        public String getKineMasterID(){
            return km_id;
        }

        public String getAssetItemId(){
            return itemId;
        }
    }

    public static LookUpTable getLookUpTable( Context context  ) {
        Context appContext = context.getApplicationContext();

        if( sSingleton!=null && !sSingleton.mAppContext.getPackageName().equals(appContext.getPackageName()) ) {
            sSingleton = null;
        }
        if( sSingleton==null ) {
            sSingleton = new LookUpTable(appContext);
            sSingleton.resolveTable();
        }
        return sSingleton;
    }

    public void releaseResource2LookUpTable() {
        Log.d(TAG,"releaseResource2LookUpTable()");
        if(mLUTCache != null) {
            mLUTCache.evictAll();
            mLUTCache = null;
        }
        if(mEntrys != null) {
            mEntrys.clear();
            mEntrys = null;
        }
        sSingleton = null;
    }

    public void cleanCache(){
        synchronized (m_cacheLock) {
            if (mLUTCache != null) {
                if (mLUTCache.size() > 0) {
                    mLUTCache.evictAll();
                }
            }
        }
    }

    public static LookUpTable getLookUpTable( ) {
        return sSingleton;
    }

    private LookUpTable(Context appContext){
        mAppContext = appContext;
    }


    public int resolveTable(){
        if( mEntrys != null ){
            mEntrys.clear();
        }else{
            mEntrys = new ArrayList<LUTEntry>();
        }
        int id = 1;

        List<? extends ItemInfo> luts = AssetPackageManager.getInstance().getInstalledItemsByCategory(ItemCategory.filter);
        for( ItemInfo info : luts){
            if( info.getType() == ItemType.lut ) {
                mEntrys.add(new LUTEntry(id, info.getId(), 0, info.isHidden()));
                id++;
            }
        }

        for(Map.Entry<String, CustomLUT> entry : custom_lut_.entrySet()){

            CustomLUT val = entry.getValue();
            mEntrys.add(new LUTEntry(val.getUID(), entry.getKey(), 0, false));
            id++;
        }
        return id - 1;
    }

    private ArrayList<Integer> custom_uid_storage_ = null;

    private int issueCustomLUTUid(){

        synchronized(this){

            if(null == custom_uid_storage_){

                custom_uid_storage_ = new ArrayList<Integer>();

                for(int i = CustomLUT_UID; i < CustomLUT_UID + 10; ++i){

                    custom_uid_storage_.add(i);
                }
            }

            int uid = custom_uid_storage_.get(0);
            custom_uid_storage_.remove(0);
            return uid;
        }
    }

    private void returnCustomLUTUid(int uid){

        synchronized(this){

            custom_uid_storage_.add(uid);
        }
    }

    public int addCustomLUT(String itemId, LUT lut) throws Exception{

        if(custom_lut_.size() >= 10)
            throw new Exception("Exceed the capacity of custom lut - it is 10");

        initLUTCache();
        CustomLUT customlut = new CustomLUT(issueCustomLUTUid(), lut);
        custom_lut_.put(itemId, customlut);
        // synchronized (m_cacheLock) {
        //     mLUTCache.put(itemId, customlut.getLUT());
        // }
        setNeedUpdate();
        return customlut.getUID();
    }

    public int removeCustomLUT(String itemId){

        //TODO
        CustomLUT customlut = custom_lut_.get(itemId);
        if(customlut != null){
            // synchronized (m_cacheLock) {
            //     if (mLUTCache != null)
            //         mLUTCache.remove(itemId);
            // }
            custom_lut_.remove(itemId);
            EditorGlobal.getEditor().resetLUTTextuer(customlut.getUID());
            returnCustomLUTUid(customlut.getUID());
            setNeedUpdate();
        }        
        return 0;
    }

    public void removeAllCustomLUT(){

        for(CustomLUT m:custom_lut_.values()){
            EditorGlobal.getEditor().resetLUTTextuer(m.getUID());
            returnCustomLUTUid(m.getUID());
        }
        custom_lut_.clear();
        setNeedUpdate();
    }

    public boolean existCustomLUT(String itemId){
        return  (custom_lut_.get(itemId) != null );
    }

    public int getUID(String itemId){

        checkUpdate();

        if( mEntrys == null )
            return 0;
        if( mEntrys.size() == 0 )
            return 0;

        CustomLUT user_lut = custom_lut_.get(itemId);
        if(user_lut != null)
            return user_lut.getUID();

        for( int i = 0 ; i < mEntrys.size() ; i++ ){
            if( mEntrys.get(i).name.compareTo(itemId) == 0 ){
                return mEntrys.get(i).idx;
            }
        }
        return 0;
    }

    private int getID2Index(int idx){
        checkUpdate();

        if( mEntrys == null )
            return -1;
        if( mEntrys.size() == 0 )
            return -1;

        for( int i = 0 ; i < mEntrys.size() ; i++ ){
            if( mEntrys.get(i).idx == idx ){
                return i;
            }
        }
        return -1;
    }

    private int getID2name(String name){
        checkUpdate();

        if( mEntrys == null )
            return -1;
        if( mEntrys.size() == 0 )
            return -1;

        for( int i = 0 ; i < mEntrys.size() ; i++ ){
            if( mEntrys.get(i).name.compareTo(name) == 0 ){
                return i;
            }
        }
        return -1;
    }

    public class LUT{

        private int w_;
        private int h_;
        private IntBuffer pixels_;
        private Bitmap bitmap_;

        public int getWidth(){

            return w_;
        }

        public int getHeight(){

            return h_;
        }

        private Bitmap dimensionconvertedBitmap(int[] src){

            IntBuffer pixels = IntBuffer.allocate(512 * 512);
            int[] dst = pixels.array();
            for(int i = 0; i < 8; ++i){

                for(int j = 0; j < 8; ++j){

                    for(int k = 0; k < 64; ++k){

                        for(int l = 0; l < 64; ++l){

                            dst[i * 64 + j * 512 * 64 + k + l * 512] 
                            = src[64 * 4096 - 64 - i * 512 * 64 - j * 64 * 64 + k - l * 64];
                        }
                    }
                }
            }

            return Bitmap.createBitmap(dst, 512, 512, Bitmap.Config.ARGB_8888);
        }

        public LUT(byte[] src, int offset, int length, int mode){

            if(mode == 2){

                int[] dst = EditorGlobal.getEditor().makeCubeLUT(src, offset, length);

                if(dst != null)
                    bitmap_ = dimensionconvertedBitmap(dst);
            }
            else if(mode == 1){

                int[] dst = EditorGlobal.getEditor().makeLGLUT(src, offset, length);
                if(dst != null)
                    bitmap_ = dimensionconvertedBitmap(dst);
            }
            else if(mode == 0){

                Bitmap bitmap_ = BitmapFactory.decodeByteArray(src, offset, length);
            }

            if(bitmap_ != null){

                w_ = bitmap_.getWidth();
                h_ = bitmap_.getHeight();
            }
        }

        public LUT(Bitmap color_lut){

            IntBuffer pixels = IntBuffer.allocate(64 * 4096);
            pixels_ = IntBuffer.allocate(512 * 512);
            color_lut.getPixels(pixels.array(), 0, color_lut.getWidth(), 0, 0, color_lut.getWidth(), color_lut.getHeight());
            int[] src = pixels.array();
            int[] dst = pixels_.array();
            for(int i = 0; i < 8; ++i){

                for(int j = 0; j < 8; ++j){

                    for(int k = 0; k < 64; ++k){

                        for(int l = 0; l < 64; ++l){

                            dst[i * 64 + j * 512 * 64 + k + l * 512] 
                            = src[64 * 4096 - 64 - i * 512 * 64 - j * 64 * 64 + k - l * 64];
                        }
                    }
                }
            }
            bitmap_ = Bitmap.createBitmap(dst, 512, 512, color_lut.getConfig());
            w_ = bitmap_.getWidth();
            h_ = bitmap_.getHeight();
        }

        public int[] getPixels(){

            return pixels_.array();
        }

        public Bitmap getBitmap(){

            return bitmap_;
        }
    }

    public List<LUTEntry> getLUTEntrys(){
        checkUpdate();
        return mEntrys;
    }

    private void initLUTCache(){
        synchronized (m_cacheLock) {
            if (mLUTCache == null) {
                mLUTCache = new LruCache(max_lut_entry) {
                    @Override
                    protected void entryRemoved(boolean evicted, Object key, Object oldValue, Object newValue) {
                        Log.d(TAG, "The entry is being removed / key:" + (String) key);
                        LUT lut = (LUT) oldValue;
                        lut.getBitmap().recycle();
                        int index = getID2name((String) key);
                        if (index < 0) {
                            Log.d(TAG, "The entry is not found");
                        } else {
                            Log.d(TAG, "The entry index =" + index + ", id=" + mEntrys.get(index).idx);
                            EditorGlobal.getEditor().resetLUTTextuer(mEntrys.get(index).idx);
                        }

                    }
                };
            }
        }
    }

    public LUT getLUT(int id){
        int index = getID2Index(id);
        if( index < 0  )
            return null;

        LUT lut = null;

        initLUTCache();
        synchronized (m_cacheLock) {
            lut = mLUTCache.get(mEntrys.get(index).itemId);
            if (lut == null) {

                CustomLUT clut = custom_lut_.get(mEntrys.get(index).itemId);
                if (clut != null) {
                    lut = clut.getLUT();
                    // mLUTCache.put(mEntrys.get(index).itemId, lut);
                } else {

                    BitmapFactory.Options opts = new BitmapFactory.Options();
                    opts.inScaled = false;

                    Bitmap bmp = null;
                    ItemInfo itemInfo = AssetPackageManager.getInstance().getInstalledItemById(mEntrys.get(index).itemId);
                    if (itemInfo != null) {
                        AssetPackageReader reader = null;
                        try {
                            reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), itemInfo.getPackageURI(), itemInfo.getAssetPackage().getAssetId());
                            if (reader != null) {
                                InputStream in = reader.openFile(itemInfo.getFilePath());
                                bmp = BitmapFactory.decodeStream(in, null, opts);
                                in.close();
                                mLUTCache.put(mEntrys.get(index).itemId, lut = new LUT(bmp));
                            }
                        } catch (IOException e) {
                            e.printStackTrace();
                        } finally {
                            CloseUtil.closeSilently(reader);
                        }
                    }
                }
            }
        }

        return lut;
    }

    public Bitmap applyColorLUTOnBitmap(Bitmap bgBitmap, int lutID){
        LUT lut = getLUT(lutID);
        if( lut == null ){
            return null;
        }
        return applyColorLUTOnBitmap(bgBitmap, lut);
    }

    private Bitmap applyColorLUTOnBitmap(Bitmap bitmap, LUT color_lut) {

        int[] lut_src = color_lut.getPixels();

        int w = bitmap.getWidth();
        int h = bitmap.getHeight();

        IntBuffer pixels = IntBuffer.allocate(w * h);
        bitmap.getPixels(pixels.array(), 0, w, 0, 0, w, h);

        int[] img_src = pixels.array();
        int len = img_src.length;
        for( int i=0; i<len; i++ ) {

            int color = img_src[i];

            float position_of_red = (float)((color >> 16) & 0xFF) / 256.0f * 63.0f;
            float position_of_green = (float)((color >> 8) & 0xFF) / 256.0f * 63.0f;
            float position_of_blue = (float)(color & 0xFF) / 256.0f * 63.0f;
            int low = (int)Math.floor(position_of_blue);
            int high = (int)Math.ceil(position_of_blue);
            int y0 = low % 8;
            int x0 = (low - y0) / 8;
            int y1 = high % 8;
            int x1 = (high - y1) / 8;

            x0 *= 64;
            y0 *= 64;
            x1 *= 64;
            y1 *= 64;

            int floor_of_red_position = (int)Math.floor(position_of_red);
            int floor_of_green_position = (int)Math.floor(position_of_green);
            int ceil_of_red_position = (int)Math.ceil(position_of_red);
            int ceil_of_green_position = (int)Math.ceil(position_of_green);

            int base_color = lut_src[(y0 + floor_of_red_position) * 512 + x0 + floor_of_green_position];
            int base_color_red = lut_src[(y0 + ceil_of_red_position) * 512 + x0 + floor_of_green_position];
            int base_color_green = lut_src[(y0 + floor_of_red_position) * 512 + x0 + ceil_of_green_position];

            int base_red_0 = (base_color >> 16) & 0xFF;
            int base_green_0 = (base_color >> 8) & 0xFF;
            int base_red_1 = (base_color_red >> 16) & 0xFF;
            int base_green_1 = (base_color_green >> 8) & 0xFF;

            float base_mix_red = position_of_red - (float)floor_of_red_position;
            float base_mix_green = position_of_green - (float)floor_of_green_position;
            float base_mix_blue = position_of_blue - (float)Math.floor(position_of_blue);

            int base_red_final = (int)((float)base_red_0 * (1.0f - base_mix_red) + (float)base_red_1 * base_mix_red);
            int base_green_final = (int)((float)base_green_0 * (1.0f - base_mix_green) + (float)base_green_1 * base_mix_green);
            int base_blue_final = base_color & 0xFF;

            int top_color = lut_src[(y1 + floor_of_red_position) * 512 + x1 + floor_of_green_position];
            int top_color_red = lut_src[(y1 + ceil_of_red_position) * 512 + x1 + floor_of_green_position];
            int top_color_green = lut_src[(y1 + floor_of_red_position) * 512 + x1 + ceil_of_green_position];

            int top_red_0 = (top_color >> 16) & 0xFF;
            int top_green_0 = (top_color >> 8) & 0xFF;
            int top_red_1 = (top_color_red >> 16) & 0xFF;
            int top_green_1 = (top_color_green >> 8) & 0xFF;

            int top_red_final = (int)((float)top_red_0 * (1.0f - base_mix_red) + (float)top_red_1 * base_mix_red);
            int top_green_final = (int)((float)top_green_0 * (1.0f - base_mix_green) + (float)top_green_1 * base_mix_green);
            int top_blue_final = top_color & 0xFF;
            
            int red_final = (int)((float)base_red_final * (1.0f - base_mix_blue) + (float)top_red_final * base_mix_blue);
            int green_final = (int)((float)base_green_final * (1.0f - base_mix_blue) + (float)top_green_final * base_mix_blue);
            int blue_final = (int)((float)base_blue_final * (1.0f - base_mix_blue) + (float)top_blue_final * base_mix_blue);

            img_src[i] = (0xFF << 24) | (base_red_final << 16) | (base_green_final << 8) | base_blue_final;
        }

        if( bitmap.getConfig() != null ){
            return Bitmap.createBitmap(img_src, bitmap.getWidth(), bitmap.getHeight(), bitmap.getConfig());
        }
        return Bitmap.createBitmap(img_src, bitmap.getWidth(), bitmap.getHeight(), Bitmap.Config.ARGB_8888);
    }

    public final Bitmap getBitmapForVignette(){
        Bitmap bm = null;
        if( m_cachedVignette != null ) {
            bm = m_cachedVignette.get();
        }
        BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inScaled = false;

        if( bm == null ){
            File file = new File( EditorGlobal.getEditorRoot().getAbsolutePath()+File.separator+"vignette.webp");
            if( file.isFile() ){
                bm = BitmapFactory.decodeFile( file.getAbsolutePath() , opts);
            }else {
                try {
                    bm = BitmapFactory.decodeStream(mAppContext.getResources().getAssets().open("vignette.webp"), null, opts);
                } catch (IOException e) {
                    bm = null;
                    e.printStackTrace();
                }
            }
        }

        if( bm == null ){
            return null;
        }
        m_cachedVignette = new WeakReference<Bitmap>(bm);
        return bm;
    }

    public LUTEntry getLutByItemId(String ItemId){
        checkUpdate();
        for( LUTEntry ent : mEntrys ){
            if( ent.itemId.compareTo(ItemId) == 0 ){
                return ent;
            }
        }
        return null;
    }

    public String[] getLutItemIds(boolean includeHidden){
        checkUpdate();
        List<String> list = new ArrayList<>();
        for( LUTEntry ent : mEntrys ){
            if( includeHidden ){
                list.add(ent.itemId);
            }else {
                if (!ent.isHidden()) {
                    list.add(ent.itemId);
                }
            }
        }

        String[] ret = new String[list.size()];
        for(int i = 0 ; i < ret.length ; i++ ){
            ret[i] = list.get(i);
        }
        return ret;
    }

    public void setNeedUpdate(){
        needUpdate = true;
    }

    private void checkUpdate(){
        if( needUpdate ){
            needUpdate = false;
            resolveTable();
        }
    }
}
