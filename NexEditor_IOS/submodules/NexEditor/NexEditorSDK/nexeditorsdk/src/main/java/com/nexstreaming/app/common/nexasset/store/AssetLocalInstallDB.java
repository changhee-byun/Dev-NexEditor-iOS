package com.nexstreaming.app.common.nexasset.store;

import android.accounts.NetworkErrorException;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.*;
import android.os.Process;
import android.util.Log;

import com.google.gson_nex.Gson;
import com.google.gson_nex.JsonIOException;
import com.google.gson_nex.JsonSyntaxException;
import com.google.gson_nex.internal.ObjectConstructor;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.InstallSourceType;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.store.json.AssetStoreAPIData;

import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.app.common.util.CopyUtil;
import com.nexstreaming.app.common.util.ZipUtil;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.LL;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.URI;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.SecretKeySpec;

/**
 * Created by jeongwook.yoon on 2016-10-13.
 */
public class AssetLocalInstallDB {
    private static final String TAG = "AssetLocalInstallDB";
    private static String localRootPath;
    private static String assetStoreRootPath;
    private static String localFeaturedPath;
    private Context mContext = null;
    private List<String> readyToInstallPackages;
    private List<String> readyToDeletePackages;

    private List<String> readyToLoadPackages;
    private static final Executor sInstallThreadPool = Executors.newSingleThreadExecutor();

    private static int supportedMimeType;

    public static final int ASSET_UNINSTALL_NOT_YET = 0;
    public static final int ASSET_UNINSTALL_FINISHED = 1;
    private static AssetLocalInstallDB instance;

    private static final boolean commAS = false;

    AssetLocalInstallDB(Context context){
        if( assetStoreRootPath == null ){
            assetStoreRootPath = EditorGlobal.getStorageRoot().getAbsolutePath() + File.separator + ".nexassets"+ File.separator + context.getPackageName();
        }

        if( localRootPath == null ){
            localRootPath = context.getFilesDir().getAbsolutePath()+File.separator + "assets";
            File file = new File(localRootPath);
            if(!file.exists()){
                file.mkdirs();
            }
        }

        if( localFeaturedPath == null) {
            localFeaturedPath = context.getFilesDir().getAbsolutePath() + File.separator + "featured";

            File info = new File(localFeaturedPath);
            if (!info.exists()) {
                info.mkdirs();
            }
        }

        readyToInstallPackages = new ArrayList<String>();
        readyToDeletePackages =  new ArrayList<String>();
        readyToLoadPackages = new ArrayList<String>();
        mContext = context;
    }

    public static AssetLocalInstallDB getInstance(Context context) {
        if(instance==null) {
            instance = new AssetLocalInstallDB(context.getApplicationContext());
        }
        return instance;
    }

    private boolean isSamePath(){
        return (localRootPath.compareTo(assetStoreRootPath) == 0);
    }

    public static void setAssetStoreRootPath(String path){
        assetStoreRootPath = path;
    }

    public static String getAssetStoreRootPath(){
        return assetStoreRootPath;
    }

    public static void setMimeType(int mimeType){
        supportedMimeType = mimeType;
    }

    private String getThumbnailOutputPath(String baseId){

        String folderPath = localRootPath;
        if( localRootPath.startsWith(mContext.getFilesDir().getAbsolutePath()) ){
            folderPath = mContext.getFilesDir().getAbsolutePath() + File.separator + "thumb";
        }
        File file = new File(folderPath);
        if(!file.exists()){
            file.mkdirs();
        }
        return folderPath + File.separator + baseId+".jpg";
    }

    public static void setInstalledAssetPath(String path){
        localRootPath = path;
        File file = new File(localRootPath);
        if(!file.exists()){
            file.mkdirs();
        }
    }

    public static String getInstalledAssetPath(){
        return localRootPath;
    }

