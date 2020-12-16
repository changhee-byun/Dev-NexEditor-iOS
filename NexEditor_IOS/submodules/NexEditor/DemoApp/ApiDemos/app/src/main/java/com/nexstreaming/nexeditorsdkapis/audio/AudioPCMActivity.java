package com.nexstreaming.nexeditorsdkapis.audio;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.ClipData;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdkapis.common.Stopwatch;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.AudioDecoder;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;

public class AudioPCMActivity extends Activity {
    private static final String TAG="AudioPCMActivity";
    private String m_soundfilepath;
    private int mDuration;
    private int mSampleRate;
    private boolean endEos;
    private Button btAudio;
    private Button btAndroidPCM;
    private Button btNexPCM;
    private Button btPCMPlay;
    private TextView tvFilePath;
    private TextView tvDuration;
    private TextView tvResult;
    private File dumpPcm ;
    private boolean isPCMPlaying;
    private StringBuilder mResult;
    private Stopwatch time = new Stopwatch();
    private ProgressDialog dialog;
    private ImageView mImageViewAudioPCM;
    private SeekBar seekStartTime;
    private SeekBar seekEndTime;
    private SeekBar seekUse;
    private SeekBar seekSkip;
    private TextView tvStartTime;
    private TextView tvEndTime;
    private TextView tvUse;
    private TextView tvSkip;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_pcm);

        mResult = new StringBuilder();

        btAudio = (Button)findViewById(R.id.button_get_audio_file);
        btAudio.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                endEos = false;
                Intent intent = new Intent( );
                intent.setType("audio/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);

            }
        });

        btAndroidPCM = (Button)findViewById(R.id.button_pcm_android);
        btAndroidPCM.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dumpAndroidPcm();
            }
        });
        btNexPCM = (Button)findViewById(R.id.button_pcm_nex);
        btNexPCM.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( -1 ==dumpNexPcmLevel())
                    Toast.makeText(AudioPCMActivity.this, "NotSupportClip", Toast.LENGTH_LONG).show();

            }
        });
        btPCMPlay = (Button)findViewById(R.id.button_pcm_play);
        btPCMPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( isPCMPlaying ){
                    dumpPlayStop();
                }else{
                    dumpPcmPlayAsync();
                }
            }
        });

        tvFilePath = (TextView)findViewById(R.id.textView_pcm_filepath);
        tvDuration = (TextView)findViewById(R.id.textView_pcm_duration);
        tvResult = (TextView)findViewById(R.id.textView_pcm_result);

        tvStartTime = (TextView)findViewById(R.id.textView_pcm_start);
        tvEndTime = (TextView)findViewById(R.id.textView_pcm_end);
        tvUse = (TextView)findViewById(R.id.textView_pcm_use);
        tvSkip = (TextView)findViewById(R.id.textView_pcm_skip);

        seekStartTime = (SeekBar)findViewById(R.id.seekBar_pcm_start);
        seekEndTime = (SeekBar)findViewById(R.id.seekBar_pcm_end);
        seekUse = (SeekBar)findViewById(R.id.seekBar_pcm_use);
        seekSkip = (SeekBar)findViewById(R.id.seekBar_pcm_skip);

        seekStartTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                tvStartTime.setText(""+progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });


        seekEndTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                tvEndTime.setText(""+progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekUse.setMax(9);
        seekUse.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                tvUse.setText(""+progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekSkip.setMax(9);
        seekSkip.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                tvSkip.setText(""+progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        btAndroidPCM.setEnabled(false);
        btNexPCM.setEnabled(false);
        btPCMPlay.setEnabled(false);

        mImageViewAudioPCM = (ImageView)findViewById(R.id.imageview_pcm_level);
    }

    void updateTextViewResult(final String str)
    {
        mResult.append(str+"\n");
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                tvResult.setText(mResult.toString());
            }
        });
    }


    void dumpPlayStop(){
        endEos = true;
    }

    void dumpPcmPlayAsync(){
        endEos = false;
        new AsyncTask<Void,Void,Void>(){
            @Override
            protected void onPreExecute() {
                btAudio.setEnabled(false);
                btAndroidPCM.setEnabled(false);
                btNexPCM.setEnabled(false);
                btPCMPlay.setText("Stop");


                updateTextViewResult("PCM Play start...");
                isPCMPlaying = true;
                super.onPreExecute();
            }

            @Override
            protected Void doInBackground(Void... params) {
                dumpPcmPlay();
                return null;
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                btAudio.setEnabled(true);
                btAndroidPCM.setEnabled(true);
                btNexPCM.setEnabled(true);
                btPCMPlay.setEnabled(false);
                super.onPostExecute(aVoid);
                updateTextViewResult("PCM Play stop...");
                isPCMPlaying = false;
            }
        }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
    }

    void dumpPcmPlay(){
        Log.d(TAG,"dumpPcmPlay() start");
        int buffsize = AudioTrack.getMinBufferSize(mSampleRate, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT);
        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, mSampleRate,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT,
                buffsize,
                AudioTrack.MODE_STREAM);
        audioTrack.play();
        try {
            int nRead = 0;
            byte[] buffer = new byte[4608];
            FileInputStream is = new FileInputStream(dumpPcm);
            while(!endEos) {
                try {
                    nRead = is.read(buffer);
                    audioTrack.write(buffer, 0, nRead);
                    if( nRead <= 0 ){
                        break;
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            try {
                is.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        Log.d(TAG,"dumpPcmPlay() stop");
        audioTrack.stop();
        audioTrack.release();
        //dumpPcm.delete();
    }

    boolean isValidateTime(int start,int end, int use, int skip, int total){
        int duration = end - start;

        if( end > total ){
            return false;
        }

        if( duration < 1000 ){
            return false;
        }

        return true;
    }

    int dumpNexPcmLevel(){
        nexClip clip = nexClip.getSupportedClip(m_soundfilepath);
        if( clip == null ){
            return -1 ;
        }
        mDuration = clip.getTotalTime();
        btAudio.setEnabled(false);
        btAndroidPCM.setEnabled(false);
        btNexPCM.setEnabled(false);
        btPCMPlay.setEnabled(false);
        time.reset();
        time.start();

        int start = seekStartTime.getProgress();
        int end = seekEndTime.getProgress();
        int use = seekUse.getProgress();
        int skip = seekSkip.getProgress();



        dialog = new ProgressDialog(this);
        dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dialog.setMessage("get nex pcm data...");
        dialog.show();

        if( isValidateTime(start,end,use,skip,mDuration) ) {


            clip.getAudioPcmLevels(start, end, use, skip, new nexClip.OnGetAudioPcmLevelsResultListener() {
                @Override
                public void onGetAudioPcmLevelsResult(byte[] pcmLevels) {
                    time.stop();
                    dialog.dismiss();
                    updateTextViewResult("NexPcm elapsed=" + time.toString());
                    btAudio.setEnabled(true);
                    btAndroidPCM.setEnabled(true);
                    btNexPCM.setEnabled(true);
                    btPCMPlay.setEnabled(true);
                    makePCMBitmap(mDuration, pcmLevels, 0);
                    //drawPCMBitmap(mDuration,pcmLevels);
                }
            });
        }else{
            clip.getAudioPcmLevels(new nexClip.OnGetAudioPcmLevelsResultListener() {
                @Override
                public void onGetAudioPcmLevelsResult(byte[] pcmLevels) {
                    time.stop();
                    dialog.dismiss();
                    updateTextViewResult("NexPcm elapsed=" + time.toString());
                    btAudio.setEnabled(true);
                    btAndroidPCM.setEnabled(true);
                    btNexPCM.setEnabled(true);
                    btPCMPlay.setEnabled(true);
                    makePCMBitmap(mDuration, pcmLevels, 0);
                    //drawPCMBitmap(mDuration,pcmLevels);
                }
            });
        }
        return 0;
    }

    void dumpAndroidPcm(){
        btAudio.setEnabled(false);
        btAndroidPCM.setEnabled(false);
        btNexPCM.setEnabled(false);
        btPCMPlay.setEnabled(false);

        AudioDecoder a = new AudioDecoder();
        a.setDataSource(m_soundfilepath);
        a.prepare();
        mDuration = (int)a.getTotalDurationMs();
        mSampleRate = a.getSampleRate();
        dumpPcm = new File(getExternalFilesDir(null),"dump_"+a.getSampleRate()+"_"+a.getChannelCount()+".pcm");

        try {
            FileOutputStream os = new FileOutputStream(dumpPcm);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        a.setOnReceivedDataCallback(new AudioDecoder.OnReceivedDataCallback() {
            FileOutputStream os ;
            @Override
            public boolean onReceivedData(byte[] rawData) {
                if( os == null ){
                    try {
                        os = new FileOutputStream(dumpPcm);
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    }
                }

                if( os != null ){
                    try {
                        os.write(rawData);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                //TODO : pcmdata
                if( endEos )
                    return true;
                return false;
            }

            @Override
            public void onEos(AudioDecoder t) {
                //End
                try {
                    os.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                t.release();
                time.stop();
                updateTextViewResult("android PCM elapsed="+time.toString());
                Log.d(TAG,"dump End() file="+dumpPcm.getAbsolutePath());
                dialog.dismiss();
                btAudio.setEnabled(true);
                btAndroidPCM.setEnabled(true);
                btNexPCM.setEnabled(true);
                btPCMPlay.setEnabled(true);
            }

            @Override
            public void onCancel(AudioDecoder t) {
                try {
                    os.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                t.release();
                dialog.dismiss();
                btAudio.setEnabled(true);
                btAndroidPCM.setEnabled(true);
                btNexPCM.setEnabled(true);
                btPCMPlay.setEnabled(true);
            }
        });

        dialog = new ProgressDialog(this);
        dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dialog.setMessage("get android pcm data...");
        dialog.show();
        time.reset();
        time.start();
        a.decode();

    }



    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 100 && resultCode == Activity.RESULT_OK) {
            ClipData clipData = data.getClipData();
            String duration = null;

            if (clipData == null) {
                m_soundfilepath = ConvertFilepathUtil.getPath(this, data.getData());
            } else {
                ClipData.Item item;
                Uri uri = null;
                item = clipData.getItemAt(0);
                uri = item.getUri();
                m_soundfilepath = ConvertFilepathUtil.getPath(this, uri);
            }
            MediaMetadataRetriever metadataRetriever = new MediaMetadataRetriever();
            metadataRetriever.setDataSource(m_soundfilepath);

            duration = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
            tvFilePath.setText(m_soundfilepath);
            if (duration == null) {
                duration = "0";
            }
            seekStartTime.setMax(Integer.parseInt(duration));
            seekStartTime.setProgress(0);
            seekEndTime.setMax(Integer.parseInt(duration));
            seekEndTime.setProgress(Integer.parseInt(duration));
            tvDuration.setText(duration);
            btAudio.setEnabled(true);
            btAndroidPCM.setEnabled(true);
            btNexPCM.setEnabled(true);
            btPCMPlay.setEnabled(false);

        }
    }

    private void makePCMBitmap(int duration, byte[] pcmData, int drawMode) {
        if (pcmData == null) {
            return ;
        }

        Paint p = new Paint();

        int originalDuration = duration;
        int width = Math.min(originalDuration / 15,8192);
        int height = 128;
        int level = -1;
        int prevLevel = -1;

        if(width <= 0 || height <= 0)
            return ;

        Log.d(TAG,"Make PCM thumbnail: " + width + "x" + height);

        Bitmap bm = Bitmap.createBitmap(width, height, Bitmap.Config.ALPHA_8);
        Canvas canvas = new Canvas(bm);
        p.setAntiAlias(true);

        Path path = new Path();
        if( drawMode == 0 ) {
            int prevIndex = -1;
            path.moveTo(-50, height + 1);

            for (int x = 0; x < width; x += 3) {
                int index = (int) ((long) x * (long) pcmData.length / (long) width);

                if (pcmData.length <= 0)
                    return;

                if (index != prevIndex) {
                    level = 0;
                    for (int i = prevIndex + 1; i <= index; i++) {
                        level += ((int) pcmData[index]) & 0xFF;
                    }
                    level /= Math.max(1, (index - prevIndex));
                    prevIndex = index;
                }

                if (level != prevLevel) {
                    path.lineTo(x, height - (level * height / 255));
                    prevLevel = level;
                }
            }

            path.lineTo(width - 1, height - (level * height / 255));
            path.lineTo(width + 50, height + 1);
            path.close();
            canvas.drawPath(path, p);
        }else {

            float[] points = new float[pcmData.length*4];
            for (int i = 0; i < pcmData.length - 1; i++) {
                points[i * 4] = width * i / (pcmData.length - 1);
                points[i * 4 + 1] = height / 2
                        + ((byte) (pcmData[i] + 128)) * (height / 2) / 128;
                points[i * 4 + 2] = width * (i + 1) / (pcmData.length - 1);
                points[i * 4 + 3] = height / 2
                        + ((byte) (pcmData[i + 1] + 128)) * (height / 2) / 128;
            }
            canvas.drawLines(points, p);
        }
        mImageViewAudioPCM.setBackgroundColor(Color.WHITE);
        mImageViewAudioPCM.setImageBitmap(bm);
    }

    @Override
    protected void onStop() {
        endEos = true;
        super.onStop();
    }

    @Override
    protected void onStart() {
        endEos = false;
        super.onStart();
    }

    @Override
    protected void onDestroy() {
        if( dumpPcm !=null ) {
            dumpPcm.delete();
        }
        super.onDestroy();
    }
}
