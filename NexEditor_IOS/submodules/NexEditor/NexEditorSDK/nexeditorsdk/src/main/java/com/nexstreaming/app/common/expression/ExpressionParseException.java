package com.nexstreaming.app.common.expression;

public class ExpressionParseException extends Exception {

	private static final long serialVersionUID = 1L;

	public ExpressionParseException( String message ) {
		super( message );
	}

	public ExpressionParseException() {
		super();
	}

	public ExpressionParseException(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

	public ExpressionParseException(Throwable throwable) {
		super(throwable);
	}
	
}
