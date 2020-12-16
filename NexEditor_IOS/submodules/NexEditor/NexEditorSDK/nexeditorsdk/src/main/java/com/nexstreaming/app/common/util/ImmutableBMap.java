package com.nexstreaming.app.common.util;

import android.support.annotation.NonNull;

import java.util.AbstractMap;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class ImmutableBMap<K,V> implements Map<K,V> {
    private final K[] keys;
    private final V[] values;
    private List<K> keyList;
    private Set<K> keySet;
    private Set<V> valueSet;
    private List<V> valueList;
    private Set<Entry<K, V>> entrySet;
    public ImmutableBMap() {
        keys = (K[]) new Object[0];
        values = (V[]) new Object[0];
    }
    public ImmutableBMap(K k0, V v0) {
        keys = (K[]) new Object[]{k0};
        values = (V[]) new Object[]{v0};
    }
    public ImmutableBMap(K k0, V v0, K k1, V v1) {
        keys = (K[]) new Object[]{k0,k1};
        values = (V[]) new Object[]{v0,v1};
    }
    public ImmutableBMap(K k0, V v0, K k1, V v1, K k2, V v2) {
        keys = (K[]) new Object[]{k0,k1,k2};
        values = (V[]) new Object[]{v0,v1,v2};
    }
    public ImmutableBMap(K k0, V v0, K k1, V v1, K k2, V v2, K k3, V v3) {
        keys = (K[]) new Object[]{k0,k1,k2,k3};
        values = (V[]) new Object[]{v0,v1,v2,v3};
    }
    public ImmutableBMap(K k0, V v0, K k1, V v1, K k2, V v2, K k3, V v3, Object... additionalKeysAndValues) {
        int len = additionalKeysAndValues.length/2;
        keys = (K[]) new Object[len+4];
        values = (V[]) new Object[len+4];
        keys[0] = k0;
        values[0] = v0;
        keys[1] = k1;
        values[1] = v1;
        keys[2] = k2;
        values[2] = v2;
        keys[3] = k3;
        values[3] = v3;
        Class<?> kclass = k0.getClass();
        Class<?> vclass = v0.getClass();
        for(int i=0; i<len; i++) {
            Object k = additionalKeysAndValues[i*2];
            Object v = additionalKeysAndValues[i*2+1];
            if( !kclass.isInstance(k) ) {
                throw new ClassCastException("key of wrong class");
            }
            if( !vclass.isInstance(v) ) {
                throw new ClassCastException("value of wrong class");
            }
            keys[i+4] = (K) k;
            values[i+4] = (V) v;
        }
    }
    public List<K> keys() {
        if( keyList==null ) {
            keyList = Collections.unmodifiableList(Arrays.asList(keys));
        }
        return keyList;
    }

    @Override
    public void clear() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean containsKey(Object key) {
        for( K k: keys ) {
            if( k==key || k.equals(key) )
                return true;
        }
        return false;
    }

    @Override
    public boolean containsValue(Object value) {
        for( V v: values ) {
            if( v==value || v.equals(value) )
                return true;
        }
        return false;
    }

    @NonNull
    @Override
    public Set<Entry<K, V>> entrySet() {
        if( entrySet==null ) {
            Set<Entry<K, V>> tmp = new HashSet<>();
            for( int i=0; i<keys.length; i++ ) {
                tmp.add(new AbstractMap.SimpleImmutableEntry<K, V>(keys[i],values[i]));
            }
            entrySet = Collections.unmodifiableSet(tmp);
        }
        return entrySet;
    }

    @Override
    public V get(Object key) {
        for( int i=0; i<keys.length; i++ ) {
            if( keys[i]==key || keys[i].equals(key) )
                return values[i];
        }
        return null;
    }

    public K getKeyForValue(V value) {
        for( int i=0; i<values.length; i++ ) {
            if( values[i]==value || values[i].equals(value) )
                return keys[i];
        }
        return null;
    }


    @Override
    public boolean isEmpty() {
        return keys.length < 1;
    }

    @NonNull
    @Override
    public Set<K> keySet() {
        if( keySet==null ) {
            keySet = Collections.unmodifiableSet(new HashSet<>(keys()));
        }
        return keySet;
    }

    @Override
    public V put(K key, V value) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void putAll(Map<? extends K, ? extends V> map) {
        throw new UnsupportedOperationException();
    }

    @Override
    public V remove(Object key) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int size() {
        return keys.length;
    }

    public List<V> values() {
        if( valueList==null ) {
            valueList = Collections.unmodifiableList(Arrays.asList(values));
        }
        return valueList;
    }


}
