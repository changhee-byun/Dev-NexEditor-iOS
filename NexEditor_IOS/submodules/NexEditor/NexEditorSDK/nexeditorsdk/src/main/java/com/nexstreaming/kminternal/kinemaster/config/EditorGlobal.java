package com.nexstreaming.kminternal.kinemaster.config;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;
import java.util.Random;
import java.util.TimeZone;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;

import com.nexstreaming.nexeditorsdk.BuildConfig;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;

public final class EditorGlobal {

    public static final String THEME_PREFIX_NAME = "n2_theme_thumb_";
    public static final String[] imageExtensions = new String[]{".jpg", ".jpeg", ".png", ".webp"};
    public static final String[] videoExtensions = new String[]{".mp4", ".3gp", ".3gpp", ".mov", ".k3g", ".acc", ".avi", ".wmv"};
    public static final String[] audioExtensions = new String[]{".aac", ".mp3", ".3gp", ".3gpp"};
    public static final String projectFileExtension = ".nexvideoproject";

    public static enum Edition {
        DeviceLock, TimeLock, PlayStore
    }

    public static enum VersionType {
        Alpha, Beta, Dev, RC, Release
    }

    public static enum DataUsage {
        WIFI_AND_MOBILE,
        WIFI_ONLY,
        NEVER,
        ASK_WIFI_OR_MOBILE,
        ASK_WIFI_MOBILE_NEVER
    }

    public static NexEditor getEditor() {
        if (KineMasterSingleTon.getApplicationInstance() != null)
            return KineMasterSingleTon.getApplicationInstance().getEditor();
        else
            return null;
        //return KineMasterSingleTon.getEditor();
    }

    /**
     * EDITION - Controls overall APK build style
     * <p>
     * DeviceLock	- Only enable APK to run on certain devices (device list
     * is configured in the engine .so file).
     * <p>
     * TimeLock	- Only enable APK to run until a specific date.  Be sure
     * to set TIMELOCK_EXPIRATION to the desired expiration date.
     * <p>
     * PlayStore	- Build an APK for release on Google Play.  Note that both
     * PRO and FREE versions use this setting.
     * <p>
     * For general testing and development, use TimeLock, unless there is a need
     * to test specific PlayStore or DeviceLock functionality.
     */
    private static final Edition EDITION = BuildConfig.KM_EDITION;// Edition.PlayStore;

    /**
     * TIMELOCK_EXPIRATION - The date when the TimeLock edition expires.  This
     * is only used if EDITION==Edition.TimeLock, and is ignored in all other cases.
     */
    private static final Date TIMELOCK_EXPIRATION = UTCDate(BuildConfig.KM_EXP_YEAR, BuildConfig.KM_EXP_MONTH, BuildConfig.KM_EXP_DAY);

    /**
     * VERSION_WARNING_MESSAGE - Warning message to display in Project Manager screen.
     * <p>
     * Set to null for no message.  This is useful for test releases.  For example:
     * <p>
     * VERSION_WARNING_MESSAGE = "PATCH VER. - NOT PASSED QA - NOT FOR RELEASE";
     * VERSION_WARNING_MESSAGE = "2.0.28.2292 BETA - INTERNAL USE ONLY";
     * VERSION_WARNING_MESSAGE = null;
     */
    public static final String VERSION_WARNING_MESSAGE = null;

    /**
     * IAB_ENABLE_SUBSCRIPTIONS - Controls whether subscriptions are enabled.
     * <p>
     * This only applies if EDITION==Edition.PlayStore; for other editions, it
     * is ignored.
     * <p>
     * This overrides SHOW_WATERMARK; the watermark will be shown or hidden
     * during export based on whether the user has an active subscription or
     * promo code.
     */
    private static final boolean IAB_ENABLE_SUBSCRIPTIONS = true;

    /**
     * SHOW_WATERMARK - Controls whether the watermark is included when
     * exporting.  If this is set to true, the watermark effect will be used;
     * if it is set to false, the watermark effect will not be used.
     * <p>
     * This is ignored if subscriptions are enabled, because in that case
     * the watermark is shown or hidden based on whether
     */
    public static final boolean SHOW_WATERMARK = true;

    /**
     * USE_CN_SERVER - Controls whether the default trace log server list
     * includes Chinese (api-cn) servers or not.  This affects all tracelog
     * server APIs.  If set to true, the list will include api-cn URLs.  If
     * set to false, the list will not include those URLs.
     */
    public static final boolean USE_CN_SERVER = false;

    // Allow direct support for individual SNS services to be enabled or disabled
    public static final boolean SNS_ENABLE_FACEBOOK_SUPPORT = true;
    public static final boolean SNS_ENABLE_GOOGLEPLUS_SUPPORT = true;
    public static final boolean SNS_ENABLE_YOUTUBE_SUPPORT = true;
    public static final boolean CLOUD_UPLOAD_ENABLE = true;
    public static final boolean SNS_ENABLE_YOUTUBE_INTENT = false;

