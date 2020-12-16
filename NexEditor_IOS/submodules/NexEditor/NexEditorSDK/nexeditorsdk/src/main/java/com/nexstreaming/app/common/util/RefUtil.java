package com.nexstreaming.app.common.util;

import android.util.SparseArray;

import java.lang.ref.WeakReference;
import java.util.Iterator;
import java.util.Map;

/**
 * Simple utility class for working with collections of weak references.
 */
public class RefUtil {
    private RefUtil(){} // Prevent instantiation

    /**
     * Remove entries containing dead weak references from an iterable (collection, list, etc.)
     *
     * @param iterable
     * @param <T>
     */
    public static <T>void removeDeadReferences(Iterable<WeakReference<T>> iterable) {
        Iterator<WeakReference<T>> iter = iterable.iterator();
        while(iter.hasNext()) {
            WeakReference<T> ref = iter.next();
            if(ref==null || ref.get()==null)
                iter.remove();
        }
    }

    /**
     * Remove entries containing dead weak references (as the value) from a Map
     *
     * @param map
     * @param <K>
     * @param <V>
     */
    public static <K,V>void removeDeadReferences(Map<K,WeakReference<V>> map) {
        Iterator<Map.Entry<K,WeakReference<V>>> iter = map.entrySet().iterator();
        while(iter.hasNext()) {
            Map.Entry<K,WeakReference<V>> entry = iter.next();
            WeakReference<V> ref = entry.getValue();
            if(ref==null || ref.get()==null)
                iter.remove();
        }
    }
    /**
     * Remove entries containing dead weak references (as the value) from a SparseArray
     *
     * @param array
     * @param <T>
     */
    public static <T>void removeDeadReferences(SparseArray<WeakReference<T>> array) {
        int size = array.size();
        for( int i=0; i<size; i++ ) {
            WeakReference<T> ref = array.valueAt(i);
            if(ref==null || ref.get()==null) {
                array.removeAt(i);
                size--;
                i--;
            }
        }
    }

}