    private void createDummyIcon(String baseId){
        File out = new File(getThumbnailOutputPath(baseId));

        int w = 32;
        int h = 18;
        int[] pixels = new int[w*h];
        for( int i=0; i<pixels.length; i++ )
            pixels[i] = 0xff000000;
        FileOutputStream fos = null;
        Bitmap bmp =Bitmap.createBitmap(pixels, w, h, Bitmap.Config.ARGB_8888);
        try {
            out.createNewFile();
            fos = new FileOutputStream(out);
            bmp.compress(Bitmap.CompressFormat.PNG, 100, fos);
        }catch (final FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    /**
                     * can ignore exception.
                     */
                }
            }
        }
    }

    public File makeThumbnail(final Bitmap bmp, int idx){
        File out = new File(getThumbnailOutputPath(""+idx));
        FileOutputStream fos = null;
        try {
            out.createNewFile();
            fos = new FileOutputStream(out);
            bmp.compress(Bitmap.CompressFormat.PNG, 100, fos);
        }catch (final FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    /**
                     * can ignore exception.
                     */
                }
            }
        }
        return out;
    }

    private File copyThumbnail(String baseId){
        if( isSamePath() ){
            File thumb = new File(assetStoreRootPath+File.separator+baseId+".jpg");
            if( !thumb.isFile() ){
                Log.d(TAG,"copyThumbnail() file not found="+thumb.getAbsolutePath());
            }
            return  thumb;
        }
        File thumbFile = new File(assetStoreRootPath+File.separator+baseId+".jpg");
        File out = new File(getThumbnailOutputPath(baseId));
        if( thumbFile.isFile() ){
            BitmapFactory.Options opts = new BitmapFactory.Options();
            opts.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(thumbFile.getAbsolutePath(), opts);


            opts.inJustDecodeBounds = false;
            int sampleSize = 1;
            while( 	sampleSize < 8 && ( opts.outWidth / sampleSize > 320 && opts.outHeight / sampleSize > 180) )
            {
                sampleSize *= 2;
            }
            opts.inSampleSize = sampleSize;

            Bitmap bmp = BitmapFactory.decodeFile(thumbFile.getAbsolutePath(),opts);

            FileOutputStream fos = null;
            try {
                out.createNewFile();
                fos = new FileOutputStream(out);
                bmp.compress(Bitmap.CompressFormat.PNG, 100, fos);
            }catch (final FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }finally {
                if (fos != null) {
                    try {
                        fos.close();
                    } catch (IOException e) {
                        /**
                         * can ignore exception.
                         */
                    }
                }
            }
            thumbFile.delete();
            return out;
        }
        createDummyIcon(baseId);
        return out;
    }

    private void createDummy(String assetStoreFilename) throws IOException {
        if( isSamePath() ){
            return;
        }
        OutputStream outputStream = new FileOutputStream(new File(assetStoreRootPath,assetStoreFilename));
        byte[] buf = {'P','K'};
        outputStream.write(buf);
        outputStream.close();
    }

    private boolean checkInstallFile(String StorePath, String baseid){

        File json = new File(StorePath,baseid+".json");

        boolean isOk = true;

        int idx = getAssetIdxInJson(baseid);
        int baseidx = Integer.parseInt(baseid);

        if( idx != baseidx ){
            Log.d(TAG,"invalid "+baseid+".json. idx="+idx);
            isOk = false;
        }

        File key = new File(StorePath, "." + baseid);
        if(commAS) {
            if (!key.isFile()) {
                isOk = false;
            }
        }
        
        File thumb = new File(StorePath,baseid+".jpg");
        if( !thumb.isFile() ){
            Log.d(TAG,"thumbnail not found ("+baseid+")");
            isOk = false;
        }

        if( !isOk ){
            if( json.isFile() )
                json.delete();

            if( thumb.isFile() )
                thumb.delete();
            if(commAS) {
                if (key.isFile())
                    key.delete();
            }
        }
        return isOk;
    }

    public int checkStoreInstall(){
        readyToInstallPackages.clear();
        int mimeType =supportedMimeType;
        String StorePath = assetStoreRootPath;
        File storeDir = new File(StorePath);
        if( storeDir.isDirectory() ){
            String [] fileList = storeDir.list();
            for( String filename : fileList ){
                File file = new File(StorePath,filename);
                if( file.isFile() ){
                    if( filename.endsWith(".zip") ) {
                        if (file.length() > 2) {
                            //install
                            String baseId = filename;
                            baseId = baseId.substring(0,baseId.length()-4);
                            if( checkInstallFile(StorePath,baseId) ) {
                                readyToInstallPackages.add(filename);
                            }else{
                                file.delete();
                            }
                        }
                    }
                }
            }
        }
        return readyToInstallPackages.size();
    }

    public List<String> getReadyToInstallPackages(){
        checkStoreInstall();
        return readyToInstallPackages;
    }

    public void checkStore(){
        readyToDeletePackages.clear();
        readyToLoadPackages.clear();
        File storeDir = new File(localRootPath);
        if( storeDir.isDirectory() ){
            String [] fileList = storeDir.list();
            for( String filename : fileList ){
                File file = new File(localRootPath,filename);
                if( file.isDirectory() ){
                    File assetStoreFile = new File(assetStoreRootPath+File.separator+filename+".zip");
                    if( !assetStoreFile.isFile() ){
                        readyToDeletePackages.add(filename);
                        try {
                            uninstallPackage(Integer.parseInt(filename));
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                    if( file.length() <= 2 ){
                        readyToLoadPackages.add(filename);
                    }
                }
            }
        }
    }

    public boolean isInstallingPackages(){
        return installing;
    }

    private boolean installing = false;

    public Task installPackageAsync( int AssetIdx ){
        final Task task = new Task();

        installing = true;

        new AsyncTask<Integer, Void, Exception>() {
            @Override
            protected void onPreExecute() {
                super.onPreExecute();

            }

            @Override
            protected Exception doInBackground(Integer... params) {
                Process.setThreadPriority(Process.THREAD_PRIORITY_DEFAULT);
                int AssetIdx = params[0];

                String[] lists = null;
                String[] datas = null;
                int i = 0;

                if( AssetIdx == 0 ){
                    //re-check install package
                    ArrayList<String> installPackages = new ArrayList<String>();
                    String StorePath = assetStoreRootPath;
                    File storeDir = new File(StorePath);
                    if( storeDir.isDirectory() ){
                        String [] fileList = storeDir.list();
                        for( String filename : fileList ){
                            File file = new File(StorePath,filename);
                            if( file.isFile() ){
                                if( filename.endsWith(".zip") ) {
                                    if (file.length() > 2) {
                                        //install
                                        String baseId = filename;
                                        baseId = baseId.substring(0,baseId.length()-4);
                                        if( checkInstallFile(StorePath,baseId) ) {
                                            installPackages.add(filename);
                                        }else{
                                            file.delete();
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if( installPackages.size() == 0 ){
                        Log.d(TAG,"download asset package not found.");
                        return new Resources.NotFoundException("download asset package not found");
                    }

                    lists = new String[installPackages.size()];
                    datas = new String[installPackages.size()];

                    for (i = 0; i < installPackages.size(); i++) {
                        lists[i] = installPackages.get(i);
                    }
                }else{
                    if( checkInstallFile(assetStoreRootPath,""+AssetIdx) ){
                        lists = new String[1];
                        datas = new String[1];
                        lists[0] = ""+AssetIdx+".zip";
                    }else{
                        Log.d(TAG,"download asset package not found. AssetIdx="+AssetIdx);
                        return new Resources.NotFoundException("download asset package not found");
                    }
                }
                //task.setProgress(0, lists.length);
                if (commAS) {
                    AssetStoreClient.makeConfig();
                    if (AssetStoreClient.getDataSync2(lists, datas, mContext.getPackageName()) < 0) {
                        if (LL.D)
                            Log.d(TAG, "install asset failed : AssetStore communicate fail!");
                        return new NetworkErrorException("AssetStore communicate fail");
                    }
                }

                for (i = 0; i < lists.length; i++) {

                    String baseId = lists[i];
                    baseId = baseId.substring(0, baseId.length() - 4); // Remove .zip extension
                    try{
                        int idx = Integer.parseInt(baseId);
                        AssetInfo ai = AssetPackageManager.getInstance().getInstalledPackageInfoByAssetIdx(idx);
                        if( ai != null ) {
                            Log.d(TAG,"already installed Asset="+idx);

                            if( ai.getInstallSourceType() == InstallSourceType.STORE ) {
                                try {
                                    uninstallPackage(idx,false);
                                } catch (Exception e) {
                                    //e.printStackTrace();
                                }
                            }else{
                                Log.d(TAG,"installed Asset is not store type. idx="+idx);
                            }
                        }
                    }catch ( NumberFormatException e){
                        //
                        Log.d(TAG,"baseId is not Integer baseId="+baseId);
                    }


                    File thumbFile = copyThumbnail(baseId);

                    task.setProgress(i + 1, lists.length);

                    try {
                        if( commAS ) {
                            installPackage(lists[i], baseId, thumbFile, task, true, datas[i]);
                        }else{
                            installPackage(lists[i], baseId, thumbFile, task, false, datas[i]);
                        }
                        Log.i(TAG, "install asset completed : " + "asset = [" + lists[i] + "]");
                    } catch (final FileNotFoundException e) {
                        if (LL.D)
                            Log.d(TAG, "install asset failed : " + "asset = [" + lists[i] + "]");
                        return e;
                    } catch (IOException e) {
                        if (LL.D)
                            Log.d(TAG, "install asset failed : " + "asset = [" + lists[i] + "]");
                        new File(assetStoreRootPath, lists[i]).delete();
                        //new File(getDownloadAssetThumbnailPath(item.getAsset_id())).delete();
                        return e;
                    } finally {
                    }

                }
                return null;
            }

            @Override
            protected void onPostExecute(Exception e) {
                super.onPostExecute(e);
                installing = false;
                if (e != null) {
                    task.sendFailure(new InstallTaskError("asset_install_failed", e));
                } else {
                    task.signalEvent(Task.Event.COMPLETE);
                }
            }
        }.executeOnExecutor(sInstallThreadPool,AssetIdx);


        return task;
    }

    public static class internalStoreAssetInfo implements StoreAssetInfo{

        private AssetStoreAPIData.AssetInfo info;

        public internalStoreAssetInfo( AssetStoreAPIData.AssetInfo info ){
            this.info = info;
        }

        @Override
        public int getAssetIndex() {
            return info.idx;
        }

        @Override
        public String getAssetId() {
            return info.asset_id;
        }

        @Override
        public String getAssetTitle() {
            return info.title;
        }

        @Override
        public String getAssetDescription() {
            return info.description;
        }

        @Override
        public int getCategoryIndex() {
            return info.category_idx;
        }

        @Override
        public String getCategoryAliasName() {
            return info.category_aliasName;
        }

        @Override
        public String getCategoryIconURL() {
            return info.categoryimagePath;
        }

        @Override
        public int getSubCategoryIndex() {
            return info.subcategory_idx;
        }

        @Override
        public Map<String, String> getSubCategoryNameMap() {
            Map<String, String> nameMap = new HashMap<>();
            if(info.subcategoryName != null){
                for(AssetStoreAPIData.LangString subCategoryName : info.subcategoryName){
                    nameMap.put(subCategoryName.language_code.toLowerCase(Locale.ENGLISH), subCategoryName.string_title);
                }
            }
            return nameMap;
        }

        @Override
        public Map<String, String> getAssetNameMap() {
            Map<String, String> nameMap = new HashMap<>();
            if(info.assetName != null){
                for(AssetStoreAPIData.LangString assetName : info.assetName){
                    nameMap.put(assetName.language_code.toLowerCase(Locale.ENGLISH), assetName.string_title);
                }
            }
            return nameMap;
        }

        @Override
        public Map<String, String> getAssetDescriptionMap() {
            return null;
        }

        @Override
        public String getAssetPackageDownloadURL() {
            return info.asset_filepath;
        }

        @Override
        public String getPriceType() {
            return info.priceType;
        }

        @Override
        public String getAssetThumbnailURL() {
            return info.thumbnail_path;
        }

        @Override
        public String getAssetThumbnailURL_L() {
            return info.thumbnail_path_l;
        }

        @Override
        public String getAssetThumbnailURL_S() {
            return info.thumbnail_path_s;
        }

        @Override
        public List<String> getThumbnailPaths() {
            List<String> paths = new ArrayList<>();
            if(info.thumb != null){
                for(AssetStoreAPIData.ThumbInfo thumbInfo : info.thumb){
                    paths.add(thumbInfo.file_path);
                }
            }
            return paths;
        }

        @Override
        public int getAssetVersion() {
            return info.asset_version;
        }

        @Override
        public int getAssetScopeVersion() {
            return info.asset_sversion;
        }

        @Override
        public int getAssetFilesize() {
            return info.asset_filesize;
        }

        @Override
        public String getAssetVideoURL() {
            return info.videoclip_path;
        }

        @Override
        public int getUpdateTime() {
            return info.update_time;
        }

        @Override
        public String getSubCategoryAliasName() {
            return info.category_aliasName;
        }

        @Override
        public long getExpireTime() {
            return info.expire_time;
        }
    }

    private int getAssetIdxInJson(String baseId){
        File jsonFile = new File(assetStoreRootPath, baseId+".json");
        AssetStoreAPIData.AssetInfo sai = null;
        if( jsonFile.isFile() ){
            InputStream is = null;
            try {
                is = new FileInputStream(jsonFile);
                try {
                    Gson gson = new Gson();
                    sai = gson.fromJson(new InputStreamReader(is), AssetStoreAPIData.AssetInfo.class );

                } finally {
                    is.close();
                }
            } catch (FileNotFoundException e) {
                //   e.printStackTrace();
            } catch (IOException e) {
                // e.printStackTrace();
            }
            if( sai == null ){
                return -1;
            }
            return sai.idx;
        }
        return -1;
    }

    private StoreAssetInfo parseStoreAssetInfo(File unzipFolder, String baseId ){
        File jsonFile = new File(assetStoreRootPath, baseId+".json");
        AssetStoreAPIData.AssetInfo sai = null;
        if( jsonFile.isFile() ){
            InputStream is = null;
            try {
                is = new FileInputStream(jsonFile);
                try {
                    Gson gson = new Gson();
                    sai = gson.fromJson(new InputStreamReader(is), AssetStoreAPIData.AssetInfo.class );

                } finally {
                    is.close();
                }
            } catch (FileNotFoundException e) {
             //   e.printStackTrace();
            } catch (IOException e) {
               // e.printStackTrace();
            }

            jsonFile.delete();
            return new internalStoreAssetInfo(sai);
        }else{
            Log.d(TAG,"jsonFile file not found!");

            sai = new AssetStoreAPIData.AssetInfo();

            try {
                AssetPackageReader reader = AssetPackageReader.readerForFolder(unzipFolder, baseId);
                if( reader != null ) {
                    if (reader.getAssetName() != null) {
                        sai.title =reader.getAssetName().get("en");
                    }else{
                        sai.title = baseId;
                    }
                    sai.idx = Integer.parseInt(baseId);
                    sai.asset_id = baseId;
                    reader.close();
                }else{
                    sai.idx = Integer.parseInt(baseId);
                    sai.asset_id = baseId;
                    sai.title = baseId;
                }
            } catch (IOException e) {
                sai.idx = Integer.parseInt(baseId);
                sai.asset_id = baseId;
                sai.title = baseId;
            }
        }

        return new internalStoreAssetInfo(sai);
    }

    public boolean installPackageSync(int idx ){

        String StorePath = assetStoreRootPath;
        String baseId = ""+idx;

        if( installing ){
            Log.d(TAG,"installPackageSync("+idx+"): installing retry");
            return false;
        }
        installing = true;

        File storeDir = new File(StorePath);
        File json = new File(StorePath,baseId+".json");
        File thumb = new File(StorePath,baseId+".jpg");
        File pack = new File(StorePath,baseId+".zip");
        AssetStoreAPIData.AssetInfo sai = null;

        if( storeDir.isDirectory() ){
            boolean checkPackage = true;

            if( !json.isFile() ){
                checkPackage = false;
                Log.d(TAG,"installPackageSync("+idx+"): info json not found!");
            }else{
                InputStream is = null;
                try {
                    is = new FileInputStream(json);
                    try {
                        Gson gson = new Gson();
                        sai = gson.fromJson(new InputStreamReader(is), AssetStoreAPIData.AssetInfo.class );

                    } finally {
                        is.close();
                    }
                } catch (FileNotFoundException e) {
                    //   e.printStackTrace();
                    checkPackage = false;
                } catch (IOException e) {
                    // e.printStackTrace();
                    Log.d(TAG,"installPackageSync("+idx+"): info json parse fail!");
                    checkPackage = false;
                }
            }

            if( sai == null ) {
                checkPackage = false;
                Log.d(TAG,"installPackageSync("+idx+"): json parsing fail!");
            }else {
                if (sai.idx != idx) {
                    Log.d(TAG, "installPackageSync(" + idx + "): invalidation idx=" + sai.idx);
                    checkPackage = false;
                }
            }

            if( !thumb.isFile() ){
                checkPackage = false;
                Log.d(TAG,"installPackageSync("+idx+"): thumbnail not found!");
            }

            if( !pack.isFile() ){
                checkPackage = false;
                Log.d(TAG,"installPackageSync("+idx+"): package not found!");
            }else{
                if( !ZipUtil.isZipFile(pack)) {
                    Log.d(TAG,"installPackageSync("+idx+"): package is not zip!");
                    checkPackage = false;
                }
            }

            if( !checkPackage ){
                if( json.isFile() )
                    json.delete();
                if( pack.isFile() )
                    pack.delete();
                if( thumb.isFile() )
                    thumb.delete();
                installing = false;
                return false;
            }
        }else{
            Log.d(TAG,"installPackageSync("+idx+"): store path not found="+StorePath);
            installing = false;
            return false;
        }

        File unzipFolder = new File(localRootPath, baseId);
        try {
            //copyThumbnail(baseId)
            File thumbnail = new File(getThumbnailOutputPath(baseId));
            copyFile(thumb,thumbnail.getAbsolutePath());
            ZipUtil.unzip(pack,unzipFolder);
            AssetPackageManager.getInstance(mContext).installStorePackage(unzipFolder, thumbnail, new internalStoreAssetInfo(sai));
        } catch (IOException e) {
            e.printStackTrace();
            Log.d(TAG,"installPackageSync("+idx+"): unzip fail="+unzipFolder.getAbsolutePath());
            installing = false;
            return false;
        }

        thumb.delete();
        json.delete();
        pack.delete();
        installing = false;
        return true;
    }

    private void installPackage(String filename,String baseId, File thumbFile, Task task ,boolean enc, String byte16) throws IOException {
        if(LL.D) Log.d(TAG, "installPackage() called with: " + "item = [" + filename + "], thumbFile = [" + thumbFile + "]");
/*
        if(thumbFile == null && !thumbFile.exists()){
            throw new FileNotFoundException("Not found asset thumbnail file");
        }
*/
        boolean benc = enc;
        File assetFile = new File(assetStoreRootPath,filename);
        if(assetFile.exists()){
            /**
             * needsUnzip Audio, Font
             */
            if( ZipUtil.isZipFile(assetFile)) {
                benc = false;
            }

             File unzipFolder = new File(localRootPath, baseId);
            try{
                    if( benc ) {
                        if (byte16.compareTo(AssetStoreClient.none) == 0) {
                            if (LL.D) Log.d(TAG, "installPackage() fail. key not found.");
                            assetFile.delete();
                            File hfile = new File(assetStoreRootPath, "." + baseId);
                            if (hfile.isFile()) {
                                hfile.delete();
                            }
                            return;
                        }
                    }
                    try {

                        unzip(assetFile, unzipFolder, task , benc, byte16);
                        File hfile = new File(assetStoreRootPath, "."+baseId);
                        if( hfile.isFile() ){
                            hfile.delete();
                        }
                    } catch (NoSuchAlgorithmException e) {
                        e.printStackTrace();
                        if(unzipFolder.exists()){
                            unzipFolder.delete();
                        }
                    } catch (NoSuchPaddingException e) {
                        e.printStackTrace();
                        if(unzipFolder.exists()){
                            unzipFolder.delete();
                        }
                    } catch (InvalidKeyException e) {
                        e.printStackTrace();
                        if(unzipFolder.exists()){
                            unzipFolder.delete();
                        }
                    }
            }catch (IOException e){
                if(LL.W) Log.w(TAG, "installPackage unzip error", e);
                if(unzipFolder.exists()){
                    unzipFolder.delete();
                }
                throw new IOException(e);
            }

            synchronized (m_assetdbLock) {
                StoreAssetInfo sai = parseStoreAssetInfo(unzipFolder, baseId);

                if (sai != null) {
                    Log.d(TAG, "install StoreAssetItem, idx=" + sai.getAssetIndex() + ", id=" + sai.getAssetId() + ", SDKLevel=" + sai.getAssetScopeVersion() + ", version=" + sai.getAssetVersion());
                }

                AssetPackageManager.getInstance(mContext).installStorePackage(unzipFolder, thumbFile, sai);

                assetFile.delete();
                createDummy(filename);
            }
        }else{
            throw new FileNotFoundException("Not found asset file");
        }
    }

    public ItemInfo getAssetInstalledItemInfoByAssetIdx(int assetIdx){
        ItemInfo itemInfo = null;
        for(ItemInfo item : AssetPackageManager.getInstance(mContext).getInstalledItemsByAssetPackageIdx(assetIdx)){
            itemInfo = item;
            break;
        }
        return itemInfo;
    }

    public List<? extends AssetInfo> getAssetInstalledDownloadItemItems(){
        List<AssetInfo> list = new ArrayList<>();
        for(AssetInfo assetInfo : AssetPackageManager.getInstance(mContext).getInstalledAssetPackages()){
            if(assetInfo.getInstallSourceType() == InstallSourceType.STORE){
                list.add(assetInfo);
            }
        }
        return  list;
    }

    public void checkInstallDB(){
        int countStoreAsset = 0;
        List<? extends AssetInfo> ainfos = AssetPackageManager.getInstance(mContext).getInstalledAssetPackages();
        for( AssetInfo ai : ainfos ){
            InstallSourceType type = ai.getInstallSourceType();
            if( type == InstallSourceType.STORE ){
                countStoreAsset++;
            }
        }
        if(  countStoreAsset == 0 ){
            Log.d(TAG,"StoreAsset NotFound!");

            String StorePath = assetStoreRootPath;
            File storeDir = new File(StorePath);
            if( storeDir.isDirectory() ){
                String [] fileList = storeDir.list();
                for( String filename : fileList ){
                    File file = new File(StorePath,filename);
                    if( file.isFile() ){
                        if( filename.endsWith(".zip") ) {
                            if (file.length() > 2) {
                                //install
                                continue;
                            }
                        }
                        file.delete();
                    }
                }
            }
        }
    }

    public void uninstallFromAssetStoreApp(){
        String StorePath = assetStoreRootPath;
        File storeDir = new File(StorePath);
        if( storeDir.isDirectory() ){
            String [] fileList = storeDir.list();
            for( String filename : fileList ){
                if( filename.endsWith(".del") ){

                    String idx = filename.substring(0,filename.length() - 4);

                    Log.d(TAG,"uninstallFromAssetStoreApp idx="+idx);

                    try {
                        uninstallPackage(Integer.parseInt(idx));
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    File file = new File(StorePath,filename);
                    file.delete();
                }
            }
        }
    }

    private void deleteDir(File file) {
        File[] contents = file.listFiles();
        if (contents != null) {
            for (File f : contents) {
                deleteDir(f);
            }
        }
        file.delete();
    }

    private Object m_assetdbLock = new Object();

    private int uninstallPackage(int baseId, boolean deleteAssetStore ) throws Exception{
        Log.d(TAG, "uninstallPackage() called with: " + "assetIdx = [" + baseId + "]");
        int returnValue = ASSET_UNINSTALL_FINISHED;
        if( baseId == 0 || baseId == 1 ){
            return  returnValue;
        }

        synchronized (m_assetdbLock) {

            ItemInfo itemInfo = getAssetInstalledItemInfoByAssetIdx(baseId);
            if (itemInfo != null) {
                /**
                 * itemInfo's uri is file or zipfile.
                 */
                URI uri = URI.create(itemInfo.getPackageURI());
                File assetFile = new File(uri.getPath());
                File thumbnail = new File(getThumbnailOutputPath("" + baseId));
                if (assetFile.isDirectory()) {
                    deleteDir(assetFile);
                } else {
                    assetFile.delete();
                }

                if (thumbnail.isFile()) {

                    if (deleteAssetStore) {
                        thumbnail.delete();
                    } else {
                        if (!isSamePath()) {
                            thumbnail.delete();
                        }
                    }
                }

                if (deleteAssetStore) {
                    File file = new File(assetStoreRootPath, baseId + ".zip");
                    if (file.isFile()) {
                        file.delete();
                    }
                }

                if (assetFile.exists()) {
                    returnValue = assetFile.delete() ? ASSET_UNINSTALL_FINISHED : ASSET_UNINSTALL_NOT_YET;
                } else {
                    returnValue = ASSET_UNINSTALL_FINISHED;
                }
                AssetPackageManager.getInstance(mContext).unregisterPackageByAssetIdx(baseId);
            }
            Log.d(TAG, "uninstallPackage() returned: " + returnValue);
            return returnValue;
        }
    }

    public int uninstallPackage(int baseId) throws Exception{
        return uninstallPackage(baseId, true);
    }

    public static final class InstallTaskError implements Task.TaskError{

        final private String message;
        final Exception exception;

        public InstallTaskError(String message, Exception e){
            this.message = message;
            this.exception = e;
        }

        public InstallTaskError(String message){
            this.message = message;
            this.exception = null;
        }

        @Override
        public Exception getException() {
            return exception;
        }

        @Override
        public String getMessage() {
            return message;
        }

        @Override
        public String getLocalizedMessage(Context context) {
            return message;
        }
    }

    private static void unzip(File zipPath, File outputPath, Task task ,boolean enc, String byte16) throws IOException, NoSuchAlgorithmException, NoSuchPaddingException, InvalidKeyException {

        if( LL.D ) Log.d(TAG,"Unzipping '" + zipPath + "' to '" + outputPath + "'");
        if( !outputPath.mkdirs() && !outputPath.exists() ) {
            throw new IOException("Failed to create directory: " + outputPath);
        }
        Cipher cipher = null;
        ZipInputStream in = null;
        long totalSize = zipPath.length();
        long progressSize = 0;
        task.setProgress(0, 100);

        if( totalSize <= 0 ){
            throw new IOException("Failed because file size is zero");
        }

        if( enc ) {
            cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
            cipher.init(Cipher.DECRYPT_MODE, new SecretKeySpec(byte16.getBytes(), "AES"));

            CipherInputStream cipherInputStream = new CipherInputStream(new FileInputStream(zipPath), cipher);
            in = new ZipInputStream(cipherInputStream);
        }else{
            in = new ZipInputStream(new FileInputStream(zipPath));
        }

        try {
            ZipEntry zipEntry;
            while ((zipEntry = in.getNextEntry()) != null) {
                String name = zipEntry.getName();

                if (name.contains("..")) {
                    throw new IOException("Relative paths not allowed"); // For security purposes
                }
                File outputFile = new File(outputPath, name);
                if (zipEntry.isDirectory()) {
                    if (!outputFile.mkdirs() && !outputFile.exists()) {
                        throw new IOException("Failed to create directory: " + outputFile);
                    }
                    if( LL.D ) Log.d(TAG,"  - unzip: made folder '" + name + "'");
                } else {
                    if( LL.D ) Log.d(TAG,"  - unzip: unzipping file '" + name + "' " + zipEntry.getCompressedSize() + "->" + zipEntry.getSize() + " (" + zipEntry.getMethod() + ")");
                    FileOutputStream out = new FileOutputStream(outputFile);
                    try {
                        copy(in, out);
                    } finally {
                        CloseUtil.closeSilently(out);
                    }
                    progressSize +=  zipEntry.getCompressedSize();
                    int progress = (int)(progressSize*100/totalSize);
                    if(progress >= 100 ){
                        progress = 99;
                    }
                    task.setProgress(progress  , 100);
                }
            }
        } finally {
            CloseUtil.closeSilently(in);
        }
        task.setProgress(100, 100);
        if( LL.D ) Log.d(TAG,"Unzipping DONE for: '" + zipPath + "' to '" + outputPath + "'");

    }

    public File getUnzipFolder(int idx){
        return new File(localRootPath, ""+idx);
    }

    public static void copy(InputStream input, OutputStream output) throws IOException {
        byte[] copybuf = new byte[1024*4];
        long count = 0;
        int n = 0;
        while (-1 != (n = input.read(copybuf))) {
            output.write(copybuf, 0, n);
            count += n;
        }
    }

    private static String readFromFile(File file) {
        String content;
        try {
            StringBuilder stringBuilder;
            InputStream inputStream = new BufferedInputStream(new FileInputStream(file));
            InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
            BufferedReader bufferReader = new BufferedReader(inputStreamReader);
            stringBuilder = new StringBuilder();
            String receiveString;

            while ((receiveString = bufferReader.readLine()) != null) {
                stringBuilder.append(receiveString);
                stringBuilder.append("\n");
            }
            inputStream.close();
            content = stringBuilder.toString();
        } catch (FileNotFoundException e) {
            return e.getMessage();
        } catch (IOException e) {
            return e.getMessage();
        }
        return content;
    }

    private boolean copyFile(File file , String save_file){
        boolean result;
        if(file!=null&&file.exists()){
            try {
                FileInputStream fis = new FileInputStream(file);
                FileOutputStream newfos = new FileOutputStream(save_file);
                int readcount=0;
                byte[] buffer = new byte[1024];
                while((readcount = fis.read(buffer,0,1024))!= -1){
                    newfos.write(buffer,0,readcount);
                }
                newfos.close();
                fis.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
            result = true;
        }else{
            result = false;
        }
        return result;
    }

    private boolean copyThumbnail(String src, String dest){

        File thumbFile = new File(src);
        File out = new File(dest);
        if( out.isFile() ){
            out.delete();
        }
        if( thumbFile.isFile() ){
            BitmapFactory.Options opts = new BitmapFactory.Options();
            opts.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(thumbFile.getAbsolutePath(), opts);

            opts.inJustDecodeBounds = false;
            int sampleSize = 1;
            while( 	sampleSize < 8 && ( opts.outWidth / sampleSize > 320 && opts.outHeight / sampleSize > 180) )
            {
                sampleSize *= 2;
            }
            opts.inSampleSize = sampleSize;

            Bitmap bmp = BitmapFactory.decodeFile(thumbFile.getAbsolutePath(),opts);

            if( bmp == null ){
                thumbFile.delete();
                return false;
            }

            FileOutputStream fos = null;
            try {
                out.createNewFile();
                fos = new FileOutputStream(out);
                bmp.compress(Bitmap.CompressFormat.PNG, 100, fos);
            }catch (final FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }finally {
                if (fos != null) {
                    try {
                        fos.close();
                    } catch (IOException e) {
                        /**
                         * can ignore exception.
                         */
                    }
                }
            }
            thumbFile.delete();
            return true;
        }
        return false;
    }

    public static class remoteAssetItem{
        public int idx;
        public String id;
        public String name;
        public String thumbnailPath;
        public String thumbnailURL;
        public String category;
    }

    private Map <Integer,ArrayList<remoteAssetItem>> mFeaturedList = new HashMap<Integer,ArrayList<remoteAssetItem>>();

    public static boolean isUpdatedFeaturedList(int index, String data){
        File list = new File(localFeaturedPath+File.separator+""+index+".json");
        boolean rval = true;
        if( list.isFile() ) {
            try {
                FileInputStream in = new FileInputStream(list);
                byte[] read = new byte[(int)list.length()];
                in.read(read);

                if( Arrays.equals(read, data.getBytes()) ){
                    Log.d(TAG,"FeaturedList equals index="+index);
                    rval = false;
                }
                in.close();
            } catch (FileNotFoundException e) {
                //e.printStackTrace();
            } catch (IOException e) {
                Log.d(TAG,"FeaturedList IOException");
            }
        }
        return rval;
    }

    public static void saveFeaturedList(int index, String data){
        String path = localFeaturedPath;
        try {
            FileOutputStream os = new FileOutputStream(path+File.separator+""+index+".json");
            try {
                os.write(data.getBytes());
            } catch (IOException e) {
                //e.printStackTrace();
            }finally {
                try {
                    os.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        } catch (FileNotFoundException e) {
            //e.printStackTrace();
        }
    }

    public static void saveFeaturedThumbnail(int assetIdx, final Bitmap bmp){
        File out = new File(localFeaturedPath,""+assetIdx);
        FileOutputStream fos = null;

        if( out.isFile() ){

            BitmapFactory.Options opts = new BitmapFactory.Options();
            opts.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(out.getAbsolutePath(), opts);
            //TODO : exception no image?
            if( opts.outWidth != bmp.getWidth() && opts.outHeight != bmp.getHeight() ){
                Log.d(TAG,"saveFeaturedThumbnail assetIdx="+assetIdx+", is not bmp . size="+out.length());
                out.delete();
            }else{
                long diff = System.currentTimeMillis()-out.lastModified();
                Log.d(TAG,"saveFeaturedThumbnail assetIdx="+assetIdx+", exists. lastModified="+diff);
                return;
            }
        }

        try {
            out.createNewFile();
            fos = new FileOutputStream(out);
            bmp.compress(Bitmap.CompressFormat.PNG, 100, fos);
        }catch (final FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    /**
                     * can ignore exception.
                     */
                }
            }
        }
    }

    private void moveFeaturedList(String vendor){
        File dir = new File(assetStoreRootPath + File.separator + vendor);

        if( dir.isDirectory() ){
            for(String filename : dir.list() ){
                if( filename.startsWith(".")  ){
                    continue;
                }

                if( filename.endsWith(".json") ){
                    File file = new File(dir.getAbsolutePath(),filename);
                    String destPath = localFeaturedPath + File.separator + filename;
                    if (!copyFile(file, destPath)) {
                        Log.d(TAG, "copyFile fail!");
                    }
                    file.delete();

                    continue;
                }
                copyThumbnail(dir.getAbsolutePath()+File.separator+filename, localFeaturedPath+File.separator+filename);
            }
        }
    }

    private boolean getFeaturedList(String path, int mode, boolean validateCheck){
        File dir = new File(path);
        boolean ret = false;
        ArrayList<remoteAssetItem> modeList = mFeaturedList.get(mode);
        if( modeList == null ){
            modeList = new ArrayList<>();
            mFeaturedList.put(mode,modeList);
        }

        if( dir.isDirectory() ){
            File listFile = new File(dir.getAbsolutePath(),""+mode+".json");
            if( listFile.isFile() ) {
                try {
                    InputStream in =null;
                    try {
                        in = new FileInputStream(listFile);
                        Gson gson = new Gson();
                        AssetStoreAPIData.GetNewAssetList list = gson.fromJson(new InputStreamReader(in),AssetStoreAPIData.GetNewAssetList.class);
                        if( list == null )
                            return false;

                        if(list.objList == null)
                            return false;
                        int total = list.objList.size();

                        if( total > 0 ){
                            modeList.clear();

                            for(AssetStoreAPIData.AssetInfo info :  list.objList ){
//check thumbnail
                                if( validateCheck ) {
                                    File thumbFile = new File(path, "" + info.idx);
                                    if (!thumbFile.isFile()) {
                                        Log.d(TAG, "getFeaturedList() mode=" + mode + ", idx=" + info.idx + ", thumbnail not found!");
                                        continue;
                                    }
                                }

                                remoteAssetItem ent = new remoteAssetItem();
                                ent.id = info.asset_id;
                                ent.idx = info.idx;
                                ent.name = info.assetName.get(0).string_title;
                                String lang = mContext.getResources().getConfiguration().locale.getLanguage();
                                for(int i = 0; i<info.assetName.size(); i++){
                                    if (info.assetName.get(i).language_code.equals(lang) && info.assetName.get(i).string_title != null) {
                                        ent.name = info.assetName.get(i).string_title;
                                        break;
                                    }
                                }
                                ent.thumbnailPath = path+File.separator+ent.idx;
                                ent.thumbnailURL = info.thumbnail_path_s;
                                if( info.category_aliasName != null ) {
                                    ent.category = info.category_aliasName;
                                }else{
                                    ent.category = "None";
                                }
                                modeList.add(ent);
                            }
                            ret = true;
                        }

                    }finally {
                        in.close();
                    }
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }catch (IOException e) {
                    e.printStackTrace();
                }catch (JsonSyntaxException e){
                    Log.e(TAG,"getFeaturedList err!");
                    e.printStackTrace();
                }catch (JsonIOException e){
                    Log.e(TAG,"getFeaturedList err!");
                    e.printStackTrace();
                }

                if( !ret ){

                    try {
                        InputStream dump = null;
                        try {
                            dump = new FileInputStream(listFile);
                            byte[] buf = new byte[(int) listFile.length()];
                            dump.read(buf);
                            Log.d(TAG,"ErrJson:"+String.valueOf(buf));
                        }finally {
                            dump.close();
                        }
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    }catch (IOException e) {
                        e.printStackTrace();
                    }
                }

            }else{
                Log.d(TAG,"file not found="+listFile.getAbsolutePath());
            }
        }
        return ret;
    }

    public List <remoteAssetItem> getRemoteAssetItem(int mode){
        getFeaturedList(localFeaturedPath, mode,false);
        return mFeaturedList.get(mode);
    }

    public List <remoteAssetItem> getList(int mode, String vendor){
        //if( vendor != null )
        //    moveFeaturedList(vendor);
        getFeaturedList(localFeaturedPath, mode,true);
        return mFeaturedList.get(mode);
    }

    public String getThumbnailUrl(int idx){

        List <remoteAssetItem> list = mFeaturedList.get(1);
        if( list != null ){
            for( remoteAssetItem item : list ){
                if( item.idx == idx ){
                    return item.thumbnailURL;
                }
            }
        }

        list = mFeaturedList.get(2);
        if( list != null ){
            for( remoteAssetItem item : list ){
                if( item.idx == idx ){
                    return item.thumbnailURL;
                }
            }
        }

        return null;
    }
}
