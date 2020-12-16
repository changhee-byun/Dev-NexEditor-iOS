package com.nexstreaming.app.assetlibrary.network.assetstore.json;


import java.util.List;

public class AssetStoreAPIData {

    private AssetStoreAPIData(){} // Prevent instantiation

    public static final int ASSET_ENV_DRAFT = 1;
    public static final int ASSET_ENV_STAGING = 2;
    public static final int ASSET_ENV_PRODUCTION = 3;

    /** AUTHORIZE
     *
     *   https://test-api-kinemaster-assetstore.nexstreaming.com/v2/oauth/authorize
     *
     *  Receive AuthorizeOrRefreshResponse
     */
    public static class AuthorizeRequest {
        public String app_uuid;
        public String app_name;
        public String app_version;
        public String app_ucode;                // KineMaster:  NDBhNDU1NGJjYjU5MzRlZThmOTU4YmYxMmYzZTU3MmZmOTM3NTZjZTpHb29nbGU
        public String client_id;
        public String application;
        public String market_id;
        public int scope;
        public String client_info;
        public String edition;
    }


    /** REFRESH AUTH
     *
     *   https://test-api-kinemaster-assetstore.nexstreaming.com/v2/oauth/refreshauth
     *
     *  Send RefreshOrDeleteAuthRequest
     *  Receive AuthorizeOrRefreshResponse
     */

    /** DELETE AUTH
     *
     *   https://test-api-kinemaster-assetstore.nexstreaming.com/v2/oauth/delauth
     *
     *  Send RefreshOrDeleteAuthRequest
     *  Receive StatusResponse
     */

    // No specific types

    /** STANDARD REQUEST
     *  This structure is used with most requests if a specific request structure is not given
     */
    public static class StandardRequest {
        public String application;
        public String edition;
        public String language;
        public String access_token;
        public int env;         // Asset environment type; an ASSET_ENV_??? constant
    }

    public static class FeaturedRequest extends StandardRequest{
        public FeaturedRequest(StandardRequest request, int category){
            this.access_token = request.access_token;
            this.application = request.application;
            this.edition = request.edition;
            this.env = request.env;
            this.language = request.language;
            this.category_idx = category;
        }
        public int category_idx;
    }

    /** GET CATEGORY INFO
     *
     *   https://test-api-kinemaster-assetstore.nexstreaming.com/v2/category/info
     *
     *   Uses standard request.
     */
    public static class GetCategoryInfoResponse {
        public List<CategoryInfo> objList;
    }

    /** GET CATEGORY LIST
     *
     *   https://test-api-kinemaster-assetstore.nexstreaming.com/v2/category/list/%categoryIdx%/
     *   https://test-api-kinemaster-assetstore.nexstreaming.com/v2/category/list/%categoryIdx%/%SubcategoryIdx%
     *
     *   Uses standard request.
     */
    public static class GetCategoryListResponse {
        public String log_date;
        public List<AssetInfo> objList;

    }

    /** GET ASSET INFO
     *
     *   https://test-api-kinemaster-assetstore.nexstreaming.com/v2/asset/info/%assetIdx%/
     *
     *   Uses standard request.
     *   Respond asset info
     */
//    public static class GetAssetInfoResponse {
//        public int language_idx;
//        public int asset_version;
//        public String thumbnail_path;
//        public int asset_sversion;
//        public String category_aliasName;
//        public List<LangString> categoryName;
//        public String asset_id;
//        public int category_idx;
//        public List<ThumbInfo> thumb;
//        public String title;
//        public List<LangString> subcategoryName;
//        public String description;
//        public String asset_filepath;
//        public int subcategory_idx;
//        public int asset_filesize;
//        public String categoryimagePath;
//        public int idx;
//        public String categoryimagePath_on;
//        public String log_date;
//        public int default_flag;
//        public List<LangString> assetName;
//        public String videoclip_path;
//        public String priceType;
//
//    }

    /** GET FEATURED ASSET INFO
     *
     *  https://test-api-kinemaster-assetstore.nexstreaming.com/v2/featured/list/%categoryIdx%/
     *
     *  Uses standard request.
     */
    public static class GetFeaturedAssetInfoResponse {
        public String log_date;
        public List<AssetInfo> objList;

    }

    /** COMMON DATA
     *
     */
    public static class LangString {
        public String string_title;
        public String string_desc;
        public int default_flag;
        public String language_code;
    }


    public static class AssetInfo {
        public int language_idx;
        public String thumbnail_path;
        public String thumbnail_path_s;
        public String thumbnail_path_l;
        public int asset_sversion ;
        public String category_aliasName;
        public List<LangString> categoryName;
        public int category_idx;
        public List<ThumbInfo> thumb;
        public String title;
        public List<LangString> subcategoryName;
        public String description;
        public String asset_filepath;
        public int asset_version ;
        public int subcategory_idx;
        public int asset_filesize;
        public String categoryimagePath;
        public int idx;
        public String categoryimagePath_on;
        public String asset_id ;
        public int default_flag;
        public List<LangString> assetName;
        public String videoclip_path;
        public String priceType;
        public String log_date; // Only included in individual asset info; not in list
        public int update_time;
    }


    public static class CategoryInfo {
        public String imagePath;
        public int category_idx;
        public List<LangString> categoryName;
        public String imagePath_on;
        public String category_url;
        public String category_aliasName ;
        public List<SubcategoryInfo> subcategory_info;

    }
    public static class ThumbInfo {
        public int asset_idx;
        public String file_path;
        public int idx;

    }

    public static class SubcategoryInfo {
        public String subcategory_aliasName;
        public List<LangString> subcategoryName;
        public int subcategory_idx;

    }

    public static class RefreshOrDeleteAuthRequest {
        public String access_token;
        public String application;
        public String app_ucode;
    }

    public static class AuthorizeOrRefreshResponse {
        public String access_token;
        public int scope;
        public int expires_in;                  // Remaining token valid time in seconds
    }

    public static class StatusResponse {
        public int result;
    }

    ///// TODO

    public static class GetNewAssetList {
        public String log_date;
        public List<AssetInfo> objList;

    }

}
