package com.nexstreaming.app.common.expression;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import com.nexstreaming.app.common.expression.Tokenizer.CharMatcher;
import com.nexstreaming.app.common.expression.Tokenizer.QuotedStringMatcher;
import com.nexstreaming.app.common.expression.Tokenizer.StringMatcher;
import com.nexstreaming.app.common.expression.Tokenizer.SymbolMatcher;
import com.nexstreaming.app.common.expression.Tokenizer.TokenMaker;

public class Expression {

    private static Tokenizer<ExpressionToken,ExpressionContext> sTokenizer = null;
    private final List<ExpressionToken> mExpressionTokens;
    private final ExpressionContext mExpressionContext;
    private Literal<?> mResult;
    private boolean mEnableDebugLogging = false;
    private ExpressionSymbolHandler mSymbolHandler = null;
    private ExpressionFunctionHandler mFunctionHandler = null;

    private static void initTokenizer() {
        if( sTokenizer != null ) {
            return;
        }
        sTokenizer = new Tokenizer<Expression.ExpressionToken,ExpressionContext>();
        sTokenizer.register(Tokenizer.WHITESPACE, sTokenizer.SKIP);
        sTokenizer.register(new SymbolMatcher("true"), new OpTokenMaker(BOOL_TRUE));
        sTokenizer.register(new SymbolMatcher("false"), new OpTokenMaker(BOOL_FALSE));
        sTokenizer.register(new SymbolMatcher("and"), new OpTokenMaker(OP_AND));
        sTokenizer.register(new SymbolMatcher("or"), new OpTokenMaker(OP_OR));
        sTokenizer.register(new StringMatcher("!="), new OpTokenMaker(OP_NE));
        sTokenizer.register(new StringMatcher("=="), new OpTokenMaker(OP_EQ));
        sTokenizer.register(new StringMatcher("<="), new OpTokenMaker(OP_LE));
        sTokenizer.register(new StringMatcher(">="), new OpTokenMaker(OP_GE));
        sTokenizer.register(new StringMatcher("&&"), new OpTokenMaker(OP_AND));
        sTokenizer.register(new StringMatcher("||"), new OpTokenMaker(OP_OR));
        sTokenizer.register(new StringMatcher("<<"), new OpTokenMaker(OP_SHIFTLEFT));
        sTokenizer.register(new StringMatcher(">>>"), new OpTokenMaker(OP_SHIFTRIGHTU));
        sTokenizer.register(new StringMatcher(">>"), new OpTokenMaker(OP_SHIFTRIGHT));
        sTokenizer.register(new CharMatcher('&'), new OpTokenMaker(OP_BITAND));
        sTokenizer.register(new CharMatcher('|'), new OpTokenMaker(OP_BITOR));
        sTokenizer.register(new CharMatcher('^'), new OpTokenMaker(OP_BITXOR));
        sTokenizer.register(new CharMatcher('+'), new OpTokenMaker(OP_PLUS));
        sTokenizer.register(new CharMatcher('~'), new OpTokenMaker(OP_INV));
        sTokenizer.register(new CharMatcher('-'), new OpTokenMaker(OP_MINUS));
        sTokenizer.register(new CharMatcher('*'), new OpTokenMaker(OP_MUL));
        sTokenizer.register(new CharMatcher('/'), new OpTokenMaker(OP_DIV));
        sTokenizer.register(new CharMatcher('%'), new OpTokenMaker(OP_MOD));
        sTokenizer.register(new CharMatcher('<'), new OpTokenMaker(OP_LT));
        sTokenizer.register(new CharMatcher('>'), new OpTokenMaker(OP_GT));
        sTokenizer.register(new CharMatcher(','), new OpTokenMaker(PARAM_DELIM));
        sTokenizer.register(new CharMatcher('!'), new OpTokenMaker(OP_NOT));
        sTokenizer.register(new CharMatcher('('), new OpTokenMaker(PRECD_OPEN));
        sTokenizer.register(new CharMatcher(')'), new OpTokenMaker(PRECD_CLOSE));
        sTokenizer.register(new QuotedStringMatcher('"','\\'), new TokenMaker<Expression.ExpressionToken,ExpressionContext>() {
            @Override
            public ExpressionToken make(String s, ExpressionContext c) {
                return new StringLiteral(s.substring(1, s.length()-1).replace("\\\"", "\""));
            }
        });
//        sTokenizer.register(new CharMatcher('['), new OpTokenMaker(ARRAY_OPEN));
//        sTokenizer.register(new CharMatcher(']'), new OpTokenMaker(ARRAY_CLOSE));
//        sTokenizer.register(new CharMatcher('{'), new OpTokenMaker(DIC_OPEN));
//        sTokenizer.register(new CharMatcher('}'), new OpTokenMaker(DIC_CLOSE));
        sTokenizer.register(Tokenizer.SYMBOL, new TokenMaker<Expression.ExpressionToken,ExpressionContext>() {
            @Override
            public ExpressionToken make(String s, ExpressionContext c) {
            	SymbolOperand sym = c.symbolTable.get(s);
            	if( sym==null ) {
            		sym = new SymbolOperand(s);
            		c.symbolTable.put(s, sym);
            	}
                return sym;
            }
        });
        sTokenizer.register(Tokenizer.DECIMAL_FLOAT_ONLY, new TokenMaker<Expression.ExpressionToken, ExpressionContext>() {
            @Override
            public ExpressionToken make(String s, ExpressionContext c) {
                return new DoubleLiteral(Double.parseDouble(s));
            }
        });
        sTokenizer.register(Tokenizer.DECIMAL_INTEGER, new TokenMaker<Expression.ExpressionToken, ExpressionContext>() {
            @Override
            public ExpressionToken make(String s, ExpressionContext c) {
                return new IntegerLiteral(Integer.parseInt(s));
            }
        });
    }

