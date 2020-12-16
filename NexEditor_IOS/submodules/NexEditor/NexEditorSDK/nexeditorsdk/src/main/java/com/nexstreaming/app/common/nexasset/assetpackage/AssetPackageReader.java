package com.nexstreaming.app.common.nexasset.assetpackage;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Typeface;
import android.util.Log;

import com.google.gson_nex.Gson;
import com.google.gson_nex.JsonSyntaxException;
import com.nexstreaming.app.common.nexasset.assetpackage.security.PackageSecurityManager;
import com.nexstreaming.app.common.nexasset.assetpackage.security.SecurityProvider;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.app.common.util.PathUtil;
import com.nexstreaming.kminternal.kinemaster.config.LL;

import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;


public class AssetPackageReader implements Closeable {

    private static final String LOG_TAG = "AssetPackageReader";
    private static final String MERGE_PREFIX = "merge";
    private ContainerReader containerReader;
    private final Gson gson = new Gson();
    private static final String PACKAGE_INFO_FILE = "packageinfo.json";
    private static final String ENCRYPTION_INFO_FILE = "e.json";
    private static final String PACKAGE_FORMAT = "com.kinemaster.assetpackage";
    private static final int PACKAGE_FORMAT_VERSION = 6;
    private final PackageInfoJSON packageInfo;
    private final EncryptionInfoJSON encryptionInfo;
    private final String baseId;
    private final boolean sharedInstance;
    private List<ItemInfo> itemList = null;
    private static Map<String,WeakReference<AssetPackageReader>> readerCache = new HashMap<>();
    private static Map<String,SecurityProvider> securityProviderMap = new HashMap<>();
    private final PackageSecurityManager securityManager;

    private static class APRWeakRef extends WeakReference<AssetPackageReader> {
        private static ReferenceQueue<AssetPackageReader> collectedReaders = new ReferenceQueue<>();
        private ContainerReader containerReader;
        public APRWeakRef(AssetPackageReader r) {
            super(r,collectedReaders);
            containerReader = r.getContainerReader();
        }
        private static void gc() {
            APRWeakRef ref;
            while((ref = (APRWeakRef) collectedReaders.poll())!=null) {
                if(ref.containerReader!=null) {
                    try {
                        ref.containerReader.close();
                        if( LL.D ) Log.d(LOG_TAG,"Closed cached container reader");
                    } catch (IOException e) {
                        // Ignored intentionally
                        if( LL.D ) Log.d(LOG_TAG,"Error closing container reader",e);
                    }
                    ref.containerReader = null;

                }
            }
        }
    }

    public static String[] getRegisteredEncInfo() {
        List<String> l=new ArrayList<>();

        for ( Map.Entry<String,SecurityProvider> e : securityProviderMap.entrySet() ) {
            String[] info = e.getValue().getEncInfo();
            for ( String s : info ) {
                l.add(s);
            }
        }

        String[] ar = new String[l.size()];
        int arsize=0;
        for ( String s : l) {
            ar[arsize++] = s;
        }

        return ar;
    }
    
    public static void registerSecurityProvider(SecurityProvider securityProvider) {
        String id = securityProvider.getProviderId();
        if( id==null || id.length()<1 ) throw new IllegalArgumentException("id is null or empty");
        if( securityProviderMap.get(id)!=null ) {
            //throw new IllegalStateException("id already in use :"+securityProviderMap.get(id));
            Log.d(LOG_TAG, "This provider ID is already registered! "+id);
            return;
        }
        if( securityProviderMap.values().contains(securityProvider) ) {
            //throw new IllegalStateException("provider already registered");
            Log.d(LOG_TAG, "This provider is already registered! ");
            return;
        }
        securityProviderMap.put(id,securityProvider);
        //if( LL.D ) Log.d(LOG_TAG,"registerSecurityProvider(), New provider ID is "+id);
    }

    public static void unregisterSecurityProvider(SecurityProvider securityProvider) {
        if( securityProvider==null ) return;
        if( securityProviderMap.get(securityProvider.getProviderId()) != securityProvider ) throw new IllegalStateException();
        securityProviderMap.remove(securityProvider.getProviderId());
    }

