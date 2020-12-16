package com.nexstreaming.kminternal.nexvideoeditor;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.media.AudioTrack;
import android.media.Image;
import android.media.ImageReader;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.StrictMode;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.SparseArray;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.support.annotation.RequiresApi;

import dalvik.system.BaseDexClassLoader;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Deque;
import java.util.EnumMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NoSuchElementException;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.task.ResultTask;
import com.nexstreaming.app.common.task.ResultTask.OnResultAvailableListener;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.app.common.task.Task.Event;
import com.nexstreaming.app.common.task.Task.OnFailListener;
import com.nexstreaming.app.common.task.Task.TaskError;
import com.nexstreaming.kminternal.kinemaster.codeccolorformat.ColorFormatChecker;
import com.nexstreaming.kminternal.kinemaster.codeccolorformat.ColorFormatChecker.ColorFormat;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.config.NexEditorDeviceProfile;

import com.nexstreaming.kminternal.nexvideoeditor.nexEditorUtils;

/*
 * This is the main interface to NexEditor engine.
 *
 * There are two kinds of commands that can be sent to the engine:  Synchronous and asynchronous.
 *
 * Synchronous command functions block until completion, and in general should not be called by
 * the main UI thread.  Asynchronous command functions return immediately, but the relevant
 * commands are queued in the engine and executed in order in the background.  When each command
 * completes, the relevant listener is called.
 *
 * There are two kinds of listeners:
 *
 * -  The general listener reports all events from the engine.
 * -  Individual functions can also take a one-time listener that reports about that particular
 *     instance of the command.
 *
 * One-time listeners are useful if you only care about THAT particular request completing.
 *
 * Note that the engine itself doesn't have any one-time listener support, so it is implemented
 * in this class instead.  Whenever an asynchronous command is sent to the engine, the appropraite
 * listener (or an empty listener if none was given) is added to a cue for that command.  For
 * example, when startPlay() is called, an OnPlayListener is added to the cue of play listeners.
 * When the actual play start (or fail) even is retrieved from the engine, the next listener in the
 * queue is notified.
 *
 * (NOTE:  We are gradually transitioning to using Task objects for asynchronous notification rather
 * than explicit listeners.  Although NexEditor.java still mostly uses explicit listeners, the
 * wrapper, VideoEditor.java, uses only Task objects.)
 *
 */
public class NexEditor {

    private static final String LOG_TAG = "NexEditor.java";
    private static final boolean DEBUG = false;

    private static final int LOAD_THEME_PREVIEW = 0x00000001;
    private static final int MAX_FAST_PREVIEW_COUNT = 2;

    private static final int DEFAULT_EXPORT_AUDIO_BITRATE = 128 * 1024;
    private static final int DEFAULT_EXPORT_CODEC = 0x10010300; //avc
    private int m_iNextClipID = 1;
    private boolean m_isPlaying = false;
    private boolean m_useThemeManager = false;
    private NexEditorEventListener m_listener = null;
    private NexThemeView mView = null;
    private NexEditorListener m_uiListener = null;
    private Theme[] m_themeList = null;
    private Effect[] m_transitionEffectList = null;
    private Effect[] m_titleEffectList = null;
    private String m_currentThemeId;
    private boolean m_didSetTimeSinceLastPlay = false;
    private Deque<OnAddClipDoneListener> m_addAudioClipDoneListeners = new LinkedList<OnAddClipDoneListener>();
    private Deque<OnAddClipDoneListener> m_addVisualClipDoneListeners = new LinkedList<OnAddClipDoneListener>();
    private Deque<OnDeleteClipDoneListener> m_deleteClipDoneListeners = new LinkedList<OnDeleteClipDoneListener>();
    private Deque<OnMoveClipDoneListener> m_moveClipDoneListeners = new LinkedList<OnMoveClipDoneListener>();
    private Deque<OnSetTimeDoneListener> m_setTimeDoneListeners = new LinkedList<OnSetTimeDoneListener>();
    private Deque<OnCompletionListener> m_onStopListeners = new LinkedList<OnCompletionListener>();
    private Deque<OnPlayListener> m_onPlayListeners = new LinkedList<OnPlayListener>();
    private Deque<OnPlayListener> m_onPlayStartListeners = new LinkedList<OnPlayListener>();
    private Deque<OnCompletionListener> m_onCommandMarkerListeners = new LinkedList<OnCompletionListener>();
    private Deque<OnCompletionListener> m_onSetTimeCancelListeners = new LinkedList<OnCompletionListener>();
    private Deque<Integer> m_movedClipId = new LinkedList<Integer>();
    private Deque<Task> m_loadListAsyncTasks = new ArrayDeque<>();
    private Deque<OnCaptureListener> m_onCaptureListeners = new LinkedList<OnCaptureListener>();
    private Deque<OnExportListener> m_onExportListeners = new LinkedList<OnExportListener>();
    private OnSurfaceChangeListener m_onSurfaceChangeListener = null;
    private int m_nextTag = 1;
    private int m_reachedTag = 0;
    private int m_currentTime = 0;
    private int m_frameTime = 0;
    private ErrorCode m_lastSetTimeResult = null;
    private int m_getInfoTag = 0;
    private static File m_thumbnailDirectory = null;
    private static final int MSG_RENDER_REQ_EXPIRED = 1;
    private int mJPEGMaxWidth;
    private int mJPEGMaxHeight;
    private int mJPEGMaxSize;
    private ColorFormat mColorFormatTaskResult = null;
    private int mFastPreviewInProgressCount = 0;
    private String mPendingFastPreview = null;
    private int mPendingFastDisplay;
    private Surface viewSurface;
    private OnDiagnosticEventListener onDiagnosticEventListener;

    private NexVisualClipChecker m_VisualClipChecker = null;

    public static final int kAspectRatio_Mode_16v9 = 1;
    public static final int kAspectRatio_Mode_1v1 = 2;
    public static final int kAspectRatio_Mode_9v16 = 3;
    public static final int kAspectRatio_Mode_2v1 = 4;
    public static final int kAspectRatio_Mode_1v2 = 5;

    private int mAspectMode = kAspectRatio_Mode_16v9; //yoon

    public static final int kScreenMode_normal = 0; //yoon
    public static final int kScreenMode_horizonDual = 1;
    private int mScreenMode = kScreenMode_normal; //yoon
    private static boolean sPrepareSurfaceSetToNull = false; //yoon

    public final static int kLayerMode_ScreenDimension = 0;
    public final static int kLayerMode_Stretching = 1;

    private static int layerWidth = 1280;
    private static int layerHeight = 720;
    private static int layerMode = kLayerMode_ScreenDimension;

    private int assetNativeFD = -1;

    void setAspectMode(int aspectRatioMode) {
        mAspectMode = aspectRatioMode;
    }//yoon

    public static void setLayerScreen(int width, int height, int mode) {
        layerWidth = width;
        layerHeight = height;
        layerMode = mode;
    }

    public static int getLayerWidth() {
        return layerWidth;
    }

    public static int getLayerHeight() {
        return layerHeight;
    }

    public static int getLayerMode() {
        return layerMode;
    }

    int getAspectMode() {
        return mAspectMode;
    }//yoon

    public void setScreenMode(int mode) {
        mScreenMode = mode;
    }

    public int getScreenMode() {
        return mScreenMode;
    }

    private Thread thread_ = null;

    public void setFirstFaceThread(Thread thread) {

        thread_ = thread;
    }

    public void waitFaceThread() {

        if (thread_ == null) {

            return;
        }

        try {

            thread_.join();
        } catch (InterruptedException e) {

            e.printStackTrace();
        }

        thread_ = null;
    }

    public void onDiagnosticEvent(int p1, int p2, int p3, int p4) {
        if (onDiagnosticEventListener != null)
            onDiagnosticEventListener.onDiagnosticEvent(p1, p2, p3, p4);
    }

    public void setOnDiagnosticEventListener(OnDiagnosticEventListener onDiagnosticEventListener) {
        this.onDiagnosticEventListener = onDiagnosticEventListener;
    }

    public interface LayerRenderCallback {
        void renderLayers(LayerRenderer renderer);
    }

    public static class EditorInitException extends Exception {
        private static final long serialVersionUID = 1L;
        public final int errorCode;
        public final boolean hasErrorCode;

        public EditorInitException() {
            super();
            this.errorCode = 0;
            this.hasErrorCode = false;
        }

        public EditorInitException(String detailMessage, Throwable throwable) {
            super(detailMessage, throwable);
            this.errorCode = 0;
            this.hasErrorCode = false;
        }

        public EditorInitException(String detailMessage) {
            super(detailMessage);
            this.errorCode = 0;
            this.hasErrorCode = false;
        }

        public EditorInitException(String detailMessage, int errorCode) {
            super(detailMessage);
            this.errorCode = errorCode;
            this.hasErrorCode = true;
        }

        public EditorInitException(Throwable throwable) {
            super(throwable);
            this.errorCode = 0;
            this.hasErrorCode = false;
        }
    }

    private Task mColorFormatTask = null;

    /**
     * Detects the color format required for the video encoder on the current device.
     * <p>
     * In most cases, this does nothing (becausae we already know the correct color format), but
     * on certain devices the color format used is not consistent between different variations of
     * the same model (currently, Exynos devices with SDK level 16, as determined by
     * NexEditorDeviceProfile.getNeedsColorFormatCheck()).  Therefore, on these devices, we need
     * to check the color format.
     * <p>
     * After checking the color format, the EDITOR_PROP_EXPORT_COLOR_FORMAT property is set in the
     * engine, so that the correct color format will be used when exporting.
     */
    public Task detectAndSetEditorColorFormat(Context ctx) {
        if (LL.D) Log.d(LOG_TAG, "detectAndSetEditorColorFormat");
        if (mColorFormatTask == null) {
            mColorFormatTask = new Task();
            if (EditorGlobal.CHECK_COLOR_FORMAT_ON_EXYNOS
                    && NexEditorDeviceProfile.getDeviceProfile().getNeedsColorFormatCheck()) {
                ColorFormatChecker.checkColorFormat(ctx).onResultAvailable(new OnResultAvailableListener<ColorFormat>() {

                    @Override
                    public void onResultAvailable(ResultTask<ColorFormat> task, Event event, ColorFormat result) {
                        mColorFormatTaskResult = result;
                        if (result != ColorFormat.UNKNOWN) {
                            if (LL.D) Log.d(LOG_TAG, "Setting color format: " + result.name());
                            setProperty(ColorFormatChecker.EDITOR_PROP_EXPORT_COLOR_FORMAT, result.name());
                            mColorFormatTask.signalEvent(Event.COMPLETE);
                        } else {
                            mColorFormatTask.signalEvent(Event.FAIL);
                        }
                    }
                }).onFailure(new OnFailListener() {
                    @Override
                    public void onFail(Task t, Event e, TaskError failureReason) {
                        mColorFormatTask.sendFailure(failureReason);
                    }
                });
            } else {
                if (LL.D) Log.d(LOG_TAG, "Skip checking color format (not needed)");
                mColorFormatTask.signalEvent(Event.COMPLETE);
            }
        }
        return mColorFormatTask;
    }

    private static NexEditor s_latestInstance = null;

