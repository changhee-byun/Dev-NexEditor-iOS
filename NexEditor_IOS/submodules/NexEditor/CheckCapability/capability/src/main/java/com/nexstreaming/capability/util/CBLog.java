package com.nexstreaming.capability.util;

import android.util.Log;

public class CBLog
{
	private final static String PREFIX = "[Capability]";
	private final static String TEST_PREFIX ="[TestCapability]";
	private static boolean m_isEnabled = true;
	private static boolean m_isTestEnabled = false;

	public static void enable(boolean isEnable)
	{
		m_isEnabled = isEnable;
	}

	public static void enableTestLog(boolean isEnable)
	{
		m_isTestEnabled = isEnable;
	}

	public static void e(String strMsg)
	{
		if (m_isEnabled)
		{
			Exception e = new Exception();
			StackTraceElement callerElement = e.getStackTrace()[1];
			Log.e(PREFIX, "[" + callerElement.getFileName() + " " + callerElement.getLineNumber() + "] " + strMsg);
		}
	}

	public static void e(String strMsg, Throwable tr)
	{
		if (m_isEnabled)
		{
			Exception e = new Exception();
			StackTraceElement callerElement = e.getStackTrace()[1];
			Log.e(PREFIX, "[" + callerElement.getFileName() + " " + callerElement.getLineNumber() + "] " + strMsg, tr);
		}
	}

	public static void w(String strMsg)
	{
		if (m_isEnabled)
		{
			Exception e = new Exception();
			StackTraceElement callerElement = e.getStackTrace()[1];
			Log.w(PREFIX, "[" + callerElement.getFileName() + " " + callerElement.getLineNumber() + "] " + strMsg);
		}
	}

	public static void i(String strMsg)
	{
		if (m_isEnabled)
		{
			Exception e = new Exception();
			StackTraceElement callerElement = e.getStackTrace()[1];
			Log.i(PREFIX, "[" + callerElement.getFileName() + " " + callerElement.getLineNumber() + "] " + strMsg);
		}
	}

	public static void d(String strMsg)
	{
		if (m_isEnabled)
		{
			Exception e = new Exception();
			StackTraceElement callerElement = e.getStackTrace()[1];
			Log.d(PREFIX, "[" + callerElement.getFileName() + " " + callerElement.getLineNumber() + "] " + strMsg);
		}
	}

	public static void v(String strMsg)
	{
		if (m_isEnabled)
		{
			Exception e = new Exception();
			StackTraceElement callerElement = e.getStackTrace()[1];
			Log.v(PREFIX, "[" + callerElement.getFileName() + " " + callerElement.getLineNumber() + "] " + strMsg);
		}
	}

	public static void t(String strMsg)
	{
		if (m_isEnabled && m_isTestEnabled)
		{
			Exception e = new Exception();
			StackTraceElement callerElement = e.getStackTrace()[1];
			Log.d(TEST_PREFIX, "[" + callerElement.getFileName() + " " + callerElement.getLineNumber() + "] " + strMsg);
		}
	}
}
