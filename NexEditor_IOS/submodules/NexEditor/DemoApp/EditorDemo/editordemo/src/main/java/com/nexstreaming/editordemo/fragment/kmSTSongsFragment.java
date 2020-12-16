package com.nexstreaming.editordemo.fragment;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.utility.musicRetriever;
import com.nexstreaming.editordemo.utility.prepareMusicRetrieverTask;
import com.nexstreaming.editordemo.utility.utilityAlbumArt;
import com.nexstreaming.editordemo.utility.utilityCode;

/**
 * Created by mj.kong on 2015-03-27.
 */
public class kmSTSongsFragment extends Fragment implements
        prepareMusicRetrieverTask.MusicRetrieverPreparedListener{

    private ListView mList;

    private OnSTSongsSelectedListener mListener;

    private musicRetriever mRetriever;
    private ContentResolver mContentResolver;

    private utilityAlbumArt mMusicAlbumArt;

    public interface OnSTSongsSelectedListener {
        public void onSTSongsSelected(String path, String title, String artist, int albumid);
    }

    public static kmSTSongsFragment newInstance() {
        kmSTSongsFragment fragment = new kmSTSongsFragment();
        return fragment;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (OnSTSongsSelectedListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnSTSongsSelectedListener");
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContentResolver = getActivity().getContentResolver();

        // Create the retriever and start an asynchronous task that will prepare it.
        mRetriever = new musicRetriever(mContentResolver);
        (new prepareMusicRetrieverTask(mRetriever, this)).execute();

        mMusicAlbumArt = new utilityAlbumArt(getActivity().getBaseContext(), mContentResolver, mRetriever);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_songs, container, false);

        mList = (ListView) v.findViewById(R.id.songslist);
        mList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                musicRetriever.Item item = (musicRetriever.Item)mRetriever.getItem(position);
                String filename = item.getFilePath();
                String title = item.getTitle();
                String artist = item.getArtist();
                int albumid = (int)item.getAlbumId();
                mListener.onSTSongsSelected(filename, title, artist, albumid);
            }
        });

        return v;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        mRetriever.releaseResource();
    }

    @Override
    public void onMusicRetrieverPrepared() {
        mList.setAdapter(new SoundSongsListAdapter());
    }

    private class SoundSongsListAdapter extends BaseAdapter {
        private LayoutInflater inflater = null;

        public SoundSongsListAdapter() {
            inflater = (LayoutInflater) getActivity().getBaseContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mRetriever.getItemSize();
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            musicRetriever.Item item = (musicRetriever.Item)mRetriever.getItem(position);

            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem2_view, null);

            ImageView iv =  (ImageView) vi.findViewById(R.id.sound_icon);
            mMusicAlbumArt.loadBitmap(position, null, iv);

            TextView tv1 = (TextView) vi.findViewById(R.id.sound_title);
            if(!item.getTitle().isEmpty()) {
                tv1.setText(item.getTitle());
            } else {
                tv1.setText("");
            }
            TextView tv2 = (TextView) vi.findViewById(R.id.sound_artist);
            if(!item.getArtist().isEmpty()) {
                tv2.setText(item.getArtist());
            } else {
                tv2.setText("");
            }
            TextView tv3 = (TextView) vi.findViewById(R.id.sound_duration);
            if(item.getDuration() != -1) {
                tv3.setText(utilityCode.stringForTime((int)item.getDuration()));
            } else {
                tv3.setText("");
            }
            return vi;
        }
    }

}
