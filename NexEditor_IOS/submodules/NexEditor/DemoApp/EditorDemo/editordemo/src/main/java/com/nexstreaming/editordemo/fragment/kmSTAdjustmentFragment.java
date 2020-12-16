package com.nexstreaming.editordemo.fragment;


import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;
import android.widget.TextView;

import com.nexstreaming.editordemo.R;

/**
 * A simple {@link Fragment} subclass.
 */
public class kmSTAdjustmentFragment extends Fragment implements SeekBar.OnSeekBarChangeListener {

    private SeekBar sb_brightness;
    private SeekBar sb_contrast;
    private SeekBar sb_saturation;

    private TextView tv_brightness;
    private TextView tv_contrast;
    private TextView tv_saturation;

    private int progress;
    private int brightness;
    private int contrast;
    private int saturation;

    private OnSTAdjustmentSelectedListener mListener;


    public interface OnSTAdjustmentSelectedListener {
        public void onStAdjustmentSelected(String value, int control_value);
    }

    void setValues(int brightness, int contrast, int saturation) {
        this.brightness = (brightness+250)/5;
        this.contrast = (contrast+250)/5;
        this.saturation = (saturation+250)/5;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Bundle bundle = getArguments();
        setValues(bundle.getInt("bright"), bundle.getInt("contrast"), bundle.getInt("saturation"));

        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_km_stadjustment, container, false);

        sb_brightness = (SeekBar) v.findViewById(R.id.bright_control);
        sb_brightness.setMax(100);
        sb_brightness.setProgress(brightness);
        sb_brightness.setOnSeekBarChangeListener(this);

        sb_contrast = (SeekBar) v.findViewById(R.id.contrast_control);
        sb_contrast.setMax(100);
        sb_contrast.setProgress(contrast);
        sb_contrast.setOnSeekBarChangeListener(this);

        sb_saturation = (SeekBar) v.findViewById(R.id.saturation_control);
        sb_saturation.setMax(100);
        sb_saturation.setProgress(saturation);
        sb_saturation.setOnSeekBarChangeListener(this);

        tv_brightness = (TextView) v.findViewById(R.id.bright_value);
        tv_brightness.setText(String.valueOf(sb_brightness.getProgress()));

        tv_contrast = (TextView) v.findViewById(R.id.contrast_value);
        tv_contrast.setText(String.valueOf(sb_contrast.getProgress()));

        tv_saturation = (TextView) v.findViewById(R.id.saturation_value);
        tv_saturation.setText(String.valueOf(sb_saturation.getProgress()));

        return v;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (OnSTAdjustmentSelectedListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnSTAdjustmentSelectedListener");
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        this.progress = progress;
        switch (seekBar.getId()) {
            case R.id.bright_control:
                tv_brightness.setText(String.valueOf(progress));
                break;
            case R.id.contrast_control:
                tv_contrast.setText(String.valueOf(progress));
                break;
            case R.id.saturation_control:
                tv_saturation.setText(String.valueOf(progress));
                break;
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        switch (seekBar.getId()) {
            case R.id.bright_control:
                if(mListener != null) {
                    mListener.onStAdjustmentSelected("bright", progress*5-250);
                }
                break;
            case R.id.contrast_control:
                if(mListener != null) {
                    mListener.onStAdjustmentSelected("contrast", progress*5-250);
                }
                break;
            case R.id.saturation_control:
                if(mListener != null) {
                    mListener.onStAdjustmentSelected("saturation", progress*5-250);
                }
                break;
        }
    }
}