    public NexEditor(Context context, NexThemeView view, String effectiveModel, int userData, NexImageLoader.OverlayPathResolver overlayPathResolver, int[] props) throws EditorInitException {

        s_latestInstance = this;

        userData ^= 0x134A958E;

        String strLibPath = nexEditorUtils.getEngineNativeLibPath(context);

        setApplicationPacakge4Protection(context.getPackageName());
		check4km(context);
        int result = createEditor(strLibPath, effectiveModel, Build.VERSION.SDK_INT, initUserData() ^ userData, props == null ? null : Arrays.copyOf(props, props.length + 2));
        if (result != 0) {
            throw new EditorInitException("Editor Initialization Failed (result=" + result + ")");
        }

        if (!EditorGlobal.PROJECT_NAME.equals("KineMaster")) {
            if (NexEditorDeviceProfile.getDeviceProfile().getUserConfigSettings()) {
                setProperty("HardWareCodecMemSize", "" + NexEditorDeviceProfile.getDeviceProfile().getHardwareCodecMemSize());
                setProperty("HardWareDecMaxCount", "" + NexEditorDeviceProfile.getDeviceProfile().getHardwareDecMaxCount());
                setProperty("HardWareEncMaxCount", "" + NexEditorDeviceProfile.getDeviceProfile().getHardwareEncMaxCount());

                setProperty("FeatureVersion", "" + 3);
                setProperty("useNexEditorSDK", "" + 1);
                setProperty("DeviceExtendMode", "" + 1);
                setProperty("forceDirectExport", "" + NexEditorDeviceProfile.getDeviceProfile().getForceDirectExport()); //yoon

                setProperty("SupportedMaxFPS", "" + NexEditorDeviceProfile.getDeviceProfile().getMaxSupportedFPS());
                setProperty("InputMaxFPS", "" + NexEditorDeviceProfile.getDeviceProfile().getMaxSupportedFPS());

                setProperty("SupportFrameTimeChecker", "" + NexEditorDeviceProfile.getDeviceProfile().getSupportedTimeCheker()); //yoon
                setProperty("DeviceMaxLightLevel", "" + NexEditorDeviceProfile.getDeviceProfile().getDeviceMaxLightLevel()); //yoon
                setProperty("DeviceMaxGamma", "" + NexEditorDeviceProfile.getDeviceProfile().getDeviceMaxGamma()); //yoon

            } else {
                setProperty("HardWareCodecMemSize", "" + 4096 * 2176);
                if (EditorGlobal.PROJECT_NAME.equals("Gionee")) {
                    setProperty("HardWareDecMaxCount", "" + 2);
                } else {
                    setProperty("HardWareDecMaxCount", "" + 4);
                }
                setProperty("HardWareEncMaxCount", "" + 1);

                setProperty("FeatureVersion", "" + 3);

                setProperty("useNexEditorSDK", "" + 1); //yoon

                setProperty("forceDirectExport", "" + NexEditorDeviceProfile.getDeviceProfile().getForceDirectExport()); //yoon

                setProperty("DeviceExtendMode", "" + 1);
                setProperty("InputMaxFPS", "" + 120);    // 120FPS

                setProperty("SupportFrameTimeChecker", "" + 1); //yoon
            }
            setProperty("UseAndroidJPEG", "" + 1);
            setProperty("supportPeakMeter", "" + 0);

            int iSkipPrefetchEffect = 0;
            if (EditorGlobal.PROJECT_NAME.equals("Vivo")) {
                iSkipPrefetchEffect = 1;
            }
            setProperty("skipPrefetchEffect", "" + iSkipPrefetchEffect);

            try {
                AssetFileDescriptor afd = context.getAssets().openFd("nexasset.jpg");
                assetNativeFD = setInputFile(afd.getFileDescriptor(), 1, -1, -1);
                afd.close();
                if (LL.D) Log.d(LOG_TAG, "assetNativeFD is: " + assetNativeFD);
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            // For KineMaster
            setCodecLimits(-1, -1, -1);
            setProperty("FeatureVersion", "" + 3);
            setProperty("DeviceExtendMode", "" + 1);
            setProperty("InputMaxFPS", "" + NexEditorDeviceProfile.getDeviceProfile().getMaxSupportedFPS(1280, 720));
            setProperty("UseAndroidJPEG", NexEditorDeviceProfile.getDeviceProfile().getUseAndroidJPEGDecoder() ? "1" : "0");
            setProperty("WaitGLRenderFinish", NexEditorDeviceProfile.getDeviceProfile().getWaitToGLFinish() ? "1" : "0");
            setProperty("SupportFrameTimeChecker", NexEditorDeviceProfile.getDeviceProfile().getSupportFrameTimeChecker() ? "1" : "0");
        }

        DisplayMetrics dm = context.getResources().getDisplayMetrics();
        int largestDimension = Math.max(dm.widthPixels, dm.heightPixels);
        int size = 1024;
        while (size * 2 < largestDimension)
            size *= 2;

        mJPEGMaxWidth = size;
        mJPEGMaxHeight = size;
        mJPEGMaxSize = size * size;

        setProperty("JpegMaxWidthFactor", "" + mJPEGMaxWidth);
        setProperty("JpegMaxHeightFactor", "" + mJPEGMaxWidth);
        setProperty("JpegMaxSizeFactor", "" + mJPEGMaxSize);

        if (LL.D)
            Log.d(LOG_TAG, "largestDimension = " + largestDimension + " selectedSize=" + size);


        if (mView != view) {
            if (mView != null) {
                mView.linkToEditor(null);
            }
            view.linkToEditor(this);
            mView = view;
        }

        if (EditorGlobal.USE_ASSET_PACKAGE_MANAGER) {
            m_listener = new NexEditorEventListener(this, context, AssetPackageManager.getInstance(context).getResourceLoader(), overlayPathResolver);
        }
        setEventHandler(m_listener);

        String[] enc = AssetPackageReader.getRegisteredEncInfo();
        if (enc.length > 0) {
            NexEditor.setEncInfoString(enc);
        }
    }

    public void setCodecLimits(int hardwareDecMaxCount, int hardwareEncMaxCount, int hardwareCodecMemSize) {
        if (hardwareDecMaxCount <= -1) {
            /*if( EditorGlobal.CODEC_CAPA_TEST ) {
                hardwareDecMaxCount = 16;
            } else*/
            if (!NexEditorDeviceProfile.getDeviceProfile().isUnknownDevice() && NexEditorDeviceProfile.getDeviceProfile().getSupportsVideoLayers(false)) {
                hardwareDecMaxCount = 4;
            } else {
                hardwareDecMaxCount = NexEditorDeviceProfile.getDeviceProfile().getHardwareDecMaxCount();
            }
        }
        if (hardwareCodecMemSize <= -1) {
            hardwareCodecMemSize = NexEditorDeviceProfile.getDeviceProfile().getHardwareCodecMemSize();
        }
        if (hardwareEncMaxCount <= -1) {
            hardwareEncMaxCount = NexEditorDeviceProfile.getDeviceProfile().getHardwareEncMaxCount();
        }
        setProperty("HardWareCodecMemSize", String.valueOf(hardwareCodecMemSize));
        setProperty("HardWareDecMaxCount", String.valueOf(hardwareDecMaxCount));
        setProperty("HardWareEncMaxCount", String.valueOf(hardwareEncMaxCount));

    }

    public int getJPEGMaxWidth() {
        return mJPEGMaxWidth;
    }

    public int getJPEGMaxHeight() {
        return mJPEGMaxHeight;
    }

    public int getJPEGMaxSize() {
        return mJPEGMaxSize;
    }

    public void destroy() {
        if (s_latestInstance == this) {
            if (LL.D) Log.d(LOG_TAG, "destroy editor instance");
            onDestroy();
            destroyEditor();
        } else {
            if (LL.D) Log.d(LOG_TAG, "skip destroying editor instance (not latest instance)");
        }

        if (assetNativeFD >= 0) {
            closeInputFile(1, assetNativeFD);
        }

        m_listener.destroyRenderer();

        m_listener = null;
        if (mView != null) {
            mView.linkToEditor(null);
            mView = null;
        }
    }

    public NexVisualClipChecker getVisualClipChecker() {
        if (m_VisualClipChecker == null)
            m_VisualClipChecker = new NexVisualClipChecker(this);
        return m_VisualClipChecker;
    }

    public void resetVisualClipChecker() {
        m_VisualClipChecker = null;
    }

    public void setEventHandler(NexEditorListener listener) {
        m_uiListener = listener;
        if (m_listener != null) {
            m_listener.setUIListener(m_uiListener);
        }
    }

    public void setCustomRenderCallback(LayerRenderCallback layerRenderCallback) {
        if (m_listener != null) {
            m_listener.setCustomRenderCallback(layerRenderCallback);
        }
    }

    public NexEditorListener getEventHandler() {
        return m_uiListener;
    }

    public void setView(NexThemeView view) {
        if (mView != view) {
            if (mView != null) {
                mView.linkToEditor(null);
            }
            if (view != null) {
                view.linkToEditor(this);
            }
            mView = view;
            if (m_listener != null) {
                m_listener.setContext(mView == null ? null : mView.getContext());
            }
        }
    }

    public void setView(NexThemeView view, Context ctx) {
        if (mView != view) {
            if (mView != null) {
                mView.linkToEditor(null);
            }
            if (view != null) {
                view.linkToEditor(this);
            }
            mView = view;
            m_listener.setContext(mView == null ? ctx : mView.getContext());
        }
    }

    public NexThemeView getView() {
        return mView;
    }

    public synchronized int updateRenderInfo(int clipid, int face_detected, Rect start, Rect end, Rect face) {

        return updateRenderInfo(clipid, start.left, start.top, start.right, start.bottom, end.left, end.top, end.right, end.bottom, face_detected, face.left, face.top, face.right, face.bottom);
    }

    public int resetFaceDetectInfoJ(int clipid) {

        return resetFaceDetectInfo(clipid);
    }


    private String seekStateDump() {
        return " m_seekSerial=" + m_seekSerial + "; m_isSeeking=" + m_isSeeking + "; m_isPendingSeek=" + m_isPendingSeek + "; m_pendingSeekLocation=" + m_pendingSeekLocation + "; m_setTimeDoneListeners.size()=" + m_setTimeDoneListeners.size();
    }

    private String setTimeListenerDump() {
        int i = 0;
        String s = "";
        for (OnSetTimeDoneListener l : m_setTimeDoneListeners) {
            i++;
            s = s + "\n     " + i + ": " + l + " m_reqTime=" + l.m_reqTime + " m_serialNumber=" + l.m_serialNumber + " ";
            if (l.m_stackTrace != null) {
                int j = 0;
                for (StackTraceElement e : l.m_stackTrace) {
                    String className = e.getClassName();
                    if (className.equals("dalvik.system.VMStack")
                            || className.equals("java.lang.Thread")
                            || className.equals("dalvik.system.VMStack")
                            || className.equals("android.app.ActivityThread")
                            || className.equals("java.lang.reflect.Method")
                            || className.equals("com.android.internal.os.ZygoteInit")
                            || className.equals("com.android.internal.os.ZygoteInit$MethodAndArgsCaller:")
                            || className.equals("dalvik.system.NativeStart")
                            || className.equals("android.os.Looper")) {
                        j++;
                        continue;
                    }
                    s = s + "\n          " + j + ": " + className + "::" + e.getMethodName() + "(" + e.getFileName() + " " + e.getLineNumber() + ")";
                    j++;
                }
            }
        }
        return s;
    }

    /*
    protected void onSetTimeDone( int iCurrentTime ) {

		if(LL.D) Log.d(LOG_TAG,"onSetTimeDone(" + iCurrentTime + ")  m_setTimeDoneListeners.size()=" + m_setTimeDoneListeners.size() + setTimeListenerDump());
		if( m_setTimeDoneListeners.size()<1 )
			return;

		for(;;) {
			OnSetTimeDoneListener listener = m_setTimeDoneListeners.peek();
			if( listener==null )
				return;
			if( listener.m_tag > m_reachedTag )
				return;

			m_setTimeDoneListeners.remove();
    		listener.onSetTimeDone(iCurrentTime);
    		if(LL.D) Log.d(LOG_TAG,"-  onSetTimeDone(" + iCurrentTime + ") -> m_reqTime=" + listener.m_reqTime + " m_tag=" + listener.m_tag);
		}

    }

    protected void onSetTimeIgnored() {
		if(LL.D) Log.d(LOG_TAG,"onSetTimeIgnored()  m_setTimeDoneListeners.size()=" + m_setTimeDoneListeners.size() + setTimeListenerDump() );
		if( m_setTimeDoneListeners.size()<1 )
			return;

		for(;;) {
			OnSetTimeDoneListener listener = m_setTimeDoneListeners.peek();
			if( listener==null )
				return;
			if( listener.m_tag > m_reachedTag )
				return;

			m_setTimeDoneListeners.remove();
	    	listener.onSetTimeFail(ErrorCode.SET_TIME_IGNORED);
    		if(LL.D) Log.d(LOG_TAG,"-  onSetTimeIgnored() -> m_reqTime=" + listener.m_reqTime + " m_tag=" + listener.m_tag);
		}
    }
    */

    public interface OnIdleListener {
        void onIdle();
    }

    public interface OnEditorDestroyedListener {
        void onEditorDestroyed();
    }

    public interface OnTranscodingListener {
        void onTranscodingDone(ErrorCode result, int userTag);

        void onTranscodingProgress(int percent, int currentTime, int maxTime);
    }

    private OnIdleListener m_onIdleListener;
    private OnEditorDestroyedListener m_onEditorDestroyedListener;

    public void setOnIdleListener(OnIdleListener onIdleListener) {
        m_onIdleListener = onIdleListener;
    }

    public void setOnEditorDestroyedListener(OnEditorDestroyedListener onEditorDestroyedListener) {
        m_onEditorDestroyedListener = onEditorDestroyedListener;
    }

    protected void onIdle() {
        if (m_onIdleListener != null) {
            m_onIdleListener.onIdle();
        }
    }

    protected void onDestroy() {
        if (m_onEditorDestroyedListener != null) {
            m_onEditorDestroyedListener.onEditorDestroyed();
        }
    }

    private OnTranscodingListener m_transcodingListener;

    public void setOnTranscodingListener(OnTranscodingListener onTranscodingListener) {
        m_transcodingListener = onTranscodingListener;
    }

    protected void onTranscodingProgress(int percent, int currentTime, int maxTime) {
        if (m_transcodingListener != null) {
            m_transcodingListener.onTranscodingProgress(percent, currentTime, maxTime);
        }
    }

    protected void onTranscodingDone(ErrorCode result, int userTag) {
        if (LL.D)
            Log.d(LOG_TAG, "-  onTranscodingDone() result : " + result + " mTranscoding =" + mTranscoding);
        if (!mTranscoding && !mReverseTranscoding) {
            return;
        }
        mTranscoding = false;
        mReverseTranscoding = false;
        if (m_transcodingListener != null) {
            m_transcodingListener.onTranscodingDone(result, userTag);
        }
    }

    private Task fastOptionPreviewTask = null;

    protected void onFastOptionPreviewDone(ErrorCode result) {
        if (result.isError()) {
            if (LL.W) Log.d(LOG_TAG, "onFastOptionPreviewDone:" + result.getMessage());
        }
        mFastPreviewInProgressCount--;
        if (m_lastSetTimeResult != ErrorCode.NONE) {
            if (mFastPreviewInProgressCount < 1 && fastOptionPreviewTask != null) {
                fastOptionPreviewTask.sendFailure(ErrorCode.FAST_PREVIEW_IGNORED);
                fastOptionPreviewTask = null;
            }
        } else if (mPendingFastPreview != null && mFastPreviewInProgressCount < MAX_FAST_PREVIEW_COUNT) {
            if (m_isSeeking) {
                if (LL.W) Log.d(LOG_TAG, "onFastOptionPreviewDone: stat is seeking");
                mPendingFastPreview = null;
                return;
            }

            int rqResult = fastOptionPreview(mPendingFastPreview, mPendingFastDisplay);
            mPendingFastPreview = null;
            if (rqResult == 0) {
                mFastPreviewInProgressCount++;
            } else {
                if (LL.W) Log.d(LOG_TAG, "onFastOptionPreviewDone: pending result=" + rqResult);
            }
        } else if (mFastPreviewInProgressCount < 1) {
            if (fastOptionPreviewTask != null) {
                fastOptionPreviewTask.signalEvent(Event.SUCCESS, Event.COMPLETE);
                fastOptionPreviewTask = null;
            }
        }
    }

    protected void onPlayError(ErrorCode resultCode) {
        if (LL.W) Log.w(LOG_TAG, "onPlayError:" + resultCode);
        if (m_exportingTask != null) {
            if (resultCode.isError()) {
                m_exportingTask.sendFailure(resultCode);
                m_exportingTask = null;
                return;
            }
        }
    }


    protected void onPlay(ErrorCode resultCode) {

        if (LL.D) Log.d(LOG_TAG, "onPlay:" + resultCode);

        if (m_exportingTask != null) {
            if (resultCode.isError()) {
                m_exportingTask.sendFailure(resultCode);
                m_exportingTask = null;
                return;
            }
        }

        m_isPlaying = true;
        if (m_onPlayListeners.size() < 1)
            return;

        OnPlayListener listener = m_onPlayListeners.remove();
        listener.onPlayRequestProcessed(resultCode);
        if (resultCode == ErrorCode.NONE) {
            m_onPlayStartListeners.add(listener);
        }
    }

    protected void onPlayStart() {
        m_playRequested = false;
        if (m_onPlayStartListeners.size() < 1)
            return;

        OnPlayListener listener = m_onPlayStartListeners.remove();
        listener.onPlayStarted();
    }

    protected void onStop(ErrorCode resultCode) {
        if (LL.D) Log.d(LOG_TAG, "onStop : m_onStopListeners.size()=" + m_onStopListeners.size());

        m_isPlaying = false;
        if (m_exportingTask != null) {
            if (resultCode.isError()) {
                m_exportingTask.sendFailure(resultCode);
                m_exportingTask = null;
            } else {
//                m_exportingTask.sendFailure(ErrorCode.EXPORT_UNEXPECTED_STOP);
            }
        }
        OnCompletionListener listener;
        while ((listener = m_onStopListeners.poll()) != null) {
            listener.onComplete(resultCode);
        }
    }

    protected void onEncodingDone(NexEditor.ErrorCode result) {
        if (m_exportingTask != null) {
            if (result.isError()) {
                m_exportingTask.sendFailure(result);
            } else {
                m_exportingTask.signalEvent(Event.SUCCESS, Event.COMPLETE);
            }
            m_exportingTask = null;
            //yoon
            closeOutPutPath();
        }
    }

    //yoon start
    public interface OnGetThumbDoneListener {
        void onGetThumbDoneListener(int iMode, int iTime, int iWidth, int iHeight, int iSize, byte[] arrayData, int iCount, int iTotal, int iTag);
    }

    private OnGetThumbDoneListener OnGetThumbDoneListener;

    public void setOnGetThumbDoneListener(OnGetThumbDoneListener listener) {
        OnGetThumbDoneListener = listener;
    }

    protected void onThumbDone(int iMode, int iTime, int iWidth, int iHeight, int iSize, byte[] arrayData, int iCount, int iTotal, int iTag) {
        if (OnGetThumbDoneListener != null) {
            OnGetThumbDoneListener.onGetThumbDoneListener(iMode, iTime, iWidth, iHeight, iSize, arrayData, iCount, iTotal, iTag);
        }
    }
//yoon end

    public interface OnHighLightDoneListener {
        int onHighLightDoneListener(int iCount, int[] arrayData);

        int onHighLightErrorListener(int errcode);
    }

    private OnHighLightDoneListener OnHighLightDoneListener;

    public void setOnHighLightDoneListener(OnHighLightDoneListener listener) {
        OnHighLightDoneListener = listener;
    }

    protected void onHighLightDone(int iCount, int[] arrayData) {
        if (OnHighLightDoneListener != null) {
            OnHighLightDoneListener.onHighLightDoneListener(iCount, arrayData);
        }
    }

    protected void onHighLightError(int errcode) {
        if (OnHighLightDoneListener != null) {
            OnHighLightDoneListener.onHighLightErrorListener(errcode);
        }
    }

    protected void onCommandMarker(ErrorCode resultCode, int iTag) {
        m_reachedTag = iTag;
        if (LL.D) Log.d(LOG_TAG, "REACHED MARKER " + iTag);
        for (; ; ) {
            OnCompletionListener listener = m_onCommandMarkerListeners.peek();
            if (listener == null) {
                if (LL.D)
                    Log.d(LOG_TAG, "    - onCommandMarker(" + resultCode + "," + iTag + "): Skipping because listener is null");
                return;
            } else if (listener.m_tag > iTag) {
                if (LL.D)
                    Log.d(LOG_TAG, "    - onCommandMarker(" + resultCode + "," + iTag + "): Skipping because " + listener.m_tag + ">" + iTag);
                return;
            }

            if (LL.D)
                Log.d(LOG_TAG, "    - onCommandMarker(" + resultCode + "," + iTag + "): Notifying because " + listener.m_tag + "<=" + iTag);
            listener = m_onCommandMarkerListeners.remove();
            listener.onComplete(resultCode);
        }
    }

    protected void onCaptureDone(int width, int height, int size, byte[] pixelData, boolean reverse) {

        if (m_onCaptureListeners.size() < 1)
            return;

        if (width < 1 || height < 1 || size < 1 || pixelData == null) {
            onCaptureFail(ErrorCode.CAPTURE_FAILED);
        }

        try {
            OnCaptureListener listener = m_onCaptureListeners.remove();

            if (reverse == true) {

                byte[] rowData = new byte[width * 4];
                for (int i = 0; i < height / 2; i++) {
                    System.arraycopy(pixelData, width * i * 4, rowData, 0, width * 4);
                    System.arraycopy(pixelData, width * (height - 1 - i) * 4, pixelData, width * i * 4, width * 4);
                    System.arraycopy(rowData, 0, pixelData, width * (height - 1 - i) * 4, width * 4);
                }
            }

            Bitmap bm = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            bm.copyPixelsFromBuffer(ByteBuffer.wrap(pixelData));

            listener.onCapture(bm);
        } catch (NoSuchElementException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    protected void onCaptureFail(ErrorCode error) {

        if (m_onCaptureListeners.size() < 1)
            return;

        OnCaptureListener listener = m_onCaptureListeners.remove();

        listener.onCaptureFail(error);
    }

    protected boolean onExportFail(ErrorCode error) {

        if (m_onExportListeners.size() < 1)
            return false;

        OnExportListener listener = m_onExportListeners.remove();
        listener.onExportFail(error);
        m_exportingTask = null;
        return true;
    }

    protected boolean onExportProgress(int curTime) {
        if (m_onExportListeners.size() < 1)
            return false;

        OnExportListener listener = m_onExportListeners.getFirst();
        listener.onExportProgress(curTime);
        return true;
    }

    protected boolean onExportDone() {

        if (m_onExportListeners.size() < 1)
            return false;

        OnExportListener listener = m_onExportListeners.remove();
        listener.onExportDone();
        m_exportingTask = null;
        return true;
    }

    protected void onAddClipDone(int returnCode, int totalDuration, int addedClipId, int addedClipType) {
        // uiParam4 : added clip type  (0=None, 1=Image, 2=Text, 3=Audio, 4=Video)

        if (addedClipType == 1 || addedClipType == 4 || addedClipType == 0) {
            // Visual clip
            if (m_addVisualClipDoneListeners == null || m_addVisualClipDoneListeners.size() < 1) {
                if (LL.D) Log.d(LOG_TAG, "Ignore onAddClipDone event -- no listeners");
                return;
            }
            for (OnAddClipDoneListener listener : m_addVisualClipDoneListeners) {
                if (listener.clipId == addedClipId) {
                    m_addVisualClipDoneListeners.remove(listener);
                    if (returnCode == 0) {
                        listener.onAddClipDone(addedClipId, totalDuration, addedClipType);
                    } else {
                        listener.onAddClipFail(ErrorCode.fromValue(returnCode));
                    }
                    return;
                }
            }
        }
        if (addedClipType == 3 || addedClipType == 0) {
            // Audio clip
            if (m_addAudioClipDoneListeners == null || m_addAudioClipDoneListeners.size() < 1) {
                if (LL.D) Log.d(LOG_TAG, "Ignore onAddClipDone event -- no listeners");
                return;
            }
            for (OnAddClipDoneListener listener : m_addAudioClipDoneListeners) {
                if (listener.clipId == addedClipId) {
                    m_addAudioClipDoneListeners.remove(listener);
                    if (returnCode == 0) {
                        listener.onAddClipDone(addedClipId, totalDuration, addedClipType);
                    } else {
                        listener.onAddClipFail(ErrorCode.fromValue(returnCode));
                    }
                    return;
                }
            }
        }

    }

    protected void onDeleteClipDone(int returnCode) {
        OnDeleteClipDoneListener listener = m_deleteClipDoneListeners.remove();
        if (returnCode == 0) {
            listener.onDeleteClipDone();
        } else {
            listener.onDeleteClipFail(ErrorCode.fromValue(returnCode));
        }
    }


    protected void onMoveClipDone(int returnCode, int movedClipId) {
        if (m_movedClipId.size() < 1 || m_moveClipDoneListeners.size() < 1)
            return;

        int waitingForId = m_movedClipId.peek();
        if (waitingForId != movedClipId)
            return;

        m_movedClipId.remove();

        OnMoveClipDoneListener listener = m_moveClipDoneListeners.remove();
        if (returnCode == 0) {
            listener.onMoveClipDone(movedClipId);
        } else {
            listener.onMoveClipFail(ErrorCode.fromValue(returnCode));
        }
    }

    private native int getNativeSDKInfoWM();

    private native int initUserData();
    private native int check4km(Context context);
    private native int createEditor(String strLibPath, String strModelName, int iAPILevel, int iUserData, int[] properties);
    private native int destroyEditor();

    private native int commandMarker(int iTag);

    private native int prepareSurface(Surface surface);

    protected native int changeSurface(int iWidth, int iHeight);

    public native int prepareAudio(AudioTrack audioTrack, int iSamplingRate, int iChannels, int iSampleForChannel);

    public native int createProject();

    public native int openProject(String strFilePath);

    public native int saveProject(String strFilePath);

    public native int closeProject();

    public native int setProjectEffect(String strFilePath);

    // 0x00000000 set clip orientation with 0.
    // 0x00000010 set clip orientation with 90.
    // 0x00000020 set clip orientation with 180.
    // 0x00000040 set clip orientation with 270.
    private native int encodeProject(String strFilePath, int iWidth, int iHeight, int iBitrate, long iMaxFileSize, int iMaxFileDuration, int iFPS, int iProjectWidth, int iProjectHeight, int iSamplingRate, int iAudioBitrate, int iEncodeProfile, int iEncodeLevel, int iVideoCodecType, int iFlag);

    private native int encodeProjectJpeg(Surface surface, String strFilePath, int iWidth, int iHeight, int iQuality, int iFlag);

    public native int encodePause();

    public native int encodeResume();

    private native int addVisualClip(int iNextToClipID, String strFilePath, int iNewID);

    private native int addAudioClip(int iStartTime, String strFilePath, int iNewID);

    private native int moveVisualClip(int iNextToClipID, int iMovedClipID);

    private native int moveAudioClip(int iStartTime, int iMovedClipID);

    private native int deleteClipID(int iClipID);

    private native int updateVisualClip(NexVisualClip clipInfo);  // DEPRECATED: Changed to private to prevent accidental calls

    private native int updateAudioClip(NexAudioClip clipInfo);  // DEPRECATED: Changed to private to prevent accidental calls

    private native int setBackgroundMusic(String strFilePath, int iNewID);

    public native int setBackgroundMusicVolume(int iVolume, int iFadeInTime, int iFadeOutTime);

    public native int setProjectVolumeFade(int iFadeInTime, int iFadeOutTime);

    public native int setProjectVolume(int iProjectVolume);

    public native int setProjectManualVolumeControl(int iManualVolumeControl);

    public native int getDuration();

    private native int setTime(int iTime, int iDisplay, int iIDRFrame);// Only display idrfreme when idrframe is 1)

    private native int captureCurrentFrame();

    private native int startPlay(int iMuteAudio);

    private native int pausePlay();

    private native int resumePlay();

    private native int stopPlay(int iFlag);

    public native NexVisualClip getVisualClip(int iClipID);

    public native NexAudioClip getAudioClip(int iClipID);

    private native int loadClipList(NexVisualClip[] visualClipList, NexAudioClip[] audioClipList);

    private native int getClipList(ClipCallback enumerator);

    private native int loadTheme(String strEffectID, String strEffectData, int iFlags);// 1: preview  other : reserve

    private native int clearRenderItems(int iFlags);// 1: preview  other : reserve

    private native int loadRenderItem(String strEffectID, String strEffectData, int iFlags);// 1: preview  other : reserve

    private native String getLoadThemeError(int iFlags);

    private native String getThemeID();

    private native int setThemeID(String strThemeID);

    private native int getThemeList(ThemeItemCallback manager);

    private native int getClipEffectList(ThemeItemCallback manager);

    private native int getTitleEffectList(ThemeItemCallback manager);

    public native int setTemporaryDirectory(String strTemporaryPath);

    //public native int setThumbnailDirectory(String strThumbnailPath);
    private native int setEventHandler(NexEditorEventListener eventListener);

    private native int getClipInfo(String strClipPath, String strThumbPath, NexClipInfo clipInfo, int isVideoThumbnail, int isPCMLevel, int isBackground, int iUserTag);

    private native int getClipInfoSync(String strClipPath, NexClipInfo clipInfo, int iFlag, int iUserTag);

    /*
	param iFlag
	    0x00000001 : get IDR frame thumbnail without skip
	    	    for example
			IDR : 0, 1000, 2000, 3000, 4000, 5000, 6000, 7000 and ...
			If iFlag is 0x00000001 : Thumbnail times are 0, 1000, 2000, 3000, 4000, 5000, 6000, 7000 and ...
			If iFlag is 0x00000000 : Thumbnail times are 0, 2000, 4000, 6000, and ...

			#define GET_THUMBRAW_RGB									(0x00000000)
			#define GET_THUMBRAW_YUV									(0x00010000)
			#define GET_THUMBRAW_Y_ONLY									(0x00020000)
			#define GET_THUMBRAW_RGB_CALLBACK							(0x00040000)

			If iFlag is 0x00000000 : Rotate 0, need resolution change
			If iFlag is 0x00000010 : Rotate 90. need resolution change
			If iFlag is 0x00000020 : Rotate 180. need resolution change
			If iFlag is 0x00000040 : Rotate 270. need resolution change
	*/
    private native int getClipVideoThumb(String strClipPath, String strThumbPath, int iWidth, int iHeight, int iStartTime, int iEndTime, int iCount, int iFlag, int iUserTag);

    private native int getClipVideoThumbWithTimeTable(String strClipPath, String strThumbPath, int iWidth, int iHeight, int iSize, int[] arrayTables, int iFlag, int iUserTag);

    private native int getClipAudioThumb(String strClipPath, String strThumbPath, int iUserTag);

    private native int getClipAudioThumbPartial(String clipPath, String pThumbFilePath, int iStartTime, int iEndTime, int iUseCount, int iSkipCount, int iUserTag);

    private native int clearScreen(int iTag);

    private native int fastOptionPreview(String strOption, int iDisplay);

    public native int clearTrackCache();

    private static native String getSystemProperty(String name);

    public native int startVoiceRecorder(String strFilePath, int iSampleRate, int iChannel, int iBitForSample);

    public native int processVoiceRecorder(byte[] arrayPCM, int iPCMLen);

    public native int endVoiceRecorder(NexClipInfo clipInfo);

    public native String getProperty(String name);

    public native int setProperty(String name, String value);

    // iTag
    // 0x00000001 rotate using clip rotate info.
    private native int transcodingStart(String strSrcClipPath, String strDstClipPath, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, int iBitrate, long iMaxFileSize, int iFPS, int iTag, String strUserData);

    private native int transcodingStop();

    public native int reverseStart(String strSrcClipPath, String strDstClipPath, String strTempClipPath, int iWidth, int iHeight, int iBitrate, long lMaxFileSize, int iStartTime, int iEndTime, int iDecodeMode);

    public native int reverseStop();

    // 0x00000001 : disable clear project
    // 0x00000010 : disable clear texture
    private native int loadList(NexVisualClip[] visualClipList, NexAudioClip[] audioClipList, int iOption);

    private native int asyncLoadList(NexVisualClip[] visualClipList, NexAudioClip[] audioClipList, int iOption);

    public native int asyncDrawInfoList(NexDrawInfo[] top, NexDrawInfo[] sub);

    public native int updateDrawInfo(NexDrawInfo drawinfo);

    private native int clearList();

    public native int clearProject(); //yoon

    /* iRet
        0 : support direct encode
        1 : Invalid editor handle
        2 : Invalid clip list handle
        3 : Clip count is 0 or clip error
        4 : Video information was different.
        5 : Video codec unmatch
		6 : Video not start idr frame
		7 : Has image clip.
		8 : Has video layer.
		9 : Enable transition effect
		10 : Enable title effect
        11 : Encoder dsi unmatch with dsi of clip
        12 : applied speed control
        13 : Unsupport codec.
        14 : Rotate invalid.
        15 : Multi track content.
        16 : Has audio tracks.
        17 : Audio Codec unmatch
    */
    public native int checkDirectExport(int iOption);

    public native int directExport(String strFilePath, long iMaxFileSize, long iMaxFileDuration, String strUserData, int iFlag);

    private native int getClipStopThumb(int iUserTag);

    public native int checkPFrameDirectExportSync(String strFilePath);

    public native int fastPreviewStart(int iStartTime, int iEndTime, int displayWidth, int displayHeight);

    public native int fastPreviewStop();

    public native int fastPreviewTime(int iSetTime);

    public native int checkIDRStart(String strFilePath);

    public native int checkIDREnd();

    public native int checkIDRTime(int iTime);

    public native int setVolumeWhilePlay(int iMasterVolume, int iSlaveVolume);

    public native int[] getVolumeWhilePlay();

    public native int addUDTA(int iType, String strData);

    public native int clearUDTA();

    //Jeff
    public native int setBaseFilterRenderItem(String uid);

    private native int getTexNameForClipID(int export_flag, int clipid);
    
    private native int getTexNameForMask(int export_flag);

    private native int getTexNameForBlend(int export_flag);

	private native int swapBlendMain(int export_flga);

    private native int getTexNameForWhite(int export_flag);

    private native int setRenderToMask(int export_flag);

    private native int setRenderToDefault(int export_flag);

    private native int setTexNameForLUT(int export_flag, int clipid, float x, float y);

    private native int cleanupMaskWithWhite(int export_flag);

    private native int createRenderItem(String effect_id, int export_flag);

    private native int releaseRenderItem(int effect_id, int export_flag);

    private native int drawRenderItemOverlay(int effect_id, int tex_if_for_second, String usersettings, int export_flag, int current_time, int start_time, int end_time, float[] matrix, float left, float top, float right, float bottom, float alpha, boolean mask_enabled);

    private native int createNexEDL(String effect_id, int export_flag);

    private native int releaseNexEDL(int effect_id, int export_flag);

    private native int drawNexEDLOverlay(int effect_id, String usersettings, int export_flag, int current_time, int start_time, int end_time, float[] matrix, float left, float top, float right, float bottom, float alpha, int mode);

    private native int setPreviewScaleFactor(float fScaleFactor);

    private native float getPreviewScaleFactor();

    private native int pushLoadedBitmap(String path, int[] pixels, int width, int height, int loadedtype);

    private native int removeBitmap(String path);

    private native int updateRenderInfo(int clipid, int start_left, int start_top, int start_right, int start_bottom, int end_left, int end_top, int end_right, int end_bottom, int face_detected, int face_left, int face_top, int face_right, int face_bottom);

    private native int resetFaceDetectInfo(int clipid);

    private native int highlightStart(String path, int indexMode, int interval, int count, int outputMode, String outputPath, int width, int height, int bitrate, long maxFileSize, int iDecodeMode);

    private native int highlightStop();

    //yoon
    private native int setOutputFile(FileDescriptor fileDescriptor, long offset, long length);

    private native void closeOutputFile(int setOutputFD);

    private native int setInputFile(FileDescriptor fileDescriptor, int predef, long offset, long length);

    private native int closeInputFile(int predef, int setInputFD);

    private native int setVideoTrackUUID(int iMode, byte[] arrayUUID);

    private native int getAudioSessionID();

    private native int set360VideoTrackPosition(int angleX, int angleY, int flags);

    private native int releaseLUTTexture(int lut_resource_id);

    private native byte[] getTimeThumbData(int time);

    public native int setTaskSleep(int sleep);

    public native int[] createLGLUT(byte[] stream);

    public native int[] createCubeLUT(byte[] stream);

    public native int setBrightness(int value);

    public native int setContrast(int value);

    public native int setSaturation(int value);

    public native int getBrightness();

    public native int getContrast();

    public native int getSaturation();

    public native void setVignette(int value);

    public native int getVignette();

    public native void setVignetteRange(int value);

    public native int getVignetteRange();

    public native void setSharpness(int value);

    public native int getSharpness();

    public native int setDeviceLightLevel(int lightLevel);

    public native int setDeviceGamma(float gamma);

    public native int setForceRTT(int val);

    private native int setThumbnailRoutine(int value);

    private native int setGIFMode(int mode);

	public native int aquireContext(int export_flag);
	public native int releaseContext(int export_flag);

    public int removeBitmapJ(String path) {

        return removeBitmap(path);
    }

    public int pushLoadedBitmapJ(String path, int[] pixels, int width, int height, int loadedtype) {

        return pushLoadedBitmap(path, pixels, width, height, loadedtype);
    }

    public int getTexNameForVideoLayer(int export_flag, int clipid) {
        return getTexNameForClipID(export_flag, clipid);
    }
    
    public int getTexNameForMaskJ(int export_flag) {
        return getTexNameForMask(export_flag);
    }

   	public int getTexNameForBlendJ(int export_flag){
		return getTexNameForBlend(export_flag);
	}

	public int beginBlend(int export_flag){

		return swapBlendMain(export_flag);
	}

	public int endBlend(int export_flag){

		return swapBlendMain(export_flag);
	}

    public int getTexNameForWhiteJ(int export_flag) {
        return getTexNameForWhite(export_flag);
    }

    public int cleanupMaskWithWhiteJ(int export_flag) {
        return cleanupMaskWithWhite(export_flag);
    }

    public int drawRenderItemOverlayJ(int effect_id, int tex_id_for_second, String usersettings, int export_flag, int current_time, int start_time, int end_time, float[] matrix, float left, float top, float right, float bottom, float alpha, boolean mask_enabled){

		return drawRenderItemOverlay(effect_id, tex_id_for_second, usersettings, export_flag, current_time, start_time, end_time, matrix, left, top, right, bottom, alpha, mask_enabled);
	}

    public int createRenderItemJ(String effect_id, int export_flag) {

        return createRenderItem(effect_id, export_flag);
    }

    public int releaseRenderItemJ(int effect_id, int export_flag) {

        return releaseRenderItem(effect_id, export_flag);
    }

    public int drawNexEDLOverlayJ(int effect_id, String usersettings, int export_flag, int current_time, int start_time, int end_time, float[] matrix, float left, float top, float right, float bottom, float alpha, int mode) {

        return drawNexEDLOverlay(effect_id, usersettings, export_flag, current_time, start_time, end_time, matrix, left, top, right, bottom, alpha, mode);
    }

    public int createNexEDLJ(String effect_id, int export_flag) {

        return createNexEDL(effect_id, export_flag);
    }

    public int releaseNexEDLJ(int effect_id, int export_flag) {

        return releaseNexEDL(effect_id, export_flag);
    }

    public int setRenderToMaskJ(int export_flag) {
        return setRenderToMask(export_flag);
    }

    public int setRenderToDefaultJ(int export_flag) {
        return setRenderToDefault(export_flag);
    }

    public int setTexNameForLUTLayer(int export_flag, int clipid, float x, float y) {
        return setTexNameForLUT(export_flag, clipid, x, y);
    }

    public int getPropertyInt(String name, int default_value) {
        String value = getProperty(name);
        if (value == null) {
            return default_value;
        }
        try {
            return Integer.parseInt(value);
        } catch (NumberFormatException e) {
            return default_value;
        }
    }

    public boolean getPropertyBool(String name, boolean default_value) {
        String value = getProperty(name);
        if (value == null) {
            return default_value;
        } else if (value.trim().equalsIgnoreCase("true")) {
            return true;
        } else if (value.trim().equalsIgnoreCase("false")) {
            return false;
        } else {
            return default_value;
        }
    }

    public void prepareSurfaceForView(Surface surface) {
        viewSurface = surface;

        if (mCaptureInProgress) {
            Log.d(LOG_TAG, "prepareSurface wait. image exporting...");
            return;
        }

        if (!mCaptureFrameInProgress) {
            if (sPrepareSurfaceSetToNull) {
                prepareSurface(null);
            }
            if (surface != null) {
                setPreviewScaleFactor(mPreviewScaleFactor);
            }
            prepareSurface(surface);
        }
    }

    private static boolean sLinkError = false;

    public static String getSysProperty(String name) {
        try {
            return getSystemProperty(name);
        } catch (UnsatisfiedLinkError e) {
            sLinkError = true;
            return "";
        }
    }

    public static boolean hadLinkErorr() {
        return sLinkError;
    }

    public ErrorCode transcodingStart_internal(String strSrcClipPath, String strDstClipPath, int iWidth, int iHeight, int iBitrate, long iMaxFileSize, String strUserData) {
        if (mTranscoding || mReverseTranscoding) {
            return ErrorCode.TRANSCODING_BUSY;
        }
        ErrorCode result = ErrorCode.fromValue(transcodingStart(strSrcClipPath, strDstClipPath, iWidth, iHeight, iWidth, iHeight, iBitrate, iMaxFileSize, 30, 0, strUserData));
        if (!result.isError()) {
            mTranscoding = true;
        }
        return result;
    }

    public ErrorCode transcodingStart_internal(String strSrcClipPath, String strDstClipPath, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, int iBitrate, long iMaxFileSize, int iFPS, int iTag, String strUserData) {
        if (mTranscoding || mReverseTranscoding) {
            return ErrorCode.TRANSCODING_BUSY;
        }
        ErrorCode result = ErrorCode.fromValue(transcodingStart(strSrcClipPath, strDstClipPath, iWidth, iHeight, iDisplayWidth, iDisplayHeight, iBitrate, iMaxFileSize, iFPS, iTag, strUserData));
        if (!result.isError()) {
            mTranscoding = true;
        }
        return result;
    }

    public ErrorCode reverseTranscodingStart_internal(String strSrcClipPath, String strDstClipPath, String tmpClipPath, int iWidth, int iHeight, int iBitrate, long iMaxFileSize, int startTime, int endTime) {
        if (mTranscoding || mReverseTranscoding) {
            return ErrorCode.TRANSCODING_BUSY;
        }
        ErrorCode result = ErrorCode.fromValue(reverseStart(strSrcClipPath, strDstClipPath, tmpClipPath, iWidth, iHeight, iBitrate, iMaxFileSize, startTime, endTime, 0));
        if (!result.isError()) {
            mReverseTranscoding = true;
        }
        return result;
    }

    public ErrorCode transcodingStop_internal() {
        if (mTranscoding) {
            return ErrorCode.fromValue(transcodingStop());
        } else if (mReverseTranscoding) {
            return ErrorCode.fromValue(reverseStop());
        } else {
            return ErrorCode.NO_ACTION;
        }
    }

    private boolean mTranscoding = false;
    private boolean mReverseTranscoding = false;

    public boolean isTranscoding() {
        return mTranscoding || mReverseTranscoding;
    }


    public boolean canUseSoftwareCodec() {
        String s = getProperty("canUseSoftwareCodec");
        if (s.equals("true")) {
            if (LL.D) Log.d("NexExprortFragment", "canUseSoftwareCodec == " + "true");
            return true;
        } else if (s.equals("false")) {
            if (LL.D) Log.d("NexExprortFragment", "canUseSoftwareCodec == " + "false");
            return false;
        } else {
            if (LL.D)
                Log.d("NexExprortFragment", "canUseSoftwareCodec == " + "throw Internal Error");
            throw new InternalError();
        }
    }

    public int setThumbnailDirectory(String strThumbnailPath) {
        m_thumbnailDirectory = new File(strThumbnailPath);
        return 0;
    }

    public static enum FastPreviewOption {
        normal, brightness, contrast, saturation, adj_brightness, adj_contrast, adj_saturation, tintColor, hue,
        left, top, right, bottom, nofx, cts, swapv, video360flag, video360_horizontal, video360_vertical, adj_vignette, adj_vignetteRange, adj_sharpness,
        customlut_clip, customlut_power,
    }

    public void fastPreview(FastPreviewOption option, int value) {
        fastPreview(option, value, true);
    }

    public void fastPreview(final Map<NexEditor.FastPreviewOption, Integer> options) {
        if (mCaptureInProgress)
            return;
        StringBuilder optstr = new StringBuilder();

        for (Entry<FastPreviewOption, Integer> entry : options.entrySet()) {
            if (optstr.length() > 0) {
                optstr.append(' ');
            }
            optstr.append(entry.getKey().name());
            optstr.append('=');
            optstr.append(entry.getValue());
        }

        fastOptionPreviewWrapper(optstr.toString(), 1);
    }

    public interface FastPreviewBuilder {
        public FastPreviewBuilder option(FastPreviewOption option, int value);

        public FastPreviewBuilder cropRect(int left, int top, int right, int bottom);

        public FastPreviewBuilder cropRect(Rect rc);

        public FastPreviewBuilder swapv(boolean swapv);

        public FastPreviewBuilder cts(int cts);

        public FastPreviewBuilder video360(int x, int y);

        public void execute();

        public void executeNoDisplay();
    }

    public FastPreviewBuilder buildFastPreview() {
        return new FastPreviewBuilder() {
            private Map<FastPreviewOption, Integer> options = new EnumMap<>(FastPreviewOption.class);

            @Override
            public FastPreviewBuilder option(FastPreviewOption option, int value) {
                options.put(option, value);
                return this;
            }

            @Override
            public FastPreviewBuilder cropRect(int left, int top, int right, int bottom) {
                return option(FastPreviewOption.left, left)
                        .option(FastPreviewOption.top, bottom)
                        .option(FastPreviewOption.right, right)
                        .option(FastPreviewOption.bottom, top);
            }

            @Override
            public FastPreviewBuilder cropRect(Rect rc) {
//                return  option(FastPreviewOption.left,rc.left)
//                        .option(FastPreviewOption.top,rc.top)
//                        .option(FastPreviewOption.right,rc.right)
//                        .option(FastPreviewOption.bottom,rc.bottom);
                return option(FastPreviewOption.left, rc.left)
                        .option(FastPreviewOption.top, rc.bottom)
                        .option(FastPreviewOption.right, rc.right)
                        .option(FastPreviewOption.bottom, rc.top);
            }

            @Override
            public FastPreviewBuilder swapv(boolean swapv) {
                return option(FastPreviewOption.swapv, swapv ? 1 : 0);
            }

            @Override
            public FastPreviewBuilder cts(int cts) {
                return option(FastPreviewOption.cts, cts);
            }

            @Override
            public FastPreviewBuilder video360(int x, int y) {
                return option(FastPreviewOption.video360flag, 1)
                        .option(FastPreviewOption.video360_horizontal, x)
                        .option(FastPreviewOption.video360_vertical, y);
            }

            @Override
            public void execute() {
                execute(true);
            }

            @Override
            public void executeNoDisplay() {
                execute(false);
            }

            private void execute(boolean display) {
                StringBuilder s = new StringBuilder();
                for (Entry<FastPreviewOption, Integer> e : options.entrySet()) {
                    if (s.length() > 0)
                        s.append(' ');
                    s.append(e.getKey().name());
                    s.append('=');
                    s.append(e.getValue());
                }
                fastOptionPreviewWrapper(s.toString(), display ? 1 : 0);
            }

        };
    }

    public boolean fastPreview(FastPreviewOption option, int value, boolean display) {
        if (mCaptureInProgress)
            return false;
        return fastOptionPreviewWrapper(option.name() + "=" + value, display ? 1 : 0);
    }

    private void fastPreviewForCapture(FastPreviewOption option, int value, boolean display) {
        fastOptionPreviewWrapper(option.name() + "=" + value, display ? 1 : 0);
    }

    private Task getFastOptionPreviewTask() {
        if (fastOptionPreviewTask == null)
            fastOptionPreviewTask = new Task();
        return fastOptionPreviewTask;
    }

    //    private boolean fastPreviewInProgress = false;
//    private String pendingFastPreviewOption;
//    private int pendingFastPreviewDisplay;
    private boolean fastOptionPreviewWrapper(String strOption, int iDisplay) {
        if (m_lastSetTimeResult != ErrorCode.NONE) {
            return false;
        }
        if (mFastPreviewInProgressCount >= MAX_FAST_PREVIEW_COUNT) {
            mPendingFastPreview = strOption;
            mPendingFastDisplay = iDisplay;
            return true;
        } else {
            int result = fastOptionPreview(strOption, iDisplay);
            if (result == 0)
                mFastPreviewInProgressCount++;
            return (result == 0);
        }
//        if( fastPreviewInProgress ) {
//            pendingFastPreviewOption = strOption;
//            pendingFastPreviewDisplay = iDisplay;
//        } else {
//            fastPreviewInProgress = true;
//            fastOptionPreview(strOption,iDisplay);
//            finishAllCommands(new OnCompletionListener() {
//                @Override
//                public void onComplete(ErrorCode resultCode) {
//                    fastPreviewInProgress = false;
//                    if( pendingFastPreviewOption!=null ) {
//                        fastOptionPreviewWrapper(pendingFastPreviewOption,pendingFastPreviewDisplay);
//                        pendingFastPreviewOption = null;
//                    }
//                }
//            });
//        }
//        return 0;
    }

    public int clearScreen() {
        return clearScreen(1);
    }

    public ErrorCode captureCurrentFrame(OnCaptureListener listener) {
        return capture(CAPTURE_CURRENT, listener);
    }

    private ResultTask<Rect> captureChangeTask;
    private SurfaceHolder.Callback captureSurfaceCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceCreated(SurfaceHolder holder) {

        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            if (captureChangeTask != null) {
                captureChangeTask.sendResult(new Rect(0, 0, width, height));
                captureChangeTask = null;
            }
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {

        }
    };

    private ResultTask<Rect> waitForCaptureDimensionsChanged() {
        if (captureChangeTask == null) {
            captureChangeTask = new ResultTask<>();
        }
        return captureChangeTask;
    }

    public ResultTask<Bitmap> captureFrame(final SurfaceView scratchView) {
        if (LL.D) Log.d(LOG_TAG, "captureFrame: in");
        final ResultTask<Bitmap> resultTask = new ResultTask<Bitmap>();

        mCaptureFrameInProgress = true;

        final int TIMEOUT_LENGTH = 500;

        scratchView.getHolder().addCallback(captureSurfaceCallback);
        scratchView.getHolder().setFixedSize(16, 16);
        waitForCaptureDimensionsChanged().setTimeout(TIMEOUT_LENGTH).onResultAvailable(new OnResultAvailableListener<Rect>() {
            @Override
            public void onResultAvailable(ResultTask<Rect> task, Event event, Rect result) {
                if (LL.D)
                    Log.d(LOG_TAG, "captureFrame: waitForCaptureDimensionsChanged[A]::onResultAvailable : " + result);
                NexEditorDeviceProfile.Size captureSize = NexEditorDeviceProfile.getDeviceProfile().getCaptureSize();
                scratchView.getHolder().setFixedSize(captureSize.width, captureSize.height);
                waitForCaptureDimensionsChanged().setTimeout(TIMEOUT_LENGTH).onResultAvailable(new OnResultAvailableListener<Rect>() {
                    @Override
                    public void onResultAvailable(ResultTask<Rect> task, Event event, Rect result) {
                        if (LL.D)
                            Log.d(LOG_TAG, "captureFrame: waitForCaptureDimensionsChanged[B]::onResultAvailable : " + result);
                        getFastOptionPreviewTask().setTimeout(TIMEOUT_LENGTH).onComplete(new Task.OnTaskEventListener() {
                            @Override
                            public void onTaskEvent(Task t, Event e) {
                                if (LL.D)
                                    Log.d(LOG_TAG, "captureFrame: getFastOptionPreviewTask::onComplete");
                                prepareSurface(scratchView.getHolder().getSurface());
                                captureCurrentFrame(new OnCaptureListener() {
                                    @Override
                                    public void onCapture(Bitmap bm) {
                                        if (LL.D)
                                            Log.d(LOG_TAG, "captureFrame: onCapture -> " + bm.getWidth() + "x" + bm.getHeight());
                                        scratchView.getHolder().removeCallback(captureSurfaceCallback);
                                        scratchView.getHolder().setSizeFromLayout();
                                        prepareSurface(viewSurface);
                                        mCaptureFrameInProgress = false;
                                        resultTask.sendResult(bm);
                                    }

                                    @Override
                                    public void onCaptureFail(ErrorCode error) {
                                        if (LL.D)
                                            Log.d(LOG_TAG, "captureFrame: onCaptureFail : " + error.getMessage());
                                        scratchView.getHolder().removeCallback(captureSurfaceCallback);
                                        scratchView.getHolder().setSizeFromLayout();
                                        prepareSurface(viewSurface);
                                        mCaptureFrameInProgress = false;
                                        resultTask.sendFailure(error);
                                    }
                                });
                            }
                        }).onFailure(new OnFailListener() {
                            @Override
                            public void onFail(Task t, Event e, TaskError failureReason) {
                                if (LL.I)
                                    Log.d(LOG_TAG, "captureFrame: getFastOptionPreviewTask::onCFail : " + failureReason.getMessage());
                                scratchView.getHolder().removeCallback(captureSurfaceCallback);
                                scratchView.getHolder().setSizeFromLayout();
                                prepareSurface(viewSurface);
                                mCaptureFrameInProgress = false;
                                resultTask.sendFailure(failureReason);
                            }
                        });
                        fastPreview(FastPreviewOption.nofx, 1, false);
                    }
                }).onFailure(new OnFailListener() {
                    @Override
                    public void onFail(Task t, Event e, TaskError failureReason) {
                        if (LL.I)
                            Log.d(LOG_TAG, "captureFrame: waitForCaptureDimensionsChanged[B]::onCFail : " + failureReason.getMessage());
                        scratchView.getHolder().removeCallback(captureSurfaceCallback);
                        scratchView.getHolder().setSizeFromLayout();
                        prepareSurface(viewSurface);
                        mCaptureFrameInProgress = false;
                        resultTask.sendFailure(failureReason);
                    }
                });
            }
        }).onFailure(new OnFailListener() {
            @Override
            public void onFail(Task t, Event e, TaskError failureReason) {
                if (LL.I)
                    Log.d(LOG_TAG, "captureFrame: waitForCaptureDimensionsChanged[A]::onCFail : " + failureReason.getMessage());
                scratchView.getHolder().removeCallback(captureSurfaceCallback);
                scratchView.getHolder().setSizeFromLayout();
                prepareSurface(viewSurface);
                mCaptureFrameInProgress = false;
                resultTask.sendFailure(failureReason);
            }
        });
        if (LL.D) Log.d(LOG_TAG, "captureFrame: out");

        return resultTask;
    }

    private int captureCurrentFrameInternal(OnCaptureListener listener) {
        int result = captureCurrentFrame();
        if (result == 0) // Success
        {
            m_onCaptureListeners.add(listener);
        }
        return result;
    }

    private boolean mCaptureFrameInProgress = false;
    private boolean mCaptureInProgress = false;
    private boolean mCaptureCompleting = false;
    private int mCaptureOriginalTime;

    public static final int CAPTURE_CURRENT = 0x7FFF0001;
    public static final int CAPTURE_CURRENT_NOFX = 0x7FFF0002;

    public boolean isCapture() {

        return mCaptureInProgress;
    }

    public ErrorCode capture(int captureTime, final OnCaptureListener listener) {

        if (captureTime == CAPTURE_CURRENT) {
            if (LL.D)
                Log.d(LOG_TAG, "Capture request : captureTime=CAPTURE_CURRENT t=" + m_currentTime + " " + (mCaptureInProgress ? "in-progress " : "ok ") + (mCaptureCompleting ? "completing " : "new "));
            captureTime = m_currentTime; //(m_isPendingSeek||m_isSeeking)?m_pendingSeekLocation:m_currentTime;
        } else if (captureTime == CAPTURE_CURRENT_NOFX) {
            if (LL.D)
                Log.d(LOG_TAG, "Capture request : captureTime=CAPTURE_CURRENT_NOFX " + (mCaptureInProgress ? "in-progress " : "ok ") + (mCaptureCompleting ? "completing " : "new "));
        } else {
            if (LL.D)
                Log.d(LOG_TAG, "Capture request : captureTime=" + captureTime + " " + (mCaptureInProgress ? "in-progress " : "ok ") + (mCaptureCompleting ? "completing " : "new "));
        }

        final int iTime = captureTime;

        if (mCaptureInProgress) {
            return ErrorCode.INVALID_STATE;
        }

        mCaptureInProgress = true;

        if (!mCaptureCompleting)
            mCaptureOriginalTime = (m_isPendingSeek || m_isSeeking) ? m_pendingSeekLocation : m_currentTime;
        if (LL.D) Log.d(LOG_TAG, "  Capture mCaptureOriginalTime=" + mCaptureOriginalTime);

        final OnCaptureListener onCapture = new OnCaptureListener() {

            void onCapture(ErrorCode error, Bitmap bm) {
                mCaptureCompleting = true;
                if (bm == null) {
                    if (LL.E)
                        Log.e(LOG_TAG, "  Capture failed; error=" + error.getValue() + " (" + error.name() + ")");
                    mCaptureInProgress = false;
                    listener.onCaptureFail(error);
                } else {
                    if (LL.D) Log.d(LOG_TAG, "  Capture success; notify listener");
                    mCaptureInProgress = false;
                    listener.onCapture(bm);
                }
                mCaptureCompleting = false;
                if (!mCaptureInProgress) {
                    mCaptureInProgress = true;
                    if (LL.D) Log.d(LOG_TAG, "  Capture done; seek to original location");
                    seek(mCaptureOriginalTime, false, new OnSetTimeDoneListener() {
                        @Override
                        public void onSetTimeFail(ErrorCode returnCode) {
                            mCaptureInProgress = false;
                        }

                        @Override
                        public void onSetTimeDone(int time, int iFrameTime) {
                            mCaptureInProgress = false;
                        }

                        @Override
                        public String getSetTimeLabel() {
                            return "seekToOriginalTimeAfterCapture";
                        }
                    });
                } else {
                    if (LL.D) Log.d(LOG_TAG, "  Capture done; SKIP SEEK");
                }
            }

            @Override
            public void onCaptureFail(ErrorCode error) {
                if (LL.D) Log.d(LOG_TAG, "  Capture onCaptureFail=" + error.name());
                onCapture(error, null);
            }

            @Override
            public void onCapture(Bitmap bm) {
                if (LL.D) Log.d(LOG_TAG, "  Capture onCapture=" + bm);
                onCapture(ErrorCode.NONE, bm);
            }

        };

        if (iTime == CAPTURE_CURRENT_NOFX) {
            if (LL.D) Log.d(LOG_TAG, "  Capture CAPTURE_CURRENT_NOFX fastPreview");
            fastPreviewForCapture(FastPreviewOption.nofx, 1, false);
            if (LL.D) Log.d(LOG_TAG, "  Capture CAPTURE_CURRENT_NOFX captureCurrentFrame");
            captureCurrentFrameInternal(onCapture);
            if (LL.D) Log.d(LOG_TAG, "  Capture CAPTURE_CURRENT_NOFX out");
        } else {
            seek(iTime, false, new OnSetTimeDoneListener() {

                @Override
                public String getSetTimeLabel() {
                    return "CAPTURE(" + iTime + ")";
                }

                @Override
                public void onSetTimeFail(ErrorCode returnCode) {
                    if (LL.D) Log.d(LOG_TAG, "  Capture onSetTimeFail=" + returnCode.name());
                    mCaptureInProgress = false;
                    listener.onCaptureFail(returnCode);
                }

                @Override
                public void onSetTimeDone(int time, int iFrameTime) {

                    if (LL.D) Log.d(LOG_TAG, "  Capture onSetTimeDone=" + time);
                    captureCurrentFrameInternal(onCapture);
                }

            });
        }

        return ErrorCode.NONE;
    }


    public void stop() {
        stop(new OnCompletionListener() {
            @Override
            public void onComplete(ErrorCode resultCode) {
            }
        });
    }

    public void startPlay(OnPlayListener completionListener) {
        startPlay(0, completionListener);
    }

    public void startPlayMuted(OnPlayListener completionListener) {
        startPlay(1, completionListener);
    }

    public void startPlay() {
        startPlay(0, new OnPlayListener() {
            @Override
            public void onPlayRequestProcessed(ErrorCode resultCode) {
            }

            @Override
            public void onPlayStarted() {
            }
        });
    }

    public void startPlayMuted() {
        startPlay(1, new OnPlayListener() {
            @Override
            public void onPlayRequestProcessed(ErrorCode resultCode) {
            }

            @Override
            public void onPlayStarted() {
            }
        });
    }

    private int m_playRequestSerial = 0;
    private boolean m_playRequestPending = false;
    private boolean m_playRequested = false;

    private void startPlay(final int iMuteAudio, final OnPlayListener playListener) {
        m_playRequestSerial++;
        m_playRequestPending = true;
        m_didSetTimeSinceLastPlay = false;
        final int playRequestSerial = m_playRequestSerial;
        setTimeCancel(new OnCompletionListener() {
            @Override
            public void onComplete(ErrorCode resultCode) {
                if (!m_playRequestPending)
                    return;
                if (m_playRequestSerial != playRequestSerial)
                    return;
                m_playRequestPending = false;
                m_playRequested = true;
                int result = startPlay(iMuteAudio);
                if (result == 0) {
                    m_onPlayListeners.add(playListener);
                } else {
                    playListener.onPlayRequestProcessed(ErrorCode.fromValue(result));
                }
            }
        });
    }

    private String mExportPath;

    /**
     * @param strFilePath
     * @param iWidth
     * @param iHeight
     * @param iBitrate
     * @param lMaxFileSize    Maximum file size in bytes
     * @param projectDuration Maximum duration (0 for unlimited)
     * @return
     */
    public Task export(String strFilePath, int iWidth, int iHeight, int iBitrate, long lMaxFileSize, int projectDuration, boolean forceDurationLimit, int frameRate, int profile, int level, int samplingRate, int iFlag) {
        Task task = new Task();
        if (m_exportingTask != null) {
            task.sendFailure(ErrorCode.ALREADY_EXPORTING);
            return task;
        }
        m_exportProjectDuration = projectDuration;
        int result = encodeProject(strFilePath, iWidth, iHeight, iBitrate, lMaxFileSize, forceDurationLimit ? projectDuration : 0, frameRate <= 0 ? 3000 : frameRate, 1920, 1080, samplingRate, DEFAULT_EXPORT_AUDIO_BITRATE, profile, level, DEFAULT_EXPORT_CODEC, iFlag);
        if (result != 0) {
            ErrorCode errorCode = ErrorCode.fromValue(result);
            task.sendFailure(errorCode);
            return task;
        }
        mExportPath = null;
        m_exportingTask = task;
        return m_exportingTask;
    }

    public Task export(String strFilePath, int iWidth, int iHeight, int iBitrate, long lMaxFileSize, int projectDuration, boolean forceDurationLimit, int iFlag) {
        Task task = new Task();
        if (m_exportingTask != null) {
            task.sendFailure(ErrorCode.ALREADY_EXPORTING);
            return task;
        }
        m_exportProjectDuration = projectDuration;
        int result = encodeProject(strFilePath, iWidth, iHeight, iBitrate, lMaxFileSize, forceDurationLimit ? projectDuration : 0, 3000, 1920, 1080, 44100, DEFAULT_EXPORT_AUDIO_BITRATE, 0, 0, DEFAULT_EXPORT_CODEC, iFlag);
        if (result != 0) {
            ErrorCode errorCode = ErrorCode.fromValue(result);
            task.sendFailure(errorCode);
            return task;
        }
        mExportPath = null;
        m_exportingTask = task;
        return m_exportingTask;
    }

    public Task export(FileOutputStream fileOutputStream, int iWidth, int iHeight, int iBitrate, long lMaxFileSize, int projectDuration, boolean forceDurationLimit, int samplingRate, int iFlag) {
        Task task = new Task();
        if (m_exportingTask != null) {
            task.sendFailure(ErrorCode.ALREADY_EXPORTING);
            return task;
        }
        m_exportProjectDuration = projectDuration;
        int result = 0;
        try {
            mExportPath = setOutPutPath(fileOutputStream.getFD());
            Log.d(LOG_TAG, "newPath=" + mExportPath);
            result = encodeProject(mExportPath, iWidth, iHeight, iBitrate, lMaxFileSize, forceDurationLimit ? projectDuration : 0, 2900, 1920, 1080, samplingRate, DEFAULT_EXPORT_AUDIO_BITRATE, 0, 0, DEFAULT_EXPORT_CODEC, iFlag);
        } catch (IOException e) {
            e.printStackTrace();
            result = ErrorCode.CREATE_FAILED.getValue();
        }

        if (result != 0) {
            ErrorCode errorCode = ErrorCode.fromValue(result);
            task.sendFailure(errorCode);
            closeOutPutPath();
            return task;
        }
        m_exportingTask = task;
        return m_exportingTask;
    }

    public Task export(FileOutputStream fileOutputStream, int iWidth, int iHeight, int iBitrate, long lMaxFileSize, int projectDuration, boolean forceDurationLimit, int iFlag) {
        Task task = new Task();
        if (m_exportingTask != null) {
            task.sendFailure(ErrorCode.ALREADY_EXPORTING);
            return task;
        }
        m_exportProjectDuration = projectDuration;
        int result = 0;
        try {
            mExportPath = setOutPutPath(fileOutputStream.getFD());
            Log.d(LOG_TAG, "newPath=" + mExportPath);
            result = encodeProject(mExportPath, iWidth, iHeight, iBitrate, lMaxFileSize, forceDurationLimit ? projectDuration : 0, 2900, 1920, 1080, 44100, DEFAULT_EXPORT_AUDIO_BITRATE, 0, 0, DEFAULT_EXPORT_CODEC, iFlag);
        } catch (IOException e) {
            e.printStackTrace();
            result = ErrorCode.CREATE_FAILED.getValue();
        }

        if (result != 0) {
            ErrorCode errorCode = ErrorCode.fromValue(result);
            task.sendFailure(errorCode);
            closeOutPutPath();
            return task;
        }
        m_exportingTask = task;
        return m_exportingTask;
    }


    public Task export(String strFilePath, int iWidth, int iHeight, int iBitrate, long lMaxFileSize, int projectDuration, boolean forceDurationLimit, int samplingRate, int fps, int iVideoCodecType, int iFlag) {
        Task task = new Task();
        if (m_exportingTask != null) {
            task.sendFailure(ErrorCode.ALREADY_EXPORTING);
            return task;
        }
        m_exportProjectDuration = projectDuration;
        int result = encodeProject(strFilePath, iWidth, iHeight, iBitrate, lMaxFileSize, forceDurationLimit ? projectDuration : 0, fps, 1920, 1080, samplingRate, DEFAULT_EXPORT_AUDIO_BITRATE, 0, 0, iVideoCodecType, iFlag);
        if (result != 0) {
            ErrorCode errorCode = ErrorCode.fromValue(result);
            task.sendFailure(errorCode);
            return task;
        }
        mExportPath = null;
        m_exportingTask = task;
        return m_exportingTask;
    }

    public Task export(String strFilePath, int iWidth, int iHeight, int iBitrate, long lMaxFileSize, int projectDuration, boolean forceDurationLimit, int samplingRate, int iEncodeProfile, int iEncodeLevel, int fps, int iVideoCodecType, int iFlag) {
        Task task = new Task();
        if (m_exportingTask != null) {
            task.sendFailure(ErrorCode.ALREADY_EXPORTING);
            return task;
        }
        m_exportProjectDuration = projectDuration;
        int result = encodeProject(strFilePath, iWidth, iHeight, iBitrate, lMaxFileSize, forceDurationLimit ? projectDuration : 0, fps, 1920, 1080, samplingRate, DEFAULT_EXPORT_AUDIO_BITRATE, iEncodeProfile, iEncodeLevel, iVideoCodecType, iFlag);
        if (result != 0) {
            ErrorCode errorCode = ErrorCode.fromValue(result);
            task.sendFailure(errorCode);
            return task;
        }
        mExportPath = null;
        m_exportingTask = task;
        return m_exportingTask;
    }

    //    private boolean m_isExporting = false;
    private Task m_exportingTask = null;
    private int m_exportProjectDuration;

    public void stop(OnCompletionListener completionListener) {
        stop(2, completionListener);
    }

    public void stop(int flag, OnCompletionListener completionListener) {
        if (m_playRequestPending) {
            for (OnCompletionListener listener : m_onSetTimeCancelListeners) {
                listener.onComplete(ErrorCode.PLAY_SUPERCEEDED);
            }
            m_onSetTimeCancelListeners.clear();
        }
        m_playRequestPending = false;
        m_playRequested = false;
        if (!m_isPlaying && m_exportingTask == null) {
            stopPlay(flag);    // flag only for SDK
            completionListener.onComplete(ErrorCode.NONE);
            return;
        }
        if (LL.D) Log.d(LOG_TAG, "stop flags(" + flag + ") : " + completionListener.toString());
//    	for( OnPlayListener listener: m_onPlayStartListeners ) {
//    		listener.onPlayRequestProcessed(ErrorCode.PLAY_SUPERCEEDED);
//    	}
        m_onPlayStartListeners.clear();

        m_onStopListeners.add(completionListener);
        int result = stopPlay(flag);   // flag only for SDK

        if (m_exportingTask != null) {
            m_exportingTask.signalEvent(Event.CANCEL);
            m_exportingTask = null;
        }

        if (result != 0) {
            m_onStopListeners.remove(completionListener);
            completionListener.onComplete(ErrorCode.fromValue(result));
        }
    }

    public void clearPendingEvents() {
        clearPendingEvents(new OnCompletionListener() {
            @Override
            public void onComplete(ErrorCode resultCode) {
            }
        });
    }

    private ImageReader nxcapture = null;
    @RequiresApi(Build.VERSION_CODES.KITKAT)
    public ErrorCode exportJpeg(int iWidth, int iHeight, int iFlag, final OnCaptureListener listener) {

        if (mCaptureInProgress) {
            return ErrorCode.INVALID_STATE;
        }

        mCaptureInProgress = true;

        if (!mCaptureCompleting)
            mCaptureOriginalTime = (m_isPendingSeek || m_isSeeking) ? m_pendingSeekLocation : m_currentTime;
        if (LL.D) Log.d(LOG_TAG, "image Capture mCaptureOriginalTime=" + mCaptureOriginalTime);


        final OnCaptureListener onCapture = new OnCaptureListener() {

            void onCapture(ErrorCode error, Bitmap bm) {

                if (nxcapture != null) {
                    nxcapture.close();
                    nxcapture = null;
                }

                mCaptureCompleting = true;
                if (bm == null) {
                    if (LL.E)
                        Log.e(LOG_TAG, "image Capture failed; error=" + error.getValue() + " (" + error.name() + ")");
                    mCaptureInProgress = false;
                    listener.onCaptureFail(error);
                } else {
                    if (LL.D) Log.d(LOG_TAG, "image Capture success; notify listener");
                    mCaptureInProgress = false;
                    listener.onCapture(bm);
                }
                mCaptureCompleting = false;
//                if( !mCaptureInProgress ) {
//                    mCaptureInProgress = true;
//                    if(LL.D) Log.d(LOG_TAG,"  Capture done; seek to original location" );
//                    seek( mCaptureOriginalTime, false, new OnSetTimeDoneListener() {
//                        @Override
//                        public void onSetTimeFail(ErrorCode returnCode) {
//                            mCaptureInProgress = false;
//                        }
//
//                        @Override
//                        public void onSetTimeDone(int time, int iFrameTime) {
//                            mCaptureInProgress = false;
//                        }
//
//                        @Override
//                        public String getSetTimeLabel() {
//                            return "seekToOriginalTimeAfterCapture";
//                        }
//                    });
//                } else {
//                    if(LL.D) Log.d(LOG_TAG,"  Capture done; SKIP SEEK" );
//                }
            }

            @Override
            public void onCaptureFail(ErrorCode error) {
                if (LL.D) Log.d(LOG_TAG, "image Capture onCaptureFail=" + error.name());
                prepareSurface(viewSurface);
                if (nxcapture != null) {
                    nxcapture.close();
                    nxcapture = null;
                }
                onCapture(error, null);
            }

            @Override
            public void onCapture(Bitmap bm) {
                if (LL.D) Log.d(LOG_TAG, "image Capture onCapture=" + bm);
                onCapture(ErrorCode.NONE, bm);
            }

        };

        if (nxcapture == null) {
            Log.d("NexEditor", "image Capture create ImageReader");
            nxcapture = ImageReader.newInstance(iWidth, iHeight, PixelFormat.RGBA_8888, 2);
        }

        nxcapture.setOnImageAvailableListener(new ImageReader.OnImageAvailableListener() {
            @Override
            public void onImageAvailable(ImageReader reader) {

                Log.d("NexEditor", "image Capture setOnImageAvailableListener jpeg == ");
                Image img = null;
                try {
                    img = reader.acquireLatestImage();
                    if (img != null) {
                        Image.Plane[] planes = img.getPlanes();
                        if (planes[0].getBuffer() == null) {
                            onCaptureFail(ErrorCode.UNKNOWN);
                            return;
                        }
                        int width = img.getWidth();
                        int height = img.getHeight();
                        int rowStride = planes[0].getRowStride();
                        int padding = rowStride - width * 4;
                        byte[] pixeldata = new byte[width * height * 4];
                        ByteBuffer buffer = planes[0].getBuffer();
                        for (int i = 0; i < height; ++i) {

                            buffer.get(pixeldata, width * i * 4, width * 4);
                            buffer.position(rowStride * (i + 1));
                        }
                        Log.d("NexEditor", "image Capture prepareSurface = " + viewSurface);
                        prepareSurface(viewSurface);
                        onCaptureDone(width, height, pixeldata.length, pixeldata, false);
                        img.close();
                        img = null;
                    }

                } catch (Exception e) {

                    e.printStackTrace();
                    Log.d("NexEditor", "image Capture prepareSurface(exception) = " + viewSurface);
                    prepareSurface(viewSurface);
                    onCaptureFail(ErrorCode.UNKNOWN);
                } finally {

                    if (null != img) {
                        img.close();
                    }
                    reader.close();
                    nxcapture = null;
                }
            }
        }, null);

        Surface surface = nxcapture.getSurface();
        // prepareSurface(surface);
        int result = encodeProjectJpeg(surface, " ", iWidth, iHeight, 0, iFlag);
        if (result == 0) // Success
        {
            m_onCaptureListeners.add(onCapture);
        }

        return ErrorCode.NONE;
    }


    private static final ExecutorService sCaptuerThreadExcutor = Executors.newSingleThreadExecutor();
    private BlockingQueue<Integer> exportImageQueue = new ArrayBlockingQueue<Integer>(3);
    private boolean exportImageCanceled = false;

    /*
		1 - captuer success.
		0 - TimeOut;
		-1 - USER cancel

	*/
    public static abstract class externalExportImageCallback {
        final static public int kFormat_RGBA = 0;

        public abstract boolean OnCaptuerData(byte[] data, int format);

        public abstract void OnEos();

        public abstract boolean OnEnd(int error);

        public abstract void OnProgress(int precent);

        public abstract int waitCount();
    }

    ;

    private externalExportImageCallback singleToneExportImageCallback = null;

    public void setExternalExportImageCallback(externalExportImageCallback callback) {
        singleToneExportImageCallback = callback;
    }

    public void exportImageFormatCancel() {
        if (mCaptureInProgress) {
            if (!exportImageCanceled) {
                exportImageCanceled = true;
                if (LL.D) Log.d(LOG_TAG, "  exportImageFormatCancel...");
            }
        }
    }

    private Object exportImageFormatLock = new Object();

    public ErrorCode exportImageFormat(final int iWidth, final int iHeight, final int intervalTime, final int startTime, final int endTime, externalExportImageCallback callback) {

        synchronized (exportImageFormatLock) {
            if (mCaptureInProgress) {
                Log.d(LOG_TAG, "exportImageFormat already exporting");
                return ErrorCode.INVALID_STATE;
            }
            mCaptureInProgress = true;
        }

        if (callback == null) {
            Log.d(LOG_TAG, "exportImageFormat ExportImageCollback is null ");
            mCaptureInProgress = false;
            return ErrorCode.INVALID_STATE;
        }
        singleToneExportImageCallback = callback;

//        ErrorCode err = ErrorCode.fromValue(setGIFMode(1));
//        if( err != ErrorCode.NONE )
//        {
//            mCaptureInProgress = false;
//            Log.d(LOG_TAG, "exportImageFormat setGIFMode failed width " + err.toString());
//            return err;
//        }

        if (!mCaptureCompleting)
            mCaptureOriginalTime = (m_isPendingSeek || m_isSeeking) ? m_pendingSeekLocation : m_currentTime;
        if (LL.D)
            Log.d(LOG_TAG, "  exportImageFormat mCaptureOriginalTime=" + mCaptureOriginalTime);

        exportImageQueue.poll();
        exportImageQueue.poll();
        exportImageQueue.poll();

        if (nxcapture == null)
            nxcapture = ImageReader.newInstance(iWidth, iHeight, PixelFormat.RGBA_8888, 2);
//		Surface surface = nxcapture.getSurface();
//		prepareSurface(surface);
        HandlerThread handlerThread = new HandlerThread("exportImageFormat");
        handlerThread.start();
        final Handler handler = new Handler(handlerThread.getLooper());

        Log.d(LOG_TAG, "  exportImageFormat 1");

        nxcapture.setOnImageAvailableListener(new ImageReader.OnImageAvailableListener() {
            int frameCount = 0;

            @Override
            public void onImageAvailable(ImageReader reader) {

                Image img = null;
                int retMessage = 1;

                try {
                    img = reader.acquireLatestImage();
                    if (img != null) {

                        Image.Plane[] planes = img.getPlanes();
                        if (planes[0].getBuffer() != null) {
                            //onCaptureFail(ErrorCode.UNKNOWN);


                            int width = img.getWidth();
                            int height = img.getHeight();
                            int rowStride = planes[0].getRowStride();
                            int padding = rowStride - width * 4;
                            byte[] pixeldata = new byte[width * height * 4];
                            ByteBuffer buffer = planes[0].getBuffer();
                            for (int i = 0; i < height; ++i) {

                                buffer.get(pixeldata, width * i * 4, width * 4);
                                buffer.position(rowStride * (i + 1));
                            }

                            boolean result = false;

                            if (!exportImageCanceled)
                                result = singleToneExportImageCallback.OnCaptuerData(pixeldata, externalExportImageCallback.kFormat_RGBA);

                            Log.d("exportImageFormat", "onImageAvailable(" + img.getTimestamp() + ") width=" + width + ", height=" + height + ", rowStride=" + rowStride + ", padding=" + padding + ", format=" + img.getFormat());
                            img.close();
                            img = null;

                            if (!result) {
                                retMessage = -2;
                            }
                        } else {
                            retMessage = -3;
                        }
                    } else {
                        retMessage = -4;
                        Log.d("exportImageFormat", "Latest image is null. Ignore it.");
                        return;
                    }

                } catch (Exception e) {
                    Log.d("exportImageFormat", "Exception");
                    e.printStackTrace();
                    retMessage = -5;
                }

                if (exportImageCanceled) {
                    retMessage = -1;
                }

                try {
                    exportImageQueue.put(retMessage);
                } catch (InterruptedException ie) {
                    ie.printStackTrace();
                }
            }
        }, handler);

        Log.d(LOG_TAG, "  exportImageFormat 2");

        new AsyncTask<Void, Integer, Integer>() {
            int lastError = 0;
            int lastProgress = 0;

            private int waitSeekDone() {
                boolean waitOk = false;
                for (int i = 0; i < 100; i++) {
                    if (!m_isSeeking) {
                        waitOk = true;
                        break;
                    }
                    try {
                        Thread.sleep(10, 0);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                if (!waitOk && m_isSeeking) {
                    Log.d("exportImageFormat", "wait waitSeekDone() fail!");
                    return -6;

                }
                return 1;
            }

            @Override
            protected Integer doInBackground(Void... params) {
                //TODO:
                int val = 0;
                int curTime = startTime;

                Log.d(LOG_TAG, "  exportImageFormat doInBackground");
                singleToneExportImageCallback.OnProgress(0);
                //publishProgress(0);
                val = waitSeekDone();
                if (val != 1) {
                    return val;
                }

                ErrorCode err = ErrorCode.fromValue(setGIFMode(1));
                if (err != ErrorCode.NONE) {
                    mCaptureInProgress = false;
                    Log.d(LOG_TAG, "exportImageFormat setGIFMode failed width " + err.toString());
                    return -7;
                }

                int lastWaitCount = singleToneExportImageCallback.waitCount();
                int timeout = 0;
                while (true) {
                    int count = singleToneExportImageCallback.waitCount();
                    Log.d("exportImageFormat", "waitCount=" + count);
                    if (count == 0) {
                        break;
                    }
                    try {
                        Thread.sleep(500, 0);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    if (lastWaitCount != count) {
                        timeout = 0;
                        lastWaitCount = count;
                        lastProgress++;
                        singleToneExportImageCallback.OnProgress(lastProgress);
                    } else {
                        timeout++;
                        if (timeout > 10) {
                            Log.d("exportImageFormat", "waitCount Timeout=" + lastWaitCount);
                            return -6;
                        }
                    }
                }
                int startPercent = lastProgress;
                int remainPercent = 100 - lastProgress;
                if (remainPercent <= 0) {
                    startPercent = 0;
                    remainPercent = 100;
                }
                Surface surface = nxcapture.getSurface();
                prepareSurface(surface);

                for (int time = startTime; time < endTime; time += intervalTime) {
                    Log.d("exportImageFormat", "seek start=" + time);
                    seek(time);

                    try {
                        Integer q = exportImageQueue.poll(10000, TimeUnit.MILLISECONDS);
                        if (q == null) {
                            val = 0;
                        } else {
                            val = q.intValue();
                        }
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    if (val <= 0) {
                        Log.d("exportImageFormat", "val=" + val);
                        break;
                    }
                    val = waitSeekDone();
                    if (val != 1) {
                        Log.d("exportImageFormat", "wait val=" + val);
                        break;
                    }

                    //int result = encodeProjectJpeg(" ", iWidth, iHeight, 0, 0);
                    int percent = startPercent + ((time - startTime) * remainPercent / (endTime - startTime));
                    if (lastProgress != percent) {
                        lastProgress = percent;
                        //publishProgress(lastProgress);
                        singleToneExportImageCallback.OnProgress(lastProgress);
                    }
                }

                if (nxcapture != null) {
                    nxcapture.setOnImageAvailableListener(null, null);
                }

                singleToneExportImageCallback.OnEos();
                if (lastProgress != 100) {
                    singleToneExportImageCallback.OnProgress(100);
                    //publishProgress(100);
                }
                return val;
            }

            @Override
            protected void onPostExecute(Integer aVoid) {

                Log.d(LOG_TAG, "  exportImageFormat onPostExecute");
                //super.onPostExecute(aVoid);
                int error = aVoid.intValue();
                lastError = error - 1; // 0 : ok , -1 : timeout , -2 : cancel , -3 : OnCaptuerData fail! , -4 ~ -6 : surface fail!, -7 GIF Mode fail!

                setGIFMode(0);

                prepareSurface(viewSurface);
                if (nxcapture != null) {

                    nxcapture.close();
                    nxcapture = null;
                }
                mCaptureInProgress = false;
                exportImageCanceled = false;
                singleToneExportImageCallback.OnEnd(lastError);

//				seek(mCaptureOriginalTime, false, new OnSetTimeDoneListener() {
//					@Override
//					public void onSetTimeFail(ErrorCode returnCode) {
//						mCaptureInProgress = false;
//						exportImageCanceled = false;
//						singleToneExportImageCallback.OnEnd(lastError);
//					}
//
//					@Override
//					public void onSetTimeDone(int time, int iFrameTime) {
//						mCaptureInProgress = false;
//						exportImageCanceled = false;
//						singleToneExportImageCallback.OnEnd(lastError);
//					}
//
//					@Override
//					public String getSetTimeLabel() {
//						return "seekToExportImageFormatEnd";
//					}
//				});
            }
        }.executeOnExecutor(sCaptuerThreadExcutor);

        Log.d(LOG_TAG, "  exportImageFormat 3");
        return ErrorCode.NONE;
    }

    // ------- SEEKING LOGIC -------------------------------------------------------------------------------

    private boolean m_isSeeking = false;
    private boolean m_isPendingSeek = false;
    private int m_pendingSeekLocation = 0;
    private int m_pendingSeekIDR = 0;
    private boolean m_isPendingNonDisplaySeek = false;
    private int m_pendingNonDisplaySeekLocation = 0;
    private int m_seekSerial = 0;                // The serial number of the NEXT seek request

    public void seek(int time) {
        seek(time, null);
    }

//    public void setTimeCancel() {
//    	setTimeCancel(null);
//    }

    private void setTimeCancel(OnCompletionListener listener) {
        if (LL.D) Log.d(LOG_TAG, "setTimeCancel m_isPendingSeek: " + m_isPendingSeek + " -> false");
        m_isPendingSeek = false;
        m_isPendingNonDisplaySeek = false;

        if (!m_isSeeking) {
            listener.onComplete(ErrorCode.NONE);
            return;
        }

        if (listener != null) {
            m_onSetTimeCancelListeners.add(listener);
        }
    }

    public void startSeekCollectCache(final int time, final OnSetTimeDoneListener listener) {
        setTimeCancel(new OnCompletionListener() {
            @Override
            public void onComplete(ErrorCode resultCode) {
                seekFromCache(time, listener);
            }
        });
    }

    public void seekFromCache(int time, OnSetTimeDoneListener listener) {
        seek_internal(time, true, 4, listener);
    }

    public void seek(int time, OnSetTimeDoneListener listener) {
        seek_internal(time, true, 0, listener);
    }

    public void seekIOnly(int time, OnSetTimeDoneListener listener) {
        seek_internal(time, true, 1, listener);
    }

    public void seekIDROnly(int time, OnSetTimeDoneListener listener) {
        seek_internal(time, true, 2, listener);
    }

    public void seek(int time, boolean display, OnSetTimeDoneListener listener) {
        seek_internal(time, display, 0, listener);
    }

    private void seek_internal(int time, boolean display, int iFlag, OnSetTimeDoneListener listener) {

        if (LL.D)
            Log.d(LOG_TAG, "in seek(display=" + display + "," + time + "," + iFlag + "," + (listener == null ? "null" : listener) + ") m_seekSerial=" + m_seekSerial + " seeking=" + m_isSeeking + "; pendingSeek=" + m_isPendingSeek + "; pendingSeekLocation=" + m_pendingSeekLocation);

        if (listener != null) {
            listener.m_reqTime = time;
            listener.m_serialNumber = m_seekSerial;
            if (DEBUG) {
                listener.m_stackTrace = Thread.currentThread().getStackTrace();
            }
        }

        if (m_isSeeking) {

            if (display) {
                if (LL.D)
                    Log.d(LOG_TAG, "seek_internal: m_isPendingSeek=" + m_isPendingSeek + " -> true, flag=" + iFlag);
                m_isPendingSeek = true;
                m_pendingSeekLocation = time;
                m_pendingSeekIDR = iFlag;
            } else {
                m_isPendingNonDisplaySeek = true;
                m_pendingNonDisplaySeekLocation = time;
            }

        } else {

            m_didSetTimeSinceLastPlay = true;
            setIsSeekingFlag(true);
            int result = setTime(time, display ? 1 : 0, iFlag); // IDRFrame param( Only display idrfreme if idrframe is 1)
            if (result != 0) {
                if (LL.D) Log.d(LOG_TAG, "setTime ERROR RETURN: " + result);
                setIsSeekingFlag(false);
                if (listener != null) {
                    listener.onSetTimeFail(ErrorCode.fromValue(result));
                }
                return;
            } else {
                if (LL.D) Log.d(LOG_TAG, "setTime SEEK STARTED: " + result);
            }

            //setIsSeekingFlag(true);
            m_pendingSeekLocation = time;
            if (display) {
                if (LL.D)
                    Log.d(LOG_TAG, "seek_internal[display]: m_isPendingSeek=" + m_isPendingSeek + " -> false");
                m_isPendingSeek = false;
            } else {
                m_isPendingNonDisplaySeek = false;
            }
            m_seekSerial++;
        }

        if (listener != null) {
            m_setTimeDoneListeners.add(listener);
            if (LL.D) Log.d(LOG_TAG, "m_setTimeDoneListeners - Added listener");
            dumpSetTimeDoneLabels();
        }

        // if(LL.D) Log.d(LOG_TAG,"out seek(" + time + ")  m_setTimeDoneListeners.size()=" + m_setTimeDoneListeners.size() + setTimeListenerDump());

    }

    private void dumpSetTimeDoneLabels() {
        int i = 0;
        for (OnSetTimeDoneListener l : m_setTimeDoneListeners) {
            if (LL.D)
                Log.d(LOG_TAG, "     " + i + ": t=" + l.m_reqTime + " sn=" + l.m_serialNumber + " " + l.getSetTimeLabel() + "    " + l);
            i++;
        }
    }


    protected void onSetTimeDone(int iCurrentTime, int iFrameTime) {
        onSetTimeDone(iCurrentTime, iFrameTime, ErrorCode.NONE);
    }

    protected void onTimeChange(int iCurrentTime) {
        if (m_exportingTask != null) {
            m_exportingTask.setProgress(Math.min(m_exportProjectDuration, iCurrentTime), m_exportProjectDuration);
        }
        m_currentTime = iCurrentTime;
    }

    protected void onSetTimeDone(int iCurrentTime, int iFrameTime, ErrorCode errorCode) {

        m_lastSetTimeResult = errorCode;

        m_currentTime = iCurrentTime;
        m_frameTime = iFrameTime;
        if (LL.D)
            Log.d(LOG_TAG, "in onSetTimeDone(" + iCurrentTime + "," + iFrameTime + ") " + seekStateDump() + " " + setTimeListenerDump());

        // Listeners with a serial number equal to m_seekSerial are
        // for a pending seek operation, but any with a serial number
        // less than that are for the seek that just completed, so
        // we can notify and remove them.
        Deque<OnSetTimeDoneListener> toNotify = new LinkedList<OnSetTimeDoneListener>();
        for (OnSetTimeDoneListener listener : m_setTimeDoneListeners) {
            if (listener.m_serialNumber < m_seekSerial) {
                toNotify.add(listener);
            }
        }

        for (OnSetTimeDoneListener listener : toNotify) {
            if (LL.D)
                Log.d(LOG_TAG, "  - START notify listener : " + listener + listener.getSetTimeLabel() + listener);
            if (errorCode == ErrorCode.NONE) {
                listener.onSetTimeDone(iCurrentTime, iFrameTime);
            } else {
                listener.onSetTimeFail(errorCode);
            }
            if (LL.D)
                Log.d(LOG_TAG, "  - DONE notify listener : " + listener.getSetTimeLabel() + " " + listener);
        }

        if (LL.D) Log.d(LOG_TAG, "(removing " + toNotify.size() + " listeners)");

        m_setTimeDoneListeners.removeAll(toNotify);
        dumpSetTimeDoneLabels();

        // We're done seeking
        setIsSeekingFlag(false);
        if (LL.D) Log.d(LOG_TAG, "(seek state set to FALSE)");

        if (m_onSetTimeCancelListeners.size() > 0) {
            if (LL.D)
                Log.d(LOG_TAG, "onSetTimeDone [m_onSetTimeCancelListeners>0]: m_isPendingSeek=" + m_isPendingSeek + " -> false");
            m_isPendingSeek = false;
            m_isPendingNonDisplaySeek = false;
            while (m_onSetTimeCancelListeners.size() > 0) {
                m_onSetTimeCancelListeners.remove().onComplete(ErrorCode.NONE);
            }
        }

        // If there's a pending seek operation, we need to start it now
        if (m_isPendingNonDisplaySeek) {
            if (LL.D)
                Log.d(LOG_TAG, "execute pending non-display seek : " + m_pendingNonDisplaySeekLocation);
            m_isPendingNonDisplaySeek = false;
            seek_internal(m_pendingNonDisplaySeekLocation, false, 0, null);
//			seek(m_pendingNonDisplaySeekLocation);
        } else if (m_isPendingSeek) {
            if (LL.D)
                Log.d(LOG_TAG, "execute pending seek : " + m_pendingSeekLocation + ", m_pendingSeekIDR=" + m_pendingSeekIDR);
            m_isPendingSeek = false;
            if (m_pendingSeekIDR == 0) {
                seek(m_pendingSeekLocation);
            } else if (m_pendingSeekIDR == 1) {
                seekIOnly(m_pendingSeekLocation, null);
            } else if (m_pendingSeekIDR == 4) {
                seekFromCache(m_pendingSeekLocation, null);
            } else {
                seekIDROnly(m_pendingSeekLocation, null);
            }
        }

        if (LL.D)
            Log.d(LOG_TAG, "out onSetTimeDone(" + iCurrentTime + "," + iFrameTime + ") " + seekStateDump() + " " + m_setTimeDoneListeners.size());

    }

    protected void onSetTimeIgnored() {
        onSetTimeDone(0, 0, ErrorCode.SET_TIME_IGNORED);
    }

    // -----------------------------------------------------------------------------------------------------

    public void clearPendingEvents(OnCompletionListener completionListener) {
        if (m_listener == null) {
            if (LL.W) Log.w(LOG_TAG, "clearPendingEvents : Null event handler");
            return;
        }
        m_listener.ignoreEventsUntilTag(m_nextTag);
        m_onPlayListeners.clear();
        m_onStopListeners.clear();
        //m_parseRequests.clear();
        m_addAudioClipDoneListeners.clear();
        m_addVisualClipDoneListeners.clear();
        m_onCaptureListeners.clear();
        m_deleteClipDoneListeners.clear();
        m_loadListAsyncTasks.clear();
        m_moveClipDoneListeners.clear();
        m_setTimeDoneListeners.clear();
        m_onSetTimeCancelListeners.clear();
        //m_getInfoListeners.clear();
        setIsSeekingFlag(false);
        m_isPendingSeek = false;
        m_isPendingNonDisplaySeek = false;
        mCaptureInProgress = false;
        if (LL.D)
            Log.d(LOG_TAG, "clearPendingEvents; untilTag=" + m_nextTag + " (completion listener given)");
        finishAllCommands(completionListener);
    }

    private void setIsSeekingFlag(boolean isSeeking) {
        if (isSeeking == m_isSeeking)
            return;
        m_isSeeking = isSeeking;
        if (m_uiListener != null) {
            m_uiListener.onSeekStateChanged(m_isSeeking);
        }
    }

    private static final OnCompletionListener m_emptyCompletionListener = new OnCompletionListener() {
        @Override
        public void onComplete(ErrorCode resultCode) {
            // Do nothing
        }
    };

    private int tagNextCommand() {

        int tvalue = m_nextTag;
        m_nextTag++;

        int result = commandMarker(tvalue);
        if (result != 0) {
            if (LL.D) Log.d(LOG_TAG, "FAILED(" + result + ") tag() -> " + tvalue);
        } else {
            if (LL.D) Log.d(LOG_TAG, "tag() -> " + tvalue);
        }

        return tvalue;
    }

    public void finishAllCommands(OnCompletionListener completionListener) {
        completionListener.m_tag = m_nextTag;
        int result = commandMarker(m_nextTag);
        if (LL.D) Log.d(LOG_TAG, "finishAllCommands; tag=" + m_nextTag + " resut=" + result);
        m_nextTag++;
        if (result == 0) {
            m_onCommandMarkerListeners.add(completionListener);
        } else {
            completionListener.onComplete(ErrorCode.fromValue(result));
        }
    }

    /*
    public int setTime(int iTime, OnSetTimeDoneListener onSetTimeDoneListener ) {
    	return setTime(iTime, onSetTimeDoneListener, false);
    }

    public int setTimeMerge(int iTime) {
    	return setTime(iTime, null, true);
    }

    //private int m_pendingSeek = -1;
    private int m_seekSerial = 0;

    private int setTime(int iTime, OnSetTimeDoneListener onSetTimeDoneListener, boolean consolidate ) {

    	if( consolidate ) {
    		m_seekSerial++;
    		final int seekSerial = m_seekSerial;
    		final int seekTime = iTime;
    		if(LL.D) Log.d(LOG_TAG,"setTime(" + iTime + ") CONSOLIDATED; sn=" + m_seekSerial );
			finishAllCommands(new OnCompletionListener() {
				@Override
				public void onComplete(ErrorCode resultCode) {
					if( m_seekSerial==seekSerial ) {
			    		if(LL.D) Log.d(LOG_TAG,"setTime(" + seekTime + ") EXECUTING PENDING SET for sn=" + m_seekSerial );
						setTime( seekTime, null, false );
					} else {
			    		if(LL.D) Log.d(LOG_TAG,"setTime(" + seekTime + ") FOR sn=" + seekSerial + " SKIPPING due to newer sn=" + m_seekSerial );
					}
				}
			});
    		return 0;
    	}

    	if( onSetTimeDoneListener==null ) {
    		onSetTimeDoneListener = new OnSetTimeDoneListener() {
				@Override
				public void onSetTimeFail(ErrorCode returnCode) {
					if(LL.D) Log.d(LOG_TAG,"[DEFAULT] onSetTimeFail(" + returnCode + ")");
				}
				@Override
				public void onSetTimeDone(int time) {
					if(LL.D) Log.d(LOG_TAG,"[DEFAULT] onSetTimeDone(" + time + ")");
				}
			};
    	}

    	if( DEBUG ) {
    		onSetTimeDoneListener.m_stackTrace = Thread.currentThread().getStackTrace();
    	}
    	onSetTimeDoneListener.m_reqTime = iTime;
    	onSetTimeDoneListener.m_tag = tagNextCommand();
    	int result = setTime(iTime);
    	if( result==0 ) // Success
    	{
    		m_seekSerial++;
    		m_setTimeDoneListeners.add(onSetTimeDoneListener);
    	}
		if(LL.D) Log.d(LOG_TAG,"setTime(" + iTime + ") result=" + result + "; m_setTimeDoneListeners.size()=" + m_setTimeDoneListeners.size() +  setTimeListenerDump() );
    	return result;
    }
    */

    public boolean isAddingVisualClip() {
        return !m_addVisualClipDoneListeners.isEmpty();
    }

    public int addVisualClip(int iNextToClipID, String strFilePath, OnAddClipDoneListener onAddClipDoneListener) {
        int clipId = m_iNextClipID++;
        onAddClipDoneListener.clipId = clipId;
        int result = addVisualClip(iNextToClipID, strFilePath, clipId);
        if (result == 0) // Success
        {
            m_addVisualClipDoneListeners.add(onAddClipDoneListener);
        }
        return result;
    }

    public int addAudioClip(int iStartTime, String strFilePath, OnAddClipDoneListener onAddClipDoneListener) {
        int clipId = m_iNextClipID++;
        onAddClipDoneListener.clipId = clipId;
        int result = addAudioClip(iStartTime, strFilePath, clipId);
        if (result == 0) // Success
        {
            m_addAudioClipDoneListeners.add(onAddClipDoneListener);
        }
        return result;
    }

    public int moveVisualClip(int iNextToClipID, int iMovedClipID, OnMoveClipDoneListener onMoveClipDoneListener) {
        int result = moveVisualClip(iNextToClipID, iMovedClipID);
        if (result == 0) // Success
        {
            m_moveClipDoneListeners.add(onMoveClipDoneListener);
            m_movedClipId.add(iMovedClipID);
        }
        return result;
    }

    public int moveAudioClip(int iStartTime, int iMovedClipID, OnMoveClipDoneListener onMoveClipDoneListener) {
        int result = moveAudioClip(iStartTime, iMovedClipID);
        if (result == 0) // Success
        {
            m_moveClipDoneListeners.add(onMoveClipDoneListener);
            m_movedClipId.add(iMovedClipID);
        }
        return result;
    }

    public int deleteClipID(int iClipID, OnDeleteClipDoneListener onDeleteClipDoneListener) {
        if (onDeleteClipDoneListener == null) {
            onDeleteClipDoneListener = new OnDeleteClipDoneListener() {
                @Override
                public void onDeleteClipDone() {
                }

                @Override
                public void onDeleteClipFail(ErrorCode returnCode) {
                }
            };
        }

        int result = deleteClipID(iClipID);
        if (result == 0) // Success
        {
            m_deleteClipDoneListeners.add(onDeleteClipDoneListener);
        }
        return result;
    }

    // Param iVersion
    // Return value is less then 0 if function fail
    // Return value is Major version if iVersio is 1
    // Return value is Minor version if iVersio is 2
    // Return value is Patch version if iVersio is 3
    // Return value is Build version if iVersio is 4
    public native int getVersionNumber(int iVersion);

    protected native int nativeVideoRender(int uiCurrentTime);

    private interface ClipCallback {
        public int addClip(NexVisualClip item);

        public int deleteClip(int iClipID);

        public void clearAllClip();
    }

    public NexVisualClip[] getClipList() {
        final List<NexVisualClip> list = new ArrayList<NexVisualClip>();
        getClipList(new ClipCallback() {
            public int deleteClip(int iClipID) {
                return 0;
            }

            public void clearAllClip() {
            }

            public int addClip(NexVisualClip item) {
                list.add(item);
                return 0;
            }
        });
        return (NexVisualClip[]) list.toArray(new NexVisualClip[list.size()]);
    }

//    public void loadThemeSet( int resource) {
//
//    	if( m_useThemeManager )
//    		throw new RuntimeException("Not supported when using theme manager");
//
//    	if( mView == null )
//    		return;
//
//    	String themeData = null;
//
//    	try {
//	    	InputStream s = mView.getContext().getResources().openRawResource(resource);
//	    	byte[] b = new byte[s.available()];
//	    	s.read(b);
//	    	themeData = new String(b);
//    	} catch( IOException e ) {
//    		themeData = "";
//    	}
//    	loadTheme( themeData, 0 );
//    }
//
//    public void loadThemeSet(String themeSetFilePath) {
//
//    	if( m_useThemeManager )
//    		throw new RuntimeException("Not supported when using theme manager");
//
//    	loadTheme( themeSetFilePath, 1 );
//    	m_themeList = null;
//    	m_titleEffectList = null;
//    	m_transitionEffectList = null;
//
//    	setTheme(0);
//    }

//    public void setTheme( int themeIndex ) {
//
//    	if( m_useThemeManager )
//    		throw new RuntimeException("Not supported when using theme manager");
//
//    	Theme[] themeList = getThemeList();
//    	if( themeIndex>=0 && themeIndex<themeList.length ) {
//			setTheme(themeList[themeIndex]);
//    	}
//    }

    public void setThemeData(String themeData, boolean forExport) {
        int iFlags = 0;

        if (!forExport) {
            iFlags |= LOAD_THEME_PREVIEW;
        }
        loadTheme(String.format(""), themeData, iFlags);
    }

    public void clearRenderItems(boolean forExport) {

        int iFlags = 0;

        if (!forExport) {
            iFlags |= LOAD_THEME_PREVIEW;
        }
        clearRenderItems(iFlags);
    }

    public void setRenderItem(String renderitem_id, String renderitem_data, boolean forExport) {

        int iFlags = 0;

        if (!forExport) {
            iFlags |= LOAD_THEME_PREVIEW;
        }
        loadRenderItem(renderitem_id, renderitem_data, iFlags);
    }

    public String getCurrentThemeId() {
        return m_currentThemeId;
    }

    public Effect[] getTransitionEffectList() {

        if (m_useThemeManager)
            throw new RuntimeException("Not supported when using theme manager");

        if (m_transitionEffectList == null) {
            final List<Effect> effectList = new ArrayList<Effect>();
            getClipEffectList(new ThemeItemCallback(effectList) {
                @Override
                public int addItem(String strEffectID, String strEffectName,
                                   String strEffectIcon, int iEffectOffset,
                                   int iEffectOverlap, int isGlobal, int minDuration,
                                   int maxDuration, int defaultDuration) {
                    if (!contains(strEffectID))
                        effectList.add(new Effect(strEffectID, strEffectName, strEffectIcon, isGlobal == 0 ? false : true, iEffectOffset, iEffectOverlap, minDuration, maxDuration, defaultDuration));
                    return 0;
                }
            });
            m_transitionEffectList = (Effect[]) effectList.toArray(new Effect[effectList.size()]);
        }
        return m_transitionEffectList;
    }

    public Effect[] getTitleEffectList() {

        if (m_useThemeManager)
            throw new RuntimeException("Not supported when using theme manager");

        if (m_titleEffectList == null) {
            final List<Effect> effectList = new ArrayList<Effect>();
            getTitleEffectList(new ThemeItemCallback(effectList) {
                @Override
                public int addItem(String strEffectID, String strEffectName,
                                   String strEffectIcon, int iEffectOffset,
                                   int iEffectOverlap, int isGlobal, int minDuration,
                                   int maxDuration, int defaultDuration) {
                    if (!contains(strEffectID))
                        effectList.add(new Effect(strEffectID, strEffectName, strEffectIcon, isGlobal == 0 ? false : true, iEffectOffset, iEffectOverlap, minDuration, maxDuration, defaultDuration));
                    return 0;
                }

            });
            m_titleEffectList = (Effect[]) effectList.toArray(new Effect[effectList.size()]);
        }
        return m_titleEffectList;
    }

    private abstract class ThemeItemCallback {
        private List<? extends ThemeElement> m_list;

        private ThemeItemCallback(List<? extends ThemeElement> list) {
            m_list = list;
        }

        @SuppressWarnings("unused")    // called from native
        public int addItem(String strEffectID, String strEffectName) {
            return addItem(strEffectID, strEffectName, null, 0, 0, 1, 0, 0, 0);
        }

        @SuppressWarnings("unused")    // called from native
        public int addItem(String strEffectID, String strEffectName, String strEffectIcon, int isGlobal) {
            return addItem(strEffectID, strEffectName, strEffectIcon, 0, 0, isGlobal, 0, 0, 0);
        }

        public abstract int addItem(String strEffectID, String strEffectName, String strEffectIcon, int iEffectOffset, int iEffectOverlap, int isGlobal, int minDuration, int maxDuration, int defaultDuration);

        @SuppressWarnings("unused")    // called from native
        public int deleteItem(String strID) {
            for (int i = 0; i < m_list.size(); i++) {
                if (m_list.get(i).getId().equals(strID)) {
                    m_list.remove(i);
                    return 0;
                }
            }
            return 0;
        }

        @SuppressWarnings("unused")    // called from native
        public void clearItems() {
            m_list.clear();
        }

        public boolean contains(String id) {
            for (int i = 0; i < m_list.size(); i++) {
                if (m_list.get(i).getId().equals(id)) {
                    return true;
                }
            }
            return false;
        }
    }

    public static abstract class ThemeElement {
        public abstract String getName();

        public abstract String getIcon();

        public abstract String getId();
    }

    public static class Theme extends ThemeElement {
        private String m_id = null;
        private String m_name = null;
        private String m_icon = null;

        protected Theme(String id, String name) {
            m_id = id;
            if (name.contains("$")) {
                int splitIdx = name.lastIndexOf("$");
                m_name = name.substring(0, splitIdx);
                m_icon = name.substring(splitIdx + 1);
            } else {
                m_name = name;
                m_icon = null;
            }
        }

        public String getIcon() {
            return m_icon;
        }

        public String getName() {
            return m_name;
        }

        public String getId() {
            return m_id;
        }
    }

    private static String unescape(String s) {
        StringBuilder b = new StringBuilder();
        int len = s.length();
        char c;
        boolean esc = false;
        for (int i = 0; i < len; i++) {
            c = s.charAt(i);
            if (esc) {
                esc = false;
                if (c == 't') {
                    b.append("\t");
                } else if (c == '\\') {
                    b.append('\\');
                }
            } else if (c == '\\') {
                esc = true;
            } else {
                b.append(c);
            }
        }
        return b.toString();
    }

//    public void setUseThemeManager( boolean bUseThemeManager ) {
//    	m_useThemeManager = bUseThemeManager;
//    }
//
//    public boolean getUseThemeManager() {
//    	return m_useThemeManager;
//    }
//
//    public NexThemeManager getThemeManager() {
//    	if( !m_useThemeManager )
//    		return null;
//
//    	return m_themeManager;
//    }

    public File thumbnailPath(File originalPath) {
        if (m_thumbnailDirectory == null)
            throw new Error("Thumbnail directory not set");
        String name = originalPath.getName();
        int pathHash = originalPath.getAbsolutePath().hashCode();
        long modTime = originalPath.lastModified();
        long fileSize = originalPath.length();
        if (name.length() > 32) {
            name = name.substring(0, 32);
        }
        int hashcode = (int) (((modTime * 212501089) + (fileSize * 194851861) + pathHash) % 0xFFFFFFF);
        name = name + "_" + String.format(Locale.US, "%08X", hashcode) + ".thumb";
        return new File(m_thumbnailDirectory, name);
    }

//    public ErrorCode getClipInfo(String strClipPath,  NexClipInfo clipInfo ) {
//    	int result = getClipInfo(strClipPath, thumbnailPath(strClipPath==null?null:new File(strClipPath)).getAbsolutePath(), clipInfo, 0, 0, 0, 0);
//    	return ErrorCode.fromValue(result);
//    }

    // MEMALLOC_FAILED      Not enough memory and etc.
    // ARGUMENT_FAILED		Wrong file path.
    // FILE_INVALID_SYNTAX	Broken file
    // UNSUPPORT_FORMAT  	(Not .MP4, .MP3 and .AAC format)

    // UNSUPPORT_MIN_DURATION(				30),
    // UNSUPPORT_MAX_RESOLUTION(			31),
    // UNSUPPORT_MIN_RESOLUTION(			32),
    // UNSUPPORT_VIDEIO_PROFILE(			33),
    // UNSUPPORT_VIDEO_LEVEL(				34),
    // UNSUPPORT_VIDEO_FPS(					35),

    // UNSUPPORT_AUDIO_PROFILE(				46),
    public ErrorCode getClipInfoSync(String strClipPath, NexClipInfo clipInfo, boolean includeSeekTable, int userTag) {
        int result = getClipInfoSync(strClipPath, clipInfo, includeSeekTable ? NexEditorType.GET_CLIPINFO_INCLUDE_SEEKTABLE : 0, userTag);

        // int iFlag = NexEditorType.GET_CLIPINFO_INCLUDE_SEEKTABLE | NexEditorType.GET_CLIPINFO_CHECK_AUDIO_DEC | NexEditorType.GET_CLIPINFO_CHECK_VIDEO_DEC;
        // int result = getClipInfoSync(strClipPath, clipInfo, iFlag, userTag);
        return ErrorCode.fromValue(result);
    }

    public ErrorCode getClipInfoSyncFlags(String strClipPath, NexClipInfo clipInfo, int flags, int userTag) {
        int result = getClipInfoSync(strClipPath, clipInfo, flags, userTag);

        // int iFlag = NexEditorType.GET_CLIPINFO_INCLUDE_SEEKTABLE | NexEditorType.GET_CLIPINFO_CHECK_AUDIO_DEC | NexEditorType.GET_CLIPINFO_CHECK_VIDEO_DEC;
        // int result = getClipInfoSync(strClipPath, clipInfo, iFlag, userTag);
        return ErrorCode.fromValue(result);
    }

//    private static class ParseRequest {
//    	public String path;
//    	public OnCompletionListener listener;
//    	boolean needPCM;
//    	ParseRequest( String path, OnCompletionListener listener, boolean needPCM ) {
//    		this.path = path;
//    		this.listener = listener;
//    		this.needPCM = needPCM;
//    	}
//    }

//    private Deque<ParseRequest> m_parseRequests = new LinkedList<ParseRequest>();
//    private boolean m_parseInProgress = false;

//    private void handleNextParseRequest() {
//    	if( m_parseInProgress || m_parseRequests.isEmpty() )
//    		return;
//    	ParseRequest rq = m_parseRequests.remove();
//    	m_getInfoTag++;
//    	m_parseInProgress = true;
//    	m_getInfoListeners.put(m_getInfoTag, rq.listener);
//    	if(LL.D) Log.d(LOG_TAG,"Request async getClipInfo (tag=" + m_getInfoTag + "): " + rq.path);
//    	int result = getClipInfo(rq.path, thumbnailPath(new File(rq.path)).getAbsolutePath(),  null, 1, rq.needPCM?1:0, 1, m_getInfoTag);
//    	if( result!= 0 ) {
//        	m_parseInProgress = false;
//        	rq.listener.onComplete(ErrorCode.fromValue(result));
//        	handleNextParseRequest();
//    	}
//    }
//
//    public void parseClip(String strClipPath, OnCompletionListener listener, boolean needPCM)
//    {
//    	m_parseRequests.add( new ParseRequest( strClipPath, listener, needPCM ));
//    	handleNextParseRequest();
//    }
//
//    public void onGetClipInfoDone(ErrorCode resultCode, int tag) {
//    	m_parseInProgress = false;
//    	handleNextParseRequest();
//
//    	OnCompletionListener listener = m_getInfoListeners.get(tag);
//    	if( listener!=null ) {
//    		m_getInfoListeners.remove(tag);
//    		listener.onComplete(resultCode);
//    	}
//    }

    public interface OnGetClipInfoDoneListener {
        void onGetClipInfoDone(ErrorCode resultCode, int tag);
    }

    private OnGetClipInfoDoneListener m_onGetClipInfoDoneListener;

    public void setOnGetClipInfoDoneListener(OnGetClipInfoDoneListener listener) {
        m_onGetClipInfoDoneListener = listener;
    }

    public void onGetClipInfoDone(ErrorCode resultCode, int tag) {
        if (LL.D)
            Log.d(LOG_TAG, "onGetClipInfoDone onGetClipInfoDone=" + resultCode + " tag=" + tag);
        if (m_onGetClipInfoDoneListener != null) {
            m_onGetClipInfoDoneListener.onGetClipInfoDone(resultCode, tag);
        }
    }


    public static class Effect extends ThemeElement {
        private String m_id = null;
        private String m_name = null;
        private String m_categoryTitle = null;
        private String m_icon = null;
        private int m_effectOffset = 0;
        private int m_videoOverlap = 0;
        private int m_maximumDuration = 0;
        private int m_minimumDuration = 0;
        private int m_defaultDuration = 0;
        private boolean m_isGlobal = true;

        protected Effect(String id, String name, String icon, boolean isGlobal, int iEffectOffset, int iEffectOverlap, int minimumDuration, int maximumDuration, int defaultDuration) {
            m_id = id;
            if (name != null && name.contains("\t")) {
                int splitIdx = name.indexOf("\t");
                m_name = unescape(name.substring(0, splitIdx));
                m_categoryTitle = unescape(name.substring(splitIdx + 1));
                name = m_name;
            }
            if (m_categoryTitle == null || m_categoryTitle.length() < 1) {
                m_categoryTitle = "Other";
            }
            if (icon != null) {
                m_name = name;
                m_icon = icon;
            } else if (name.contains("$")) {
                int splitIdx = name.lastIndexOf("$");
                m_name = name.substring(0, splitIdx);
                m_icon = name.substring(splitIdx + 1);
            } else {
                m_name = name;
                m_icon = null;
            }
            m_effectOffset = iEffectOffset;
            m_videoOverlap = iEffectOverlap;
            m_maximumDuration = maximumDuration;
            m_minimumDuration = minimumDuration;
            m_defaultDuration = defaultDuration;
            m_isGlobal = isGlobal;
        }

        public String getIcon() {
            return m_icon;
        }

        public String getName() {
            return m_name;
        }

        public String getCategoryTitle() {
            return m_categoryTitle;
        }

        public String getId() {
            return m_id;
        }

        public int getOffset() {
            return m_effectOffset;
        }

        public int getOverlap() {
            return m_videoOverlap;
        }

        public int getMinimumDuration() {
            return m_minimumDuration;
        }

        public int getMaximumDuration() {
            return m_maximumDuration;
        }

        public int getDefaultDuration() {
            return m_defaultDuration;
        }

        public boolean isGlobal() {
            return m_isGlobal;
        }
    }

    static {
        StrictMode.ThreadPolicy prevThreadPolicy = StrictMode.allowThreadDiskReads();
        try {
        	/*
    		switch( Build.VERSION.SDK_INT ) {
    		case 8:			// FROYO
        		if(LL.E) Log.e(LOG_TAG, "[NexEditor.java] No binaries for API : " + Build.VERSION.SDK_INT + " (FROYO)");
    			break;
    		case 9:			// GB
    		case 10:		// GB-MR1
	    		System.loadLibrary("nexcal_oc_dec_gb");
	    		System.loadLibrary("nexcalbody_iomxenc_gb");
	    		System.loadLibrary("nexvideoeditor_gb");
    			break;
    		case 11:		// HONEYCOMB
    		case 12:		// HONEYCOMB-MR1
    		case 13:		// HONEYCOMB-MR2
        		if(LL.E) Log.e(LOG_TAG, "[NexEditor.java] No binaries for API : " + Build.VERSION.SDK_INT + " (HONEYCOMB)");
    			break;
    		case 14:		// ICS
    		case 15:		// ICS-MR1
	    		System.loadLibrary("nexcal_oc_dec_ics");
	    		System.loadLibrary("nexcalbody_iomxenc_ics");
	    		System.loadLibrary("nexvideoeditor_ics");
    			break;
    		default:
        		if(LL.E) Log.e(LOG_TAG, "[NexEditor.java] No binaries for API : " + Build.VERSION.SDK_INT);
    			break;
    		}
    		*/

            // System.loadLibrary("stlport_shared");
            // System.loadLibrary("nexralbody_audio");
/*
            if( new File("/system/lib/","libnexeditorsdk.so").exists() ) {
                if(LL.D) Log.d(LOG_TAG,"libnexeditorsdk.so libs found in: /system/lib/" );
                System.load("/system/lib/libnexeditorsdk.so");
            } else {
                if(LL.D) Log.d(LOG_TAG,"libnexeditorsdk.so libs NOT FOUND in: /system/lib/");
                System.loadLibrary("nexeditorsdk");
            }
*/
            System.loadLibrary("nexeditorsdk" + EditorGlobal.SO_FREFIX);
        } catch (UnsatisfiedLinkError exc) {
            if (LL.E) Log.e(LOG_TAG, "[NexEditor.java] nexeditor load failed : " + exc);
        } finally {
            StrictMode.setThreadPolicy(prevThreadPolicy);
        }
    }

    public void setOnSurfaceChangeListener(OnSurfaceChangeListener listener) {
        m_onSurfaceChangeListener = listener;
    }

    public void notifySurfaceChanged() {
        if (m_onSurfaceChangeListener != null) {
            m_onSurfaceChangeListener.onSurfaceChanged();
        }
    }

    public static abstract class OnSurfaceChangeListener {
        public abstract void onSurfaceChanged();
    }

    public static abstract class OnAddClipDoneListener {
        int clipId;

        public abstract void onAddClipDone(int clipId, int newTotalDuration, int clipType);

        public abstract void onAddClipFail(ErrorCode returnCode);
    }

    public static abstract class OnDeleteClipDoneListener {
        public abstract void onDeleteClipDone();

        public abstract void onDeleteClipFail(ErrorCode returnCode);
    }

    public static abstract class OnMoveClipDoneListener {
        public abstract void onMoveClipDone(int clipId);

        public abstract void onMoveClipFail(ErrorCode returnCode);
    }

    public static abstract class OnSetTimeDoneListener {
        private int m_reqTime;
        private int m_serialNumber;
        StackTraceElement[] m_stackTrace;

        public abstract void onSetTimeDone(int time, int iFrameTime);

        public abstract void onSetTimeFail(ErrorCode returnCode);

        public abstract String getSetTimeLabel();
    }

    public interface OnDiagnosticEventListener {
        void onDiagnosticEvent(int p1, int p2, int p3, int p4);
    }

    public static abstract class OnCompletionListener {
        private int m_tag;

        public abstract void onComplete(ErrorCode resultCode);
    }

    public static abstract class OnPlayListener {
        public abstract void onPlayRequestProcessed(ErrorCode resultCode);

        public abstract void onPlayStarted();
    }

    public static abstract class OnSeekDoneListener {
        public abstract void onSeekDone(int time, ErrorCode resultCode);
    }

    public static abstract class OnCaptureListener {
        public abstract void onCapture(Bitmap bm);

        public abstract void onCaptureFail(NexEditor.ErrorCode error);
    }

    public static abstract class OnExportListener {
        public abstract void onExportFail(NexEditor.ErrorCode error);

        public abstract void onExportProgress(int cur);

        public abstract void onExportDone();
    }

    public enum PlayState {

        NONE(0),
        IDLE(1),
        RUN(2),
        RECORD(3),
        PAUSE(4),
        RESUME(5);

        private int mValue;

        PlayState(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }

        public static PlayState fromValue(int value) {
            for (PlayState v : PlayState.values()) {
                if (v.getValue() == value)
                    return v;
            }
            return null;
        }
    }

    public void waitForSeekDone(OnSetTimeDoneListener listener) {

        if (listener == null)
            return;

        if (!isSeeking()) {
            listener.onSetTimeDone(m_currentTime, m_frameTime); //TODO:
            return;
        }

        listener.m_serialNumber = m_isPendingSeek ? m_seekSerial : (m_seekSerial - 1);
//    	listener.m_serialNumber = m_isPendingSeek?m_seekSerial+1:(m_seekSerial);
        if (DEBUG) {
            listener.m_stackTrace = Thread.currentThread().getStackTrace();
        }

        m_setTimeDoneListeners.add(listener);
        if (LL.D) Log.d(LOG_TAG, "waitForSeekDone : m_setTimeDoneListeners - Added listener");
        dumpSetTimeDoneLabels();

    }

    public void setBackgroundMusic(File path) {
        if (path == null) {
            setBackgroundMusic((String) null, 0);
            return;
        }
        setBackgroundMusic(path.getAbsolutePath(), m_iNextClipID++);
    }

    public ErrorCode makeVideoThumbs(File clipPath, File thumbPath, int width, int height, int startTime, int endTime, int count, int flags, int userTag) throws IOException {

        // Touch the file once to make sure it's created with the
        // correct permissions.
//    	 FileOutputStream out = new FileOutputStream(thumbPath);
//    	 out.write(0);
//    	 out.close();

        // int result = getClipVideoThumb(clipPath.getAbsolutePath(), thumbPath.getAbsolutePath(), width, height, 1000, 10000, 30, 0x00000003, userTag);
        int result = getClipVideoThumb(clipPath.getAbsolutePath(), thumbPath.getAbsolutePath(), width, height, startTime, endTime, count, flags, userTag);
        return ErrorCode.fromValue(result);
    }

    public ErrorCode makeVideoThumbs(String clipPath, File thumbPath, int width, int height, int startTime, int endTime, int count, int flags, int userTag) throws IOException {
        int result = getClipVideoThumb(clipPath, thumbPath.getAbsolutePath(), width, height, startTime, endTime, count, flags, userTag);
        return ErrorCode.fromValue(result);
    }

    public ErrorCode makeVideoThumbs(File clipPath, File thumbPath, int width, int height, int arraySize, int[] arrayTables, int flags, int userTag) throws IOException {

        // Touch the file once to make sure it's created with the
        // correct permissions.
//    	 FileOutputStream out = new FileOutputStream(thumbPath);
//    	 out.write(0);
//    	 out.close();

        // int result = getClipVideoThumb(clipPath.getAbsolutePath(), thumbPath.getAbsolutePath(), width, height, 1000, 10000, 30, 0x00000003, userTag);
        int result = getClipVideoThumbWithTimeTable(clipPath.getAbsolutePath(), thumbPath.getAbsolutePath(), width, height, arraySize, arrayTables, flags, userTag);
        return ErrorCode.fromValue(result);
    }

    public ErrorCode makeVideoThumbs(String clipPath, File thumbPath, int width, int height, int arraySize, int[] arrayTables, int flags, int userTag) throws IOException {
        int result = getClipVideoThumbWithTimeTable(clipPath, thumbPath.getAbsolutePath(), width, height, arraySize, arrayTables, flags, userTag);
        return ErrorCode.fromValue(result);
    }

    public ErrorCode makeAudioPCM(File clipPath, File pcmPath, int userTag) {
        int result = getClipAudioThumb(clipPath.getAbsolutePath(), pcmPath.getAbsolutePath(), userTag);
        return ErrorCode.fromValue(result);
    }

    public ErrorCode makeAudioPCMPartial(File clipPath, File pcmPath, int startTiem, int endTime, int useCount, int skipCount, int userTag) {
        int result = getClipAudioThumbPartial(clipPath.getAbsolutePath(), pcmPath.getAbsolutePath(), startTiem, endTime, useCount, skipCount, userTag);
        return ErrorCode.fromValue(result);
    }


    public ErrorCode makeAudioPCM(String clipPath, File pcmPath, int userTag) {
        int result = getClipAudioThumb(clipPath, pcmPath.getAbsolutePath(), userTag);
        return ErrorCode.fromValue(result);
    }

    public ErrorCode makeAudioPCMPartial(String clipPath, File pcmPath, int startTiem, int endTime, int useCount, int skipCount, int userTag) {
        int result = getClipAudioThumbPartial(clipPath, pcmPath.getAbsolutePath(), startTiem, endTime, useCount, skipCount, userTag);
        return ErrorCode.fromValue(result);
    }

    public ErrorCode getThumbStop(int userTag) {
        int result = getClipStopThumb(userTag);
        return ErrorCode.fromValue(result);
    }

    public int loadClips(NexVisualClip[] visualClipList, NexAudioClip[] audioClipList, int flag) {
        int maxClipId = 0;
        if (visualClipList != null) {
            for (NexVisualClip clip : visualClipList) {
                if (clip != null && clip.mClipID > maxClipId) {
                    maxClipId = clip.mClipID;
                }
            }
        }
        if (audioClipList != null) {
            for (NexAudioClip clip : audioClipList) {
                if (clip != null && clip.mClipID > maxClipId) {
                    maxClipId = clip.mClipID;
                }
            }
        }
        m_iNextClipID = maxClipId + 1;
        int result = loadList(visualClipList, audioClipList, flag); //3.0 used clip list
        return result;
    }

    void onAsyncLoadListDone(int errorCode) {
        if (LL.D) Log.d(LOG_TAG, "onAsyncLoadListDone(" + errorCode + ")");
        if (m_loadListAsyncTasks.size() > 0) {
            Task task = m_loadListAsyncTasks.remove();
            if (errorCode == 0) {
                task.signalEvent(Event.COMPLETE, Event.SUCCESS);
            } else {
                task.sendFailure(ErrorCode.fromValue(errorCode));
            }
        }
    }

    public Task loadClipsAsync(NexVisualClip[] visualClipList, NexAudioClip[] audioClipList) {
        return loadClipsAsync(visualClipList, audioClipList, 0);
    }

    public Task loadClipsAsync(NexVisualClip[] visualClipList, NexAudioClip[] audioClipList, int flag) {
        Task task = new Task();
        int maxClipId = 0;
        if (visualClipList != null) {
            for (NexVisualClip clip : visualClipList) {
                if (clip != null && clip.mClipID > maxClipId) {
                    maxClipId = clip.mClipID;
                }
            }
        }
        if (audioClipList != null) {
            for (NexAudioClip clip : audioClipList) {
                if (clip != null && clip.mClipID > maxClipId) {
                    maxClipId = clip.mClipID;
                }
            }
        }
        m_iNextClipID = maxClipId + 1;

        int result = asyncLoadList(visualClipList, audioClipList, flag);
        if (result == 0) {
            m_loadListAsyncTasks.add(task);
        } else {
            task.sendFailure(ErrorCode.fromValue(result));
        }
        return task;
    }


    public boolean isPlayPending() {
        return m_playRequested || m_playRequestPending;
    }

    public boolean isSeeking() {
        return m_isSeeking;
    }

    public boolean seekedSinceLastPlay() {
        return m_didSetTimeSinceLastPlay;
    }

    public ColorFormat getColorFormatTaskResult() {
        return mColorFormatTaskResult;
    }

    public void setWatermark(boolean flag) {

        if (m_listener != null) {
            m_listener.setWatermark(flag);
        }
    }

    public static final int PROP_MULTISAMPLE = 1;
    public static final int PROP_DEPTH_BUFFER_BITS = 2;
    public static final int PROP_ENGINE_LOG_LEVEL = 3;

    public void setSyncMode(boolean set) {
        if (m_listener != null) {
            m_listener.setSyncMode(set);
        }
    }

    public int getSDKInfo_WM() {
        return getNativeSDKInfoWM();
    }

    //
    private static native void setPacakgeName4Protection(String package_name);

    public static void setApplicationPacakge4Protection(String package_name) {
        setPacakgeName4Protection(package_name);
    }

    private static native void setEncInfo(String[] info);

    public static void setEncInfoString(String[] info) {
        setEncInfo(info);
    }

    public int autoTrim(String path, int idrIndex, int interval, int count, int decodeMode) {
        return highlightStart(path, idrIndex, interval, count, 0, "dummy", 0, 0, 0, Long.MAX_VALUE, decodeMode);
    }

    public int autoTrimStop() {
        return highlightStop();
    }

    private String setOutPutPath(FileDescriptor fileDescriptor) {
        int fd = setOutputFile(fileDescriptor, 0, 0xffffffff);
        if (fd >= 0) {
            return String.format("writefd://%d", fd);
        }
        return null;
    }

    /* test
	private FileOutputStream mExportOutputStream;
	public String setOutPutPath(String Path){
		try {

			mExportOutputStream = new FileOutputStream(new File(Path));
			try {
				FileDescriptor fd = mExportOutputStream.getFD();
				return setOutPutPath(fd);
			} catch (IOException e) {
				e.printStackTrace();
			}
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		return null;
	}
*/
    private void closeOutPutPath() {
        if (mExportPath != null) {
            if (mExportPath.contains("writefd://")) {
                int start = "writefd://".length();
                int fd = Integer.parseInt(mExportPath.substring(start));
                closeOutputFile(fd);
                mExportPath = null;
            }
        }
/*
		if( mExportOutputStream != null ){
			try {
				mExportOutputStream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
			mExportOutputStream = null;
		}
*/
    }

    public void setExportVideoTrackUUID(int mode, byte[] uuid) {
        setVideoTrackUUID(mode, uuid);
    }

    public void setExportListener(OnExportListener listener) {
        if (listener == null) {
            m_onExportListeners.clear();
            return;
        }
        m_onExportListeners.add(listener);
    }

    public static void setPrepareSurfaceSetToNull(boolean on) {
        sPrepareSurfaceSetToNull = on;
    }

    public int getAudioSessionID(boolean f) {
        return getAudioSessionID();
    }

    public boolean set360VideoViewPosition(int angleX, int angleY) {
        set360VideoTrackPosition(angleX, angleY, 1);
        return true;
    }

    public boolean set360VideoForceNormalView() {
        set360VideoTrackPosition(0, 0, 0);
        return true;
    }

    public int resetLUTTextuer(int res_id) {
        return releaseLUTTexture(res_id);
    }

    public int[] makeLGLUT(byte[] src, int w, int h) {

        return createLGLUT(src);
    }

    public int[] makeCubeLUT(byte[] cube_str, int offset, int length) {

        return createCubeLUT(cube_str);
    }

    private float mPreviewScaleFactor = 1f;

    public boolean setPreviewScale(float scaleFactor) {
        if (scaleFactor > 0 && scaleFactor <= 1f) {
            mPreviewScaleFactor = scaleFactor;
            return true;
        }
        return false;
    }

    public int[] decodeNV12(byte[] nv12, int width, int height) {

        final int frameSize = width * height;

        int rgb[] = new int[width * height];
        for (int j = 0, yp = 0; j < height; j++) {
            int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
            for (int i = 0; i < width; i++, yp++) {
                int y = (0xff & ((int) nv12[yp])) - 16;
                if (y < 0) y = 0;
                if ((i & 1) == 0) {
                    u = (0xff & nv12[uvp++]) - 128;
                    v = (0xff & nv12[uvp++]) - 128;
                }

                int y1192 = 1192 * y;
                int r = (y1192 + 1634 * v);
                int g = (y1192 - 833 * v - 400 * u);
                int b = (y1192 + 2066 * u);

                if (r < 0) r = 0;
                else if (r > 262143) r = 262143;
                if (g < 0) g = 0;
                else if (g > 262143) g = 262143;
                if (b < 0) b = 0;
                else if (b > 262143) b = 262143;

                rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) &
                        0xff00) | ((b >> 10) & 0xff);
            }
        }
        return rgb;
    }

    public Bitmap getCacheThumbData(int time) {
        byte[] yuv = getTimeThumbData(time);
        if (yuv == null)
            return null;
        if (yuv.length == 0) {
            return null;
        }

        final int[] rgb = decodeNV12(yuv, 320, 240);
        return Bitmap.createBitmap(rgb, 320, 240, Bitmap.Config.ARGB_8888);
    }

    public int setThumbnailRoutineFirst(int value) {
        return setThumbnailRoutine(value);
    }

    public static class PerformanceCounterSnapshot {
        private long[] initialValues = new long[PerformanceCounter.values().length];
        private long[] snapshotValues;

        public PerformanceCounterSnapshot() {
            snapshotValues = initialValues;
            for (PerformanceCounter pc : PerformanceCounter.values()) {
                initialValues[pc.ordinal()] = pc.get();
            }
        }

        public PerformanceCounterSnapshot(PerformanceCounterSnapshot since) {
            snapshotValues = new long[PerformanceCounter.values().length];
            for (PerformanceCounter pc : PerformanceCounter.values()) {
                initialValues[pc.ordinal()] = since.initialValues[pc.ordinal()];
                snapshotValues[pc.ordinal()] = pc.get() - since.initialValues[pc.ordinal()];
            }
        }
    }

    public enum PerformanceCounter {
        FRAME_DROP(0),
        FRAME_WAIT_TIMEOUT(1);
        private final int value;

        PerformanceCounter(int value) {
            this.value = value;
        }

        public long get() {
            return getPerformanceCount(value);
        }

        public long get(PerformanceCounterSnapshot pcs) {
            if (pcs == null)
                return 0;
            return pcs.snapshotValues[ordinal()];
        }

        public long since(PerformanceCounterSnapshot pcs) {
            return get() - pcs.initialValues[ordinal()];
        }
    }
//	public static final int PC_FRAME_DROP = 0;
//	public static final int PC_FRAME_WAIT_TIMEOUT = 1;

    private static native long getPerformanceCount(int type);

    public enum ErrorCode implements TaskError {
        NONE(0),
        GENERAL(1),
        UNKNOWN(2),
        NO_ACTION(3),
        INVALID_INFO(4),
        INVALID_STATE(5),
        VERSION_MISMATCH(6),
        CREATE_FAILED(7),
        MEMALLOC_FAILED(8, "Memory allocation failed", 0/*R.string.engine_error_memalloc_failed*/),
        ARGUMENT_FAILED(9),
        NOT_ENOUGH_NEMORY(10, "Insufficient memory", 0/*	R.string.engine_error_not_enough_memory*/),
        EVENTHANDLER(11),
        FILE_IO_FAILED(12, "Error accessing file", 0/*	R.string.engine_error_fileio_fail*/),
        FILE_INVALID_SYNTAX(13),
        FILEREADER_CREATE_FAIL(14, "Could not open file", 0/*	R.string.engine_error_filereader_create_fail*/),
        FILEWRITER_CREATE_FAIL(15),
        AUDIORESAMPLER_CREATE_FAIL(16),
        UNSUPPORT_FORMAT(17, "Unsupported format", 0/*R.string.engine_error_unsupported_format*/),
        FILEREADER_FAILED(18, "Error reading file format", 0/*R.string.engine_error_filereader_fail*/),
        PLAYSTART_FAILED(19),
        PLAYSTOP_FAILED(20),
        PROJECT_NOT_CREATE(21),
        PROJECT_NOT_OPEN(22),
        CODEC_INIT(23, "Codec init failed", 0/*R.string.engine_error_codec_init_failed*/),
        RENDERER_INIT(24),
        THEMESET_CREATE_FAIL(25),
        ADD_CLIP_FAIL(26, "Unable to add clip", 0/*R.string.engine_error_add_clip_failed*/),
        ENCODE_VIDEO_FAIL(27),
        INPROGRESS_GETCLIPINFO(28),
        THUMBNAIL_BUSY(29),

        UNSUPPORT_MIN_DURATION(30),
        UNSUPPORT_MAX_RESOLUTION(31),
        UNSUPPORT_MIN_RESOLUTION(32),
        UNSUPPORT_VIDEIO_PROFILE(33),
        UNSUPPORT_VIDEO_LEVEL(34),
        UNSUPPORT_VIDEO_FPS(35),

        TRANSCODING_BUSY(36),
        TRANSCODING_NOT_SUPPORTED_FORMAT(37),
        TRANSCODING_USER_CANCEL(38),

        TRANSCODING_NOT_ENOUGHT_DISK_SPACE(39),
        TRANSCODING_CODEC_FAILED(40),

        EXPORT_WRITER_INVAILED_HANDLE(41),
        EXPORT_WRITER_INIT_FAIL(42),
        EXPORT_WRITER_START_FAIL(43),
        EXPORT_AUDIO_DEC_INIT_FAIL(44),
        EXPORT_VIDEO_DEC_INIT_FAIL(45),
        EXPORT_VIDEO_ENC_FAIL(46),
        EXPORT_VIDEO_RENDER_INIT_FAIL(47),
        EXPORT_NOT_ENOUGHT_DISK_SPACE(48, "Not enough space", 0/*R.string.fail_enospc*/),

        UNSUPPORT_AUDIO_PROFILE(49),

        THUMBNAIL_INIT_FAIL(50),

        UNSUPPORT_AUDIO_CODEC(51),
        UNSUPPORT_VIDEO_CODEC(52),

        HIGHLIGHT_FILEREADER_INIT_ERROR(53),
        HIGHLIGHT_TOO_SHORT_CONTENTS(54),
        HIGHLIGHT_CODEC_INIT_ERROR(55),
        HIGHLIGHT_CODEC_DECODE_ERROR(56),
        HIGHLIGHT_RENDER_INIT_ERROR(57),
        HIGHLIGHT_WRITER_INIT_ERROR(58),
        HIGHLIGHT_WRITER_WRITE_ERROR(59),
        HIGHLIGHT_GET_INDEX_ERROR(60),    // 60
        HIGHLIGHT_USER_CANCEL(61),

        GETCLIPINFO_USER_CANCEL(62),

        DIRECTEXPORT_CLIPLIST_ERROR(63),
        DIRECTEXPORT_CHECK_ERROR(64),
        DIRECTEXPORT_FILEREADER_INIT_ERROR(65),
        DIRECTEXPORT_FILEWRITER_INIT_ERROR(66),
        DIRECTEXPORT_DEC_INIT_ERROR(67),
        DIRECTEXPORT_DEC_INIT_SURFACE_ERROR(68),
        DIRECTEXPORT_DEC_DECODE_ERROR(69),
        DIRECTEXPORT_ENC_INIT_ERROR(70),
        DIRECTEXPORT_ENC_ENCODE_ERROR(71),
        DIRECTEXPORT_ENC_INPUT_SURFACE_ERROR(72),
        DIRECTEXPORT_ENC_FUNCTION_ERROR(73),
        DIRECTEXPORT_ENC_DSI_DIFF_ERROR(74),
        DIRECTEXPORT_ENC_FRAME_CONVERT_ERROR(75),
        DIRECTEXPORT_RENDER_INIT_ERROR(76),
        DIRECTEXPORT_WRITER_WRITE_ERROR(77),
        DIRECTEXPORT_WRITER_UNKNOWN_ERROR(78),

        FASTPREVIEW_USER_CANCEL(79),
        FASTPREVIEW_CLIPLIST_ERROR(80),
        FASTPREVIEW_FIND_CLIP_ERROR(81),
        FASTPREVIEW_FIND_READER_ERROR(82),
        FASTPREVIEW_VIDEO_RENDERER_ERROR(83),
        FASTPREVIEW_DEC_INIT_SURFACE_ERROR(84),

        HW_NOT_ENOUGH_MEMORY(85),
        EXPORT_USER_CANCEL(86),

        FASTPREVIEW_DEC_INIT_ERROR(87),
        FASTPREVIEW_FILEREADER_INIT_ERROR(88),
        FASTPREVIEW_TIME_ERROR(89),
        FASTPREVIEW_RENDER_INIT_ERROR(90),
        FASTPREVIEW_OUTPUTSURFACE_INIT_ERROR(91),
        FASTPREVIEW_BUSY(92),

        CODEC_DECODE(93, "Codec decode failed", 0/*R.string.engine_error_codec_init_failed*/),
        RENDERER_AUDIO(94),
        UNSUPPORT_AUDIO_SAMPLINGRATE(95),

        IMAGE_PROCESS(0x00001001),
        SET_TIME_IGNORED("Set time ignored", 0/*		R.string.error_settime_ignored	*/),
        SET_TIME_CANCELED("Set time canceled", 0/*	R.string.error_settime_canceled	 */),
        CAPTURE_FAILED("Capture failed", 0/*R.string.error_capture_failed	*/),
        SOURCE_FILE_NOT_FOUND(),
        TRANSCODING_ABORTED(),
        DESTINATION_FILE_ALREADY_EXISTS(),
        TEMP_FILE_ALREADY_EXISTS(),
        NO_INSTANCE_AVAILABLE(),
        EXPORT_NO_SUCCESS(),
        PLAY_SUPERCEEDED(),
        WRAPPER_BUSY(),
        NOT_READY_TO_PLAY(),
        SEEKING_LOCKED(),
        NO_PROJECT_LOADED(),
        ALREADY_EXPORTING(),
        EMPTY_PROJECT(),
        MISSING_RESOURCES(),
        EXPORT_UNEXPECTED_STOP(),
        RENAME_FAIL(),
        CAPTURE_FAIL_ENOSPC("Not enough space", 0/*R.string.fail_enospc*/),
        CAPTURE_FAIL_OTHER("Capture failed", 0/*R.string.capture_fail_other*/),
        CAPTURE_FAIL_SCANNING("Media scanner failed", 0/*R.string.capture_fail_other*/),
        UNRECOGNIZED_ERROR_CODE("Unrecognized error code", 0/*R.string.error_unrecognized_code*/),
        EDITOR_INSTANCE_DESTROYED(),
        FILE_MISSING(),
        FAST_PREVIEW_IGNORED();


        private final boolean mFromEngine;
        private final int mValue;
        private final String mDescription;
        private final int mDescriptionRsrcId;

        public boolean isError() {
            return (this != NONE);
        }

        ErrorCode(int value) {
            mValue = value;
            mDescription = null;
            mDescriptionRsrcId = 0;
            mFromEngine = true;
        }

        ErrorCode(int value, String description, int rsrcid) {
            mValue = value;
            mDescription = description;
            mDescriptionRsrcId = rsrcid;
            mFromEngine = true;
        }

        ErrorCode(String description, int rsrcid) {
            mValue = 0;
            mDescription = description;
            mDescriptionRsrcId = rsrcid;
            mFromEngine = false;
        }

        ErrorCode() {
            mValue = 0;
            mDescription = null;
            mDescriptionRsrcId = 0;
            mFromEngine = false;
        }

        public int getValue() {
            return mValue;
        }

        public String getDescription() {
            if (mDescription == null)
                return this.name();
            else
                return mDescription;
        }

        public String getLocalizedDescription(Context context) {
            if (mDescriptionRsrcId != 0) {
                return context.getResources().getString(mDescriptionRsrcId);
            }
            if (mDescription == null)
                return this.name();
            else
                return mDescription;
        }

        public static ErrorCode fromValue(int value) {
            for (ErrorCode v : ErrorCode.values()) {
                if (v.mFromEngine && v.getValue() == value)
                    return v;
            }
            if (LL.E) Log.e(LOG_TAG, "Unrecognized error code : " + value);
            return ErrorCode.UNRECOGNIZED_ERROR_CODE;
        }

        @Override
        public Exception getException() {
            return null;
        }

        @Override
        public String getMessage() {
            return getDescription();
        }

        @Override
        public String getLocalizedMessage(Context context) {
            return getLocalizedDescription(context);
        }

        @Override
        public String toString() {
            if (this == NONE) {
                return "NONE(0)";
            } else if (mValue != 0) {
                return name() + "(" + mValue + ")";
            } else {
                return name();
            }
        }
    }

}
