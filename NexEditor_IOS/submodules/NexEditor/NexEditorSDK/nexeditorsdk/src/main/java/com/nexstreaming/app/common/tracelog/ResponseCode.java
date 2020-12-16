package com.nexstreaming.app.common.tracelog;

import android.content.Context;

import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.app.common.util.DiagnosticLogger;

public enum ResponseCode implements Task.TaskError, DiagnosticLogger.IntError {

	SUCCESS(0),
	APPAUTH_FAILED(1),
	LICENSE_EXPIRED(2),
	DLLIMIT_EXCEEDED(3),
	NO_AVAILABLE_LICENSE(4),
	INVALID_APIVERSION(5),
	ERR_NORMAL(6),
	ERR_FATAL(7),
	INVALID_CODECID(8),
	INVALID_DEVICE(9),
	UPDATE_AVAILABLE(10),
	UPDATE_NOTAVAILABLE(11),
	APP_VERSION_EXISTED(12),
	NOTICE_AVAILABLE(13),
	NOTICE_NOTAVAILABLE(14),
	INVALID_PROMOCODE(15),
	EXPIRED_PROMOCODE(16),
	ALREADYINUSE_PROMOCODE(17),			// E-mail address already has a valid promo-code associated with it
	PROMOCODE_NONE(18),					// E-mail address has NO promo code associated with it
	PROMOCODE_ALREADY(19),				// E-mail address is already used by a different account
	ALREADY_IAP_REGISTERED(20),
	APP_BLOCKDATE(21),
	INVALID_SOUNDTRACKID(22),
	NO_MATCH(23),
	SOMETHING_WRONG(505),
	UNKNOWN;
	
	private final int value;
	private final boolean unknown;
	ResponseCode( int value ) {
		this.value = value;
		this.unknown = false;
	}
	ResponseCode() {
		this.value = 0;
		this.unknown = true;
	}
	
	public int value() {
		return this.value;
	}

	@Override
	public Exception getException() {
		return null;
	}

	@Override
	public String getMessage() {
		if( unknown ) {
			return "UNKNOWN";
		} else {
			return name() + " [" + value + "]";
		}
	}
	
	static ResponseCode fromValue(int value) {
		for( ResponseCode c: values() ) {
			if( c.value == value ) {
				return c;
			}
		}
		return UNKNOWN;
	}
	
	@Override
	public String getLocalizedMessage(Context context) {
		return getMessage();
	}
	@Override
	public int getIntErrorCode() {
		return value();
	}
	
}
