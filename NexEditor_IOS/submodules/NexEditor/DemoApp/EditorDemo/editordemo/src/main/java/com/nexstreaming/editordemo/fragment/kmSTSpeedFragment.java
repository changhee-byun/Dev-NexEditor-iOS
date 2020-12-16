package com.nexstreaming.editordemo.fragment;


import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.nexstreaming.editordemo.R;

/**
 * A simple {@link Fragment} subclass.
 */
public class kmSTSpeedFragment extends Fragment implements View.OnClickListener {

    private ImageView iv_speed_0_25;
    private ImageView iv_speed_0_5;
    private ImageView iv_speed_1_0;
    private ImageView iv_speed_1_5;
    private ImageView iv_speed_2_0;

    private OnSTSpeedSelectedListener mListener;

    public interface OnSTSpeedSelectedListener {
        public void onStSpeedSelected(int speed);
    }

    void setValues(int speed) {
        if(speed == 25) {
            setSelectedView(R.id.speedcontrol_select_025);
        } else if(speed == 50) {
            setSelectedView(R.id.speedcontrol_select_050);
        } else if(speed == 100) {
            setSelectedView(R.id.speedcontrol_select_100);
        } else if(speed == 150) {
            setSelectedView(R.id.speedcontrol_select_150);
        } else if(speed == 200) {
            setSelectedView(R.id.speedcontrol_select_200);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_km_stspeed, container, false);

        iv_speed_0_25 = (ImageView) v.findViewById(R.id.speedcontrol_select_025);
        iv_speed_0_25.setOnClickListener(this);
        iv_speed_0_5 = (ImageView) v.findViewById(R.id.speedcontrol_select_050);
        iv_speed_0_5.setOnClickListener(this);
        iv_speed_1_0 = (ImageView) v.findViewById(R.id.speedcontrol_select_100);
        iv_speed_1_0.setOnClickListener(this);
        iv_speed_1_5 = (ImageView) v.findViewById(R.id.speedcontrol_select_150);
        iv_speed_1_5.setOnClickListener(this);
        iv_speed_2_0 = (ImageView) v.findViewById(R.id.speedcontrol_select_200);
        iv_speed_2_0.setOnClickListener(this);

        Bundle bundle = getArguments();
        setValues(bundle.getInt("speed"));

        return v;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (OnSTSpeedSelectedListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnSTSpeedSelectedListener");
        }
    }

    void setInitView() {
        iv_speed_0_25.setImageResource(R.mipmap.ic_speed_025x_nor);
        iv_speed_0_5.setImageResource(R.mipmap.ic_speed_05x_nor);
        iv_speed_1_0.setImageResource(R.mipmap.ic_speed_10x_nor);
        iv_speed_1_5.setImageResource(R.mipmap.ic_speed_15x_nor);
        iv_speed_2_0.setImageResource(R.mipmap.ic_speed_20x_nor);
    }

    void setSelectedView(int idx) {
        switch(idx) {
            case R.id.speedcontrol_select_025:
                iv_speed_0_25.setImageResource(R.mipmap.ic_speed_025x_focus);
                break;
            case R.id.speedcontrol_select_050:
                iv_speed_0_5.setImageResource(R.mipmap.ic_speed_05x_focus);
                break;
            case R.id.speedcontrol_select_100:
                iv_speed_1_0.setImageResource(R.mipmap.ic_speed_10x_focus);
                break;
            case R.id.speedcontrol_select_150:
                iv_speed_1_5.setImageResource(R.mipmap.ic_speed_15x_focus);
                break;
            case R.id.speedcontrol_select_200:
                iv_speed_2_0.setImageResource(R.mipmap.ic_speed_20x_focus);
                break;
        }
    }

    @Override
    public void onClick(View v) {
        setInitView();

        switch(v.getId()) {
            case R.id.speedcontrol_select_025:
                setSelectedView(R.id.speedcontrol_select_025);
                v.invalidate();
                if(mListener != null) {
                    mListener.onStSpeedSelected(25);
                }
                break;
            case R.id.speedcontrol_select_050:
                setSelectedView(R.id.speedcontrol_select_050);
                v.invalidate();
                if(mListener != null) {
                    mListener.onStSpeedSelected(50);
                }
                break;
            case R.id.speedcontrol_select_100:
                setSelectedView(R.id.speedcontrol_select_100);
                v.invalidate();
                if(mListener != null) {
                    mListener.onStSpeedSelected(100);
                }
                break;
            case R.id.speedcontrol_select_150:
                setSelectedView(R.id.speedcontrol_select_150);
                v.invalidate();
                if(mListener != null) {
                    mListener.onStSpeedSelected(150);
                }
                break;
            case R.id.speedcontrol_select_200:
                setSelectedView(R.id.speedcontrol_select_200);
                v.invalidate();
                if(mListener != null) {
                    mListener.onStSpeedSelected(200);
                }
                break;
        }
    }
}
