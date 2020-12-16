package com.nexstreaming.app.assetstore.zte.ui.adapter;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;

import com.nexstreaming.app.assetlibrary.adapter.BaseFragmentPagerAdapter;
import com.nexstreaming.app.assetstore.zte.ui.fragment.BaseFragment;

import java.util.List;

/**
 * Created by ojin.kwon on 2016-11-18.
 */

public class MainPagerAdapter extends BaseFragmentPagerAdapter {

    private static final String TAG = "MainPagerAdapter";

    private List<FragmentSet> mFragments;

    public MainPagerAdapter(FragmentManager fm, List<FragmentSet> list) {
        super(fm);
        mFragments = list;
    }

    @Override
    public Fragment getItem(int position) {
        return mFragments.get(position).getFragment();
    }

    @Override
    public int getCount() {
        return mFragments.size();
    }

    @Override
    public int getRealCount() {
        return mFragments.size();
    }

    @Override
    public int getVirtualPosition(int position) {
        return position;
    }

    public static class FragmentSet{

        public FragmentSet(Class<? extends BaseFragment> cls, Bundle arg){
            this.cls = cls;
            this.arg = arg;
        }
        public final Class<? extends BaseFragment> cls;
        public final Bundle arg;
        private BaseFragment fragment;

        private BaseFragment getFragment(){
            if(fragment == null){
                try {
                    fragment = cls.newInstance();
                    fragment.setArguments(arg);
                } catch (InstantiationException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                }
            }
            return  fragment;
        }
    }
}
