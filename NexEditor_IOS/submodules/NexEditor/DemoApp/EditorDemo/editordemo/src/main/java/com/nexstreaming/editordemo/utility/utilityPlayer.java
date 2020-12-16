package com.nexstreaming.editordemo.utility;

import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.fragment.kmSTPreviewFragment;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;

/**
 * Created by pg on 16. 1. 25..
 */
public class utilityPlayer {

    ImageView IViewBack;
    ImageView IViewPlay;
    ImageView IViewETC;

    TextView TViewCurrentPosition;
    TextView TViewTotalTime;

    SeekBar seekBar;

    kmSTPreviewFragment.OnSTPreviewListener listener;

    public void setPreviewListener(kmSTPreviewFragment.OnSTPreviewListener listener) {
        this.listener = listener;
    }

    public void setView4Back(View view) {
        IViewBack = (ImageView)view;
        IViewBack.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(getEngineStatus() != ERROR) {
                    setEngineStatus(IDLE);
                    IViewPlay.setBackgroundResource(R.mipmap.ic_play_arrow_white_72dp);
                    engine.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            if (listener != null) {
                                listener.onSTActivityBack();
                            }
                        }
                    });
                } else {
                    if (listener != null) {
                        listener.onSTActivityBack();
                    }
                }
            }
        });
    }

    public void setView4ETC(View view) {
        IViewETC = (ImageView)view;
        IViewETC.setBackgroundResource(R.mipmap.ic_launch_white_24dp);
        IViewETC.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (getEngineStatus() == ENCODING) {
                    return;
                } else if(getEngineStatus() == ERROR
                        || getEngineStatus() == ENCODING_COMPLETE) {
                    if (listener != null) {
                        listener.onSTHome();
                    }
                } else {
                    engine.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            engine.seek(0);
                            setEngineStatus(ENCODING);
                        }
                    });
                }
            }
        });
    }

    public void setView4Play(View view) {
        IViewPlay = (ImageView)view;
        IViewPlay.setBackgroundResource(R.mipmap.ic_pause_white_72dp);
        IViewPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (getEngineStatus() == ERROR) {
                    return;
                } else if (getEngineStatus() == ENCODING_COMPLETE) {
                    if (listener != null) {
                        listener.onSTEncodedFilePlay();
                    }
                } else if (getEngineStatus() == PLAYING) {
                    setPause();
                } else {
                    setPlay(false);
                }
            }
        });
    }

    public void setView4Time(View playtimeView, View durationView) {
        TViewCurrentPosition = (TextView)playtimeView;
        TViewTotalTime = (TextView)durationView;
    }


    public void setView4Seek(View view) {
        seekBar = (SeekBar)view;
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (getEngineStatus() != ERROR
                        && fromUser && trackingSeekBarTouch) {
                    playingTime = progress;
                    seekTarget = progress;
                    setSeekPosition();
                    Log.d("Preview", "seekTS:" + seekCurrentTS + " progress:" + progress + "/" + Math.abs(seekCurrentTS - progress));
                    if(Math.abs(seekCurrentTS - progress) > 150/*Tmp Value.*/) {
                        seekMode = 2;
                    } else {
                        seekMode = 1;
                    }
                    updateSeek();
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                if(getEngineStatus() == PLAYING) {
                    setEngineStatus(PAUSE);
                    engine.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            processingStopForSeek = false;
                            updateSeek();
                        }
                    });
                    processingStopForSeek = true;
                    seekTarget = -1;
                } else if(getEngineStatus() == PAUSE) {
                    processingStopForSeek = false;
                } else {
                    setPause();
                    processingStopForSeek = false;
                }
                IViewPlay.setBackgroundResource(R.mipmap.ic_play_arrow_white_72dp);
                trackingSeekBarTouch = true;
                isDrag = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                trackingSeekBarTouch = false;
                isDrag = false;
                updateSeek();
            }
        });
    }

    nexEngine engine;
    nexEngineView engineView;

    public void setEngineView(nexEngineView engineView) {
        this.engineView = engineView;
        this.engineView.setBlackOut(true);
        this.engineView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (getEngineStatus() != ENCODING_COMPLETE) {
                    if (IViewPlay.getVisibility() == View.VISIBLE) {
                        IViewPlay.setVisibility(View.INVISIBLE);
                    } else {
                        IViewPlay.setVisibility(View.VISIBLE);
                    }
                } else {
                    IViewPlay.setVisibility(View.VISIBLE);
                }
                return false;
            }
        });
    }

    public void setEngine(final nexEngine engine, final boolean bPlaying) {
        this.engine = engine;
        this.engine.setView(engineView);
        this.engine.setOnSurfaceChangeListener(new nexEngine.OnSurfaceChangeListener() {
            @Override
            public void onSurfaceChanged() {
                if(bPlaying
                    && getEngineStatus()!=ENCODING_COMPLETE) {
                    setPlay(false);
                }
            }
        });
        this.engine.setEventHandler(new nexEngineListener() {
            @Override
            public void onStateChange(int i, int i1) {

            }

            @Override
            public void onTimeChange(int currentTime) {
                playingTime = currentTime;
                setCurrentPosition();
            }

            @Override
            public void onSetTimeDone(int i) {
                if(getEngineStatus()==ENCODING) {
                    if(listener!=null) {
                        listener.onSTEncodingStart();
                    }
                } else {
                    if(finalSeekSerial==seekSerial) {
                        seekInProgress=false;
                        updateSeek();
                    }
                }
            }

            @Override
            public void onSetTimeFail(int i) {
                if(listener!=null
                        && getEngineStatus()!=ERROR) {
                    engine.stop();
                }
                setEngineStatus(ERROR);
            }

            @Override
            public void onSetTimeIgnored() {

            }

            @Override
            public void onEncodingDone(boolean isError, int error) {
                if(isError) {
                    if(listener!=null) {
                        listener.onSTEncodingFail(error);
                    }
                } else {
                    if(listener!=null) {
                        listener.onSTEncodingSuccess();
                    }
                    setEngineStatus(ENCODING_COMPLETE);
                    IViewETC.setBackground(null);
                    IViewETC.setBackgroundResource(R.mipmap.ic_home_white_24dp);
                    IViewPlay.setBackground(null);
                    IViewPlay.setBackgroundResource(R.mipmap.ic_play_arrow_white_72dp);
                    seekBar.setEnabled(false);
                }
            }

            @Override
            public void onPlayEnd() {
                setEngineStatus(COMPLETE);
                IViewPlay.setBackground(null);
                IViewPlay.setBackgroundResource(R.mipmap.ic_play_arrow_white_72dp);
            }

            @Override
            public void onPlayFail(int error, int clipID) {
                if(listener!=null) {
                    listener.onSTPlayFail(error);
                }
            }

            @Override
            public void onPlayStart() {
                availableData = true;
                seekBar.setMax(engine.getDuration());
                TViewTotalTime.setText(utilityCode.stringForTime(engine.getDuration()));
            }

            @Override
            public void onClipInfoDone() {

            }

            @Override
            public void onSeekStateChanged(boolean b) {

            }

            @Override
            public void onEncodingProgress(int percent) {
                if(listener!=null) {
                    listener.onSTEncodingProgress(percent);
                }
            }

            @Override
            public void onCheckDirectExport(int i) {

            }

            @Override
            public void onProgressThumbnailCaching(int i, int i1) {

            }

            @Override
            public void onFastPreviewStartDone(int i, int i1, int i2) {

            }

            @Override
            public void onFastPreviewStopDone(int i) {

            }

            @Override
            public void onFastPreviewTimeDone(int i) {

            }

            @Override
            public void onPreviewPeakMeter(int i, int i1) {

            }
        });
    }

    final int PLAYING = 1;
    final int PAUSE = 2;
    final int COMPLETE = 3;
    final int ENCODING = 4;
    final int ENCODING_COMPLETE = 5;
    final int IDLE = 6;
    final int ERROR = 7;

    int engieStatue = IDLE;

    int getEngineStatus() {
        return engieStatue;
    }

    void setEngineStatus(int status) {
        engieStatue = status;
    }

    int getCurrentPosition() {
        return playingTime;
    }

    void setPosition2SeekBar(int position, int duration) {
        if(availableData && duration > 0) {
            seekBar.setProgress(position);
        }
    }

    void setSeekPosition() {
        int position = seekTarget;
        setPosition2SeekBar(position, engine.getDuration());
        TViewCurrentPosition.setText(utilityCode.stringForTime(position));
    }

    void setCurrentPosition() {
        setPosition2SeekBar(getCurrentPosition(), engine.getDuration());
        TViewCurrentPosition.setText(utilityCode.stringForTime(getCurrentPosition()));
    }

    final int MSG_PROGRESS = 1;
    Handler m_Handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if(msg.what == MSG_PROGRESS) {
                setCurrentPosition();
                if(getEngineStatus()<COMPLETE && !isDrag) {
                    msg = obtainMessage(MSG_PROGRESS);
                    sendMessageDelayed(msg, (engine.getDuration()-getCurrentPosition())/1000);
                }
            }
        }
    };

    boolean processingStopForSeek = false;
    boolean seekInProgress = false;
    boolean trackingSeekBarTouch = false;
    boolean isDrag;
    boolean availableData;

    int playingTime;
    int seekTarget = -1;
    int seekSerial = 0;
    int seekCurrentTS = -1;
    int finalSeekSerial = -2;
    int seekMode = 1; /*1: Seek, 2: Seek IDR*/

    void updateSeek() {
        if(getEngineStatus() == ERROR
                || processingStopForSeek) {
            return;
        }
        if(seekTarget >= 0) {
            int st = seekTarget;
            seekTarget = -1;
            seekSerial++;
            finalSeekSerial = seekSerial;
            seekInProgress = true;
            if(seekMode == 1) /*Seek*/{
                seekCurrentTS = st;
                engine.seek(st);
            } else /*Seek IDR*/{
                seekCurrentTS = st;
                engine.seekIDROnly(st);
            }
        } else if(!seekInProgress && !trackingSeekBarTouch) {
            setPlay(false);
        }
    }

    public void setSeek(int seekTS) {
        setEngineStatus(PLAYING);

        playingTime = seekTS;
        seekTarget = seekTS;
        setSeekPosition();

        seekMode = 1;

        processingStopForSeek = false;
        updateSeek();

        IViewPlay.setBackgroundResource(R.mipmap.ic_play_arrow_white_72dp);
    }

    public void setPause() {
        setEngineStatus(PAUSE);
        IViewPlay.setBackgroundResource(R.mipmap.ic_play_arrow_white_72dp);
        engine.pause();
    }

    public void setPlay(boolean bignoreEnginStatus) {
        IViewPlay.setBackgroundResource(R.mipmap.ic_pause_white_72dp);
        if(bignoreEnginStatus) {
            setEngineStatus(PLAYING);
            engine.play();
            isDrag = false;
        } else {
            if (getEngineStatus() == IDLE
                    || getEngineStatus() == COMPLETE) {
                setEngineStatus(PLAYING);
                engine.play();
                isDrag = false;
            } else {
                setEngineStatus(PLAYING);
                engine.resume();
            }
        }
//        m_Handler.sendEmptyMessage(MSG_PROGRESS);
    }

    public void reset() {
        setEngineStatus(COMPLETE);
    }
}