    /**
     * Factory function to make an asset package reader for a zip file.
     * @param path          Path to the .zip file
     * @param baseId        The base ID to use for items in this package that don't have an ID
     *                      explicitly given in the package.  This is generally the asset package
     *                      ID itself.  This is used to generate item IDs as follows:
     *                              baseId + ".items." + itemName
     * @return              An asset package reader for the zip file.
     * @throws IOException
     */
    public static AssetPackageReader readerForZipPackage(File path, String baseId) throws IOException {
        return new AssetPackageReader(new ZipContainerReader(path),baseId,false);
    }

    /**
     * Factory function to make an asset package reader for a folder.
     * @param path          Path to the folder.
     * @param baseId        The base ID to use for items in this package that don't have an ID
     *                      explicitly given in the package.  This is generally the asset package
     *                      ID itself.  This is used to generate item IDs as follows:
     *                              baseId + ".items." + itemName
     * @return              An asset package reader for the zip folder.
     * @throws IOException
     */
    public static AssetPackageReader readerForFolder(File path, String baseId) throws IOException {
        return new AssetPackageReader(new FolderContainerReader(path),baseId,false);
    }

    public static AssetPackageReader readerForAndroidAppAssets(AssetManager assetManager, String assetPath, String baseId) throws IOException {
        return new AssetPackageReader(new AssetContainerReader(assetManager, assetPath),baseId,false);
    }

    public static AssetPackageReader readerForPackageURI(Context context, String packageURI, String baseId) throws IOException {
        APRWeakRef.gc();
        AssetPackageReader reader;
        WeakReference<AssetPackageReader> readerRef = readerCache.get(packageURI);
        if( readerRef!=null ) {
            reader = readerRef.get();
            if( reader!=null )
                return reader;
        }
        String subURI = packageURI.substring(packageURI.indexOf(':')+1);
        if( packageURI.startsWith("assets:")) {
            reader = new AssetPackageReader(new AssetContainerReader(context.getApplicationContext().getAssets(), subURI),baseId,true);
        } else if( packageURI.startsWith("file:")) {
            reader = new AssetPackageReader(new FolderContainerReader(new File(subURI)),baseId,true);
        } else if( packageURI.startsWith("zipfile:")) {
            reader = new AssetPackageReader(new ZipContainerReader(new File(subURI)),baseId,true);
        } else {
            throw new PackageReaderException();
        }
        readerCache.put(packageURI,new APRWeakRef(reader));
        return reader;
    }

    private AssetPackageReader(ContainerReader containerReader, String baseId, boolean sharedInstance) throws IOException {
        if( LL.D ) Log.d(LOG_TAG,"NEW APR Instance (Container:" + containerReader.getClass().getSimpleName() + ") baseId=" + baseId + " shared=" + sharedInstance);
        this.containerReader = containerReader;
        this.baseId = baseId;
        this.sharedInstance = sharedInstance;
        encryptionInfo = readEncryptionInfo();
        securityManager = prepareSecurityManager(encryptionInfo);
        if( securityManager!=null ) {
            this.containerReader = new EncryptedContainerReader(containerReader,securityManager);
        }
        packageInfo = readPackageInfo();
    }

    public int getPackageFormatVersion() {
        return packageInfo.packageContentVersion;
    }

    public int getTargetVersion() {
        return packageInfo.targetVersionCode;
    }

    public int getMinVersion(){
        return packageInfo.minVersionCode;
    }

    public List<ItemInfo> getItems() throws IOException {
        makeItemList();
        return itemList;
    }

    private EncryptionInfoJSON readEncryptionInfo() throws IOException {
        EncryptionInfoJSON encryptionInfo;
        InputStream in = null;
        try {
            in = containerReader.openFile(ENCRYPTION_INFO_FILE);
            encryptionInfo = gson.fromJson(new InputStreamReader(in), EncryptionInfoJSON.class);
			if( LL.D ) Log.d(LOG_TAG,"Parse e.json file! : "+encryptionInfo.provider.toString() + " / " + encryptionInfo.psd.toString());
        } catch (FileNotFoundException e) {
            encryptionInfo = null;
        } finally {
            CloseUtil.closeSilently(in);
        }
        return encryptionInfo;
    }

    private PackageSecurityManager prepareSecurityManager(EncryptionInfoJSON encryptionInfo ) throws PackageReaderException {
        if( encryptionInfo!=null && encryptionInfo.provider!=null && encryptionInfo.provider.length()>0 ) {
            SecurityProvider provider = securityProviderMap.get(encryptionInfo.provider);
            if( provider==null ) throw new PackageReaderException(this,"invalid provider");
            return provider.getSecurityManagerForPackage(encryptionInfo.psd);
        } else {
            return null;
        }
    }

