package com.nexstreaming.app.assetlibrary.utils;

import android.content.Context;
import android.content.DialogInterface;
import android.support.v7.app.AlertDialog;

import com.nexstreaming.app.assetlibrary.R;


/**
 * Created by ojin.kwon on 2016-12-02.
 */

public class DialogUtil {

    public static void createSimpleAlertDialog(Context context, int message, DialogInterface.OnClickListener ll){
        if(context != null && ll != null){
            AlertDialog.Builder builder = new AlertDialog.Builder(context);
            builder.setTitle(android.R.string.dialog_alert_title)
                    .setMessage(message)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i) {
                            dialogInterface.dismiss();
                        }
                    }).show();
        }
    }

}
