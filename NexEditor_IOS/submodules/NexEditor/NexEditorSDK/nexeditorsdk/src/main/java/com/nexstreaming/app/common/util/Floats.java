package com.nexstreaming.app.common.util;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public final class Floats
{
    public static final float[] toFloats( Collection<Float> c ) {
        if( c==null )
            return new float[0];
        float[] result = new float[c.size()];
        int i=0;
        for( Float v: c ) {
            result[i++] = (v==null?0f:v);
        }
        return result;
    }

    public static final List<Float> asList( float[] floats ) {
        List<Float> result = new ArrayList<>(floats.length);
        for( float f: floats )
            result.add(f);
        return result;
    }


}
