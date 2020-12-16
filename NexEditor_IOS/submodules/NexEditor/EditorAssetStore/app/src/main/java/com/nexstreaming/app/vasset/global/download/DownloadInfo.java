package com.nexstreaming.app.vasset.global.download;

import android.os.Parcel;
import android.os.Parcelable;

import java.io.File;
import java.net.URI;

/**
 * Created by ojin.kwon on 2016-08-22.
 */
public class DownloadInfo implements Parcelable{

    private final String id;
    private final String name;
    private final String imageUrl;
    private final String downloadUrl;
    private final String destinationPath;
    private long downloadContentSize;
    private final long createDate;

    public DownloadInfo(String id, String name, String imageUrl,  String downloadUrl, String destinationPath, long downloadContentSize){
        this.id = id;
        this.name = name;
        this.imageUrl = imageUrl;
        this.downloadUrl = downloadUrl;
        this.destinationPath = destinationPath;
        this.downloadContentSize = downloadContentSize;
        this.createDate = System.currentTimeMillis();
    }

    protected DownloadInfo(Parcel in) {
        id = in.readString();
        name = in.readString();
        imageUrl = in.readString();
        downloadUrl = in.readString();
        destinationPath = in.readString();
        downloadContentSize = in.readLong();
        createDate = in.readLong();
    }

    public static final Creator<DownloadInfo> CREATOR = new Creator<DownloadInfo>() {
        @Override
        public DownloadInfo createFromParcel(Parcel in) {
            return new DownloadInfo(in);
        }

        @Override
        public DownloadInfo[] newArray(int size) {
            return new DownloadInfo[size];
        }
    };

    public String getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public String getImageUrl() {
        return imageUrl;
    }

    public String getDownloadUrl() {
        return downloadUrl;
    }

    public String getDestinationPath() {
        return destinationPath;
    }

    public long getDownloadContentSize() {
        return downloadContentSize;
    }

    public void setDownloadContentSize(long downloadContentSize) {
        this.downloadContentSize = downloadContentSize;
    }

    public long getCreateDate() {
        return createDate;
    }

    public URI getDownloadURI(){
        return URI.create(downloadUrl);
    }


    public File getDestinationFile(){
        return new File(destinationPath);
    }

    public boolean isExists(){
        boolean b = false;
        File file = getDestinationFile();
        if(file != null && file.exists()){
            b = file.length() == downloadContentSize;
        }
        return b;
    }

    @Override
    public boolean equals(Object o) {
        if(o instanceof DownloadInfo){
            return this.downloadContentSize == ((DownloadInfo) o).downloadContentSize && this.downloadUrl.equals(((DownloadInfo) o).downloadUrl);
        }
        return super.equals(o);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(id);
        dest.writeString(name);
        dest.writeString(imageUrl);
        dest.writeString(downloadUrl);
        dest.writeString(destinationPath);
        dest.writeLong(downloadContentSize);
        dest.writeLong(createDate);
    }

    public DownloadInfo readFromParcel(Parcel p){
        return new DownloadInfo(p);
    }

    @Override
    public String toString() {
        return "{" +
                "id='" + id + '\'' +
                ", name='" + name + '\'' +
                ", destinationPath='" + destinationPath + '\'' +
                ", createDate=" + createDate +
                '}';
    }
}
