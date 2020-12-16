package nexstreaming.com.capabilitysample.codecutil;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.media.MediaCodecInfo;
import android.media.MediaCodecInfo.VideoCapabilities;
//import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.net.Uri;
import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;

import nexstreaming.com.capabilitysample.R;

public class CodecSearchActivity extends AppCompatActivity {

    private Button btn_search;
    private Button btn_sendEmail;
    private TextView tv_result;
    private StringBuilder str_result;

    private void printOneMediaCodecInfo(MediaCodecInfo info, int index, int h) {
        tv_result.append("\n" + index + ". " + info.getName());
        if (info.isEncoder())   tv_result.append("(Encoder)");
        else                    tv_result.append("(Decoder)");
        String[] types = info.getSupportedTypes();
        for (int j = 0; j < types.length; j++) {
            tv_result.append("\n- " + types[j]);

            if( Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP ) {
                if (info.isEncoder()) {
                    VideoCapabilities vc = info.getCapabilitiesForType(types[j]).getVideoCapabilities();
                    if ( vc != null ) {
                        tv_result.append("\n video-caps. width align="+vc.getWidthAlignment());
                        tv_result.append("\n-videoc-caps, height_align=" + vc.getHeightAlignment());
                    }
                }
            }
        }

    }

    private void enumCodecInfo1() {

        if( Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP ) {
            int numCodecs = MediaCodecList.getCodecCount();
            tv_result.append("\n\nBefore API 21. ( total " + numCodecs + " )");

            for (int i = 0; i < numCodecs; i++) {
                MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
                printOneMediaCodecInfo(codecInfo, i, 0);
            }
        }
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private void enumCodecInfo2() {
        if( Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP ) {
            final MediaCodecList codecs = new MediaCodecList(MediaCodecList.REGULAR_CODECS);
            final MediaCodecInfo[] infos = codecs.getCodecInfos();
            int numCodecs = infos.length;

            tv_result.append("\n\nAfter API 21. R( total " + numCodecs + " )");


            for (int i = 0; i < numCodecs; i++) {
                printOneMediaCodecInfo(infos[i], i, 0);
            }
        }
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private void enumCodecInfo3() {
        if( Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP ) {
            final MediaCodecList codecs = new MediaCodecList(MediaCodecList.ALL_CODECS);
            final MediaCodecInfo[] infos = codecs.getCodecInfos();
            int numCodecs = infos.length;

            tv_result.append("\n\nAfter API 21. A( total " + numCodecs + " )");


            for (int i = 0; i < numCodecs; i++) {
                printOneMediaCodecInfo(infos[i], i,0);
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_codec_capability);

        tv_result = (TextView)findViewById(R.id.edit_text_result);

        btn_search = (Button)findViewById(R.id.button_search);
        btn_search.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                enumCodecInfo1();
                enumCodecInfo2();
                enumCodecInfo3();
            }
        });

        btn_sendEmail = (Button)findViewById(R.id.button_sendEmail);
        btn_sendEmail.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent it = new Intent(Intent.ACTION_SEND);
                it.setData(Uri.parse("mailto:"));
                it.setType("text/plain");
                it.putExtra(Intent.EXTRA_EMAIL, new String[]{"editor@nexstreaming.com"});
                it.putExtra(Intent.EXTRA_TEXT, tv_result.getText().toString());
                startActivity(Intent.createChooser(it, "Choose Email Client"));
            }
        });
    }
}
