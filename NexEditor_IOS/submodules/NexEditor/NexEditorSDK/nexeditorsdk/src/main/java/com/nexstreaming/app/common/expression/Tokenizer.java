package com.nexstreaming.app.common.expression;

import java.util.ArrayList;
import java.util.List;

import android.util.Pair;

public class Tokenizer<T,C> {
    
    private List<Pair<TokenMatcher,TokenMaker<T,C>>> mReg = new ArrayList<Pair<TokenMatcher,TokenMaker<T,C>>>();

    public interface TokenMatcher {
        int match(String s);      // Return number of initial characters of s that match, or 0 if none match
    }
    
    public interface TokenMaker<T,C> {
        T make(String s, C context);
    }
    
    public Tokenizer() {
    }
    
    public void register( TokenMatcher matcher, TokenMaker<T,C> maker ) {
        mReg.add(new Pair<TokenMatcher,TokenMaker<T,C>>(matcher,maker));
    }
    
    public final TokenMaker<T,C> SKIP = new TokenMaker<T,C>() {
        @Override
        public T make(String s, C context) {
            return null;
        }
    };
    
    public static final TokenMatcher DECIMAL_FLOAT_ONLY = new TokenMatcher(){
        @Override
        public int match(String s) {
            int strlen = s.length();
            boolean dfound = false;
            for( int i=0; i<strlen; i++) {
                char c = s.charAt(i);
                if( c >= '0' && c <= '9' ) {
                    continue;
                } else if ( c=='.' && !dfound ) {
                    dfound = true;
                    continue;
                }
                if( i==1 || !dfound ) {
                    return 0;
                }
                return i;
            }
            if( strlen==1 || !dfound ) {
                return 0;
            }
            return strlen;
        }
    };
    
    
    
    public static final TokenMatcher DECIMAL_FLOAT = new TokenMatcher(){
        @Override
        public int match(String s) {
            int strlen = s.length();
            boolean dfound = false;
            for( int i=0; i<strlen; i++) {
                char c = s.charAt(i);
                if( c >= '0' && c <= '9' ) {
                    continue;
                } else if ( c=='.' && !dfound ) {
                    dfound = true;
                    continue;
                }
                if( i==1 && dfound ) {
                    return 0;
                }
                return i;
            }
            if( strlen==1 && dfound ) {
                return 0;
            }
            return strlen;
        }
    };
    
    
    public static final TokenMatcher DECIMAL_INTEGER = new TokenMatcher(){
        @Override
        public int match(String s) {
            int strlen = s.length();
            for( int i=0; i<strlen; i++) {
                char c = s.charAt(i);
                if( c >= '0' && c <= '9' ) {
                    continue;
                }
                return i;
            }
            return strlen;
        }
    };
    
    public static class StringMatcher implements TokenMatcher {

        private final String mMatch;
        
        public StringMatcher( String match ) {
            mMatch = match;
        }
        
        @Override
        public int match(String s) {
            if( s.startsWith(mMatch) ) {
                return mMatch.length();
            } else {
                return 0;
            }
        }
        
    }
    
    public static class QuotedStringMatcher implements TokenMatcher {

        private final char mQuoteChar;
        private final char mEscapeChar;
        
        public QuotedStringMatcher( char quoteChar, char escapeChar ) {
        	mQuoteChar = quoteChar;
        	mEscapeChar = escapeChar;
        }
        
        public QuotedStringMatcher( char quoteChar ) {
        	mQuoteChar = quoteChar;
        	mEscapeChar = 0;
        }

        
        @Override
        public int match(String s) {
        	int len = s.length();
            if(len<2 || s.charAt(0)!=mQuoteChar ) {
            	return 0;
            }
            char c;
            for( int i=1; i<len; i++ ) {
            	c = s.charAt(i);
            	if( c==mQuoteChar ) {
            		if( mQuoteChar==mEscapeChar && i+1<len && s.charAt(i+1)==mQuoteChar ) {
            			i++;
            		} else {
            			return i+1;
            		}
            	} else if( mEscapeChar!=0 && c==mEscapeChar ) {
            		i++;
            	}
            }
        	return 0;
        }
        
    }
    
    
    public static class CharMatcher implements TokenMatcher {
        private final char mMatch;
        
        public CharMatcher( char match ) {
            mMatch = match;
        }
        
        @Override
        public int match(String s) {
            return (s.length()>0 && s.charAt(0)==mMatch)?1:0;
        }
    }
    
    private static boolean isSymbolStart( char c ) {
        return (c>='A' && c<='Z') || (c>='a' && c<='z') || c=='_';
    }

    private static boolean isSymbolContinue( char c ) {
        return (c>='0' && c<='9') || isSymbolStart(c); 
    }

    public static class SymbolMatcher implements TokenMatcher {

        private final String mMatch;
        private final int mLen;
        
        public SymbolMatcher() {
            mMatch = null;
            mLen = 0;
        }
        
        public SymbolMatcher( String match ) {
            mMatch = match;
            mLen = match.length();
        }
        
        @Override
        public int match(String s) {
            if( mMatch==null || mLen<1 ) {
                int strlen = s.length();
                if( strlen < 1 || !isSymbolStart(s.charAt(0)) ) {
                    return 0;
                }
                for( int i=1; i<strlen; i++) {
                    if( isSymbolContinue(s.charAt(i)) ) {
                        continue;
                    } else {
                        return i;
                    }
                }
                return strlen;
            } else {
                if( s.startsWith(mMatch) && (s.length()==mLen || !isSymbolContinue(s.charAt(mLen))) ) {
                    return mLen;
                } else {
                    return 0;
                }
            }
        }
        
    }
    
    public static final TokenMatcher SYMBOL = new SymbolMatcher();
    
    public static final TokenMatcher WHITESPACE = new TokenMatcher(){
        @Override
        public int match(String s) {
            int strlen = s.length();
            for( int i=0; i<strlen; i++) {
                switch( s.charAt(i) ) {
                case 0x0009: 
                case 0x000A: 
                case 0x000B: 
                case 0x000C: 
                case 0x000D: 
                case 0x0020: 
                case 0x0085: 
                case 0x00A0: 
                case 0x1680: 
                case 0x180E: 
                case 0x2000: 
                case 0x2001: 
                case 0x2002: 
                case 0x2003: 
                case 0x2004: 
                case 0x2005: 
                case 0x2006: 
                case 0x2007: 
                case 0x2008: 
                case 0x2009: 
                case 0x200A: 
                case 0x2028: 
                case 0x2029: 
                case 0x202F: 
                case 0x205F: 
                case 0x3000:
                    break;
                default:
                    return i;
                }
            }
            return strlen;
        }
    };
    
    public void skipWhitespace() {
        register(WHITESPACE, SKIP);
    }
    
    public List<T> tokenize( String str, C context ) throws TokenNotMatchedException {
        List<T> tokens = new ArrayList<T>();
        
        String s = str;
        while( s.length() > 0 ) {
            boolean found = false;
            for( Pair<TokenMatcher,TokenMaker<T,C>> p: mReg ) {
                int len = p.first.match(s);
                if( len > 0 ) {
                    T token = p.second.make(s.substring(0, len),context);
                    if( token!=null ) {
                        tokens.add(token);
                    }
                    s = s.substring(len);
                    found = true;
                    break;
                }
            }
            if( !found ) {
                throw new TokenNotMatchedException(str.length()-s.length());
            }
        }
        
        return tokens;
    }
    
}
