package com.nexstreaming.editordemo.fragment;


import android.app.Activity;
import android.app.Fragment;
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
import android.widget.ScrollView;

import com.nexstreaming.editordemo.R;

/**
 * A simple {@link Fragment} subclass.
 */
public class kmSTTextAnimationFragment extends Fragment implements View.OnClickListener, View.OnTouchListener {

    private static String TAG = "km_TextFragment";

    private FrameLayout rootView;

    private ImageView iv_ta1;
    private ImageView iv_ta2;
    private ImageView iv_ta3;
    private ImageView iv_ta4;
    private ImageView iv_ta5;
    private ImageView iv_ta6;
    private ImageView iv_ta7;
    private ImageView iv_ta8;

    private ImageView iv_page1;
    private ImageView iv_page2;

    private ScrollView sv;

    private ImageView iv_focused;

    private GestureDetector gestureDetector;

    // KMSA-545
    private int prev_idx = 0;
    //

    private static int cur_page = 1;
    private int page = 1;

    private OnSTTextSelectedListener mListener;

    public interface OnSTTextSelectedListener {
        public void onStTextAnimationSelected(int ta_idx);
    }

    void setValues(int ta_index) {
        switch (ta_index) {
            case 1:
            case 9:
            {
                iv_focused = iv_ta1;
                break;
            }
            case 2:
            case 10:
            {
                iv_focused = iv_ta2;
                break;
            }
            case 3:
            case 11:
            {
                iv_focused = iv_ta3;
                break;
            }
            case 4:
            {
                iv_focused = iv_ta4;
                break;
            }
            case 5:
            {
                iv_focused = iv_ta5;
                break;
            }
            case 6:
            {
                iv_focused = iv_ta6;
                break;
            }
            case 7:
            {
                iv_focused = iv_ta7;
                break;
            }
            case 8:
            {
                iv_focused = iv_ta8;
                break;
            }
            default:
            {
                iv_focused = null;
                break;
            }
        }
        if(iv_focused != null) {
            Log.d(TAG, "cur_page=" + cur_page + " page=" + page);
            if(cur_page == page) {
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
            } else {
                iv_focused.setImageDrawable(null);
            }
        }
    }

    void setResource4Page(int page) {
        switch(page) {
            case 1:
            {
                iv_ta1.setBackgroundResource(R.mipmap.ic_effect_none_80dp);
                iv_ta2.setBackgroundResource(R.mipmap.ic_text_animation_basic_80dp);
                iv_ta3.setBackgroundResource(R.mipmap.ic_text_animation_fade_80dp);
                iv_ta4.setBackgroundResource(R.mipmap.ic_text_animation_pop_80dp);
                iv_ta5.setBackgroundResource(R.mipmap.ic_text_animation_slide_80dp);
                iv_ta6.setBackgroundResource(R.mipmap.ic_text_animation_spin_80dp);
                iv_ta7.setBackgroundResource(R.mipmap.ic_text_animation_drop_80dp);
                iv_ta8.setBackgroundResource(R.mipmap.ic_text_animation_scale_80dp);

                iv_page1.setImageResource(R.mipmap.ic_navi_focus);
                iv_page2.setImageResource(R.mipmap.ic_navi_normal);
                break;
            }
            case 2:
            {
                iv_ta1.setBackgroundResource(R.mipmap.ic_text_animation_floating_80dp);
                iv_ta2.setBackgroundResource(R.mipmap.ic_text_animation_drifting_80dp);
                iv_ta3.setBackgroundResource(R.mipmap.ic_text_animation_squishing_80dp);
                iv_ta4.setBackgroundResource(0);
                iv_ta5.setBackgroundResource(0);
                iv_ta6.setBackgroundResource(0);
                iv_ta7.setBackgroundResource(0);
                iv_ta8.setBackgroundResource(0);

                iv_page1.setImageResource(R.mipmap.ic_navi_normal);
                iv_page2.setImageResource(R.mipmap.ic_navi_focus);
                break;
            }
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        gestureDetector = new GestureDetector(
                new SwipeGestureDetector());
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_km_sttext, container, false);
        rootView = (FrameLayout) v.findViewById(R.id.mainLayout);

        sv = (ScrollView) v.findViewById(R.id.containView);
        sv.setOnTouchListener(this);

        iv_ta1 = (ImageView) v.findViewById(R.id.ta_id_1);
        iv_ta1.setOnTouchListener(this);
        iv_ta1.setOnClickListener(this);
        iv_ta2 = (ImageView) v.findViewById(R.id.ta_id_2);
        iv_ta2.setOnTouchListener(this);
        iv_ta2.setOnClickListener(this);
        iv_ta3 = (ImageView) v.findViewById(R.id.ta_id_3);
        iv_ta3.setOnTouchListener(this);
        iv_ta3.setOnClickListener(this);
        iv_ta4 = (ImageView) v.findViewById(R.id.ta_id_4);
        iv_ta4.setOnTouchListener(this);
        iv_ta4.setOnClickListener(this);
        iv_ta5 = (ImageView) v.findViewById(R.id.ta_id_5);
        iv_ta5.setOnTouchListener(this);
        iv_ta5.setOnClickListener(this);
        iv_ta6 = (ImageView) v.findViewById(R.id.ta_id_6);
        iv_ta6.setOnTouchListener(this);
        iv_ta6.setOnClickListener(this);
        iv_ta7 = (ImageView) v.findViewById(R.id.ta_id_7);
        iv_ta7.setOnTouchListener(this);
        iv_ta7.setOnClickListener(this);
        iv_ta8 = (ImageView) v.findViewById(R.id.ta_id_8);
        iv_ta8.setOnTouchListener(this);
        iv_ta8.setOnClickListener(this);

        iv_page1 = (ImageView) v.findViewById(R.id.page_id_1);
        iv_page2 = (ImageView) v.findViewById(R.id.page_id_2);

        setResource4Page(1);

        Bundle bundle = getArguments();
        if(bundle != null) {
            int idx = bundle.getInt("textAnimation");
            // KMSA-545
            prev_idx = idx;
            //
            if(idx < 9) {
                cur_page = 1;
            } else {
                cur_page = 2;
            }
            setValues(idx);
        }

        return v;
    }

