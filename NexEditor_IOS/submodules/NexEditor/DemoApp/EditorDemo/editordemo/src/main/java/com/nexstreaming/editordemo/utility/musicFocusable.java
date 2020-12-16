package com.nexstreaming.editordemo.utility;

/**
 * Created by mj.kong on 2015-03-31.
 */
public interface musicFocusable {
    /** Signals that audio focus was gained. */
    public void onGainedAudioFocus();

    /**
     * Signals that audio focus was lost.
     *
     * @param canDuck If true, audio can continue in "ducked" mode (low volume). Otherwise, all
     * audio must stop.
     */
    public void onLostAudioFocus(boolean canDuck);
}
