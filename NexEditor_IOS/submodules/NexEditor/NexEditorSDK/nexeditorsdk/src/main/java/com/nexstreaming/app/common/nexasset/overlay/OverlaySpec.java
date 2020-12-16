package com.nexstreaming.app.common.nexasset.overlay;

import android.util.Xml;

import com.nexstreaming.app.common.util.CloseUtil;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

public class OverlaySpec {

    public List<Layer> layers;
    public int width;
    public int height;
    public int fps;
    public int minFps;
    public int duration;

    public enum AnimDirection {
        NORMAL, REVERSE, ALTERNATE, ALTERNATE_REVERSE
    }

    public static class Layer {
        public List<Frame> frames;
        public int iterationCount;      // # of iterations, or -1 for infinite
        public int duration;
        public AnimDirection direction;
    }

    public static class Frame {
        public String src;
        public boolean blank;
        public int hold;
    }

    public static OverlaySpec fromInputStream(InputStream in) throws XmlPullParserException, IOException {
        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false);
            parser.setInput(in, null);
            parser.nextTag();
            return parseSpec(parser);
        } finally {
            CloseUtil.closeSilently(in);
        }
    }

    private static OverlaySpec parseSpec(XmlPullParser parser) throws XmlPullParserException, IOException {
        parser.require(XmlPullParser.START_TAG, null, "overlay");
        OverlaySpec result = new OverlaySpec();
        result.width = parseInt(parser.getAttributeValue(null,"width"),-1);
        result.height = parseInt(parser.getAttributeValue(null,"height"),-1);
        result.fps = parseInt(parser.getAttributeValue(null,"fps"),15);
        result.minFps = parseInt(parser.getAttributeValue(null,"min-fps"),result.fps);
        result.duration = parseInt(parser.getAttributeValue(null,"duration"),-1);
        result.layers = new ArrayList<>();

        while( parser.next() != XmlPullParser.END_TAG ) {
            if( parser.getEventType()!=XmlPullParser.START_TAG )
                continue;
            String name = parser.getName();
            if( name.equalsIgnoreCase("layer")) {
                Layer layer = parseLayer(parser);
                if( layer!=null )
                    result.layers.add(layer);
            } else {
                skip(parser);
            }
        }

        return result;

    }

    private static Layer parseLayer(XmlPullParser parser) throws XmlPullParserException, IOException {
        parser.require(XmlPullParser.START_TAG, null, "layer");
        Layer result = new Layer();
        result.iterationCount = parseInt(parser.getAttributeValue(null,"iteration-count"),-1);
        String direction = parser.getAttributeValue(null,"direction");
        if( direction==null ) {
        } else if( "normal".equalsIgnoreCase(direction) ) {
            result.direction = AnimDirection.NORMAL;
        } else if( "reverse".equalsIgnoreCase(direction) ) {
            result.direction = AnimDirection.REVERSE;
        } else if( "alternate".equalsIgnoreCase(direction) ) {
            result.direction = AnimDirection.ALTERNATE;
        } else if( "alternate-reverse".equalsIgnoreCase(direction) ) {
            result.direction = AnimDirection.ALTERNATE_REVERSE;
        } else {
            throw new XmlPullParserException("Unrecognized direction: " + direction,parser,null);
        }
        result.frames = new ArrayList<>();

        while( parser.next() != XmlPullParser.END_TAG ) {
            if( parser.getEventType()!=XmlPullParser.START_TAG )
                continue;
            String name = parser.getName();
            if( name.equalsIgnoreCase("frame")) {
                Frame frame = parseFrame(parser);
                if( frame!=null ) {
                    result.frames.add(frame);
                    result.duration += frame.hold;
                }
            } else {
                skip(parser);
            }
        }

        return result;

    }

    private static Frame parseFrame(XmlPullParser parser) throws XmlPullParserException, IOException {
        parser.require(XmlPullParser.START_TAG, null, "frame");
        Frame result = new Frame();
        result.src = parser.getAttributeValue(null,"src");
        result.blank = parseBoolean(parser.getAttributeValue(null,"blank"),false);
        result.hold = Math.max(1,parseInt(parser.getAttributeValue(null,"hold"),1));

        skip(parser);
        return result;

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

    private static int parseInt( String value, int defaultValue ) {
        if( value==null )
            return defaultValue;
        try {
            return Integer.parseInt(value);
        } catch( NumberFormatException e) {
            return defaultValue;
        }
    }

    private static boolean parseBoolean( String value, boolean defaultValue ) {
        if( value==null )
            return defaultValue;
        if( value.equalsIgnoreCase("true") )
            return true;
        if( value.equalsIgnoreCase("false") )
            return false;
        return defaultValue;
    }

}