    /**
     * USE_FLURRY - Controls whether Flurry is used or not.  If this is set to true,
     * be sure to set the appropriate FLURRY_API_KEY for the APK.
     */
    public static final boolean USE_FLURRY = EDITION == Edition.DeviceLock ? false : true;
//  public static final String FLURRY_API_KEY = /* PRO */ "M69Q2MMKT8QBP999NN42";
//  public static final String FLURRY_API_KEY = /* FREE */ "JP3SX25DKCXTXY7YV6GV";
//  public static final String FLURRY_API_KEY = /* OEM */ "BKB7KJVHMNYZJFG3TN6Z";
//  public static final String FLURRY_API_KEY = /* EVA */ "V6GF5FJBRNHN5JV8PM2N";
//	public static final String FLURRY_API_KEY = /* OPPO */ "2W29ZW4X2P2WFHTCGGC7";
//  public static final String FLURRY_API_KEY = /* HWA */ "XXNCHVQ5FKJ5C9G8QMKJ";
//  public static final String FLURRY_API_KEY = /* FIH */ "KS9QWJYRJMZ7278GHSK5";
//  public static final String FLURRY_API_KEY = /* CoolPad */ "CZ32VH6BXPNF58VC44RZ";
//    public static final String FLURRY_API_KEY = /* DEVELOP */ "MSTJGJWDCNQHQZPVC2XD";
//  public static final String FLURRY_API_KEY = null;
//    public static final String FLURRY_API_KEY = BuildConfig.KM_FLURRY_API_KEY;

    // Various UI features can be enabled or disabled here
    public static final boolean UI_ENABLE_CHANGE_FONT_BUTTON = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_VIDEO_SPEED_CONTROL = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_MUSIC_FROM_OTHER_THEMES = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_DIVIDED_VIDEOANDAUDIO = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_MEDIA_BROWSER_FAVORITES = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_OPTION_BAR_COLOR_EFFECT = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_PROJECT_SETTINGS = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_OPTION_BAR_CROP_ICON = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_VISUALCLIP_MIRROR = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_CLIPINFO_DIAGNOSTIC_POPUP = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_MEDIA_BROWSER_CLOUD_FOLDER = true;        // implementation complete; normally enabled
    public static final boolean UI_ENABLE_IMAGE_ROTATION = false;        // in development; enable for testing only
    public static final boolean UI_ENABLE_PROJECT_FOR_SNS = false;        // in development; enable for testing only
    public static final boolean UI_ENABLE_3_VERSION = true;
    public static final boolean USE_ASSET_PACKAGE_MANAGER = true;

    /**
     * LVL_FOR_PLAYSTORE - Controls whether LVL checking is done.
     * <p>
     * If true, check LVL and redirect to Google Play if there's no valid LVL authorization.
     * This is ignored unless EDITION==Edition.PlayStore.
     */
    public static final boolean LVL_FOR_PLAYSTORE = false;
//  static final String LVL_BASE64_PUBLIC_KEY = /*Pro*/ "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqaxIQunyHQXXGBgvCmCdVIXK6HNTKbICGY1+hKCi9wrQE5LFZPb/ewduvlkd7IqmsncRwTLt1vgU5SXzYS/x9o/gAxLzNpdEVjbrjRPPkoEda0RewQnAmeJ9AzBgVOgCY2V17GVYp60ITgyrOF1QQPwQsE61HvoqX6hI700f0C9yzdDpnjxRwxmZQYbffwiEGtJkHB1jHRRGLWiv1kMaS7ExY77W6FGrx+L26U4/U2dyXO/axMCq6I6n1tD3Po8Yk6UXIuPDo45wWBbdgpz5vZTvS3CuXHe0ZxxQLfxBvw5aV9EXnonFzLTX0dZxKauUresEIgvA5M+hyDj+LvA2OQIDAQAB";
//  static final String LVL_BASE64_PUBLIC_KEY = /*Free*/ "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAnocqe5df88ObegB1H8pnwBikfqWhOMDj2eFSNRlE6fEZB7pqxMpF0A15NmN7063A6XajUdNo7dnK1Gxj2akeAH/dmiG0vLx0rnx5K6Ql5q6MhaQYo5utE/BRELeu+HQCYnirbWxu1kS9rbVMBr3+WsvnRKIpATuJMappDGO3b5Ss2MGmOMZqJcNt9nayyU78DoqK86ZomRONbdOCYiPRBcQhlYCkxjRKTwDhtIUo5UzGooAfjQr86bILMdxi0qEe3AykV+Pc10dXU+59eP7L5labETM+z3NmelvAdGqDHuYPWfEnRJ9R7gOSRzdMfNIDfAhCX/1fdAMayu2liaRzVQIDAQAB";
//  static final String LVL_BASE64_PUBLIC_KEY = /*None*/ null;

