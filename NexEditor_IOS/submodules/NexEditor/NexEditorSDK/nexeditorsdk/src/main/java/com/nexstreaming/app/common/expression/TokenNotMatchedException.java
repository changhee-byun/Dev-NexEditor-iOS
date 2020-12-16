package com.nexstreaming.app.common.expression;

public class TokenNotMatchedException extends Exception {

    private static final long serialVersionUID = 1L;

    public TokenNotMatchedException(int pos) {
        super("No matching token at: " + pos);
    }

}
