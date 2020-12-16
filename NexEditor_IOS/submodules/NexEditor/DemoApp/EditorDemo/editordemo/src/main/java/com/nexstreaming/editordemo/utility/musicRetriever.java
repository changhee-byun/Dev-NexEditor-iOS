package com.nexstreaming.editordemo.utility;

import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Created by mj.kong on 2015-04-01.
 */
public class musicRetriever {
    ContentResolver mContentResolver;

    // the items (songs) we have queried
    List<Item> mItems = new ArrayList<>();

    public musicRetriever(ContentResolver cr) {
        mContentResolver = cr;
    }

    /**
     * Loads music data. This method may take long, so be sure to call it asynchronously without
     * blocking the main thread.
     */
    public void prepare() {
        Uri uri = android.provider.MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;

        // Perform a query on the content resolver. The URI we're passing specifies that we
        // want to query for all audio media on external storage (e.g. SD card)
        Cursor cur = mContentResolver.query(uri, null,
                MediaStore.Audio.Media.IS_MUSIC + " = 1", null, null);

        if (cur == null) {
            // Query failed...
            return;
        }
        if (!cur.moveToFirst()) {
            // Nothing to query. There is no music on the device. How boring.
            return;
        }

        // retrieve the indices of the columns where the ID, title, etc. of the song are
        int artistColumn = cur.getColumnIndex(MediaStore.Audio.Media.ARTIST);
        int titleColumn = cur.getColumnIndex(MediaStore.Audio.Media.TITLE);
        int albumColumn = cur.getColumnIndex(MediaStore.Audio.Media.ALBUM_ID);
        int pathColumn = cur.getColumnIndex(MediaStore.Audio.Media.DATA);
        int durationColumn = cur.getColumnIndex(MediaStore.Audio.Media.DURATION);
        int idColumn = cur.getColumnIndex(MediaStore.Audio.Media._ID);

        // add each song to mItems
        do {
            mItems.add(new Item(
                    idColumn!=-1?cur.getLong(idColumn):-1,
                    artistColumn!=-1?cur.getString(artistColumn):"",
                    titleColumn!=-1?cur.getString(titleColumn):"",
                    albumColumn!=-1?cur.getLong(albumColumn):-1,
                    pathColumn!=-1?cur.getString(pathColumn):"",
                    durationColumn!=-1?cur.getLong(durationColumn):-1));
        } while (cur.moveToNext());

    }

    public int getItemSize() {
        return mItems.size();
    }

    public Object getItem(int position) {
        if(position > mItems.size()) {
            return null;
        }
        return mItems.get(position);
    }

    public static class Item {
        long id;
        String artist;
        String title;
        long album_id;
        String file_path;
        long duration;

        public Item(long id, String artist, String title, long album_id, String file_path, long duration) {
            this.id = id;
            this.artist = artist;
            this.title = title;
            this.album_id = album_id;
            this.file_path = file_path;
            this.duration = duration;
        }

        public long getId() {
            return id;
        }

        public String getArtist() {
            return artist;
        }

        public String getTitle() {
            return title;
        }

        public long getAlbumId() {
            return album_id;
        }

        public String getFilePath() {
            return file_path;
        }

        public long getDuration() {
            return duration;
        }
    }

    public void releaseResource() {
        mItems.clear();
    }
}
