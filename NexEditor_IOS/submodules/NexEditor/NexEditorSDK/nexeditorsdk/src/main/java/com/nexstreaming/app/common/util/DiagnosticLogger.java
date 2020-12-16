package com.nexstreaming.app.common.util;

import java.util.Random;

import android.util.Log;

public class DiagnosticLogger {
	
	private static final String VERSION_CODE = "A";
	private static final int MAX_LEN = 100;
	private static final boolean DISCARD_OVERFLOW_AT_END = true;
	private static final String LOG_TAG = "DiagnosticLogger";
	private static final boolean ENABLE_LOGGING = false;
	
	public interface IntError {
		int getIntErrorCode();
	}
	
	public static enum Tag {
		// Y - special; not used even as part (used as terminator)
		// N,U,Q - special; not used even as part (used in replace for compression)
		// I - reserved; not used due to confusion with 1
		// O - reserved; not used due to confusion with 0
		// P,R - unused as prefix
		NEF_CLICK_EXPORT("A"),
		NEF_APC_VALID("B"),
		NEF_APC_NONE_CACHED("C"),
		NEF_APC_NONE_UNCACHED("D"),
		NEF_START_EXPORT_WM("E"),
		NEF_START_EXPORT_NOWM("ZF"),
		NEF_SUB_HAS("G"),
		NEF_SUB_NOHAS_OKSKU("ZV"),
		APCM_UNEX_CC("ZA"),
		APCM_HFAIL("J"),
		APCM_UNEX_NEG("K"),
		IW_GETSKU_INVALID("ZG"),
		IW_GETSKU_OK("M"),
		IW_GETSKU_MISSING("FT"),
		IW_P_NOTVALID("ZB"),
		IW_P_NOTPURCHASED_REFUNDED("FG"),
		IW_P_NOTPURCHASED_CANCELED("FJ"),
		IW_P_NOTPURCHASED_OTHER("FK"),
		IW_P_NO_PLD("S"),
		IW_P_OLD_PLD("T"),
		IW_P_BAD_PLD("FM"),
		IW_P_OK_PLD("V"),
		IW_P_PREFIX_OK("W"),
		IW_P_PREFIX_NOT_FOUND("X"),
		IH_GET_PURCHASES_START("H"),
		IH_SCONN_OK("ZC"),
		IH_SCONN_DIS("ZD"),
		IH_GETSKU_SCONN_DIS("PA"),
		IH_GETSKU_SCONN_NOCTX("PB"),
		IH_VFY_SVF("ZE"),
		IH_VFY_SVOK("L"),
		IH_VFY_NSALGO("ZH"),
		IH_VFY_INVALKEY("ZM"),
		IH_VFY_SIGEX("ZJ"),
		IH_VFY_SVX("ZK"),
		IH_VFY_SKIP("ZL"),
		IH_BIND_FAIL("FL"),
		;
		final String code;
		private Tag(String code) {
			this.code = code;
		}
	}
	
	public static enum ParamTag {
		NEF_APC_FAIL_GET_STATUS("FA"),
		NEF_SUB_NOHAS_BADSKU("FB"),
		NEF_SUB_FAIL("FC"),
		APCM_FAIL("FD"),
		APCM_RESPONSE("ZT"),
		IW_GETP_FAIL("FE"),
		IW_GETSKU_FAIL("FF"),
		IW_GETP_RESULT("ZP"),
		IH_LIST_SIZES("ZW"),
		IH_VFY_CONT("ZR"),
		IH_GETP_SIZE("ZS"),
		IH_GETSKU_SCONN_EX("PC"),
		IH_GETSKU_FAIL("PD"),
		IH_GETSKU_OK("PE"),
		;
		final String code;
		private ParamTag(String code) {
			this.code = code;
		}
	}

	
	private static final DiagnosticLogger sInstance = new DiagnosticLogger(); 
	
	private StringBuilder mLog = new StringBuilder();
	boolean mNeedTerminator = false;
	
	private DiagnosticLogger() {
	}
	
	public static DiagnosticLogger getInstance() {
		return sInstance;
	}
	
	private static boolean isNum(char c) {
		return ( c >='0' && c <='9');
	}
	
	private void limitLength() {
		if( mLog.length() > MAX_LEN ) {
			if( DISCARD_OVERFLOW_AT_END ) {
				mLog.setLength(MAX_LEN);
			} else {
				mLog.delete(0, mLog.length()-MAX_LEN);
			}
		}

	}
	
