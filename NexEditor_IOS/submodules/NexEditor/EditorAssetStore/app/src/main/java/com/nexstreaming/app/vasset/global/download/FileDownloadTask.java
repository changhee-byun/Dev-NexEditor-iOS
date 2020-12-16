package com.nexstreaming.app.vasset.global.download;

import android.os.AsyncTask;
import android.util.Log;

import com.nexstreaming.app.assetlibrary.utils.FreeSpaceChecker;
import com.nexstreaming.app.vasset.global.LL;
import com.nexstreaming.app.vasset.global.R;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.net.HttpURLConnection;
import java.net.SocketException;
import java.net.URI;

/**
 * Created by ojin.kwon on 2016-08-22.
 */
public class FileDownloadTask extends AsyncTask<Void , FileDownloadTask.DownloadProgress, DownloadError> {

    private static final String TAG = "FileDownloadTask";

    private static final int CONNECT_TIMEOUT_MILLISECONDS = 15000;
    private static final int READ_TIMEOUT_MILLISECONDS = 60000;

    public interface OnFileDownloadListener{
        void onStart(FileDownloadTask task, DownloadInfo info);
        void onProgress(DownloadInfo info, long current, long max, int percent);
        void onFinish(DownloadInfo info);
        void onCancel(DownloadInfo info);
        void onError(DownloadInfo info, DownloadError error);
    }

    private static final long PROGRESS_UPDATE_DURATION = 100L;

    private DownloadProgress mProgress = null;

    private final DownloadInfo mDownloadInfo;
    private final OnFileDownloadListener mOnFileDownloadListener;

    private long mLastProgressUpdate = 0;

    public FileDownloadTask(DownloadInfo info, OnFileDownloadListener ll){
        mDownloadInfo = info;
        mOnFileDownloadListener = ll;
        mProgress = new DownloadProgress(0, 0, DownloadError.DOWNLOAD_ERROR_PENDING);
    }

    public DownloadProgress getDownloadProgress() {
        return mProgress;
    }

    @Override
    protected void onCancelled(DownloadError downloadError) {
        super.onCancelled(downloadError);
        if(LL.D) Log.d(TAG, "onCancelled() called with: " + "downloadError = [" + downloadError + "]");
        if(mDownloadInfo != null){
            File tempFile = new File(mDownloadInfo.getDestinationPath() + ".download");
            if(tempFile.exists()){
                if(LL.D) Log.d(TAG, "onCancelled: try deleting temp file : " + mDownloadInfo);
                tempFile.delete();
            }
        }
        if(mOnFileDownloadListener != null){
            if(LL.D) Log.d(TAG, "download Cancel completed");
            mOnFileDownloadListener.onCancel(mDownloadInfo);
        }
    }

    @Override
    protected void onPreExecute() {
        super.onPreExecute();
        if(mOnFileDownloadListener != null){
            mOnFileDownloadListener.onStart(this, mDownloadInfo);
        }
    }

