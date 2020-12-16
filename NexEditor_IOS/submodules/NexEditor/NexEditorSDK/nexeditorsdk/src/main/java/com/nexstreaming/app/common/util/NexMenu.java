package com.nexstreaming.app.common.util;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SubMenu;

import java.util.ArrayList;
import java.util.List;

public class NexMenu implements Menu {

    private List<NexMenuItem> m_menuItems = new ArrayList<>();
    private Context context;

    public NexMenu(Context context) {
        this.context = context;
    }

    private Context getContext() {
        return context;
    }


    @Override
    public MenuItem add(CharSequence title) {
        NexMenuItem item = new NexMenuItem(getContext());
        item.setTitle(title);
        m_menuItems.add(item);
        return item;
    }

    @Override
    public MenuItem add(int titleRes) {
        NexMenuItem item = new NexMenuItem(getContext());
        item.setTitle(titleRes);
        m_menuItems.add(item);
        return item;
    }

    @Override
    public MenuItem add(int groupId, int itemId, int order, CharSequence title) {
        NexMenuItem item = new NexMenuItem(getContext(),groupId,itemId,order);
        item.setTitle(title);
        m_menuItems.add(item);
        return item;
    }

    @Override
    public MenuItem add(int groupId, int itemId, int order, int titleRes) {
        NexMenuItem item = new NexMenuItem(getContext(),groupId,itemId,order);
        item.setTitle(titleRes);
        m_menuItems.add(item);
        return item;
    }

    @Override
    public SubMenu addSubMenu(CharSequence title) {
        throw new UnsupportedOperationException();
    }

    @Override
    public SubMenu addSubMenu(int titleRes) {
        throw new UnsupportedOperationException();
    }

    @Override
    public SubMenu addSubMenu(int groupId, int itemId, int order, CharSequence title) {
        throw new UnsupportedOperationException();
    }

    @Override
    public SubMenu addSubMenu(int groupId, int itemId, int order, int titleRes) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int addIntentOptions(int groupId, int itemId, int order, ComponentName caller, Intent[] specifics, Intent intent, int flags, MenuItem[] outSpecificItems) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void removeItem(int id) {
        if( id==0 )
            throw new IllegalArgumentException("ID cannot be zero");
        List<MenuItem> toRemove = new ArrayList<>();
        for( MenuItem item: m_menuItems ) {
            if( item.getItemId()==id )
            {
                toRemove.add(item);
            }
        }
        m_menuItems.removeAll(toRemove);
    }

    @Override
    public void removeGroup(int groupId) {
        if( groupId==0 )
            throw new IllegalArgumentException("group ID cannot be zero");
        List<MenuItem> toRemove = new ArrayList<>();
        for( MenuItem item: m_menuItems ) {
            if( item.getGroupId()==groupId )
            {
                toRemove.add(item);
            }
        }
        m_menuItems.removeAll(toRemove);
    }

    @Override
    public void clear() {
        m_menuItems.clear();
    }

    @Override
    public void setGroupCheckable(int group, boolean checkable, boolean exclusive) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void setGroupVisible(int group, boolean visible) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void setGroupEnabled(int group, boolean enabled) {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean hasVisibleItems() {
        for( MenuItem item: m_menuItems ) {
            if( item.isVisible() )
            {
                return true;
            }
        }
        return false;
    }

    @Override
    public MenuItem findItem(int id) {
        if( id==0 )
            throw new IllegalArgumentException("ID cannot be zero");
        for( MenuItem item: m_menuItems ) {
            if( item.getItemId()==id )
                return item;
        }
        return null;
    }

    @Override
    public int size() {
        return m_menuItems.size();
    }

    @Override
    public MenuItem getItem(int index) {
        return m_menuItems.get(index);
    }

    @Override
    public void close() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean performShortcut(int keyCode, KeyEvent event, int flags) {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean isShortcutKey(int keyCode, KeyEvent event) {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean performIdentifierAction(int id, int flags) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void setQwertyMode(boolean isQwerty) {
        throw new UnsupportedOperationException();
    }
}