    public static final boolean ENABLE_EXTENDED_DEVICE_SUPPORT = false;
    public static final VersionType VERSION_TYPE = VersionType.RC;
    public static final boolean CHECK_COLOR_FORMAT_ON_EXYNOS = true;
    public static final boolean AUTO_ADD_LOGO_CLIP = false;
    public static final boolean ALLOW_EDIT_LOGO_CLIP = false;
    //	public static final boolean ASK_ALLOW_MOBILE_DATA = ((EDITION==Edition.PlayStore)?false:true);
    public static final DataUsage DATA_USAGE = EDITION == Edition.PlayStore ? DataUsage.WIFI_AND_MOBILE : DataUsage.ASK_WIFI_MOBILE_NEVER;
    public static final boolean ASK_REPORT_USAGE = ((EDITION == Edition.DeviceLock) ? true : false);
    public static final boolean SIMULATE_SLOW_DOWNLOADS = false;

    public static final boolean USE_NATIVE_CAMERA_APP = false;

    public static final boolean ALLOW_HIERARCHY_VIEWER = BuildConfig.DEBUG;  // MUST BE FALSE FOR RELEASE BUILDS!!!!!!


    private static final boolean USE_UPDATE_SERVER = true;//((EDITION==Edition.TimeLock)?false:true);


    public static final String YOUTUBE_API_KEY = /* FREE */ "AIzaSyC56BwKH_dtuXy_f36IKVHF7M-ldGJOy-8";
//  public static final String YOUTUBE_API_KEY = /* PRO */ "";

    private static String sAppName = "KineMaster";

    private static final String keepFile = "keepfileFG2HJ6D4";

    public static final boolean USE_VASSET = BuildConfig.USE_VASSET;

    public static final String SO_FREFIX = BuildConfig.SO_FREFIX;

    public static final boolean SUPPORT_COLLAGE = BuildConfig.SUPPORT_COLLAGE;

    public static void setAppName(String name) {
        sAppName = name;
    }

    public static String getMarketId() {
//        return BuildConfig.KM_MARKET_ID;
        return "Google";
//		if( IS_PLAYSTORE_VERSION ) {
//			return "Google";
//		} else {
//			return null;
//		}
    }

    public static String keep() {
        return keepFile;
    }

    public static final boolean EXPERIMENTAL_CLIP_LENGTH = false;
    public static final String DEFAULT_THEME_ID = "com.nexstreaming.kinemaster.basic";
    public static final String DEFAULT_TRANSITION_ID = "none";
    public static final String DEFAULT_EFFECT_ID = null;

    public static final int IFRAME_GAP_WARNING = 4000;
    public static final int AUDIO_MIN_DURATION = 500;
    public static final int AUDIO_MIN_LOOP_DURATION = AUDIO_MIN_DURATION;
    public static final int VIDEO_MIN_WIDTH = 176;
    public static final int VIDEO_MIN_HEIGHY = 144;
    public static final int VIDEO_MAX_WIDTH = 1920;
    public static final int VIDEO_MAX_HEIGHT = 1088;
    public static final int VIDEO_MIN_DURATION = 1000;
    public static final int IMAGE_MIN_DURATION = 500;
    public static final int SPACE_BETWEEN_TRANSITIONS = 750;
    public static final int IMAGE_MAX_DURATION = 60000;
    public static final int ONPAUSE_STOP_TIMEOUT = 150;
    //	private static final float ASPECT_RATIO_5_3 = 5f/3f;	// For example, 800 x 480 (TCL).  About 1.666...
    private static final float ASPECT_RATIO_16_9 = 16f / 9f;    // For example, 720p.  About 1.777...
    public static final float PROJECT_ASPECT_RATIO = ASPECT_RATIO_16_9;
    public static final int CLIP_FX_MIN_DURATION = 500;
    //	public static final boolean USE_NATIVE_MEDIA_DB = NexEditorDeviceProfile.getDeviceProfile().getUseNativeMediaDB();
//	public static final boolean USE_CAMERA = NexEditorDeviceProfile.getDeviceProfile().getUseCamera();
//	public static final boolean VISUAL_MEDIA_FROM_KINEMASTER_FOLDER_ONLY = NexEditorDeviceProfile.getDeviceProfile().getVisualMediaFromKineMasterFolderOnly();
    public static final boolean ENABLE_PROJECT_PROTECTION = false; //NexEditorDeviceProfile.getDeviceProfile().getEnableProjectProtection();
    public static final boolean SHOW_VIDEO_THUMBNAILS = true;
    public static final boolean FONTS_FROM_NETWORK = true;

    public static final long POPUP_DOUBLETAP_SAFETY_DELAY = 350;

    // public static final int ACTIVITY_MEDIABROWSER = 1;  // not uesd
    public static final int ACTIVITY_RECORD_VIDEO = 2;
    public static final int ACTIVITY_RECORD_AUDIO = 5;
    public static final int ACTIVITY_TAKE_PICTURES = 4;
    //	public static final int ACTIVITY_IMAGECROPPING = 3;
    public static final int ACTIVITY_RECORD_VIDEO_KM = 20;
    public static final int HANDWRITING_UNDO_LEVELS = 10;

