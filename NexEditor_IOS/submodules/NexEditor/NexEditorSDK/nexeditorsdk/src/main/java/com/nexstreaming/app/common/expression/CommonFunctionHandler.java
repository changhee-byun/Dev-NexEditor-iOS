package com.nexstreaming.app.common.expression;

public class CommonFunctionHandler implements ExpressionFunctionHandler {

	@Override
	public Object execFunction(String name, Object... param) {
		if( name.equals("min") ) {
			if( param.length < 1 ) {
				return null;
			}
			double doubleResult = ((Number)param[0]).doubleValue();
			int intResult = ((Number)param[0]).intValue();
			for( int i=1; i<param.length; i++ ) {
				doubleResult = Math.min(doubleResult,((Number)param[0]).doubleValue());
				intResult = Math.min(intResult,((Number)param[0]).intValue());
			}
			if( ((double)intResult)==doubleResult ) {
				return intResult;
			} else {
				return doubleResult;
			}
		} else if( name.equals("max") ) {
			if( param.length < 1 ) {
				return null;
			}
			double doubleResult = ((Number)param[0]).doubleValue();
			int intResult = ((Number)param[0]).intValue();
			for( int i=1; i<param.length; i++ ) {
				doubleResult = Math.max(doubleResult,((Number)param[0]).doubleValue());
				intResult = Math.max(intResult,((Number)param[0]).intValue());
			}
			if( ((double)intResult)==doubleResult ) {
				return intResult;
			} else {
				return doubleResult;
			}
		}
		if( param.length==1 ) {
			Object o = param[0];
			if( name.equals("sqrt") ) {
				return Math.sqrt(((Number)o).doubleValue());
			} else if( name.equals("cbrt") ) {
				return Math.cbrt(((Number)o).doubleValue());
			} else if( name.equals("abs") ) {
				if( o instanceof Integer ) {
					return Math.abs(((Number)o).intValue());
				} else {
					return Math.sin(((Number)o).doubleValue());
				}
			} else if( name.equals("chr") ) {
				return Character.toString((char)((Number)o).shortValue());
			} else if( name.equals("len") ) {
				return (int)(o.toString().length());
			} else if( name.equals("ord") ) {
				String s = o.toString();
				if( s.length()<1 ) {
					return (int)0;
				}
				return (int)(s.charAt(0));
			} else if( name.equals("sin") ) {
				return Math.sin(((Number)o).doubleValue());
			} else if( name.equals("sinh") ) {
				return Math.sinh(((Number)o).doubleValue());
			} else if( name.equals("cos") ) {
				return Math.cos(((Number)o).doubleValue());
			} else if( name.equals("cosh") ) {
				return Math.cosh(((Number)o).doubleValue());
			} else if( name.equals("ceil") ) {
				return Math.ceil(((Number)o).doubleValue());
			} else if( name.equals("floor") ) {
				return Math.floor(((Number)o).doubleValue());
			} else if( name.equals("tan") ) {
				return Math.tan(((Number)o).doubleValue());
			} else if( name.equals("tanh") ) {
				return Math.tanh(((Number)o).doubleValue());
			} else if( name.equals("asin") ) {
				return Math.asin(((Number)o).doubleValue());
			} else if( name.equals("acos") ) {
				return Math.acos(((Number)o).doubleValue());
			} else if( name.equals("atan") ) {
				return Math.atan(((Number)o).doubleValue());
			} else if( name.equals("exp") ) {
				return Math.exp(((Number)o).doubleValue());
			} else if( name.equals("log") ) {
				return Math.log(((Number)o).doubleValue());
			} else if( name.equals("log10") ) {
				return Math.log10(((Number)o).doubleValue());
			} else if( name.equals("log1p") ) {
				return Math.log1p(((Number)o).doubleValue());
			} else if( name.equals("int") ) {
				return ((Number)o).intValue();
			} else if( name.equals("double") ) {
				return ((Number)o).doubleValue();
			} else if( name.equals("str") ) {
				return o.toString();
			}
		} else if( param.length==2 ) {
			Object o1 = param[0];
			Object o2 = param[1];
			if( name.equals("tan") ) {
				return Math.atan2(((Number)o1).doubleValue(), ((Number)o2).doubleValue());
			} else if( name.equals("pow") ) {
				return Math.pow(((Number)o1).doubleValue(), ((Number)o2).doubleValue());
			}
		}
		return null;
	}

}
