package com.nexstreaming.editordemo.activity;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.commonsware.cwac.merge.MergeAdapter;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexTransitionEffect;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.utility.dataParcel;
import com.nexstreaming.editordemo.utility.utilityTransitionIcon;

import java.util.ArrayList;
import java.util.Random;

public class kmTransitionListActivity extends ActionBarActivity {

    private ListView mList;
    private Toolbar mToolbar;

    private ArrayList<nexTransitionEffect> m3DTransList;
    private ArrayList<nexTransitionEffect> mClassicTransList;
    private ArrayList<nexTransitionEffect> mFunTransList;
    private ArrayList<nexTransitionEffect> mPIPTransList; // Picture-in-Picture and Transition
    private ArrayList<nexTransitionEffect> mPresentationList;
    private ArrayList<nexTransitionEffect> mTextTransList;

    private ArrayList<String> mTransitionList;

    private dataParcel mDataParcel;
    private int mEdittoolDataParcel;
    private utilityTransitionIcon m3DTransIcon;
    private utilityTransitionIcon mClassicTransIcon;
    private utilityTransitionIcon mFunTransIcon;
    private utilityTransitionIcon mPIPTransIcon;
    private utilityTransitionIcon mPresentationIcon;
    private utilityTransitionIcon mTextTransIcon;

    private MergeAdapter mMergeAdapter;

    public static Activity translistActivity;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        translistActivity = this;

        if(getIntent().getIntExtra("from_preview", 0) == 1) kmTransitionPreviewActivity.previewAct.finish();
        mEdittoolDataParcel = getIntent().getIntExtra("EdittoolData", -1);
        mDataParcel = getIntent().getParcelableExtra("parcelData");

        if(m3DTransList != null && m3DTransList.size() != 0) m3DTransList.clear();
        if(mClassicTransList != null && mClassicTransList.size() != 0) mClassicTransList.clear();
        if(mFunTransList != null && mFunTransList.size() != 0) mFunTransList.clear();
        if(mPIPTransList != null && mPIPTransList.size() != 0) mPIPTransList.clear();
        if(mPresentationList != null && mPresentationList.size() != 0) mPresentationList.clear();
        if(mTextTransList != null && mTextTransList.size() != 0) mTextTransList.clear();

        m3DTransList = new ArrayList<>();
        mClassicTransList = new ArrayList<>();
        mFunTransList = new ArrayList<>();
        mPIPTransList = new ArrayList<>();
        mPresentationList = new ArrayList<>();
        mTextTransList = new ArrayList<>();
        mTransitionList = new ArrayList<>();

        m3DTransIcon = new utilityTransitionIcon(getBaseContext(), m3DTransList);
        mClassicTransIcon = new utilityTransitionIcon(getBaseContext(), mClassicTransList);
        mFunTransIcon = new utilityTransitionIcon(getBaseContext(), mFunTransList);
        mPIPTransIcon = new utilityTransitionIcon(getBaseContext(), mPIPTransList);
        mPresentationIcon = new utilityTransitionIcon(getBaseContext(), mPresentationList);
        mTextTransIcon = new utilityTransitionIcon(getBaseContext(), mTextTransList);

        for(nexTransitionEffect transEffect: nexEffectLibrary.getEffectLibrary(this).getTransitionEffectsEx()) {
            String id = transEffect.getName(getBaseContext());
            switch(id) {
                /*  3D Transition
                **/
                case "3D Flip":
                case "3D Zoom Flip":
                case "Bouncy Box":
                case "Checker Flip":
                case "Strips":
                    m3DTransList.add(transEffect);
                    break;
                /*  Classic Transition
                **/
                case "Crossfade":
                case "Fade Through Color":
                case "Zoom out":
                    mClassicTransList.add(transEffect);
                    break;
                /*  Fun Transition
                **/
                case "Circle Wipe":
                case "Heart Wipe":
                case "Knock Aside":
                case "Many Circles":
                case "Star Wipe":
                    mFunTransList.add(transEffect);
                    break;
                /*  Picture-in-Picture and Transition
                **/
                case "Color Tiles":
                case "Corners":
                case "Inset Video":
                case "Split-screen":
                case "Video Tiles":
                    mPIPTransList.add(transEffect);
                    break;
                /*  Presentation Transition
                **/
                case "Block In":
                case "Clock Wipe":
                case "Cover":
                case "Double Split":
                case "Slide":
                case "Spin":
                case "Split":
                case "Strip Wipe":
                case "Uncover":
                case "Wipe":
                case "Zoom out, then in":
                    mPresentationList.add(transEffect);
                    break;
                /*  Text Transition
                **/
                case "Captioned Inset":
                case "Retro Pastel Title":
                case "Retro Terminal Title":
                case "Split Title":
                case "Square Title":
                    mTextTransList.add(transEffect);
                    break;
                default:
                    Log.d("Transitionlist", "Name="+id);
                    break;
            }
        }

