package com.nexstreaming.app.common.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Created by jeongwook.yoon on 2016-11-14.
 */

public class CopyUtil {
    static public void  in2sdcard(InputStream in, File destFile) throws IOException {
        int size;
        byte[] w = new byte[1024];
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(destFile);
            while(true) {
                size = in.read(w);
                if (size <= 0)
                    break;
                fos.write(w, 0, size);
            }
        } catch (IOException e) {
            throw e;
        } finally {
            if (fos != null) {
                fos.close();
            }
        }
    }

    static public void rename(String path, String srcFileName, String destFileName) throws IOException {
        File oldFile = new File(path +File.separator+ srcFileName);
        File newFile = new File(path +File.separator+ destFileName);

        if(!oldFile.renameTo(newFile)){
            FileInputStream fin = new FileInputStream(oldFile);
            FileOutputStream fout = new FileOutputStream(newFile);
            byte[] buf = new byte[1024];

            int read = 0;
            while((read=fin.read(buf,0,buf.length))!=-1){
                fout.write(buf, 0, read);
            }

            fin.close();
            fout.close();
            oldFile.delete();
        }
    }
}