	public synchronized void log(Tag tag) {
		if(ENABLE_LOGGING) {
			Log.d(LOG_TAG,"diag log : " + tag.name());
		}
		if( mNeedTerminator && isNum(tag.code.charAt(0)) ) {
			mLog.append('Y');
		}
		mNeedTerminator = false;
		mLog.append(tag.code);
		limitLength();
	}

	public synchronized void log(ParamTag tag, int param) {
		if(ENABLE_LOGGING) {
			Log.d(LOG_TAG,"diag log : " + tag.name() + " = " + param);
		}
		mLog.append(tag.code);
		mLog.append(param);
		mNeedTerminator = true;
		limitLength();
	}
	
//	public synchronized void log(ParamTag tag, com.nexstreaming.app.common.task.Task.TaskError taskError ) {
//		if(ENABLE_LOGGING) {
//			if( LL.D ) Log.d(LOG_TAG,"diag log : " + tag.name() + " = " + taskError.getMessage(), taskError.getException());
//		}
//		mLog.append(tag.code);
//		int param = 0;
//		if( taskError instanceof IntError ) {
//			param=((IntError)taskError).getIntErrorCode();
//		}
//		mLog.append(param);
//		mNeedTerminator = true;
//		limitLength();
//	}
	
	public synchronized void log(ParamTag tag, com.nexstreaming.app.common.task.Task.TaskError taskError ) {
		if(ENABLE_LOGGING) {
			Log.d(LOG_TAG,"diag log : " + tag.name() + " = " + taskError.getMessage(), taskError.getException());
		}
		mLog.append(tag.code);
		int param = 0;
		if( taskError instanceof IntError ) {
			param=((IntError)taskError).getIntErrorCode();
		}
		mLog.append(param);
		mNeedTerminator = true;
		limitLength();
	}
	
	private String mFrozen = null;
	
	public synchronized void freeze() {
		if( mFrozen==null ) {
			mFrozen = toString();
		}
	}
	
	public synchronized void unfreeze() {
		mFrozen = null;
	}
	
	public synchronized String toString() {
		
		if( mFrozen!=null ) {
			return mFrozen;
		}
		
		Random rnd = new Random();
		int scramble_offs = (rnd.nextInt()&0x0FFFFFFF) % 32;
		final String scramble_a = "AY71GR3BU6D0SKE84Q9PZLJVTCX2M5FWNH";
		final String scramble_b = "T24GMZAHC7W8D1LR6KBPUV50EJNQ3X9FYS";
		final String ctable     = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
		limitLength();
//		if( mLog.length() > MAX_LEN ) {
//			mLog.delete(0, mLog.length()-MAX_LEN);
//		}
		String str = mLog.toString();
		str = str.replace("0Z", "N");
		str = str.replace("1Z", "Q");
		str = str.replace("0F", "U");
		str = scramble(str, scramble_a, scramble_b, scramble_offs);
		str = VERSION_CODE + ctable.charAt(scramble_offs) + str;
		int hash_a = 0;
		int hash_b = 0;
		for(int i=0, len=str.length(); i<len; i++) {
			char c = str.charAt(i);
			hash_a *= 31;
			hash_a += c;
			hash_b *= 17;
			hash_b += c;
		}
		String full_str = ctable.charAt(hash_a&0x1F) + str + ctable.charAt(hash_b&0x1F);
		String result_str = "";
		for( int i=0, len=full_str.length(); i<len; i+=4 ) {
			if( result_str.length()>0 ) {
				result_str = result_str + "-";
			}
			if( len-i < 7 ) {
				result_str = result_str + full_str.substring(i);
				break;
			} else {
				result_str = result_str + full_str.substring(i, i+4);
			}
		}
		return result_str;
	}

	private String scramble(String str, String scramble_a, String scramble_b, int scramble_offs) {
		String result = "";
		for(int i=0, len=str.length(); i<len; i++) {
			char c = str.charAt(i);
			int idx = scramble_a.indexOf(c);
			if( idx < 0 ) {
				result = result + c;
			} else {
				result = result + scramble_b.charAt((idx+scramble_offs)%34);
				scramble_offs = (scramble_offs+idx)%34;
			}
		}
		return result;
	}



}
