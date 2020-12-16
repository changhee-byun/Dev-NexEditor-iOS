package com.nexstreaming.speedmetadata;


import android.app.Activity;
import android.app.ProgressDialog;
import android.content.ClipData;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;


import com.nexstreaming.anigifsdk.nexAniGifSDK;
import com.nexstreaming.scmbsdk.nxImageEntry;
import com.nexstreaming.scmbsdk.nxSCMBEntry;
import com.nexstreaming.scmbsdk.nxSCMBSDK;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {

    private nxSCMBSDK instance;
    private long startTrim;
    private long endTrim;
    private int speed;
    private LinearLayout mGallery;
    private LayoutInflater mInflater;
    private SeekBar seekBarStart;
    private SeekBar seekBarEnd;
    private SeekBar seekBarSpeed;
    private List<nxImageEntry> imageEntries = new ArrayList<>();

    private Button buttonSelectImage;
    private Button buttonSaveImage;

    private static byte [] writeInts(int [] array) {
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream(array.length * 4);
            DataOutputStream dos = new DataOutputStream(bos);
            for (int i = 0; i < array.length; i++) {
                dos.writeInt(array[i]);
            }

            return bos.toByteArray();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private boolean createDumpFile(String file){
        int i = 0;
        FileOutputStream output = null;
        FileInputStream input = null;
        byte[] buffer = new byte[480*480*4];
        try {
            output = new FileOutputStream(file);//Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator + "KM"+File.separator+"test.dump");
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        }
        for( i = 0 ; i < 40 ; i++ ) {
            String path = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "KM" + File.separator + "test_480X480_" + i + ".dump";
            try {
                input = new FileInputStream(path);
                input.read(buffer);
                output.write(buffer);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                try {
                    output.close();
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
                break;
            }catch (IOException e1) {
                e1.printStackTrace();
                try {
                    input.close();
                    output.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                break;
            }
            try {
                input.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        try {
            output.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return true;
    }

    void testCase2(){
        int w = 480;
        int h = 480;
        int[] pixels = new int[w * h];
        int[] colors = {0xff0000ff, 0x00ff00ff, 0x0000ffff};
        int selectColor = 0;

        String outputFile = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "KM" + File.separator + "test.gif";
        nexAniGifSDK aniGifSDK = nexAniGifSDK.createInstance();
        aniGifSDK.setCpuCoreCount(4);
        aniGifSDK.setWidth(w);
        aniGifSDK.setHeight(h);
        aniGifSDK.setDelayMs(200);

        aniGifSDK.setup();
        aniGifSDK.useBufferPool(outputFile);

        for (int frame = 0; frame < 40; frame++) {
            for (int i = 0; i < pixels.length; i++)
                pixels[i] = colors[selectColor];
            aniGifSDK.pushRGBAFrame( writeInts(pixels));

            selectColor++;
            if (selectColor >= colors.length) {
                selectColor = 0;
            }
        }
        aniGifSDK.sendEoS();
        aniGifSDK.destroy();

    }

    void testCase3(){
        new AsyncTask<Void,Void,Void>(){
            ProgressDialog dlg = new ProgressDialog(MainActivity.this);
            @Override
            protected void onPreExecute() {
                dlg.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                dlg.setMessage("exporting gif...");
                dlg.show();
                super.onPreExecute();
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                dlg.dismiss();
                super.onPostExecute(aVoid);
            }

            @Override
            protected Void doInBackground(Void... params) {
                long start = System.currentTimeMillis();
                String dumpFile = Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator + "KM"+File.separator+"dump_1440X1080.yuv";
                String outputFile = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "KM" + File.separator + "test.gif";
                nexAniGifSDK aniGifSDK = nexAniGifSDK.createInstance();
                aniGifSDK.setWidth(1440);
                aniGifSDK.setHeight(1080);
                aniGifSDK.setDelayMs(66);
                aniGifSDK.setMode(nexAniGifSDK.Mode.Quality);
                aniGifSDK.setSampleFactor(30);
                int processCount = Runtime.getRuntime().availableProcessors() * 2-2;
                if( processCount < Runtime.getRuntime().availableProcessors() )
                {
                    processCount = Runtime.getRuntime().availableProcessors();
                }
                aniGifSDK.setCpuCoreCount(processCount);
                //aniGifSDK.setCpuCoreCount(1);
                aniGifSDK.setup();
                aniGifSDK.encodeFileToFile(dumpFile,nexAniGifSDK.kFormat_YUV420,outputFile);
                aniGifSDK.destroy();
                Log.d("GIF","time="+(System.currentTimeMillis()-start)+"(ms)");
                return null;
            }
        }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
    }

    private void addThumb(String path){
        View view = mInflater.inflate(R.layout.seek_thumb_item, mGallery, false);
        ImageView img = (ImageView) view.findViewById(R.id.id_index_gallery_item_image);
        Bitmap thumb = null;
        nxImageEntry ent = null;
        try {
            ent = nxImageEntry.createEntry(path,0,0);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return;
        }
        thumb = ent.getImage();

        if (thumb == null) {
            //Log.d(TAG, "getThumbnail is null=" + clip.getPath());
        }
        imageEntries.add(ent);
        img.setImageBitmap(thumb);
        mGallery.addView(view);
    }

    private void addThumb(){
        for( nxImageEntry ent : imageEntries ) {
            View view = mInflater.inflate(R.layout.seek_thumb_item, mGallery, false);
            ImageView img = (ImageView) view.findViewById(R.id.id_index_gallery_item_image);
            Bitmap thumb = null;
            thumb = ent.getImage();

            if (thumb == null) {
                //Log.d(TAG, "getThumbnail is null=" + clip.getPath());
            }

            img.setImageBitmap(thumb);
            mGallery.addView(view);
        }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mInflater = LayoutInflater.from(this);
        mGallery = (LinearLayout) findViewById(R.id.layout_thumb_seekbar);
        final Button btgif = (Button)findViewById(R.id.button_gif);
        btgif.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                testCase3();
                //nexAniGifSDK aniGifSDK = nexAniGifSDK.createInstance();
                //aniGifSDK.pool();
                //testCase2();
                /*
                String dumpFile = Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator + "KM"+File.separator+"test.dump";
                String outputFile = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "KM" + File.separator + "test.gif";
                if( createDumpFile(dumpFile) ){
                    nexAniGifSDK aniGifSDK = nexAniGifSDK.createInstance();
                    aniGifSDK.setWidth(480);
                    aniGifSDK.setHeight(480);
                    aniGifSDK.setDelayMs(200);
                    aniGifSDK.setMode(nexAniGifSDK.Mode.Quality);
                    aniGifSDK.setSampleFactor(10);
                    aniGifSDK.setCpuCoreCount(Runtime.getRuntime().availableProcessors());
                    //aniGifSDK.setCpuCoreCount(1);
                    aniGifSDK.setup();
                    aniGifSDK.pool(dumpFile,outputFile);
                    aniGifSDK.destroy();

                    //if( aniGifSDK.encodeFileToFile(dumpFile,outputFile) < 0 ){
                    //    aniGifSDK.destroy();
                    //}

                }else {
                    new AsyncTask<Void, Void, Void>() {
                        @Override
                        protected Void doInBackground(Void... params) {
                            int w = 480;
                            int h = 480;
                            int[] pixels = new int[w * h];
                            int[] colors = {0xff0000ff, 0x00ff00ff, 0x0000ffff};
                            int selectColor = 0;

                            String outputFile = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "KM" + File.separator + "test.gif";
                            nexAniGifSDK aniGifSDK = nexAniGifSDK.createInstance();
                            aniGifSDK.setCpuCoreCount(1);
                            aniGifSDK.setWidth(w);
                            aniGifSDK.setHeight(h);
                            aniGifSDK.setDelayMs(200);

                            aniGifSDK.setup();
                            try {
                                FileOutputStream output = new FileOutputStream(outputFile);
                                for (int frame = 0; frame < 30; frame++) {
                                    for (int i = 0; i < pixels.length; i++)
                                        pixels[i] = colors[selectColor];
                                    boolean last = (frame == 29);
                                    byte[] result = aniGifSDK.encodeFrame(last, writeInts(pixels));

                                    output.write(result);
                                    selectColor++;
                                    if (selectColor >= colors.length) {
                                        selectColor = 0;
                                    }
                                }
                                output.close();

                            } catch (FileNotFoundException e) {
                                e.printStackTrace();
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                            aniGifSDK.destroy();

                            return null;
                        }

                    }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                }
*/
            }
        });

        final Button bt = (Button)findViewById(R.id.button_pickup);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent( );
                intent.setType("video/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE,false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });


        seekBarStart = (SeekBar)findViewById(R.id.seekBar_start);
        seekBarStart.setMax(10000);

        seekBarStart.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if( fromUser)
                    startTrim = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekBarEnd = (SeekBar)findViewById(R.id.seekBar_end);
        seekBarEnd.setMax(10000);
        seekBarEnd.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if( fromUser)
                    endTrim = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekBarSpeed = (SeekBar)findViewById(R.id.seekBar_speed);
        seekBarSpeed.setMax(400);
        seekBarSpeed.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if( fromUser)
                    speed = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        final Button reload =  (Button) findViewById(R.id.button_reload);
        reload.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( instance != null ){
                    nxSCMBEntry[] entry = instance.getEntries();
                    if( entry != null ){
                        if( entry.length == 3 ){
                            startTrim = entry[1].getStartMs();
                            endTrim = entry[1].getEndMs();
                            speed = entry[1].getSpeed();
                            byte[] extra = entry[1].getExtra();
                        }
                    }
                    seekBarStart.setProgress((int)startTrim);
                    seekBarEnd.setProgress((int)endTrim);
                    seekBarSpeed.setProgress(speed);
                }
            }
        });


        final Button save =  (Button) findViewById(R.id.button_save);
        save.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( instance != null ){
                    byte[] extra = null;

                    for( int i =0 ; i < extra.length ; i++ ){
                        extra[i] = (byte)(i+1);
                    }

                    nxSCMBEntry[] entries  = new nxSCMBEntry[3];

                    entries[0] = new nxSCMBEntry(100,0,startTrim,extra);
                    entries[1] = new nxSCMBEntry(speed,startTrim,endTrim,extra);
                    entries[2] = new nxSCMBEntry(100,endTrim,10000,extra);

                    instance.setEntries(entries);
                }
            }
        });

        buttonSelectImage = (Button)findViewById(R.id.button_select_image);
        buttonSelectImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent( );
                intent.setType("image/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE,true);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 200);
            }
        });

        buttonSaveImage = (Button)findViewById(R.id.button_save_image);
        buttonSaveImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d("SCMBjava","Entry count="+imageEntries.size());
                if( imageEntries.size() > 0 ){
                    if( instance != null ){
                        nxImageEntry[] entries = new nxImageEntry[imageEntries.size()];
                        for( int i = 0 ; i < imageEntries.size() ; i++ ){
                            entries[i] = imageEntries.get(i);
                        }
                        instance.setImageEntries(entries);
                    }
                }
            }
        });

        final Button reloadImage = (Button)findViewById(R.id.button_reload_image);
        reloadImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mGallery.removeAllViews();
                imageEntries.clear();
                if( instance != null ) {
                    nxImageEntry[] images = instance.getImageEntries();
                    if (images != null) {
                        for (nxImageEntry ent : images) {
                            imageEntries.add(ent);
                        }
                        addThumb();
                    }
                }
            }
        });

        /*
        String path;
        nxImageEntry entry = new nxImageEntry(path);

        nxImageEntry entry = null;
        try {
            entry = nxImageEntry.createEntry(path,0,0);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        */

        /*
        List<nxImageEntry>imageEntries = new ArrayList<>();
        nxSCMBSDK instance = nxSCMBSDK.createInstance(filepath);
        if( instance != null ) {
            // ex) load speed meta
            nxSCMBEntry[] entry = instance.getEntries();
            if (entry != null) {
                if (entry.length == 3) {
                    startTrim = entry[1].getStartMs();
                    endTrim = entry[1].getEndMs();
                    speed = entry[1].getSpeed();
                }
            }

            // ex) load Image Meta
            nxImageEntry[] images = instance.getImageEntries();
            if (images != null) {
                for (nxImageEntry ent : images) {
                    imageEntries.add(ent);
                    Bitmap thumb = ent.getImage();
                    //or
                    ent.saveFile("/sdcard/temp/image1.jpg");
                }
            }

            //ex) save image Meta
            if( imageEntries.size() > 0 ){
                if( instance != null ){
                    nxImageEntry[] entries = new nxImageEntry[imageEntries.size()];
                    for( int i = 0 ; i < imageEntries.size() ; i++ ){
                        entries[i] = imageEntries.get(i);
                    }
                    instance.setImageEntries(entries);
                }
            }
        }
        */

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 100 && resultCode == Activity.RESULT_OK) {

            ClipData clipData = data.getClipData();
            String filepath = null;

            if (clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
            } else {
                ClipData.Item item;
                Uri uri = null;
                for (int i = 0; i < clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                }
            }

            instance = nxSCMBSDK.createInstance(filepath);
            if( instance != null ){
                nxSCMBEntry[] entry = instance.getEntries();
                if( entry != null ){
                    if( entry.length == 3 ){
                        startTrim = entry[1].getStartMs();
                        endTrim = entry[1].getEndMs();
                        speed = entry[1].getSpeed();
                    }
                    Log.d("SCMBjava","Entry count="+entry.length);
                    seekBarStart.setProgress((int)startTrim);
                    seekBarEnd.setProgress((int)endTrim);
                    seekBarSpeed.setProgress(speed);
                }
                //todo:
                mGallery.removeAllViews();
                imageEntries.clear();

                nxImageEntry[] images = instance.getImageEntries();
                if( images != null ){
                    for(nxImageEntry ent : images ){
                        imageEntries.add(ent);
                    }
                    addThumb();
                }

                if( imageEntries.size() > 0 ) {
                    buttonSelectImage.setEnabled(false);
                    buttonSaveImage.setEnabled(false);
                }else{
                    buttonSelectImage.setEnabled(true);
                    buttonSaveImage.setEnabled(false);
                }

            }
        }else if(requestCode == 200 && resultCode == Activity.RESULT_OK){
            ClipData clipData = data.getClipData();
            String filepath = null;
            mGallery.removeAllViews();
            imageEntries.clear();
            if (clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                addThumb(filepath);
            } else {
                ClipData.Item item;
                Uri uri = null;
                for (int i = 0; i < clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                    addThumb(filepath);
                }
            }

            if( imageEntries.size() > 0 ) {
                buttonSaveImage.setEnabled(true);
            }else{
                buttonSaveImage.setEnabled(false);
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if( instance != null ){
            instance.destroyInstance();
        }
    }
}
