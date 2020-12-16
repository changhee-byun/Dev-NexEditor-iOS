package com.nexstreaming.nexeditorsdkapis.qatest;

import android.app.AlertDialog;
import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdkapis.R;

/**
 * Created by jeongwook.yoon on 2016-05-09.
 */
public class AutoQATestAdapter extends BaseAdapter  {
    private static TestCaseManager.TestCaseResult[] report;
    private static Context mContext;
    public AutoQATestAdapter( TestCaseManager.TestCaseResult[] report){
        this.report = report;
    }

    @Override
    public int getCount() {
        return report.length;
    }

    @Override
    public Object getItem(int position) {
        return report[position];
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        Log.e("test","getView position="+position);
        final int pos = position;
        final Context context = parent.getContext();

        if( convertView == null ){
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.activity_qatest_item, parent, false);
        }
        TextView name = (TextView)convertView.findViewById(R.id.textView_qatest_name);
        TextView state = (TextView)convertView.findViewById(R.id.textView_qatest_state);
        TextView errcount = (TextView)convertView.findViewById(R.id.textView_qatest_errcount);

        name.setText(report[position].mName);
        state.setText(", State: " + report[position].mState);
        errcount.setText(", Errs: " + report[position].mErrorCount);
        return convertView;
    }


}
