package com.nexstreaming.kminternal.kinemaster.config;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

import android.os.Environment;
import android.os.StrictMode;
import android.util.Log;

public class KMConfigFile {
	
	private static final String LOG_TAG = "KMConfigFile";
	
	private static KMConfigFile sIntance = null;
	
	private String[] mRegisterServerURL = null;
	private String[] mDeviceSupportServerURL = null;
	private String[] mUpdateServerURL = null;
	private String[] mUsageServerURL = null;
	private String[] mNotifyServerURL = null;
	private String[] mThemeServerURL = null;
	private String[] mRegisterIAPServerURL = null;
	private String[] mAuthPromocodeServerURL = null;
	private String mUpdateServerParam = null;
	private Boolean mDeviceSupportServerCache = null;
    private Boolean mDeviceSupportLocalOnly = null;
	private Integer mDeviceSupportServerTimeout = null;
    private String mConfigFileSrc = null;
	
	public static KMConfigFile getInstance() {
		
		if( sIntance==null ) {

            StrictMode.ThreadPolicy previousPolicy = StrictMode.allowThreadDiskReads();
            try {

			File configFile = new File(Environment.getExternalStorageDirectory(),"kinemaster.cfg");
			if( configFile.exists() ) {
				BufferedReader reader = null;
				try {
					reader = new BufferedReader(new InputStreamReader( new FileInputStream(configFile) ));
					sIntance = new KMConfigFile(reader);
					reader.close();
					if( LL.I ) Log.i(LOG_TAG, "Using config file" );
				} catch (IOException e) {
					if( LL.D ) Log.d(LOG_TAG, "Error reading config file; skipping", e );
					sIntance = new KMConfigFile();
				}
			} else {
				sIntance = new KMConfigFile();
				if( LL.D ) Log.d(LOG_TAG, "Config file not found; using defaults" );
                }
            } finally {
                StrictMode.setThreadPolicy(previousPolicy);
            }

		}
		
		return sIntance;
		
	}
	
	private KMConfigFile() {
	}

	private KMConfigFile( BufferedReader reader ) throws IOException {

        StringBuilder sb = new StringBuilder();

		String line;
		while( (line=reader.readLine())!=null ) {
            if( sb.length() < 4096 ) {
                sb.append("  ").append(line).append('\n');
            }
			if( line.startsWith("#") || line.startsWith(";") || !line.contains("=")) {
				continue;
			}
			String[] kv = line.split("=", 2);
			if( kv.length<2 ) {
				continue;
			}
			String key = kv[0].trim();
			String value = kv[1].trim();
			if( key.length()<1 ) {
				return;
			}
			processKeyValuePair(key,value);
		}
        mConfigFileSrc = sb.toString();
	}
	
