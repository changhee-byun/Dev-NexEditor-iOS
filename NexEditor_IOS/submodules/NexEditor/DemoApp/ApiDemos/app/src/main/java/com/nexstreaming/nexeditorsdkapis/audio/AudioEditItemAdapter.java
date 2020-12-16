/******************************************************************************
 * File Name        : AudioEditItemAdapter.java
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

package com.nexstreaming.nexeditorsdkapis.audio;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.lang.ref.WeakReference;

/**
 * Created by jeongwook.yoon on 2015-04-22.
 */
public class AudioEditItemAdapter extends BaseAdapter {
    private final String TAG = "AudioEditItemAdapter";
    private nexProject mProject;

    private int mMusicEffect = -1;
    private boolean isTrackingPanLeftSeekBar = false;
    private boolean isTrackingPanRightSeekBar = false;

    public AudioEditItemAdapter(nexProject project) {
        mProject = project;
    }

    @Override
    public int getCount() {
        return mProject.getTotalClipCount(true) + mProject.getTotalClipCount(false);
    }

    @Override
    public Object getItem(int position) {
        return mProject.getClip(position, true);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(final int position, View convertView, final ViewGroup parent) {

        // because primary -> video, secondary -> audio
        // index 0 = video, index 1 ... ~ = audio
        final Context context = parent.getContext();

        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.audio_edit_item, parent, false);
        }

        if ( mProject.getTotalClipCount(true) == 0 ) {
            return convertView;
        }

