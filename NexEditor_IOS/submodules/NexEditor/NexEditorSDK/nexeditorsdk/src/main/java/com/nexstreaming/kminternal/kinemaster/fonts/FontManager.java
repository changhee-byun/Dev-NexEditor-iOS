package com.nexstreaming.kminternal.kinemaster.fonts;

import android.content.Context;
import android.graphics.Typeface;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.app.common.util.CloseUtil;
//import com.nexstreaming.app.kinemasterfree.R;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class serves two roles:
 * -  It provides an index of built-in and system fonts.
 * -  It provides a single point for turning a font ID (including asset item ID, system font ID, or built-in font ID) into a Typeface
 */
public class FontManager {

    private static final String LOG_TAG = "FontManager";
    private static FontManager instance;

    public static FontManager getInstance() {
        if( instance==null ) {
            instance = new FontManager();
        }
        return instance;
    }

    private static Map<String,Integer> getCollectionIdNameMap() {
        Map<String,Integer> collectionIdNameMap = new HashMap<>();
/*
        collectionIdNameMap.put("latin", R.string.fontcoll_latin);
        collectionIdNameMap.put("hangul", R.string.fontcoll_korean);
        collectionIdNameMap.put("chs", R.string.fontcoll_chs);
        collectionIdNameMap.put("cht", R.string.fontcoll_cht);
        collectionIdNameMap.put("japanese", R.string.fontcoll_jp);
        collectionIdNameMap.put("android", R.string.fontcoll_android);
*/
        collectionIdNameMap.put("latin", 0);
        collectionIdNameMap.put("hangul", 0);
        collectionIdNameMap.put("chs", 0);
        collectionIdNameMap.put("cht",0);
        collectionIdNameMap.put("japanese", 0);
        collectionIdNameMap.put("android", 0);

        return collectionIdNameMap;
    }

    private static class FLibFontCollection implements FontCollection {

        private final String m_id;
        private final int m_labelResourceId;
        private final List<Font> m_fonts = new ArrayList<>();

        FLibFontCollection( String id, int labelResourceId ) {
            m_id = id;
            m_labelResourceId = labelResourceId;
        }

        public String getName( Context ctx ) {
            if(m_labelResourceId != 0) {
                return ctx.getString(m_labelResourceId);
            } else {
                return null;
            }
        }

        public List<Font> getFonts() {
            return Collections.unmodifiableList(m_fonts);
        }

        public String getId() {
            return m_id;
        }

        List<Font> getMutableFontList() {
            return m_fonts;
        }

    }

    public List<FontCollection> getBuiltinFontCollections() {
        if( mBuiltinFontCollectionList ==null ) {
            rebuildFontCollections();
        }
        return mBuiltinFontCollectionList;
    }

    private List<FontCollection> mBuiltinFontCollectionList = null;
    private void rebuildFontCollections() {

        Map<String,Integer> collectionIdNameMap = getCollectionIdNameMap();

        Map<String,FLibFontCollection> fontCollections = new HashMap<>();
        for( Font f: getBuiltinFonts().values() ) {
            String cid = f.getCollectionId();
            FLibFontCollection collection = fontCollections.get(cid);
            if( collection==null ) {
                Integer rsrc = collectionIdNameMap.get(cid);
                if( rsrc==null ) {
                    //rsrc = R.string.fontcoll_other;
                    rsrc = 0;
                }
                collection = new FLibFontCollection(cid, rsrc);
                fontCollections.put(cid, collection);
            }
            collection.getMutableFontList().add(f);
        }

        mBuiltinFontCollectionList = Collections.unmodifiableList(new ArrayList<FontCollection>(fontCollections.values()));
    }

    private Map<String,Font> m_builtinFonts;
    private Map<String,Font> getBuiltinFonts() {
        if( m_builtinFonts==null ) {
            List<Font> builtinFonts = BuiltInFonts.makeBuiltInFontList();
            m_builtinFonts = new HashMap<>();
            for( Font f: builtinFonts ) {
                m_builtinFonts.put(f.getId(),f);
            }
        }
        return m_builtinFonts;
    }

    public void clearBuiltinFonts(){
        m_builtinFonts = null;
    }

    public Font getBuiltinFont(String typeface_id){
        if( typeface_id==null || typeface_id.trim().length()<1)
            return null;

        typeface_id = typeface_id.substring(typeface_id.indexOf('/')+1);

        return getBuiltinFonts().get(typeface_id);
    }

    public boolean isInstalled(String typeface_id) {
        Font builtin = getBuiltinFonts().get(typeface_id);
        if( builtin!=null )
            return true;
        ItemInfo item = AssetPackageManager.getInstance().getInstalledItemById(typeface_id);
        return (item!=null && item.getType()== ItemType.font);
    }

    public Typeface getTypeface(String typeface_id) {

        if( typeface_id==null || typeface_id.trim().length()<1)
            return null;

        typeface_id = typeface_id.substring(typeface_id.indexOf('/')+1);

        Font builtin = getBuiltinFonts().get(typeface_id);
        if( builtin!=null ) {
            try {
                return builtin.getTypeface(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
            } catch (Font.TypefaceLoadException e) {
                return null;
            }
        }

        if( LL.D ) Log.d(LOG_TAG,"Get typeface: " + typeface_id);
        ItemInfo item = AssetPackageManager.getInstance().getInstalledItemById(typeface_id);
        if( item==null || item.getType()!=ItemType.font ) {
            if( LL.W ) Log.w(LOG_TAG,"Typeface not found: " + typeface_id);
            return null;
        } else {
            if( AssetPackageManager.getInstance().checkExpireAsset(item.getAssetPackage()) ){
                if( LL.W ) Log.w(LOG_TAG,"Typeface expire: " + typeface_id);
                return null;
            }

            AssetPackageReader reader;
            try {
                reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),item.getPackageURI(),item.getAssetPackage().getAssetId());
            } catch (IOException e) {
                if( LL.E ) Log.e(LOG_TAG,"Error loading typeface: " + typeface_id,e);
                return null;
            }
            try {
                return reader.getTypeface(item.getFilePath());
            } catch (AssetPackageReader.LocalPathNotAvailableException e) {
                if( LL.E ) Log.e(LOG_TAG,"Error loading typeface: " + typeface_id,e);
                return null;
            } finally {
                CloseUtil.closeSilently(reader);
            }
        }
    }
}
