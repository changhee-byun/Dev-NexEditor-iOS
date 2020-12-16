package com.nexstreaming.nexeditorsdk.module;

import android.graphics.Rect;

/**
 *
 * @since 2.0.0
 */
public interface UserField {

    /**
     *
     * @return
     * @since 2.0.0
     */
    public String key();

    /**
     *
     * @return
     * @since 2.0.0
     */
    public UserFieldType type();

    /**
     *
     * @return
     * @since 2.0.0
     */
    public Object max();

    /**
     *
     * @return
     * @since 2.0.0
     */
    public Object min();
}
