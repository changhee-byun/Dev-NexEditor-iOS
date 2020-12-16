package com.nexstreaming.app.common.nexasset.assetpackage;

/**
 * UI category for the item.
 *
 * This determines the role that the item plays from the user's point of view.
 *
 * For example, a RenderItem technically is capable of being used as an effect,
 * transition, or overlay, but in practice each item is designed with a particular
 * use-case in mind and should appear only in that part of the app UI.
 *
 * The ItemCategory determines the functional (from a user's point of view) use
 * case for an item, as separate from the technical implementation of the item
 * (for the latter, see ItemType instead).
 */
public enum ItemCategory {
    audio,
    audiofilter,
    background,
    effect,
    filter,
    font,
    overlay,
    template,
    transition,
    collage,
    staticcollage,
    dynamiccollage,
    beattemplate
}