    // RequestCode for calling MediaBrowser
    public static final int REQUEST_AT_NEW_PROJECT_ACTIVITY = 100;
    public static final int REQUEST_AT_EDITOR_ACTIVITY = 200;
    public static final int REQUEST_AT_DRAWING_EDITOR_FRAGMENT = 300;

    //public static final boolean SUPPORT_AUDIO_TRACKS_UI = true;
    //public static final boolean SUPPORT_AUDIO_TRACKS_ENGINE = true;
    public static Bitmap audioClipIcon;
    public static Bitmap audioRecordingIcon;
    public static Bitmap videoNoSoundIcon;
    public static Bitmap videoHasTextIcon;

    public static final boolean USE_PAUSEBUTTON = true;

    public static final boolean IS_PLAYSTORE_VERSION = (EDITION == Edition.PlayStore);

    public static final String APP_PACKAGE_NAME = BuildConfig.APPLICATION_ID; //"com.nexstreaming.app.kinemasterfree";
    //	public static final String  APP_PACKAGE_NAME = "com.nexstreaming.app.kinemaster";
    public static final boolean APP_RATE_POPUP = (EDITION == Edition.PlayStore);

    public static final int kUnLimited = 0;
    public static final int kOverlayImageLimited = 1;
    public static final int kOverlayVideoLimited = 2;
    public static final int kOverlayAnimateLimited = 3;

    public static final int ApiLevel = BuildConfig.KM_API_LEVEL;

    public static final String PROJECT_NAME = BuildConfig.KM_PROJECT;

    public static final String PrefixTempFile = "nexTemp";

    public static final boolean isSupportedApi(int level) {
        if (EditorGlobal.ApiLevel < level) {
            return true;
        }
        return false;
    }

    public static void preProcessActionVideoCapture(Intent intent) {
        // This is a callback that can be used to customize the ACTION_VIDEO_CAPTURE intent
        // before it is sent.  For standard KineMaster releases, this function must do nothing.
        // For custom releases (for example, OEM releases) this can be customized to add or
        // change parameters in the intent before it is sent.

        // Enable this ONLY for Coolpad releases:
        //intent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, CamcorderProfile.QUALITY_720P);
    }

    /*protected*/
    public static int getUserData() {
        int userData = 0;
        if (EDITION == Edition.DeviceLock) {
            for (int i = 0; i < 4; i++) {
                userData <<= 8;
                userData |= 0x20;
            }
        } else if (EDITION == Edition.TimeLock || EDITION == Edition.PlayStore) {
            userData = 0x8456;
            for (int i = 0; i < 1; i++) {
                userData <<= 16;
                userData |= 0x2845;
            }
        }
        userData ^= 0x134A0000;
        return userData ^ 0x0000958E;
    }

    public static String getEffectiveModel() {
        return Build.MODEL;
    }

    public static boolean isIABSubscriptionEnabled() {
        return (EDITION == Edition.PlayStore && IAB_ENABLE_SUBSCRIPTIONS);
    }

//	private static UiStyle chooseUIStyle()
//	{
//		String[] TCLModels = {
//				"ALCATEL ONE TOUCH 8000A",
//				"ALCATEL ONE TOUCH 8000E",
//				"ALCATEL ONE TOUCH 8000X",
//				"ALCATEL ONE TOUCH 8000D",
//				"TCL Y710",
//				"TCL_S830"
//		};
//		
//		for( String s: TCLModels ) {
//			if(getEffectiveModel().equals(s))
//				return UiStyle.TCL;
//		}
//		
//		return UiStyle.NEX1;
//	}

    public static enum GeneralizedScreenSize {
        SMALL, NORMAL, LARGE, XLARGE, UNDEFINED
    }

    public static boolean screenIsXLarge(Context context) {
        return getGeneralizedScreenSize(context) == GeneralizedScreenSize.XLARGE;
    }

    public static boolean screenIsLarge(Context context) {
        return getGeneralizedScreenSize(context) == GeneralizedScreenSize.LARGE;
    }

    public static boolean screenIsSmall(Context context) {
        return getGeneralizedScreenSize(context) == GeneralizedScreenSize.SMALL;
    }

    public static GeneralizedScreenSize getGeneralizedScreenSize(Context context) {
        Configuration conf = context.getResources().getConfiguration();
        int layoutsize = conf.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK;
        switch (layoutsize) {
            case Configuration.SCREENLAYOUT_SIZE_LARGE:
                return GeneralizedScreenSize.LARGE;
            case Configuration.SCREENLAYOUT_SIZE_NORMAL:
                return GeneralizedScreenSize.NORMAL;
            case Configuration.SCREENLAYOUT_SIZE_SMALL:
                return GeneralizedScreenSize.SMALL;
            case Configuration.SCREENLAYOUT_SIZE_XLARGE:
                return GeneralizedScreenSize.XLARGE;
            case Configuration.SCREENLAYOUT_SIZE_UNDEFINED:
                return GeneralizedScreenSize.UNDEFINED;
            default:
                return null;
        }
    }

