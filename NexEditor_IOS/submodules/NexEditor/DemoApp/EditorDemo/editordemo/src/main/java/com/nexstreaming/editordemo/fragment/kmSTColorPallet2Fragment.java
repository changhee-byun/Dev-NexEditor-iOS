package com.nexstreaming.editordemo.fragment;


import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.TextView;

import com.nexstreaming.editordemo.R;

/**
 * A simple {@link Fragment} subclass.
 */
public class kmSTColorPallet2Fragment extends Fragment implements View.OnClickListener {

    private TextView tv_pallet_title;
    private ImageView iv_cur_color;
    private ImageView iv_white;
    private ImageView iv_lightgray;
    private ImageView iv_darkgray;
    private ImageView iv_verydarkgray;
    private ImageView iv_vividred;
    private ImageView iv_black;
    private ImageView iv_vividorange;
    private ImageView iv_vividorange2;
    private ImageView iv_pureyellow;
    private ImageView iv_moderategreen;
    private ImageView iv_limegreen;
    private ImageView iv_softblue;
    private ImageView iv_strongblue;
    private ImageView iv_darkblue;
    private ImageView iv_verydarkblue;
    private ImageView iv_mostlydesaturatedmagenta;
    private ImageView iv_darkmagenta;
    private ImageView iv_vividpink;

    private ImageView iv_focused;
    private Switch sw_view;

    private String To;

    final int WHITE             = 0xFFFFFFFF;
    final int LIGHT_GRAY        = 0xFFC2C2C2;
    final int DARK_GRAY         = 0xFF848484;
    final int VERY_DARK_GRAY    = 0xFF363636;
    final int VIVID_RED         = 0xFFED1C24;
    final int BLACK             = 0xFF000000;
    final int VIVID_ORANGE      = 0xFFF26522;
    final int VIVID_ORANGE2     = 0xFFF7941D;
    final int PURE_YELLOW       = 0xFFFFF200;
    final int MODERATE_GREEN    = 0xFF8DC63F;
    final int LIME_GREEN        = 0xFF3DB878;
    final int SOFT_BLUE         = 0xFF6DCFF6;
    final int STRING_BLUE       = 0xFF0072BC;
    final int DARK_BLUE         = 0xFF0054A6;
    final int VERY_DARK_BLUE    = 0xFF180F6C;
    final int MOSTLY_DESATURATED_MAGENTA = 0xFFA864A8;
    final int DARK_MAGENTA      = 0xFF92278F;
    final int VIVID_PINK        = 0xFFED145B;

    private OnSTColorPalletSelectedListener mListener;

    public interface OnSTColorPalletSelectedListener {
        public void onStColorPalletSelected(String from, int colorpallet);
        public void onStSwitchSelected(String from, boolean checked);
    }

