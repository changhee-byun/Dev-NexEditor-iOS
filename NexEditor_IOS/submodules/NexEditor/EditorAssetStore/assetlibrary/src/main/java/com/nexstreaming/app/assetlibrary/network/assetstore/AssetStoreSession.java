package com.nexstreaming.app.assetlibrary.network.assetstore;


import android.content.Context;
import android.content.SharedPreferences;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.util.Log;

import com.android.volley.AuthFailureError;
import com.android.volley.Cache;
import com.android.volley.NetworkResponse;
import com.android.volley.ParseError;
import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.ServerError;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.HttpHeaderParser;
import com.android.volley.toolbox.ImageLoader;
import com.google.gson.Gson;
import com.google.gson.JsonSyntaxException;
import com.nexstreaming.app.assetlibrary.LL;
import com.nexstreaming.app.assetlibrary.utils.UserInfo;
import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.app.assetlibrary.network.KMVolley;
import com.nexstreaming.app.assetlibrary.network.assetstore.json.AssetStoreAPIData;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class AssetStoreSession {

    private static final String LOG_TAG = "AssetStoreSession";
    private static final int NEW_ASSET_DAYS = 14;
    private static AssetStoreSession instance = null;
    private final KMVolley volley;
    private static final String PROTOCOL_CHARSET = "utf-8";
    private static final String PROTOCOL_CONTENT_TYPE = "application/json";// String.format("application/json; charset=%s", PROTOCOL_CHARSET);
    private static final int DEFAULT_API_VERSION = 2;
    private static final int DEFAULT_SCOPE = 1;
    private static final Gson gson = new Gson();
    private String baseURL = null;
    private String clientID = null;
    private String ucode = null;
    private String marketId = null;
    private String clientInfo = null;
    private int apiVersion = DEFAULT_API_VERSION;
    private int scope = DEFAULT_SCOPE;
    private Context context;
    private Task authorizeTask = new Task();
    private Task refreshTask;
    private boolean authInProgress = false;
    private boolean refreshInProgress = false;
    private String accessToken;
    private String edition;
    private String application;
    private String language;
    private long tokenExpiration;
    private long tokenReceived;
    private AssetEnv assetEnv = AssetEnv.PRODUCTION;
    private ResultTask<List<StoreCategoryInfo>> cachedCategoryList;

    // Controllers
    private static final String CTL_OAUTH       = "oauth";
    private static final String CTL_CATEGORY    = "category";
    private static final String CTL_ASSET       = "asset";
    private static final String CTL_FEATURED    = "featured";

    // Error codes
    private static final int ERR_ERR_NORMAL         = 501;
    private static final int ERR_APPAUTH_FAILED     = 502;
    private static final int ERR_INVAILD_LICENSE    = 503;
    private static final int ERR_INVAILD_SCOPE      = 504;
    private static final int ERR_TOKEN_EXPIRE       = 505;
    private static final int ERR_INVALID_APIVERSION = 506;
    private static final int ERR_INVALID_REQUEST    = 507;
    private static final int ERR_INVALID_PARAMETERS = 508;
    private static final int ERR_NOT_FOUND_LIST     = 509;
    private static final int ERR_SOMETHING_WRONG    = 510;

    public enum SessionError implements Task.TaskError {
        AUTH_BAD_SERVER_RESPONSE,
        EXPIRATION_TOO_SHORT,
        TOKEN_EXPIRED, NO_TOKEN;

        @Override
        public Exception getException() {
            return null;
        }

        @Override
        public String getMessage() {
            return name();
        }

        @Override
        public String getLocalizedMessage(Context context) {
            return name();
        }
    }

    public enum AssetEnv {
        DRAFT, STAGING, PRODUCTION
    }

    public static AssetStoreSession getInstance(Context context) {
        if( instance==null ) {
            instance = new AssetStoreSession(context.getApplicationContext());
        }
        return instance;
    }

    private AssetStoreSession(Context context) {
        volley = KMVolley.getInstance(context);
        this.context = context.getApplicationContext();
    }

    public void setBaseURL(String baseURL) {
        if( !baseURL.endsWith("/")) {
            this.baseURL = baseURL + "/";
        } else {
            this.baseURL = baseURL;
        }
    }

    public void setUcode(String ucode) {
        this.ucode = ucode;     // KineMaster: NDBhNDU1NGJjYjU5MzRlZThmOTU4YmYxMmYzZTU3MmZmOTM3NTZjZTpHb29nbGU
    }

    public void setApiVersion(int apiVersion) {
        this.apiVersion = apiVersion;
    }

    public void setClientID(String clientID) {
        this.clientID = clientID;
    }

    public void setMarketId(String marketId) {
        this.marketId = marketId;
    }

    public void setEdition(String edition) {
        this.edition = edition;
    }

    public void setApplication(String application) {
        this.application = application;
    }

    public void setLanguage(String language) {
        this.language = language;
    }

    public void setClientInfo(String clientInfo) {
        this.clientInfo = clientInfo;
    }

    public void beginSession() {
        ensureAuthorized();
    }

    public void clearCache() {
        KMVolley.getInstance(context).getRequestQueue().getCache().clear();
        cachedCategoryList = null;
    }

    public void precacheCategories() {
        getCategoryList().onResultAvailable(new ResultTask.OnResultAvailableListener<List<StoreCategoryInfo>>() {
            @Override
            public void onResultAvailable(ResultTask<List<StoreCategoryInfo>> task, Task.Event event, List<StoreCategoryInfo> result) {
                for( StoreCategoryInfo cat: result ) {
                    String iconURL = cat.getIconURL();
                    if( iconURL!=null ) {
                        KMVolley.getInstance(context).getCategoryIconImageLoader().get(iconURL, new ImageLoader.ImageListener() {
                            @Override
                            public void onResponse(ImageLoader.ImageContainer response, boolean isImmediate) {

                            }

                            @Override
                            public void onErrorResponse(VolleyError error) {

                            }
                        });
                    }
                }
            }
        });
    }

    /**
     * Returns a list of categories, along with the list of subcategories for each category.
     * @return
     */
    public ResultTask<List<StoreCategoryInfo>> getCategoryList() {

        if( cachedCategoryList!=null ) return cachedCategoryList;

        final ResultTask<List<StoreCategoryInfo>> resultTask = new ResultTask<>();
        ensureAuthorized().onComplete(new Task.OnTaskEventListener() {
            @Override
            public void onTaskEvent(Task t, Task.Event e) {
                raw_getCategoryInfo(makeStandardRequest()).onResultAvailable(new ResultTask.OnResultAvailableListener<AssetStoreAPIData.GetCategoryInfoResponse>() {
                    @Override
                    public void onResultAvailable(ResultTask<AssetStoreAPIData.GetCategoryInfoResponse> task, Task.Event event, AssetStoreAPIData.GetCategoryInfoResponse result) {
                        if( result.objList==null ) {
                            resultTask.sendResult(Collections.<StoreCategoryInfo>emptyList());
                        } else {
                            List<StoreCategoryInfo> categoryList = new ArrayList<StoreCategoryInfo>(result.objList.size());
                            for( AssetStoreAPIData.CategoryInfo o: result.objList ) {
                                categoryList.add(new StoreCategoryInfoImp(o));
                            }
                            resultTask.sendResult(categoryList);
                            cachedCategoryList = resultTask;
                        }
                    }
                }).onFailure(resultTask);
            }
        }).onFailure(resultTask);

        return resultTask;
    }

    public ResultTask<List<StoreAssetInfo>> getAssetsInCategory(StoreCategoryInfo category) {
        return getAssetsInCategory(category.getCategoryIdx());
    }

    public ResultTask<List<StoreAssetInfo>> getAssetsInSubcategory(StoreSubcategoryInfo subcategory) {
        return getAssetsInSubcategory(subcategory.getCategoryIdx(),subcategory.getSubcategoryIdx());
    }

    public ResultTask<List<StoreAssetInfo>> getAssetsInCategory(final int categoryIdx) {
        final ResultTask<List<StoreAssetInfo>> resultTask = new ResultTask<>();
        ensureAuthorized().onComplete(new Task.OnTaskEventListener() {
            @Override
            public void onTaskEvent(Task t, Task.Event e) {
                raw_getCategoryList(makeStandardRequest(),categoryIdx).onResultAvailable(new ResultTask.OnResultAvailableListener<AssetStoreAPIData.GetCategoryListResponse>() {
                    @Override
                    public void onResultAvailable(ResultTask<AssetStoreAPIData.GetCategoryListResponse> task, Task.Event event, AssetStoreAPIData.GetCategoryListResponse result) {
                        if( result.objList==null ) {
                            resultTask.sendResult(Collections.<StoreAssetInfo>emptyList());
                        } else {
                            List<StoreAssetInfo> categoryList = new ArrayList<StoreAssetInfo>(result.objList.size());
                            for( AssetStoreAPIData.AssetInfo o: result.objList ) {
                                categoryList.add(new StoreAssetInfoImp(o));
                            }
                            resultTask.sendResult(categoryList);
                        }
                    }
                }).onFailure(resultTask);
            }
        }).onFailure(resultTask);
        return resultTask;
    }

    public ResultTask<List<StoreAssetInfo>> getAssetsInSubcategory(final int categoryIdx, final int subCategoryIdx) {
        final ResultTask<List<StoreAssetInfo>> resultTask = new ResultTask<>();
        ensureAuthorized().onComplete(new Task.OnTaskEventListener() {
            @Override
            public void onTaskEvent(Task t, Task.Event e) {
                raw_getCategoryList(makeStandardRequest(),categoryIdx,subCategoryIdx).onResultAvailable(new ResultTask.OnResultAvailableListener<AssetStoreAPIData.GetCategoryListResponse>() {
                    @Override
                    public void onResultAvailable(ResultTask<AssetStoreAPIData.GetCategoryListResponse> task, Task.Event event, AssetStoreAPIData.GetCategoryListResponse result) {
                        if( result.objList==null ) {
                            resultTask.sendResult(Collections.<StoreAssetInfo>emptyList());
                        } else {
                            List<StoreAssetInfo> categoryList = new ArrayList<StoreAssetInfo>(result.objList.size());
                            for( AssetStoreAPIData.AssetInfo o: result.objList ) {
                                categoryList.add(new StoreAssetInfoImp(o));
                            }
                            resultTask.sendResult(categoryList);
                        }
                    }
                }).onFailure(resultTask);
            }
        }).onFailure(resultTask);
        return resultTask;
    }

    public ResultTask<StoreAssetInfo> getAssetInfo(final int assetIdx) {
        final ResultTask<StoreAssetInfo> resultTask = new ResultTask<>();
        ensureAuthorized().onComplete(new Task.OnTaskEventListener() {
            @Override
            public void onTaskEvent(Task t, Task.Event e) {
                raw_getAssetInfo(makeStandardRequest(),assetIdx).onResultAvailable(new ResultTask.OnResultAvailableListener<AssetStoreAPIData.AssetInfo>() {
                    @Override
                    public void onResultAvailable(ResultTask<AssetStoreAPIData.AssetInfo> task, Task.Event event, AssetStoreAPIData.AssetInfo result) {
                            StoreAssetInfo taskResult = new StoreAssetInfoImp(result);
                            resultTask.sendResult(taskResult);
                    }
                }).onFailure(resultTask);
            }
        }).onFailure(resultTask);
        return resultTask;
    }

    public ResultTask<StoreFeaturedAssetInfo> getFeaturedAssets(final int featureIndex, final int categoryIndex) {
        final ResultTask<StoreFeaturedAssetInfo> resultTask = new ResultTask<>();
        ensureAuthorized().onComplete(new Task.OnTaskEventListener() {
            @Override
            public void onTaskEvent(Task t, Task.Event e) {
                AssetStoreAPIData.StandardRequest request = makeStandardRequest();
                if(categoryIndex > 0){
                    request = new AssetStoreAPIData.FeaturedRequest(makeStandardRequest(), categoryIndex);
                }
                raw_getFeaturedAssetList(request, featureIndex).onResultAvailable(new ResultTask.OnResultAvailableListener<AssetStoreAPIData.GetFeaturedAssetInfoResponse>() {
                    @Override
                    public void onResultAvailable(ResultTask<AssetStoreAPIData.GetFeaturedAssetInfoResponse> task, Task.Event event, AssetStoreAPIData.GetFeaturedAssetInfoResponse result) {
                        StoreFeaturedAssetInfo taskResult = new StoreFeaturedAssetInfoImp(result);
                        resultTask.sendResult(taskResult);
                    }
                }).onFailure(resultTask);
            }
        }).onFailure(resultTask);
        return resultTask;
    }

    private static Map<String,String> stringMapFromLangStringTitle(List<AssetStoreAPIData.LangString> langStrings) {
        Map<String,String> result = new HashMap<>();
        for(AssetStoreAPIData.LangString s: langStrings) {
            result.put(s.language_code, s.string_title);
        }
        return result;
    }

    private static Map<String,String> stringMapFromLangStringDesc(List<AssetStoreAPIData.LangString> langStrings) {
        Map<String,String> result = new HashMap<>();
        for(AssetStoreAPIData.LangString s: langStrings) {
            result.put(s.language_code, s.string_desc);
        }
        return result;
    }

    private static class StoreFeaturedAssetInfoImp implements StoreFeaturedAssetInfo {

        private final AssetStoreAPIData.GetFeaturedAssetInfoResponse wrapped;
        private List<StoreAssetInfo> storeAssetInfoList = null;

        private StoreFeaturedAssetInfoImp(AssetStoreAPIData.GetFeaturedAssetInfoResponse wrapped) {
            this.wrapped = wrapped;
        }

        @Override
        public long getServerTimestamp() {
            return Long.parseLong(wrapped.log_date); // TODO: Check original structure
        }

        @Override
        public String getFeatuedAssetLayoutType() {
            return null;            // TODO: Implement after implemented on server side
        }

        @Override
        public List<StoreAssetInfo> getFeaturedAssetList() {
            if( storeAssetInfoList==null ) {
                storeAssetInfoList = new ArrayList<>();
                for( AssetStoreAPIData.AssetInfo o: wrapped.objList ) {
                    storeAssetInfoList.add(new StoreAssetInfoImp(o));
                }
            }
            return storeAssetInfoList;
        }
    }

    public static class StoreAssetInfoImp implements StoreAssetInfo {

        private final AssetStoreAPIData.AssetInfo wrapped;
        private Map<String,String> subcategoryName;
        private Map<String,String> assetName;
        private Map<String,String> assetDescription;
        List<String> thumbnailPaths;

        public StoreAssetInfoImp(AssetStoreAPIData.AssetInfo wrapped) {
            this.wrapped = wrapped;
        }

        @Override
        public int getAssetIndex() {
            return wrapped.idx;
        }

        @Override
        public String getAssetId() {
            return wrapped.asset_id;
        }

        @Override
        public String getAssetTitle() {
            return wrapped.title;
        }

        @Override
        public String getAssetDescription() {
            return wrapped.description;
        }

        @Override
        public int getCategoryIndex() {
            return wrapped.category_idx;
        }

        @Override
        public String getCategoryAliasName() {
            return wrapped.category_aliasName;
        }

        @Override
        public String getCategoryIconURL() {
            return wrapped.categoryimagePath;
        }

        @Override
        public int getSubCategoryIndex() {
            return wrapped.subcategory_idx;
        }

        @Override
        public Map<String, String> getSubCategoryNameMap() {
            if( subcategoryName==null ) {
                subcategoryName = stringMapFromLangStringTitle(wrapped.subcategoryName);
            }
            return subcategoryName;
        }

        @Override
        public Map<String, String> getAssetNameMap() {
            if( assetName==null ) {
                assetName = stringMapFromLangStringTitle(wrapped.assetName);
            }
            return assetName;
        }

        @Override
        public Map<String, String> getAssetDescriptionMap() {
            if(assetDescription == null){
                assetDescription = stringMapFromLangStringDesc(wrapped.assetName);
            }
            return assetDescription;
        }

        @Override
        public String getAssetPackageDownloadURL() {
            return wrapped.asset_filepath;
        }

        @Override
        public String getPriceType() {
            return wrapped.priceType;
        }

        @Override
        public String getAssetThumbnailURL() {
            return wrapped.thumbnail_path;
        }

        @Override
        public String getAssetThumbnailURL_L() {
            return wrapped.thumbnail_path_l;
        }

        @Override
        public String getAssetThumbnailURL_S() {
            return wrapped.thumbnail_path_s;
        }

        @Override
        public List<String> getThumbnailPaths() {
            if( thumbnailPaths==null ) {
                thumbnailPaths = new ArrayList<>();
                if(wrapped.thumb!=null) {
                    for (AssetStoreAPIData.ThumbInfo thumb : wrapped.thumb) {
                        if (thumb.file_path != null) {
                            thumbnailPaths.add(thumb.file_path);
                        }
                    }
                }
            }
            return thumbnailPaths;
        }

        @Override
        public int getAssetVersion() {
            return wrapped.asset_version;
        }

        @Override
        public int getAssetScopeVersion() {
            return wrapped.asset_sversion;
        }

        @Override
        public int getAssetFilesize() {
            return wrapped.asset_filesize;
        }

        @Override
        public String getAssetVideoURL() {
            return wrapped.videoclip_path;
        }

        @Override
        public int getUpdateTime() {
            return wrapped.update_time;
        }

        @Override
        public String getSubCategoryAliasName() {
            return null;
        }

        @Override
        public boolean equals(Object obj) {
            if(obj != null && obj instanceof StoreAssetInfo){
                return getAssetIndex() == ((StoreAssetInfo) obj).getAssetIndex();
            }
            return super.equals(obj);
        }

    }

    private static class StoreCategoryInfoImp implements StoreCategoryInfo {

        private final AssetStoreAPIData.CategoryInfo wrapped;
        private Map<String,String> categoryName = null;
        private List<StoreSubcategoryInfo> subcategoryList = null;

        private StoreCategoryInfoImp(AssetStoreAPIData.CategoryInfo wrapped) {
            this.wrapped = wrapped;
        }

        @Override
        public String getIconURL() {
            return wrapped.imagePath;
        }

        @Override
        public String getSelectedIconURL() {
            return wrapped.imagePath_on;
        }

        @Override
        public int getCategoryIdx() {
            return wrapped.category_idx;
        }

        @Override
        public Map<String, String> getCategoryName() {
            if( categoryName==null ) {
                categoryName = stringMapFromLangStringTitle(wrapped.categoryName);
            }
            return categoryName;
        }

        @Override
        public String getCategoryAliasName() {
            return wrapped.category_aliasName;
        }

        @Override
        public List<StoreSubcategoryInfo> getSubCategories() {
            if(subcategoryList==null) {
                if( wrapped.subcategory_info==null ) {
                    subcategoryList = Collections.emptyList();
                } else {
                    subcategoryList = new ArrayList<>(wrapped.subcategory_info.size());
                    for (AssetStoreAPIData.SubcategoryInfo subcategoryInfo : wrapped.subcategory_info) {
                        subcategoryList.add(new StoreSubcategoryInfoImp(subcategoryInfo, getCategoryIdx()));
                    }
                }
            }
            return subcategoryList;
        }
    }

    private static class StoreSubcategoryInfoImp implements StoreSubcategoryInfo {

        private final AssetStoreAPIData.SubcategoryInfo wrapped;
        private final int categoryIdx;
        private Map<String,String> subcategoryName = null;

        private StoreSubcategoryInfoImp(AssetStoreAPIData.SubcategoryInfo wrapped, int categoryIdx) {
            this.wrapped = wrapped;
            this.categoryIdx = categoryIdx;
        }

        @Override
        public String getSubcategoryAliasName() {
            return wrapped.subcategory_aliasName;
        }

        @Override
        public Map<String, String> getSubcategoryName() {
            if( subcategoryName==null ) {
                subcategoryName = stringMapFromLangStringTitle(wrapped.subcategoryName);
            }
            return subcategoryName;
        }

        @Override
        public int getSubcategoryIdx() {
            return wrapped.subcategory_idx;
        }

        @Override
        public int getCategoryIdx() {
            return categoryIdx;
        }
    }

    private AssetStoreAPIData.StandardRequest makeStandardRequest() {
        if( edition==null )
            throw new IllegalStateException("edition not set");
        AssetStoreAPIData.StandardRequest req = new AssetStoreAPIData.StandardRequest();
        req.access_token = accessToken;
        req.application = application==null? UserInfo.getAppPackageName(context):application;
        req.edition = edition;
        req.language = language==null?context.getResources().getConfiguration().locale.toString().replace("_","-"):language;
        switch(assetEnv) {
            case DRAFT:
                req.env = AssetStoreAPIData.ASSET_ENV_DRAFT;
                break;
            case STAGING:
                req.env = AssetStoreAPIData.ASSET_ENV_STAGING;
                break;
            case PRODUCTION:
            default:
                req.env = AssetStoreAPIData.ASSET_ENV_PRODUCTION;
                break;
        }
        return req;
    }

    public void setAssetEnv(AssetEnv assetEnv) {
        this.assetEnv = assetEnv;
    }

    private boolean isAuthorized() {
        return accessToken!=null && SystemClock.uptimeMillis()<=tokenExpiration;
    }

    private boolean needsRefresh() {
        long t = SystemClock.uptimeMillis();
        return ( accessToken!=null && t<=tokenExpiration-10000 && t>(tokenReceived+tokenExpiration)/2 );
    }

    public AssetEnv getAssetEnv() {
        return assetEnv;
    }

    private Task ensureAuthorized() {

        if( authInProgress || refreshInProgress )
            return authorizeTask;

        if( !authorizeTask.isRunning() )
            authorizeTask = new Task();

        if (isAuthorized()) {
            if( needsRefresh() ) {
                refresh().onComplete(new Task.OnTaskEventListener() {
                    @Override
                    public void onTaskEvent(Task t, Task.Event e) {
                        authorizeTask.signalEvent(Task.Event.COMPLETE, Task.Event.SUCCESS);
                    }
                }).onFailure(authorizeTask);
                return authorizeTask;
            } else {
                return Task.COMPLETED_TASK;
            }
        }

        if( ucode==null )
            throw new IllegalStateException("no ucode set");
        if( marketId==null )
            throw new IllegalStateException("no market id set");
        if( edition==null )
            throw new IllegalStateException("no edition id set");

        AssetStoreAPIData.AuthorizeRequest req = new AssetStoreAPIData.AuthorizeRequest();
        req.app_uuid = UserInfo.getAppUuid(context);
        req.app_name = UserInfo.getAppName(context);
        req.app_version = UserInfo.getAppVersionName(context);
        req.app_ucode = ucode;
        req.client_id = clientID==null?("UUID!"+req.app_uuid):clientID;
        req.application = UserInfo.getAppPackageName(context);
        req.market_id = marketId;
        req.edition = edition;
//        req.scope = scope;
//        req.client_info = clientInfo;
        req.scope = 1;
        req.client_info = null;
        authInProgress = true;
        raw_authorize(req).onResultAvailable(new ResultTask.OnResultAvailableListener<AssetStoreAPIData.AuthorizeOrRefreshResponse>() {
            @Override
            public void onResultAvailable(ResultTask<AssetStoreAPIData.AuthorizeOrRefreshResponse> task, Task.Event event, AssetStoreAPIData.AuthorizeOrRefreshResponse result) {
                authInProgress = false;
                if( result.access_token == null || result.access_token.length()<1 ) {
                    authorizeTask.sendFailure(SessionError.AUTH_BAD_SERVER_RESPONSE);
                } else if( result.expires_in < 60 ) {
                    authorizeTask.sendFailure(SessionError.EXPIRATION_TOO_SHORT);
                } else {
                    accessToken = result.access_token;
                    tokenReceived = SystemClock.uptimeMillis();
                    tokenExpiration = SystemClock.uptimeMillis() + result.expires_in*1000;
                    authorizeTask.signalEvent(Task.Event.COMPLETE, Task.Event.SUCCESS);
                }
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                authInProgress = false;
                if(!handleFailure(failureReason))
                    authorizeTask.sendFailure(failureReason);
            }
        });

        return authorizeTask;
    }

    private Task refresh() {

        if( refreshInProgress )
            return refreshTask;

        if( authInProgress )
            throw new IllegalStateException();

        if( ucode==null )
            throw new IllegalStateException();

        if( accessToken==null )
            return Task.makeFailedTask(SessionError.NO_TOKEN);

        refreshTask = new Task();
        AssetStoreAPIData.RefreshOrDeleteAuthRequest req = new AssetStoreAPIData.RefreshOrDeleteAuthRequest();
        req.access_token = accessToken;
        req.app_ucode = ucode;
        req.application = UserInfo.getAppPackageName(context);
        refreshInProgress = true;
        raw_refreshAuth(req).onResultAvailable(new ResultTask.OnResultAvailableListener<AssetStoreAPIData.AuthorizeOrRefreshResponse>() {
            @Override
            public void onResultAvailable(ResultTask<AssetStoreAPIData.AuthorizeOrRefreshResponse> task, Task.Event event, AssetStoreAPIData.AuthorizeOrRefreshResponse result) {
                refreshInProgress = false;
                if( result.access_token == null || result.access_token.length()<1 ) {
                    refreshTask.sendFailure(SessionError.AUTH_BAD_SERVER_RESPONSE);
                } else if( result.expires_in < 60 ) {
                    refreshTask.sendFailure(SessionError.EXPIRATION_TOO_SHORT);
                } else {
                    accessToken = result.access_token;
                    tokenReceived = SystemClock.uptimeMillis();
                    tokenExpiration = SystemClock.uptimeMillis() + result.expires_in*1000;
                    refreshTask.signalEvent(Task.Event.COMPLETE, Task.Event.SUCCESS);
                }
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                refreshInProgress = false;
                refreshTask.sendFailure(failureReason);
                handleFailure(failureReason);
            }
        });

        return refreshTask;
    }

    private boolean handleFailure(Task.TaskError failureReason) {
        if( failureReason== SessionError.TOKEN_EXPIRED) {
            accessToken = null;
            ensureAuthorized();
            return true;
        }
        return false;
    }

    private ResultTask<AssetStoreAPIData.AuthorizeOrRefreshResponse> raw_authorize(AssetStoreAPIData.AuthorizeRequest request) {
        return callAPI(request,AssetStoreAPIData.AuthorizeOrRefreshResponse.class,CTL_OAUTH,"authorize");
    }

    private ResultTask<AssetStoreAPIData.AuthorizeOrRefreshResponse> raw_refreshAuth(AssetStoreAPIData.RefreshOrDeleteAuthRequest request) {
        return callAPI(request,AssetStoreAPIData.AuthorizeOrRefreshResponse.class,CTL_OAUTH,"refreshauth");
    }

    private ResultTask<AssetStoreAPIData.StatusResponse> raw_delAuth(AssetStoreAPIData.RefreshOrDeleteAuthRequest request) {
        return callAPI(request,AssetStoreAPIData.StatusResponse.class,CTL_OAUTH,"delauth");
    }

    private ResultTask<AssetStoreAPIData.GetCategoryInfoResponse> raw_getCategoryInfo(AssetStoreAPIData.StandardRequest request) {
        return callAPI(request,AssetStoreAPIData.GetCategoryInfoResponse.class,CTL_CATEGORY,"info");
    }

    private ResultTask<AssetStoreAPIData.GetCategoryListResponse> raw_getCategoryList(AssetStoreAPIData.StandardRequest request, int categoryIdx) {
        return callAPI(request,AssetStoreAPIData.GetCategoryListResponse.class,CTL_CATEGORY,"list",categoryIdx);
    }

    private ResultTask<AssetStoreAPIData.GetCategoryListResponse> raw_getCategoryList(AssetStoreAPIData.StandardRequest request, int categoryIdx, int subCategoryIdx) {
        return callAPI(request,AssetStoreAPIData.GetCategoryListResponse.class,CTL_CATEGORY,"list",categoryIdx,subCategoryIdx);
    }

    private ResultTask<AssetStoreAPIData.AssetInfo> raw_getAssetInfo(AssetStoreAPIData.StandardRequest request, int assetIdx) {
        return callAPI(request,AssetStoreAPIData.AssetInfo.class,CTL_ASSET,"info",assetIdx);
    }

    private ResultTask<AssetStoreAPIData.GetFeaturedAssetInfoResponse> raw_getFeaturedAssetList(AssetStoreAPIData.StandardRequest request, int featuredIdx) {
        return callAPI(request,AssetStoreAPIData.GetFeaturedAssetInfoResponse.class,CTL_FEATURED,"list",featuredIdx);
    }

    private <TREQUEST,TRESPONSE> ResultTask<TRESPONSE> callAPI(TREQUEST request, Class<TRESPONSE> responseClass, String controller, String function, Object... elements) {
        if( baseURL==null )
            throw new IllegalStateException("Base URL not set");
        ResultTask<TRESPONSE> resultTask = new ResultTask<>();
        StringBuilder sb = new StringBuilder(baseURL.length()+64);
        sb.append(baseURL).append('v').append(apiVersion).append('/').append(controller).append('/').append(function);
        for( Object e: elements ) {
            sb.append('/').append(String.valueOf(e));
        }
        if(LL.D) Log.d(LOG_TAG,"Volley: callAPI:" + sb.toString());
        volley.add(new StoreRequest<TREQUEST,TRESPONSE>(sb.toString(),request,resultTask,responseClass));
        return resultTask;
    }

    private static class StoreRequest<TREQUEST,TRESPONSE> extends Request<TRESPONSE> {

        private final ResultTask<TRESPONSE> resultTask;
        private final Class<TRESPONSE> responseClass;
        private final TREQUEST request;

        public StoreRequest(String url, TREQUEST request, final ResultTask<TRESPONSE> resultTask, Class<TRESPONSE> responseClass) {
            super(Method.POST, url, new Response.ErrorListener() {
                @Override
                public void onErrorResponse(VolleyError error) {
                    if(LL.D) Log.d(LOG_TAG,"Volley: ERROR: " + error.getMessage());
                    if( error.networkResponse!=null && error.networkResponse.statusCode==ERR_TOKEN_EXPIRE ) {
                        resultTask.sendFailure(SessionError.TOKEN_EXPIRED);
                    } else {
                        String msg = error.getMessage();
                        if( msg==null || msg.trim().length()<1 ) {
                            if( error.networkResponse != null ) {
                                if( error.networkResponse.statusCode != 200 ) {
                                    msg = error.getClass().getSimpleName() + " (" + error.networkResponse.statusCode + ")";
                                } else {
                                    msg = error.getClass().getSimpleName();
                                }
                            } else {
                                msg = error.getClass().getSimpleName();
                            }
                        }
                        resultTask.sendFailure(Task.makeTaskError(msg,error));
                    }
                }
            });
            this.resultTask = resultTask;
            this.responseClass = responseClass;
            this.request = request;
        }

        @Override
        protected Response<TRESPONSE> parseNetworkResponse(NetworkResponse response) {
            if( response.statusCode!=200 ) {
                if(LL.D) Log.d(LOG_TAG,"Volley: parseNetworkResponse: ERROR " + response.statusCode);
                return Response.error(new ServerError(response));
            }
            try {
                String charset = HttpHeaderParser.parseCharset(response.headers, PROTOCOL_CHARSET);
                String jsonString = new String(response.data, charset).trim();
                // Sometimes there's garbage after the end of the JSON (the server
                // sends a response that looks like "{....}null") so we need to clean that
                // up first, othewise GSON will choke on it.
                if( jsonString.length()>0 ) {
                    char c = jsonString.charAt(0);
                    if( c=='[' ) {
                        jsonString = jsonString.substring(0,jsonString.lastIndexOf(']')+1);
                    } else if( c=='{' ) {
                        jsonString = jsonString.substring(0,jsonString.lastIndexOf('}')+1);
                    } else if( c=='"' ) {
                        jsonString = jsonString.substring(0,jsonString.lastIndexOf('"')+1);
                    }
                }
                if(LL.D) Log.d(LOG_TAG,"Volley: parseNetworkResponse: " + response.statusCode + " charset=" + charset + " response=" + jsonString);
                TRESPONSE actualResponse = gson.fromJson(jsonString,responseClass);
                if(LL.D) Log.d(LOG_TAG,"Volley: parseNetworkResponse got=" + String.valueOf(actualResponse));
                Cache.Entry entry = HttpHeaderParser.parseCacheHeaders(response);
                return Response.success(actualResponse, entry );
            } catch (UnsupportedEncodingException e) {
                return Response.error(new ParseError(e));
            } catch (JsonSyntaxException e) {
                return Response.error(new ParseError(e));
            }
        }

        @Override
        public String getBodyContentType() {
            return PROTOCOL_CONTENT_TYPE;
        }

        @Override
        public byte[] getBody() throws AuthFailureError {
            String body = gson.toJson(request);
            if(LL.D) Log.d(LOG_TAG,"Volley: getBody:" +body);
            return body.getBytes();
        }

        @Override
        protected void deliverResponse(TRESPONSE response) {
            resultTask.sendResult(response);
        }

    }

    public boolean isNewAsset(StoreAssetInfo assetInfo) {
        final String KEY_FIRSTTIME = "km.assetstore.firsttimeentry";
        if( assetInfo==null ) return false;
        long updateTime = assetInfo.getUpdateTime();
        long currentTime = System.currentTimeMillis()/1000;
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        long firstTime = prefs.getLong(KEY_FIRSTTIME,0);
        if( firstTime<=0 ) {
            firstTime = currentTime;
            prefs.edit().putLong(KEY_FIRSTTIME,firstTime).apply();
        }
        if(LL.D) Log.d(LOG_TAG,"isNewAsset:" + String.valueOf(assetInfo.getAssetTitle()) + ": upd=" + updateTime + " cur=" + currentTime);
        return ( currentTime > updateTime && updateTime > firstTime && currentTime < updateTime + (NEW_ASSET_DAYS*24*60*60));
    }
}
