/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.android.camera2basic;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.media.MediaScannerConnection;
import android.opengl.GLSurfaceView;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.util.Size;
import android.util.SparseIntArray;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.anigifsdk.nexAniGifSDK;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

public class Camera2BasicFragment extends Fragment
        implements View.OnClickListener, ActivityCompat.OnRequestPermissionsResultCallback, DialogInterface.OnDismissListener {
    /**
     * Conversion from screen rotation to JPEG orientation.
     */
    private static final SparseIntArray ORIENTATIONS = new SparseIntArray();
    private static final int REQUEST_CAMERA_PERMISSION = 1;
    private static final String FRAGMENT_DIALOG = "dialog";

    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }

    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "Camera2BasicFragment";

    /**
     * Camera state: Showing camera preview.
     */
    private static final int STATE_PREVIEW = 0;

    /**
     * Camera state: Waiting for the focus to be locked.
     */
    private static final int STATE_WAITING_LOCK = 1;

    /**
     * Camera state: Waiting for the exposure to be precapture state.
     */
    private static final int STATE_WAITING_PRECAPTURE = 2;

    /**
     * Camera state: Waiting for the exposure state to be something other than precapture.
     */
    private static final int STATE_WAITING_NON_PRECAPTURE = 3;

    /**
     * Camera state: Picture was taken.
     */
    private static final int STATE_PICTURE_TAKEN = 4;

    /**
     * Max preview width that is guaranteed by Camera2 API
     */
    private static final int MAX_PREVIEW_WIDTH = 1920;

    /**
     * Max preview height that is guaranteed by Camera2 API
     */
    private static final int MAX_PREVIEW_HEIGHT = 1080;

    /**
     * {@link TextureView.SurfaceTextureListener} handles several lifecycle events on a
     * {@link TextureView}.
     */
    private final TextureView.SurfaceTextureListener mSurfaceTextureListener
            = new TextureView.SurfaceTextureListener() {

        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture texture, int width, int height) {
            openCamera(width, height);
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture texture, int width, int height) {
            configureTransform(width, height);
        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture texture) {
            return true;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture texture) {
        }

    };

    /**
     * ID of the current {@link CameraDevice}.
     */
    private String mCameraId;

    /**
     * An {@link AutoFitTextureView} for camera preview.
     */
    private AutoFitTextureView mTextureView;

    /**
     * A {@link CameraCaptureSession } for camera preview.
     */
    private CameraCaptureSession mCaptureSession;

    /**
     * A reference to the opened {@link CameraDevice}.
     */
    private CameraDevice mCameraDevice;

    /**
     * The {@link android.util.Size} of camera preview.
     */
    private Size mPreviewSize;

    /**
     * {@link CameraDevice.StateCallback} is called when {@link CameraDevice} changes its state.
     */
    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            // This method is called when the camera is opened.  We start camera preview here.
            mCameraOpenCloseLock.release();
            mCameraDevice = cameraDevice;
            createCameraPreviewSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int error) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
            Activity activity = getActivity();
            if (null != activity) {
                activity.finish();
            }
        }

    };

    private HandlerThread mSaveGiFThread;
    private Handler mSaveGiFHandler;
    /**
     * An additional thread for running tasks that shouldn't block the UI.
     */
    private HandlerThread mBackgroundThread;

    /**
     * A {@link Handler} for running tasks in the background.
     */
    private Handler mBackgroundHandler;

    /**
     * An {@link ImageReader} that handles still image capture.
     */
    private ImageReader mImageReader;

    /**
     * This is the output file for our picture.
     */
    private File mFile;
    private FileOutputStream mFOS;


    private int captureCount;
    private boolean captureEnd;
    private long lastCaptureTimeStamp;

    public static byte[] imageToYUV420(Image image) {
        ByteBuffer buffer;
        int rowStride;
        int pixelStride;
        int width = image.getWidth();
        int height = image.getHeight();
        int offset = 0;

        Image.Plane[] planes = image.getPlanes();
        byte[] data = new byte[image.getWidth() * image.getHeight() * ImageFormat.getBitsPerPixel(ImageFormat.YUV_420_888) / 8];
        byte[] rowData = new byte[planes[0].getRowStride()];

        for (int i = 0; i < planes.length; i++) {
            buffer = planes[i].getBuffer();
            rowStride = planes[i].getRowStride();
            pixelStride = planes[i].getPixelStride();
            int w = (i == 0) ? width : width / 2;
            int h = (i == 0) ? height : height / 2;
            for (int row = 0; row < h; row++) {
                int bytesPerPixel = ImageFormat.getBitsPerPixel(ImageFormat.YUV_420_888) / 8;
                if (pixelStride == bytesPerPixel) {
                    int length = w * bytesPerPixel;
                    buffer.get(data, offset, length);

                    if (h - row != 1) {
                        buffer.position(buffer.position() + rowStride - length);
                    }
                    offset += length;
                } else {


                    if (h - row == 1) {
                        buffer.get(rowData, 0, width - pixelStride + 1);
                    } else {
                        buffer.get(rowData, 0, rowStride);
                    }

                    for (int col = 0; col < w; col++) {
                        data[offset++] = rowData[col * pixelStride];
                    }
                }
            }
        }
        return data;
    }

    ProgressDialog progressDialog;


    /**
     * This a callback object for the {@link ImageReader}. "onImageAvailable" will be called when a
     * still image is ready to be saved.
     */
    private final ImageReader.OnImageAvailableListener mOnImageAvailableListener
            = new ImageReader.OnImageAvailableListener() {

        @Override
        public void onImageAvailable(ImageReader reader) {
            //mBackgroundHandler.post(new ImageSaver(reader.acquireNextImage(), mFile));
            Image img = null;
            try {
                img = reader.acquireLatestImage();

                if (img != null) {
                    if (captureCount >= SettingDialogFragment.getValues().mFrameCount) {
                        img.close();
                        mBackgroundHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                try {
                                    mFOS.close();
                                } catch (IOException e) {
                                    e.printStackTrace();
                                }
                            }
                        });

                        if (!captureEnd) {
                            captureEnd = true;

                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    progressDialog = new ProgressDialog(getActivity());
                                    progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                                    progressDialog.setMessage("exporting gif...");
                                    progressDialog.show();
                                }
                            });

                            mSaveGiFHandler.post(new Runnable() {
                                @Override
                                public void run() {

                                    AniGifSave();
                                }
                            });

                            mSaveGiFHandler.post(new Runnable() {
                                @Override
                                public void run() {
                                    getActivity().runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            progressDialog.dismiss();
                                            stopSaveGiFThread();
                                        }
                                    });
                                }
                            });

                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    createCameraPreviewSession();

                                }
                            });
                        }
                        return;
                    }

                    long currentTime = img.getTimestamp();
                    if (lastCaptureTimeStamp != 0) {
                        if (currentTime < (lastCaptureTimeStamp + 60000000)) {
                            //skip
                            Log.d("exportImageFormat", "skip time=" + currentTime);
                            img.close();
                            return;
                        } else {
                            lastCaptureTimeStamp = currentTime;
                        }
                    } else {
                        lastCaptureTimeStamp = currentTime;
                    }

                    captureCount++;
                    mBackgroundHandler.post(new dumpYUV(img, mFOS, captureCount, mCaptureImageOrientation));

                } else {
                    Log.d("exportImageFormat", "Latest image is null. Ignore it.");
                    return;
                }

            } catch (Exception e) {
                Log.d("exportImageFormat", "Exception");
                if (img != null)
                    img.close();
                e.printStackTrace();
            }
        }

    };

    /**
     * {@link CaptureRequest.Builder} for the camera preview
     */
    private CaptureRequest.Builder mPreviewRequestBuilder;

    /**
     * {@link CaptureRequest} generated by {@link #mPreviewRequestBuilder}
     */
    private CaptureRequest mPreviewRequest;

    /**
     * The current state of camera state for taking pictures.
     *
     * @see #mCaptureCallback
     */
    private int mState = STATE_PREVIEW;

    /**
     * A {@link Semaphore} to prevent the app from exiting before closing the camera.
     */
    private Semaphore mCameraOpenCloseLock = new Semaphore(1);

    /**
     * Whether the current camera device supports Flash or not.
     */
    private boolean mFlashSupported;

    /**
     * Orientation of the camera sensor
     */
    private int mSensorOrientation;

    /**
     * A {@link CameraCaptureSession.CaptureCallback} that handles events related to JPEG capture.
     */
    private CameraCaptureSession.CaptureCallback mCaptureCallback
            = new CameraCaptureSession.CaptureCallback() {

        private void process(CaptureResult result) {
            switch (mState) {
                case STATE_PREVIEW: {
                    // We have nothing to do when the camera preview is working normally.
                    break;
                }
                case STATE_WAITING_LOCK: {
                    Integer afState = result.get(CaptureResult.CONTROL_AF_STATE);
                    if (afState == null) {
                        captureStillPicture();
                    } else if (CaptureResult.CONTROL_AF_STATE_FOCUSED_LOCKED == afState ||
                            CaptureResult.CONTROL_AF_STATE_NOT_FOCUSED_LOCKED == afState) {
                        // CONTROL_AE_STATE can be null on some devices
                        Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                        if (aeState == null ||
                                aeState == CaptureResult.CONTROL_AE_STATE_CONVERGED) {
                            mState = STATE_PICTURE_TAKEN;
                            captureStillPicture();
                        } else {
                            runPrecaptureSequence();
                        }
                    }
                    break;
                }
                case STATE_WAITING_PRECAPTURE: {
                    // CONTROL_AE_STATE can be null on some devices
                    Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                    if (aeState == null ||
                            aeState == CaptureResult.CONTROL_AE_STATE_PRECAPTURE ||
                            aeState == CaptureRequest.CONTROL_AE_STATE_FLASH_REQUIRED) {
                        mState = STATE_WAITING_NON_PRECAPTURE;
                    }
                    break;
                }
                case STATE_WAITING_NON_PRECAPTURE: {
                    // CONTROL_AE_STATE can be null on some devices
                    Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                    if (aeState == null || aeState != CaptureResult.CONTROL_AE_STATE_PRECAPTURE) {
                        mState = STATE_PICTURE_TAKEN;
                        captureStillPicture();
                    }
                    break;
                }
            }
        }

        @Override
        public void onCaptureProgressed(@NonNull CameraCaptureSession session,
                                        @NonNull CaptureRequest request,
                                        @NonNull CaptureResult partialResult) {
            process(partialResult);
        }

        @Override
        public void onCaptureCompleted(@NonNull CameraCaptureSession session,
                                       @NonNull CaptureRequest request,
                                       @NonNull TotalCaptureResult result) {
            process(result);
        }

    };

    /**
     * Shows a {@link Toast} on the UI thread.
     *
     * @param text The message to show
     */
    private void showToast(final String text) {
        final Activity activity = getActivity();
        if (activity != null) {
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(activity, text, Toast.LENGTH_SHORT).show();
                }
            });
        }
    }

    private void setResult(final String text){
        final Activity activity = getActivity();
        if (activity != null) {
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mResult.setText(text);
                }
            });
        }

    }

    /**
     * Given {@code choices} of {@code Size}s supported by a camera, choose the smallest one that
     * is at least as large as the respective texture view size, and that is at most as large as the
     * respective max size, and whose aspect ratio matches with the specified value. If such size
     * doesn't exist, choose the largest one that is at most as large as the respective max size,
     * and whose aspect ratio matches with the specified value.
     *
     * @param choices           The list of sizes that the camera supports for the intended output
     *                          class
     * @param textureViewWidth  The width of the texture view relative to sensor coordinate
     * @param textureViewHeight The height of the texture view relative to sensor coordinate
     * @param maxWidth          The maximum width that can be chosen
     * @param maxHeight         The maximum height that can be chosen
     * @param aspectRatio       The aspect ratio
     * @return The optimal {@code Size}, or an arbitrary one if none were big enough
     */
    private static Size chooseOptimalSize(Size[] choices, int textureViewWidth,
                                          int textureViewHeight, int maxWidth, int maxHeight, Size aspectRatio) {
        Log.d(TAG, "chooseOptimalSize textureView=(" + textureViewWidth + "X" + textureViewHeight + "), max=(" + maxWidth + "X" + maxHeight + "), aspect=(" + aspectRatio.getWidth() + "X" + aspectRatio.getHeight() + ")");
        // Collect the supported resolutions that are at least as big as the preview Surface
        List<Size> bigEnough = new ArrayList<>();
        // Collect the supported resolutions that are smaller than the preview Surface
        List<Size> notBigEnough = new ArrayList<>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            if (option.getWidth() <= maxWidth && option.getHeight() <= maxHeight &&
                    option.getHeight() == option.getWidth() * h / w) {
                if (option.getWidth() >= textureViewWidth &&
                        option.getHeight() >= textureViewHeight) {
                    bigEnough.add(option);
                } else {
                    notBigEnough.add(option);
                }
            }
        }

        // Pick the smallest of those big enough. If there is no one big enough, pick the
        // largest of those not big enough.
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else if (notBigEnough.size() > 0) {
            return Collections.max(notBigEnough, new CompareSizesByArea());
        } else {
            Log.e(TAG, "Couldn't find any suitable preview size");
            return choices[0];
        }
    }

    public static Camera2BasicFragment newInstance() {
        return new Camera2BasicFragment();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_camera2_basic, container, false);
    }
    TextView mResult;
    @Override
    public void onViewCreated(final View view, Bundle savedInstanceState) {
        view.findViewById(R.id.picture).setOnClickListener(this);
        view.findViewById(R.id.info).setOnClickListener(this);
        mTextureView = (AutoFitTextureView) view.findViewById(R.id.texture);
        mResult = (TextView)view.findViewById(R.id.textView_result);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mFile = new File(getActivity().getExternalFilesDir(null), "dump.yuv");
    }

    @Override
    public void onResume() {
        super.onResume();
        startBackgroundThread();

        // When the screen is turned off and turned back on, the SurfaceTexture is already
        // available, and "onSurfaceTextureAvailable" will not be called. In that case, we can open
        // a camera and start preview from here (otherwise, we wait until the surface is ready in
        // the SurfaceTextureListener).
        if (mTextureView.isAvailable()) {
            openCamera(mTextureView.getWidth(), mTextureView.getHeight());
        } else {
            mTextureView.setSurfaceTextureListener(mSurfaceTextureListener);
        }
    }

    @Override
    public void onPause() {
        closeCamera();
        stopBackgroundThread();
        super.onPause();
    }

    private void requestCameraPermission() {
        if (shouldShowRequestPermissionRationale(Manifest.permission.CAMERA)) {
            new ConfirmationDialog().show(getChildFragmentManager(), FRAGMENT_DIALOG);
        } else {
            requestPermissions(new String[]{Manifest.permission.CAMERA}, REQUEST_CAMERA_PERMISSION);
        }
    }

    private void requestStoragePermission() {
        requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_CAMERA_PERMISSION);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        if (requestCode == REQUEST_CAMERA_PERMISSION) {
            if (grantResults.length != 1 || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                ErrorDialog.newInstance(getString(R.string.request_permission))
                        .show(getChildFragmentManager(), FRAGMENT_DIALOG);
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    private void cameraInfo() {
        if (SettingDialogFragment.getValues().frontCameraResolution != null) {
            return;
        }

        if (SettingDialogFragment.getValues().backCameraResolution != null) {
            return;
        }

        Activity activity = getActivity();
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics characteristics
                        = manager.getCameraCharacteristics(cameraId);
                Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);

                if (facing == null) {
                    continue;
                }

                if (facing == CameraCharacteristics.LENS_FACING_FRONT) {
                    Log.d("CameraInfo", "Front Camera");
                } else if (facing == CameraCharacteristics.LENS_FACING_BACK) {
                    Log.d("CameraInfo", "Back Camera");
                }

                StreamConfigurationMap map = characteristics.get(
                        CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                if (map == null) {
                    continue;
                }

                if (facing == CameraCharacteristics.LENS_FACING_FRONT) {
                    SettingDialogFragment.getValues().frontCameraResolution = map.getOutputSizes(SurfaceTexture.class);
                } else if (facing == CameraCharacteristics.LENS_FACING_BACK) {
                    SettingDialogFragment.getValues().backCameraResolution = map.getOutputSizes(SurfaceTexture.class);
                }

                for (Size si : map.getOutputSizes(SurfaceTexture.class)) {
                    Log.d("CameraInfo", "(" + si.getWidth() + "X" + si.getHeight() + ")");
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Sets up member variables related to camera.
     *
     * @param width  The width of available size for camera preview
     * @param height The height of available size for camera preview
     */
    @SuppressWarnings("SuspiciousNameCombination")
    private void setUpCameraOutputs(int width, int height) {
        Activity activity = getActivity();
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics characteristics
                        = manager.getCameraCharacteristics(cameraId);

                // We don't use a front facing camera in this sample.
                Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);

                if (SettingDialogFragment.getValues().mFrontCamera) {
                    if (facing != null && facing == CameraCharacteristics.LENS_FACING_BACK) {
                        continue;
                    }
                } else {
                    if (facing != null && facing == CameraCharacteristics.LENS_FACING_FRONT) {
                        continue;
                    }
                }

                StreamConfigurationMap map = characteristics.get(
                        CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                if (map == null) {
                    continue;
                }

                // For still image captures, we use the largest available size.
                /*
                Size largest = Collections.max(
                        Arrays.asList(map.getOutputSizes(ImageFormat.JPEG)),
                        new CompareSizesByArea());
                mImageReader = ImageReader.newInstance(largest.getWidth(), largest.getHeight(),
                        ImageFormat.JPEG, 2);
                mImageReader.setOnImageAvailableListener(
                        mOnImageAvailableListener, mBackgroundHandler);
*/
                Size largest = Collections.max(
                        Arrays.asList(map.getOutputSizes(ImageFormat.JPEG)),
                        new CompareSizesByArea());

                // Find out if we need to swap dimension to get the preview size relative to sensor
                // coordinate.
                int displayRotation = activity.getWindowManager().getDefaultDisplay().getRotation();
                //noinspection ConstantConditions
                mSensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
                boolean swappedDimensions = false;
                switch (displayRotation) {
                    case Surface.ROTATION_0:
                    case Surface.ROTATION_180:
                        if (mSensorOrientation == 90 || mSensorOrientation == 270) {
                            swappedDimensions = true;
                        }
                        break;
                    case Surface.ROTATION_90:
                    case Surface.ROTATION_270:
                        if (mSensorOrientation == 0 || mSensorOrientation == 180) {
                            swappedDimensions = true;
                        }
                        break;
                    default:
                        Log.e(TAG, "Display rotation is invalid: " + displayRotation);
                }

                Point displaySize = new Point();
                activity.getWindowManager().getDefaultDisplay().getSize(displaySize);
                int rotatedPreviewWidth = width;
                int rotatedPreviewHeight = height;
                int maxPreviewWidth = displaySize.x;
                int maxPreviewHeight = displaySize.y;

                if (swappedDimensions) {
                    rotatedPreviewWidth = height;
                    rotatedPreviewHeight = width;
                    maxPreviewWidth = displaySize.y;
                    maxPreviewHeight = displaySize.x;
                }

                if (maxPreviewWidth > MAX_PREVIEW_WIDTH) {
                    maxPreviewWidth = MAX_PREVIEW_WIDTH;
                }

                if (maxPreviewHeight > MAX_PREVIEW_HEIGHT) {
                    maxPreviewHeight = MAX_PREVIEW_HEIGHT;
                }

                if (SettingDialogFragment.getValues().mPreviewWidth == 0 || SettingDialogFragment.getValues().mPreviewHeight == 0) {
                    // Danger, W.R.! Attempting to use too large a preview size could  exceed the camera
                    // bus' bandwidth limitation, resulting in gorgeous previews but the storage of
                    // garbage capture data.
                    mPreviewSize = chooseOptimalSize(map.getOutputSizes(SurfaceTexture.class),
                            rotatedPreviewWidth, rotatedPreviewHeight, maxPreviewWidth,
                            maxPreviewHeight, largest);
                    SettingDialogFragment.getValues().mPreviewWidth = mPreviewSize.getWidth();
                    SettingDialogFragment.getValues().mPreviewHeight = mPreviewSize.getHeight();
                } else {
                    mPreviewSize = new Size(SettingDialogFragment.getValues().mPreviewWidth, SettingDialogFragment.getValues().mPreviewHeight);
                }


                mImageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(),
                        ImageFormat.YUV_420_888, 5);
                mImageReader.setOnImageAvailableListener(
                        mOnImageAvailableListener, mBackgroundHandler);


                // We fit the aspect ratio of TextureView to the size of preview we picked.
                int orientation = getResources().getConfiguration().orientation;
                if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
                    mTextureView.setAspectRatio(
                            mPreviewSize.getWidth(), mPreviewSize.getHeight());
                } else {
                    mTextureView.setAspectRatio(
                            mPreviewSize.getHeight(), mPreviewSize.getWidth());
                }

                // Check if the flash is supported.
                Boolean available = characteristics.get(CameraCharacteristics.FLASH_INFO_AVAILABLE);
                mFlashSupported = available == null ? false : available;

                mCameraId = cameraId;
                return;
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            // Currently an NPE is thrown when the Camera2API is used but not supported on the
            // device this code runs.
            ErrorDialog.newInstance(getString(R.string.camera_error))
                    .show(getChildFragmentManager(), FRAGMENT_DIALOG);
        }
    }

    /**
     * Opens the camera specified by {@link Camera2BasicFragment#mCameraId}.
     */
    private void openCamera(int width, int height) {
        if (ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            requestCameraPermission();
            return;
        }
        setUpCameraOutputs(width, height);
        configureTransform(width, height);
        Activity activity = getActivity();
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            manager.openCamera(mCameraId, mStateCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }
    }

    /**
     * Closes the current {@link CameraDevice}.
     */
    private void closeCamera() {
        try {
            mCameraOpenCloseLock.acquire();
            if (null != mCaptureSession) {
                mCaptureSession.close();
                mCaptureSession = null;
            }
            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }
            if (null != mImageReader) {
                mImageReader.close();
                mImageReader = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraOpenCloseLock.release();
        }
    }

    /**
     * Starts a background thread and its {@link Handler}.
     */
    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("CameraBackground");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    /**
     * Stops the background thread and its {@link Handler}.
     */
    private void stopBackgroundThread() {
        mBackgroundThread.quitSafely();
        try {
            mBackgroundThread.join();
            mBackgroundThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void startSaveGiFThread() {
        mSaveGiFThread = new HandlerThread("GIFSave");
        mSaveGiFThread.start();
        mSaveGiFHandler = new Handler(mSaveGiFThread.getLooper());
    }

    /**
     * Stops the background thread and its {@link Handler}.
     */
    private void stopSaveGiFThread() {
        mSaveGiFThread.quitSafely();
        try {
            mSaveGiFThread.join();
            mSaveGiFThread = null;
            mSaveGiFHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }


    /**
     * Creates a new {@link CameraCaptureSession} for camera preview.
     */
    private void createCameraPreviewSession() {
        if (null == mCameraDevice || !mTextureView.isAvailable() || null == mPreviewSize) {
            return;
        }

        try {
            closePreviewSession();
            GLSurfaceView glSurfaceView;

            SurfaceTexture texture = mTextureView.getSurfaceTexture();
            assert texture != null;

            // We configure the size of default buffer to be the size of camera preview we want.
            texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());

            // This is the output Surface we need to start preview.
            Surface surface = new Surface(texture);

            // We set up a CaptureRequest.Builder with the output Surface.
            mPreviewRequestBuilder
                    = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewRequestBuilder.addTarget(surface);

            // Here, we create a CameraCaptureSession for camera preview.
            mCameraDevice.createCaptureSession(Arrays.asList(surface, mImageReader.getSurface()),
                    new CameraCaptureSession.StateCallback() {

                        @Override
                        public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
                            // The camera is already closed
                            if (null == mCameraDevice) {
                                return;
                            }

                            // When the session is ready, we start displaying the preview.
                            mCaptureSession = cameraCaptureSession;
                            try {
                                // Auto focus should be continuous for camera preview.
                                mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                                // Flash is automatically enabled when necessary.
                                setAutoFlash(mPreviewRequestBuilder);

                                // Finally, we start displaying the camera preview.
                                mPreviewRequest = mPreviewRequestBuilder.build();
                                mCaptureSession.setRepeatingRequest(mPreviewRequest,
                                        null, mBackgroundHandler);
                            } catch (CameraAccessException e) {
                                e.printStackTrace();
                            }
                        }

                        @Override
                        public void onConfigureFailed(
                                @NonNull CameraCaptureSession cameraCaptureSession) {
                            showToast("Failed");
                        }
                    }, null
            );
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Configures the necessary {@link android.graphics.Matrix} transformation to `mTextureView`.
     * This method should be called after the camera preview size is determined in
     * setUpCameraOutputs and also the size of `mTextureView` is fixed.
     *
     * @param viewWidth  The width of `mTextureView`
     * @param viewHeight The height of `mTextureView`
     */
    private void configureTransform(int viewWidth, int viewHeight) {
        Activity activity = getActivity();
        if (null == mTextureView || null == mPreviewSize || null == activity) {
            return;
        }
        int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        Matrix matrix = new Matrix();
        RectF viewRect = new RectF(0, 0, viewWidth, viewHeight);
        RectF bufferRect = new RectF(0, 0, mPreviewSize.getHeight(), mPreviewSize.getWidth());
        float centerX = viewRect.centerX();
        float centerY = viewRect.centerY();
        if (Surface.ROTATION_90 == rotation || Surface.ROTATION_270 == rotation) {
            bufferRect.offset(centerX - bufferRect.centerX(), centerY - bufferRect.centerY());
            matrix.setRectToRect(viewRect, bufferRect, Matrix.ScaleToFit.FILL);
            float scale = Math.max(
                    (float) viewHeight / mPreviewSize.getHeight(),
                    (float) viewWidth / mPreviewSize.getWidth());
            matrix.postScale(scale, scale, centerX, centerY);
            matrix.postRotate(90 * (rotation - 2), centerX, centerY);
        } else if (Surface.ROTATION_180 == rotation) {
            matrix.postRotate(180, centerX, centerY);
        }
        mTextureView.setTransform(matrix);
    }

    /**
     * Initiate a still image capture.
     */
    private void takePicture() {
        startSaveGiFThread();
        captureStillPicture();
        //lockFocus();
    }

    /**
     * Lock the focus as the first step for a still image capture.
     */
    private void lockFocus() {
        try {
            // This is how to tell the camera to lock focus.
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER,
                    CameraMetadata.CONTROL_AF_TRIGGER_START);
            // Tell #mCaptureCallback to wait for the lock.
            mState = STATE_WAITING_LOCK;
            mCaptureSession.capture(mPreviewRequestBuilder.build(), mCaptureCallback,
                    mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Run the precapture sequence for capturing a still image. This method should be called when
     * we get a response in {@link #mCaptureCallback} from {@link #lockFocus()}.
     */
    private void runPrecaptureSequence() {
        try {
            // This is how to tell the camera to trigger.
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                    CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
            // Tell #mCaptureCallback to wait for the precapture sequence to be set.
            mState = STATE_WAITING_PRECAPTURE;
            mCaptureSession.capture(mPreviewRequestBuilder.build(), mCaptureCallback,
                    mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private int mCaptureImageOrientation = 0;
    /**
     * Capture a still picture. This method should be called when we get a response in
     * {@link #mCaptureCallback} from both {@link #lockFocus()}.
     */
    private void captureStillPicture() {
        if (null == mCameraDevice || !mTextureView.isAvailable() || null == mPreviewSize) {
            return;
        }

        int rotation = getActivity().getWindowManager().getDefaultDisplay().getRotation();

        mCaptureImageOrientation = getOrientation(rotation);

        try {
            closePreviewSession();

            SurfaceTexture texture = mTextureView.getSurfaceTexture();
            assert texture != null;
            texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
            Surface previewSurface = new Surface(texture);

            // This is the CaptureRequest.Builder that we use to take a picture.
            mPreviewRequestBuilder =
                    mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
            mPreviewRequestBuilder.addTarget(previewSurface);
            mPreviewRequestBuilder.addTarget(mImageReader.getSurface());

            mCameraDevice.createCaptureSession(Arrays.asList(previewSurface, mImageReader.getSurface()),
                    new CameraCaptureSession.StateCallback() {

                        @Override
                        public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
                            // The camera is already closed
                            if (null == mCameraDevice) {
                                return;
                            }

                            try {
                                mFile = new File(getActivity().getExternalFilesDir(null), "dump_" + mPreviewSize.getWidth() + "X" + mPreviewSize.getHeight() + ".yuv");
                                mFOS = new FileOutputStream(mFile);
                            } catch (FileNotFoundException e) {
                                e.printStackTrace();
                            }
                            captureEnd = false;
                            captureCount = 0;
                            lastCaptureTimeStamp = 0;
                            // When the session is ready, we start displaying the preview.
                            mCaptureSession = cameraCaptureSession;
                            try {
                                // Auto focus should be continuous for camera preview.
                                mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                                        CaptureRequest.CONTROL_MODE_AUTO);

                                // Finally, we start displaying the camera preview.
                                mPreviewRequest = mPreviewRequestBuilder.build();
                                mCaptureSession.setRepeatingRequest(mPreviewRequest,
                                        null, mBackgroundHandler);
                            } catch (CameraAccessException e) {
                                e.printStackTrace();
                            }
                        }

                        @Override
                        public void onConfigureFailed(
                                @NonNull CameraCaptureSession cameraCaptureSession) {
                            showToast("Failed");
                        }
                    }, null
            );
/*
            // Use the same AE and AF modes as the preview.
            captureBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                    CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            setAutoFlash(captureBuilder);

            // Orientation
            int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, getOrientation(rotation));

            CameraCaptureSession.CaptureCallback CaptureCallback
                    = new CameraCaptureSession.CaptureCallback() {

                @Override
                public void onCaptureCompleted(@NonNull CameraCaptureSession session,
                                               @NonNull CaptureRequest request,
                                               @NonNull TotalCaptureResult result) {
                    showToast("Saved: " + mFile);
                    Log.d(TAG, mFile.toString());
                    unlockFocus();
                }
            };

            mCaptureSession.stopRepeating();
            mCaptureSession.abortCaptures();
            mCaptureSession.capture(captureBuilder.build(), CaptureCallback, mBackgroundHandler);
            */
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Retrieves the JPEG orientation from the specified screen rotation.
     *
     * @param rotation The screen rotation.
     * @return The JPEG orientation (one of 0, 90, 270, and 360)
     */
    private int getOrientation(int rotation) {
        // Sensor orientation is 90 for most devices, or 270 for some devices (eg. Nexus 5X)
        // We have to take that into account and rotate JPEG properly.
        // For devices with orientation of 90, we simply return our mapping from ORIENTATIONS.
        // For devices with orientation of 270, we need to rotate the JPEG 180 degrees.
        return (ORIENTATIONS.get(rotation) + mSensorOrientation + 270) % 360;
    }

    /**
     * Unlock the focus. This method should be called when still image capture sequence is
     * finished.
     */
    private void unlockFocus() {
        try {
            // Reset the auto-focus trigger
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER,
                    CameraMetadata.CONTROL_AF_TRIGGER_CANCEL);
            setAutoFlash(mPreviewRequestBuilder);
            mCaptureSession.capture(mPreviewRequestBuilder.build(), mCaptureCallback,
                    mBackgroundHandler);
            // After this, the camera will go back to the normal state of preview.
            mState = STATE_PREVIEW;
            mCaptureSession.setRepeatingRequest(mPreviewRequest, mCaptureCallback,
                    mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.picture: {
                takePicture();
                break;
            }
            case R.id.info: {
                showDialog();
                /*
                Activity activity = getActivity();
                if (null != activity) {
                    new AlertDialog.Builder(activity)
                            .setMessage(R.string.intro_message)
                            .setPositiveButton(android.R.string.ok, null)
                            .show();
                }
                */
                break;
            }
        }
    }

    private void setAutoFlash(CaptureRequest.Builder requestBuilder) {
        if (mFlashSupported) {
            requestBuilder.set(CaptureRequest.CONTROL_AE_MODE,
                    CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);
        }
    }

    private static class dumpYUV implements Runnable {
        private final Image mImage;
        private final FileOutputStream mFOS;
        private final int captureCount;
        private final int orientation;
        dumpYUV(Image mImage, FileOutputStream mFOS, int captureCount, int orientation) {
            this.mImage = mImage;
            this.mFOS = mFOS;
            this.captureCount = captureCount;
            this.orientation = orientation;
        }

        @Override
        public void run() {
            if (mFOS == null) {
                return;
            }
            Image.Plane[] planes = mImage.getPlanes();
            if (planes[0].getBuffer() != null) {
                //onCaptureFail(ErrorCode.UNKNOWN);


                int width = mImage.getWidth();
                int height = mImage.getHeight();

                int rowStride = planes[0].getRowStride();
                int pixelStride = planes[0].getPixelStride();

                byte[] yuv420 = imageToYUV420(mImage);

                byte[] temp = null;

                if( orientation == 90 ){
                    temp = YUVUtils.rotateYUV420Degree90(yuv420,width,height);
                }else if( orientation == 180 ){
                    temp = YUVUtils.rotateYUV420Degree180(yuv420,width,height);
                }else if( orientation == 270 ){
                    temp = YUVUtils.rotateYUV420Degree270(yuv420,width,height);
                }

                if( temp != null )
                    yuv420 = temp;

                Log.d("exportImageFormat", "[" + captureCount + "]captureCountonImageAvailable(" + mImage.getTimestamp() + ") width=" + width + ", height=" + height + ", rowStride=" + rowStride + ", pixelStride=" + pixelStride + ", format=" + mImage.getFormat());
                Log.d("exportImageFormat", "yuv420 size = " + yuv420.length+" ,orientation="+orientation);

                try {
                    mFOS.write(yuv420);
                } catch (IOException e) {
                    e.printStackTrace();
                }

                mImage.close();
            }
        }
    }

    /**
     * Saves a JPEG {@link Image} into the specified {@link File}.
     */
    private static class ImageSaver implements Runnable {

        /**
         * The JPEG image
         */
        private final Image mImage;
        /**
         * The file we save the image into.
         */
        private final File mFile;

        ImageSaver(Image image, File file) {
            mImage = image;
            mFile = file;
        }

        @Override
        public void run() {
            ByteBuffer buffer = mImage.getPlanes()[0].getBuffer();
            byte[] bytes = new byte[buffer.remaining()];
            buffer.get(bytes);
            FileOutputStream output = null;
            try {
                output = new FileOutputStream(mFile);
                output.write(bytes);
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                mImage.close();
                if (null != output) {
                    try {
                        output.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

    }

    /**
     * Compares two {@code Size}s based on their areas.
     */
    static class CompareSizesByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                    (long) rhs.getWidth() * rhs.getHeight());
        }

    }


    /**
     * Shows an error message dialog.
     */
    public static class ErrorDialog extends DialogFragment {

        private static final String ARG_MESSAGE = "message";

        public static ErrorDialog newInstance(String message) {
            ErrorDialog dialog = new ErrorDialog();
            Bundle args = new Bundle();
            args.putString(ARG_MESSAGE, message);
            dialog.setArguments(args);
            return dialog;
        }

        @NonNull
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Activity activity = getActivity();
            return new AlertDialog.Builder(activity)
                    .setMessage(getArguments().getString(ARG_MESSAGE))
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i) {
                            activity.finish();
                        }
                    })
                    .create();
        }

    }

    /**
     * Shows OK/Cancel confirmation dialog about camera permission.
     */
    public static class ConfirmationDialog extends DialogFragment {

        @NonNull
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Fragment parent = getParentFragment();
            return new AlertDialog.Builder(getActivity())
                    .setMessage(R.string.request_permission)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            parent.requestPermissions(new String[]{Manifest.permission.CAMERA},
                                    REQUEST_CAMERA_PERMISSION);
                        }
                    })
                    .setNegativeButton(android.R.string.cancel,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    Activity activity = parent.getActivity();
                                    if (activity != null) {
                                        activity.finish();
                                    }
                                }
                            })
                    .create();
        }
    }

    private void closePreviewSession() {
        if (mCaptureSession != null) {
            mCaptureSession.close();
            mCaptureSession = null;
        }
    }

    private void AniGifSave() {
        //ProgressDialog dlg = new ProgressDialog(getActivity());
        long start = System.currentTimeMillis();
        int width = mPreviewSize.getWidth();
        int height = mPreviewSize.getHeight();

        if( mCaptureImageOrientation == 90 || mCaptureImageOrientation == 270 ){
            width = mPreviewSize.getHeight();
            height = mPreviewSize.getWidth();
        }

        String dumpFile = mFile.getAbsolutePath();

        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));

        String outputFile = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "nex_" + width + "X" + height + "_sf" + SettingDialogFragment.getValues().mSampleFactor + "_fc" + SettingDialogFragment.getValues().mFrameCount +"_"+export_time+ ".gif";
        nexAniGifSDK aniGifSDK = nexAniGifSDK.createInstance();

        aniGifSDK.setWidth(width);
        aniGifSDK.setHeight(height);

        aniGifSDK.setDelayMs(66);
        aniGifSDK.setMode(nexAniGifSDK.Mode.Quality);
        aniGifSDK.setSampleFactor(SettingDialogFragment.getValues().mSampleFactor);
        aniGifSDK.setUseRGB666(SettingDialogFragment.getValues().mRgb666);
        int processCount = Runtime.getRuntime().availableProcessors() * 2-2;
        if( processCount < Runtime.getRuntime().availableProcessors() )
        {
            processCount = Runtime.getRuntime().availableProcessors();
        }
        aniGifSDK.setCpuCoreCount(processCount);

        aniGifSDK.setup();
        aniGifSDK.encodeFileToFile(dumpFile, nexAniGifSDK.kFormat_YUV420, outputFile);
        //aniGifSDK.encodeFileToFile(dumpFile, nexAniGifSDK.kFormat_RGBA, outputFile);
        aniGifSDK.destroy();
        long osize = new File(outputFile).length()/1000;
        Log.d("GIF", "time=" + (System.currentTimeMillis() - start) + "(ms)");
        setResult(outputFile+", time=" + (System.currentTimeMillis() - start) + "(ms) ,size="+osize+"(KB)");

        mFile.delete(); // do comment if want to get the yuv dump file.
        MediaScannerConnection.scanFile(getActivity().getApplicationContext(), new String[]{outputFile}, null, null);

