package com.nexstreaming.app.vasset.global.ui;

import android.os.Handler;
import android.support.v4.app.Fragment;
import android.support.v4.view.ViewPager;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.nexstreaming.app.assetlibrary.view.PagerSlidingTabStrip;
import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.app.vasset.global.ConfigGlobal;
import com.nexstreaming.app.vasset.global.LL;
import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.app.assetlibrary.network.assetstore.AssetStoreSession;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreCategoryInfo;
import com.nexstreaming.app.vasset.global.ui.adapter.MainPagerAdapter;
import com.nexstreaming.app.vasset.global.ui.fragment.AssetCategoryFragment;
import com.nexstreaming.app.vasset.global.util.DeeplinkManager;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends BaseActivity {

    private static final String TAG = "MainActivity";

    private TextView mTitle;
    private PagerSlidingTabStrip mPageTabStrip;
    private ViewPager mViewPager;
    private ProgressBar mLoadingView;

    private View mNetworkErrorLayout;
    private TextView mNetworkErrorMessage;

    private MainPagerAdapter mPagerAdapter;
    private ArrayList<MainPagerAdapter.FragmentSet> mFragmentSets;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initData();
        initView();
        loadData();
        checkIntent();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void initData(){
        mFragmentSets = new ArrayList<>();
    }

    private void initView(){
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle("");

        mTitle = (TextView) findViewById(R.id.toolbar_title);
        mPageTabStrip = (PagerSlidingTabStrip) findViewById(R.id.tabs_main);
        mViewPager = (ViewPager) findViewById(R.id.vp_main);
        mLoadingView = (ProgressBar) findViewById(R.id.pb_main);

        mNetworkErrorLayout = findViewById(R.id.layout_main_network_error);
        mNetworkErrorMessage = (TextView) findViewById(R.id.tv_network_error_message);

        mViewPager.setOffscreenPageLimit(3);

        mTitle.setText(getString(R.string.app_name));
    }

    private void loadData(){
        AssetStoreSession.getInstance(this).getCategoryList().onResultAvailable(new ResultTask.OnResultAvailableListener<List<StoreCategoryInfo>>() {
            @Override
            public void onResultAvailable(ResultTask<List<StoreCategoryInfo>> task, Task.Event event, List<StoreCategoryInfo> result) {
                mPageTabStrip.setVisibility(View.VISIBLE);
                mViewPager.setVisibility(View.VISIBLE);
                mNetworkErrorLayout.setVisibility(View.GONE);
                setCategoryData(result);
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                mPageTabStrip.setVisibility(View.GONE);
                mViewPager.setVisibility(View.GONE);
                mNetworkErrorLayout.setVisibility(View.VISIBLE);
                mLoadingView.setVisibility(View.GONE);
            }
        });
    }

    private void checkIntent(){
        if(getIntent() != null){
            DeeplinkManager.goLink(this, getIntent().getData());
        }
    }

    private void setCategoryData(List<StoreCategoryInfo> categoryData){
        if(LL.D) Log.d(TAG, "setCategory Data () called with: " +  categoryData);
        if(categoryData != null){
            if(categoryData.size() > 0){
                for(StoreCategoryInfo info : categoryData){
                    mFragmentSets.add(new MainPagerAdapter.FragmentSet(AssetCategoryFragment.class, AssetCategoryFragment.makeArguments(this, info)));
                }
                if(mPagerAdapter == null){
                    mPagerAdapter = new MainPagerAdapter(getSupportFragmentManager(), mFragmentSets);
                    mViewPager.setAdapter(mPagerAdapter);
                    mPageTabStrip.setViewPager(mViewPager);
                }else{
                }

            }else{
                //TODO : NO DATA
            }
        }
        mLoadingView.setVisibility(View.GONE);
        new Handler().post(new Runnable() {
            @Override
            public void run() {
                int type = getIntent().getIntExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, 0);
                int pos;
                switch (type){
                    case ConfigGlobal.AssetStoreMimeType_Audio:
                        pos = 2;
                        break;

                    case ConfigGlobal.AssetStoreMimeType_TitleTemplate :
                        pos = 1;
                        break;

                    case ConfigGlobal.AssetStoreMimeType_Template :
                        default:
                        pos = 0;
                        break;
                }
                mViewPager.setCurrentItem(pos);
            }
        });
    }

    public void updateTab(){
        if(mPageTabStrip != null){
            mPageTabStrip.notifyDataSetChanged();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main_menu, menu);
        return true;
    }

    @Override
    protected void onUpdateAssetList() {
        super.onUpdateAssetList();
        for(Fragment f : getSupportFragmentManager().getFragments()){
            if(f instanceof AssetCategoryFragment){
                if(f.isAdded()){
                    ((AssetCategoryFragment) f).updateAssetList();
                }
            }
        }
    }
}