    private PackageInfoJSON readPackageInfo() throws IOException {
        if( LL.D ) Log.d(LOG_TAG,"readPackageInfo IN");
        PackageInfoJSON packageInfo;
        try {
            InputStream is = containerReader.openFile(PACKAGE_INFO_FILE);
            try {
                packageInfo = gson.fromJson(new InputStreamReader(is), PackageInfoJSON.class);
                if( LL.D ) Log.d(LOG_TAG,"readPackageInfo(), asset name: "+ packageInfo.assetName);

                // 15 Nov 2016, mark.lee, Changes the key to lowercase.
                if (packageInfo.assetName != null && packageInfo.assetName.size() > 0) {
                    Set<String> keySet = packageInfo.assetName.keySet();
                    Map<String, String> newAssetName = new HashMap<>();
                    for (String key : keySet) {
                        newAssetName.put(key.toLowerCase(Locale.ENGLISH), packageInfo.assetName.get(key));
                    }
                    packageInfo.assetName.clear();
                    packageInfo.assetName.putAll(newAssetName);
                }
            } finally {
                is.close();
            }
        } catch (FileNotFoundException e) {
            if( LL.W ) Log.w(LOG_TAG,"Package missing file: " + PACKAGE_INFO_FILE,e);
            throw new PackageReaderException(this, "Package missing file: " + PACKAGE_INFO_FILE, e);
        } catch(JsonSyntaxException e){
            if(LL.W) Log.w(LOG_TAG, "PackageInfoJSON file : " + PACKAGE_INFO_FILE, e);
            throw new PackageReaderException(this, "PackageInfoJSON file: " + PACKAGE_INFO_FILE, e);
        }
        if( packageInfo.minVersionCode > PACKAGE_FORMAT_VERSION ) {
            if( LL.W ) Log.w(LOG_TAG,"Unsupported package format version: " + packageInfo.minVersionCode);
            throw new PackageReaderException(this, "Unsupported package format version");
        }
        if( packageInfo.format==null ) {
            if( LL.W ) Log.w(LOG_TAG,"Missing package format");
            throw new PackageReaderException(this, "Missing package format");
        } else if( packageInfo.format.equals(PACKAGE_FORMAT)) {
            if( LL.D ) Log.d(LOG_TAG,"readPackageInfo OUT");
            // Nothing speical to read for this format
            return packageInfo;
        } else {
            if( LL.W ) Log.w(LOG_TAG,"Unsupported package format: " + packageInfo.format );
            throw new PackageReaderException(this, "Unsupported package format: " + packageInfo.format );
        }
    }

    public Map<String,String> getAssetName() {
        return packageInfo.assetName;
    }

    private void makeItemList() throws IOException {
        if( itemList!=null )
            return;

        List<ItemInfo> itemArrayList = new ArrayList<>();
        int idx = 0;

        if( packageInfo.itemRoots !=null && packageInfo.itemRoots.size()>0 ) {
            if (LL.D) Log.d(LOG_TAG, "makeItemList: using root index");
            for (String root : packageInfo.itemRoots) {
                if( root == null )
                    continue;

                idx++;
                String path = combinePaths(root,"_info.json");
                //if (LL.D) Log.d(LOG_TAG, "makeItemList[" + idx + "]:" + path);
                ItemInfo info = processFile(path);
                if (info != null) {
                    itemArrayList.add(info);
                }
            }
        } else {
            if (LL.D) Log.d(LOG_TAG, "makeItemList: no root index; scanning entire package");
            for (String path : containerReader.listFiles()) {
                if( path == null )
                    continue;

                idx++;
                //if (LL.D) Log.d(LOG_TAG, "makeItemList[" + idx + "]:" + path);
                ItemInfo info = processFile(path);
                if (info != null) {
                    itemArrayList.add(info);
                }
            }
        }
        itemList = itemArrayList;
    }

