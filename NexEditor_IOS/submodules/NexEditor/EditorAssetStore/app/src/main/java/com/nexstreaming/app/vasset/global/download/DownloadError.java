package com.nexstreaming.app.vasset.global.download;

import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;

import com.nexstreaming.app.general.task.Task;

/**
 * Created by ojin.kwon on 2016-08-22.
 */
public class DownloadError implements Task.TaskError, Parcelable {


    @Override
    public Exception getException() {
        return new Exception(devMessage == null ? "DownloadError has no any dev message" : devMessage );
    }

    @Override
    public String getMessage() {
        return message;
    }

    @Override
    public String getLocalizedMessage(Context context) {
        return context.getString(resId);
    }

    public static final int DOWNLOAD_ERROR_PENDING = 0x00;
    public static final int DOWNLOAD_ERROR_SUCCESS = 0x01;
    public static final int DOWNLOAD_ERROR_RUNNING = 0x10;
    public static final int DOWNLOAD_ERROR_CANCEL = 0x11;
    public static final int DOWNLOAD_ERROR_NOT_ENOUGH_SPACE = 0x20;
    public static final int DOWNLOAD_ERROR_DOWNLOAD_IO_EXCEPTION = 0x21;
    public static final int DOWNLOAD_ERROR_CONTENT_SIZE_MISSING = 0x22;
    public static final int DOWNLOAD_ERROR_SERVER_CONNECTION_ERROR = 0x23;
    public static final int DOWNLOAD_ERROR_SERVER_HOST_MISSMATCH = 0x24;
    public static final int DOWNLOAD_ERROR_SERVICE_CONNECTION_FAIL = 0x30;
    public static final int DOWNLOAD_ERROR_SERVICE_CALLBACK = 0x31;
    public static final int DOWNLOAD_ERROR_PARAMETER_ERROR = 0x40;

    public final int errorCode;
    public final int resId;
    public final String message;
    public final String devMessage;

    public DownloadError(int errorCode, int resId, Exception e){
        this.errorCode = errorCode;
        this.resId = resId;
        this.message = e.getMessage();
        this.devMessage = e.getMessage() + (e.getCause() != null ? "\n" + e.getCause().getMessage() : "");
    }

    public DownloadError(int errorCode, int resId, String message){
        this.errorCode = errorCode;
        this.resId = resId;
        this.message = message;
        this.devMessage = null;
    }

    public DownloadError(int errorCode, int resId){
        this.errorCode = errorCode;
        this.resId = resId;
        message = null;
        devMessage = null;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(errorCode);
        dest.writeInt(resId);
        dest.writeString(message);
        dest.writeString(devMessage);
    }

    protected DownloadError(Parcel in) {
        errorCode = in.readInt();
        resId = in.readInt();
        message = in.readString();
        devMessage = in.readString();
    }

    public static final Creator<DownloadError> CREATOR = new Creator<DownloadError>() {
        @Override
        public DownloadError createFromParcel(Parcel in) {
            return new DownloadError(in);
        }

        @Override
        public DownloadError[] newArray(int size) {
            return new DownloadError[size];
        }
    };

    public DownloadError readFromParcel(Parcel p){
        return new DownloadError(p);
    }

    @Override
    public String toString() {
        return "{errorCode=" + errorCode +
                ", message='" + message +
                "}";
    }
}
