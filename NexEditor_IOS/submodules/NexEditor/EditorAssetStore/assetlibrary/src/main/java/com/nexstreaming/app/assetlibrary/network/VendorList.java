package com.nexstreaming.app.assetlibrary.network;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by jeongwook.yoon on 2016-11-16.
 */

public class VendorList {

    private static Map<String,Vendor> sVendorAuthMap = new HashMap<>();

    public static class Vendor {
        public final String market_id;
        public final String base_url;
        public final String app_ucode;
        public final String app_name;

        Vendor(String market_id, String base_url, String app_ucode, String app_name) {
            this.market_id = market_id;
            this.base_url = base_url;
            this.app_ucode = app_ucode;
            this.app_name = app_name;
        }
    }

    private static void init(){
        if(sVendorAuthMap.size() == 0) {
            sVendorAuthMap.put("LGE", new Vendor("LG", "https://test-apis-assetstore.nexstreaming.com", "YWFmMWZjNjJjZWU5NDNhOWM3Mzg4MWVkYTFiNDU0YzlmODM4N2JjZTpMRw", "com.nexstreaming.assetstore.test1"));
            sVendorAuthMap.put("ZTE", new Vendor("ZTE", "https://test-apis-assetstore.nexstreaming.com", "NzRjNTVjYzBkM2YyM2IwMTA0ZDU4ZGU1ODE5YmRmMzc5ZDRhZWQ0NDpaVEU=", "com.nexstreaming.app.itemstore.zte"));
            sVendorAuthMap.put("NexStreaming", new Vendor("LG", "https://test-apis-assetstore.nexstreaming.com", "YWFmMWZjNjJjZWU5NDNhOWM3Mzg4MWVkYTFiNDU0YzlmODM4N2JjZTpMRw", "com.nexstreaming.assetstore.test1"));
        }
    }

    public static Vendor getVendorDefine(String vendor){
        if(vendor == null){
            throw new IllegalStateException("vendor must be not null");
        }
        init();
        Vendor vd = sVendorAuthMap.get(vendor);
        if(vd == null){
            throw new IllegalStateException(vendor + " is invalid vendor");
        }
        return vd;
    }

}
