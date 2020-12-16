package com.nexstreaming.nexeditorsdk.module;

import android.support.annotation.NonNull;

/**
 *
 * @since 2.0.0
 */
public interface nexModuleProvider {

    /**
     *
     * @return
     * @since 2.0.0
     */
    @NonNull
    public String name();

    /**
     *
     * @return
     * @since 2.0.0
     */
    @NonNull
    public String uuid();

    /**
     *
     * @return
     * @since 2.0.0
     */
    @NonNull
    public String description();

    /**
     *
     * @return
     * @since 2.0.0
     */
    @NonNull
    public String auth();

    /**
     *
     * @return
     * @since 2.0.0
     */
    @NonNull
    public String format();

    /**
     *
     * @return
     * @since 2.0.0
     */
    public int version();

    /**
     *
     * @return
     * @since 2.0.0
     */
    public UserField[] userFields();

}
