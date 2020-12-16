package com.nexstreaming.editordemo.utility;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.ArrayList;

/**
 * Created by mj.kong on 2015-03-27.
 */
public class dataParcel implements Parcelable {

        private static ArrayList<String> mPath;
        private static ArrayList<String> mOtherInfo;
        private static String mNameId;
        private static String mTemplateType;

        private static dataParcel dataParcel;

        public dataParcel() {}

    public synchronized static dataParcel getDataParcel(String nameId,
                                                        ArrayList<String> path,ArrayList<String> otherinfo) {
        if(dataParcel == null) {
            dataParcel = new dataParcel();
        }

        setResource(nameId, path, otherinfo);

        return dataParcel;
    }

    public static void releaseDataTheme() {
        dataParcel = null;
        releaseResource();
    }

    private static void setResource(String nameId,
                                    ArrayList<String> path,
                                    ArrayList<String> otherinfo) {
        mPath = path;
        mNameId = nameId;
        mOtherInfo = otherinfo;
    }

    private static void releaseResource() {
        mNameId = null;
        if(mPath!= null && mPath.size()!=0) {
            mPath.clear();
        }
        if(mOtherInfo!= null && mOtherInfo.size()!=0) {
            mOtherInfo.clear();
        }
    }

    public String getRandomTransitionId(int position) {
        position %= mOtherInfo.size();
        return mOtherInfo.get(position);
    }

    public dataParcel(Parcel in) {
        readFromParcel(in);
    }

    public ArrayList<String> getPath() {
        return mPath;
    }

    public String getNameId() {
        return mNameId;
    }

    public String getTemplateType() {
        return mOtherInfo.get(0);
    }

    public String getEndingTitle() {
        return mOtherInfo.get(1);
    }

    public String getOpeningTitle() {
        return mOtherInfo.get(0);
    }

    private void readFromParcel(Parcel in) {
        mNameId = in.readString();
        if(mPath == null) {
            mPath = new ArrayList<>();
        } else if(mPath.size()!=0) {
            mPath.clear();
        }
        in.readStringList(mPath);
        if(mOtherInfo == null) {
            mOtherInfo = new ArrayList<>();
        } else if(mOtherInfo.size()!=0) {
            mOtherInfo.clear();
        }
        in.readStringList(mOtherInfo);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mNameId);
        dest.writeStringList(mPath);
        dest.writeStringList(mOtherInfo);
    }

    public static final Parcelable.Creator<dataParcel> CREATOR = new Parcelable.Creator<dataParcel>() {

        @Override
        public dataParcel createFromParcel(Parcel source) {
            return new dataParcel(source);
        }

        @Override
        public dataParcel[] newArray(int size) {
            return new dataParcel[size];
        }

    };

}
