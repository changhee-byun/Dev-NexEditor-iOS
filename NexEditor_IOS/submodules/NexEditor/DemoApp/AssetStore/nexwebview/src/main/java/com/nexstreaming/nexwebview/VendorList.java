package com.nexstreaming.nexwebview;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by jeongwook.yoon on 2016-11-16.
 */

public class VendorList {
    static class vendorDefine{
        String app_ucode;
        String market_id;
        String app_name;
        String base_url;
        String webview_url;

        vendorDefine(String market_id, String base_url, String app_ucode, String app_name,String webview_url) {
            this.app_name = app_name;
            this.app_ucode = app_ucode;
            this.base_url = base_url;
            this.market_id = market_id;
            this.webview_url = webview_url;
        }
    }

    private static Map<String,vendorDefine> sVendorAuthMap = new HashMap<>();

    private static void init(){
        if(sVendorAuthMap.size() == 0) {
            sVendorAuthMap.put("LGE", new vendorDefine("LG", "https://test-apis-assetstore.nexstreaming.com", "YWFmMWZjNjJjZWU5NDNhOWM3Mzg4MWVkYTFiNDU0YzlmODM4N2JjZTpMRw", "com.nexstreaming.assetstore.test1","https://test-lg-assetstore.nexstreaming.com"));
            sVendorAuthMap.put("ZTE", new vendorDefine("ZTE", "https://test-apis-assetstore.nexstreaming.com", "NzRjNTVjYzBkM2YyM2IwMTA0ZDU4ZGU1ODE5YmRmMzc5ZDRhZWQ0NDpaVEU=", "com.nexstreaming.app.itemstore.zte","https://test-zte-assetstore.nexstreaming.com"));
            sVendorAuthMap.put("NexStreaming", new vendorDefine("LG", "https://test-apis-assetstore.nexstreaming.com", "YWFmMWZjNjJjZWU5NDNhOWM3Mzg4MWVkYTFiNDU0YzlmODM4N2JjZTpMRw", "com.nexstreaming.assetstore.test1","https://test-lg-assetstore.nexstreaming.com"));
        }
    }

    static vendorDefine getVendorDefine(String vendor){
        init();
        return sVendorAuthMap.get(vendor);
    }

}