/*
        new AsyncTask<Void,Void,Void>(){
            ProgressDialog dlg = new ProgressDialog(getActivity());
            String outputFile;
            @Override
            protected void onPreExecute() {
                outputFile = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "nex_"+mPreviewSize.getWidth()+"X"+mPreviewSize.getHeight()+"_sf"+SettingDialogFragment.getValues().mSampleFactor+"_fc"+SettingDialogFragment.getValues().mFrameCount+".gif";
                dlg.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                dlg.setMessage("exporting "+outputFile);
                dlg.show();

                super.onPreExecute();
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                dlg.dismiss();
                Log.d(TAG, "gif saved: " + outputFile);
                super.onPostExecute(aVoid);
            }

            @Override
            protected Void doInBackground(Void... params) {
                long start = System.currentTimeMillis();
                String dumpFile = mFile.getAbsolutePath();

                nexAniGifSDK aniGifSDK = nexAniGifSDK.createInstance();
                aniGifSDK.setWidth(mPreviewSize.getWidth());
                aniGifSDK.setHeight(mPreviewSize.getHeight());
                aniGifSDK.setDelayMs(66);
                aniGifSDK.setMode(nexAniGifSDK.Mode.Quality);
                aniGifSDK.setSampleFactor(SettingDialogFragment.getValues().mSampleFactor);
                aniGifSDK.setCpuCoreCount(Runtime.getRuntime().availableProcessors());
                //aniGifSDK.setCpuCoreCount(1);
                aniGifSDK.setup();
                aniGifSDK.encodeFileToFile(dumpFile,nexAniGifSDK.kFormat_YUV420,outputFile);
                aniGifSDK.destroy();
                Log.d("GIF","time="+(System.currentTimeMillis()-start)+"(ms)");
                mFile.delete();
                return null;
            }
        }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
        */
    }

    public static class SettingValues {
        boolean mFrontCamera = true;
        int mPreviewWidth = 0;
        int mPreviewHeight = 0;
        int mSampleFactor = 10;
        int mFrameCount = 30;
        boolean mRgb666 = true;
        Size[] frontCameraResolution;
        Size[] backCameraResolution;
    }


    private void showDialog() {
        cameraInfo();

        closeCamera();
        stopBackgroundThread();

        DialogFragment settingDialog = SettingDialogFragment.newInstance();
        settingDialog.setTargetFragment(this, 1);
        settingDialog.show(getFragmentManager(), "dialog");
        //fm.executePendingTransactions();
        /*
        settingDialog.getDialog().setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialogInterface) {
                Log.d(TAG,"Setting dialog dismiss.");
            }
        });
        */
    }

    public static class SettingDialogFragment extends DialogFragment {
        private static final String KEY_SAVE_RATING_BAR_VALUE = "KEY_SAVE_RATING_BAR_VALUE";
        private RadioButton mFrontRadio;
        private RadioButton mBackRadio;
        private Spinner mResolutionSpinner;
        private SeekBar mSampleFactorSeekBar;
        private TextView mSampleFactorTextView;
        private SeekBar mFrameCountSeekBar;
        private TextView mFrameCountTextView;
        private RadioButton mRgb666Radio;
        private RadioButton mRgb888Radio;

        public static SettingValues mValues = new SettingValues();

        public static SettingDialogFragment newInstance() {
            SettingDialogFragment frag = new SettingDialogFragment();
            return frag;
        }

        public static SettingValues getValues() {
            return mValues;
        }


        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getActivity());

            View view = getActivity().getLayoutInflater().inflate(R.layout.fragment_setting_dialog, null);
            mFrontRadio = (RadioButton) view.findViewById(R.id.radioButton_front);
            mBackRadio = (RadioButton) view.findViewById(R.id.radioButton_back);
            mResolutionSpinner = (Spinner) view.findViewById(R.id.spinner_resolution);
            mSampleFactorSeekBar = (SeekBar) view.findViewById(R.id.seekBar_sf);
            mFrameCountSeekBar = (SeekBar) view.findViewById(R.id.seekBar_frameCount);
            mSampleFactorTextView = (TextView) view.findViewById(R.id.textView_sf);
            mFrameCountTextView = (TextView) view.findViewById(R.id.textView_frameCount);
            mRgb666Radio = (RadioButton) view.findViewById(R.id.radioButton_quant6);
            mRgb888Radio = (RadioButton) view.findViewById(R.id.radioButton_quant8);

            if (mValues.mFrontCamera) {
                mFrontRadio.setChecked(true);
                mBackRadio.setChecked(false);
                if (mValues.frontCameraResolution != null) {
                    ArrayList<String> spinnerArray = new ArrayList<String>();
                    int i = 0;
                    int current = 0;
                    for (Size size : mValues.frontCameraResolution) {
                        spinnerArray.add("" + size.getWidth() + " X " + size.getHeight());
                        if (size.getWidth() == mValues.mPreviewWidth && size.getHeight() == mValues.mPreviewHeight) {
                            current = i;
                        }
                        i++;
                    }
                    ArrayAdapter spinnerArrayAdapter = new ArrayAdapter(getActivity(),
                            android.R.layout.simple_spinner_dropdown_item,
                            spinnerArray);
                    mResolutionSpinner.setAdapter(spinnerArrayAdapter);
                    mResolutionSpinner.setSelection(current);
                }
            } else {
                mFrontRadio.setChecked(false);
                mBackRadio.setChecked(true);
                if (mValues.backCameraResolution != null) {
                    ArrayList<String> spinnerArray = new ArrayList<String>();
                    int i = 0;
                    int current = 0;
                    for (Size size : mValues.backCameraResolution) {
                        spinnerArray.add("" + size.getWidth() + " X " + size.getHeight());
                        if (size.getWidth() == mValues.mPreviewWidth && size.getHeight() == mValues.mPreviewHeight) {
                            current = i;
                        }
                        i++;
                    }
                    ArrayAdapter spinnerArrayAdapter = new ArrayAdapter(getActivity(),
                            android.R.layout.simple_spinner_dropdown_item,
                            spinnerArray);
                    mResolutionSpinner.setAdapter(spinnerArrayAdapter);
                    mResolutionSpinner.setSelection(current);
                }
            }

            mFrontRadio.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    if (b) {
                        if (mValues.frontCameraResolution != null) {
                            ArrayList<String> spinnerArray = new ArrayList<String>();
                            int i = 0;
                            int current = 0;
                            for (Size size : mValues.frontCameraResolution) {
                                spinnerArray.add("" + size.getWidth() + " X " + size.getHeight());
                                if (size.getWidth() == mValues.mPreviewWidth && size.getHeight() == mValues.mPreviewHeight) {
                                    current = i;
                                }
                                i++;
                            }
                            ArrayAdapter spinnerArrayAdapter = new ArrayAdapter(getActivity(),
                                    android.R.layout.simple_spinner_dropdown_item,
                                    spinnerArray);
                            mResolutionSpinner.setAdapter(spinnerArrayAdapter);
                            mResolutionSpinner.setSelection(current);
                        }
                    }
                }
            });

            mBackRadio.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    if (mValues.backCameraResolution != null) {
                        ArrayList<String> spinnerArray = new ArrayList<String>();
                        int i = 0;
                        int current = 0;
                        for (Size size : mValues.backCameraResolution) {
                            spinnerArray.add("" + size.getWidth() + " X " + size.getHeight());
                            if (size.getWidth() == mValues.mPreviewWidth && size.getHeight() == mValues.mPreviewHeight) {
                                current = i;
                            }
                            i++;
                        }
                        ArrayAdapter spinnerArrayAdapter = new ArrayAdapter(getActivity(),
                                android.R.layout.simple_spinner_dropdown_item,
                                spinnerArray);
                        mResolutionSpinner.setAdapter(spinnerArrayAdapter);
                        mResolutionSpinner.setSelection(current);
                    }
                }
            });

            mSampleFactorSeekBar.setMax(20);
            mSampleFactorSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                    mSampleFactorTextView.setText("" + (i * 10));
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {

                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {

                }
            });
            mSampleFactorSeekBar.setProgress(mValues.mSampleFactor / 10);

            mFrameCountSeekBar.setMax(60);
            mFrameCountSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                    mFrameCountTextView.setText("" + i);
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {

                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {

                }
            });
            mFrameCountSeekBar.setProgress(mValues.mFrameCount);

            if (mValues.mRgb666) {
                mRgb666Radio.setChecked(true);
                mRgb888Radio.setChecked(false);
            } else {
                mRgb666Radio.setChecked(false);
                mRgb888Radio.setChecked(true);
            }

            mRgb666Radio.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    if (b) {
                        mRgb666Radio.setChecked(true);
                        mRgb888Radio.setChecked(false);
                        mValues.mRgb666 = true;
                    }
                }
            });

            mRgb888Radio.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    if (b) {
                        mRgb666Radio.setChecked(false);
                        mRgb888Radio.setChecked(true);
                        mValues.mRgb666 = false;
                    }
                }
            });

            /*
            if (savedInstanceState != null) {
                if (savedInstanceState.containsKey(KEY_SAVE_RATING_BAR_VALUE)) {
                    mRatingBar.setRating(savedInstanceState.getFloat(KEY_SAVE_RATING_BAR_VALUE));
                }
            }
*/
            alertDialogBuilder.setView(view);
            alertDialogBuilder.setTitle("Setting");
            alertDialogBuilder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    //Toast.makeText(getActivity(), getString(R.string.dialog_positive_toast_message), Toast.LENGTH_SHORT).show();
                    mValues.mFrameCount = mFrameCountSeekBar.getProgress();
                    mValues.mSampleFactor = mSampleFactorSeekBar.getProgress() * 10;
                    mValues.mFrontCamera = mFrontRadio.isChecked();
                    int select = mResolutionSpinner.getSelectedItemPosition();
                    if (mValues.mFrontCamera) {
                        mValues.mPreviewWidth = mValues.frontCameraResolution[select].getWidth();
                        mValues.mPreviewHeight = mValues.frontCameraResolution[select].getHeight();
                    } else {
                        mValues.mPreviewWidth = mValues.backCameraResolution[select].getWidth();
                        mValues.mPreviewHeight = mValues.backCameraResolution[select].getHeight();
                    }
                    dialog.dismiss();

                }
            });
            alertDialogBuilder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    //Toast.makeText(getActivity(), getString(R.string.dialog_negative_toast_message), Toast.LENGTH_SHORT).show();
                    dialog.cancel();
                }
            });

            return alertDialogBuilder.create();
        }

        @Override
        public void onSaveInstanceState(Bundle outState) {
            //outState.putFloat(KEY_SAVE_RATING_BAR_VALUE, mRatingBar.getRating());
            super.onSaveInstanceState(outState);
        }

        @Override
        public void onDismiss(DialogInterface dialog) {
            super.onDismiss(dialog);
            Log.d(TAG, "Dialog onDismiss start");
            final Fragment parentFragment = getTargetFragment();

            if (parentFragment instanceof DialogInterface.OnDismissListener) {
                Log.d(TAG, "Dialog onDismiss send");
                ((DialogInterface.OnDismissListener) parentFragment).onDismiss(dialog);
            }
        }
    }

    @Override
    public void onDismiss(final DialogInterface dialog) {
        Log.d(TAG, "Dialog dismiss recv.");
        startBackgroundThread();

        // When the screen is turned off and turned back on, the SurfaceTexture is already
        // available, and "onSurfaceTextureAvailable" will not be called. In that case, we can open
        // a camera and start preview from here (otherwise, we wait until the surface is ready in
        // the SurfaceTextureListener).
        if (mTextureView.isAvailable()) {
            openCamera(mTextureView.getWidth(), mTextureView.getHeight());
        } else {
            mTextureView.setSurfaceTextureListener(mSurfaceTextureListener);
        }
    }
/*
    private int getRotation() {
        Display display = getWindowManager().getDefaultDisplay();
        int rotation = display.getRotation();
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0: degrees = 0; break;
            case Surface.ROTATION_90: degrees = 90; break;
            case Surface.ROTATION_180: degrees = 180; break;
            case Surface.ROTATION_270: degrees = 270; break;
        }
        int result = 0;
        if (cameraInfo.facing == CameraInfo.CAMERA_FACING_FRONT) {
            result = (cameraInfo.orientation + degrees) % 360;
            result = (360 - result) % 360;	// compensate the mirror
        } else {
            result = (cameraInfo.orientation - degrees + 360) % 360;
        }
        return result;
    }
    */
}