    private ItemInfo processFile(String path) throws IOException {

        if( !path.endsWith("/_info.json") ) {
            return null;
        }

        //if( LL.D ) Log.d(LOG_TAG,"processFile:" + path);

        int firstSep = path.indexOf('/');
        int secondSep = path.indexOf('/', firstSep + 1);
        int thirdSep = path.indexOf('/', secondSep + 1);
        if( firstSep==-1 || secondSep==-1 || thirdSep!=-1 ) {
            if(LL.W) Log.w(LOG_TAG,"Malformed path");
            return null;
        }

        if( path.startsWith( MERGE_PREFIX + "/"))  // Ignore merge items
            return null;

        ItemCategory itemCategory = null;
        for( ItemCategory e: ItemCategory.values() ) {
            String ename = e.name();
            if( ename.length()==firstSep && path.startsWith(ename) ) {
                itemCategory = e;
                break;
            }
        }

        if( itemCategory==null ) {
            if(LL.W) Log.w(LOG_TAG,"Unrecognized item category");
            return null;
        }

        String itemName = path.substring(firstSep + 1, secondSep);
        String itemRoot = path.substring(0,secondSep+1);

        ItemInfoJSON itemInfoJSON;
        try {
            InputStream is = containerReader.openFile(path);
            try {
                itemInfoJSON = gson.fromJson(new InputStreamReader(is), ItemInfoJSON.class);

                // 15 Nov 2016, mark.lee, Changes the key to lowercase.
                if (itemInfoJSON != null && itemInfoJSON.label != null && itemInfoJSON.label.size() > 0) {
                    Set<String> keySet = itemInfoJSON.label.keySet();
                    Map<String, String> newLabel = new HashMap<>();
                    for (String key : keySet) {
                        newLabel.put(key.toLowerCase(Locale.ENGLISH), itemInfoJSON.label.get(key));
                    }
                    itemInfoJSON.label.clear();
                    itemInfoJSON.label.putAll(newLabel);
                }
            } finally {
                is.close();
            }
        } catch (JsonSyntaxException e) {
            throw new PackageReaderException(this, "JSON Syntax Error in: " + path,e);
        } catch (FileNotFoundException e) {
            //MATTFIX: Need to check why this is happening; these should work correctly (!)
            if(LL.W) Log.w(LOG_TAG,"Item in index but missing in package",e);
            return null;
        }

        if( itemInfoJSON.filename==null ) {
            throw new PackageReaderException(this, "Missing base file for: " + path);
        }

        if( itemInfoJSON.icon==null ) {
            itemInfoJSON.icon = "_icon.svg";
        }

        if( itemInfoJSON.thumbnail==null ) {
            itemInfoJSON.thumbnail = "_thumb.jpeg";
        }

        if( itemInfoJSON.id==null ) {
            itemInfoJSON.id = baseId + ".items." + itemName;
        }

        ItemType itemType = ItemType.fromId(itemInfoJSON.type);
        if( itemType==null ) {
            throw new PackageReaderException(this, "Unrecognized item type '" + itemInfoJSON.type + "' for: " + path);
        }

        ItemInfoWrapper itemInfo = new ItemInfoWrapper();
        itemInfo.readerClass = containerReader.getClass();
        itemInfo.packageURI = containerReader.getPackageURI();
        itemInfo.filePath = combinePaths(itemRoot, itemInfoJSON.filename);
        itemInfo.iconPath = combinePaths(itemRoot, itemInfoJSON.icon);
        itemInfo.thumbPath = combinePaths(itemRoot, itemInfoJSON.thumbnail);
        itemInfo.id = itemInfoJSON.id;
        itemInfo.label = itemInfoJSON.label;
        itemInfo.itemType = itemType;
        itemInfo.itemCategory = itemCategory;
        itemInfo.sampleText = itemInfoJSON.sampleText;
        itemInfo.hidden = itemInfoJSON.hidden;

        //if( LL.D ) Log.d(LOG_TAG,"processFileOUT:" + path + " OUT -- id=" + String.valueOf(itemInfo.id) + " cat=" + itemCategory+", hidden="+itemInfo.hidden);

        return itemInfo;
    }

    private static String combinePaths( String a, String b ) {
        if( b.startsWith("..") || b.contains( "/..") )
            throw new SecurityException("Parent Path References Not Allowed");
        if(a.endsWith("/")) {
            return a + b;
        } else {
            return a + "/" + b;
        }
    }