    public static boolean isTablet(Resources rs) {
//		if( (rs.getConfiguration().screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) == Configuration.SCREENLAYOUT_SIZE_XLARGE )
//			return true;
//		else
//			return false;
        return false;
    }

    public static boolean needHighResThumbs(Resources rs) {
        int screenLayoutSize = (rs.getConfiguration().screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK);
        if (screenLayoutSize == Configuration.SCREENLAYOUT_SIZE_XLARGE
                || screenLayoutSize == Configuration.SCREENLAYOUT_SIZE_LARGE) {
            return true;
        }
        return false;
    }


    public static boolean isLongTablet(Resources rs) {
//		int screenLayout = rs.getConfiguration().screenLayout;
//		if( (screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) == Configuration.SCREENLAYOUT_SIZE_XLARGE
//				&& (screenLayout & Configuration.SCREENLAYOUT_LONG_MASK) == Configuration.SCREENLAYOUT_LONG_YES )
//			return true;
//		else
//			return false;
        return false;
    }

    public static boolean isLargeTablet(Resources rs) {
        int screenLayout = rs.getConfiguration().screenLayout;
        if ((screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) == Configuration.SCREENLAYOUT_SIZE_LARGE)
            return true;
        else if ((screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) == Configuration.SCREENLAYOUT_SIZE_XLARGE)
            return true;
        else
            return false;
    }

    /**
     * Picxel -> DP
     **/
    public static int getPixelToDp(Context context, int pixel) {
        float dp = 0;
        try {
            DisplayMetrics metrics = context.getResources().getDisplayMetrics();
            dp = pixel / (metrics.densityDpi / 160f);
        } catch (Exception e) {

        }
        return (int) dp;
    }

    /**
     * DP -> Picxel
     **/
    public static int getDpToPixel(Context context, int DP) {
        float px = 0;
        try {
            px = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, DP, context.getResources().getDisplayMetrics());
        } catch (Exception e) {
            if (LL.E) Log.e("EditorGlobal", "getDpToPixel() Error");
        }
        return (int) px;
    }

    /**
     * DP -> Picxel
     **/
    public static int getDpToPixel(Context context, float DP) {
        float px = 0;
        try {
            px = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, DP, context.getResources().getDisplayMetrics());
        } catch (Exception e) {
            if (LL.E) Log.e("EditorGlobal", "getDpToPixel() Error");
        }
        return (int) px;
    }

    public static String fmtDuration(int ms) {
        if (ms >= 3600000) {
            return String.format(Locale.US, "%02d:%02d:%02d.%01d",
                    ms / 3600000,
                    (ms % 3600000) / 60000,
                    (ms % 60000) / 1000,
                    (ms % 1000) / 100);
        } else if (ms >= 60000) {
            return String.format(Locale.US, "%02d:%02d.%01d",
                    ms / 60000,
                    (ms % 60000) / 1000,
                    (ms % 1000) / 100);
        } else {
            return String.format(Locale.US, "%02d.%01d",
                    ms / 1000,
                    (ms % 1000) / 100);
        }
    }

    public static String fmtDuration_long(int t) {
        if (t > 3600000) {
            return String.format(Locale.US, "%d:%02d:%02d.%01d",
                    t / 3600000,
                    (t % 3600000) / 60000,
                    (t % 60000) / 1000,
                    (t % 1000) / 100);
        } else {
            return String.format(Locale.US, "%02d:%02d.%01d",
                    (t % 3600000) / 60000,
                    (t % 60000) / 1000,
                    (t % 1000) / 100);
        }
    }

    public static String fmtDurationLong(Resources res, int ms) {
        if (ms >= 3600000) {
            return String.format("%1$2d hours, %2$2d minutes, %3$2d.%4$01d seconds",
                    ms / 3600000,
                    (ms % 3600000) / 60000,
                    (ms % 60000) / 1000,
                    (ms % 1000) / 100);
        } else if (ms >= 60000) {
            return String.format("%1$2d minutes, %2$2d.%3$01d seconds",
                    ms / 60000,
                    (ms % 60000) / 1000,
                    (ms % 1000) / 100);
        } else {
            return String.format("%1$2d.%2$01d seconds",
                    ms / 1000,
                    (ms % 1000) / 100);
        }
    }

    public static String fmtDuration_export(Resources res, int ms) {
        if (ms >= 3600000) {
            return String.format("about %1$d:%2$02d:%3$02d remaining",
                    ms / 3600000,
                    (ms % 3600000) / 60000,
                    (ms % 60000) / 1000);
        } else if (ms >= 60000) {

            return String.format("about %1$d:%2$02d remaining",
                    (ms % 3600000) / 60000,
                    (ms % 60000) / 1000);
        } else {
            return String.format("about 0:%1$02d remaining",
                    ms / 1000);
        }
    }

    public static String fmtDurationHMS(long ms) {
        if (ms > 3600000) {
            return String.format(Locale.US, "%d:%02d:%02d",
                    ms / 3600000,
                    (ms % 3600000) / 60000,
                    (ms % 60000) / 1000);
        } else {
            return String.format(Locale.US, "%02d:%02d",
                    (ms % 3600000) / 60000,
                    (ms % 60000) / 1000);
        }
    }

    public static String formatFileSize(Context ctx, long size) {
        if (size < 1) {
            return "?";
        }
        if (size < 1024) {
            return size + " B";
        }
        int exp = (int) (Math.log10(size) / 3.0103);
        return String.format(ctx.getResources().getConfiguration().locale, "%.1f %sB", size / Math.pow(1024, exp), "KMGTPE".charAt(exp - 1));
    }

    public static File getStorageRoot() {
        return Environment.getExternalStorageDirectory();
//		return new File("/storage/sdcard1");
    }

    public static File getEditorRoot() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName);
    }

    public static File getEditorOldRoot() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + "NexEditor");
    }

