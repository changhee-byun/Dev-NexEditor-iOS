/******************************************************************************
 * File Name        : AssetListActivity.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdkapis.etc;
import android.app.Activity;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ListView;

import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexTemplateManager;
import com.nexstreaming.nexeditorsdkapis.R;

import java.util.ArrayList;
import java.util.List;

public class AssetListActivity extends Activity {
    private boolean template = true;
    private boolean audio = true;
    private boolean  effect = true;
    private boolean  transition = true;
    private boolean  lut = true;
    private boolean  overlay = true;
    private boolean  filter = true;
    private boolean  asset = true;
    private boolean  share = true;
    private boolean  store = true;

    private boolean  a16v9 = true;
    private boolean  a9v16 = true;
    private boolean  a1v1 = true;
    private boolean  a2v1 = true;

    private List<nexAssetPackageManager.Item> items;
    private List<String> listViewItems = new ArrayList<>();

    private ListView listView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_asset_list);

        ((CheckBox)findViewById( R.id.checkBox_template)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                template = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_audio)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                audio = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_effect)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                effect = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_transition)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                transition = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_lut)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                lut = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_overlay)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                overlay = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_filter)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                filter = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_asset)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                asset = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_share)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                share = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_store)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                store = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_store)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                store = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_16v9)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                a16v9 = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_9v16)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                a9v16 = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_1v1)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                a1v1 = isChecked;
                updateListView();
            }
        });

        ((CheckBox)findViewById( R.id.checkBox_2v1)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                a2v1 = isChecked;
                updateListView();
            }
        });

        items = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItems();
        listView = (ListView)findViewById(R.id.listView_assets);
        updateListView();
    }

    private void updateListView(){
        listViewItems.clear();

        for( nexAssetPackageManager.Item item:  items){
            if( template ){
                if( item.category() == nexAssetPackageManager.Category.template ){
                    listViewItems.add(item.id());
                    continue;
                }
            }

            if( audio ){
                if( item.category() == nexAssetPackageManager.Category.audio ){
                    listViewItems.add(item.id());
                    continue;
                }
            }

            if( effect ){
                if( item.category() == nexAssetPackageManager.Category.effect ){
                    listViewItems.add(item.id());
                    continue;
                }
            }

            if( transition ){
                if( item.category() == nexAssetPackageManager.Category.transition ){
                    listViewItems.add(item.id());
                    continue;
                }
            }

            if( filter ){
                if( item.category() == nexAssetPackageManager.Category.filter ){
                    listViewItems.add(item.id());
                    continue;
                }
            }

            if( overlay ){
                if( item.category() == nexAssetPackageManager.Category.overlay ){
                    listViewItems.add(item.id());
                    continue;
                }
            }

            if( a16v9 ){
                if( item.category() == nexAssetPackageManager.Category.template ){
                    nexTemplateManager.Template temp = nexTemplateManager.Template.promote(item);
                    if( Float.compare(temp.aspect(),16/9f) == 0 ){
                        listViewItems.add(item.id());
                        continue;
                    }
                }
            }

            if( a9v16 ){
                if( item.category() == nexAssetPackageManager.Category.template ){
                    nexTemplateManager.Template temp = nexTemplateManager.Template.promote(item);
                    if( Float.compare(temp.aspect(),9/16f) == 0 ){
                        listViewItems.add(item.id());
                        continue;
                    }
                }
            }

            if( a1v1 ){
                if( item.category() == nexAssetPackageManager.Category.template ){
                    nexTemplateManager.Template temp = nexTemplateManager.Template.promote(item);
                    if( Float.compare(temp.aspect() , 1/1f) == 0 ){
                        listViewItems.add(item.id());
                        continue;
                    }
                }
            }

            if( a2v1 ){
                if( item.category() == nexAssetPackageManager.Category.template ){
                    nexTemplateManager.Template temp = nexTemplateManager.Template.promote(item);
                    if( Float.compare(temp.aspect() , 2/1f) == 0 ){
                        listViewItems.add(item.id());
                        continue;
                    }
                }
            }

            if( asset ){
                if( item.packageInfo().installedType() == nexAssetPackageManager.AssetInstallType.APP_ASSETS ){
                    listViewItems.add(item.id());
                    continue;
                }
            }

            if( share ){
                if( item.packageInfo().installedType() == nexAssetPackageManager.AssetInstallType.SHARE ){
                    listViewItems.add(item.id());
                    continue;
                }
            }

            if( store ){
                if( item.packageInfo().installedType() == nexAssetPackageManager.AssetInstallType.STORE ){
                    listViewItems.add(item.id());
                    continue;
                }
            }

        }
        listView.setAdapter(new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_list_item_1, listViewItems));
    }
}