    private Map<String,String> mergeMap = null;
    public InputStream openFile(String path) throws FileNotFoundException, IOException {

        if( mergeMap!=null ) {
            String subst = mergeMap.get(path);
            if( subst!=null && subst.length()>0 )
                return containerReader.openFile(subst);
        }
        try {
            return containerReader.openFile(path);
        } catch (FileNotFoundException e) {
            // Find _info.json (check this folder and all parent folders in container recursively)
            String basePath = PathUtil.getBasePath(path);
            ItemInfoJSON itemInfo = null;
            while(basePath!=null) {
                String infoPath = PathUtil.relativePath(basePath, "_info.json");
                InputStream infoIn = null;
                try {
                    infoIn = containerReader.openFile(infoPath);
                    itemInfo = gson.fromJson(new InputStreamReader(infoIn),ItemInfoJSON.class);
                    if( itemInfo!=null )
                        break;
                } catch (FileNotFoundException ignored) {
                } finally {
                    CloseUtil.closeSilently(infoIn);
                }
                basePath = PathUtil.getParent(basePath);
            }
            // Check if _info.json was found...
            if( itemInfo!=null && itemInfo.mergePaths!=null ) {
                // If it was found, check each of the merge paths
                String relPath = path.substring(basePath.length(),path.length());
                for( String mergePath: itemInfo.mergePaths ) {
                    if(mergePath!=null) {
                        mergePath = PathUtil.combinePaths(MERGE_PREFIX,mergePath.trim());

                        if( !mergePath.endsWith("/")){
                            mergePath += "/";
                        }

                        if( mergePath.length()>0 ) {
                            String resolvedPath = PathUtil.relativePath(mergePath,relPath);

                            try {
                                InputStream in = containerReader.openFile(resolvedPath);
                                // If the above line didn't throw an exception, it means we found
                                // the file.  We should cache this, because we don't want to search
                                // every time (the search process involves exceptions, which are
                                // heavy).
                                if( mergeMap==null ) mergeMap = new HashMap<>();
                                mergeMap.put(path,resolvedPath);
                                return in;
                            } catch (FileNotFoundException ignored) {
                                // The file wasn't foudn at this location, so we just keep
                                // searching.
                            }
                        }
                    }
                }
            }
            // We couldn't find a substitute file from any merged folder, so just
            // throw the original file-not-found exception normally.
            throw e;
        }
    }

    private Map<String,String> mergeMapForPath = null;
    public String getFilePath(String path) {

        if( mergeMapForPath!=null ) {
            String subst = mergeMapForPath.get(path);
            if( subst!=null && subst.length()>0 )
                return subst;
        }

        String basePath = PathUtil.getBasePath(path);
        ItemInfoJSON itemInfo = null;
        while(basePath!=null) {
            String infoPath = PathUtil.relativePath(basePath, "_info.json");
            InputStream infoIn = null;
            try {
                infoIn = containerReader.openFile(infoPath);
                itemInfo = gson.fromJson(new InputStreamReader(infoIn),ItemInfoJSON.class);
                if( itemInfo!=null )
                    break;
            } catch (FileNotFoundException ignored) {
            } catch (IOException ignored) {
            } finally {
                CloseUtil.closeSilently(infoIn);
            }
            basePath = PathUtil.getParent(basePath);
        }

        // Check if _info.json was found...
        if( itemInfo!=null && itemInfo.mergePaths!=null ) {
            // If it was found, check each of the merge paths
            String relPath = path.substring(basePath.length(), path.length());
            for (String mergePath : itemInfo.mergePaths) {
                if (mergePath != null) {
                    mergePath = PathUtil.combinePaths(MERGE_PREFIX, mergePath.trim());

                    if (!mergePath.endsWith("/")) {
                        mergePath += "/";
                    }

                    if (mergePath.length() > 0) {
                        String resolvedPath = PathUtil.relativePath(mergePath, relPath);
                        if (mergeMapForPath == null) mergeMapForPath = new HashMap<>();
                        if( LL.D ) Log.d(LOG_TAG, "getFilePath mergePath : " + resolvedPath);
                        mergeMapForPath.put(path, resolvedPath);
                        return resolvedPath;
                    }
                }
            }
        }
        return null;
    }

