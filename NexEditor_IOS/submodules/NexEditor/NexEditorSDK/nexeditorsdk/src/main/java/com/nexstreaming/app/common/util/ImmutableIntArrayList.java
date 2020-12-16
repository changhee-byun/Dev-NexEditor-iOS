package com.nexstreaming.app.common.util;

import java.util.AbstractList;

public class ImmutableIntArrayList extends AbstractList<Integer> {

    private final int[] array;

    public ImmutableIntArrayList(int... array) {
        if( array==null ) {
            throw new NullPointerException();
        }
        this.array = array;
    }

    @Override
    public Integer get(int location) {
        return array[location];
    }

    public int getPrimitive(int location) {
        return array[location];
    }

    @Override
    public int size() {
        return array.length;
    }
}
