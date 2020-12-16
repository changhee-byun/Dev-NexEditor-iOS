package com.nexstreaming.app.common.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

public class SupportLogger {
    private int[] log = new int[256];
    private int len = 0;
    private int cut = 0;
    private SupportLogger() {}
    public static final SupportLogger instance = new SupportLogger();
    private Event previousEvent = null;
    private int repeatCount = 0;

    public void onApplicationCreated(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        String prev_log = prefs.getString("supportLogger",null);
        int savedCut = prefs.getInt("supportLoggerCut",0);
        if( prev_log!=null ) {
            len = 0;
            cut = savedCut;
            for( String s: prev_log.split(",") ) {
                int n;
                try {
                    n = Integer.parseInt(s);
                } catch (NumberFormatException e) {
                    continue;
                }
                if( len < log.length )
                    log[len++] = n;
            }
            if( (savedCut<log.length/3 && len > log.length*2/3) || (savedCut > log.length*2/3) || savedCut > len ) {
                len = 0;
                cut = 0;
            }
        }
        Event.KineMaster_App_Started.log();
    }

    public void saveLog(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        StringBuilder sb = new StringBuilder();
        for( int i=0; i<len; i++ ) {
            if( i>0 )
                sb.append(',');
            sb.append(log[i]);
        }
        prefs.edit().putString("supportLogger",sb.toString()).putInt("supportLoggerCut",cut).apply();
    }

    public enum Event {
        Seek(1),
        SeekFail(2),
        SeekDeferred(3),
        Stop(4),
        BaseActivity_OnCreate(5),
        BaseActivity_OnDestroy(6),
        BaseActivity_OnPause(7),
        BaseActivity_OnStart(8),
        BaseActivity_OnResume(9),
        BaseActivity_OnStop(10),
        BaseActivity_OnRestart(11),
        Export_In(12),
        Export_Fail(13),
        Export_Start(14),
        Export_ProjectLoaded(15),
        Export_MediaTaskNotBusy(16),
        Export_DetectAndSetColorFormat(17),
        Export_GetEffectLibrary(18),
        Export_Prep(19),
        Export_SignalFail(20),
        Export_SignalCancel(21),
        Export_SignalSuccess(22),
        Export_RemoveOutOutputFile(23),
        Export_BeginSeek(24),
        Export_SeekComplete(25),
        Export_CompleteEventFromEngine(26),
        Export_ScanComplete(27),
        Activity_ShareQuality_OnCreate(28),
        Activity_ShareQuality_OnProjectInfoAvailable(29),
        Activity_ShareQuality_ExportQualityClick(30),
        BaseActivity_PAC(31),
        BaseActivity_DTL(32),
        ShareBaseActivity_Export(33),
        ShareBaseActivity_ExportFileExists(34),
        ShareBaseActivity_BeginGetPurchases(35),
        ShareBaseActivity_GetPurchasesComplete(36),
        ShareBaseActivity_MadeExportTask(37),
        ShareBaseActivity_MadeExportDialog(38),
        ExportDialogFragment_SetExportTask(39),
        ExportDialogFragment_OnCreate(40),
        ExportDialogFragment_OnCreateDialog(41),
        ExportDialogFragment_ShowCompletionDialog(42),
        ExportDialogFragment_ShowFailureDialog(43),
        ExportDialogFragment_OnFail(44),
        ExportDialogFragment_OnExportComplete(45),
        Export_Progress(46),
        IW_Subs_Success(47),
        IW_Subs_Failure(48),
        IW_Once_Success(49),
        IW_Once_Failure(50),
        IH_Service_Disconnected(51),
        IH_ResponseCode(52),
        IH_RemoteException(53),
        IH_GetSkuFail(54),
        ShareBaseActivity_APC(55),
        ShareBaseActivity_APCFail(56),
        KineMaster_App_Started(57),
        Export_PossibleFailItem(58),
        ;

        public final int code;
        Event(int code) {
            this.code = code;
        }
        public void log(int... params) {
            SupportLogger.instance.log(this,params);
        }
    }

    public enum Abbreviation {
        KineMasterBaseActivity(1),
        ProjectEditActivity(2),
        ShareBaseActivity(3),
        ShareQualityActivity(4),
        ProjectGalleryActivity(5),
        SettingsActivity(6),
        HelpActivity(7),
        FontBrowserActivity(8),
        NewProjectActivity(9),
        ShareActivity(10),
        ShareSubscriptionActivity(11),
        FullScreenInputActivity(12),
        PreviewPlayActivity(13),
        CamcorderPreviewActivity(14),
        SupportAppsActivity(15),
        ShareSNSActivity(16),
        ExpiredActivity(17),
        BailActivity(18),
        CamcorderActivity(19),
        NexSNSUploadActivity(20),
        NexCloudUploadActivity(21),
        KMMediaStoreActivity(22),
        RelaunchActivity(23)
        ;
        public final int code;
        Abbreviation(int code) {
            this.code = code;
        }
        public static Abbreviation forString(String s) {
            for( Abbreviation abbr: values() ) {
                if( abbr.name().equals(s) )
                    return abbr;
            }
            return null;
        }
        public static Abbreviation forStringIgnoreCase(String s) {
            for( Abbreviation abbr: values() ) {
                if( abbr.name().equalsIgnoreCase(s) )
                    return abbr;
            }
            return null;
        }
    }

    public synchronized void log( Event event, int... params ) {
        if( event==previousEvent && event==Event.Seek ) {
            repeatCount++;
            return;
        }
        if( len + params.length + 4 > log.length ) {
            System.arraycopy(log,cut,log,0,len-cut);
            len -= cut;
            cut = 0;
        }
        if( repeatCount > 0 ) {
            if( repeatCount==1 ) {
                log[len++] = -2;
            } else {
                log[len++] = -1;
                log[len++] = repeatCount;
            }
            repeatCount = 0;
        }
        previousEvent = event;

        log[len++] = event.code;
        log[len++] = params.length;
        for( int i=0; i<params.length; i++ ) {
            log[len++] = params[i];
        }
        if( len > log.length/2 && cut < 1 ) {
            cut = len;
        }
    }

    public String getSupportLog() {
        StringBuilder sb = new StringBuilder();

        sb.append("SL");
        for( int i=0; i<len; i++ ) {
            int n = log[i];
            if( n==0 ) {
                sb.append('Z');
            } else if( n==-1 ) {
                sb.append('Y');
            } else if( n==-2 ) {
                sb.append('X');
            } else if( n==-3 ) {
                sb.append('W');
            } else if( n>=1 && n <=11 ) {
                sb.append((char)('A'+n-1));
            } else {
                if( sb.length()>0 ) {
                    char lastChar = sb.charAt(sb.length()-1);
                    if( lastChar >= '0' && lastChar <='9') {
                        sb.append(',');
                    }
                }
                sb.append(n);
            }
        }
        sb.append("S");

        return sb.toString().replace("BAB1","!").replace("BAB","*").replace("AE","_").replace("__","@");

    }
}
