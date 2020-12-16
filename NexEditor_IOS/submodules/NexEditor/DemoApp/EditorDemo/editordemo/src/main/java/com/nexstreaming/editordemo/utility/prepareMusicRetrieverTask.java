package com.nexstreaming.editordemo.utility;

import android.os.AsyncTask;

/**
 * Created by mj.kong on 2015-04-01.
 */
public class prepareMusicRetrieverTask extends AsyncTask<Void, Void, Void> {
    musicRetriever mRetriever;
    MusicRetrieverPreparedListener mListener;

    public prepareMusicRetrieverTask(musicRetriever retriever,
                                     MusicRetrieverPreparedListener listener) {
        mRetriever = retriever;
        mListener = listener;
    }

    @Override
    protected Void doInBackground(Void... arg0) {
        mRetriever.prepare();
        return null;
    }

    @Override
    protected void onPostExecute(Void result) {
        mListener.onMusicRetrieverPrepared();
    }

    public interface MusicRetrieverPreparedListener {
        public void onMusicRetrieverPrepared();
    }
}
