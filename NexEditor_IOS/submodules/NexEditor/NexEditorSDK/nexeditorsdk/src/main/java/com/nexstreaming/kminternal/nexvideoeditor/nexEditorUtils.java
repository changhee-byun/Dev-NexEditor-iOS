package com.nexstreaming.kminternal.nexvideoeditor;

import android.content.Context;
import android.os.StrictMode;
import android.util.Log;

import java.io.File;
import dalvik.system.BaseDexClassLoader;

import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.nexvideoeditor.NexRectangle;

public class nexEditorUtils {
    static final private String LOG_TAG = "nexEditorUtils";
    public enum Abi{
        NONE, ARM_64, ARM, X86_64, X86;
    }

    private static native int getKenBurnsRectsN(int width, int height, int faceCount, NexRectangle[] faceInfos, int aspectWidth, int aspectHeight, int duration, NexRectangle[] results );

    static {
        StrictMode.ThreadPolicy prevThreadPolicy = StrictMode.allowThreadDiskReads();
        try {
            System.loadLibrary("nexeditorsdk" + EditorGlobal.SO_FREFIX);
        } catch (UnsatisfiedLinkError exc) {
            if (LL.E) Log.e(LOG_TAG, "[nexUtils.java] nexeditor load failed : " + exc);
        } finally {
            StrictMode.setThreadPolicy(prevThreadPolicy);
        }
    }

    public static int getKenBurnsRects(int width, int height, NexRectangle[] faceInfos, int aspectWidth, int aspectHeight, int duration, NexRectangle[] results )
    {
	    int iArrayCount = 0;
		if (faceInfos instanceof Object[]) {
			iArrayCount = ((Object[])faceInfos).length;
		}
		return getKenBurnsRectsN(width, height, iArrayCount, faceInfos, aspectWidth, aspectHeight, duration, results);
    }

	private static String getEngineLibPathWithContext(Context context) {
		String strWorkingEngineLibPath = null;

		File fileDir = context.getFilesDir();
		final String sdkLibName = "libnexeditorsdk" + EditorGlobal.SO_FREFIX + ".so";
		
		if (fileDir == null)
			throw new IllegalStateException("No files directory - cannot play video - relates to Android issue: 8886!");
		
		String strMcPath = ((BaseDexClassLoader) context.getClassLoader()).findLibrary("nexcralbody_mc_jb" + EditorGlobal.SO_FREFIX);
		if (strMcPath != null) {
			int end = strMcPath.lastIndexOf(File.separator);
			strWorkingEngineLibPath = strMcPath.substring(0, end) + File.separator;
			if (LL.D) Log.d(LOG_TAG, "[nexlib] getApplicationInfo mc libarays in: " + strWorkingEngineLibPath);
		} else {
			strWorkingEngineLibPath = context.getApplicationInfo().nativeLibraryDir;
			
			if (!strWorkingEngineLibPath.endsWith(File.separator))
				strWorkingEngineLibPath = strWorkingEngineLibPath + File.separator;
			
			if (LL.D) Log.d(LOG_TAG, "[nexlib] getApplicationInfo says libs are in: " + strWorkingEngineLibPath);
			if (LL.D) Log.d(LOG_TAG, "[nexlib] sdk lib name: " + sdkLibName);
			if (new File(strWorkingEngineLibPath, sdkLibName).exists()) {
				if (LL.D) Log.d(LOG_TAG, "[nexlib] libs found in: " + strWorkingEngineLibPath);
			} else {
				strWorkingEngineLibPath = null;
				if (LL.D) Log.d(LOG_TAG, "[nexlib] libs are not found");
			}
		}

		return strWorkingEngineLibPath;
	}

	private static String getEngineLibPathWithJavaLibraryPath(Context context) {
		String strWorkingEngineLibPath = null;

		final String sdkLibName = "libnexeditorsdk" + EditorGlobal.SO_FREFIX + ".so";
		String javaLibraryPath = System.getProperty("java.library.path");
		if (javaLibraryPath != null) {
			String[] paths = javaLibraryPath.split(":");
			// Add a '/' to the end of each directory so we don't have to do it every time.
			for (int i = 0; i < paths.length; ++i) {
				if (!paths[i].endsWith("/")) {
					paths[i] += "/";
				}

				if (LL.D) Log.d(LOG_TAG, "[nexlib] trying: " + paths[i]);
				if (new File(paths[i], sdkLibName).exists()) {
					if (LL.D) Log.d(LOG_TAG, "[nexlib] libs found in: " + paths[i]);
					strWorkingEngineLibPath = paths[i];
					break;
				} else {
					if (LL.D) Log.d(LOG_TAG, "[nexlib] libs NOT FOUND!");
				}
			}
		}

		return strWorkingEngineLibPath;
	}	
	public static String getEngineNativeLibPath(Context context) {
		String strLibPath = getEngineLibPathWithContext(context);

		if (strLibPath == null || strLibPath.length() == 0) {
			strLibPath = getEngineLibPathWithJavaLibraryPath(context);
			if (strLibPath == null || strLibPath.length() == 0) {

	            strLibPath = context.getApplicationInfo().nativeLibraryDir;
	            if (!strLibPath.endsWith(File.separator))
	                strLibPath = strLibPath + File.separator;

				if (strLibPath.contains("/arm64")) {
					strLibPath = "/system/lib64/";
				} else if (strLibPath.contains("/x86_64")) {
					strLibPath = "/system/lib64/";
				} else {
					strLibPath = "/system/lib/";
				}
				if (LL.D) Log.d(LOG_TAG, "[nexlib]2 trying: " + strLibPath);

				final String sdkLibName = "libnexeditorsdk" + EditorGlobal.SO_FREFIX + ".so";
				if (new File(strLibPath, sdkLibName).exists()) {
					if (LL.D) Log.d(LOG_TAG, "[nexlib]2 libs found in: " + strLibPath);
				} else {
					if (LL.D) Log.d(LOG_TAG, "[nexlib]2 libs NOT FOUND!");
				}
			}
		}

		return strLibPath;
	}

	public static Abi getWorkingEngineAbi(Context context) {
		Abi eRet = Abi.NONE;
		String strLibPath = getEngineLibPathWithContext(context);

		if (strLibPath == null || strLibPath.length() == 0) {			
			strLibPath = getEngineLibPathWithJavaLibraryPath(context);
			if (strLibPath == null || strLibPath.length() == 0) {
	            strLibPath = context.getApplicationInfo().nativeLibraryDir;
			}
		}

		if (strLibPath != null && strLibPath.length() != 0) {
			if (strLibPath.contains("/arm64")) {
				eRet = Abi.ARM_64;
			} else if (strLibPath.contains("/arm")) {
				eRet = Abi.ARM;
			} else if (strLibPath.contains("/x86_64")) {
				eRet = Abi.X86_64;
			} else if (strLibPath.contains("/x86")) {
				eRet = Abi.X86;
			}
		}

		if (LL.D) Log.d(LOG_TAG, "getAbiMode: strLibPath = " + strLibPath + ", iRet = " + eRet);
		return eRet;
	}
}
