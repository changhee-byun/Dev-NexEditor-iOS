package com.nexstreaming.nexeditorsdkapis.qatest.tc;

import android.media.MediaPlayer;
import android.media.audiofx.Visualizer;
import android.net.Uri;
import android.util.Log;

import com.nexstreaming.nexeditorsdkapis.qatest.QAConfig;
import com.nexstreaming.nexeditorsdkapis.qatest.TestCase;

import java.io.IOException;
import java.net.URI;

/**
 * Created by jeongwook.yoon on 2016-05-11.
 */
public class AudioTest extends TestCase {
    private static final String TAG = "AudioTest";
    private MediaPlayer mMediaPlayer;
    private Visualizer mVisualizer;
    private boolean mStop = false;

    @Override
    protected void preTest() {
        mMediaPlayer = new MediaPlayer();
        try {
            mMediaPlayer.setDataSource(QAConfig.getContentPath("fade_in.mp3"));
            mMediaPlayer.prepare();
            mMediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
                @Override
                public void onCompletion(MediaPlayer mp) {
                    mVisualizer.setEnabled(false);
                    unLock(1);
                }
            });

            mVisualizer = new Visualizer(mMediaPlayer.getAudioSessionId());
            mVisualizer.setCaptureSize(Visualizer.getCaptureSizeRange()[1]);
            mVisualizer.setDataCaptureListener(new Visualizer.OnDataCaptureListener() {
                @Override
                public void onWaveFormDataCapture(Visualizer visualizer, byte[] waveform, int samplingRate) {

                    int energy = computeEnergy(waveform,true);
                    int time = mMediaPlayer.getCurrentPosition();
                    Log.d(TAG, "["+time+"]waveform.energy=" + energy + ", samplingRate=" + samplingRate);
                    if( mStop ){
                        mMediaPlayer.stop();
                        mVisualizer.setEnabled(false);
                        unLock(1);
                    }
                }

                @Override
                public void onFftDataCapture(Visualizer visualizer, byte[] fft, int samplingRate) {
                    int energy = computeEnergy(fft,false);
                    Log.d(TAG, "fft.energy=" + energy + ", samplingRate=" + samplingRate);
                }
            }, Visualizer.getMaxCaptureRate() / 2, true, false);

            mVisualizer.setEnabled(true);
            mMediaPlayer.start();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected boolean postTest(int lockNum) {
        mMediaPlayer.release();
        mVisualizer.release();
        resultOk("test complete!");
        return false;
    }

    private int computeEnergy(byte[] data, boolean pcm) {
        int energy = 0;
        if (data.length != 0) {
            if (pcm) {
                for (int i = 0; i < data.length; i++) {
                    int tmp = ((int)data[i] & 0xFF) - 128;
                    energy += tmp*tmp;
                }
            } else {
                energy = (int)data[0] * (int)data[0];
                for (int i = 2; i < data.length; i += 2) {
                    int real = (int)data[i];
                    int img = (int)data[i + 1];
                    energy += real * real + img * img;
                }
            }
        }
        return energy;
    }

    @Override
    protected void cancel() {
        mStop = true;
    }
}
