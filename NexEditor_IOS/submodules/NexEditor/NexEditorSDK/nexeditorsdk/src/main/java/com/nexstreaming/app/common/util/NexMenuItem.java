package com.nexstreaming.app.common.util;

import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.view.ActionProvider;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;

/**
 * Created by matthew on 1/30/15.
 */
public class NexMenuItem implements MenuItem {

    private int itemId;
    private int groupId;
    private int order;
    private CharSequence title;
    private CharSequence titleCondensed;
    private Context context;
    private Drawable icon;
    private Intent intent;
    private boolean checkable = false;
    private boolean checked = false;
    private boolean visible = true;
    private boolean enabled = true;
    private OnMenuItemClickListener menuItemClickListener;

    NexMenuItem(Context context) {
        this.context = context;
    }

    public NexMenuItem(Context context, int groupId, int itemId, int order) {
        this.context = context;
        this.groupId = groupId;
        this.itemId = itemId;
        this.order = order;
    }

    @Override
    public int getItemId() {
        return itemId;
    }

    @Override
    public int getGroupId() {
        return groupId;
    }

    @Override
    public int getOrder() {
        return order;
    }

    @Override
    public MenuItem setTitle(CharSequence title) {
        this.title = title;
        return this;
    }

    @Override
    public MenuItem setTitle(int title) {
        this.title = context.getString(title);
        return this;
    }

    @Override
    public CharSequence getTitle() {
        return title;
    }

    @Override
    public MenuItem setTitleCondensed(CharSequence title) {
        titleCondensed = title;
        return this;
    }

    @Override
    public CharSequence getTitleCondensed() {
        return titleCondensed;
    }

    @Override
    public MenuItem setIcon(Drawable icon) {
        this.icon = icon;
        return this;
    }

    @Override
    public MenuItem setIcon(int iconRes) {
        if(iconRes == 0)
            this.icon = null;
        else
            this.icon = context.getResources().getDrawable(iconRes);
        return this;
    }

    @Override
    public Drawable getIcon() {
        return icon;
    }

    @Override
    public MenuItem setIntent(Intent intent) {
        this.intent = intent;
        return this;
    }

    @Override
    public Intent getIntent() {
        return intent;
    }

    @Override
    public MenuItem setShortcut(char numericChar, char alphaChar) {
        return this;
//        throw new UnsupportedOperationException();
    }

    @Override
    public MenuItem setNumericShortcut(char numericChar) {
        return this;
//        throw new UnsupportedOperationException();
    }

    @Override
    public char getNumericShortcut() {
        throw new UnsupportedOperationException();
    }

    @Override
    public MenuItem setAlphabeticShortcut(char alphaChar) {
        return this;
//        throw new UnsupportedOperationException();
    }

    @Override
    public char getAlphabeticShortcut() {
        throw new UnsupportedOperationException();
    }

    @Override
    public MenuItem setCheckable(boolean checkable) {
        this.checkable = checkable;
        return this;
    }

    @Override
    public boolean isCheckable() {
        return checkable;
    }

    @Override
    public MenuItem setChecked(boolean checked) {
        this.checked= checked;
        return this;
    }

    @Override
    public boolean isChecked() {
        return checked;
    }

    @Override
    public MenuItem setVisible(boolean visible) {
        this.visible = visible;
        return this;
    }

    @Override
    public boolean isVisible() {
        return visible;
    }

    @Override
    public MenuItem setEnabled(boolean enabled) {
        this.enabled = enabled;
        return this;
    }

    @Override
    public boolean isEnabled() {
        return enabled;
    }

    @Override
    public boolean hasSubMenu() {
        return false;
    }

    @Override
    public SubMenu getSubMenu() {
        throw new UnsupportedOperationException();
    }

    @Override
    public MenuItem setOnMenuItemClickListener(OnMenuItemClickListener menuItemClickListener) {
        this.menuItemClickListener = menuItemClickListener;
        return this;
    }

    public void notifyOnClickListener() {
        if( menuItemClickListener!=null )
            menuItemClickListener.onMenuItemClick(this);
    }

    @Override
    public ContextMenu.ContextMenuInfo getMenuInfo() {
        throw new UnsupportedOperationException();
    }

    @Override
    public void setShowAsAction(int actionEnum) {
        throw new UnsupportedOperationException();
    }

    @Override
    public MenuItem setShowAsActionFlags(int actionEnum) {
        throw new UnsupportedOperationException();
    }

    @Override
    public MenuItem setActionView(View view) {
        throw new UnsupportedOperationException();
    }

    @Override
    public MenuItem setActionView(int resId) {
        throw new UnsupportedOperationException();
    }

    @Override
    public View getActionView() {
        throw new UnsupportedOperationException();
    }

    @Override
    public MenuItem setActionProvider(ActionProvider actionProvider) {
        throw new UnsupportedOperationException();
    }

    @Override
    public ActionProvider getActionProvider() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean expandActionView() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean collapseActionView() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean isActionViewExpanded() {
        throw new UnsupportedOperationException();
    }

    @Override
    public MenuItem setOnActionExpandListener(OnActionExpandListener listener) {
        throw new UnsupportedOperationException();
    }
}