        final nexClip clip = mProject.getClip(position == 0 ? position : position - 1, position == 0 ? true : false);
        // thumbnail
        if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
            if(clip.getMainThumbnail(240f, context.getResources().getDisplayMetrics().density) == null) {
                new AsyncThumbnail((ImageView) convertView.findViewById(R.id.imageview_audio_edit_item), position == 0 ? position : position - 1);
            } else {
                ImageView iv = (ImageView) convertView.findViewById(R.id.imageview_audio_edit_item);
                iv.setImageBitmap(clip.getMainThumbnail(240f, context.getResources().getDisplayMetrics().density));
            }
        }
        else {
            ImageView iv = (ImageView) convertView.findViewById(R.id.imageview_audio_edit_item);
            iv.setImageDrawable(context.getResources().getDrawable(R.drawable.ic_music));
        }

        // path
        TextView tvPath = (TextView) convertView.findViewById(R.id.textview_audio_edit_item_path);
        tvPath.setText("" + clip.getPath());


        // event listener
        Button btnVoice = (Button) convertView.findViewById(R.id.button_audio_edit_item_voice);
        btnVoice.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                final CharSequence menuVoiceChanger[] = new CharSequence[]{"0", "1", "2", "3", "4"};

                AlertDialog.Builder builder = new AlertDialog.Builder(context);
                builder.setTitle("change clip voice changer");
                builder.setItems(menuVoiceChanger, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        clip.setVoiceChangerFactor(Integer.parseInt(menuVoiceChanger[which].toString()));
                    }
                });
                builder.show();
            }
        });

        Button btnSpeed = (Button) convertView.findViewById(R.id.button_audio_edit_item_speed);
        if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
            btnSpeed.setEnabled(true);
            btnSpeed.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    final CharSequence menuSpeed[] = new CharSequence[]{"12", "25", "50", "75", "100", "125","150","175","200","400"};

                    AlertDialog.Builder builder = new AlertDialog.Builder(context);
                    builder.setTitle("change clip speed");
                    builder.setItems(menuSpeed, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            clip.getVideoClipEdit().setSpeedControl(Integer.parseInt(menuSpeed[which].toString()));
                        }
                    });
                    builder.show();
                }
            });
        }
        else {
            btnSpeed.setEnabled(false);
        }

        Button btnCompressor = (Button) convertView.findViewById(R.id.button_audio_edit_item_compressor);
        btnCompressor.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                final CharSequence menuCompressor[] = new CharSequence[]{"0", "1", "2", "3", "4", "5", "6", "7"};

                AlertDialog.Builder builder = new AlertDialog.Builder(context);
                builder.setTitle("change clip compressor");
                builder.setItems(menuCompressor, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Log.d(TAG, "setCompressor : " + menuCompressor[which].toString());
                        clip.getAudioEdit().setCompressor(Integer.parseInt(menuCompressor[which].toString()));

                    }
                });
                builder.show();
            }
        });

        Button btnPitch = (Button) convertView.findViewById(R.id.button_audio_edit_item_pitch);
        btnPitch.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                final CharSequence menuPitch[] = new CharSequence[]{"-6", "-5", "-4", "-3", "-2", "-1", "0", "1", "2", "3", "4", "5", "6"};

                AlertDialog.Builder builder = new AlertDialog.Builder(context);
                builder.setTitle("change clip pitch");
                builder.setItems(menuPitch, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Log.d(TAG, "setPitch : " + menuPitch[which].toString());
                        clip.getAudioEdit().setPitch(Integer.parseInt(menuPitch[which].toString()));

                    }
                });
                builder.show();
            }
        });

        Button btnMusicEffect = (Button) convertView.findViewById(R.id.button_audio_edit_item_music_effect);
        btnMusicEffect.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                LayoutInflater factory = LayoutInflater.from(context);
                final View dialogView = factory.inflate(R.layout.dialog_audio_edit_item_music_effect, null);
                final RadioButton rbtnMusicEffectNone = (RadioButton) dialogView.findViewById(R.id.radiobutton_audio_edit_item_music_effect_none);
                mMusicEffect = 0;
                rbtnMusicEffectNone.setOnClickListener(new Button.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Log.d(TAG, "rbtnMusicEffectNone click");
                        mMusicEffect = 0;
                    }
                });

                final RadioButton rbtnMusicEffectLiveConcert = (RadioButton) dialogView.findViewById(R.id.radiobutton_audio_edit_item_music_effect_live_concert);
                rbtnMusicEffectLiveConcert.setOnClickListener(new Button.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Log.d(TAG, "rbtnMusicEffectLiveConcert click");
                        mMusicEffect = 1;

                    }
                });

                final RadioButton rbtnMusicEffectStereoChorus = (RadioButton) dialogView.findViewById(R.id.radiobutton_audio_edit_item_music_effect_stereo_chorus);
                rbtnMusicEffectStereoChorus.setOnClickListener(new Button.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Log.d(TAG, "rbtnMusicEffectStereoChorus click");
                        mMusicEffect = 2;
                    }
                });

                final RadioButton rbtnMusicEffectMusicEnhancer = (RadioButton) dialogView.findViewById(R.id.radiobutton_audio_edit_item_music_effect_music_enhancer);
                rbtnMusicEffectMusicEnhancer.setOnClickListener(new Button.OnClickListener() {
                    @Override
                    public void onClick(View v) {

                        Log.d(TAG, "rbtnMusicEffectMusicEnhancer click");
                        mMusicEffect = 3;
                    }
                });

                final Spinner spinnerProcessorStrength = (Spinner) dialogView.findViewById(R.id.spinner_audio_edit_item_music_effect_processor_strength);
                Integer[] processStrengthItems = new Integer[]{0, 1, 2, 3, 4, 5, 6};
                ArrayAdapter<Integer> processStrengthAdapter = new ArrayAdapter<Integer>(context, R.layout.simple_spinner_dropdown_item_1, processStrengthItems);

                spinnerProcessorStrength.setAdapter(processStrengthAdapter);
                spinnerProcessorStrength.setSelection(0);

                final Spinner spinnerBassStrength = (Spinner) dialogView.findViewById(R.id.spinner_audio_edit_item_music_effect_bass_strength);
                Integer[] bassStrengthItems = new Integer[]{0, 1, 2, 3, 4, 5, 6};
                ArrayAdapter<Integer> bassStrengthAdapter = new ArrayAdapter<Integer>(context, R.layout.simple_spinner_dropdown_item_1, bassStrengthItems);

                spinnerBassStrength.setAdapter(bassStrengthAdapter);
                spinnerBassStrength.setSelection(0);

                final AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
                alertDialogBuilder.setView(dialogView);
                alertDialogBuilder.setTitle("Music Effect");
                alertDialogBuilder.setCancelable(false);
                alertDialogBuilder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Log.d(TAG, "btnMusicEffectOK");
                        dialog.dismiss();
                        Integer processorStrengthValue = (Integer) spinnerProcessorStrength.getSelectedItem();
                        Integer bassStrengthValue = (Integer) spinnerBassStrength.getSelectedItem();

                        Log.d(TAG, "processorStrength : " + processorStrengthValue);
                        Log.d(TAG, "bassStrengthValue : " + bassStrengthValue);
                        Log.d(TAG, "musicEffect : " + mMusicEffect);

                        clip.getAudioEdit().setMusicEffect(mMusicEffect);
                        clip.getAudioEdit().setProcessorStrength(processorStrengthValue);
                        clip.getAudioEdit().setBassStrength(bassStrengthValue);


                    }
                });

                alertDialogBuilder.show();
            }
        });

        Button btnJson = (Button) convertView.findViewById(R.id.button_audio_edit_item_json);
        btnJson.setText("J");
        btnJson.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                File file = new File("sdcard/nex/");
                final File[] children = file.listFiles();

                if ( children==null || children.length == 0 ) {
                    Log.d(TAG, "No files in "+file.getPath());
                    return;
                }

                final CharSequence menuVoiceChanger[] = new CharSequence[children.length];
                for (int i=0; i<children.length; i++) {
                    menuVoiceChanger[i] = children[i].getName();
                }
                AlertDialog.Builder builder = new AlertDialog.Builder(context);
                builder.setTitle("change clip voice changer");
                builder.setItems(menuVoiceChanger, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        String textFileName = children[which].getAbsolutePath();
                        try {
                            BufferedReader in = new BufferedReader(new FileReader(textFileName));
                            StringBuilder contentReceiver = new StringBuilder();
                            String s;

                            while ((s = in.readLine()) != null) {
                                contentReceiver.append(s);
                                contentReceiver.append("\n");
                            }
                            in.close();

                            clip.getAudioEdit().setEnhancedAudioFilter(contentReceiver.toString());
                            Log.d(TAG, "================"+clip.getAudioEdit().getEnhancedAudioFilter().length()+ "======================");
                            Log.d(TAG, "textFileName=" + textFileName);
                            //Log.d(TAG, "textFile=" + contentReceiver.toString());
                            Log.d(TAG, clip.getAudioEdit().getEnhancedAudioFilter());
                            Log.d(TAG, "++++++++++++++++++++++++++++++++++++++++++++++");
                        }
                        catch (FileNotFoundException e) {
                            Log.d(TAG, textFileName + " File is not exist");
                            e.printStackTrace();
                        }
                        catch (IOException e) {
                            Log.d(TAG, textFileName + " Can't read file");
                            e.printStackTrace();
                        }
                    }
                });
                builder.show();
            }
        });

        SeekBar seekBarLeftPan = (SeekBar)convertView.findViewById(R.id.seekbar_audio_edit_item_left_pan);
        seekBarLeftPan.setMax(200);


        final TextView textviewLeftPan = (TextView)convertView.findViewById(R.id.textview_audio_edit_item_left_pan);
        seekBarLeftPan.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                isTrackingPanLeftSeekBar = true;
                value = progress - 100;
                Log.d("Parkminsu", "onProgressChanged progress: " + value);
                textviewLeftPan.setText("" + value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (isTrackingPanLeftSeekBar != true)
                    return;

                Log.d("Parkminsu", "onStopTrackingTouch progress: " + value);
                textviewLeftPan.setText("" + value);
                clip.getAudioEdit().setPanLeft(value);
            }
        });

        SeekBar seekBarRightPan = (SeekBar)convertView.findViewById(R.id.seekbar_audio_edit_item_right_pan);
        seekBarRightPan.setMax(200);


        final TextView textviewRightPan = (TextView)convertView.findViewById(R.id.textview_audio_edit_item_right_pan);
        seekBarRightPan.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                isTrackingPanRightSeekBar = true;
                value = progress - 100;
                textviewRightPan.setText(""+ value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if(isTrackingPanRightSeekBar != true)
                    return ;

                textviewRightPan.setText("" + value);
                clip.getAudioEdit().setPanRight(value);
            }
        });

        return convertView;
    }

    class AsyncThumbnail{
        private final WeakReference<ImageView> imageViewReference;
        private final int mPos;

        public AsyncThumbnail(ImageView imageView, int pos ){
            imageViewReference = new WeakReference<ImageView>(imageView);
            mPos = pos;
            mProject.getClip(mPos, true).loadVideoClipThumbnails(new nexClip.OnLoadVideoClipThumbnailListener() {
                @Override
                public void onLoadThumbnailResult(int event) {
                    if (event == 0) {
                        final ImageView imageView = imageViewReference.get() ;
                        if (imageView != null) {
                            imageView.setImageBitmap(mProject.getClip(mPos, true).getMainThumbnail(240f, 0f ));
                        }
                    } else {
                        ;
                    }
                }
            });
        }
    }
}
