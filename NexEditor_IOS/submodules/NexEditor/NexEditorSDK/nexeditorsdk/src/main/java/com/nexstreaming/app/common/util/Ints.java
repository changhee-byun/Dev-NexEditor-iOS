package com.nexstreaming.app.common.util;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public final class Ints
{
	public static final int[] toInts( Collection<Integer> c ) {
		if( c==null )
			return new int[0];
		int[] result = new int[c.size()];
		int i=0;
		for( Integer v: c ) {
			result[i++] = (v==null?0:v);
		}
		return result;
	}

    public static final List<Integer> asList( int[] ints ) {
        List<Integer> result = new ArrayList<>(ints.length);
        for( int i: ints )
            result.add(i);
        return result;
    }

}