    private InputStream openMerged(String path) {
        String infoPath = PathUtil.relativePath(path,"_info.json");
        try {
            containerReader.openFile(path);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    public Iterable<String> listFiles() {
        return containerReader.listFiles();
    }

    public void close() throws IOException {
        if( sharedInstance )
            return;
        containerReader.close();
    }

//    public String getPackagePath() {
//        return null;
//    }

    public String getPackageURI() {
        return containerReader.getPackageURI();
    }

    public File getAssetPackageLocalPath() {
        return containerReader.getAssetPackageLocalPath();
    }

    private static class EncryptionInfoJSON {
        public String provider; //provider
        public String psd;          // Provider-Specific Data
    }

    private static class PackageInfoJSON {
        public String format;
        public int targetVersionCode;
        public int minVersionCode;
        public int packageContentVersion;
        public Map<String,String> assetName;
        public List<String> itemRoots;
    }

    private static class ItemInfoJSON {
        public String type;
        public Map<String,String> label;
        public String filename;
        public String thumbnail;
        public String icon;
        public String id;
        public String sampleText;
        public boolean hidden;
        public List<String> mergePaths;
    }

    private static class ItemInfoWrapper implements ItemInfo {
        Class<? extends ContainerReader> readerClass;
        String packageURI;
        String filePath;
        String iconPath;
        String thumbPath;
        String id;
        String sampleText;
        Map<String,String> label;
        ItemType itemType;
        ItemCategory itemCategory;
        boolean hidden;

        @Override
        public String getId() {
            return id;
        }

        @Override
        public String getPackageURI() {
            return packageURI;
        }

        @Override
        public String getFilePath() {
            return filePath;
        }

        @Override
        public String getIconPath() {
            return iconPath;
        }

        @Override
        public String getThumbPath() {
            return thumbPath;
        }

        @Override
        public Map<String, String> getLabel() {
            return label;
        }

        @Override
        public String getSampleText() {
            return sampleText;
        }

        @Override
        public ItemType getType() {
            return itemType;
        }

        @Override
        public ItemCategory getCategory() {
            return itemCategory;
        }

        @Override
        public AssetInfo getAssetPackage() {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean isHidden() {
            return hidden;
        }
    }

    interface ContainerReader {
        InputStream openFile(String path) throws FileNotFoundException, IOException;
        Iterable<String> listFiles();
        void close() throws IOException;
        String getPackageURI();
        File getLocalPath(String path) throws LocalPathNotAvailableException,EncryptedException, FileNotFoundException, IOException;
        Typeface getTypeface(String path) throws LocalPathNotAvailableException;
        File getAssetPackageLocalPath();
    }

    public static class EncryptedException extends IOException {
        public EncryptedException() {
        }

        public EncryptedException(String detailMessage) {
            super(detailMessage);
        }

        public EncryptedException(String message, Throwable cause) {
            super(message, cause);
        }

        public EncryptedException(Throwable cause) {
            super(cause);
        }
    }


    public static class LocalPathNotAvailableException extends IOException {
        public LocalPathNotAvailableException() {
        }

        public LocalPathNotAvailableException(String detailMessage) {
            super(detailMessage);
        }

        public LocalPathNotAvailableException(String message, Throwable cause) {
            super(message, cause);
        }

        public LocalPathNotAvailableException(Throwable cause) {
            super(cause);
        }
    }

    private static class ZipContainerReader implements ContainerReader {

        private final ZipFile zipFile;
        private final File zipPath;

        public ZipContainerReader(File path) throws IOException {
            zipFile = new ZipFile(path);
            zipPath = path;
        }

        @Override
        public InputStream openFile(String path) throws FileNotFoundException, IOException {
            ZipEntry entry = zipFile.getEntry(path);
            if( entry==null ) {
                entry = zipFile.getEntry(PathUtil.combinePaths(PathUtil.getBasePath(path),PathUtil.getName(path).toLowerCase(Locale.ENGLISH)));
                if( entry==null ) {
                    throw new FileNotFoundException("File '" + path + "' not found in '" + zipFile.getName() + "'");
                }
            }
            return zipFile.getInputStream(entry);
        }

//        @Override
//        public boolean fileExists(String path) {
//            ZipEntry entry = zipFile.getEntry(path);
//            return ( entry!=null );
//        }
//
        @Override
        public Iterable<String> listFiles() {
            return new Iterable<String>() {
                @Override
                public Iterator<String> iterator() {
                    final Enumeration<? extends ZipEntry> entries = zipFile.entries();
                    return new Iterator<String>() {
                        @Override
                        public boolean hasNext() {
                            return entries.hasMoreElements();
                        }

                        @Override
                        public String next() {
                            return entries.nextElement().getName();
                        }

                        @Override
                        public void remove() {
                            throw new UnsupportedOperationException();
                        }
                    };
                }
            };
        }

        @Override
        public void close() throws IOException {
            zipFile.close();
        }

        @Override
        public String getPackageURI() {
            return "zipfile:" + zipFile.getName();
        }

        @Override
        public File getLocalPath(String path) throws LocalPathNotAvailableException, FileNotFoundException, IOException {
            throw new LocalPathNotAvailableException();
        }

        @Override
        public Typeface getTypeface(String path) throws LocalPathNotAvailableException {
            throw new LocalPathNotAvailableException();
        }

        @Override
        public File getAssetPackageLocalPath() {
            return zipPath;
        }

    }

    public File getLocalPath(String path) throws IOException, LocalPathNotAvailableException {
        return containerReader.getLocalPath(path);
    }

    public Typeface getTypeface(String path) throws LocalPathNotAvailableException {
        return containerReader.getTypeface(path);
    }

    private static class FolderContainerReader implements ContainerReader {

        private final File folderPath;

        private FolderContainerReader(File folderPath) {
            this.folderPath = folderPath;
        }

        @Override
        public InputStream openFile(String path) throws FileNotFoundException, IOException {
            //Log.d(LOG_TAG,"FolderContainerReader::openFile="+path);
            return new FileInputStream(getAbsolutePath(path));
        }

        @Override
        public File getLocalPath(String path) throws LocalPathNotAvailableException, FileNotFoundException, IOException {
            return getAbsolutePath(path);
        }

        @Override
        public Typeface getTypeface(String path) throws LocalPathNotAvailableException {
            return Typeface.createFromFile(getAbsolutePath(path));
        }

        @Override
        public File getAssetPackageLocalPath() {
            return folderPath;
        }

        //        @Override
//        public boolean fileExists(String path) {
//            return getAbsolutePath(path).exists();
//        }
//
        private String getContainerRelativePath(File absPath) {
            String full = absPath.getAbsolutePath();
            String base = folderPath.getAbsolutePath();
            if( !full.startsWith(base) ) {
                throw new IllegalStateException();
            }
            if( full.length()<=base.length() ) {
                return "";
            }
            if( full.charAt(base.length())=='/' ) {
                return full.substring(base.length()+1);
            } else {
                return full.substring(base.length());
            }
        }

        private File getAbsolutePath(String containerRelativePath) {
            return new File(folderPath, containerRelativePath);
        }

        @Override
        public Iterable<String> listFiles() {
            return new Iterable<String>() {
                @Override
                public Iterator<String> iterator() {

                    final List<File> toDoList = new ArrayList<>();
                    File[] files = folderPath.listFiles();
                    if( files!=null ) {
                        for (File f:files) {
                            toDoList.add(f);
                        }
                    }

                    return new Iterator<String>() {
                        @Override
                        public boolean hasNext() {
                            return !toDoList.isEmpty();
                        }

                        @Override
                        public String next() {
                            File nextfile = toDoList.remove(0);
                            if( nextfile.isDirectory() ) {
                                File[] files = nextfile.listFiles();
                                if( files!=null ) {
                                    for (File f:files) {
                                        toDoList.add(f);
                                    }
                                }
                            }
                            return getContainerRelativePath(nextfile);
                        }

                        @Override
                        public void remove() {

                        }
                    };
                }
            };
        }

        @Override
        public void close() throws IOException {
            // Nothing to do
        }

        @Override
        public String getPackageURI() {
            return "file:" + folderPath.getAbsolutePath();
        }
    }

    private static class EncryptedContainerReader implements ContainerReader {

        private final ContainerReader wrappedReader;
        private final PackageSecurityManager packageSecurityManager;

        private EncryptedContainerReader(ContainerReader wrappedReader, PackageSecurityManager packageSecurityManager) {
            this.wrappedReader = wrappedReader;
            this.packageSecurityManager = packageSecurityManager;
        }

        @Override
        public InputStream openFile(String path) throws FileNotFoundException, IOException {
            return packageSecurityManager.wrapInputStream(wrappedReader.openFile(path),path);
        }

        @Override
        public Iterable<String> listFiles() {
            return wrappedReader.listFiles();
        }

        @Override
        public void close() throws IOException {
            wrappedReader.close();
        }

        @Override
        public String getPackageURI() {
            return wrappedReader.getPackageURI();
        }

        @Override
        public File getLocalPath(String path) throws LocalPathNotAvailableException,EncryptedException, FileNotFoundException, IOException {
            if( !packageSecurityManager.plaintextAvailable(path)) {
                throw new EncryptedException();
            }
            return wrappedReader.getLocalPath(path);
        }

        @Override
        public Typeface getTypeface(String path) throws LocalPathNotAvailableException {
            if( !packageSecurityManager.plaintextAvailable(path)) {
                throw new LocalPathNotAvailableException();
            }
            return wrappedReader.getTypeface(path);
        }

        @Override
        public File getAssetPackageLocalPath() {
            return wrappedReader.getAssetPackageLocalPath();
        }
    }

    private static class AssetContainerReader implements ContainerReader {

        private final String assetPath;
        private AssetManager assetManager;

        private AssetContainerReader(AssetManager assetManager, String assetPath) {
            if( assetManager==null )
                throw new IllegalArgumentException();
            if( assetPath==null )
                throw new IllegalArgumentException();
            this.assetPath = assetPath;
            this.assetManager = assetManager;
            if( LL.D ) Log.d(LOG_TAG,"Created ACR:" + String.valueOf(this));
        }

        @Override
        public InputStream openFile(String path) throws FileNotFoundException, IOException {
            if( LL.D ) Log.d(LOG_TAG,"openFile:" + String.valueOf(this));
            return assetManager.open(combinePaths(assetPath,path));
        }

//        @Override
//        public boolean fileExists(String path) {
//            if( LL.W ) Log.w(LOG_TAG,"WARNING: Not recommended to check file existence this way; performance may suffer.");
//            try {
//                InputStream in = openFile(path);
//                in.close();
//            } catch (IOException e) {
//                return false;
//            }
//            return true;
//        }

        @Override
        public Iterable<String> listFiles() {
            return new Iterable<String>() {
                @Override
                public Iterator<String> iterator() {

                    final List<String> toDoList = new ArrayList<>();
                    String[] files = null;
                    try {
                        files = assetManager.list(assetPath);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    if( files!=null ) {
                        for (String f:files) {
                            toDoList.add(combinePaths(assetPath,f));
                        }
                    }

                    return new Iterator<String>() {
                        @Override
                        public boolean hasNext() {
                            return !toDoList.isEmpty();
                        }

                        @Override
                        public String next() {
                            String nextfile = toDoList.remove(0);
                            if( LL.D ) Log.d(LOG_TAG,"iter:next -> " + nextfile + " (todo list size: " + toDoList.size() + ")");
                            String children[] = null;
                            try {
                                children = assetManager.list(nextfile);
                                if( LL.D ) Log.d(LOG_TAG,"Has " + children.length + " children.");
                            } catch (IOException e) {
                                // NO-OP; children will remain null in this case
                                if( LL.D ) Log.d(LOG_TAG,"Has no children.");
                            }
                            if( children!=null && children.length>0 ) {
                                for (String f:children) {
                                    toDoList.add(combinePaths(nextfile,f));
                                }
                                if( LL.D ) Log.d(LOG_TAG,"Added " + children.length + " children; todo list size: " + toDoList.size());
                            }
                            int apl = assetPath.length();
                            return nextfile.substring(apl>0?apl+1:0);
                        }

                        @Override
                        public void remove() {

                        }
                    };
                }
            };
        }

        @Override
        public void close() throws IOException {
            assetManager = null;
        }

        @Override
        public String getPackageURI() {
            return "assets:" + assetPath;
        }

        @Override
        public File getLocalPath(String path) throws LocalPathNotAvailableException, FileNotFoundException, IOException {
            throw new LocalPathNotAvailableException();
        }

        @Override
        public Typeface getTypeface(String path) throws LocalPathNotAvailableException {
            return Typeface.createFromAsset(assetManager,combinePaths(assetPath,path));
        }

        @Override
        public File getAssetPackageLocalPath() {
            return null;
        }

    }

    public static class PackageReaderException extends IOException {

        PackageReaderException() {
        }

        PackageReaderException(AssetPackageReader assetPackageReader, String message) {
            super(message + " (in package '" + assetPackageReader.getPackageURI() + "' via " + assetPackageReader.getClass().getSimpleName() + ")");
        }

        PackageReaderException(AssetPackageReader assetPackageReader, String message, Throwable e) {
            super(message + " (in package '" + assetPackageReader.getPackageURI() + "' via " + assetPackageReader.getClass().getSimpleName() + ")", e);
        }

    }

    private ContainerReader getContainerReader() {
        return containerReader;
    }
}
