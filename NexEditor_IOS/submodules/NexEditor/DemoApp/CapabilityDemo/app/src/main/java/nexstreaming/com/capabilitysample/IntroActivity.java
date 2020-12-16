package nexstreaming.com.capabilitysample;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.Arrays;

import nexstreaming.com.capabilitysample.capability.CheckCapabilityActivity;
import nexstreaming.com.capabilitysample.codecutil.CodecSearchActivity;

public class IntroActivity extends AppCompatActivity {

    private ListView introList;

    private ArrayList<String> intromenu;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_intro);

        intromenu = new ArrayList<String>(Arrays.asList("codec utility", "capability")); // 간단히 2개만
        introList = (ListView) findViewById(R.id.introlist);
        introList.setAdapter(new ArrayAdapter<String>(IntroActivity.this, android.R.layout.simple_list_item_1, intromenu));
        introList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if(position == 0) {
                    Intent intent = new Intent(getBaseContext(), CodecSearchActivity.class);
                    startActivity(intent);
                } else {
                    Intent intent = new Intent(getBaseContext(), CheckCapabilityActivity.class);
                    startActivity(intent);
                }
            }
        });
    }
}