//	public static File getThumbnailDirectory() {
//		return new File(getStorageRoot().getAbsolutePath() + File.separator + "KineMaster" + File.separator + "Thumbs");
//	}

//	public static File getTempDirectory() {
//		return new File(getStorageRoot().getAbsolutePath() + File.separator + "KineMaster" + File.separator + "Temp");
//	}
//	
//	public static File getThemeDirectory() {
//		return new File(getStorageRoot().getAbsolutePath() + File.separator + "KineMaster" + File.separator + "Themes");
//	}
//	
//	public static File getEffectDirectory() {
//		return new File(getStorageRoot().getAbsolutePath() + File.separator + "KineMaster" + File.separator + "Effects");
//	}

    public static File getLogDirectory() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "Log");
    }

    public static File getMonkeyDirectory() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "Monkey");
    }

    public static File getPluginDirectory() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "Plugins");
    }

    public static File getLutDirectory() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "Lut");
    }

    public static File getOverlayDirectory() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "Overlay");
    }

    public static File getPluginAssetDirectory() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "AssetPlugins");
    }

    //////////////////// This is for DesignPreview app.
    public static File getPluginAssetDirectory(Context ctx) {
        String cachePath = ctx.getFilesDir().getAbsolutePath() + File.separator + sAppName + File.separator + "AssetPlugins";
        File file = new File(cachePath);
        if (!file.exists()) {
            file.mkdirs();
        }
        return file;
    }

    public static File getPluginAssetEffectDirectory(Context ctx) {
        String cachePath = ctx.getFilesDir().getAbsolutePath() + File.separator + sAppName + File.separator + "AssetPlugins" + File.separator + "effects";
        File file = new File(cachePath);
        if (!file.exists()) {
            file.mkdirs();
        }
        return file;
        //return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "AssetPlugins"+ File.separator + "effects");
    }

    public static File getPluginAssetTranstionDirectory(Context ctx) {
        String cachePath = ctx.getFilesDir().getAbsolutePath() + File.separator + sAppName + File.separator + "AssetPlugins" + File.separator + "transitions";
        File file = new File(cachePath);
        if (!file.exists()) {
            file.mkdirs();
        }
        return file;
        //return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "AssetPlugins"+ File.separator + "transitions");
    }

    public static File getPluginAssetRenderItemDirectory(Context ctx) {
        //String cachePath = ctx.getFilesDir().getAbsolutePath() + File.separator + sAppName + File.separator + "AssetPlugins"+ File.separator + "renderitems";
        String cachePath = getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "AssetPlugins" + File.separator + "renderitems";
        File file = new File(cachePath);
        if (!file.exists()) {
            file.mkdirs();
        }
        return file;
        //return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "AssetPlugins"+ File.separator + "effects");
    }
////////////////////

    public static String getExportAndConvertPath(Context ctx) {
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.JELLY_BEAN_MR2) {
            return Environment.getExternalStorageDirectory().getAbsolutePath();
        } else {
            SharedPreferences sdCardPref = ctx.getSharedPreferences("sdcardloc", 0);
            return sdCardPref.getString("sdcardlocation", Environment.getExternalStorageDirectory().getAbsolutePath());
        }
    }

    public static File getConvertedDirectory(Context ctx) {

        if (ctx == null) {
            return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "Converted");
        }

        String sdCardPath = getExportAndConvertPath(ctx);
        if (LL.D) Log.d("EditorGlobal", "sdcardPath : " + sdCardPath);
        File convertDir = new File(sdCardPath + File.separator
                + sAppName + File.separator + "Converted");
        convertDir.mkdirs();
        return convertDir;