    void setValues(String title, int color) {
//        Log.d("colorPallet2", "title=" + title + " color=" + color);

        tv_pallet_title.setText(title);
        iv_cur_color.setBackgroundColor(color);


        switch(color) {
            case WHITE:
                iv_focused = iv_white;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case LIGHT_GRAY:
                iv_focused = iv_lightgray;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case DARK_GRAY:
                iv_focused = iv_darkgray;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case VERY_DARK_GRAY:
                iv_focused = iv_verydarkgray;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case BLACK:
                iv_focused = iv_black;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case VIVID_RED:
                iv_focused = iv_vividred;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case VIVID_ORANGE:
                iv_focused = iv_vividorange;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case VIVID_ORANGE2:
                iv_focused = iv_vividorange2;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case PURE_YELLOW:
                iv_focused = iv_pureyellow;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case MODERATE_GREEN:
                iv_focused = iv_moderategreen;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case LIME_GREEN:
                iv_focused = iv_limegreen;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case SOFT_BLUE:
                iv_focused = iv_softblue;
                iv_softblue.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case STRING_BLUE:
                iv_focused = iv_strongblue;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case DARK_BLUE:
                iv_focused = iv_darkblue;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case VERY_DARK_BLUE:
                iv_focused = iv_verydarkblue;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case MOSTLY_DESATURATED_MAGENTA:
                iv_focused = iv_mostlydesaturatedmagenta;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case DARK_MAGENTA:
                iv_focused = iv_darkmagenta;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
            case VIVID_PINK:
                iv_focused = iv_vividpink;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                break;
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_km_stcolorpallet2, container, false);

        tv_pallet_title = (TextView) v.findViewById(R.id.pallet_title);
        iv_cur_color = (ImageView) v.findViewById(R.id.iv_curcolor);
        iv_white = (ImageView) v.findViewById(R.id.iv_white);
        iv_white.setOnClickListener(this);
        iv_lightgray = (ImageView) v.findViewById(R.id.iv_lightgray);
        iv_lightgray.setOnClickListener(this);
        iv_darkgray = (ImageView) v.findViewById(R.id.iv_darkgray);
        iv_darkgray.setOnClickListener(this);
        iv_verydarkgray = (ImageView) v.findViewById(R.id.iv_verydarkgray);
        iv_verydarkgray.setOnClickListener(this);
        iv_vividred = (ImageView) v.findViewById(R.id.iv_vividred);
        iv_vividred.setOnClickListener(this);
        iv_black = (ImageView) v.findViewById(R.id.iv_black);
        iv_black.setOnClickListener(this);
        iv_vividorange = (ImageView) v.findViewById(R.id.iv_vividorange);
        iv_vividorange.setOnClickListener(this);
        iv_vividorange2 = (ImageView) v.findViewById(R.id.iv_vividorange2);
        iv_vividorange2.setOnClickListener(this);
        iv_pureyellow = (ImageView) v.findViewById(R.id.iv_pureyellow);
        iv_pureyellow.setOnClickListener(this);
        iv_moderategreen = (ImageView) v.findViewById(R.id.iv_moderategreen);
        iv_moderategreen.setOnClickListener(this);
        iv_limegreen = (ImageView) v.findViewById(R.id.iv_limegreen);
        iv_limegreen.setOnClickListener(this);
        iv_softblue = (ImageView) v.findViewById(R.id.iv_softblue);
        iv_softblue.setOnClickListener(this);
        iv_strongblue = (ImageView) v.findViewById(R.id.iv_strongblue);
        iv_strongblue.setOnClickListener(this);
        iv_darkblue = (ImageView) v.findViewById(R.id.iv_darkblue);
        iv_darkblue.setOnClickListener(this);
        iv_verydarkblue = (ImageView) v.findViewById(R.id.iv_verydarkblue);
        iv_verydarkblue.setOnClickListener(this);
        iv_mostlydesaturatedmagenta = (ImageView) v.findViewById(R.id.iv_mostlydesaturatedmagenta);
        iv_mostlydesaturatedmagenta.setOnClickListener(this);
        iv_darkmagenta = (ImageView) v.findViewById(R.id.iv_darkmagenta);
        iv_darkmagenta.setOnClickListener(this);
        iv_vividpink = (ImageView) v.findViewById(R.id.iv_vividpink);
        iv_vividpink.setOnClickListener(this);

        sw_view = (Switch) v.findViewById(R.id.btn_switch);
        sw_view.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (mListener != null) {
                    mListener.onStSwitchSelected(To, isChecked);
                }
            }
        });

        Bundle bundle = getArguments();
        if(bundle != null) {
            To = bundle.getString("From");
            setValues(bundle.getString("titlePallet"), bundle.getInt("colorPallet"));
            sw_view.setChecked(bundle.getBoolean("visibleInfo"));
        }

        return v;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (OnSTColorPalletSelectedListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnSTColorPalletSelectedListener");
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.iv_white:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_white;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, WHITE);
                }
                break;
            case R.id.iv_lightgray:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_lightgray;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, LIGHT_GRAY);
                }
                break;
            case R.id.iv_darkgray:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_darkgray;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, DARK_GRAY);
                }
                break;
            case R.id.iv_verydarkgray:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_verydarkgray;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, VERY_DARK_GRAY);
                }
                break;
            case R.id.iv_vividred:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_vividred;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, VIVID_RED);
                }
                break;
            case R.id.iv_black:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_black;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, BLACK);
                }
                break;
            case R.id.iv_vividorange:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_vividorange;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, VIVID_ORANGE);
                }
                break;
            case R.id.iv_vividorange2:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_vividorange2;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, VIVID_ORANGE2);
                }
                break;
            case R.id.iv_pureyellow:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_pureyellow;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, PURE_YELLOW);
                }
                break;
            case R.id.iv_moderategreen:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_moderategreen;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, MODERATE_GREEN);
                }
                break;
            case R.id.iv_limegreen:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_limegreen;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, LIME_GREEN);
                }
                break;
            case R.id.iv_softblue:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_softblue;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, SOFT_BLUE);
                }
                break;
            case R.id.iv_strongblue:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_strongblue;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, STRING_BLUE);
                }
                break;
            case R.id.iv_darkblue:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_darkblue;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, DARK_BLUE);
                }
                break;
            case R.id.iv_verydarkblue:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_verydarkblue;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, VERY_DARK_BLUE);
                }
                break;
            case R.id.iv_mostlydesaturatedmagenta:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_mostlydesaturatedmagenta;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, MOSTLY_DESATURATED_MAGENTA);
                }
                break;
            case R.id.iv_darkmagenta:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_darkmagenta;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, DARK_MAGENTA);
                }
                break;
            case R.id.iv_vividpink:
                if(iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_vividpink;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_selected));
                if(mListener != null) {
                    mListener.onStColorPalletSelected(To, VIVID_PINK);
                }
                break;
        }
    }

}
