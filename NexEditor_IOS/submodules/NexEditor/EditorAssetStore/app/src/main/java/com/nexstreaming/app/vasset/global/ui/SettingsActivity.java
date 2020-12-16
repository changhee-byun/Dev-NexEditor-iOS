package com.nexstreaming.app.vasset.global.ui;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AlertDialog;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.app.vasset.global.LL;
import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.app.vasset.global.asset.AssetManager;
import com.nexstreaming.app.assetlibrary.model.NexInstalledAssetItem;
import com.nexstreaming.app.vasset.global.ui.adapter.SettingAssetListAdapter;

import java.util.ArrayList;
import java.util.List;

import jp.wasabeef.recyclerview.animators.FadeInAnimator;

/**
 * Created by ojin.kwon on 2016-11-21.
 */

public class SettingsActivity extends BaseActivity {

    private static final String TAG = "SettingsActivity";

    private TextView mTitle;

    private RecyclerView mRecyclerView;
    private SettingAssetListAdapter mAdapter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_setting);
        initData();
        initView();
        loadData();
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    private void initData(){
        mAdapter = new SettingAssetListAdapter(new ArrayList<NexInstalledAssetItem>(), new SettingAssetListAdapter.OnClickUninstallButtonListener() {
            @Override
            public void onClick(final int pos, final NexInstalledAssetItem item) {
                AlertDialog.Builder builder = new AlertDialog.Builder(SettingsActivity.this);
                builder.setTitle(R.string.delete)
                        .setMessage(R.string.asset_delete_message)
                        .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {
                                AssetManager.getInstance().uninstallAsset(item.index).onComplete(new Task.OnTaskEventListener() {
                                    @Override
                                    public void onTaskEvent(Task t, Task.Event e) {
                                        int pos = mAdapter.getData().indexOf(item);
                                        mAdapter.getData().remove(pos);
                                        mAdapter.notifyItemRemoved(pos);
                                        sendBroadcast(new Intent(ACTION_UPDATE_ASSET_LIST));
                                    }
                                }).onFailure(new Task.OnFailListener() {
                                    @Override
                                    public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                                        Log.d(TAG, "onFail() called with: t = [" + t + "], e = [" + e + "], failureReason = [" + failureReason + "]");
                                    }
                                });
                                dialogInterface.dismiss();
                            }
                        }).setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                    }
                }).show();
            }
        });
    }

    private void initView(){
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle("");

        mTitle = (TextView) findViewById(R.id.toolbar_title);

        mRecyclerView = (RecyclerView) findViewById(R.id.rv_settings);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false));
        mRecyclerView.setItemAnimator(new FadeInAnimator());
        mRecyclerView.setAdapter(mAdapter);

        mTitle.setText(R.string.settings);
    }

    private void loadData(){
        AssetManager.getInstance().loadInstalledAsset().onResultAvailable(new ResultTask.OnResultAvailableListener<List<NexInstalledAssetItem>>() {
            @Override
            public void onResultAvailable(ResultTask<List<NexInstalledAssetItem>> task, Task.Event event, List<NexInstalledAssetItem> result) {
                setData(result);
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                if(LL.E) Log.e(TAG, "onFail: ", failureReason.getException());
            }
        });
    }

    private void setData(List<NexInstalledAssetItem> data){
        if(data != null){
            Log.d(TAG, "setData() called with: data = [" + data + "]");
            for(NexInstalledAssetItem info : data){
                mAdapter.getData().add(info);
                mAdapter.notifyItemInserted(mAdapter.getItemCount() - 1);
            }
        }
    }

    @Override
    public void finish() {
        super.finish();
    }
}
