package com.nexstreaming.kminternal.kinemaster.mediainfo;

public interface ThumbnailCallbackRaw extends ThumbnailCallback {
    void processThumbnail( byte[] data, int index, int totalCount, int timestamp );
}