        setContentView(R.layout.activity_km_transition_list);

        mToolbar = (Toolbar) findViewById(R.id.translist_toolbar);
        setSupportActionBar(mToolbar);

        ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(getResources().getDrawable(R.mipmap.ic_arrow_back_white_24dp_noshadow));
        actionBar.setTitle("Transition List");

        mMergeAdapter = new MergeAdapter();

        mMergeAdapter.addAdapter(new FirstTransListAdapter());
        mMergeAdapter.addView(buildLine());
        mMergeAdapter.addView(buildHeader("3D Transition"));
        mMergeAdapter.addAdapter(new SecondTransListAdapter());
        mMergeAdapter.addView(buildLine());
        mMergeAdapter.addView(buildHeader("Classic Transition"));
        mMergeAdapter.addAdapter(new ThirdTransListAdapter());
        mMergeAdapter.addView(buildLine());
        mMergeAdapter.addView(buildHeader("Fun Transition"));
        mMergeAdapter.addAdapter(new FourthTransListAdapter());
        mMergeAdapter.addView(buildLine());
        mMergeAdapter.addView(buildHeader("Picture-in-Picture Transition"));
        mMergeAdapter.addAdapter(new FifthTansListAdapter());
        mMergeAdapter.addView(buildLine());
        mMergeAdapter.addView(buildHeader("Presentation"));
        mMergeAdapter.addAdapter(new SixthTransListAdapter());
//        mMergeAdapter.addView(buildLine());
//        mMergeAdapter.addView(buildHeader("Text Transition"));
//        mMergeAdapter.addAdapter(new SeventhTransListAdapter());

