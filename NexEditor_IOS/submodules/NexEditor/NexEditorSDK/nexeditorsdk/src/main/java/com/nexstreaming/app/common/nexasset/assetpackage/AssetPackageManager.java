package com.nexstreaming.app.common.nexasset.assetpackage;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.database.sqlite.SQLiteFullException;
import android.graphics.Typeface;
import android.os.Build;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.store.AbstractStoreAssetInfo;
import com.nexstreaming.app.common.nexasset.store.StoreAssetInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.db.AssetPackageDb;
import com.nexstreaming.app.common.nexasset.assetpackage.db.AssetPackageRecord;
import com.nexstreaming.app.common.nexasset.assetpackage.db.CategoryRecord;
import com.nexstreaming.app.common.nexasset.assetpackage.db.InstallSourceRecord;
import com.nexstreaming.app.common.nexasset.assetpackage.db.ItemRecord;
import com.nexstreaming.app.common.nexasset.assetpackage.db.SubCategoryRecord;
import com.nexstreaming.app.common.nexasset.store.json.AssetStoreAPIData;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.app.common.util.NexEDLConverter;
import com.nexstreaming.app.common.util.PathUtil;
import com.nexstreaming.app.common.util.StreamUtil;
import com.nexstreaming.kminternal.kinemaster.fonts.Font;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.fonts.FontManager;
import com.nexstreaming.kminternal.nexvideoeditor.EffectResourceLoader;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.nexeditorsdk.BuildConfig;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

// MATTFIX: Need packaging tool?   ENCRYPTION

public class AssetPackageManager {

    private static final String LOG_TAG = "AssetPackageManager";
    private static final long LOCAL_SUBCATEGORY_IDX = -1;
    private static AssetPackageManager instance;
    private final AssetPackageDb assetPackageDb;
    private InstallSourceRecord storeInstallSourceCache = null;

    public static AssetPackageManager getInstance(Context context) {
        if(instance==null) {
            instance = new AssetPackageManager(context.getApplicationContext());
        }
        return instance;
    }