    public Expression( String expr ) throws ExpressionParseException {

        initTokenizer();
        List<ExpressionToken> tokens;
        ExpressionContext expressionContext = new ExpressionContext();
		try {
			tokens = sTokenizer.tokenize(expr,expressionContext);
		} catch (TokenNotMatchedException e) {
			throw new ExpressionParseException("Unexpected character encountered", e);
		}
        List<ExpressionToken> output = new ArrayList<Expression.ExpressionToken>(tokens.size());
        Deque<ExpressionToken> stack = new ArrayDeque<Expression.ExpressionToken>(tokens.size());
        
        ExpressionToken prevToken = null;

        for( int i=0; i<tokens.size()-1; i++ ) {
        	ExpressionToken t = tokens.get(i);
        	ExpressionToken next = tokens.get(i+1);
        	if( t instanceof SymbolOperand && next==PRECD_OPEN ) {
        		tokens.set(i, new FunctionOperator(((SymbolOperand)t).symbolName));
        	}
        }

        for( ExpressionToken t: tokens ) {
        	if( t instanceof MultiOperator ) {
        		if( prevToken!=null && (prevToken instanceof CloseParen || prevToken instanceof Operand )) {
        			t = ((MultiOperator)t).getBinary();
        		} else {
        			t = ((MultiOperator)t).getUnary();
        		}
        	}
        	if( t instanceof Operand ) {
        		output.add(t);
        	} else if ( t instanceof FunctionOperator  ) {
        		stack.push(t);
        	} else if ( t instanceof Delimeter ) {
        		while( stack.size()>0 && !(stack.peek() instanceof OpenParen ) ) {
        			output.add(stack.pop());
        		}
        		if( stack.size()<1 || !(stack.peek() instanceof OpenParen ) ) {
        			throw new ExpressionParseException("Unbalanced parenthesis");
        		}
        		((OpenParen)stack.peek()).argCount++;
        	} else if ( t instanceof Operator ) {
        		Operator o1 = (Operator) t;
        		while( stack.peek() instanceof Operator ) {
        			Operator o2 = (Operator)stack.peek();
        			if( (o1.getAssoc()==Assoc.LEFT && o1.getPrecd().level==o2.getPrecd().level)
        					|| o1.getPrecd().level < o2.getPrecd().level ) {
            			output.add(stack.pop());
        			} else {
        				break;
        			}
        		}
				stack.push(o1);
        	} else if ( t instanceof OpenParen ) {
        		stack.push(t);
        	} else if ( t instanceof CloseParen ) {
        		int xargs = 1;
        		if( prevToken instanceof OpenParen ) {
        			xargs = 0;
        		}
        		CloseParen closeParen = (CloseParen)t;
        		while( stack.size()>0 && !(stack.peek() instanceof OpenParen ) ) {
        			output.add(stack.pop());
        		}
        		if( stack.size()<1 || !(stack.peek() instanceof OpenParen ) ) {
        			throw new ExpressionParseException("Unbalanced parenthesis");
        		}
        		OpenParen openParen = (OpenParen)stack.pop();
        		if( !openParen.matches(closeParen) ) {
        			throw new ExpressionParseException("Mismatched parenthesis");
        		}
        		if( stack.peek() instanceof FunctionOperator ) {
        			((FunctionOperator)stack.peek()).argCount = openParen.argCount + xargs;
        		}
        		// TODO: Check if openParam has an associated auto function, and if so, push it
        		if( stack.peek() instanceof UnaryOperator ) {
        			output.add(stack.pop());
        		}
        	} else {
        		throw new ExpressionParseException("Bad state : " + t.toString());
        	}
        	prevToken = t;
        }

        while( stack.size()>0 ) {
        	ExpressionToken t = stack.pop();
        	if( t instanceof Paren ) {
    			throw new ExpressionParseException("Mismatched parenthesis");
        	}
        	output.add(t);
        }

        mExpressionTokens = Collections.unmodifiableList(output);
        mExpressionContext = expressionContext;

    }

