package com.nexstreaming.kminternal.nexvideoeditor;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.util.Base64;
import android.content.res.Resources;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.editorwrapper.LookUpTable;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor.ErrorCode;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor.PlayState;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Locale;
import java.util.ArrayList;

public class NexEditorEventListener {
    private static final String TAG = "NexEditorEventHandler";
    private NexEditor mEditor = null;
    private NexEditorListener mUIListener = null;
    public AudioTrack mAudioTrack = null;
    public NexImageLoader mImage = null;
    private Handler uiThreadHandler = null;
    private PlayState mPlayState = null;
    private int mIgnoreUntilTag = 0;
    private boolean mIgnoreEvents = false;
    private AudioManager mAudioManager;
    private NexEditor.LayerRenderCallback mLayerRenderCallback;
    private LayerRenderer mLayerRenderer = new LayerRenderer();
    private boolean mSyncMode = false; //yoon
    private boolean watermark_flag = true;//jeff

    public void setSyncMode(boolean set) { //yoon
        mSyncMode = set;
    }

    public synchronized void ignoreEventsUntilTag(int iTag) {
        if (!mIgnoreEvents) {
            mIgnoreEvents = true;
            mIgnoreUntilTag = iTag;
        } else if (mIgnoreUntilTag < iTag) {
            mIgnoreUntilTag = iTag;
        }
    }

    private synchronized void reachedTag(int iTag) {
        if (mIgnoreEvents && iTag >= mIgnoreUntilTag) {
            mIgnoreEvents = false;
        }
    }

    public NexEditorEventListener(NexEditor editor, Context ctx, EffectResourceLoader resourceLoader, NexImageLoader.OverlayPathResolver overlayPathResolver) {
        if (ctx != null)
            mAudioManager = (AudioManager) ctx.getApplicationContext().getSystemService(Context.AUDIO_SERVICE);
        else
            mAudioManager = null;

        mEditor = editor;
        mImage = new NexImageLoader(ctx == null ? null : ctx.getResources(), resourceLoader, overlayPathResolver, editor.getJPEGMaxWidth(), editor.getJPEGMaxHeight(), editor.getJPEGMaxSize());
        uiThreadHandler = new Handler(Looper.getMainLooper());
    }

    public void setContext(Context ctx) {

        mImage.setResources(ctx == null ? null : ctx.getApplicationContext().getResources());

        if (ctx != null)
            mAudioManager = (AudioManager) ctx.getApplicationContext().getSystemService(Context.AUDIO_SERVICE);
        else
            mAudioManager = null;
    }

    public void setUIListener(NexEditorListener listener) {
        mUIListener = listener;
        if (mPlayState != null)
            mUIListener.onStateChange(PlayState.NONE, mPlayState);
    }

    @SuppressWarnings("unused") // called from native
    public int callbackCapture(final int iWidth, final int iHeight, final int iSize, final byte[] arrayData) {
        uiThreadHandler.post(new Runnable() {
            public void run() {
                if (mEditor != null) {
                    mEditor.onCaptureDone(iWidth, iHeight, iSize, arrayData, true);
                }
            }
        });
        return 0;
    }

    @SuppressWarnings("unused") // called from native
    public int callbackThumb(final int iMode, final int iTag, final int iTime, final int iWidth, final int iHeight, final int iCount, final int iTotal, final int iSize, final byte[] arrayData) {
        Log.d(TAG, "callbackThumb start iMode=" + iMode + ", iTime=" + iTime + ", iWidth=" + iWidth + ", iHeight=" + iHeight + ", iSize=" + iSize + ", tag=" + iTag);

        uiThreadHandler.post(new Runnable() {
            public void run() {
                if (mEditor != null) {
                    mEditor.onThumbDone(iMode, iTime, iWidth, iHeight, iSize, arrayData, iCount, iTotal, iTag);
                }
            }
        });
        Log.d(TAG, "salabara callbackThumb end");
        return 0;
    }

