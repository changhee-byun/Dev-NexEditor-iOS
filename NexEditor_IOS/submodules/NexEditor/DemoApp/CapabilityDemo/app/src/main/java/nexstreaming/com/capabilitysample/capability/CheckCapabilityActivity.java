package nexstreaming.com.capabilitysample.capability;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.ClipData;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.nexstreaming.capability.util.ConvertFilepathUtil;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import nexstreaming.com.capabilitysample.R;

public class CheckCapabilityActivity extends AppCompatActivity {
    final static String TAG = "CheckCapabilityActivity";
    FrameLayout frameLayout;
    LinearLayout capabilityLayout;

    PowerManager.WakeLock wakeLock = null;

    Context mContext;
    Activity mActivity;

    private static final int EDITOR_DEMO_PERMISSION_REQUEST_STORAGE = 100;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContext = getApplicationContext();
        mActivity = CheckCapabilityActivity.this;

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        PowerManager pm = (PowerManager) getBaseContext().getSystemService(Context.POWER_SERVICE);
        wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE, CheckCapabilityActivity.class.getName());
        wakeLock.setReferenceCounted(false);
        wakeLock.acquire();

        setContentView(R.layout.activity_check_capability);

        frameLayout = (FrameLayout) findViewById(R.id.framelayout);

        capabilityLayout = new CheckCapabilityTestView(mContext, mActivity);
        frameLayout.addView(capabilityLayout);

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
            checkPermission();
    }

    @TargetApi(Build.VERSION_CODES.M)
    private void checkPermission() {
        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED
                || checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {

            // Should we show an explanation?
            if (shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                // Explain to the user why we need to write the permission.
                Toast.makeText(mContext, "Read/Write external storage", Toast.LENGTH_SHORT).show();
            }

            requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    EDITOR_DEMO_PERMISSION_REQUEST_STORAGE);
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        Log.d(TAG, "onConfigurationChanged");
        super.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {

            Log.d(TAG, "copy test start");
            ClipData clipData = data.getClipData();
            String filepath = null;

            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                Log.d(TAG, "Selected file = " + filepath);
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                    Log.d(TAG, "Selected file = " + filepath);
                }
            }
            try {
            MediaExtractor extractor = new MediaExtractor();

                extractor.setDataSource(filepath);

            for (int i = 0; i < extractor.getTrackCount(); i++) {
                MediaFormat tmp_format = extractor.getTrackFormat(i);
                String mime = tmp_format.getString(MediaFormat.KEY_MIME);
                if (mime.startsWith("video/")) {
                    extractor.selectTrack(i);
                    extractor.seekTo(0, MediaExtractor.SEEK_TO_CLOSEST_SYNC);
                    break;
                }
            }
            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                File orgfile = new File(filepath);
                File dstfile = new File(filepath+".temp");
                FileInputStream in = new FileInputStream(orgfile);
                FileOutputStream out = new FileOutputStream(dstfile);
                int len = (int)orgfile.length();
                int received = 0;
                byte[] bytes  = new byte[1024];
                long time = SystemClock.currentThreadTimeMillis();
                while (received != len) {
                    final int chunk = in.read(bytes);
                    out.write(bytes, 0, chunk);
                    if (chunk < 0) {
                        throw new IOException(
                                "I/O failure while receiving SDK controller data from socket.");
                    }
                    received += chunk;
                }
                Toast.makeText(getApplicationContext(), "copy time(ms):"+(SystemClock.currentThreadTimeMillis()-time), Toast.LENGTH_SHORT).show();

            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e){
                e.printStackTrace();
            }
        }

    }
}
