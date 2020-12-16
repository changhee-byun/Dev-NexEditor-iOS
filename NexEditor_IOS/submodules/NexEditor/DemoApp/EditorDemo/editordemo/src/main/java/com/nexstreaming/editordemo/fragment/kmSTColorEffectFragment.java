package com.nexstreaming.editordemo.fragment;


import android.app.Activity;
import android.app.Fragment;
import android.graphics.Bitmap;
import android.graphics.pdf.PdfDocument;
import android.media.Image;
import android.os.AsyncTask;
import android.os.Bundle;
import android.transition.Fade;
import android.transition.TransitionManager;
import android.util.Log;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;

import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.utility.utilityColorEffect2Thumbnail;
import com.nexstreaming.nexeditorsdk.nexColorEffect;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * A simple {@link Fragment} subclass.
 */
public class kmSTColorEffectFragment extends Fragment implements View.OnClickListener, View.OnTouchListener {

    private static String TAG = "km_ColorFragment";

    private FrameLayout rootView;

    private ImageView iv_ce1;
    private ImageView iv_ce2;
    private ImageView iv_ce3;
    private ImageView iv_ce4;
    private ImageView iv_ce5;
    private ImageView iv_ce6;
    private ImageView iv_ce7;
    private ImageView iv_ce8;

    private ImageView iv_sel1;
    private ImageView iv_sel2;
    private ImageView iv_sel3;
    private ImageView iv_sel4;
    private ImageView iv_sel5;
    private ImageView iv_sel6;
    private ImageView iv_sel7;
    private ImageView iv_sel8;

    private TextView tv_ce1;
    private TextView tv_ce2;
    private TextView tv_ce3;
    private TextView tv_ce4;
    private TextView tv_ce5;
    private TextView tv_ce6;
    private TextView tv_ce7;
    private TextView tv_ce8;

    private ScrollView sv;

    private ImageView iv_focused;

    private GestureDetector gestureDetector;

    private int total_page;
    private int remain_element;

    private int page = 1;

    private String selectedPresetNaming;
    private int selectedPresetPage;
    private int selectedPresetId;

    private OnSTColorSelectedListener mListener;

    public interface OnSTColorSelectedListener {
        public void onStColorSelected(int ce_idx);
    }

    void setFocusedView(int page) {
        if(page == selectedPresetPage) {
            if(selectedPresetId == 0) {
                iv_focused = iv_sel1;
            } else if(selectedPresetId == 1) {
                iv_focused = iv_sel2;
            } else if(selectedPresetId == 2) {
                iv_focused = iv_sel3;
            } else if(selectedPresetId == 3) {
                iv_focused = iv_sel4;
            } else if(selectedPresetId == 4) {
                iv_focused = iv_sel5;
            } else if(selectedPresetId == 5) {
                iv_focused = iv_sel6;
            } else if(selectedPresetId == 6) {
                iv_focused = iv_sel7;
            } else {
                iv_focused = iv_sel8;
            }
            iv_focused.setVisibility(View.VISIBLE);
        } else {
            if(iv_focused != null)
                iv_focused.setVisibility(View.INVISIBLE);
        }
    }

    private ArrayList<String> presetList = new ArrayList<>();

