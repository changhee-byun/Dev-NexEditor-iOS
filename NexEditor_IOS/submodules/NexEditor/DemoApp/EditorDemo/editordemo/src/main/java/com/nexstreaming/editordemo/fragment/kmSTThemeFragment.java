package com.nexstreaming.editordemo.fragment;

import android.app.Activity;
import android.content.Context;
import android.graphics.BitmapFactory;
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

import java.util.ArrayList;

/**
 * Created by mj.kong on 2015-03-27.
 */
public class kmSTThemeFragment extends Fragment {

    private ListView mList;

    private OnSTThemeSelectedListener mListener;

    private ArrayList<String> mPathList;
    private String[] mPaths = new String[] {"whistling and fun", "baby", "baseball", "iloveyou", "spring", "themepark"};

    public interface OnSTThemeSelectedListener {
        public void onSThemeSelected(String path);
    }

    public static kmSTThemeFragment newInstance() {
        kmSTThemeFragment fragment = new kmSTThemeFragment();
        return fragment;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (OnSTThemeSelectedListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnSTThemeSelectedListener");
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mPathList = new ArrayList<>();
        for(String path: mPaths) mPathList.add(path);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        mPathList.clear();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_theme, container, false);

        mList = (ListView) v.findViewById(R.id.themelist);
        mList.setAdapter(new SoundThemeListAdapter());
        mList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String path = mPathList.get(position);
                mListener.onSThemeSelected(path);
            }
        });

        return v;
    }

    private class SoundThemeListAdapter extends BaseAdapter {
        private LayoutInflater inflater=null;

        public SoundThemeListAdapter() {
            inflater = (LayoutInflater)getActivity().getBaseContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mPathList.size();
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
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem2_view, null);

            ImageView iv =  (ImageView) vi.findViewById(R.id.sound_icon);

            TextView tv1 = (TextView) vi.findViewById(R.id.sound_title);
            tv1.setText(mPathList.get(position));

            TextView tv2 = (TextView) vi.findViewById(R.id.sound_artist);
            tv2.setText("KineMaster Theme");

            TextView tv3 = (TextView) vi.findViewById(R.id.sound_duration);

            /**
             * 해당 부분은 어플 쪽에 내장된 파일에 대한 것을 표시하고 있기에 아래와 같이 구현하였습니다.
             */
            switch(position) {
                case 0:
                    iv.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp));
                    tv3.setText("01:51");
                    break;
                case 1:
                    iv.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_baby));
                    tv3.setText("00:51");
                    break;
                case 2:
                    iv.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_baseball));
                    tv3.setText("01:00");
                    break;
                case 3:
                    iv.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_iloveyou));
                    tv3.setText("01:00");
                    break;
                case 4:
                    iv.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_spring));
                    tv3.setText("01:15");
                    break;
                case 5:
                    iv.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_themepark));
                    tv3.setText("00:52");
                    break;
            }

            return vi;
        }
    }
}