//		return new File(getStorageRoot().getAbsolutePath() + File.separator + "KineMaster" + File.separator + "Converted");
    }

    public static File getMediaDownloadDirectory(Context ctx) {
        if (ctx == null) {
            return new File(Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + sAppName + File.separator + "Download");
        }

        SharedPreferences sdCardPref = ctx.getSharedPreferences("sdcardloc", 0);
        String sdCardPath = sdCardPref.getString("sdcardlocation", Environment.getExternalStorageDirectory().getAbsolutePath());
        if (LL.D) Log.d("EditorGlobal", "sdcardPath : " + sdCardPath);
        File convertDir = new File(sdCardPath + File.separator
                + sAppName + File.separator + "Download");
        convertDir.mkdirs();
        return convertDir;
    }

//	public static File getTransitionDirectory() {
//		return new File(getStorageRoot().getAbsolutePath() + File.separator + "KineMaster" + File.separator + "Transitions");
//	}

    public static File getVoiceRecordingDirectory() {
        return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC);
    }

    public static File getPhotoDirectory() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "Photo");
    }

    public static File getProjectsDirectory() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "Projects");
    }

    public static File getOverlaysDirectory() {
        return new File(getStorageRoot().getAbsolutePath() + File.separator + sAppName + File.separator + "Projects" + File.separator + "Overlays");
    }


    public static File getGuideStateFile(Context ctx) {
        return new File(ctx.getFilesDir(), "guide.dat");
    }

    public static boolean isImage(String filename) {
        String lcfilename = filename.toLowerCase(Locale.US);
        for (String s : imageExtensions) {
            if (lcfilename.endsWith(s))
                return true;
        }
        return false;
    }

    public static boolean isVideo(String filename) {
        String lcfilename = filename.toLowerCase(Locale.US);
        for (String s : videoExtensions) {
            if (lcfilename.endsWith(s))
                return true;
        }
        return false;
    }

    public static boolean isAudio(String filename) {
        String lcfilename = filename.toLowerCase(Locale.US);
        for (String s : audioExtensions) {
            if (lcfilename.endsWith(s))
                return true;
        }
        return false;
    }

//	public static String musicPathForTheme( Resources resources, NexThemeManager.Theme theme ) {
//		String themeMusicSpec = theme.getMusicString();
//		if( themeMusicSpec!=null && themeMusicSpec.trim().length()>0 ) {
//			File f = null;
//			try {
//				f = theme.getAssociatedFile(resources.getAssets(), themeMusicSpec);
//			} catch (IOException e) {
//				e.printStackTrace();
//			}
//			if( f!=null )
//				return f.getAbsolutePath();
//		}
//		String themeId = theme.getId();
//		int resid = musicResourceForTheme(themeId);
//		File tempFile = new File(EditorGlobal.getTempDirectory(), resources.getResourceEntryName(resid) + ".tmp");
//		if( !tempFile.exists() ) {
//			InputStream in = resources.openRawResource(resid);
//			OutputStream out;
//			try {
//				out = new FileOutputStream(tempFile);
//				byte[] buffer = new byte[10240];
//				int readlen;
//				while( (readlen = in.read(buffer)) > -1 ) {
//					out.write(buffer, 0, readlen);
//				}
//				out.close();
//				in.close();
//			} catch (IOException e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//				return null;
//			}
//		}
//		return tempFile.getAbsolutePath();
//	}

//	public static int musicResourceForTheme( String themeId ) {
//		themeId = themeId.toLowerCase();
//		if( themeId.contains("stage") ) {
//			return R.raw.theme_music_onstage;
//		} else if( themeId.contains("love") ) {
//			return R.raw.theme_music_love;
//		} else if( themeId.contains("fun") ) {
//			return R.raw.theme_music_fun;
//		} else if( themeId.contains("energy") ) {
//			return R.raw.theme_music_energy;
//		} else if( themeId.contains("travel") ) {
//			return R.raw.theme_music_travel;
//		} else if( themeId.contains("serene") ) {
//			return R.raw.theme_music_serene;
//		} else if( themeId.contains("museum") ) {
//			return R.raw.theme_music_travel;
//		} else if( themeId.contains("snow") ) {
//			return R.raw.theme_music_serene;
//		} else if( themeId.contains("baby") ) {
//			return R.raw.theme_music_baby;
//		} else if( themeId.contains("nature") ) {
//			return R.raw.theme_music_nature;
//		} else {
//			return R.raw.theme_music_fun;
//		}
//	}

