package com.nexstreaming.app.common.expression;

public class ExpressionEvalException extends Exception {

	private static final long serialVersionUID = 1L;

	public ExpressionEvalException() {
		super();
	}

	public ExpressionEvalException(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

	public ExpressionEvalException(String detailMessage) {
		super(detailMessage);
	}

	public ExpressionEvalException(Throwable throwable) {
		super(throwable);
	}
	
}