    void setResource4Page(int page) {
        // 초기화 시켜주고
        //
        iv_ce1.setVisibility(View.INVISIBLE);
        tv_ce1.setText("");
        iv_ce2.setVisibility(View.INVISIBLE);
        tv_ce2.setText("");
        iv_ce3.setVisibility(View.INVISIBLE);
        tv_ce3.setText("");
        iv_ce4.setVisibility(View.INVISIBLE);
        tv_ce4.setText("");
        iv_ce5.setVisibility(View.INVISIBLE);
        tv_ce5.setText("");
        iv_ce6.setVisibility(View.INVISIBLE);
        tv_ce6.setText("");
        iv_ce7.setVisibility(View.INVISIBLE);
        tv_ce7.setText("");
        iv_ce8.setVisibility(View.INVISIBLE);
        tv_ce8.setText("");

        // page 값에 따라서 그려주고
        //
        int loop = (page-1)*8;
        int limit = loop+8;

        String displayedLabel;

        if(page == 1) {
            for(; loop<limit; loop++) {
                if(loop%8 == 0) {
                    iv_ce1.setVisibility(View.VISIBLE);
                    iv_ce1.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_effect_none_80dp));
                } else if(loop%8 == 1) {
                    iv_ce2.setVisibility(View.VISIBLE);
                    iv_ce2.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce2.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 2) {
                    iv_ce3.setVisibility(View.VISIBLE);
                    iv_ce3.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce3.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 3) {
                    iv_ce4.setVisibility(View.VISIBLE);
                    iv_ce4.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce4.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 4) {
                    iv_ce5.setVisibility(View.VISIBLE);
                    iv_ce5.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce5.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 5) {
                    iv_ce6.setVisibility(View.VISIBLE);
                    iv_ce6.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce6.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 6){
                    iv_ce7.setVisibility(View.VISIBLE);
                    iv_ce7.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                   displayedLabel = presetList.get(loop);
                    tv_ce7.setText(displayedLabel.replaceAll("_", "\n"));
                } else {
                    iv_ce8.setVisibility(View.VISIBLE);
                    iv_ce8.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce8.setText(displayedLabel.replaceAll("_", "\n"));
                }
            }
        } else {
            if(page == total_page) {
                limit = loop+remain_element;
            }
            //
            for(; loop<limit; loop++) {
                if(loop%8 == 0) {
                    iv_ce1.setVisibility(View.VISIBLE);
                    iv_ce1.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce1.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 1) {
                    iv_ce2.setVisibility(View.VISIBLE);
                    iv_ce2.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce2.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 2) {
                    iv_ce3.setVisibility(View.VISIBLE);
                    iv_ce3.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce3.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 3) {
                    iv_ce4.setVisibility(View.VISIBLE);
                    iv_ce4.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce4.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 4) {
                    iv_ce5.setVisibility(View.VISIBLE);
                    iv_ce5.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce5.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 5) {
                    iv_ce6.setVisibility(View.VISIBLE);
                    iv_ce6.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce6.setText(displayedLabel.replaceAll("_", "\n"));
                } else if(loop%8 == 6){
                    iv_ce7.setVisibility(View.VISIBLE);
                    iv_ce7.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce7.setText(displayedLabel.replaceAll("_", "\n"));
                } else {
                    iv_ce8.setVisibility(View.VISIBLE);
                    iv_ce8.setImageBitmap(colorEffect2Thumbnail.getColorEffect2Thumbnail(presetList.get(loop)));
                    displayedLabel = presetList.get(loop);
                    tv_ce8.setText(displayedLabel.replaceAll("_", "\n"));
                }
            }
        }
    }

    utilityColorEffect2Thumbnail colorEffect2Thumbnail;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        colorEffect2Thumbnail = utilityColorEffect2Thumbnail.getInstance();

        gestureDetector = new GestureDetector(
                new SwipeGestureDetector());
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_km_stcolor, container, false);
        rootView = (FrameLayout) v.findViewById(R.id.mainLayout);

        sv = (ScrollView) v.findViewById(R.id.containView);
        sv.setOnTouchListener(this);

        iv_ce1 = (ImageView) v.findViewById(R.id.effect_id_1);
        iv_ce1.setOnTouchListener(this);
        iv_ce1.setOnClickListener(this);
        iv_ce2 = (ImageView) v.findViewById(R.id.effect_id_2);
        iv_ce2.setOnTouchListener(this);
        iv_ce2.setOnClickListener(this);
        iv_ce3 = (ImageView) v.findViewById(R.id.effect_id_3);
        iv_ce3.setOnTouchListener(this);
        iv_ce3.setOnClickListener(this);
        iv_ce4 = (ImageView) v.findViewById(R.id.effect_id_4);
        iv_ce4.setOnTouchListener(this);
        iv_ce4.setOnClickListener(this);
        iv_ce5 = (ImageView) v.findViewById(R.id.effect_id_5);
        iv_ce5.setOnTouchListener(this);
        iv_ce5.setOnClickListener(this);
        iv_ce6 = (ImageView) v.findViewById(R.id.effect_id_6);
        iv_ce6.setOnTouchListener(this);
        iv_ce6.setOnClickListener(this);
        iv_ce7 = (ImageView) v.findViewById(R.id.effect_id_7);
        iv_ce7.setOnTouchListener(this);
        iv_ce7.setOnClickListener(this);
        iv_ce8 = (ImageView) v.findViewById(R.id.effect_id_8);
        iv_ce8.setOnTouchListener(this);
        iv_ce8.setOnClickListener(this);

        iv_sel1 = (ImageView) v.findViewById(R.id.selected_id_1);
        iv_sel2 = (ImageView) v.findViewById(R.id.selected_id_2);
        iv_sel3 = (ImageView) v.findViewById(R.id.selected_id_3);
        iv_sel4 = (ImageView) v.findViewById(R.id.selected_id_4);
        iv_sel5 = (ImageView) v.findViewById(R.id.selected_id_5);
        iv_sel6 = (ImageView) v.findViewById(R.id.selected_id_6);
        iv_sel7 = (ImageView) v.findViewById(R.id.selected_id_7);
        iv_sel8 = (ImageView) v.findViewById(R.id.selected_id_8);

        tv_ce1 = (TextView) v.findViewById(R.id.effect_naming_1);
        tv_ce2 = (TextView) v.findViewById(R.id.effect_naming_2);
        tv_ce3 = (TextView) v.findViewById(R.id.effect_naming_3);
        tv_ce4 = (TextView) v.findViewById(R.id.effect_naming_4);
        tv_ce5 = (TextView) v.findViewById(R.id.effect_naming_5);
        tv_ce6 = (TextView) v.findViewById(R.id.effect_naming_6);
        tv_ce7 = (TextView) v.findViewById(R.id.effect_naming_7);
        tv_ce8 = (TextView) v.findViewById(R.id.effect_naming_8);

        if(getArguments() != null) {
            selectedPresetNaming = getArguments().getString("colorEffect");
        }

        List<nexColorEffect> colorEffects = nexColorEffect.getPresetList();
        //
        int count2predefinedEffect = 18;
        int loopid = 0;

        for(nexColorEffect colorEffect : colorEffects) {
            if(loopid == 0) {
                loopid++;
            } else {
                if(loopid < count2predefinedEffect) {
                    loopid++;
                    continue;
                }
            }
            presetList.add(colorEffect.getPresetName());
        }

        total_page = (presetList.size() + 1) / 8;
        remain_element = (presetList.size() + 1) % 8;

        selectedPresetId = presetList.indexOf(selectedPresetNaming) % 8;
        selectedPresetPage = presetList.indexOf(selectedPresetNaming) / 8 + 1;

        setResource4Page(1);

        setFocusedView(1);

        return v;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (OnSTColorSelectedListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnSTColorSelectedListener");
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (gestureDetector.onTouchEvent(event)) {
            return true;
        }
        return false;
    }

    @Override
    public void onClick(View v) {
        iv_focused.setVisibility(View.INVISIBLE);

        switch (v.getId()) {
            case R.id.effect_id_1:
            {
                if(iv_focused != null) {
                    // 이전에 선택된 화면을 지운다
                    iv_focused.setImageDrawable(null);
                }

                iv_focused = iv_sel1;
                //
                selectedPresetId = (page-1)*8;
                selectedPresetPage = page;
                break;
            }
            case R.id.effect_id_2:
            {
                if(iv_focused != null) {
                    // 이전에 선택된 화면을 지운다
                    iv_focused.setImageDrawable(null);
                }

                iv_focused = iv_sel2;
                //
                selectedPresetId = (page-1)*8+1;
                selectedPresetPage = page;
                break;
            }
            case R.id.effect_id_3:
            {
                if(iv_focused != null) {
                    // 이전에 선택된 화면을 지운다
                    iv_focused.setImageDrawable(null);
                }

                iv_focused = iv_sel3;
                //
                selectedPresetId = (page-1)*8+2;
                selectedPresetPage = page;
                break;
            }
            case R.id.effect_id_4:
            {
                if(iv_focused != null) {
                    // 이전에 선택된 화면을 지운다
                    iv_focused.setImageDrawable(null);
                }

                iv_focused = iv_sel4;
                //
                selectedPresetId = (page-1)*8+3;
                selectedPresetPage = page;
                break;
            }
            case R.id.effect_id_5:
            {
                if(iv_focused != null) {
                    // 이전에 선택된 화면을 지운다
                    iv_focused.setImageDrawable(null);
                }

                iv_focused = iv_sel5;
                //
                selectedPresetId = (page-1)*8+4;
                selectedPresetPage = page;
                break;
            }
            case R.id.effect_id_6:
            {
                if(iv_focused != null) {
                    // 이전에 선택된 화면을 지운다
                    iv_focused.setImageDrawable(null);
                }

                iv_focused = iv_sel6;
                //
                selectedPresetId = (page-1)*8+5;
                selectedPresetPage = page;
                break;
            }
            case R.id.effect_id_7:
            {
                if(iv_focused != null) {
                    // 이전에 선택된 화면을 지운다
                    iv_focused.setImageDrawable(null);
                }

                iv_focused = iv_sel7;
                //
                selectedPresetId = (page-1)*8+6;
                selectedPresetPage = page;
                break;
            }
            case R.id.effect_id_8:
            {
                if(iv_focused != null) {
                    // 이전에 선택된 화면을 지운다
                    iv_focused.setImageDrawable(null);
                }

                iv_focused = iv_sel8;
                //
                selectedPresetId = (page-1)*8+7;
                selectedPresetPage = page;
                break;
            }
        }
        if(mListener != null) {
            if(iv_focused != null) {
                iv_focused.setVisibility(View.VISIBLE);
            }

            if(selectedPresetPage == 0 && selectedPresetId == 0) {
                mListener.onStColorSelected(0);
            } else {
                int count2predefinedEffect = 17;
                int presetId = count2predefinedEffect + selectedPresetId;
                mListener.onStColorSelected(presetId);
            }
        }
    }

    private void onRightSwipe() {
        Log.d(TAG, "onRightSwipe");
        page--;
        if(page < 1) {
            page = 1;
            return;
        }
        rootView.removeView(sv);
        TransitionManager.beginDelayedTransition(rootView, new Fade());
        rootView.addView(sv);
        setResource4Page(page);
        if(iv_focused != null) {
            Log.d(TAG, "selectedPresetPage="+selectedPresetPage+" page="+page);
            if(selectedPresetPage == page) {
                iv_focused.setVisibility(View.VISIBLE);
            } else {
                iv_focused.setVisibility(View.INVISIBLE);
            }
        }
    }

    private void onLeftSwipe() {
        Log.d(TAG, "onLeftSwipe");
        page++;
        if(page > total_page) {
            page = total_page;
            return;
        }
        rootView.removeView(sv);
        TransitionManager.beginDelayedTransition(rootView, new Fade());
        rootView.addView(sv);
        setResource4Page(page);
        if(iv_focused != null) {
            Log.d(TAG, "cur_page="+selectedPresetPage+" page="+page);
            if(selectedPresetPage == page) {
                iv_focused.setVisibility(View.VISIBLE);
            } else {
                iv_focused.setVisibility(View.INVISIBLE);
            }
        }
    }

    // Private class for gestures
    private class SwipeGestureDetector
            extends GestureDetector.SimpleOnGestureListener {
        // Swipe properties, you can change it to make the swipe
        // longer or shorter and speed
        private static final int SWIPE_MIN_DISTANCE = 80;
        private static final int SWIPE_MAX_OFF_PATH = 200;
        private static final int SWIPE_THRESHOLD_VELOCITY = 200;

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2,
                               float velocityX, float velocityY) {
            try {
                float diffAbs = Math.abs(e1.getY() - e2.getY());
                float diff = e1.getX() - e2.getX();

                if (diffAbs > SWIPE_MAX_OFF_PATH)
                    return false;

                // Left swipe
                if (diff > SWIPE_MIN_DISTANCE
                        && Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
                    onLeftSwipe();

                    // Right swipe
                } else if (-diff > SWIPE_MIN_DISTANCE
                        && Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
                    onRightSwipe();
                }
            } catch (Exception e) {
                Log.e("YourActivity", "Error on gestures");
            }
            return false;
        }
    }
}
