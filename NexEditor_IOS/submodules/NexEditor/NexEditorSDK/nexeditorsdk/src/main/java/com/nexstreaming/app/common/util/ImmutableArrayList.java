package com.nexstreaming.app.common.util;

import java.util.AbstractList;

public class ImmutableArrayList<E> extends AbstractList<E> {

    private final E[] array;

    public ImmutableArrayList(E... array) {
        if( array==null ) {
            throw new NullPointerException();
        }
        this.array = array;
    }

    @Override
    public E get(int location) {
        return array[location];
    }

    @Override
    public int size() {
        return array.length;
    }
}