    @SuppressWarnings("unused") // called from native
    public int callbackHighLightIndex(final int iCount, final int[] arrayData) {
        Log.d(TAG, "callbackHighLightIndex start iCount=" + iCount);

        uiThreadHandler.post(new Runnable() {
            public void run() {
                if (mEditor != null) {
                    mEditor.onHighLightDone(iCount, arrayData);
                }
            }
        });
        Log.d(TAG, "callbackHighLightIndex end");

        return 0;
    }

    private class ImageWorker extends AsyncTask<String, Void, NexImage> {

        private String filename_;

        public void setFilename(String filename) {

            filename_ = filename;
        }

        @Override
        protected NexImage doInBackground(String... params) {

            NexImage image = mImage.openThemeImage(params[0]);

            if (null == image)
                return image;

            int width = image.getWidth();
            int height = image.getHeight();
            int loadedtype = image.getLoadedType();
            int[] pixels = new int[width * height];
            image.getPixels(pixels);
            if (mEditor != null) {
                mEditor.pushLoadedBitmapJ(params[0], pixels, width, height, loadedtype);
            }
            return image;
        }

        @Override
        protected void onPostExecute(NexImage result) {

            image_worker_list_.remove(this);
            if (LL.D)
                Log.d(TAG, String.format("Done:image thread queue length:%d", image_worker_list_.size()));
        }

        @Override
        protected void onCancelled(NexImage image) {

            if (mEditor != null && filename_ != null) {

                mEditor.removeBitmapJ(filename_);
            }
        }
    }

    ;

    private ArrayList<ImageWorker> image_worker_list_ = new ArrayList<ImageWorker>();

