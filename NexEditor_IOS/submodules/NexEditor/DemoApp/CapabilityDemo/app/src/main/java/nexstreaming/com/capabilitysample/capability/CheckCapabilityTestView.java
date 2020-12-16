package nexstreaming.com.capabilitysample.capability;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.util.JsonReader;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import nexstreaming.com.capabilitysample.R;

import com.nexstreaming.capability.sdk.CapabilitySDK;
import com.nexstreaming.capability.test.CodecCapabilityManager;
import com.nexstreaming.capability.util.JSonWriterUtil;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 * A placeholder fragment containing a simple view.
 */
public class CheckCapabilityTestView extends LinearLayout implements
        CapabilitySDK.CapabilityListener {

    final String TAG = "CheckCapabilityTestView";
    CapabilitySDK mCapabilitySDK;

    //RadioButton encRadio;
    int encoderSet;
    //RadioButton decRadio;
    int decoderSet;
    int maxCodecCount=16;

    Spinner spinnerMax;
    Button btnStart;
    Button btnStop;
    Button btnCopy;

    ProgressBar progressBar;

    Context mContext;
    Activity mActivity;

    boolean bDeleteResultFile=false;

    public CheckCapabilityTestView(Context context, Activity activity) {
        super(context);

        mContext = context;
        mActivity = activity;
        LayoutInflater.from(getContext()).inflate(R.layout.fragment_check_capability, this);

        mCapabilitySDK = new CapabilitySDK(context, activity, this);

        Log.d(TAG, "Capability SDK Version : " + mCapabilitySDK.getSDKVersionInfo());

        spinnerMax = (Spinner) findViewById(R.id.spinner_max);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(mContext, android.R.layout.simple_spinner_dropdown_item, getResources().getStringArray(R.array.MaxCodecCountArray));
        spinnerMax.setAdapter(adapter);
        spinnerMax.setEnabled(true);
        spinnerMax.setSelection(4);
        spinnerMax.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                maxCodecCount = Integer.parseInt((String)parent.getSelectedItem());
            }
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        btnStart = (Button) findViewById(R.id.button_start);
        btnStop = (Button) findViewById(R.id.button_stop);
        btnCopy = (Button) findViewById(R.id.button_access);

        RadioButton.OnClickListener radioOnClickListener = new RadioButton.OnClickListener() {
            public void onClick(View v) {
                boolean checked = ((RadioButton) v).isChecked();
                // Check which radio button was clicked
                switch(v.getId()) {
                    case R.id.enc720p:
                        if (checked) encoderSet = CodecCapabilityManager.TEST_CONTENT_720P;
                        break;
                    case R.id.enc1080p:
                        if (checked) encoderSet = CodecCapabilityManager.TEST_CONTENT_1080P;
                        break;
                    case R.id.enc3840p:
                        if (checked) encoderSet = CodecCapabilityManager.TEST_CONTENT_3840P;
                        spinnerMax.setSelection(0);
                        break;
                    case R.id.dec720p:
                        if (checked) decoderSet = CodecCapabilityManager.TEST_CONTENT_720P;
                        break;
                    case R.id.dec1080p:
                        if (checked) decoderSet = CodecCapabilityManager.TEST_CONTENT_1080P;
                        break;
                    case R.id.dec3840p:
                        if (checked) decoderSet = CodecCapabilityManager.TEST_CONTENT_3840P;
                        spinnerMax.setSelection(0);
                        break;
                }
            }
        };

        ((RadioButton)findViewById(R.id.enc720p)).setOnClickListener(radioOnClickListener);
        ((RadioButton)findViewById(R.id.enc1080p)).setOnClickListener(radioOnClickListener);
        ((RadioButton)findViewById(R.id.enc1080p)).setChecked(true);
        encoderSet = CodecCapabilityManager.TEST_CONTENT_1080P;
        ((RadioButton)findViewById(R.id.enc3840p)).setOnClickListener(radioOnClickListener);
        ((RadioButton)findViewById(R.id.dec720p)).setOnClickListener(radioOnClickListener);
        ((RadioButton)findViewById(R.id.dec1080p)).setOnClickListener(radioOnClickListener);
        ((RadioButton)findViewById(R.id.dec1080p)).setChecked(true);
        decoderSet = CodecCapabilityManager.TEST_CONTENT_1080P;
        ((RadioButton)findViewById(R.id.dec3840p)).setOnClickListener(radioOnClickListener);

        progressBar = (ProgressBar) findViewById(R.id.progressbar);

        btnStart.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                progressBar.setVisibility(View.VISIBLE);

                btnStart.setEnabled(false);
                btnStop.setEnabled(true);

                if(mCapabilitySDK.startCapabilityTest(encoderSet, decoderSet, maxCodecCount) == false) {
                    progressBar.setVisibility(View.INVISIBLE);
                    btnStart.setEnabled(true);
                    btnStop.setEnabled(false);
                }
            }
        });

        btnStop.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                progressBar.setVisibility(View.INVISIBLE);
                btnStart.setEnabled(true);
                btnStop.setEnabled(false);

                mCapabilitySDK.stopCapabilityTest();
            }
        });
        btnCopy.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent( );
                intent.setType("*/*");
                intent.setAction(Intent.ACTION_GET_CONTENT);
                mActivity.startActivityForResult(intent, 100);
            }
        });

        try {
            InputStream in=JSonWriterUtil.loadResultJsonFile();
            if ( in != null ) {
                onCapabilityListener(in);
                JSonWriterUtil.removeResultFile();
            }
        } catch (IOException e) {
        }
    }

    @Override
    protected void onDetachedFromWindow() {
        if ( bDeleteResultFile == true )
            JSonWriterUtil.removeResultFile();
        super.onDetachedFromWindow();
    }

    @Override
    public void onCapabilityListener(InputStream inputStream) {

        int count1 = 0;
        int count2 = 0;
        int count3 = 0;
        int count4 = 0;
        int count5 = 0;
        int count6 = 0;
        int count7 = 0;
        int count8 = 0;

        JsonReader reader = new JsonReader(new InputStreamReader(inputStream));

        try {
            reader.beginObject();
            while(reader.hasNext()) {
                String name = reader.nextName();
                if(name.equals("value1_in_type1")/*available decoder count, encoder operates in the background*/) {
//                    Log.d(TAG, "available decoder count(Encoder) : " +  reader.nextInt());
                    count1 = reader.nextInt();
                } else if(name.equals("value2_in_type1")/*available decoder count, encoder doesn't operate in the background*/) {
//                    Log.d(TAG, "available decoder count : " + reader.nextInt());
                    count2 = reader.nextInt();
                }  else if(name.equals("value1_in_type2")/*real time decoder count, encoder operates in the background*/) {
//                    Log.d(TAG, "real time decoder count(Encoder) : " + reader.nextInt());
                    count3 = reader.nextInt();
                } else if(name.equals("value2_in_type2")/*real time decoder count, encoder doesn't operate in the background*/) {
//                    Log.d(TAG, "real time decoder count : " + reader.nextInt());
                    count4 = reader.nextInt();
                } else if(name.equals("value1_in_type3")/*available decoder count, encoder operates in the background*/) {
//                    Log.d(TAG, "available decoder count(Encoder) : " +  reader.nextInt());
                    count5 = reader.nextInt();
                } else if(name.equals("value2_in_type3")/*available decoder count, encoder doesn't operate in the background*/) {
//                    Log.d(TAG, "available decoder count : " + reader.nextInt());
                    count6 = reader.nextInt();
                }  else if(name.equals("value1_in_type4")/*real time decoder count, encoder operates in the background*/) {
//                    Log.d(TAG, "real time decoder count(Encoder) : " + reader.nextInt());
                    count7 = reader.nextInt();
                } else{
//                    Log.d(TAG, "real time decoder count : " + reader.nextInt());
                    count8 = reader.nextInt();
                }
//                else{ // 720p data has not used
//                    reader.nextInt();
//                }
            }
            reader.endObject();
        } catch (IOException e) {
            e.printStackTrace();
        }

        StringBuffer stringBuffer = new StringBuffer();
        stringBuffer.append("Decoder count\n/*executing Encoder in the background*/: "+ count1
                +"\nDecoder count\n/*not executing Encoder in the background*/: "+ count2
                +"\nIn Real Time count\n/*executing Encoder in the background*/: "+ count3
                +"\nIn Real Time count\n/*not executing Encoder in the background*/: "+ count4);
        alertDialog = generateAlertPopUp("Result", stringBuffer.toString());
        alertDialog.show();

        progressBar.setVisibility(View.INVISIBLE);
        btnStart.setEnabled(true);
        btnStop.setEnabled(false);
        mCapabilitySDK = null;
        mCapabilitySDK = new CapabilitySDK(mContext, mActivity, this);
        bDeleteResultFile = true;
    }

    AlertDialog alertDialog;

    AlertDialog generateAlertPopUp(String title, String message) {
        LayoutInflater factory = LayoutInflater.from(mContext);
        final View view = factory.inflate(R.layout.dialog_layout, null);

        TextView tv_title = (TextView) view.findViewById(R.id.dialog_title);
        tv_title.setText(title);

        TextView tv_message = (TextView) view.findViewById(R.id.dialog_message);
        tv_message.setText(message);

        return new AlertDialog.Builder(mActivity, AlertDialog.THEME_DEVICE_DEFAULT_LIGHT)
                .setView(view)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        alertDialog.dismiss();
                        //
                    }
                })
                .create();
    }

}