    private DownloadError downloadFile(DownloadInfo info) {
        /**
         * Chekcking cancel
         */
        if(getDownloadProgress().error == DownloadError.DOWNLOAD_ERROR_CANCEL){
            publishProgress(new DownloadProgress(0, 0, DownloadError.DOWNLOAD_ERROR_CANCEL));
            return new DownloadError(DownloadError.DOWNLOAD_ERROR_CANCEL, R.string.asset_download_failed, new Exception("User sent cancel signal"));
        }

        /**
         * Connection
         */
        URI uri = info.getDownloadURI();
        if(LL.D) Log.d(TAG, "[HttpURLConnection] try URI: " + uri);
        HttpURLConnection connection = null;
        try {
            connection = (HttpURLConnection)(uri.toURL().openConnection());
            if(LL.D) Log.d(TAG,"[HttpURLConnection] made connection object");
            File tempFile = new File(info.getDestinationPath() + ".download");

            /**
             * File not exists, create new file.
             */
            if(!tempFile.exists()){
                tempFile.createNewFile();
            }

            /**
             * checking offset of file for downloading continue
             */
            final RandomAccessFile out = new RandomAccessFile(tempFile, "rw");
            long offset = out.length();
            out.getChannel().force(true);
            out.seek(offset);

            connection.setConnectTimeout(CONNECT_TIMEOUT_MILLISECONDS);
            connection.setReadTimeout(READ_TIMEOUT_MILLISECONDS);
            connection.setRequestProperty("Accept-Ranges", "bytes");
            connection.setRequestProperty("Range", "bytes=" + offset + "-");

            if(LL.D) Log.d(TAG,"[HttpURLConnection] configured connection object");

            int code = connection.getResponseCode();
            int remainSize = connection.getContentLength();
            long lengthOfFile = remainSize + offset;

            if(LL.D) Log.d(TAG, "downloadFile offset = " + offset);
            if(LL.D) Log.d(TAG, "downloadFile remainSize = " + remainSize);
            if(LL.D) Log.d(TAG, "downloadFile lengthOfFile = " + lengthOfFile);

            mDownloadInfo.setDownloadContentSize(lengthOfFile);

            if(LL.D) Log.d(TAG,"[HttpURLConnection] response code: " + code);
            /**
             * checking disk free space
             */
            long freeSpace = FreeSpaceChecker.getFreeSpace(tempFile);
            if( freeSpace < remainSize + (1024*1024*10) ) {
                if(out != null)
                    out.close();
                if(connection != null)
                    connection.disconnect();
                publishProgress(new DownloadProgress(0, lengthOfFile, DownloadError.DOWNLOAD_ERROR_NOT_ENOUGH_SPACE));
                return new DownloadError(DownloadError.DOWNLOAD_ERROR_NOT_ENOUGH_SPACE, R.string.fail_enospc);
            }

            /**
             * Connection stream
             */
            final InputStream is = new BufferedInputStream(connection.getInputStream());
            String uriHost = uri.getHost();
            String connHost = connection.getURL().getHost();
            if(LL.D) Log.d(TAG,"[HttpURLConnection] host : " + uriHost + " : " + connHost);

            /**
             * checking download server host.
             */
            if (!uriHost.equals(connHost)) {
                if(LL.D) Log.d(TAG,"[HttpURLConnection] connection redirected : " + uriHost + " : " + connHost);
                if(out != null)
                    out.close();
                if(is != null)
                    is.close();
                if(connection != null)
                    connection.disconnect();

                publishProgress(new DownloadProgress(0, lengthOfFile, DownloadError.DOWNLOAD_ERROR_SERVER_HOST_MISSMATCH));
                return new DownloadError(DownloadError.DOWNLOAD_ERROR_SERVER_HOST_MISSMATCH, R.string.asset_download_failed);
            }

            if(remainSize < 0 || remainSize == offset) {
                if(out != null)
                    out.close();
                if(is != null)
                    is.close();
                if(connection != null)
                    connection.disconnect();

                tempFile.delete();

                publishProgress(new DownloadProgress(0, lengthOfFile, DownloadError.DOWNLOAD_ERROR_CONTENT_SIZE_MISSING));
                return new DownloadError(DownloadError.DOWNLOAD_ERROR_CONTENT_SIZE_MISSING, R.string.asset_download_failed);
            }

            byte[] buffer = new byte[8192];
            int len;
            int completed = 0;
            long noti = System.nanoTime();
            try{
                while( (len=is.read(buffer)) != -1) {
                    out.write(buffer, 0, len);
                    completed += len;
                    long t = System.nanoTime();
                    if( t-noti > 16000000 || completed >= remainSize ) {
                        noti = t;
                        publishProgress(new DownloadProgress(completed + offset, lengthOfFile, DownloadError.DOWNLOAD_ERROR_RUNNING));
                    }
                    if( completed >= remainSize ) {
                        break;
                    }
                    if(isCancelled()){
                        if(LL.D) Log.d(TAG, "cancel download while downloading file");
                        publishProgress(new DownloadProgress(completed + offset, lengthOfFile, DownloadError.DOWNLOAD_ERROR_CANCEL));
                        return new DownloadError(DownloadError.DOWNLOAD_ERROR_CANCEL, R.string.asset_download_failed);
                    }
                }
            }catch (SocketException e){
                if(out != null) {
                    out.close();
                }
                if(is != null) {
                    is.close();
                }
                if(connection != null) {
                    connection.disconnect();
                }
                publishProgress(new DownloadProgress(completed + offset, lengthOfFile, DownloadError.DOWNLOAD_ERROR_SERVER_CONNECTION_ERROR));
                return new DownloadError(DownloadError.DOWNLOAD_ERROR_SERVER_CONNECTION_ERROR, R.string.asset_download_failed,  e);
            }
            info.getDestinationFile().delete();
            tempFile.renameTo(info.getDestinationFile());
            publishProgress(new DownloadProgress(completed + offset, lengthOfFile, DownloadError.DOWNLOAD_ERROR_SUCCESS));
        } catch (IOException e) {
            if(connection != null)
                connection.disconnect();
            publishProgress(new DownloadProgress(0, 0, DownloadError.DOWNLOAD_ERROR_DOWNLOAD_IO_EXCEPTION));
            return new DownloadError(DownloadError.DOWNLOAD_ERROR_DOWNLOAD_IO_EXCEPTION, R.string.asset_download_failed, e);
        }
        if(connection != null)
            connection.disconnect();

        return new DownloadError(DownloadError.DOWNLOAD_ERROR_SUCCESS, R.string.asset_download_failed);
    }

