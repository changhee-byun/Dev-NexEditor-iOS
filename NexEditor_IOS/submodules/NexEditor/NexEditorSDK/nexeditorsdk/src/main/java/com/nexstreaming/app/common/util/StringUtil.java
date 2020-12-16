package com.nexstreaming.app.common.util;

import android.content.Context;

import java.util.Collection;
import java.util.Locale;
import java.util.Map;

public class StringUtil {

	private static final char[] sHexCharsUpper = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	private static final char[] sHexCharsLower = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

	private StringUtil() {
	}

	public static String bytesToHexStringUpper( byte[] bytes ) {
		char result[] = new char[bytes.length*2];
		for( int i=0; i<bytes.length; i++ ) {
			result[i*2] = sHexCharsUpper[(((int)bytes[i]) & 0xF0) >> 4];
			result[i*2+1] = sHexCharsUpper[(((int)bytes[i]) & 0x0F)];
		}
		return new String(result);
	}

	public static String bytesToHexStringLower( byte[] bytes ) {
		char result[] = new char[bytes.length*2];
		for( int i=0; i<bytes.length; i++ ) {
			result[i*2] = sHexCharsLower[(((int)bytes[i]) & 0xF0) >> 4];
			result[i*2+1] = sHexCharsLower[(((int)bytes[i]) & 0x0F)];
		}
		return new String(result);
	}

	public static int comparePossiblyNullStrings( String first, String second ) {
		if( first==null && second==null ) {
			return 0;
		} else if( first==null ) {
			return -1;
		} else if( second==null ) {
			return 1;
		} else {
			return first.compareTo(second);
		}
	}

	public static boolean isEmpty(String s) {
		return s==null || s.length()<1;
	}

	public static String collectionToString(Collection c) {
		if( c==null )
			return "null";
		StringBuilder sb = new StringBuilder();
		boolean first = true;
		for( Object o: c ) {
			if( first ) {
				first = false;
			} else {
				sb.append(',');
			}
			sb.append(String.valueOf(o));
		}
		return sb.toString();
	}

	public static String getLocalizedString( Context context, Map<String,String> localeStringMap ) {
		return resolveResourceRefs(context,localizeString(context, localeStringMap, null));
	}

	public static String getLocalizedString( Context context, Map<String,String> localeStringMap, String defString ) {
		return resolveResourceRefs(context,localizeString(context, localeStringMap, defString));
	}

	private static String localizeString(Context context, Map<String,String> localizedStringMap, String defString ) {
		Locale locale = null;
		if( context!=null )
			locale = context.getResources().getConfiguration().locale;
		if( localizedStringMap==null ) {
			return null;
		}
		if( locale==null ) {
			locale = Locale.ENGLISH;
		}
		String lang = locale.getLanguage().toLowerCase(Locale.ENGLISH);
		String country = locale.getCountry().toLowerCase(Locale.ENGLISH);
		String variant = locale.getVariant().toLowerCase(Locale.ENGLISH);
		String useLocale = "";
		if( variant.isEmpty() && country.isEmpty() ) {
			useLocale = lang;
		} else if( variant.isEmpty() ) {
			useLocale = lang + "-" + country;
		} else if( country.isEmpty() ) {
			useLocale = lang + "-" + variant;
		} else {
			useLocale = lang + "-" + country + "-" + variant;
		}
		String useLabel = localizedStringMap.get(useLocale);
		if( useLabel==null ) {
			useLabel = localizedStringMap.get(lang);
			if( useLabel==null ) {
				useLabel = localizedStringMap.get("");
				if( useLabel==null ) {
					useLabel = localizedStringMap.get("en");
					if( useLabel==null ) {
						useLabel = localizedStringMap.get("en-us");
						if( useLabel==null ) {
							useLabel = defString;
						}
					}
				}
			}
		}
		return useLabel;
	}

	private static String resolveResourceRefs( Context context, String string ) {
		if( string==null || context == null )
			return string;

		if( string.startsWith("@string/") ) {
			int id = context.getResources().getIdentifier("string/kedl_" + string.substring(8), "string", context.getPackageName());
			if( id!=0 ) {
				return context.getResources().getString(id);
			}
		} else if( string.startsWith("@") ) {
			int id = context.getResources().getIdentifier(string.substring(1), "string", context.getPackageName());
			if( id!=0 ) {
				return context.getResources().getString(id);
			}
		}
		return string;
	}

}
