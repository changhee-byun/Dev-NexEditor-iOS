package com.nexstreaming.app.common.nexasset.assetpackage;

/**
 * These are common asset category aliases used in the asset store.
 *
 * It is important to note that there may be additional asset categories in the store that
 * are not covered by this list.
 *
 * For now, the UI display order for categories is the order given here in this
 * enum, following by any categories not covered by the enum in alphabetical order by their alias.
 * This should be changed in the future so that we get order information from the store.
 */
public enum AssetCategoryAlias {
    Effect,
    Transition,
    Font,
    Overlay,
    Audio,
    Template,
    ClipGraphics,
    TextEffect,
    Collage,
    StaticCollage,
    DynamicCollage,
    BeatTemplate
}