    @SuppressWarnings("unused") // called from native
    public NexImage callbackGetThemeImage(String strFilePath, int asyncmode) {
        if (asyncmode == 1 || asyncmode == 3) {

            if (asyncmode == 3) {

                for (ImageWorker item : image_worker_list_) {

                    item.cancel(false);
                }

                image_worker_list_.clear();
                if (LL.D)
                    Log.d(TAG, String.format("CLEAR:image thread queue length:%d", image_worker_list_.size()));
            }

            ImageWorker worker = new ImageWorker();
            worker.setFilename(strFilePath);
            image_worker_list_.add(worker);
            if (LL.D)
                Log.d(TAG, String.format("NEW:image thread queue length:%d asyncmode:%d", image_worker_list_.size(), asyncmode));
            worker.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, strFilePath);
            return null;
        }
        return mImage.openThemeImage(strFilePath);
    }

    @SuppressWarnings("unused") // called from native
    public int callbackCheckImageWorkDone() {
        return image_worker_list_.size();
    }

    @SuppressWarnings("unused") // called from native
    public byte[] callbackGetThemeFile(String path) {
        String untagged_path = path;
        String base_id;
        String rel_path;

        if (untagged_path.contains(".force_effect/"))
            untagged_path = untagged_path.replace(".force_effect/", "/");

        int delim_idx = untagged_path.indexOf('/');
        if (delim_idx >= 0) {
            base_id = untagged_path.substring(0, delim_idx);
            rel_path = untagged_path.substring(delim_idx + 1);
        } else {
            base_id = untagged_path;
            rel_path = "";
        }

        return mImage.callbackReadAssetItemFile(base_id, rel_path);
    }

    @SuppressWarnings("unused") // called from native
    public NexImage callbackGetThemeImageUsingResource(String strFilePath) {
        return mImage.openFile(strFilePath, 0);
    }

    @SuppressWarnings("unused") // called from native
    public NexImage callbackGetImageUsingFile(String strFilePath, int iThumb) {
        return mImage.openFile(strFilePath, iThumb);
    }

    /*
    public long callbackGetImageDirect( String strFilePath, Buffer buffer ) {
        return NexImage.getImageDirect( strFilePath, buffer );
    }
     */
    @SuppressWarnings("unused") // called from native
    public NexImage callbackGetImageUsingText(String strText) {
        return mImage.openFile(strText, 0);
    }

    @SuppressWarnings("unused") // called from native
    public void callbackReleaseImage() {
//		mImage.close();
    }

    @SuppressWarnings("unused") // called from native
    public AudioTrack callbackGetAudioTrack(int iSampleRate, int iChannels) {
        if (LL.D)
            Log.d(TAG, String.format("callbackGetAudioTrack(SampleRate(%d) Channel(%d)", iSampleRate, iChannels));
        if (mAudioTrack != null)
            mAudioTrack.release();

        int nChannelConfig = AudioFormat.CHANNEL_CONFIGURATION_MONO;
        if (iChannels != 1)
            nChannelConfig = AudioFormat.CHANNEL_CONFIGURATION_STEREO;

        int nMinBufferSize = AudioTrack.getMinBufferSize(iSampleRate, nChannelConfig, AudioFormat.ENCODING_PCM_16BIT);

        mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, iSampleRate, nChannelConfig, AudioFormat.ENCODING_PCM_16BIT, nMinBufferSize, AudioTrack.MODE_STREAM);
        return mAudioTrack;
    }

    @SuppressWarnings("unused") // called from native
    public void callbackReleaseAudioTrack() {
        if (LL.D) Log.d(TAG, String.format("callbackReleaseAudioTrack"));
        if (mAudioTrack != null) {
            mAudioTrack.release();
            mAudioTrack = null;
        }
    }

    @SuppressWarnings("unused") // called from native
    public AudioManager callbackGetAudioManager() {
        if (LL.D) Log.d(TAG, "callbackGetAudioManager " + mAudioManager);
        return mAudioManager;
    }

    private void dispatchStateChangeEvent(PlayState oldState, PlayState newState) {
        if (mPlayState != newState) {
            mPlayState = newState;
            if (mUIListener != null) {
                mUIListener.onStateChange(oldState, newState);
            }
            if (newState == PlayState.IDLE) {
                mEditor.onIdle();
            }

            // for mantis 5463 and 5493
            if (oldState == null || oldState == PlayState.NONE)
                return;

            switch (newState) {
                case RUN:
                case RECORD:
                    mEditor.onPlay(NexEditor.ErrorCode.NONE);
                    break;
                default:
                    mEditor.onStop(NexEditor.ErrorCode.NONE);
                    break;
            }
			
			/*
			if( oldState==PlayState.RUN && newState!=PlayState.RUN && mEditor!=null )
				mEditor.onStop(NexEditor.ErrorCode.NONE);
			else if( oldState!=PlayState.RUN && newState==PlayState.RUN && mEditor!=null )
				mEditor.onPlay(NexEditor.ErrorCode.NONE)
			*/
        }
    }

    @SuppressWarnings("unused") // called from native
    public int notifyEvent(int iEventType, int iParam1, int iParam2, int iParam3, int iParam4) {
        final int p1 = iParam1;
        final int p2 = iParam2;
        final int p3 = iParam3;
        final int p4 = iParam4;

        if (iEventType == NexEditorEventType.VIDEOEDITOR_EVENT_CMD_MARKER_CHECKED) {
            if (LL.D) Log.d(TAG, "REACHED MARKER " + p1);
            reachedTag(p1);
            uiThreadHandler.post(new Runnable() {
                public void run() {
                    if (mEditor != null) {
                        mEditor.onCommandMarker(NexEditor.ErrorCode.NONE, p1);
                    }
                }
            });
        }

        if (mIgnoreEvents) {
            if (LL.D)
                Log.d(TAG, "IGNORING EVENT iEventType=" + iEventType + " (awaiting tag " + mIgnoreUntilTag + ")");
            return 0;
        }
        if (LL.D)
            Log.d(TAG, "EEVENT(" + iEventType + "; " + p1 + "," + p2 + "," + p3 + "," + p4 + ")");

        switch (iEventType) {
            case NexEditorEventType.VIDEOEDITOR_EVENT_STATE_CHANGE: {
                if (mSyncMode) {
                    NexEditor.PlayState oldState = NexEditor.PlayState.fromValue(p1);
                    NexEditor.PlayState newState = NexEditor.PlayState.fromValue(p2);
                    if (newState == PlayState.RESUME) {
                        newState = PlayState.RECORD;
                    }
                    dispatchStateChangeEvent(oldState, newState);
                } else {
                    uiThreadHandler.post(new Runnable() {
                        public void run() {
                            NexEditor.PlayState oldState = NexEditor.PlayState.fromValue(p1);
                            NexEditor.PlayState newState = NexEditor.PlayState.fromValue(p2);
                            if (newState == PlayState.RESUME) {
                                newState = PlayState.RECORD;
                            }
                            dispatchStateChangeEvent(oldState, newState);
                        }
                    });
                }
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_CURRENTTIME: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (!EditorGlobal.PROJECT_NAME.equals("KineMaster") && mEditor != null && mEditor.onExportProgress(p1)) {
                            return;
                        }
                        if (mUIListener != null) {
                            mUIListener.onTimeChange(p1);
                        }
                        if (mEditor != null) {
                            mEditor.onTimeChange(p1);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_ADDCLIP_DONE: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            // uiParam4 : added clip type  (0=None, 1=Image, 2=Text, 3=Audio, 4=Video)
                            mEditor.onAddClipDone(p1, p2, p3, p4);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_MOVECLIP_DONE: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onMoveClipDone(p1, p2);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_DELETECLIP_DONE: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onDeleteClipDone(p1);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_EFFECTCHANGE_DONE: {
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_ENCODE_PROJECT: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (!EditorGlobal.PROJECT_NAME.equals("KineMaster") && mEditor != null && mEditor.onExportDone()) {
                            dispatchStateChangeEvent(mPlayState, PlayState.IDLE);
                            return;
                        }

                        if (mUIListener != null) {
                            mUIListener.onEncodingDone(ErrorCode.NONE);
                        }
                        if (mEditor != null) {
                            mEditor.onEncodingDone(ErrorCode.NONE);
                        }
                        dispatchStateChangeEvent(mPlayState, PlayState.IDLE);
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_ENCODE_ERROR: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (!EditorGlobal.PROJECT_NAME.equals("KineMaster") && mEditor != null && mEditor.onExportFail(NexEditor.ErrorCode.fromValue(p1))) {
                            dispatchStateChangeEvent(mPlayState, PlayState.IDLE);
                            return;
                        }

                        if (mUIListener != null) {
                            mUIListener.onEncodingDone(NexEditor.ErrorCode.fromValue(p1));
                        }
                        if (mEditor != null) {
                            mEditor.onEncodingDone(NexEditor.ErrorCode.fromValue(p1));
                        }
                        dispatchStateChangeEvent(mPlayState, PlayState.IDLE);
                    }
                });
                break;
            }

            // Before
            // p2 == 1 or error
            // p1 == seek time

            // After
            // p1 == 0 : success, other : failed
            // p2 == 1 : ignore,
            // p3 == set time
            // p4 == result frame time
            case NexEditorEventType.VIDEOEDITOR_EVENT_SETTIME_DONE: {
                if (LL.D)
                    Log.d(TAG, "VIDEOEDITOR_EVENT_SETTIME_DONE (" + p1 + "," + p2 + "," + p3 + "," + p4 + ")");
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            if (p2 == 1) {
                                mEditor.onSetTimeIgnored();
                            } else {
                                mEditor.onSetTimeDone(p3, p4);
                            }
                        }
                        if (mUIListener != null) {
                            if (p2 == 1) {
                                mUIListener.onSetTimeIgnored();
                            } else {
                                if (p1 != 0) {
                                    mUIListener.onSetTimeFail(NexEditor.ErrorCode.fromValue(p1));
                                } else {
                                    mUIListener.onSetTimeDone(p3);
                                }
                            }
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_ASYNC_LOADLIST: {
                if (LL.D) Log.d(TAG, "VIDEOEDITOR_EVENT_ASYNC_LOADLIST(" + p1 + ")");
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onAsyncLoadListDone(p1);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_PLAY_END: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onPlayEnd();
                        }
                        dispatchStateChangeEvent(mPlayState, PlayState.IDLE);
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_PLAY_ERROR: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            NexEditor.ErrorCode errcode = NexEditor.ErrorCode.fromValue(p1);
                            mEditor.onPlayError(errcode);
                            mEditor.stop();
                            mEditor.onPlay(errcode);
                        }
                        if (mUIListener != null) {
                            mUIListener.onPlayFail(NexEditor.ErrorCode.fromValue(p1), p2);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_VIDEO_STARTED: {
                if (LL.I) Log.i(TAG, String.format("[nexEditorEventHandler.java] VIDEO_STARTED "));
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onPlayStart();
                        }
                        if (mUIListener != null) {
                            mUIListener.onPlayStart();
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_CAPTURE_DONE: {
                if (LL.I)
                    Log.i(TAG, String.format("[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_CAPTURE_DONE p1=" + p1));
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onCaptureFail(NexEditor.ErrorCode.fromValue(p1));
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_PREPARE_CLIP_LOADING: {
                if (LL.I)
                    Log.i(TAG, String.format("[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_PREPARE_CLIP_LOADING p1=" + p1));
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_TRANSCODING_PROGRESS: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onTranscodingProgress(p1, p2, p3);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_TRANSCODING_DONE: {
                if (LL.D)
                    Log.d(TAG, "VIDEOEDITOR_EVENT_TRANSCODING_DONE delivery p1=" + p1 + " p2=" + p2);
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onTranscodingDone(NexEditor.ErrorCode.fromValue(p1), p2);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_MAKE_REVERSE_PROGRESS: {
                Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_MAKE_REVERSE_PROGRESS p1= " + p1 + ", p2= " + p2 + ", p3: " + p3);
                uiThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onReverseProgress(p1, 100);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_MAKE_REVERSE_DONE: {
                Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_MAKE_REVERSE_DONE p1= " + p1 + ", p2= " + p2);
                uiThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mUIListener != null) {
                            if (mUIListener != null) {
                                mUIListener.onReverseDone(NexEditor.ErrorCode.fromValue(p1));
                            }
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_GETCLIPINFO_DONE: {
                // p1 : error code
                // p2 : user tag
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (LL.D)
                            Log.d(TAG, "VIDEOEDITOR_EVENT_GETCLIPINFO_DONE delivery p1=" + p1 + " p2=" + p2);
                        if (mEditor != null) {
                            if (LL.D)
                                Log.d(TAG, "VIDEOEDITOR_EVENT_GETCLIPINFO_DONE deliver to editor");
                            mEditor.onGetClipInfoDone(NexEditor.ErrorCode.fromValue(p1), p2);
                        }
                        if (mUIListener != null) {
                            if (LL.D)
                                Log.d(TAG, "VIDEOEDITOR_EVENT_GETCLIPINFO_DONE deliver to UI listener");
                            mUIListener.onClipInfoDone();
                        }
                    }
                });

                if (LL.I)
                    Log.i(TAG, String.format("[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_GETCLIPINFO_DONE p1=" + p1));
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_GETCLIPINFO_STOP_DONE: {
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            if (LL.D)
                                Log.d(TAG, "VIDEOEDITOR_EVENT_GETCLIPINFO_STOP_DONE deliver to editor p1=" + p1 + ", p2=" + p2);
                            mEditor.onGetClipInfoDone(ErrorCode.GETCLIPINFO_USER_CANCEL, p2);
                        }
                        if (mUIListener != null) {
                            if (LL.D)
                                Log.d(TAG, "VIDEOEDITOR_EVENT_GETCLIPINFO_STOP_DONE deliver to UI listener");
                            //mEditor.onGetClipInfoDone(ErrorCode.GETCLIPINFO_USER_CANCEL,p2);
                            mUIListener.onClipInfoDone();
                        }

                    }
                });

                if (LL.I)
                    Log.i(TAG, String.format("[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_GETCLIPINFO_STOP_DONE p1=" + p1));
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_FAST_OPTION_PREVIEW_DONE: {
                if (LL.D) Log.d(TAG, "VIDEOEDITOR_EVENT_FAST_OPTION_PREVIEW_DONE");
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onFastOptionPreviewDone(NexEditor.ErrorCode.fromValue(p1));
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_CHECK_DIRECT_EXPORT: {
                if (LL.I)
                    Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_CHECK_DIRECT_EXPORT p1=" + p1);
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onCheckDirectExport(ErrorCode.fromValue(p1), p2);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_DIRECT_EXPORT_DONE: {
                if (LL.I)
                    Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_DIRECT_EXPORT_DONE p1=" + p1);
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onEncodingDone(NexEditor.ErrorCode.fromValue(p1));
                        }
                        if (mEditor != null) {
                            mEditor.onEncodingDone(NexEditor.ErrorCode.fromValue(p1));
                        }
                        dispatchStateChangeEvent(mPlayState, PlayState.IDLE);
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_DIRECT_EXPORT_PROGRESS: {
                if (LL.I)
                    Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_DIRECT_EXPORT_PROGRESS=" + p1);
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_HIGHLIGHT_THUMBNAIL_PROGRESS: {
                if (LL.I)
                    Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_HIGHLIGHT_THUMBNAIL_PROGRESS=" + p1 + ", p2=" + p2);
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onProgressThumbnailCaching(p1, p2);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_FAST_PREVIEW_START_DONE: {
                Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_FAST_PREVIEW_START_DONE p1= " + p1 + ", p2= " + p2 + ", p3: " + p3);
                uiThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onFastPreviewStartDone(NexEditor.ErrorCode.fromValue(p1), p2, p3);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_FAST_PREVIEW_STOP_DONE: {
                Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_FAST_PREVIEW_STOP_DONE p1= " + p1);
                uiThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onFastPreviewStopDone(NexEditor.ErrorCode.fromValue(p1));
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_FAST_PREVIEW_TIME_DONE: {
                Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_FAST_PREVIEW_TIME_DONE p1= " + p1);
                uiThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onFastPreviewTimeDone(NexEditor.ErrorCode.fromValue(p1));
                        }
                    }
                });
                break;
            }

            case NexEditorEventType.VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_PROGRESS_INDEX: {
                if (LL.I)
                    Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_PROGRESS_INDEX=" + p1 + ", p2=" + p2);
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onProgressThumbnailCaching(p1, p2);
                        }
                    }
                });
                break;
            }

            case NexEditorEventType.VIDEOEDITOR_EVENT_PREVIEW_PEAKMETER: {
                if (LL.I)
                    Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_PREVIEW_PEAKMETER cts=" + p1 + ", left=" + p2 + ", right=" + p3);
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mUIListener != null) {
                            mUIListener.onPreviewPeakMeter(p1, p2, p3);
                        }
                    }
                });
                break;
            }
            case NexEditorEventType.VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_DONE: {
                if (LL.I)
                    Log.i(TAG, "[nexEditorEventHandler.java] VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_DONE errcode=" + p1);
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onHighLightError(p1);
                        }
                    }
                });
                break;
            }

            case NexEditorEventType.KM_EVENT_DIAGNOSIS: {
                if (LL.D)
                    Log.d(TAG, "[nexEditorEventHandler.java] KM_EVENT_DIAGNOSIS eventType: " + p1 + ", params: { " + p2 + ", " + p3 + ", " + p4 + " }");
                uiThreadHandler.post(new Runnable() {
                    public void run() {
                        if (mEditor != null) {
                            mEditor.onDiagnosticEvent(p1, p2, p3, p4);
                        }
                    }
                });
                break;
            }
            default: {
                if (LL.I)
                    Log.i(TAG, String.format("[nexEditorEventHandler.java] not implement event(%d) Param(%d %d %d %d) ", iEventType, iParam1, iParam2, iParam3, iParam4));
                break;
            }
        }
        // if(LL.I) Log.i(TAG, String.format("[nexEditorEventHandler.java] event(%d) Param(%d %d %d %d) ", iEventType, iParam1, iParam2, iParam3, iParam4));

        return 0;
    }

    @SuppressWarnings("unused") // called from native
    public int notifyError(int iEventType, int iResult, int iParam1, int iParam2) {
        if (LL.I)
            Log.i(TAG, String.format("[nexEditorEventHandler.java] event(%d) Param(%d %d) ", iEventType, iParam1, iParam2));
        return 0;
    }

    private static class LayerRenderFlag {
        public static final int EXPORT = 0x00000001;
    }

    private static final String TAG_ThemeImage = "[ThemeImage]";

    @SuppressWarnings("unused") // called from native
    public String getAssetResourceKey(String resourcePath) {
        String untagged_path = resourcePath.substring(TAG_ThemeImage.length());
        String base_id;
        String rel_path;
        String key;
        String mid = "16v9";
        int delim_idx = untagged_path.indexOf('/');
        if (delim_idx >= 0) {
            base_id = untagged_path.substring(0, delim_idx);
            rel_path = untagged_path.substring(delim_idx + 1);
            ItemInfo info = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItemById(base_id);
            if (info == null) {
                key = resourcePath;
            } else {
                if (base_id.contains("9v16")) {
                    mid = "9v16";
                } else if (base_id.contains("2v1")) {
                    mid = "2v1";
                } else if (base_id.contains("1v2")) {
                    mid = "1v2";
                } else if (base_id.contains("1v1")) {
                    mid = "1v1";
                } else if (base_id.contains("4v3")) {
                    mid = "4v3";
                } else if (base_id.contains("3v4")) {
                    mid = "3v4";
                }
                key = TAG_ThemeImage + info.getAssetPackage().getAssetId() + "/" + mid + "/" + rel_path;
            }
        } else {
            key = resourcePath;

        }
        return key;

    }

    private int dataMode = 0;

    public void setWatermark(boolean flag) {
        chkdata();
        watermark_flag = flag;
    }

    private static Bitmap watermark_image_ = null;
    private static String watermark_target_language_ = "";

    private void chkdata() {
        if (dataMode == 0) {
            try {
                InputStream inputStream = KineMasterSingleTon.getApplicationInstance().getApplicationContext().getAssets().open(EditorGlobal.keep());
                MessageDigest md = null;

                md = MessageDigest.getInstance("SHA-256");

                byte[] dataBytes = new byte[4096];
                int nread = 0;
                while ((nread = inputStream.read(dataBytes)) != -1) {
                    md.update(dataBytes, 0, nread);
                }
                inputStream.close();
                byte[] mdbytes = md.digest();
                String str = Base64.encodeToString(mdbytes, Base64.DEFAULT);

                if (str.startsWith("5i/mnZqgIegSRcn19oeAQavHHw9HeyJZugRi3/4ASTY=")) {
                    dataMode = 2;
                    return;
                }

            } catch (IOException e) {
                e.printStackTrace();
            } catch (NoSuchAlgorithmException e) {
                e.printStackTrace();
            }
            dataMode = 1;
        }
    }

    private void drawWatermark() {
        if (dataMode != 2) {
            mLayerRenderer.fillRect(0xffff00ff, 0, 0, mLayerRenderer.getOutputWidth(), mLayerRenderer.getOutputHeight());
            return;
        }

        if (true == watermark_flag) {

            Locale locale = KineMasterSingleTon.getApplicationInstance().getApplicationContext().getResources().getConfiguration().locale;
            String language = locale.getLanguage();

            if (watermark_image_ == null || language != watermark_target_language_) {

                watermark_target_language_ = language;
                BitmapFactory.Options opts = new BitmapFactory.Options();
                opts.inScaled = false;
                try {
                    InputStream inputStream = KineMasterSingleTon.getApplicationInstance().getApplicationContext().getAssets().open(EditorGlobal.keep());
                    watermark_image_ = BitmapFactory.decodeStream(inputStream);
                    inputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                } finally {

                }

                //watermark_image_ = BitmapFactory.dec
            }

            if (watermark_image_ != null) {

                float ratio = mLayerRenderer.getOutputHeight() / 1080.0f;
                float width = watermark_image_.getWidth() * ratio;
                float height = watermark_image_.getHeight() * ratio;
                float pos_x = 1235.0f / 1280.0f * mLayerRenderer.getOutputWidth();
                float pos_y = 35.0f / 720.0f * mLayerRenderer.getOutputHeight();
                mLayerRenderer.drawBitmap(watermark_image_, pos_x - width, pos_y, pos_x, pos_y + height);
                return;
            }
        }

    }

    @SuppressWarnings("unused") // called from native
    public int callbackPrepareCustomLayer(int shader, int cts, int a_position, int a_texCoord, int u_mvp_matrix,
                                          int u_tex_matrix, int u_alpha, int u_realX, int u_realY, int u_colorconv, int u_textureSampler, int u_textureSampler_for_mask,
                                          int u_ambient_light, int u_diffuse_light, int u_specular_light, int flags, int screen_dimension_width, int screen_dimension_height) {
        if (mLayerRenderCallback == null) {
            return 1;
        }
        boolean isExport = ((flags & LayerRenderFlag.EXPORT) != 0);
        synchronized (mLayerRenderer){
            mLayerRenderer.setShaderAndParams(isExport);
            //int ws = 1;
            //int hs = 1;
            int w = NexEditor.getLayerWidth();
            int h = NexEditor.getLayerHeight();

            if (NexEditor.getLayerMode() == NexEditor.kLayerMode_ScreenDimension) {
                if (screen_dimension_width > screen_dimension_height) {
                    w = Math.round(720.0f * (float) screen_dimension_width / (float) screen_dimension_height);
                    h = 720;
                } else {
                    w = 720;
                    h = Math.round(720.0f * (float) screen_dimension_height / (float) screen_dimension_width);
                }

            }

            if (mEditor.getScreenMode() == mEditor.kScreenMode_horizonDual) {
                w *= 2;
            }



    /*
            if( mEditor.getAspectMode() == NexEditor.kAspectRatio_Mode_1v1 ) {
                w = 720 *ws;
                h = 720 *hs;
            }else if( mEditor.getAspectMode() == NexEditor.kAspectRatio_Mode_9v16  ) {
                w = 720 * ws;
                h = 1280 * hs;
            }else if( mEditor.getAspectMode() == NexEditor.kAspectRatio_Mode_2v1  ){
                w = 1440 *ws;
                h = 720 *hs;
            }else if( mEditor.getAspectMode() == NexEditor.kAspectRatio_Mode_1v2  ){
                w = 720 *ws;
                h = 1440 *hs;
            }else{
                w = 1280 *ws;
                h = 720 *hs;
            }
    */
            mLayerRenderer.setFrameDimensions(w, h);
            mLayerRenderer.setScreenDimensions(screen_dimension_width, screen_dimension_height);
            mLayerRenderer.setCurrentTime(cts);
            mLayerRenderer.preRender();
            mLayerRenderCallback.renderLayers(mLayerRenderer);
            drawWatermark();
            mLayerRenderer.postRender();
        }
        return 0;
    }

    public void setCustomRenderCallback(NexEditor.LayerRenderCallback layerRenderCallback) {
        mLayerRenderCallback = layerRenderCallback;
    }

    @SuppressWarnings("unused") // called from native
    public int getLutTextWithID(int lut_resource_id, int export_flag) {

        if (mEditor == null) {
            Log.d(TAG, "getLutTextWithID() engine is null");
            return 0;
        }

        LookUpTable tab = LookUpTable.getLookUpTable();
        if (tab == null) {
            Log.d(TAG, "getLutTextWithID() getLookUpTable is null");
            return 0;
        }
        LookUpTable.LUT lut = tab.getLUT(lut_resource_id);

        if (lut == null) {
            Log.d(TAG, "getLutTextWithID() lut is null");
            return 0;
        }

        return mLayerRenderer.getTexNameForBitmap(lut.getBitmap(), export_flag);
    }

    @SuppressWarnings("unused") // called from native
    public int getVignetteTexID(int export_flag) {
        LookUpTable tab = LookUpTable.getLookUpTable();
        if (tab == null) {
            Log.d(TAG, "getVignetteTexID() getLookUpTable is null");
            return 0;
        }
        Log.d(TAG, "getVignetteTexID() call... export_flag=" + export_flag);
        return mLayerRenderer.getTexNameForBitmap(tab.getBitmapForVignette(), export_flag);
    }

    public void destroyRenderer(){
        mLayerRenderer.destroy();
    }
}
