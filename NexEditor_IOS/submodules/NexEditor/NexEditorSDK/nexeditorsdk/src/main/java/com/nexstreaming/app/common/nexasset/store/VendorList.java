package com.nexstreaming.app.common.nexasset.store;


import java.util.HashMap;
import java.util.Map;

/**
 * Created by jeongwook.yoon on 2016-11-21.
 */

public class VendorList {
    private static final String assetStoreAppGlobalPackageName = "com.nexstreaming.app.vasset.global";

    private static final String assetStoreAppTestPackageName = "com.nexstreaming.assetstore.test1";
    private static final String assetStoreAppServiceName = ".AssetStoreService";
    private static final String KineMasterPackageName = "com.nexstreaming.app.kinemasterfree";

    private Map<String,vendor> m_builtinVendors;
    private static final String LOG_TAG = "VendorList";
    private static VendorList instance;

    public static VendorList getInstance() {
        if( instance==null ) {
            instance = new VendorList();
        }
        return instance;
    }

    public static class vendor{
        private final String assetStoreAppPackageName;
        private final String assetStoreAppServiceName;
        private final String kineMasterPackageName;

        vendor(String assetStoreAppPackageName, String assetStoreAppServiceName, String kineMasterPackageName) {
            this.assetStoreAppPackageName = assetStoreAppPackageName;
            this.assetStoreAppServiceName = assetStoreAppServiceName;
            this.kineMasterPackageName = kineMasterPackageName;
        }

        public String getAssetStoreAppPackageName() {
            return assetStoreAppPackageName;
        }

        public String getAssetStoreAppServiceName() {
            return assetStoreAppServiceName;
        }

        public String getKineMasterPackageName() {
            return kineMasterPackageName;
        }
    }

    static Map<String, vendor> builtinVendor(){
        Map<String, vendor> ret = new HashMap<>();

        ret.put("NexStreaming",new vendor(assetStoreAppGlobalPackageName,assetStoreAppGlobalPackageName+assetStoreAppServiceName,KineMasterPackageName));
        ret.put("LGE",new vendor(assetStoreAppGlobalPackageName,assetStoreAppGlobalPackageName+assetStoreAppServiceName,KineMasterPackageName));
        ret.put("ZTE",new vendor("com.nexstreaming.app.assetstore.zte.china","com.nexstreaming.app.assetstore.zte.china"+assetStoreAppServiceName,KineMasterPackageName));
        ret.put("ZTEGlobal",new vendor("com.nexstreaming.app.assetstore.zte.global","com.nexstreaming.app.assetstore.zte.global"+assetStoreAppServiceName,KineMasterPackageName));
        ret.put("TINNO",new vendor("com.nexstreaming.app.assetstore.tinno.china","com.nexstreaming.app.assetstore.tinno.china"+assetStoreAppServiceName,KineMasterPackageName));
        ret.put("TINNOGlobal",new vendor("com.nexstreaming.app.assetstore.tinno.global","com.nexstreaming.app.assetstore.tinno.global"+assetStoreAppServiceName,KineMasterPackageName));
		ret.put("SEERSTECH",new vendor("com.nexstreaming.app.assetstore.seerstech.global","com.nexstreaming.app.assetstore.seerstech.global"+assetStoreAppServiceName,KineMasterPackageName));
        ret.put("APPChina",new vendor("com.nexstreaming.app.assetstore.sp.china","com.nexstreaming.app.assetstore.sp.china"+assetStoreAppServiceName,KineMasterPackageName));
        ret.put("APPGlobal",new vendor("com.nexstreaming.app.assetstore.sp.global","com.nexstreaming.app.assetstore.sp.global"+assetStoreAppServiceName,KineMasterPackageName));
        ret.put("XIAOMI",new vendor("com.nexstreaming.app.assetstore.xiaomi.china","com.nexstreaming.app.assetstore.xiaomi.china"+assetStoreAppServiceName,KineMasterPackageName));
        ret.put("XIAOMIGlobal",new vendor("com.nexstreaming.app.assetstore.xiaomi.global","com.nexstreaming.app.assetstore.xiaomi.global"+assetStoreAppServiceName,KineMasterPackageName));
        return ret;
    }

    public void init(){
        if( m_builtinVendors == null ){
            m_builtinVendors = builtinVendor();
        }
    }

    public String getAssetStoreAppPackageName(String vendor){
        init();
        vendor v = m_builtinVendors.get(vendor);
        if( v == null ){
            return assetStoreAppGlobalPackageName;
        }
        return v.getAssetStoreAppPackageName();
    }

    public String getAssetStoreAppServiceName(String vendor){
        init();
        vendor v = m_builtinVendors.get(vendor);
        if( v == null ){
            return assetStoreAppGlobalPackageName + assetStoreAppServiceName;
        }
        return v.getAssetStoreAppServiceName();
    }

    public String getKineMasterPackageName(String vendor){
        init();
        vendor v = m_builtinVendors.get(vendor);
        if( v == null ){
            return KineMasterPackageName;
        }
        return v.getKineMasterPackageName();
    }

}
