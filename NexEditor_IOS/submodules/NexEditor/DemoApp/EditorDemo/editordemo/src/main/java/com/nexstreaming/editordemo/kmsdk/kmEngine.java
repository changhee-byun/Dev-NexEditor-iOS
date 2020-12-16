package com.nexstreaming.editordemo.kmsdk;

import android.app.Application;
import android.content.Context;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexConfig;
import com.nexstreaming.nexeditorsdk.nexEngine;

public class kmEngine extends Application {

    private static nexEngine mEngine;
    private static Context mContext;

    public enum EngineState {
        InitializedState,
        NonInitializedState
    };

    static EngineState engineState
            = EngineState.NonInitializedState;

    public static EngineState getState() {
        return engineState;
    }

    private static void setState(EngineState state) {
        engineState = state;
    }

    public static nexEngine getEngine() {
        return mEngine;
    }

    public static void createEngine() {
        setState(EngineState.InitializedState);

        nexApplicationConfig.createApp(mContext);
        nexConfig.setProperty(nexConfig.kSetUserConfig, 1);
        nexConfig.setProperty(nexConfig.kMaxResolution, 1920 * 1080);
        nexApplicationConfig.init(mContext, "NexDemo");

        if(mEngine == null) {
            mEngine = new nexEngine(mContext);
        }
    }

    public static float getAspectMode() {
        return nexApplicationConfig.getAspectRatioMode();
    }

    public static void releaseEngine() {
        setState(EngineState.NonInitializedState);

        nexApplicationConfig.releaseApp();
        mEngine = null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mContext = getApplicationContext();
    }
}