    public void set(String symbolName, int value ) {
    	SymbolOperand sym = mExpressionContext.symbolTable.get(symbolName);
    	if(sym!=null) {
    		sym.value = new IntegerLiteral(value);
    	}
    }

    public void set(String symbolName, double value ) {
    	SymbolOperand sym = mExpressionContext.symbolTable.get(symbolName);
    	if(sym!=null) {
    		sym.value = new DoubleLiteral(value);
    	}
    }

    public void setSymbolHandler( ExpressionSymbolHandler symbolHandler ) {
    	mSymbolHandler = symbolHandler;
    }

    public void setFunctionHandler( ExpressionFunctionHandler functionHandler ) {
    	mFunctionHandler = functionHandler;
    }

    public void setDebugLogging( boolean enableDebugLogging ) {
    	mEnableDebugLogging = enableDebugLogging;
    }

    public void eval() throws ExpressionEvalException {
    	if( mFunctionHandler == null ) {
    		mFunctionHandler = new CommonFunctionHandler();
    	}
    	mResult = null;
    	if( mSymbolHandler!=null ) {
    		for( Entry<String, SymbolOperand> e: mExpressionContext.symbolTable.entrySet() ) {
    			Object o = mSymbolHandler.lookupSymbol(e.getKey());
    			e.getValue().value = literalFromObject(o);
    		}
    	}
    	Deque<Literal<?>> stack = new ArrayDeque<Literal<?>>(32);
    	for( ExpressionToken t: mExpressionTokens ) {
    		t.exec(stack,mFunctionHandler);
    		if(mEnableDebugLogging) {
    			System.out.println(  "-> " + t + "  :  " + stack);
    		}
    	}
    	if( stack.size()!=1 ) {
    		throw new ExpressionEvalException("Malformed expression");
    	}
    	mResult = stack.pop();
    }

    public int getIntResult() {
    	try {
			return mResult.toInt();
		} catch (ExpressionEvalException e) {
			return 0;
		}
    }

    public String getStringResult() {
    	try {
			return mResult.toText();
		} catch (ExpressionEvalException e) {
			return null;
		}
    }

