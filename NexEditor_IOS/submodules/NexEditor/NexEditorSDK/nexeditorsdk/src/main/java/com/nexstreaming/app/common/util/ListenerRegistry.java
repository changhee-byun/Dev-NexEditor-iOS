package com.nexstreaming.app.common.util;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

public class ListenerRegistry<LISTENER> {

    private List<WeakReference<LISTENER>> m_listeners = new ArrayList<WeakReference<LISTENER>>();

    public interface NotifyCB<LISTENER> {
        void notify( LISTENER listener );
    }

    public void register( LISTENER listener ) {
        unregister(null);
        m_listeners.add(new WeakReference<LISTENER>(listener));
    }

    public void unregister( LISTENER listenerToRemove ) {
        List<WeakReference<LISTENER>> toRemove = new ArrayList<WeakReference<LISTENER>>();
        for( WeakReference<LISTENER> ref: m_listeners ) {
            LISTENER listener = ref.get();
            if( listener==null || listener==listenerToRemove ) {
                toRemove.add(ref);
            }
        }
        m_listeners.removeAll(toRemove);
    }

    public void notifyListeners( NotifyCB<LISTENER> notifyCB ) {
        for( WeakReference<LISTENER> ref: m_listeners ) {
            LISTENER listener = ref.get();
            if( listener!=null ) {
                notifyCB.notify(listener);
            }
        }
    }

}
