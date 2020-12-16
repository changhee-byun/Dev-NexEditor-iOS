package com.nexstreaming.app.common.nexasset.assetpackage;

import android.content.Context;
import android.graphics.RectF;
import android.util.LruCache;
import android.util.Xml;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class XMLItemDefReader {

    private XMLItemDefReader(){}

    private static XMLItemDef EMPTY_ITEM_DEF = new XMLItemDef() {
        @Override
        public List<ItemParameterDef> getParameterDefinitions() {
            return Collections.emptyList();
        }

        @Override
        public int getTransitionOffset() {
            return 0;
        }

        @Override
        public int getTransitionOverlap() {
            return 0;
        }

        @Override
        public int getIntrinsicWidth() {
            return 0;
        }

        @Override
        public int getIntrinsicHeight() {
            return 0;
        }
    };

    private static LruCache<String,XMLItemDef> itemDefCache = new LruCache<>(100);

    public static XMLItemDef getItemDef(Context context, String itemId) throws XmlPullParserException, IOException {
        if( itemId==null )
            return EMPTY_ITEM_DEF;
        XMLItemDef cached = itemDefCache.get(itemId);
        if( cached!=null )
            return cached;
        ItemInfo itemInfo = AssetPackageManager.getInstance(context).getInstalledItemById(itemId);
        if( itemInfo==null )
            return EMPTY_ITEM_DEF;
        switch( itemInfo.getType() ) {
            case kedl:
            case renderitem:
            case overlay:
                AssetPackageReader reader = AssetPackageReader.readerForPackageURI(context,itemInfo.getPackageURI(),itemInfo.getAssetPackage().getAssetId());
                XMLItemDef result = parseItemDef(reader.openFile(itemInfo.getFilePath()));
                itemDefCache.put(itemId,result);
                return result;
            default:
                return EMPTY_ITEM_DEF;
        }
    }

    /**
     * Reads render item parameter definitions from RenderItem XML.
     *
     * This supports both the new render item parameter definition format as well as the
     * older KEDL user field definition format.
     *
     * @param in
     * @return
     * @throws XmlPullParserException
     * @throws IOException
     */
    public static XMLItemDef parseItemDef(InputStream in) throws XmlPullParserException, IOException {
        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false);
            parser.setInput(in, null);
            parser.nextTag();
            return parseItemDef(parser);
        } finally {
            in.close();
        }
    }

    public static XMLItemDef parseItemDef(XmlPullParser parser) throws XmlPullParserException, IOException {

        XMLItemDefImpl result = new XMLItemDefImpl();

        String tagName = parser.getName();
        if( parser.getEventType()!=XmlPullParser.START_TAG || !(tagName.equalsIgnoreCase("effect") || tagName.equalsIgnoreCase("renderitem") || tagName.equalsIgnoreCase("overlay")) ) {
            throw new XmlPullParserException( "expected <effect>, <overlay> or <renderitem>"+ parser.getPositionDescription());
        }

        if( tagName.equalsIgnoreCase("effect") ) {
            if( "transition".equalsIgnoreCase(parser.getAttributeValue(null, "type")) ) {
                result.transitionOffset = parseInt(parser.getAttributeValue(null, "effectoffset"), 100);
                String overlap = parser.getAttributeValue(null, "effectoverlap");
                if( overlap==null )
                    overlap = parser.getAttributeValue(null, "videooverlap");
                result.transitionOverlap = parseInt(overlap, 100);
            }
            result.width = parseInt(parser.getAttributeValue(null, "width"), 0);
            result.height = parseInt(parser.getAttributeValue(null, "height"), 0);
        } else if ( tagName.equalsIgnoreCase("renderitem") ) {
            if( "transition".equalsIgnoreCase(parser.getAttributeValue(null, "type")) ) {
                result.transitionOffset = parseInt(parser.getAttributeValue(null, "transitionoffset"), 100);
                result.transitionOverlap = parseInt(parser.getAttributeValue(null, "transitionoverlap"), 100);
            }
            result.width = parseInt(parser.getAttributeValue(null, "width"), 0);
            result.height = parseInt(parser.getAttributeValue(null, "height"), 0);
        }
//        effectoffset="100" effectoverlap="100"
        result.itemParameterDefs = new ArrayList<>();

        while( parser.next() != XmlPullParser.END_TAG ) {
            if( parser.getEventType()!=XmlPullParser.START_TAG )
                continue;
            String name = parser.getName();
            if( name.equalsIgnoreCase("parameter")) {
                result.itemParameterDefs.add(readParameter(parser));
            } else if( name.equalsIgnoreCase("userfield")) {
                ItemParameterDef itemParameterDef = readUserField(parser);
                if( itemParameterDef !=null )
                    result.itemParameterDefs.add(itemParameterDef);
            } else {
                skip(parser);
            }
        }

        return result;
    }

    private static ItemParameterDef readUserField(XmlPullParser parser) throws XmlPullParserException, IOException {
        parser.require(XmlPullParser.START_TAG, null, "userfield");
        ItemParameterDefImpl itemParameter = new ItemParameterDefImpl();
        itemParameter.id = parser.getAttributeValue(null,"id");
        itemParameter.defaultValue = parser.getAttributeValue(null, "default");
        itemParameter.maxLength = Integer.MAX_VALUE;
        itemParameter.isMultiline = (parseInt(parser.getAttributeValue(null, "maxlines"), 1)>1);
        itemParameter.isPrivate = false;
        itemParameter.minimumValue = 0;
        itemParameter.maximumValue = 100;
        itemParameter.stepSize = 1;
        itemParameter.bounds = parseRectF(parser.getAttributeValue(null, "step"), null);

        String label = parser.getAttributeValue(null,"label");
        if( label!=null ) {
            itemParameter.strings = new HashMap<>();
            itemParameter.strings.put("label",new HashMap<String, String>());
            itemParameter.strings.get("label").put("",label);
        }

        String typeStr = parser.getAttributeValue(null,"type");

        if( "selection".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.CHOICE;
        } else if( "color".equalsIgnoreCase(typeStr)  ) {
            itemParameter.type = ItemParameterType.RGB;
        } else if( "text".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.TEXT;
        } else if( "overlay".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.IMAGE;
        } else if( "typeface".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.TYPEFACE;
        } else if( "undefined".equalsIgnoreCase(typeStr) ) {
            skip(parser);
            return null;
        } else {
            throw new XmlPullParserException( "unrecognized parameter type"+ parser.getPositionDescription());
        }

        while( parser.next() != XmlPullParser.END_TAG ) {
            if( parser.getEventType()!=XmlPullParser.START_TAG )
                continue;
            String name = parser.getName();
            if( name.equalsIgnoreCase("fieldlabel")) {
                if( itemParameter.strings==null )
                    itemParameter.strings = new HashMap<>();
                String stringName = "label";
                String stringLang = parser.getAttributeValue(null,"locale");
                String text = parser.getAttributeValue(null,"value");
                skip(parser);
                if( stringLang!=null && text!=null ) {
                    Map<String, String> nameMap = itemParameter.strings.get(stringName);
                    if( nameMap==null ) {
                        nameMap = new HashMap<>();
                        itemParameter.strings.put(stringName,nameMap);
                    }
                    nameMap.put(stringLang,text);
                }
            } else if( name.equalsIgnoreCase("option")) {
                if( itemParameter.options==null ) {
                    itemParameter.options = new ArrayList<>();
                }
                itemParameter.options.add(readUserFieldOption(parser));
            } else if( name.equalsIgnoreCase("icon")) {
                if( itemParameter.iconPath!=null ) {
                    throw new XmlPullParserException( "multiple <icon> tags not allowed"+ parser.getPositionDescription());
                }
                itemParameter.iconPath = parser.getAttributeValue(null,"src");
            } else {
                skip(parser);
            }
        }

        return itemParameter; // TODO
    }

    private static class OptionImpl implements ItemParameterDef.Option {

        public Map<String, Map<String, String>> strings;
        public String iconPath;
        public String value;

        @Override
        public Map<String, Map<String, String>> getStrings() {
            return strings;
        }

        @Override
        public String getIconPath() {
            return iconPath;
        }

        @Override
        public String getValue() {
            return value;
        }
    }

    private static class XMLItemDefImpl implements XMLItemDef {

        private int transitionOffset;
        private int transitionOverlap;
        private int width;
        private int height;
        private List<ItemParameterDef> itemParameterDefs;

        @Override
        public List<ItemParameterDef> getParameterDefinitions() {
            return itemParameterDefs;
        }

        @Override
        public int getTransitionOffset() {
            return transitionOffset;
        }

        @Override
        public int getTransitionOverlap() {
            return transitionOverlap;
        }

        @Override
        public int getIntrinsicWidth() {
            return width;
        }

        @Override
        public int getIntrinsicHeight() {
            return height;
        }
    }
    private static class ItemParameterDefImpl implements ItemParameterDef {

        public ItemParameterType type;
        public String defaultValue;
        public String valueOff;
        public String valueOn;
        public String id;
        public int maxLength;
        public boolean isMultiline;
        public boolean isPrivate;
        public int minimumValue;
        public int maximumValue;
        public int stepSize;
        public RectF bounds;
        public Map<String, Map<String, String>> strings;
        public List<Option> options;
        public String iconPath;

        @Override
        public ItemParameterType getType() {
            return type;
        }

        @Override
        public String getDefaultValue() {
            return defaultValue;
        }

        @Override
        public String getOffValue() {
            return valueOff;
        }

        @Override
        public String getOnValue() {
            return valueOn;
        }

        @Override
        public String getId() {
            return getTypeTag()+ ":" + id;
        }

        private String getTypeTag() {
            switch(type) {
                case CHOICE:
                    return "selection";
                case SWITCH:
                    return "switch";
                case IMAGE:
                    return "image";
                case RANGE:
                    return "range";
                case RECT:
                    return "rect";
                case RGB:
                case RGBA:
                    return "color";
                case TEXT:
                    return "text";
                case TYPEFACE:
                    return "typeface";
                case XY:
                case XYZ:
                    return "point";
            }
            throw new IllegalStateException("Unknown type: " + String.valueOf(type));
        }

        @Override
        public int getMaxLength() {
            return maxLength;
        }

        @Override
        public boolean isMultiline() {
            return isMultiline;
        }

        @Override
        public boolean isPrivate() {
            return isPrivate;
        }

        @Override
        public int getMinimumValue() {
            return minimumValue;
        }

        @Override
        public int getMaximumValue() {
            return maximumValue;
        }

        @Override
        public int getStepSize() {
            return stepSize;
        }

        @Override
        public RectF getBounds() {
            return bounds;
        }

        @Override
        public Map<String, Map<String, String>> getStrings() {
            return strings;
        }

        @Override
        public List<Option> getOptions() {
            return options;
        }

        @Override
        public String getIconPath() {
            return iconPath;
        }
    }


    private static int parseInt( String value, int defaultValue ) {
        if( value==null )
            return defaultValue;
        try {
            return Integer.parseInt(value);
        } catch( NumberFormatException e) {
            return defaultValue;
        }
    }

    private static RectF parseRectF(String value, RectF defaultValue) {
        if( value==null )
            return defaultValue;
        try {
            String s[] = value.split(" +");
            if( s.length<4 )
                return defaultValue;
            return new RectF(Float.parseFloat(s[0]),Float.parseFloat(s[1]),Float.parseFloat(s[2]),Float.parseFloat(s[3]));
        } catch( NumberFormatException e) {
            return defaultValue;
        }
    }

    private static boolean parseBool( String value, boolean defaultValue ) {
        if( value==null ) {
            return defaultValue;
        } else if ("true".equalsIgnoreCase(value)) {
            return true;
        } else if ("false".equalsIgnoreCase(value)) {
            return false;
        } else {
            return defaultValue;
        }
    }

    private static ItemParameterDef readParameter(XmlPullParser parser) throws XmlPullParserException, IOException {
        parser.require(XmlPullParser.START_TAG, null, "parameter");
        ItemParameterDefImpl itemParameter = new ItemParameterDefImpl();
        itemParameter.id = parser.getAttributeValue(null,"id");
        itemParameter.defaultValue = parser.getAttributeValue(null, "default");
        itemParameter.maxLength = parseInt(parser.getAttributeValue(null, "maxlen"), Integer.MAX_VALUE);
        itemParameter.isMultiline = parseBool(parser.getAttributeValue(null, "multiline"), false);
        itemParameter.isPrivate = parseBool(parser.getAttributeValue(null, "private"), false);
        itemParameter.minimumValue = parseInt(parser.getAttributeValue(null, "minvalue"), 0);
        itemParameter.maximumValue = parseInt(parser.getAttributeValue(null, "maxvalue"), 100);
        itemParameter.stepSize = parseInt(parser.getAttributeValue(null, "step"), 1);
        itemParameter.bounds = parseRectF(parser.getAttributeValue(null, "bounds"), null);
        String typeStr = parser.getAttributeValue(null,"type");
        if( "choice".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.CHOICE;
        } else if( "point".equalsIgnoreCase(typeStr) ) {
            int dimensions = parseInt(parser.getAttributeValue(null, "dimensions"), 2);
            if( dimensions==2 ) {
                itemParameter.type = ItemParameterType.XY;
            } else if( dimensions==3 ) {
                itemParameter.type = ItemParameterType.XYZ;
            } else {
                throw new XmlPullParserException( "unsupported number of parameter dimensions: "+ dimensions);
            }
        } else if( "color".equalsIgnoreCase(typeStr) ) {
            if(parseBool(parser.getAttributeValue(null,"alpha"),false)) {
                itemParameter.type = ItemParameterType.RGBA;
            } else {
                itemParameter.type = ItemParameterType.RGB;
            }
        } else if( "range".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.RANGE;
        } else if( "rect".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.RECT;
        } else if( "text".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.TEXT;
        } else if( "switch".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.SWITCH;
            itemParameter.valueOff = parser.getAttributeValue(null, "off");
            itemParameter.valueOn = parser.getAttributeValue(null, "on");
        } else if( "image".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.IMAGE;
        } else if( "typeface".equalsIgnoreCase(typeStr) ) {
            itemParameter.type = ItemParameterType.TYPEFACE;
        } else {
            throw new XmlPullParserException( "unrecognized parameter type"+ parser.getPositionDescription());
        }
        while( parser.next() != XmlPullParser.END_TAG ) {
            if( parser.getEventType()!=XmlPullParser.START_TAG )
                continue;
            String name = parser.getName();
            if( name.equalsIgnoreCase("string")) {
                if( itemParameter.strings==null )
                    itemParameter.strings = new HashMap<>();
                String stringName = parser.getAttributeValue(null,"name");
                String stringLang = parser.getAttributeValue(null,"lang");
                String text = parseText(parser);
                if( stringName!=null && stringLang!=null && text!=null ) {
                    Map<String, String> nameMap = itemParameter.strings.get(stringName);
                    if( nameMap==null ) {
                        nameMap = new HashMap<>();
                        itemParameter.strings.put(stringName,nameMap);
                    }
                    nameMap.put(stringLang,text);
                }
            } else if( name.equalsIgnoreCase("option")) {
                if( itemParameter.options==null ) {
                    itemParameter.options = new ArrayList<>();
                }
                itemParameter.options.add(readOption(parser));
            } else if( name.equalsIgnoreCase("icon")) {
                if( itemParameter.iconPath!=null ) {
                    throw new XmlPullParserException( "multiple <icon> tags not allowed"+ parser.getPositionDescription());
                }
                itemParameter.iconPath = parser.getAttributeValue(null,"src");
            } else {
                skip(parser);
            }
        }

        return itemParameter;
    }

    private static ItemParameterDef.Option readOption(XmlPullParser parser ) throws XmlPullParserException, IOException {
        parser.require(XmlPullParser.START_TAG, null, "option");
        OptionImpl option = new OptionImpl();
        option.value = parser.getAttributeValue(null,"value");
        while( parser.next() != XmlPullParser.END_TAG ) {
            if (parser.getEventType() != XmlPullParser.START_TAG)
                continue;
            String name = parser.getName();
            if( name.equalsIgnoreCase("string")) {
                String stringName = parser.getAttributeValue(null,"name");
                String stringLang = parser.getAttributeValue(null,"lang");
                String text = parseText(parser);
                if( stringName!=null && stringLang!=null && text!=null ) {
                    if( option.strings==null )
                        option.strings = new HashMap<>();
                    Map<String, String> nameMap = option.strings.get(stringName);
                    if( nameMap==null ) {
                        nameMap = new HashMap<>();
                        option.strings.put(stringName,nameMap);
                    }
                    nameMap.put(stringLang,text);
                }
            } else if( name.equalsIgnoreCase("icon")) {
                if( option.iconPath!=null ) {
                    throw new XmlPullParserException( "multiple <icon> tags not allowed"+ parser.getPositionDescription());
                }
                option.iconPath = parser.getAttributeValue(null,"src");
            } else {
                skip(parser);
            }
        }
        return option;
    }

    private static ItemParameterDef.Option readUserFieldOption(XmlPullParser parser ) throws XmlPullParserException, IOException {
        parser.require(XmlPullParser.START_TAG, null, "option");
        OptionImpl option = new OptionImpl();
        option.value = parser.getAttributeValue(null,"value");
        option.iconPath = parser.getAttributeValue(null,"icon");
        String label = parser.getAttributeValue(null,"label");
        if( label!=null ) {
            option.strings = new HashMap<>();
            option.strings.put("label",new HashMap<String, String>());
            option.strings.get("label").put("",label);
        }
        while( parser.next() != XmlPullParser.END_TAG ) {
            if (parser.getEventType() != XmlPullParser.START_TAG)
                continue;
            String name = parser.getName();
            if( name.equalsIgnoreCase("fieldlabel")) {
                if( option.strings==null )
                    option.strings = new HashMap<>();
                String stringName = "label";
                String stringLang = parser.getAttributeValue(null,"locale");
                String text = parser.getAttributeValue(null, "value");
                skip(parser);
                if( stringLang!=null && text!=null ) {
                    Map<String, String> nameMap = option.strings.get(stringName);
                    if( nameMap==null ) {
                        nameMap = new HashMap<>();
                        option.strings.put(stringName,nameMap);
                    }
                    nameMap.put(stringLang,text);
                }
            } else {
                skip(parser);
            }
        }
        return option;
    }


    private static void skip(XmlPullParser parser) throws XmlPullParserException, IOException {
        if (parser.getEventType() != XmlPullParser.START_TAG) {
            throw new IllegalStateException();
        }
        int depth = 1;
        while (depth != 0) {
            switch (parser.next()) {
                case XmlPullParser.END_TAG:
                    depth--;
                    break;
                case XmlPullParser.START_TAG:
                    depth++;
                    break;
            }
        }
    }

    private static String parseText(XmlPullParser parser) throws XmlPullParserException, IOException {
        String result = null;
        if (parser.getEventType() != XmlPullParser.START_TAG) {
            throw new IllegalStateException();
        }
        int depth = 1;
        while (depth != 0) {
            switch (parser.next()) {
                case XmlPullParser.TEXT:
                    if( result==null )
                        result = parser.getText();
                    else
                        result += parser.getText();
                    break;
                case XmlPullParser.END_TAG:
                    depth--;
                    break;
                case XmlPullParser.START_TAG:
                    depth++;
                    break;
            }
        }
        return result;
    }


}
