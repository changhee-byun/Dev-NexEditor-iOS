package com.nexstreaming.editordemo.fragment;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.kmsdk.kmEngine;
import com.nexstreaming.editordemo.utility.utilityPlayer;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineView;

public class kmSTPreviewFragment extends Fragment {

    utilityPlayer player;
    nexEngineView engineView;

    OnSTPreviewListener listener;
    public interface OnSTPreviewListener {
        public void onSTEncodingStart();
        public void onSTEncodingSuccess();
        public void onSTEncodingFail(int error);
        public void onSTEncodingProgress(int percent);
        public void onSTActivityBack();
        public void onSTHome();
        public void onSTEncodedFilePlay();
        public void onSTSetupEngine();
        public void onSTPlayFail(int error);
        public void onSTSeekFail(int result);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_km_preview, container, false);
        engineView = (nexEngineView) v.findViewById(R.id.preview);

        player = new utilityPlayer();

        if(listener != null) {
            listener.onSTSetupEngine();
        }
        return v;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        player.reset();
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            listener = (OnSTPreviewListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnSTPreviewListener");
        }
    }

    public void setEngine(nexEngine engine, boolean play) {
        player.setPreviewListener(listener);
        player.setEngineView(engineView);
        player.setEngine(engine, play);
    }

    public void setSeek(int seekTS) {
        player.setSeek(seekTS);
    }

    public void setPlay(boolean ignoreEngineStatus) {
        player.setPlay(ignoreEngineStatus);
    }

    public void setPause() {
        player.setPause();
    }

    public void setViews(View previewBack,
                         View previewETC,
                         View previewPlay,
                         View previewPlayingTime,
                         View previewTotalTime,
                         View previewSeekBar) {
        player.setView4Back(previewBack);
        player.setView4ETC(previewETC);
        player.setView4Play(previewPlay);
        player.setView4Time(previewPlayingTime, previewTotalTime);
        player.setView4Seek(previewSeekBar);
    }
}
