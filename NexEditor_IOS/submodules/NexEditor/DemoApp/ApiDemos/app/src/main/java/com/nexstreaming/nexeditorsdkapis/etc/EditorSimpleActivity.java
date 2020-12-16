/******************************************************************************
 * File Name        : EditorSimpleActivity.java
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
import android.content.ClipData;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.Switch;

import com.nexstreaming.nexeditorsdk.nexAssetStoreAppUtils;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;


import java.util.ArrayList;

public class EditorSimpleActivity extends Activity {
    private ArrayList<String> mListFilePath;
    final static String TAG = "EditorSimple";
    private Spinner vendor;
    private Switch localAssetStore;
    private Switch autoAspect;
    private Switch denyFeaturedList;
    private boolean bLocalStore;
    private RadioGroup servers;
    private EditText marketId;
    private CheckBox catTemplate;
    private CheckBox catBeat;
    private CheckBox catAudio;
    private CheckBox catTitle;
    private CheckBox catCollage;
    private CheckBox catStaticCollage;
    private CheckBox catDynamicCollage;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        ApiDemosConfig.getApplicationInstance().getEngine();
        
        setContentView(R.layout.activity_editor_simple);

        autoAspect = (Switch)findViewById(R.id.switch_auto_aspect);
        denyFeaturedList = (Switch)findViewById(R.id.switch_deny_featuredlist);

        servers = (RadioGroup)findViewById(R.id.radioGroup_server);

        marketId = (EditText)findViewById(R.id.editText_marketid);
        catTemplate = (CheckBox)findViewById(R.id.checkBox_c_template);
        catAudio = (CheckBox)findViewById(R.id.checkBox_c_audio);
        catTitle = (CheckBox)findViewById(R.id.checkBox_c_title);
        catCollage = (CheckBox)findViewById(R.id.checkBox_c_collage);
        catStaticCollage = (CheckBox)findViewById(R.id.checkBox_c_staticcalloage);
        catDynamicCollage = (CheckBox)findViewById(R.id.checkBox_c_dynamiccollage);
        catBeat = (CheckBox)findViewById(R.id.checkBox_c_beat);

        mListFilePath = new ArrayList<String>();
        Button bt = (Button)findViewById(R.id.button_pickup);
        Log.d(TAG, "Version: v1.3");
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                String vendorName = (String)vendor.getSelectedItem();
                nexAssetStoreAppUtils.setVendor(vendorName);
                int id = servers.getCheckedRadioButtonId();

                int serverType = nexAssetStoreAppUtils.AssetStoreServerPRODUCTION;
                switch(id){
                    case R.id.radioButton_server_drift:
                        serverType = nexAssetStoreAppUtils.AssetStoreServerDRAFT;
                        break;

                    case R.id.radioButton_server_staging:
                        serverType = nexAssetStoreAppUtils.AssetStoreServerSTAGING;
                        break;
                    default:
                        break;
                }
                nexAssetStoreAppUtils.setServer(serverType);

                int category = 0;
                if( catTemplate.isChecked() ){
                    category |= nexAssetStoreAppUtils.AssetStoreMimeType_Template;
                }

                if( catBeat.isChecked() ){
                    category |= nexAssetStoreAppUtils.AssetStoreMimeType_BeatTemplate;
                }

                if( catAudio.isChecked() ){
                    category |= nexAssetStoreAppUtils.AssetStoreMimeType_Audio;
                }

                if( catTitle.isChecked() ){
                    category |= nexAssetStoreAppUtils.AssetStoreMimeType_TitleTemplate;
                }

                if( catCollage.isChecked() ){
                    category |= nexAssetStoreAppUtils.AssetStoreMimeType_IntegratedCollage;
                }

                if( catStaticCollage.isChecked() ){
                    category |= nexAssetStoreAppUtils.AssetStoreMimeType_StaticCollage;
                }

                if( catDynamicCollage.isChecked() ){
                    category |= nexAssetStoreAppUtils.AssetStoreMimeType_DynamicCollage;
                }

                nexAssetStoreAppUtils.setAvailableCategorys(category);

                nexAssetStoreAppUtils.setDenyFeaturedList(denyFeaturedList.isChecked());

                String market = marketId.getText().toString();
                if( market != null ){
                    nexAssetStoreAppUtils.setMarketId(market);
                }

                Intent intent = new Intent( );
                //intent.setType("video/*");
                intent.setType("*/*");
                String[] mimetypes = {"image/*","video/*"};
                intent.putExtra(Intent.EXTRA_MIME_TYPES, mimetypes);
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE,true);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });

        vendor = (Spinner)findViewById(R.id.spinner_vender);
        String[] vendorList = getResources().getStringArray(R.array.VendorList);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                R.layout.simple_spinner_dropdown_item_1, vendorList);
        vendor.setAdapter(adapter);
		vendor.setEnabled(true);
        vendor.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                String vendorName=(String)parent.getSelectedItem();
                if ( pos == 0 ) {
                    return;
                }
                nexAssetStoreAppUtils.setVendor(vendorName);
            }
			public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        localAssetStore = (Switch)findViewById(R.id.switch_asset_type);
        localAssetStore.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if( isChecked ){
                    vendor.setEnabled(false);
                    bLocalStore = true;
                }else{
                    vendor.setEnabled(true);
                    bLocalStore = false;
                }
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 100 && resultCode == Activity.RESULT_OK) {

            ClipData clipData = data.getClipData();
            String filepath = null;

            mListFilePath.clear();

            if (clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                mListFilePath.add(filepath);
            } else {
                ClipData.Item item;
                Uri uri = null;
                for (int i = 0; i < clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                    mListFilePath.add(filepath);
                }
            }
            boolean bAutoAspect = autoAspect.isChecked();
            Intent intent = new Intent(this, EditorSampleTemplateActivity.class);
            intent.putStringArrayListExtra("filelist", mListFilePath);
            //intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK |Intent.FLAG_ACTIVITY_MULTIPLE_TASK );
            intent.putExtra("localAssetStore",bLocalStore);
            intent.putExtra("autoAspect",bAutoAspect);
            startActivity(intent);

        }
    }
}