    public static AssetPackageManager getInstance() {
        if(instance==null) {
            instance = new AssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        }
        return instance;
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    public static AssetPackageManager getInstance(Context context, Resources resources) {
        if(instance==null) {
            instance = new AssetPackageManager( context.getApplicationContext().createConfigurationContext(resources.getConfiguration()));
        }
        return instance;
    }

    private AssetPackageManager(Context context) {
        assetPackageDb = new AssetPackageDb(context);
    }
/*
    public void installStorePackage(File path, File thumbFile, String storeAssetInfoJSON ) throws IOException {
        Gson gson = new Gson();
        StoreAssetInfo storeAssetInfo = gson.fromJson(storeAssetInfoJSON, StoreAssetInfo.class);
        installStorePackage(path, thumbFile, storeAssetInfo);
    }
*/
    public boolean itemExistsInCategory( String itemId, ItemCategory category ) {
        itemId = itemId.substring(itemId.indexOf('/')+1);
        ItemInfo item = getInstalledItemById(itemId);
        return (item!=null && item.getCategory()==category );
    }

    public boolean needsUnzipBeforeInstall(File path) throws IOException {
        AssetPackageReader reader = AssetPackageReader.readerForZipPackage(path, null);
        try {
            for( ItemInfo item: reader.getItems() ) {
                switch(item.getType()) {
                    case audio:
                    case font:
                        return true;
                }
            }
        } finally {
            CloseUtil.closeSilently(reader);
        }
        return false;
    }

    public void installStorePackage(File path, File thumbFile, StoreAssetInfo storeAssetItem ) throws IOException {
        AssetPackageReader reader;
        if( path.isDirectory() ) {
            reader = AssetPackageReader.readerForFolder(path, storeAssetItem.getAssetId());
        } else {
            reader = AssetPackageReader.readerForZipPackage(path, storeAssetItem.getAssetId());
        }
        installPackageInternal(reader, thumbFile, storeAssetItem, getStoreInstallSource());
    }

    private InstallSourceRecord getStoreInstallSource() {
        if( storeInstallSourceCache!=null )
            return storeInstallSourceCache;
        InstallSourceRecord isr = assetPackageDb.findFirst(InstallSourceRecord.class, "install_source_id = ?", "store");
        if( isr==null ) {
            isr = new InstallSourceRecord();
            isr.installSourceId = "store";
            isr.installSourceType = InstallSourceType.STORE;
            isr.installSourceVersion = 0;
            assetPackageDb.add(isr);
        }
        storeInstallSourceCache = isr;
        return isr;
    }

    private InstallSourceRecord getInstallSource(InstallSourceType installSourceType, String path) {
        String installSource;
        switch( installSourceType ) {
            case STORE:
                return getStoreInstallSource();
            case APP_ASSETS:
                installSource = "assets:" + path;
                break;
            case FOLDER:
                installSource = "file:" + path;
                break;
            default:
                throw new IllegalArgumentException();
        }
        InstallSourceRecord isr = assetPackageDb.findFirst(InstallSourceRecord.class, "install_source_id = ?", installSource);
        if( isr==null ) {
            isr = new InstallSourceRecord();
            isr.installSourceId = installSource;
            isr.installSourceType = installSourceType;
            isr.installSourceVersion = 0;
            assetPackageDb.add(isr);
        }
        return isr;
    }

//    private void installPackageInternal(AssetPackageReader reader, File thumbFile, StoreAssetInfo storeAssetInfo, InstallSourceRecord installSourceRecord) throws IOException {
    private void installPackageInternal(AssetPackageReader reader, File thumbFile, StoreAssetInfo storeAssetInfo, InstallSourceRecord installSourceRecord) throws IOException {

        assetPackageDb.beginTransaction();
        try {

            CategoryRecord categoryRecord = null;
            if (storeAssetInfo.getCategoryIndex() != 0) {
                categoryRecord = assetPackageDb.findFirst(CategoryRecord.class, "category_id = ?", storeAssetInfo.getCategoryIndex());
                if (categoryRecord == null) {
                    categoryRecord = new CategoryRecord();
                    categoryRecord.categoryId = storeAssetInfo.getCategoryIndex();
                    categoryRecord.categoryName = storeAssetInfo.getCategoryAliasName();
                    categoryRecord.categoryIconURL = storeAssetInfo.getCategoryIconURL();
                    assetPackageDb.addOrUpdate(categoryRecord);
                    //} else if (!categoryRecord.categoryIconURL.equals(storeAssetInfo.getCategoryIconURL())) {
                }else{
                    if( categoryRecord.categoryIconURL != null && storeAssetInfo.getCategoryIconURL() != null ){
                        if (!categoryRecord.categoryIconURL.equals(storeAssetInfo.getCategoryIconURL())) {
                            categoryRecord.categoryIconURL = storeAssetInfo.getCategoryIconURL();
                            assetPackageDb.update(categoryRecord);
                        }
                    }
                }
            }

            SubCategoryRecord subCategoryRecord = null;
            if (storeAssetInfo.getSubCategoryIndex() != 0) {
                subCategoryRecord = assetPackageDb.findFirst(SubCategoryRecord.class, "sub_category_id = ?", storeAssetInfo.getSubCategoryIndex());
                if (subCategoryRecord == null) {
                    subCategoryRecord = new SubCategoryRecord();
                    subCategoryRecord.subCategoryId = storeAssetInfo.getSubCategoryIndex();
                    subCategoryRecord.subCategoryName = storeAssetInfo.getSubCategoryNameMap();
                    subCategoryRecord.subCategoryAlias = storeAssetInfo.getSubCategoryAliasName();
                    assetPackageDb.add(subCategoryRecord);
                } else {
                    subCategoryRecord.subCategoryName = storeAssetInfo.getSubCategoryNameMap();
                    assetPackageDb.update(subCategoryRecord);
                }
            } else {
                subCategoryRecord = assetPackageDb.findFirst(SubCategoryRecord.class, "sub_category_id = ?", LOCAL_SUBCATEGORY_IDX);
                if (subCategoryRecord == null) {
                    Map<String,String> nameMap = new HashMap<>();
                    nameMap.put("en","Local");
                    subCategoryRecord = new SubCategoryRecord();
                    subCategoryRecord.subCategoryId = LOCAL_SUBCATEGORY_IDX;
                    subCategoryRecord.subCategoryAlias = "local";
                    subCategoryRecord.subCategoryName = nameMap;
                    assetPackageDb.add(subCategoryRecord);
//                } else {
//                    subCategoryRecord.subCategoryName = storeAssetInfo.getSubcategoryNameMap();
//                    assetPackageDb.update(subCategoryRecord);
                }
            }

            AssetPackageRecord apr = new AssetPackageRecord();
            apr.assetIdx = storeAssetInfo.getAssetIndex();
            apr.packageURI = reader.getPackageURI();
            apr.assetId = storeAssetInfo.getAssetId();
            apr.assetUrl = storeAssetInfo.getAssetPackageDownloadURL();
            apr.thumbPath = thumbFile == null ? null : thumbFile.getAbsolutePath();
            apr.assetDesc = null;
            apr.assetName = storeAssetInfo.getAssetNameMap();
            if( apr.assetName.size()<1 ) {
                apr.assetName = reader.getAssetName();
            }
            apr.priceType = storeAssetInfo.getPriceType();
            apr.thumbUrl = storeAssetInfo.getAssetThumbnailURL();
            apr.installSource = installSourceRecord;
            apr.category = categoryRecord;
            apr.subCategory = subCategoryRecord;
            File localPath = reader.getAssetPackageLocalPath();
            apr.localPath = localPath==null?null:localPath.getAbsolutePath();
            apr.expireTime = storeAssetInfo.getExpireTime();
            apr.installedTime = System.currentTimeMillis();
            apr.minVersion = storeAssetInfo.getAssetScopeVersion();
            apr.packageVersion = storeAssetInfo.getAssetVersion();

            assetPackageDb.add(apr);

            int itemCount = 0;

            try {
                List<ItemInfo> items = reader.getItems();
                for (ItemInfo item : items) {
                    ItemRecord itemRecord = new ItemRecord();
                    itemRecord.assetPackageRecord = apr;
                    itemRecord.itemId = item.getId();
                    itemRecord.packageURI = item.getPackageURI();
                    itemRecord.filePath = item.getFilePath();
                    itemRecord.iconPath = item.getIconPath();
                    itemRecord.hidden = item.isHidden();
                    itemRecord.thumbPath = item.getThumbPath();
                    itemRecord.label = item.getLabel();
                    itemRecord.itemType = item.getType();
                    itemRecord.itemCategory = item.getCategory();
                    itemRecord.sampleText = item.getSampleText();
                    assetPackageDb.add(itemRecord);
                    itemCount++;
                }
            } finally {
                reader.close();
            }
            assetPackageDb.setTransactionSuccessful();
            if( LL.D ) Log.d(LOG_TAG,"Added DB Record for: " + apr.assetId + " and " + itemCount + " items.");
        } finally {
            try {
                assetPackageDb.endTransaction();
            } catch (SQLiteFullException e) {
            	if( LL.D ) Log.d(LOG_TAG, "AssetPackageDb.endTransaction() throws SQLiteFullException. e="+e);
                throw new IOException(e);
            }
        }

    }

    public void syncPackagesFromAndroidAssets(Context context, String path, long versionCode) throws IOException {

        if( LL.D ) Log.d(LOG_TAG,"syncPackagesFromAndroidAssets - IN : " + path);

        InstallSourceRecord isr = getInstallSource(InstallSourceType.APP_ASSETS, path);

        //int versionCode = 6;
        //long versionCode = BuildConfig.BUILD_DATE;

        if( LL.D ) Log.d(LOG_TAG,"syncPackagesFromAndroidAssets - ISR CHECK: " + isr.installSourceVersion + " / " + versionCode);

        if( isr.installSourceVersion!=versionCode ) {
                if( LL.D ) Log.d(LOG_TAG,"syncPackagesFromAndroidAssets - ISR MISMATCH; UPDATING : " + isr.installSourceVersion + " -> " + versionCode);

                isr.installSourceVersion = 0;
                assetPackageDb.update(isr);

                unregisterAllPackages(isr);

                AssetManager assets = context.getAssets();
            for (final String assetFileName : assets.list(path)) {
                String[] contents;
                String packagePath = combinePaths(path, assetFileName);
                try {
                    contents = assets.list(packagePath);
                } catch (IOException e) {
                    contents = null;
                }
                if( contents!=null && contents.length > 0 ) {
                    if( LL.D ) Log.d(LOG_TAG,"syncPackagesFromAndroidAssets - Processing package: " + assetFileName + " (in " + packagePath + ")");
                    AssetPackageReader reader = AssetPackageReader.readerForAndroidAppAssets(assets,packagePath,assetFileName);
                    //final String title = reader.getAssetName().get("en");
                    StoreAssetInfo sai = new AbstractStoreAssetInfo(reader.getMinVersion(),reader.getPackageFormatVersion()) {
                        @Override
                        public String getAssetId() {
                            return assetFileName;
                        }

                        @Override
                        public String getAssetTitle() {
                            return assetFileName;
                        }

                        @Override
                        public String getSubCategoryAliasName() {
                            return "local";
                        }
                    };

                    installPackageInternal(reader, null, sai, isr);
                }
            }

            isr.installSourceVersion = versionCode;
            assetPackageDb.update(isr);

        }

        if( LL.D ) Log.d(LOG_TAG,"syncPackagesFromAndroidAssets - OUT");

    }


    public void unregisterPackagesLoadedFromFolder(File path) {
        InstallSourceRecord isr = getInstallSource(InstallSourceType.FOLDER,path.getAbsolutePath());
        unregisterAllPackages(isr);
        isr.installSourceVersion = 0;
        assetPackageDb.update(isr);
    }

    public boolean syncPackagesFromFolder(File path) throws IOException {

        InstallSourceRecord isr = getInstallSource(InstallSourceType.FOLDER,path.getAbsolutePath());

        File[] files = path.listFiles();
        if( files==null || files.length<1 ) {
            unregisterAllPackages(isr);
            return false;
        }

        long lastModified = 0;
        for( File f: files ) {
            lastModified = Math.max(lastModified,f.lastModified());
        }

        if( isr.installSourceVersion!=lastModified ) {

            isr.installSourceVersion = 0;
            assetPackageDb.update(isr);

            unregisterAllPackages(isr);

            for (File f: files) {
                String baseId = null;
                AssetPackageReader reader = null;
                if( f.isDirectory() ) {
                    baseId = f.getName();
                    reader = AssetPackageReader.readerForFolder(f, baseId);
                } else if( f.getName().endsWith(".zip") ) {
                    baseId = f.getName();
                    baseId = baseId.substring(0, baseId.length() - 4); // Remove .zip extension
                    reader = AssetPackageReader.readerForZipPackage(f, baseId);
                }

                if( reader!=null ) {
                    final String finalBaseId = baseId;
                    //final String title = reader.getAssetName().get("en");
                    StoreAssetInfo sai = new AbstractStoreAssetInfo(reader.getMinVersion(),reader.getPackageFormatVersion()) {
                        @Override
                        public int getAssetIndex() {
                            return 1;
                        }

                        @Override
                        public String getAssetId() {
                            return finalBaseId;
                        }

                        @Override
                        public String getSubCategoryAliasName() {
                            return "local";
                        }
                    };

                    installPackageInternal(reader, null, sai, isr);
                }
            }

            isr.installSourceVersion = lastModified;
            assetPackageDb.update(isr);

            return true;
        }
        return false;
    }

    public void unregisterAllPackages(InstallSourceRecord installSource) {
        assetPackageDb.beginTransaction();
        try {
            List<AssetPackageRecord> aprs = assetPackageDb.query(AssetPackageRecord.class, "install_source = ?", installSource.getDbRowID());
            for( AssetPackageRecord apr: aprs ) {
                List<ItemRecord> items = assetPackageDb.query(ItemRecord.class, "asset_package_record = ?", apr.getDbRowID());
                for( ItemRecord item: items ) {
                    assetPackageDb.delete(item);
                }
                assetPackageDb.delete(apr);
            }
            assetPackageDb.setTransactionSuccessful();
        } finally {
            try {
                assetPackageDb.endTransaction();
            } catch (SQLiteFullException e) {
				if( LL.D ) Log.d(LOG_TAG, "AssetPackageDb.endTransaction() throws SQLiteFullException. e="+e);
            }
        }
    }

    public void unregisterPackageByAssetIdx(int assetIdx) {
        assetPackageDb.beginTransaction();
        try {
            AssetPackageRecord apr = assetPackageDb.findFirst(AssetPackageRecord.class, "asset_idx = ?", assetIdx);
            List<ItemRecord> items = assetPackageDb.query(ItemRecord.class, "asset_package_record = ?", apr.getDbRowID());
            for( ItemRecord item: items ) {
                assetPackageDb.delete(item);
            }
            assetPackageDb.delete(apr);
            assetPackageDb.setTransactionSuccessful();
        } finally {
            try {
                assetPackageDb.endTransaction();
            } catch (SQLiteFullException e) {
				if( LL.D ) Log.d(LOG_TAG, "AssetPackageDb.endTransaction() throws SQLiteFullException. e="+e);
            }
        }
    }

    public void unregisterPackageByAssetId(String assetId) {
        assetPackageDb.beginTransaction();
        try {
            AssetPackageRecord apr = assetPackageDb.findFirst(AssetPackageRecord.class, "asset_id = ?", assetId);
            List<ItemRecord> items = assetPackageDb.query(ItemRecord.class, "asset_package_record = ?", apr.getDbRowID());
            for( ItemRecord item: items ) {
                assetPackageDb.delete(item);
            }
            assetPackageDb.delete(apr);
            assetPackageDb.setTransactionSuccessful();
        } finally {
            try {
                assetPackageDb.endTransaction();
            } catch (SQLiteFullException e) {
				if( LL.D ) Log.d(LOG_TAG, "AssetPackageDb.endTransaction() throws SQLiteFullException. e="+e);
            }
        }
    }

    // TODO: Register DB change listeners

    public AssetInfo getInstalledPackageInfo(String assetId) {
        return assetPackageDb.findFirst(AssetPackageRecord.class, "asset_id = ?", assetId);
    }

    public AssetInfo getInstalledPackageInfoByAssetIdx(int assetIdx){
        return assetPackageDb.findFirst(AssetPackageRecord.class, "asset_idx = ?", assetIdx);
    }

    public List<String> getInstalledPackageAssetIdList() {
        return assetPackageDb.queryIndividualField(AssetPackageRecord.class, "asset_id", null);
    }

    public List<? extends AssetInfo> getInstalledAssetPackages() {
        return assetPackageDb.query(AssetPackageRecord.class);
    }

    public List<? extends ItemInfo> getInstalledItemsByAssetPackage(String assetPackageId) {
        long rowId = assetPackageDb.findFirstRowId(AssetPackageRecord.class, "asset_id = ?", assetPackageId);
        return assetPackageDb.query(ItemRecord.class,"asset_package_record = ?",rowId);
    }

    public List<? extends ItemInfo> getInstalledItemsByAssetPackageIdx(int assetPackageIdx) {
        long rowId = assetPackageDb.findFirstRowId(AssetPackageRecord.class, "asset_idx = ?", assetPackageIdx);
        return assetPackageDb.query(ItemRecord.class,"asset_package_record = ?",rowId);
    }

    public List<? extends ItemInfo> getInstalledItemsByCategory(ItemCategory category) {
        return assetPackageDb.query(ItemRecord.class,"item_category = ?",category);
    }

    public List<? extends ItemInfo> getInstalledItemsBySubCategory(AssetSubCategory subCategory, ItemCategory itemCategory) {
        List<ItemInfo> result = new ArrayList<>();
        for( ItemInfo item: getInstalledItemsByCategory(itemCategory) ) {
            if( item.getAssetPackage().getAssetSubCategory().equals(subCategory) )
                result.add(item);
        }
        return result;
    }


    public List<? extends ItemInfo> getInstalledItemsByAssetPackageAndCategory(String assetPackageId, ItemCategory category) {
        long rowId = assetPackageDb.findFirstRowId(AssetPackageRecord.class, "asset_id = ?", assetPackageId);
        return assetPackageDb.query(ItemRecord.class,"asset_package_record = ? AND item_category = ?",rowId,category);
    }

    public List<? extends ItemInfo> getInstalledItemsByAssetPackageIdxAndCategory(int assetIdx, ItemCategory category) {
        long rowId = assetPackageDb.findFirstRowId(AssetPackageRecord.class, "asset_idx = ?", assetIdx);
        return assetPackageDb.query(ItemRecord.class,"asset_package_record = ? AND item_category = ?",rowId,category);
    }

    public boolean doesAssetPackageContainItemsOfCategory(String assetPackageId, ItemCategory category) {
        long rowId = assetPackageDb.findFirstRowId(AssetPackageRecord.class, "asset_id = ?", assetPackageId);
        return assetPackageDb.count(ItemRecord.class,"asset_package_record = ? AND item_category = ?",rowId,category)>0;
    }

    public boolean doesAssetPackageContainItemsOfCategory(AssetInfo assetInfo, ItemCategory category) {
        long rowId;
        if( assetInfo instanceof AssetPackageRecord ) {
            rowId = ((AssetPackageRecord)assetInfo)._id;
        } else {
            rowId = assetPackageDb.findFirstRowId(AssetPackageRecord.class, "asset_id = ?", assetInfo.getAssetId());
        }
        return assetPackageDb.count(ItemRecord.class,"asset_package_record = ? AND item_category = ?",rowId,category)>0;
    }


    public List<AssetInfo> getInstalledAssetPackagesByItemCategory(ItemCategory category) {

        List<AssetInfo> result = new ArrayList<>();
        for( AssetInfo ai: assetPackageDb.query(AssetPackageRecord.class) ) {
            if(doesAssetPackageContainItemsOfCategory(ai,category)) {
                result.add(ai);
            }
        }
        return result;
    }

    public List<? extends AssetInfo> getInstalledAssetPackagesByCategory(AssetCategory category) {
        if( category instanceof CategoryRecord ) {
            return assetPackageDb.query(AssetPackageRecord.class, "category = ?",((CategoryRecord)category).getDbRowID());
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public List<? extends AssetInfo> getInstalledAssetPackagesBySubCategory(AssetSubCategory subCategory) {
        if( subCategory instanceof SubCategoryRecord ) {
            return assetPackageDb.query(AssetPackageRecord.class, "sub_category = ?",((SubCategoryRecord)subCategory).getDbRowID());
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public List<? extends ItemInfo> getInstalledItems() {
        return assetPackageDb.query(ItemRecord.class);
    }

    public List<? extends AssetCategory> getAssetCategories() {
        return sortAssetCategories(assetPackageDb.query(CategoryRecord.class));
    }

    public List<? extends AssetCategory> getAssetCategoriesWithInstalledItems() {
        List<AssetCategory> result = new ArrayList<>();
        for( CategoryRecord category: assetPackageDb.query(CategoryRecord.class) ) {
            if( assetPackageDb.count(AssetPackageRecord.class, "category = ?",((CategoryRecord)category).getDbRowID()) > 0 ) {
                result.add(category);
            }
        }
        return sortAssetCategories(result);
    }

    private List<AssetCategory> sortAssetCategories(List<? extends AssetCategory> list) {
        // Note:  In reality, we should not sort here; we should get the list in the proper
        // order from the server. However, the server does not currently send order information,
        // so in the meantime, we have hardcoded the order for known asset aliases, and for
        // remaining asset categories, we put them in the list after the known ones in order
        // by the category alias.
        List<AssetCategory> result = new ArrayList<>();
        List<AssetCategory> unsorted = new ArrayList<>(list);
        for( AssetCategoryAlias alias: AssetCategoryAlias.values()) {
            AssetCategory found = null;
            for( AssetCategory ac: unsorted ) {
                if(ac.getCategoryAlias().equalsIgnoreCase(alias.name()) || (ac.getCategoryAlias()).equalsIgnoreCase(alias.name()+"s")) {
                    found = ac;
                    break;
                }
            }
            if( found!=null ) {
                result.add(found);
                unsorted.remove(found);
            }
        }
        if( !unsorted.isEmpty() ) {
            Collections.sort(unsorted, new Comparator<AssetCategory>() {
                @Override
                public int compare(AssetCategory lhs, AssetCategory rhs) {
                    return lhs.getCategoryAlias().compareTo(rhs.getCategoryAlias());
                }
            });
            result.addAll(unsorted);
        }
        return result;
    }

    public ItemInfo getInstalledItemById(String itemId) {
        return assetPackageDb.findFirst(ItemRecord.class,"item_id = ?",itemId);
    }

    public List<? extends AssetSubCategory> getAssetSubCategories() {
        return assetPackageDb.query(SubCategoryRecord.class);
    }

    public Set<AssetSubCategory> getAssetSubCategoriesWithItems(ItemCategory itemCategory) {
        Set<AssetSubCategory> result = new HashSet<>();
        for( ItemInfo ii: getInstalledItems() ) {
            if( ii.getCategory()==itemCategory )
                result.add(ii.getAssetPackage().getAssetSubCategory());
        }
        return result;
    }


//    /**
//     * Asset information from Asset Store.
//     *
//     * This information is decoded from JSON structure sent from the store, and
//     * all of the field names match.  See the JavaScript Asset Store API spec
//     * for details.
//     */
//    private static class StoreAssetInfo {
//
//        public String assetId;
//        public String assetUrl;
//        public String thumbUrl;
//        public String priceType;
//        public long categoryId;
//        public String categoryIconUrl;
//        public long subCategoryId;
//        public Map<String,String> subCategoryName;
//        public Map<String,String> assetName;
//        public Map<String,String> assetDesc;
//
//    }

    private static String combinePaths( String a, String b ) {
        if( b.startsWith("..") || b.contains( "/..") )
            throw new SecurityException("Parent Path References Not Allowed");
        if(a.endsWith("/")) {
            return a + b;
        } else {
            return a + "/" + b;
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

    public void loadEffectsInEditor( Iterable<String> effects, NexEditor editor, boolean forExport, boolean needAdditionalTransition) {
        if( editor==null || effects==null ) {
            return;
        }
        try {
            editor.setThemeData(themeDataForEffects(effects, needAdditionalTransition), forExport);
        } catch (IOException e1) {
            throw new RuntimeException(e1);
        }
    }

    public void loadRenderItemsInEditor( Iterable<String> effects, NexEditor editor, boolean forExport ) {

        if( editor==null || effects==null ) {
            return;
        }

        if(LL.D) Log.d(LOG_TAG,"loadRenderItemsInEditor");

        ByteArrayOutputStream buf = new ByteArrayOutputStream();
        Context context = KineMasterSingleTon.getApplicationInstance().getApplicationContext();

        editor.clearRenderItems(forExport);
        for( String effectId: effects ) {
            if( effectId==null ) {
                continue;
            }
            ItemInfo item = null;
            if( effectId.endsWith(".force_effect") ) {
                item = getInstalledItemById(effectId.substring(0, effectId.length() - ".force_effect".length()));
            }
            else {
                item = getInstalledItemById(effectId);
            }
            if( item==null ) {
                if(LL.W) Log.w(LOG_TAG,"Could not find item for id: " + effectId);
                continue;
            }
            if(item.getType() != ItemType.renderitem)
                continue;

            try{
                AssetPackageReader reader = AssetPackageReader.readerForPackageURI(context,item.getPackageURI(),item.getAssetPackage().getAssetId());
                try {
                    InputStream in = reader.openFile(item.getFilePath());
                    buf.reset();
                    StreamUtil.copy(in, buf);
                } finally {
                    reader.close();
                }
            }
            catch (IOException e1) {
                throw new RuntimeException(e1);
            }

            editor.setRenderItem(effectId, buf.toString(), forExport);
        }
    }

    public String renderItemDataForEffects(Iterable<String> effects) throws IOException {
        Context context = KineMasterSingleTon.getApplicationInstance().getApplicationContext();
        ByteArrayOutputStream buf = new ByteArrayOutputStream();
        StringBuilder sb = new StringBuilder();
        for( String effectId: effects ) {
            if( effectId==null ) {
                continue;
            }
            ItemInfo item = null;
            if( effectId.endsWith(".force_effect") ) {
                item = getInstalledItemById(effectId.substring(0, effectId.length() - ".force_effect".length()));
            }
            else {
                item = getInstalledItemById(effectId);
            }
            if( item==null ) {
                if(LL.W) Log.w(LOG_TAG,"Could not find item for id: " + effectId);
                continue;
            }
            if(item.getType() != ItemType.renderitem)
                continue;

            try{
                AssetPackageReader reader = AssetPackageReader.readerForPackageURI(context,item.getPackageURI(),item.getAssetPackage().getAssetId());
                try {
                    InputStream in = reader.openFile(item.getFilePath());
                    buf.reset();
                    StreamUtil.copy(in, buf);
                } finally {
                    reader.close();
                }
                sb.append(buf.toString());
            }
            catch (IOException e1) {
                throw new RuntimeException(e1);
            }
        }
        return sb.toString();
    }

    public String themeDataForEffects( Iterable<String> effectIds, boolean needAdditionalTransition ) throws IOException {

        Context context = KineMasterSingleTon.getApplicationInstance().getApplicationContext();

        StringBuilder sb = new StringBuilder();

        sb.append("<themeset name=\"KM\" defaultTheme=\"none\" defaultTransition=\"none\" >");
        sb.append("<texture id=\"video_out\" video=\"1\" />");
        sb.append("<texture id=\"video_in\" video=\"2\" />");

        ByteArrayOutputStream buf = new ByteArrayOutputStream();

        for( String effectId: effectIds ) {
            if( effectId==null ) {
                continue;
            }

            ItemInfo item = null;
            if( effectId.endsWith(".force_effect") ) {
                item = getInstalledItemById(effectId.substring(0, effectId.length() - ".force_effect".length()));
            }
            else {
                item = getInstalledItemById(effectId);
            }

            if( item==null ) {
                if(LL.W) Log.w(LOG_TAG,"Could not find item for id: " + effectId);
                continue;
            }

            if(item.getType() == ItemType.renderitem)
                continue;

            AssetPackageReader reader = AssetPackageReader.readerForPackageURI(context,item.getPackageURI(),item.getAssetPackage().getAssetId());
            try {
                InputStream in = reader.openFile(item.getFilePath());
                buf.reset();
                StreamUtil.copy(in, buf);

            } finally {
                reader.close();
            }

            Log.d(LOG_TAG,"template effects: " + item.getId());

            if( !needAdditionalTransition && effectId.endsWith(".force_effect") ) {
                Log.d(LOG_TAG,"Original template transition: " + buf.toString());
                String modify = NexEDLConverter.Transition2Title(buf.toString(), effectId);

                Log.d(LOG_TAG,"Modify template transition: " + modify);
                sb.append(modify);
            }
            else {
                sb.append(buf.toString());
            }
        }

        sb.append("</themeset>");

        // Log.d(LOG_TAG, sb.toString());

        return sb.toString();
    }

    public boolean checkExpireAsset(AssetInfo assetInfo ){
        if( assetInfo == null ){
            Log.d(LOG_TAG,"checkExpireAsset assetinfo is null.");
            return true;
        }

        if( assetInfo.getExpireTime() > 0 ){
            long maxExpireTime = assetInfo.getInstalledTime() + assetInfo.getExpireTime();
            long minExpireTime = assetInfo.getInstalledTime() - (60*60*24*1000);// 1 Day previous margin.
            long systemTime = System.currentTimeMillis();
            if( maxExpireTime < systemTime || minExpireTime > systemTime ){
                return true;
            }
        }
        return false;
    }

    public EffectResourceLoader getResourceLoader() {
        return new EffectResourceLoader() {
            @Override
            public File getAssociatedFile(String effect_id, String filename) throws IOException {
                throw new UnsupportedOperationException();
            }

            @Override
            public AssetPackageReader getReader(String effect_id) throws IOException {
                ItemInfo item = getInstalledItemById(effect_id);
                AssetPackageReader reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), item.getPackageURI(), item.getAssetPackage().getAssetId());
                return reader;
            }

            @Override
            public InputStream openAssociatedFile(String effect_id, String filename) throws IOException {
                ItemInfo item = getInstalledItemById(effect_id);
                AssetPackageReader reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),item.getPackageURI(),item.getAssetPackage().getAssetId());
                String fullPath;
                if( filename==null ) {
                    fullPath = item.getFilePath();
                } else {
                    fullPath = relativePath(item.getFilePath(),filename);
                }
                return reader.openFile(fullPath);
            }

            @Override
            public Typeface getTypeface(String typeface_id) throws Font.TypefaceLoadException {
                return FontManager.getInstance().getTypeface(typeface_id);
//                if( LL.D ) Log.d(LOG_TAG,"Get typeface: " + typeface_id);
//                ItemInfo item = getInstalledItemById(typeface_id);
//                if( item==null ) {
//                    if( LL.W ) Log.w(LOG_TAG,"Typeface not found: " + typeface_id);
//                    return null;
//                } else {
//                    AssetPackageReader reader;
//                    try {
//                        reader = AssetPackageReader.readerForPackageURI(KineMasterApplication.getApplicationInstance().getApplicationContext(),item.getPackageURI(),item.getAssetPackage().getAssetId());
//                    } catch (IOException e) {
//                        if( LL.E ) Log.e(LOG_TAG,"Error loading typeface: " + typeface_id,e);
//                        return null;
//                    }
//                    try {
//                        return reader.getTypeface(item.getFilePath());
//                    } catch (AssetPackageReader.LocalPathNotAvailableException e) {
//                        if( LL.E ) Log.e(LOG_TAG,"Error loading typeface: " + typeface_id,e);
//                        return null;
//                    } finally {
//                        CloseUtil.closeSilently(reader);
//                    }
//                }
            }

            public Typeface getTypeface(String effect_id, String filename) throws Font.TypefaceLoadException, IOException {
                ItemInfo item = getInstalledItemById(effect_id);
                AssetPackageReader reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),item.getPackageURI(),item.getAssetPackage().getAssetId());

                String packageURI = reader.getPackageURI();
                String path = relativePath(item.getFilePath(),filename);

                if( packageURI.startsWith("file:") ) {
                    String base = packageURI.substring("file:".length());
                    String file = base + "/" +  path;
                    File f = new File(file);
                    if( !f.isFile() ){
                        file = base + "/" + reader.getFilePath(path);
                    }

                    Typeface tf = null;
                    try {
                        tf = Typeface.createFromFile(file);
                    }catch (RuntimeException e) {
                        throw new Font.TypefaceLoadException();
                    }

                    return tf;
                }
                else if (packageURI.startsWith("assets:") ) {
                    AssetManager am = KineMasterSingleTon.getApplicationInstance().getApplicationContext().getAssets();

                    if( am != null ) {
                        String base = packageURI.substring("assets:".length());
                        String file = base + "/" + reader.getFilePath(path);

                        Typeface tf = null;
                        try {
                            tf = Typeface.createFromAsset(am, file);
                        }catch (RuntimeException e) {
                            throw new Font.TypefaceLoadException();
                        }

                        return tf;
                    }
                }
                return null;
            }
        };
    }
}
