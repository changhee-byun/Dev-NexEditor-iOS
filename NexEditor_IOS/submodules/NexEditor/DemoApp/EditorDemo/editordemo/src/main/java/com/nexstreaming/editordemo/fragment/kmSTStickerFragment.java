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
public class kmSTStickerFragment extends Fragment implements View.OnClickListener {

    private ImageView iv_none;
    private ImageView iv_ilove;
    private ImageView iv_zoo;
    private ImageView iv_sun;
    private ImageView iv_fish;
    private ImageView iv_christmas_newyear;
    private ImageView iv_hello;
    private ImageView iv_birthday;

    private ImageView iv_focused;

    private OnSTStickerSelectedListener mListener;

    public interface OnSTStickerSelectedListener {
        public void onStStickerSelected(String stickereffect);
    }

    void setValue(String stickereffect) {
        if(stickereffect.equals("ilove")) {
            iv_focused = iv_ilove;
            iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
        } else if(stickereffect.equals("zoo")) {
            iv_focused = iv_zoo;
            iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
        } else if(stickereffect.equals("sun")) {
            iv_focused = iv_sun;
            iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
        } else if(stickereffect.equals("fish")) {
            iv_focused = iv_fish;
            iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
        } else if(stickereffect.equals("christmas_newyear")) {
            iv_focused = iv_christmas_newyear;
            iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
        } else if(stickereffect.equals("hello")) {
            iv_focused = iv_hello;
            iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
        } else if(stickereffect.equals("birthday")) {
            iv_focused = iv_birthday;
            iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
        } else if(stickereffect.equals("none")) {
            iv_focused = iv_none;
            iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_km_ststicker, container, false);

        iv_ilove = (ImageView) v.findViewById(R.id.stickereffect_ilove);
        iv_ilove.setOnClickListener(this);
        iv_zoo = (ImageView) v.findViewById(R.id.stickereffect_zoo);
        iv_zoo.setOnClickListener(this);
        iv_sun = (ImageView) v.findViewById(R.id.stickereffect_sun);
        iv_sun.setOnClickListener(this);
        iv_fish = (ImageView) v.findViewById(R.id.stickereffect_fish);
        iv_fish.setOnClickListener(this);
        iv_christmas_newyear = (ImageView) v.findViewById(R.id.stickereffect_christmas_newyear);
        iv_christmas_newyear.setOnClickListener(this);
        iv_hello = (ImageView) v.findViewById(R.id.stickereffect_hello);
        iv_hello.setOnClickListener(this);
        iv_birthday = (ImageView) v.findViewById(R.id.stickereffect_birthday);
        iv_birthday.setOnClickListener(this);
        iv_none = (ImageView) v.findViewById(R.id.stickereffect_none);
        iv_none.setOnClickListener(this);

        Bundle bundle = getArguments();
        if(bundle != null) {
            setValue(bundle.getString("stickereffect"));
        }

        return v;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (OnSTStickerSelectedListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnSTStickerSelectedListener");
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.stickereffect_ilove:
                if (iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_ilove;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
                if (mListener != null) {
                    mListener.onStStickerSelected("ilove");
                }
                break;
            case R.id.stickereffect_zoo:
                if (iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_zoo;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
                if (mListener != null) {
                    mListener.onStStickerSelected("zoo");
                }
                break;
            case R.id.stickereffect_sun:
                if (iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_sun;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
                if (mListener != null) {
                    mListener.onStStickerSelected("sun");
                }
                break;
            case R.id.stickereffect_fish:
                if (iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_fish;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
                if (mListener != null) {
                    mListener.onStStickerSelected("fish");
                }
                break;
            case R.id.stickereffect_christmas_newyear:
                if (iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_christmas_newyear;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
                if (mListener != null) {
                    mListener.onStStickerSelected("christmas_newyear");
                }
                break;
            case R.id.stickereffect_hello:
                if (iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_hello;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
                if (mListener != null) {
                    mListener.onStStickerSelected("hello");
                }
                break;
            case R.id.stickereffect_birthday:
                if (iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_birthday;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
                if (mListener != null) {
                    mListener.onStStickerSelected("birthday");
                }
                break;
            case R.id.stickereffect_none:
                if (iv_focused != null) {
                    iv_focused.setImageDrawable(null);
                }
                iv_focused = iv_none;
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
                if (mListener != null) {
                    mListener.onStStickerSelected("none");
                }
                break;
        }
    }
}