    public double getDoubleResult() {
    	try {
			return mResult.toDouble();
		} catch (ExpressionEvalException e) {
			return 0;
		}
    }

    public long getLongResult() {
    	try {
			return mResult.toLong();
		} catch (ExpressionEvalException e) {
			return 0;
		}
    }

    public boolean getBooleanResult() {
    	try {
			return mResult.toBoolean();
		} catch (ExpressionEvalException e) {
			return false;
		}
    }

    public Object getResult() {
		return mResult.getValue();
    }

    private abstract static class ExpressionToken {
        public abstract void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException;
    }

    private static abstract class Operand extends ExpressionToken {

    }

    private static Literal<?> literalFromObject( Object o ) {
    	if( o==null ) {
    		return null;
    	} else if( o instanceof Integer ) {
    		return new IntegerLiteral((Integer)o);
    	} else if( o instanceof Short ) {
    		return new IntegerLiteral((Short)o);
    	} else if( o instanceof Character ) {
    		return new IntegerLiteral((Character)o);
    	} else if( o instanceof Double ) {
    		return new DoubleLiteral((Double)o);
    	} else if( o instanceof String ) {
    		return new StringLiteral((String)o);
    	} else if( o instanceof Boolean ) {
    		return ((Boolean)o)?BOOL_TRUE:BOOL_FALSE;
    	} else {
    		return null;
    	}
    }

