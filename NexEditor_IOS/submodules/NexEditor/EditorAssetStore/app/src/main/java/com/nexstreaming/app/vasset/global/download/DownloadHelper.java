package com.nexstreaming.app.vasset.global.download;

import android.app.NotificationManager;
import android.content.Context;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.app.vasset.global.LL;
import com.nexstreaming.app.vasset.global.R;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Created by ojin.kwon on 2016-09-01.
 */
public class DownloadHelper {

    private static final String TAG = "DownloadHelper";

    private static final ExecutorService sDownloadExecutor = Executors.newFixedThreadPool(4);
    private static final Map<String, FileDownloadTask> sActiveFileDownloads = new HashMap<String, FileDownloadTask>();

    private static final Map<String, ResultTask<DownloadInfo>> sDownloadingTaskMap = new HashMap<>();
    private static final Map<String, NotificationCompat.Builder> sNotificationMap = new HashMap<>();


    private Context mContext;

    private NotificationManager mNotificationManager;


    private static DownloadHelper sInstance;

    public static DownloadHelper getInstance(Context context){
        if(sInstance == null){
            sInstance = new DownloadHelper(context.getApplicationContext());
        }
        return sInstance;
    }

    private DownloadHelper(Context context){
        mContext = context;
        mNotificationManager = (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);
    }

    public ResultTask<DownloadInfo> getDownloadingTask(String id){
        return sDownloadingTaskMap.get(id);
    }

    public ResultTask<DownloadInfo> download(DownloadInfo downloadInfo){
        ResultTask resultTask = null;
        if(downloadInfo != null){
            if(sDownloadingTaskMap.get(downloadInfo.getId()) != null){
                resultTask = sDownloadingTaskMap.get(downloadInfo.getId());
            }else{
                if(downloadInfo.getDestinationFile().exists()){
                    downloadInfo.getDestinationFile().delete();
                }
                resultTask = new ResultTask();
                FileDownloadTask task = sActiveFileDownloads.get(downloadInfo.getDestinationPath());
                if(task != null){
                    if(task.getDownloadProgress().error == DownloadError.DOWNLOAD_ERROR_SUCCESS){
                        sActiveFileDownloads.remove(downloadInfo.getDestinationPath());
                    }
                }
                task = new FileDownloadTask(downloadInfo, onFileDownloadListener);
                task.executeOnExecutor(sDownloadExecutor);
                sDownloadingTaskMap.put(downloadInfo.getId(), resultTask);
                sActiveFileDownloads.put(downloadInfo.getDestinationPath(), task);
            }
        }else{
            throw new IllegalArgumentException("downloadInfo must be not null");
        }
        return resultTask;
    }

    private void showDownloadNotification(final DownloadInfo downloadInfo, final int progress){
        if(downloadInfo != null){
            int id = 0;
            try{
                id = Integer.parseInt(downloadInfo.getId());
            }catch (NumberFormatException e){
                if(LL.W) Log.w(TAG, "showDownloadStartNotification: id parse error", e);
                return;
            }
            NotificationCompat.Builder builder = sNotificationMap.get(downloadInfo.getId());
            if(builder == null){
                    builder = new NotificationCompat.Builder(mContext);
                    sNotificationMap.put(downloadInfo.getId(), builder);
                    builder.setContentTitle(mContext.getString(R.string.download))
                            .setPriority(NotificationCompat.PRIORITY_MAX)
                            .setContentText(downloadInfo.getName())
                            .setOngoing(true)
                            .setOnlyAlertOnce(true)
                            .setProgress(100, progress, false)
                            .setSmallIcon(android.R.drawable.stat_sys_download);

            }else{
                builder.setProgress(100, progress, false);
            }
            mNotificationManager.notify(id, builder.build());
        }
    }

    private void cancelDownloadNotification(DownloadInfo downloadInfo){
        if(downloadInfo != null){
            int id = 0;
            try{
                id = Integer.parseInt(downloadInfo.getId());
            }catch (NumberFormatException e){
                if(LL.W) Log.w(TAG, "removeDownloadNotification: id parse error", e);
                return;
            }
            mNotificationManager.cancel(id);
            sNotificationMap.remove(downloadInfo.getId());
        }
    }



    public void cancelAllDownload(){
        for(String key : sActiveFileDownloads.keySet()){
            FileDownloadTask task =  sActiveFileDownloads.get(key);
            task.cancel(true);
        }
    }

    public void cancelDownload(DownloadInfo info){
        if(LL.D) Log.d(TAG, "cancelDownload() called with: info = [" + info + "]");
        FileDownloadTask task = sActiveFileDownloads.get(info.getDestinationPath());
        if(task != null){
            task.cancel(true);
            sActiveFileDownloads.remove(info.getDestinationPath());
        }
    }

    public int getDownloadPercent(String destinationPath){
        int percent = 0;
        if(destinationPath != null && sActiveFileDownloads != null){
            FileDownloadTask task = sActiveFileDownloads.get(destinationPath);
            if(task != null && task.getDownloadProgress() != null){
                percent = task.getDownloadProgress().getPercents();
            }
        }
        return percent;
    }

    public boolean isDownloading(String destinationPath){
        boolean b = false;
        if(destinationPath != null && sActiveFileDownloads != null){
            FileDownloadTask task = sActiveFileDownloads.get(destinationPath);
            if(task != null && task.getDownloadProgress() != null){
                b = task.getDownloadProgress().error == DownloadError.DOWNLOAD_ERROR_RUNNING
                        || task.getDownloadProgress().error == DownloadError.DOWNLOAD_ERROR_PENDING;
            }
        }
        return b;
    }

    private FileDownloadTask.OnFileDownloadListener onFileDownloadListener = new FileDownloadTask.OnFileDownloadListener() {
        @Override
        public void onStart(FileDownloadTask task, DownloadInfo info) {
            if(LL.D) Log.d(TAG, "start Download () called with: " +  info);
        }

        @Override
        public void onProgress(DownloadInfo info, long current, long max, int percent) {
            if(LL.D) Log.d(TAG, "onProgress () called with: " +  info + "[" + percent + "]");
            if(sDownloadingTaskMap.get(info.getId()) != null){
                sDownloadingTaskMap.get(info.getId()).setProgress(percent, 100);
            }
            showDownloadNotification(info, percent);
        }

        @Override
        public void onFinish(DownloadInfo info) {
            if(LL.D) Log.d(TAG, "download Finish() called with: " +  info);
            if(sDownloadingTaskMap.get(info.getId()) != null){
                sDownloadingTaskMap.get(info.getId()).sendResult(info);
                sDownloadingTaskMap.remove(info.getId());
                sActiveFileDownloads.remove(info.getDestinationPath());
            }
            cancelDownloadNotification(info);
        }

        @Override
        public void onCancel(DownloadInfo info) {
            if(LL.D) Log.d(TAG, "download cancel () called with: " +  info);
            if(sDownloadingTaskMap.get(info.getId()) != null){
                sDownloadingTaskMap.get(info.getId()).signalEvent(Task.Event.CANCEL);
                sDownloadingTaskMap.remove(info.getId());
                sActiveFileDownloads.remove(info.getDestinationPath());
            }
            cancelDownloadNotification(info);
        }

        @Override
        public void onError(DownloadInfo info, DownloadError error) {
            if(LL.D) Log.d(TAG, "download error () called with: " +  error);
            if(sDownloadingTaskMap.get(info.getId()) != null){
                sDownloadingTaskMap.get(info.getId()).sendFailure(error);
                sDownloadingTaskMap.remove(info.getId());
            }
            cancelDownloadNotification(info);
        }
    };
}