        mList = (ListView) findViewById(R.id.translist);
        mList.setAdapter(mMergeAdapter);
        mList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if(position == 0) {
                    // Random
                    ArrayList<nexTransitionEffect> transEffect = nexEffectLibrary.getEffectLibrary(getBaseContext()).getTransitionEffectsEx();

                    for(int loopid=0, randomId; loopid<transEffect.size(); loopid++) {
                        setTransitionList(transEffect);
                    }
                    if(mEdittoolDataParcel >= 0) {
                        Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                        String effectname = mTransitionList.get(0);
                        Log.d("Transitionlist", "effectname="+effectname);
                        intent.putExtra("parcelDataString", effectname);
                        setResult(2000, intent);
                        finish();
                    }
                    else {
                        dataParcel parcel = dataParcel.getDataParcel(null, mDataParcel.getPath(), mTransitionList);
                        Intent intent = new Intent(getBaseContext(), kmTransitionPreviewActivity.class);
                        intent.putExtra("parcelData", parcel);
                        startActivity(intent);
                    }
                } else {
                    if(mEdittoolDataParcel >= 0) {
                        Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                        String effectname = ((nexTransitionEffect)mList.getItemAtPosition(position)).getId();
                        intent.putExtra("parcelDataString", effectname);
                        setResult(2000, intent);
                        finish();
                    }
                    else {
                        dataParcel parcel = dataParcel.getDataParcel(((nexTransitionEffect)mList.getItemAtPosition(position)).getId(), mDataParcel.getPath(), null);

                        Intent intent = new Intent(getBaseContext(), kmTransitionPreviewActivity.class);
                        intent.putExtra("parcelData", parcel);
                        startActivity(intent);
                    }
                }
            }
        });
    }

    void setTransitionList(ArrayList<nexTransitionEffect> transEffect) {

        int randomId = new Random().nextInt(transEffect.size());
        String transEffectName = transEffect.get(randomId).getName(getBaseContext());
        String transEffectId = transEffect.get(randomId).getId();

        switch(transEffectName) {
            /*  3D Transition
            **/
            case "3D Flip":
            case "3D Zoom Flip":
            case "Bouncy Box":
            case "Checker Flip":
            case "Strips":
            /*  Classic Transition
            **/
            case "Crossfade":
            case "Fade Through Color":
            case "Zoom out":
            /*  Fun Transition
            **/
            case "Circle Wipe":
            case "Heart Wipe":
            case "Knock Aside":
            case "Many Circles":
            case "Star Wipe":
            /*  Picture-in-Picture and Transition
            **/
            case "Color Tiles":
            case "Corners":
            case "Inset Video":
            case "Split-screen":
            case "Video Tiles":
            /*  Presentation Transition
            **/
            case "Block In":
            case "Clock Wipe":
            case "Cover":
            case "Double Split":
            case "Slide":
            case "Spin":
            case "Split":
            case "Strip Wipe":
            case "Uncover":
            case "Wipe":
            case "Zoom out, then in":
            /*  Text Transition
            **/
//            case "Captioned Inset":
//            case "Retro Pastel Title":
//            case "Retro Terminal Title":
//            case "Split Title":
//            case "Square Title":
                mTransitionList.add(transEffectId);
                break;
            default:
                setTransitionList(transEffect);
                break;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        m3DTransList.clear();
        mClassicTransList.clear();
        mFunTransList.clear();
        mPIPTransList.clear();
        mPresentationList.clear();
        mTextTransList.clear();
        mTransitionList.clear();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private View buildLine() {
        ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 3/*1dp*/);

        View view = new View(this);
        view.setBackgroundColor(getResources().getColor(R.color.colorControlNormal));
        view.setLayoutParams(params);
        return view;
    }

    private View buildHeader(CharSequence text) {
        ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 144/*48dp*/);

        TextView tv = new TextView(this);
        tv.setBackgroundColor(getResources().getColor(R.color.appBackground));
        tv.setGravity(Gravity.LEFT | Gravity.CENTER);
        tv.setTextAppearance(this, android.R.style.TextAppearance_Medium);
        tv.setPadding(48/*16dp*/, 24/*8dp*/, 0, 24/*8dp*/);
        tv.setTextColor(getResources().getColor(R.color.textColorHeader));
        tv.setText(text);
        tv.setLayoutParams(params);
        return tv;
    }

    /*Basic Transition*/
    private class FirstTransListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public FirstTransListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return 1; // Random
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem_view, null);

            ImageView imageView = (ImageView) vi.findViewById(R.id.list_icon);
            imageView.setImageResource(R.mipmap.ic_list_avatar_random_40dp);

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText("Random");

            return vi;
        }
    }

    /*3D Transition*/
    private class SecondTransListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public SecondTransListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return m3DTransList.size();
        }

        @Override
        public Object getItem(int position) {
            return m3DTransList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem_view, null);

            ImageView imageView = (ImageView) vi.findViewById(R.id.list_icon);
            m3DTransIcon.loadBitmap(position, null, imageView);

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText(m3DTransList.get(position).getName(getBaseContext()));
            return vi;
        }
    }

    /*Classic Transition*/
    private class ThirdTransListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public ThirdTransListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mClassicTransList.size();
        }

        @Override
        public Object getItem(int position) {
            return mClassicTransList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem_view, null);

            ImageView imageView = (ImageView) vi.findViewById(R.id.list_icon);
            mClassicTransIcon.loadBitmap(position, null, imageView);

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText(mClassicTransList.get(position).getName(getBaseContext()));
            return vi;
        }
    }

    /*Fun Transition*/
    private class FourthTransListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public FourthTransListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mFunTransList.size();
        }

        @Override
        public Object getItem(int position) {
            return mFunTransList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem_view, null);

            ImageView imageView = (ImageView) vi.findViewById(R.id.list_icon);
            mFunTransIcon.loadBitmap(position, null, imageView);

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText(mFunTransList.get(position).getName(getBaseContext()));
            return vi;
        }
    }

    /*Picture-in-Picture and Transition*/
    private class FifthTansListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public FifthTansListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mPIPTransList.size();
        }

        @Override
        public Object getItem(int position) {
            return mPIPTransList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem_view, null);

            ImageView imageView = (ImageView) vi.findViewById(R.id.list_icon);
            mPIPTransIcon.loadBitmap(position, null, imageView);

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText(mPIPTransList.get(position).getName(getBaseContext()));
            return vi;
        }
    }

    /*Presentation*/
    private class SixthTransListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public SixthTransListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mPresentationList.size();
        }

        @Override
        public Object getItem(int position) {
            return mPresentationList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem_view, null);

            ImageView imageView = (ImageView) vi.findViewById(R.id.list_icon);
            mPresentationIcon.loadBitmap(position, null, imageView);

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText(mPresentationList.get(position).getName(getBaseContext()));
            return vi;
        }
    }

    /*Text Transition*/
    private class SeventhTransListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public SeventhTransListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mTextTransList.size();
        }

        @Override
        public Object getItem(int position) {
            return mTextTransList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem_view, null);

            ImageView imageView = (ImageView) vi.findViewById(R.id.list_icon);
            mTextTransIcon.loadBitmap(position, null, imageView);

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText(mTextTransList.get(position).getName(getBaseContext()));
            return vi;
        }
    }
}