    private static abstract class Literal<T> extends Operand {
        protected T mValue;
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	stack.push(this);
        }
        public T getValue() {
            return mValue;
        }
        public Literal<?> add(Literal<?> operand) throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public Literal<?> sub(Literal<?> operand) throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public Literal<?> neg() throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public Literal<?> pos() throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type"); // TODO: Avoid repeating error messages
        }
        public Literal<?> not() throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public Literal<?> mul(Literal<?> operand) throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public Literal<?> div(Literal<?> operand) throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public Literal<?> mod(Literal<?> operand) throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public int compare(Literal<?> operand) throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public Literal<?> logical_and(Literal<?> operand) throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public Literal<?> logical_or(Literal<?> operand) throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public int toInt() throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public double toDouble() throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public boolean toBoolean() throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
        public String toText() throws ExpressionEvalException {
        	return mValue.toString();
        	//throw new ExpressionEvalException("Operation not supported for this type");
        }
        public long toLong() throws ExpressionEvalException {
        	throw new ExpressionEvalException("Operation not supported for this type");
        }
		public String toString() { return mValue.toString(); };
    }

    private static class StringLiteral extends Literal<String> {

        public StringLiteral(String value) {
            mValue = value;
        }

        @Override
        public Literal<?> add(Literal<?> operand)
        		throws ExpressionEvalException {
        	return new StringLiteral(mValue + operand.toText());
        }

        @Override
        public String toText() throws ExpressionEvalException {
        	return mValue;
        }

    }

    private static class IntegerLiteral extends Literal<Integer> {
        public IntegerLiteral(int value) {
            mValue = value;
        }

        @Override
        public Literal<?> add(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new IntegerLiteral(((IntegerLiteral)operand).mValue + mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(((DoubleLiteral)operand).mValue + mValue);
        	} else if( operand instanceof StringLiteral) {
        		return new StringLiteral(mValue + ((StringLiteral)operand).mValue);
        	} else {
        		return super.add(operand);
        	}
        }

        @Override
        public Literal<?> sub(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new IntegerLiteral(mValue - ((IntegerLiteral)operand).mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(mValue - ((DoubleLiteral)operand).mValue);
        	} else {
        		return super.sub(operand);
        	}
        }

        @Override
        public Literal<?> mul(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new IntegerLiteral(((IntegerLiteral)operand).mValue * mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(((DoubleLiteral)operand).mValue * mValue);
        	} else {
        		return super.mul(operand);
        	}
        }

        @Override
        public Literal<?> div(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new IntegerLiteral(mValue / ((IntegerLiteral)operand).mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(mValue / ((DoubleLiteral)operand).mValue);
        	} else {
        		return super.div(operand);
        	}
        }

        @Override
        public Literal<?> mod(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new IntegerLiteral(mValue % ((IntegerLiteral)operand).mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(mValue % ((DoubleLiteral)operand).mValue);
        	} else {
        		return super.mod(operand);
        	}
        }

        @Override
        public Literal<?> neg() throws ExpressionEvalException {
    		return new IntegerLiteral(-mValue);
        }

        @Override
        public Literal<?> pos() throws ExpressionEvalException {
        	return this;
        }

        @Override
        public int compare(Literal<?> operand) throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return mValue.compareTo((((IntegerLiteral)operand).mValue));
        	} else if( operand instanceof DoubleLiteral) {
        		return Double.valueOf((double)mValue).compareTo((((DoubleLiteral)operand).mValue));
        	} else {
        		return super.compare(operand);
        	}
        }

        @Override
        public int toInt() throws ExpressionEvalException {
        	return mValue;
        }

        @Override
        public double toDouble() throws ExpressionEvalException {
        	return mValue;
        }

        @Override
        public long toLong() throws ExpressionEvalException {
        	return mValue;
        }

        @Override
        public String toText() throws ExpressionEvalException {
        	return mValue.toString();
        }

    }

    private static class BooleanLiteral extends Literal<Boolean> {
        public BooleanLiteral(boolean value) {
            mValue = value;
        }
        @Override
        public boolean toBoolean() throws ExpressionEvalException {
        	return mValue;
        }
        @Override
        public int toInt() throws ExpressionEvalException {
        	return mValue?1:0;
        }
        @Override
        public String toText() throws ExpressionEvalException {
        	return mValue?"true":"false";
        }
        @Override
        public Literal<?> add(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof StringLiteral) {
        		return new StringLiteral((mValue?"true":"false") + ((StringLiteral)operand).mValue);
        	} else {
        		return super.add(operand);
        	}
        }
        @Override
        public Literal<?> logical_and(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof BooleanLiteral ) {
        		return (((BooleanLiteral)operand).mValue && mValue)?BOOL_TRUE:BOOL_FALSE;
        	} else {
        		return super.logical_and(operand);
        	}
        }
        @Override
        public Literal<?> logical_or(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof BooleanLiteral ) {
        		return (((BooleanLiteral)operand).mValue || mValue)?BOOL_TRUE:BOOL_FALSE;
        	} else {
        		return super.logical_and(operand);
        	}
        }
        @Override
        public Literal<?> not() throws ExpressionEvalException {
        	return mValue?BOOL_FALSE:BOOL_TRUE;
        }
    }

    private static final BooleanLiteral BOOL_TRUE = new BooleanLiteral(true);
    private static final BooleanLiteral BOOL_FALSE = new BooleanLiteral(false);

    private static class DoubleLiteral extends Literal<Double> {
        public DoubleLiteral(double value) {
            mValue = value;
        }

        @Override
        public Literal<?> add(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new DoubleLiteral(((IntegerLiteral)operand).mValue + mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(((DoubleLiteral)operand).mValue + mValue);
        	} else if( operand instanceof StringLiteral) {
        		return new StringLiteral(mValue + ((StringLiteral)operand).mValue);
        	} else {
        		return super.add(operand);
        	}
        }

        @Override
        public Literal<?> sub(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new DoubleLiteral(mValue - ((IntegerLiteral)operand).mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(mValue - ((DoubleLiteral)operand).mValue);
        	} else {
        		return super.sub(operand);
        	}
        }

        @Override
        public Literal<?> mul(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new DoubleLiteral(((IntegerLiteral)operand).mValue * mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(((DoubleLiteral)operand).mValue * mValue);
        	} else {
        		return super.mul(operand);
        	}
        }

        @Override
        public Literal<?> div(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new DoubleLiteral(mValue / ((IntegerLiteral)operand).mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(mValue / ((DoubleLiteral)operand).mValue);
        	} else {
        		return super.div(operand);
        	}
        }

        @Override
        public Literal<?> mod(Literal<?> operand)
        		throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return new DoubleLiteral(mValue % ((IntegerLiteral)operand).mValue);
        	} else if( operand instanceof DoubleLiteral) {
        		return new DoubleLiteral(mValue % ((DoubleLiteral)operand).mValue);
        	} else {
        		return super.mod(operand);
        	}
        }

        @Override
        public Literal<?> neg() throws ExpressionEvalException {
    		return new DoubleLiteral(-mValue);
        }

        @Override
        public Literal<?> pos() throws ExpressionEvalException {
        	return this;
        }

        @Override
        public int compare(Literal<?> operand) throws ExpressionEvalException {
        	if( operand instanceof IntegerLiteral) {
        		return mValue.compareTo((((IntegerLiteral)operand).mValue.doubleValue()));
        	} else if( operand instanceof DoubleLiteral) {
        		return mValue.compareTo((((DoubleLiteral)operand).mValue));
        	} else {
        		return super.compare(operand);
        	}
        }

        @Override
        public int toInt() throws ExpressionEvalException {
        	return mValue.intValue();
        }

        @Override
        public double toDouble() throws ExpressionEvalException {
        	return mValue;
        }

        @Override
        public long toLong() throws ExpressionEvalException {
        	return mValue.longValue();
        }

        @Override
        public String toText() throws ExpressionEvalException {
        	return mValue.toString();
        }
    }

    private static class SymbolOperand extends Operand {
        public final String symbolName;
        public Literal<?> value;
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	if( value==null ) {
        		throw new ExpressionEvalException("Undefined symbol: " + symbolName);
        	}
        	stack.push(value);
        }
        SymbolOperand( String symbolName ) {
            this.symbolName = symbolName;
        }
        @Override
        public String toString() {
        	return symbolName;
        }
    }

    private static class OpTokenMaker implements TokenMaker<Expression.ExpressionToken,ExpressionContext> {

        private final ExpressionToken mToken;

        public OpTokenMaker( ExpressionToken token ) {
            mToken = token;
        }

        @Override
        public ExpressionToken make(String s, ExpressionContext context) {
            return mToken;
        }

    }

    private static abstract class Delimeter extends ExpressionToken {
    }

    private static abstract class Operator extends ExpressionToken {
    	public abstract Assoc getAssoc();
    	public abstract Precd getPrecd();
    }

    private static abstract class UnaryOperator extends Operator {
    }

    private static enum Assoc {
    	LEFT, RIGHT
    }

    private static enum Precd {
    	UNARYFUNC(14),
    	UNARYNEG(13),
    	EXPONENT(12),
    	MULTIPLICATIVE(11),
    	ADDITIVE(10),
    	SHIFT(9),
    	BITWISE(8),
    	RELATIONAL(7),
    	EQUALITY(6),
    	UNARYNOT(5),
    	LOGICAL_AND(4),
    	LOGICAL_OR(3);

    	public final int level;
    	Precd( int level ) {
    		this.level = level;
    	}
    }

    private static abstract class BinaryOperator extends Operator {
    }

    private static abstract class Paren extends ExpressionToken {
        private final char mOpenChar;
        private final char mCloseChar;
        private final boolean mIsOpen;
        protected Paren( char open, char close, boolean isOpen ) {
            mOpenChar = open;
            mCloseChar = close;
            mIsOpen = isOpen;
        }
        public char getChar() {
            return mIsOpen?mOpenChar:mCloseChar;
        }
        public char getMatchingChar() {
            return mIsOpen?mCloseChar:mOpenChar;
        }
//        public char getOpenChar() {
//            return mOpenChar;
//        }
//        public char getCloseChar() {
//            return mCloseChar;
//        }
//        public boolean isOpen() {
//            return mIsOpen;
//        }
        public boolean matches( Paren p ) {
            if( p!=null && p.getChar()==getMatchingChar()) {
                return true;
            } else {
                return false;
            }
        }
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	throw new RuntimeException("Bad state : attempt to execute paren.");
        }
    }

    private static class OpenParen extends Paren {
    	public int argCount;
		protected OpenParen(char open, char close) {
			super(open, close, true);
		}
    }

    private static class CloseParen extends Paren {
		protected CloseParen(char open, char close) {
			super(open, close, false);
		}
    }

    private static class MultiOperator extends Operator {
        private final UnaryOperator mUnary;
        private final BinaryOperator mBinary;
    	public Assoc getAssoc() {
    		throw new IllegalStateException();
    	}
    	public Precd getPrecd() {
    		throw new IllegalStateException();
    	}
        MultiOperator( UnaryOperator unop, BinaryOperator binop ) {
            mUnary = unop;
            mBinary = binop;
        }
        public UnaryOperator getUnary() {
            return mUnary;
        }
        public BinaryOperator getBinary() {
            return mBinary;
        }
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	throw new RuntimeException("Bad state : attempt to execute multi-op (should have been resolved first).");
        }
    }

    private static final BinaryOperator OP_ADD = new BinaryOperator() {

		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.ADDITIVE;
		}

		@Override
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.add(right));
        }

		public String toString() { return "+"; };

    };

    private static class FunctionOperator extends UnaryOperator {

    	public int argCount = 0;

        public final String symbolName;
        FunctionOperator( String symbolName ) {
            this.symbolName = symbolName;
        }

		@Override
		public Assoc getAssoc() {
			return Assoc.RIGHT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.UNARYFUNC;
		}

		@Override
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
			if( functionHandler==null ) {
				throw new ExpressionEvalException("Undefined function: " + symbolName);
			}
			Object[] params = new Object[argCount];
			for( int i=argCount-1; i>=0; i-- ) {
				params[i] = stack.pop().mValue;
			}
			Literal<?> result = literalFromObject(functionHandler.execFunction(symbolName, params));
			if( result==null ) {
				throw new ExpressionEvalException("Undefined or broken function: " + symbolName);
			}
        	stack.push(result);
        }
		public String toString() { return symbolName + "(" + argCount + ")"; };
    }

    private static final UnaryOperator OP_POS = new UnaryOperator() {

		@Override
		public Assoc getAssoc() {
			return Assoc.RIGHT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.UNARYNEG;
		}

		@Override
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	stack.push(stack.pop().pos());
        }
		public String toString() { return "[+]"; };
    };

    private static final UnaryOperator OP_NOT = new UnaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.RIGHT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.UNARYNOT;
		}
		@Override
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	stack.push(stack.pop().not());
        }
		public String toString() { return "!"; };
    };

    private static final BinaryOperator OP_SUB = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.ADDITIVE;
		}
		@Override
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.sub(right));
        }

		public String toString() { return "-"; };

    };

    private static final UnaryOperator OP_NEG = new UnaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.RIGHT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.UNARYNEG;
		}

		@Override
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	stack.push(stack.pop().neg());
        }
		public String toString() { return "[-]"; };
    };

    private static final BinaryOperator OP_MUL = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.MULTIPLICATIVE;
		}
		@Override
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.mul(right));
        }

		public String toString() { return "*"; };

    };

    private static final BinaryOperator OP_DIV = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.MULTIPLICATIVE;
		}
		@Override
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.div(right));
        }

		public String toString() { return "/"; };

    };

    private static final BinaryOperator OP_MOD = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.MULTIPLICATIVE;
		}
		@Override
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.mod(right));
        }
		public String toString() { return "%"; };
    };

    private static final BinaryOperator OP_LT = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.RELATIONAL;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.compare(right) < 0?BOOL_TRUE:BOOL_FALSE);
        }
		public String toString() { return "<"; };
    };

    private static final BinaryOperator OP_GT = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.RELATIONAL;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.compare(right) > 0?BOOL_TRUE:BOOL_FALSE);
        }
		public String toString() { return ">"; };
    };

    private static final BinaryOperator OP_LE = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.RELATIONAL;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.compare(right) <= 0?BOOL_TRUE:BOOL_FALSE);
        }
		public String toString() { return "<="; };
    };

    private static final BinaryOperator OP_GE = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.RELATIONAL;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.compare(right) >= 0?BOOL_TRUE:BOOL_FALSE);
        }
		public String toString() { return ">="; };
    };

    private static final BinaryOperator OP_EQ = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.EQUALITY;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.compare(right) == 0?BOOL_TRUE:BOOL_FALSE);
        }
		public String toString() { return "=="; };
    };

    private static final BinaryOperator OP_NE = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.EQUALITY;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.compare(right) != 0?BOOL_TRUE:BOOL_FALSE);
        }
		public String toString() { return "!="; };
    };

    private static final BinaryOperator OP_AND = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.LOGICAL_AND;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.logical_and(right));
        }
		public String toString() { return "&&"; };
    };

    private static final BinaryOperator OP_OR = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.LOGICAL_OR;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(left.logical_or(right));
        }
		public String toString() { return "||"; };
    };

    private static final BinaryOperator OP_SHIFTRIGHTU = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.SHIFT;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(new IntegerLiteral(left.toInt() >>> right.toInt()));
        }
		public String toString() { return ">>>"; };
    };


    private static final BinaryOperator OP_SHIFTRIGHT = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.SHIFT;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(new IntegerLiteral(left.toInt() >> right.toInt()));
        }
		public String toString() { return ">>"; };
    };

    private static final BinaryOperator OP_SHIFTLEFT = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.SHIFT;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(new IntegerLiteral(left.toInt() << right.toInt()));
        }
		public String toString() { return "<<"; };
    };

    private static final UnaryOperator OP_INV = new UnaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.UNARYNOT;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	stack.push(new IntegerLiteral(~(stack.pop().toInt())));
        }
		public String toString() { return "~"; };
    };


    private static final BinaryOperator OP_BITOR = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.BITWISE;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(new IntegerLiteral(left.toInt() | right.toInt()));
        }
		public String toString() { return "|"; };
    };

    private static final BinaryOperator OP_BITXOR = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.BITWISE;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(new IntegerLiteral(left.toInt() ^ right.toInt()));
        }
		public String toString() { return "^"; };
    };

    private static final BinaryOperator OP_BITAND = new BinaryOperator() {
		@Override
		public Assoc getAssoc() {
			return Assoc.LEFT;
		}

		@Override
		public Precd getPrecd() {
			return Precd.BITWISE;
		}
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	Literal<?> right = stack.pop();
        	Literal<?> left = stack.pop();
        	stack.push(new IntegerLiteral(left.toInt() & right.toInt()));
        }
		public String toString() { return "&"; };
    };


    private static final Delimeter PARAM_DELIM = new Delimeter() {
        public void exec(Deque<Literal<?>> stack, ExpressionFunctionHandler functionHandler) throws ExpressionEvalException {
        	throw new RuntimeException("Bad state : attempt to execute delimeter.");
        }
    };


    private static final MultiOperator OP_PLUS = new MultiOperator(OP_POS, OP_ADD);
    private static final MultiOperator OP_MINUS = new MultiOperator(OP_NEG, OP_SUB);

    private static final Paren PRECD_OPEN = new OpenParen('(', ')');
    private static final Paren PRECD_CLOSE = new CloseParen('(', ')');
//    private static final Paren ARRAY_OPEN = new OpenParen('[', ']');
//    private static final Paren ARRAY_CLOSE = new CloseParen('[', ']');
//    private static final Paren DIC_OPEN = new OpenParen('{', '}');
//    private static final Paren DIC_CLOSE = new CloseParen('{', '}');

    private static class ExpressionContext {

    	public Map<String,SymbolOperand> symbolTable = new HashMap<String,SymbolOperand>();;

    }

}
