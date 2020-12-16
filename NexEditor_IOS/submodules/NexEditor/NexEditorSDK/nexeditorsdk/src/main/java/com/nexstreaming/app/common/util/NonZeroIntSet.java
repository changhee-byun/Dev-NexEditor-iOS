package com.nexstreaming.app.common.util;

import android.support.annotation.NonNull;

import java.util.Collection;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Set;

public class NonZeroIntSet implements Set<Integer> {

    private int[] elements;

    public NonZeroIntSet() {
        elements = new int[8];
    }

    public NonZeroIntSet(int initialSize) {
        elements = new int[initialSize];
    }

    public boolean add(int e) {
        if(e==0)
            return false;
        int len = elements.length;
        int free = -1;
        for(int i=0; i<len; i++) {
            if( elements[i]==e )
                return false;
            else if( elements[i]==0 && free<0 )
                free=i;
        }
        if( free<0 ) {
            int expand = elements.length;
            if( expand < 0 ) expand = 1;
            if( expand > 4096 ) expand = 4096;
            int[] realloc = new int[elements.length + expand];
            System.arraycopy(elements,0,realloc,0,elements.length);
            realloc[elements.length] = e;
            elements = realloc;
        } else {
            elements[free]=e;
        }
        return true;
    }

    public boolean contains(int e) {
        if(e==0)
            return false;
        int len = elements.length;
        for(int i=0; i<len; i++) {
            if( elements[i]==e )
                return true;
        }
        return false;
    }

    public boolean remove(int e) {
        if(e==0)
            return false;
        int len = elements.length;
        for(int i=0; i<len; i++) {
            if( elements[i]==e ) {
                elements[i]=0;
                return true;
            }
        }
        return false;
    }

    @Override
    public boolean add(Integer object) {
        return add((int)object);
    }

    @Override
    public boolean addAll(Collection<? extends Integer> collection) {
        boolean mod = false;
        for( int i: collection )
            mod = mod | add(i);
        return mod;
    }

    @Override
    public void clear() {
        int len = elements.length;
        for( int i=0; i<len; i++ )
            elements[i] = 0;
    }

    @Override
    public boolean contains(Object object) {
        if( object instanceof Integer ) {
            return contains((int) object);
        } else {
            return false;
        }
    }

    @Override
    public boolean containsAll(Collection<?> collection) {
        for( Object o: collection ) {
            if( !contains(o) )
                return false;
        }
        return true;
    }

    @Override
    public boolean isEmpty() {
        int len = elements.length;
        for(int i=0; i<len; i++) {
            if( elements[i]!=0 )
                return false;
        }
        return true;
    }

    @NonNull
    @Override
    public Iterator<Integer> iterator() {
        int startIdx = -1;
        int len = elements.length;
        for(int i=0; i<len; i++) {
            if( elements[i]!=0 ) {
                startIdx = i;
                break;
            }
        }
        final int finalStartIdx = startIdx;
        return new Iterator<Integer>() {

            int nextIdx = finalStartIdx;
            int last = -1;

            @Override
            public boolean hasNext() {
                return nextIdx>=0;
            }

            @Override
            public Integer next() {
                if( nextIdx < 0 )
                    throw new NoSuchElementException();
                last = elements[nextIdx];
                int len = elements.length;
                while( nextIdx < len && elements[nextIdx]==0 )
                    nextIdx++;
                if( nextIdx>=len )
                    nextIdx = -1;
                return last;
            }

            @Override
            public void remove() {
                if( last < 0 )
                    throw new IllegalStateException();
                NonZeroIntSet.this.remove(last);
                last = -1;
            }
        };
    }

    @Override
    public boolean remove(Object object) {
        if( object instanceof Integer ) {
            return remove((int) object);
        } else {
            return false;
        }
    }

    @Override
    public boolean removeAll(Collection<?> collection) {
        boolean mod = false;
        for( Object e: collection )
            mod = mod | remove(e);
        return mod;
    }

    @Override
    public boolean retainAll(Collection<?> collection) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int size() {
        int size=0;
        int len = elements.length;
        for(int i=0; i<len; i++) {
            if( elements[i]!=0 )
                size++;
        }
        return size;
    }

    public int[] toIntArray() {
        int size = size();
        int len = elements.length;
        int[] a = new int[size];
        for( int i=0, j=0; i<len; i++ ) {
            if( elements[i]!=0 )
                a[j++] = elements[i];
        }
        return a;
    }

    @NonNull
    @Override
    public Object[] toArray() {
        int size = size();
        int len = elements.length;
        Integer[] a = new Integer[size];
        for( int i=0, j=0; i<len; i++ ) {
            if( elements[i]!=0 )
                a[j++] = elements[i];
        }
        return a;
    }

    @NonNull
    @Override
    public <T> T[] toArray(T[] array) {
        if( array instanceof Integer[] )
            return (T[]) toArray();
        else
            throw new ClassCastException();
    }
}
