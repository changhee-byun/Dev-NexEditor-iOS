package com.nexstreaming.editordemo.activity;

import android.app.KeyguardManager;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.provider.MediaStore;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.fragment.kmSTSongsFragment;
import com.nexstreaming.editordemo.fragment.kmSTThemeFragment;
import com.nexstreaming.editordemo.utility.audioFocusHelper;
import com.nexstreaming.editordemo.utility.musicFocusable;
import com.nexstreaming.editordemo.utility.utilityCode;
import com.nexstreaming.editordemo.widget.SlidingTabLayout;

import java.io.File;
import java.io.IOException;

public class kmMusicListActivity extends ActionBarActivity implements MediaPlayer.OnCompletionListener,
        MediaPlayer.OnPreparedListener, musicFocusable,
        kmSTThemeFragment.OnSTThemeSelectedListener,
        kmSTSongsFragment.OnSTSongsSelectedListener {

    private Toolbar mToolbar;

    private ProgressBar mMiniPlayerProgressBar;
    private ImageView mMiniPlayerAlbum;
    private ImageButton mMiniPlayerButton;
    private TextView mMiniPlayerTitle;

    private String mMusicPath;
    private String mMusicTitle;
    private String mMusicArtist;


    // The volume we set the media player to when we lose audio focus, but are allowed to reduce
    // the volume instead of stopping playback.
    public static final float DUCK_VOLUME = 0.1f;

    // our AudioFocusHelper object, if it's available (it's available on SDK level >= 8)
    // If not available, this will be null. Always check for null before using!
    private audioFocusHelper mAudioFocusHelper = null;

    private MediaPlayer mMiniPlayer;

    private enum State {
        Preparing,
        Playing,
        Paused,
        Idle
    }

    private State mState = State.Idle;

    // do we have audio focus?
    private enum AudioFocus {
        NoFocusNoDuck,    // we don't have audio focus, and can't duck
        NoFocusCanDuck,   // we don't have focus, but can play at a low volume ("ducking")
        Focused           // we have full audio focus
    }
    private AudioFocus mAudioFocus = AudioFocus.NoFocusNoDuck;

    private boolean mIsLCDoff;
    private boolean mIsSecured;

    private int mEdittoolDataParcel;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mEdittoolDataParcel = getIntent().getIntExtra("EdittoolData", -1);

        setContentView(R.layout.activity_km_music_list);

        mToolbar = (Toolbar) findViewById(R.id.sound_toolbar);
        setSupportActionBar(mToolbar);

        android.support.v7.app.ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(getResources().getDrawable(R.mipmap.ic_arrow_back_white_24dp_noshadow));
        actionBar.setTitle("Soundtrack");

        // Get the ViewPager and set it's PagerAdapter so that it can display items
        ViewPager viewPager = (ViewPager) findViewById(R.id.viewpager);
        viewPager.setAdapter(new SoundtrackPagerAdapter(getSupportFragmentManager(),
                kmMusicListActivity.this));

        // Give the SlidingTabLayout the ViewPager
        SlidingTabLayout slidingTabLayout = (SlidingTabLayout) findViewById(R.id.sliding_tabs);
        // Center the tabs in the layout
        slidingTabLayout.setDistributeEvenly(true);
        // Customize tab color
        slidingTabLayout.setCustomTabColorizer(new SlidingTabLayout.TabColorizer() {
            @Override
            public int getIndicatorColor(int position) {
                return getResources().getColor(R.color.textColorNormal);
            }
        });
        slidingTabLayout.setViewPager(viewPager);

        mMiniPlayerTitle = (TextView) findViewById(R.id.player_title);
        mMiniPlayerAlbum = (ImageView) findViewById(R.id.player_icon);
        mMiniPlayerButton = (ImageButton) findViewById(R.id.player_button);
        mMiniPlayerButton.setBackground(null);
        mMiniPlayerButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                processTogglePlaybackRequest();
            }
        });
        mMiniPlayerProgressBar = (ProgressBar) findViewById(R.id.player_progress);
        mMiniPlayerProgressBar.setMax(1000);

        mAudioFocusHelper = new audioFocusHelper(getApplicationContext(), this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if(mMiniPlayer != null) {
            mMiniPlayer.reset();
            mMiniPlayer.release();
            mMiniPlayer = null;
        }
    }

    @Override
    protected void onPause() {
        super.onPause();

        KeyguardManager keyguardManager = (KeyguardManager) getSystemService(KEYGUARD_SERVICE);
        if(keyguardManager.inKeyguardRestrictedInputMode()) mIsSecured = true;

        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        boolean isScreenOn = powerManager.isScreenOn();
        if(!isScreenOn) {
            mIsLCDoff = true;
        } else {
            mIsLCDoff = false;
        }
        mMiniPlayerButton.setBackgroundResource(R.mipmap.ic_play_arrow_grey600_24dp);
        processPauseRequest();
    }

    @Override
    protected void onResume() {
        super.onResume();

        if(!mIsLCDoff && !mIsSecured) {
            mMiniPlayerButton.setBackgroundResource(R.mipmap.ic_pause_grey600_24dp);
            processPlayRequest();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_km_music_list, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        processStopRequest(true); // stop everything!

        switch(item.getItemId()) {
            case R.id.action_soundtrack:
            {
                mState = State.Idle;

                if(mMusicPath!=null) {
                    Intent intent = null;
                    if(mEdittoolDataParcel == -1) {
                        intent = new Intent(getBaseContext(), kmTransitionPreviewActivity.class);
                    } else {
                        intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                    }
                    intent.putExtra("soundtrack_path", mMusicPath);
                    intent.putExtra("album_title", mMusicTitle);
                    intent.putExtra("alubm_artist", mMusicArtist);
                    if(mMusicPath.contains("/baby_music.aac")
                        || mMusicPath.contains("/baseball_music.aac")
                        || mMusicPath.contains("/iloveyou_music.aac")
                        || mMusicPath.contains("/spring_music.aac")
                        || mMusicPath.contains("/themepark_music.aac")
                        || mMusicPath.contains("/whistling_and_fun.mp3")
                        ) {
                        setResult(1, intent);
                    } else {
                        setResult(2, intent);
                    }
                }

                finish();
                return true;
            }
            case android.R.id.home:
            {
                mState = State.Idle;

                finish();
                return true;
            }
        }
        return super.onOptionsItemSelected(item);
    }

    public class SoundtrackPagerAdapter extends FragmentPagerAdapter {

        final int PAGE_COUNT = 2;
        private String tabTitles[] = new String[] {"THEME", "SONGS"};
        private Context context;

        public SoundtrackPagerAdapter(FragmentManager fm, Context context) {
            super(fm);
            this.context = context;
        }

        @Override
        public Fragment getItem(int position) {
            if(position == 0) {
                return kmSTThemeFragment.newInstance();
            } else {
                return kmSTSongsFragment.newInstance();
            }
        }

        @Override
        public int getCount() {
            return PAGE_COUNT;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            return tabTitles[position];
        }
    }

    @Override
    public void onSThemeSelected(String title) {
        Uri uri = null;
        mMiniPlayerTitle.setText(title);
        switch(title) {
            case "baby":
                mMiniPlayerAlbum.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_baby));
                try {
                    utilityCode.raw2file(this, R.raw.baby_music, "baby_music.aac");
                    mMusicPath = getFilesDir().getAbsolutePath()+"/baby_music.aac";
                    uri = Uri.fromFile(new File(mMusicPath));
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
            case "baseball":
                mMiniPlayerAlbum.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_baseball));
                try {
                    utilityCode.raw2file(this, R.raw.baseball_music, "baseball_music.aac");
                    mMusicPath = getFilesDir().getAbsolutePath()+"/baseball_music.aac";
                    uri = Uri.fromFile(new File(mMusicPath));
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
            case "iloveyou":
                mMiniPlayerAlbum.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_iloveyou));
                try {
                    utilityCode.raw2file(this, R.raw.iloveyou_music, "iloveyou_music.aac");
                    mMusicPath = getFilesDir().getAbsolutePath()+"/iloveyou_music.aac";
                    uri = Uri.fromFile(new File(mMusicPath));
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
            case "spring":
                mMiniPlayerAlbum.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_spring));
                try {
                    utilityCode.raw2file(this, R.raw.spring_music, "spring_music.aac");
                    mMusicPath = getFilesDir().getAbsolutePath()+"/spring_music.aac";
                    uri = Uri.fromFile(new File(mMusicPath));
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
            case "themepark":
                mMiniPlayerAlbum.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_themepark));
                try {
                    utilityCode.raw2file(this, R.raw.themepark_music, "themepark_music.aac");
                    mMusicPath = getFilesDir().getAbsolutePath()+"/themepark_music.aac";
                    uri = Uri.fromFile(new File(mMusicPath));
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
            case "whistling and fun":
                mMiniPlayerAlbum.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp));
                try {
                    utilityCode.raw2file(this, R.raw.whistling_and_fun, "whistling_and_fun.mp3");
                    mMusicPath = getFilesDir().getAbsolutePath()+"/whistling_and_fun.mp3";
                    uri = Uri.fromFile(new File(mMusicPath));
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
        }
        if(uri != null) playSong(uri);
        mMiniPlayerButton.setBackgroundResource(R.mipmap.ic_pause_grey600_24dp);
    }

    @Override
    public void onSTSongsSelected(String path, String title, String artist, int albumid) {
        mMusicPath = path;
        mMusicTitle = title;
        mMusicArtist = artist;

        if(albumid != -1) {
            final Uri ART_CONTENT_URI = Uri.parse("content://media/external/audio/albumart");
            Uri albumArtUri = ContentUris.withAppendedId(ART_CONTENT_URI, albumid);
            try {
                Bitmap albumartbitmap = MediaStore.Images.Media.getBitmap(getContentResolver(), albumArtUri);
                mMiniPlayerAlbum.setImageBitmap(albumartbitmap);
            } catch (IOException e) {
                e.printStackTrace();
                mMiniPlayerAlbum.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp));
            }
        } else {
            mMiniPlayerAlbum.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp));
        }

        if(!title.isEmpty()) {
            mMiniPlayerTitle.setText(title);
        } else {
            mMiniPlayerTitle.setText("");
        }

        if(path != null) playSong(path);
        mMiniPlayerButton.setBackgroundResource(R.mipmap.ic_pause_grey600_24dp);
    }

    private void createMediaPlayer() {
        if(mMiniPlayer == null) {
            mMiniPlayer = new MediaPlayer();
            // Make sure the media player will acquire a wake-lock while playing. If we don't do
            // that, the CPU might go to sleep while the song is playing, causing playback to stop.
            //
            // Remember that to use this, we have to declare the android.permission.WAKE_LOCK
            // permission in AndroidManifest.xml.
            mMiniPlayer.setWakeMode(getApplicationContext(), PowerManager.PARTIAL_WAKE_LOCK);
            // we want the media player to notify us when it's ready preparing, and when it's done
            // playing:
            mMiniPlayer.setOnPreparedListener(this);
            mMiniPlayer.setOnCompletionListener(this);
        }
    }

    private void processTogglePlaybackRequest() {
        if (mState == State.Paused || mState == State.Idle) {
            mMiniPlayerButton.setBackgroundResource(R.mipmap.ic_pause_grey600_24dp);
            processPlayRequest();
        } else {
            mMiniPlayerButton.setBackgroundResource(R.mipmap.ic_play_arrow_grey600_24dp);
            processPauseRequest();
        }
    }

    private void playSong(String path) {
        if(mMiniPlayer != null) {
            processStopRequest(true);
        }

        try {
            mState = State.Idle;

            createMediaPlayer();
            mMiniPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            mMiniPlayer.setDataSource(path);
        } catch (IOException e) {
            e.printStackTrace();
        }

        mState = State.Preparing;
        mMiniPlayer.prepareAsync();
    }

    private void playSong(Uri uri) {
        if(mMiniPlayer != null) {
            processStopRequest(true);
        }

        try {
            mState = State.Idle;

            createMediaPlayer();
            mMiniPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            mMiniPlayer.setDataSource(getBaseContext(), uri);
        } catch (IOException e) {
            e.printStackTrace();
        }

        mState = State.Preparing;
        mMiniPlayer.prepareAsync();
    }

    private void tryToGetAudioFocus() {
        if (mAudioFocus != AudioFocus.Focused && mAudioFocusHelper != null
                && mAudioFocusHelper.requestFocus())
            mAudioFocus = AudioFocus.Focused;
    }

    private void giveUpAudioFocus() {
        if (mAudioFocus == AudioFocus.Focused && mAudioFocusHelper != null
                && mAudioFocusHelper.abandonFocus())
            mAudioFocus = AudioFocus.NoFocusNoDuck;
    }

    private void processPlayRequest() {
        tryToGetAudioFocus();

        if (mState == State.Paused) {
            // If we're paused, just continue playback and restore the 'foreground service' state.
            mState = State.Playing;
            configAndStartMediaPlayer();
        }
    }

    private void processPauseRequest() {
        if (mState == State.Playing) {
            // Pause media player and cancel the 'foreground service' state.
            mState = State.Paused;
            mMiniPlayer.pause();
        }
    }

    private void processStopRequest() {
        processStopRequest(false);
    }

    private void processStopRequest(boolean force) {
        if (mState == State.Playing || mState == State.Paused || force) {
            if(mMiniPlayer != null) {
                mMiniPlayer.reset();
                mMiniPlayer.release();
                mMiniPlayer = null;
            }
            mState = State.Idle;
        }
        giveUpAudioFocus();
    }

    /**
     * Reconfigures MediaPlayer according to audio focus settings and starts/restarts it. This
     * method starts/restarts the MediaPlayer respecting the current audio focus state. So if
     * we have focus, it will play normally; if we don't have focus, it will either leave the
     * MediaPlayer paused or set it to a low volume, depending on what is allowed by the
     * current focus settings. This method assumes mPlayer != null, so if you are calling it,
     * you have to do so from a context where you are sure this is the case.
     */
    void configAndStartMediaPlayer() {
        if (mAudioFocus == AudioFocus.NoFocusNoDuck) {
            // If we don't have audio focus and can't duck, we have to pause, even if mState
            // is State.Playing. But we stay in the Playing state so that we know we have to resume
            // playback once we get the focus back.
            if (mMiniPlayer.isPlaying()) mMiniPlayer.pause();
            return;
        }
        else if (mAudioFocus == AudioFocus.NoFocusCanDuck)
            mMiniPlayer.setVolume(DUCK_VOLUME, DUCK_VOLUME);  // we'll be relatively quiet
        else
            mMiniPlayer.setVolume(1.0f, 1.0f); // we can be loud

        if (!mMiniPlayer.isPlaying()) {
            mMiniPlayer.start();

            m_Handler.sendEmptyMessage(MSG_PROGRESS);
        }
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        mMiniPlayerProgressBar.setProgress(1000);
        processStopRequest(true); // stop everything!
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        mState = State.Playing;
        tryToGetAudioFocus();
        configAndStartMediaPlayer();
    }


    @Override
    public void onGainedAudioFocus() {
        mAudioFocus = AudioFocus.Focused;

        // restart media player with new focus settings
        if (mState == State.Playing)
            configAndStartMediaPlayer();
    }

    @Override
    public void onLostAudioFocus(boolean canDuck) {
        mAudioFocus = canDuck ? AudioFocus.NoFocusCanDuck : AudioFocus.NoFocusNoDuck;

        // start/restart/pause media player with new focus settings
        if (mMiniPlayer != null && mMiniPlayer.isPlaying())
            configAndStartMediaPlayer();
    }

    private final int MSG_PROGRESS = 1;
    private Handler m_Handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if(msg.what == MSG_PROGRESS) {

                if (mState == State.Playing
                        // mjkong.
                        && mMiniPlayer != null) {
                    int position = mMiniPlayer.getCurrentPosition();
                    int duration = mMiniPlayer.getDuration();

                    int percent = (int)((long)((position*1000L)/duration));
                    mMiniPlayerProgressBar.setProgress(percent);

                    msg = obtainMessage(MSG_PROGRESS);
                    sendMessageDelayed(msg, 1000-(position%1000));
                }
            }
        }
    };
}