	private void processKeyValuePair( String key, String value ) {
		
		if( key.equalsIgnoreCase("DEVICE_SUPPORT_SERVER_TIMEOUT")) {
			try {
				int intval = Integer.parseInt(value);
				mDeviceSupportServerTimeout = intval;
			} catch( NumberFormatException e ) {
			}
			return;
        } else if( key.equalsIgnoreCase("DEVICE_SUPPORT_SERVER_CACHE")) {
            if( value.equalsIgnoreCase("true")) {
                mDeviceSupportServerCache = true;
            } else if( value.equalsIgnoreCase("false")) {
                mDeviceSupportServerCache = false;
            }
            return;
        } else if( key.equalsIgnoreCase("DEVICE_SUPPORT_LOCAL_ONLY")) {
            if( value.equalsIgnoreCase("true")) {
                mDeviceSupportLocalOnly = true;
            } else if( value.equalsIgnoreCase("false")) {
                mDeviceSupportLocalOnly = false;
            }
            return;
		}
		
		String[] tmpvalues = value.split(";");
		int count = 0;
		for( int i=0; i<tmpvalues.length; i++ ) {
			tmpvalues[i] = tmpvalues[i].trim();
			if( tmpvalues[i].length()>0 ) {
				count++;
			}
		}
		String[] values = new String[count];
		count = 0;
		for( int i=0; i<tmpvalues.length; i++ ) {
			if( tmpvalues[i].length()>0 ) {
				values[count] = tmpvalues[count];
				count++;
			}
		}
		
		if( key.equalsIgnoreCase("REGISTER_SERVER_URL")) {
			mRegisterServerURL = values;
		} else if( key.equalsIgnoreCase("UPDATE_SERVER_URL")) {
			mUpdateServerURL = values;
		} else if( key.equalsIgnoreCase("UPDATE_SERVER_PARAM")) {
			mUpdateServerParam = value;
		} else if( key.equalsIgnoreCase("USAGE_SERVER_URL")) {
			mUsageServerURL = values;
		} else if( key.equalsIgnoreCase("NOTIFY_SERVER_URL")) {
			mNotifyServerURL = values;
		} else if( key.equalsIgnoreCase("DEVICE_SUPPORT_SERVER_URL")) {
			mDeviceSupportServerURL = values;
		} else if( key.equalsIgnoreCase("THEME_SERVER_URL")) {
			mThemeServerURL = values;
		} else if( key.equalsIgnoreCase("REGISTERIAP_SERVER_URL")) {
			mRegisterIAPServerURL = values;
		} else if( key.equalsIgnoreCase("AUTHPROMOCODE_SERVER_URL")) {
			mAuthPromocodeServerURL = values;
		}
	}

    public String getConfigFileSrc() {
        return mConfigFileSrc;
    }

    public String[] getRegisterServerURL() {
		return mRegisterServerURL;
	}
	
	public String[] getUpdateServerURL() {
		return mUpdateServerURL;
	}
	
	public String getUpdateServerParam() {
		return mUpdateServerParam;
	}
	
	public String[] getUsageServerURL() {
		return mUsageServerURL;
	}
	
	public String[] getThemeServerURL() {
		return mThemeServerURL;
	}
	
	public String[] getRegisterServerURL( String[] defaultURL ) {
		return mRegisterServerURL==null?defaultURL:mRegisterServerURL;
	}
	
	public String[] getUpdateServerURL( String[] defaultURL ) {
		return mUpdateServerURL==null?defaultURL:mUpdateServerURL;
	}
	
	public String[] getUsageServerURL( String[] defaultURL ) {
		return mUsageServerURL==null?defaultURL:mUsageServerURL;
	}
	
	public String[] getNotifyServerURL( String[] defaultURL ) {
		return mNotifyServerURL==null?defaultURL:mNotifyServerURL;
	}
	
	public String[] getDeviceSupportServerURL( String[] defaultURL ) {
		return mDeviceSupportServerURL==null?defaultURL:mDeviceSupportServerURL;
	}
	
	public String[] getThemeServerURL( String[] defaultURL ) {
		return mThemeServerURL==null?defaultURL:mThemeServerURL;
	}
	
	public String[] getRegisterIAPServerURL( String[] defaultURL ) {
		return mRegisterIAPServerURL==null?defaultURL:mRegisterIAPServerURL;
	}
	
	public String[] getAuthPromocodeServerURL( String[] defaultURL ) {
		return mAuthPromocodeServerURL==null?defaultURL:mAuthPromocodeServerURL;
	}
	
	public int getDeviceSupportServerTimeout( int defaultValue ) {
		return mDeviceSupportServerTimeout==null?defaultValue:mDeviceSupportServerTimeout;
	}

    public boolean getDeviceSupportServerCache( boolean defaultValue ) {
        return mDeviceSupportServerCache==null?defaultValue:mDeviceSupportServerCache;
    }

    public boolean getDeviceSupportLocalOnly( boolean defaultValue ) {
        return mDeviceSupportLocalOnly==null?defaultValue:mDeviceSupportLocalOnly;
    }


}