    @Override
    protected DownloadError doInBackground(Void... params) {
        if(mDownloadInfo != null){
            if(mDownloadInfo.isExists()) {
                File file = mDownloadInfo.getDestinationFile();
                publishProgress(new DownloadProgress(file.length(), file.length(), DownloadError.DOWNLOAD_ERROR_SUCCESS));
                return new DownloadError(DownloadError.DOWNLOAD_ERROR_SUCCESS, R.string.done);
            } else {
                return downloadFile(mDownloadInfo);
            }
        }else{
            return new DownloadError(DownloadError.DOWNLOAD_ERROR_PARAMETER_ERROR, R.string.asset_download_failed);
        }
    }

    @Override
    protected void onPostExecute(DownloadError result) {
        super.onPostExecute(result);
        if(LL.D) Log.d(TAG, "onPostExecute() called with: " + "result = [" + result + "]");
        if(mOnFileDownloadListener != null){
            switch (result.errorCode){
                case DownloadError.DOWNLOAD_ERROR_SUCCESS:
                    mOnFileDownloadListener.onFinish(mDownloadInfo);
                    return;
                case DownloadError.DOWNLOAD_ERROR_CANCEL:
                    mOnFileDownloadListener.onCancel(mDownloadInfo);
                    return;
                default:
                    mOnFileDownloadListener.onError(mDownloadInfo, result);
                    return;

            }
        }
    }

    @Override
    protected void onProgressUpdate(DownloadProgress... values) {
        super.onProgressUpdate(values);
        if( values.length>0 ) {
            mProgress = values[0];
            if (mProgress != null) {
                if(mOnFileDownloadListener != null){
                    switch (mProgress.error){
                        case DownloadError.DOWNLOAD_ERROR_RUNNING:
                        case DownloadError.DOWNLOAD_ERROR_SUCCESS:
                            if(mLastProgressUpdate + PROGRESS_UPDATE_DURATION < System.currentTimeMillis()){
                                if (LL.D) Log.d(TAG, "onProgressUpdate : " + mProgress.completedBytes + " / " + mProgress.totalBytes);
                                mLastProgressUpdate = System.currentTimeMillis();
                                mOnFileDownloadListener.onProgress(mDownloadInfo, mProgress.completedBytes, mProgress.totalBytes, mProgress.getPercents());
                            }
                            break;
                    }
                }
            }
        }
    }

    public static final class DownloadProgress{
        public final long completedBytes;
        public final long totalBytes;
        public final int error;

        public DownloadProgress(long completedBytes, long totalBytes, int error) {
            this.completedBytes = completedBytes;
            this.totalBytes = totalBytes;
            this.error = error;
        }

        public boolean isCompleted(){
            return error == DownloadError.DOWNLOAD_ERROR_SUCCESS && completedBytes == totalBytes;
        }

        public int getPercents(){
            if(totalBytes <= 0){
                return 0;
            }
            return (int) (((double)completedBytes / (double)totalBytes) * 100);
        }

        @Override
        public String toString() {
            return "DownloadStatus{" +
                    "completedBytes=" + completedBytes +
                    ", totalBytes=" + totalBytes +
                    ", error=" + error +
                    '}';
        }
    }
}