    public void rollBack() {
        iv_focused.setImageDrawable(null);
        if(prev_idx < 9) {
            cur_page = 1;
        } else {
            cur_page = 2;
        }
        setValues(prev_idx);
    }

    public void updatePrevInfo() {
        int idx  = 0;
        if(iv_focused == iv_ta1) {
            idx = 1;
        } else if(iv_focused == iv_ta2) {
            idx = 2;
        } else if(iv_focused == iv_ta3) {
            idx = 3;
        } else if(iv_focused == iv_ta4) {
            idx = 4;
        } else if(iv_focused == iv_ta5) {
            idx = 5;
        } else if(iv_focused == iv_ta6) {
            idx = 6;
        } else if(iv_focused == iv_ta7) {
            idx = 7;
        } else if(iv_focused == iv_ta8) {
            idx = 8;
        }
        prev_idx = idx+(cur_page-1)*8;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (OnSTTextSelectedListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnSTTextSelectedListener");
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
        if(iv_focused != null) {
            iv_focused.setImageDrawable(null);
        }

        int tmp_page = page;

        int selected_ce_id = -1;
        switch (v.getId()) {
            case R.id.ta_id_1:
            {
                cur_page = page;
                iv_focused = iv_ta1;
                selected_ce_id = 1+(cur_page-1)*8;
                break;
            }
            case R.id.ta_id_2:
            {
                cur_page = page;
                iv_focused = iv_ta2;
                selected_ce_id = 2+(cur_page-1)*8;
                break;
            }
            case R.id.ta_id_3:
            {
                cur_page = page;
                iv_focused = iv_ta3;
                selected_ce_id = 3+(cur_page-1)*8;
                break;
            }
            case R.id.ta_id_4:
            {
                if(tmp_page != 2) {
                    cur_page = page;
                    iv_focused = iv_ta4;
                    selected_ce_id = 4+(cur_page-1)*8;
                } else {
                    selected_ce_id = -1;
                }
                break;
            }
            case R.id.ta_id_5:
            {
                if(tmp_page != 2) {
                    cur_page = page;
                    iv_focused = iv_ta5;
                    selected_ce_id = 5+(cur_page-1)*8;
                } else {
                    selected_ce_id = -1;
                }
                break;
            }
            case R.id.ta_id_6:
            {
                if(tmp_page != 2) {
                    cur_page = page;
                    iv_focused = iv_ta6;
                    selected_ce_id = 6+(cur_page-1)*8;
                } else {
                    selected_ce_id = -1;
                }
                break;
            }
            case R.id.ta_id_7:
            {
                if(tmp_page != 2) {
                    cur_page = page;
                    iv_focused = iv_ta7;
                    selected_ce_id = 7+(cur_page-1)*8;
                } else {
                    selected_ce_id = -1;
                }
                break;
            }
            case R.id.ta_id_8:
            {
                if(tmp_page != 2) {
                    cur_page = page;
                    iv_focused = iv_ta8;
                    selected_ce_id = 8+(cur_page-1)*8;
                } else {
                    selected_ce_id = -1;
                }
                break;
            }
        }
        if(iv_focused != null
                && selected_ce_id != -1) {
            iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
        }
        if(mListener != null
                && selected_ce_id != -1) {
            mListener.onStTextAnimationSelected(selected_ce_id);
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
            Log.d(TAG, "cur_page="+cur_page+" page="+page);
            if(cur_page == page) {
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
            } else {
                iv_focused.setImageDrawable(null);
            }
        }
    }

    private void onLeftSwipe() {
        Log.d(TAG, "onLeftSwipe");
        page++;
        if(page > 2) {
            page = 2;
            return;
        }
        rootView.removeView(sv);
        TransitionManager.beginDelayedTransition(rootView, new Fade());
        rootView.addView(sv);
        setResource4Page(page);
        if(iv_focused != null) {
            Log.d(TAG, "cur_page="+cur_page+" page="+page);
            if(cur_page == page) {
                iv_focused.setImageDrawable(getActivity().getResources().getDrawable(R.mipmap.ic_menu_selected));
            } else {
                iv_focused.setImageDrawable(null);
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
