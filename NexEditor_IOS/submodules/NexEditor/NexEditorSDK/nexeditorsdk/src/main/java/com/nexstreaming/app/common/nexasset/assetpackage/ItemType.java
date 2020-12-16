package com.nexstreaming.app.common.nexasset.assetpackage;

/**
 * Determines the implementation type of the item, as separate from the functional category of the
 * item.
 *
 * For many categories of item (such as 'audio'), there is only a single implementation type
 * avaialble (in this case, also 'audio'), and for such items there is a 1:1 relationship between
 * the category and implementation type.
 *
 * However, for some categories, such as 'overlay', multiple item types are possible, such as
 * 'overlay' and 'renderitem'.
 */
public enum ItemType {
    overlay("nex.overlay"),
    renderitem("nex.renderitem"),
    kedl("nex.kedl"),
    audio("nex.audio"),
    font("nex.font"),
    template("nex.template"),
    lut("nex.lut"),
    media("nex.media"),
    collage("nex.collage"),
    staticcollage("nex.staticcollage"),
    dynamiccollage("nex.dynamiccollage"),
    beat("nex.beat");
    private final String typeId;

    private ItemType(String typeId) {
        this.typeId = typeId;
    }

    public static ItemType fromId(String typeId) {
        for( ItemType itemType: values() ) {
            if( itemType.typeId.equals(typeId) )
                return itemType;
        }
        return null;
    }

    public String getTypeId() {
        return typeId;
    }
}