//	public static NexGuideManager findGuideManager( Activity activity ) {
//		NexGuideManager gm = null;
//		if( activity!=null && activity instanceof MainActivity ) {
//			gm = ((MainActivity)activity).getGuideManager();
//		} else if( activity!=null && activity instanceof NexGuideManagerWrapper ) {
//			gm = ((NexGuideManagerWrapper)activity).getGuideManager();
//		}
//		
//		if( gm==null ) {
//			gm = new NexGuideManager(activity,EditorGlobal.getGuideStateFile(activity));
//			gm.setBackgroundResource(R.drawable.guide_frame);
//			
//			final View contentView = activity.findViewById(android.R.id.content);
//			ViewTreeObserver vto = contentView.getViewTreeObserver();
//			final NexGuideManager final_gm = gm;
//			vto.addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
//				public void onGlobalLayout() {
//					contentView.getViewTreeObserver().removeGlobalOnLayoutListener(this);
//						if( final_gm!=null )
//							final_gm.setLayoutCompleted(true);
//				}
//			});
//
//		}
//		return gm;
//	}

    private static Date UTCDate(int year, int month, int day) {
        return UTCDate(year, month, day, 0, 0, 0);
    }

    private static Date UTCDate(int year, int month, int day, int hour, int minute, int second) {
        Calendar cal = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
        cal.set(year, month - 1, day, hour, minute, second);
        return cal.getTime();
    }

    static Date getXInfo() {
        if (EDITION == Edition.TimeLock) {
            return TIMELOCK_EXPIRATION;
        } else {
            return null;
        }
    }

    static boolean useUpdateServer() {
        return USE_UPDATE_SERVER;
    }

    public static class DiagnosticLogger {
        private final File m_target;

        @SuppressLint("SimpleDateFormat")
        public DiagnosticLogger(String name) {
            EditorGlobal.getLogDirectory().mkdirs();
            m_target = new File(EditorGlobal.getLogDirectory(), name + ".log");
            java.text.SimpleDateFormat sdf = new java.text.SimpleDateFormat("yyyy-MM-dd HH-mm-ss");
            Date now = new Date();
            log(null, "--- " + sdf.format(now) + " -----------------------------------------");
        }

        public void log(String tag, String text) {
            try {
                DataOutputStream os = new DataOutputStream(new FileOutputStream(m_target, true));
                if (tag == null) {
                    os.writeUTF((text == null ? "(null)" : text) + "\n");
                } else {
                    os.writeUTF(tag + ": " + (text == null ? "(null)" : text) + "\n");
                }
                os.close();
            } catch (IOException e) {
                if (LL.D) Log.d(tag, text);
            }
        }
    }

    public static String getProjectEffect(String effectId) {
        return encodeProjectEffect("com.nexstreaming.kinemaster.builtin.watermark." + effectId);
    }

    private static String encodeProjectEffect(String effectId) {

        if (effectId == null) {
            return null;
        }

        String mask = "Ax/VXn_zsAiwFi[CITPC;y2c}*0B'S0-7&QznQlMa6U9gmSoighZeC&@$-hAaXiN";
        StringBuilder encoded_id = new StringBuilder();

        int idlen = effectId.length();

        Random rand = new Random();

        char[] prefix = new char[8];
        for (int i = 0; i < prefix.length; i++) {
            prefix[i] = (char) (((rand.nextInt() & 0x0FFFFFFF) % 94) + ' ');
        }

        int offsa = prefix[0] ^ 0x3F;
        int offsb = prefix[1] ^ 0x81;
        int i;

        encoded_id.append(prefix);

//    	Log.d(LOG_TAG,"epe : prefix=" + new String(prefix) + " encoded_id=" + encoded_id);

        for (i = 0; i < idlen; i++) {
            int maska = mask.charAt((offsa + i) % 64);
            int maskb = prefix[(offsb + i) % 8];
            char cai = effectId.charAt(i);
            char c = (char) ((cai - 32 + maska - maskb + 188) % 94 + 32);
//	    	Log.d(LOG_TAG,"epe : i=" + i + " maska=" + maska + " maskb=" + maskb + " cai=" + ((int)cai) + " c=" + ((int)c));
            encoded_id.append(c);
        }

        return encoded_id.toString();

    }

    public static boolean checkAllowDataUse(SharedPreferences prefs, boolean wifi_connected) {
        switch (DATA_USAGE) {
            case ASK_WIFI_MOBILE_NEVER:
                String data_usage = prefs.getString("data_usage", "none");
                if (data_usage.equalsIgnoreCase("mobile")) {
                    return true;
                } else if (data_usage.equalsIgnoreCase("wifi")) {
                    return wifi_connected;
                } else {
                    return false;
                }
            case ASK_WIFI_OR_MOBILE:
                return wifi_connected || (prefs.getBoolean("mobile_updates", false));
            case NEVER:
                return false;
            case WIFI_AND_MOBILE:
                return true;
            case WIFI_ONLY:
                return wifi_connected;
        }

        return false;
    }

    public static boolean checkAllowDataUse(Context ctx) {
        return false; //TODO: yoon delete confirm
//		return checkAllowDataUse(PreferenceManager.getDefaultSharedPreferences(ctx),NetworkManager.isWifiConnected(ctx));
    }

    public static final Executor NETWORK_OPERATION_THREAD_POOL = Executors.newCachedThreadPool();

}


