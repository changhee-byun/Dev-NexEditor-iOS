package com.larvalabs.svgandroid;

import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Picture;
import android.graphics.RadialGradient;
import android.graphics.RectF;
import android.graphics.Shader;
import android.util.Log;

import com.nexstreaming.app.common.util.ColorUtil;

import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

/*

   Licensed to the Apache Software Foundation (ASF) under one or more
   contributor license agreements.  See the NOTICE file distributed with
   this work for additional information regarding copyright ownership.
   The ASF licenses this file to You under the Apache License, Version 2.0
   (the "License"); you may not use this file except in compliance with
   the License.  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 */

/**
 * Entry point for parsing SVG files for Android.
 * Use one of the various static methods for parsing SVGs by resource, asset or input stream.
 * Optionally, a single color can be searched and replaced in the SVG while parsing.
 * You can also parse an svg path directly.
 *
 * @author Larva Labs, LLC
 * @see #getSVGFromResource(Resources, int)
 * @see #getSVGFromAsset(AssetManager, String)
 * @see #getSVGFromString(String)
 * @see #getSVGFromInputStream(InputStream)
 * @see #parsePath(String)
 */
public class SVGParser {

    private static final String TAG = "SVGAndroid";
    private static final boolean LOGGING = true;

    public static class ParseParams {

        private boolean fillOverride = false;
        private int fillOverrideColor = 0;
        private boolean strokeOverride = false;
        private int strokeOverrideColor = 0;
        private float strokeOverrideWidth = 0;
        private int searchColor = 0;
        private int replaceColor = 0;
        private Map<Integer,Integer> colorReplacements;
        private Set<Integer> originalColors = new HashSet<>();
        private int colorReplacementTolerance;

        public ParseParams(){}

        public ParseParams(ParseParams other){
            if( other!=null ) {
                fillOverride = other.fillOverride;
                fillOverrideColor = other.fillOverrideColor;
                strokeOverride = other.strokeOverride;
                strokeOverrideColor = other.strokeOverrideColor;
                strokeOverrideWidth = other.strokeOverrideWidth;
                searchColor = other.searchColor;
                replaceColor = other.replaceColor;
                if( other.colorReplacements!=null ) {
                    colorReplacements = new HashMap<>();
                    colorReplacements.putAll(other.colorReplacements);
                }
                colorReplacementTolerance = other.colorReplacementTolerance;
                originalColors.addAll(other.originalColors);
            }
        }

        public void reset() {
            fillOverride = false;
            fillOverrideColor = 0;
            strokeOverride = false;
            strokeOverrideColor = 0;
            strokeOverrideWidth = 0;
            searchColor = 0;
            replaceColor = 0;
        }

        public void setFill(int fillColor) {
            fillOverride = true;
            fillOverrideColor = fillColor;
        }

        public void setStroke(int strokeColor, float strokeWidth) {
            strokeOverride = true;
            strokeOverrideColor = strokeColor;
            strokeOverrideWidth = strokeWidth;
        }

        public void setReplaceColor(int searchColor, int replaceColor) {
            this.searchColor = searchColor;
            this.replaceColor = replaceColor;
        }

        public void setColorReplacements(Map<Integer, Integer> colorReplacements, int tolerance) {
            this.colorReplacements = colorReplacements;
            this.colorReplacementTolerance = tolerance;
        }

        private int mapColor(int color) {
            if( LOGGING ) Log.d(TAG,"mapColor");
            if( colorReplacements!=null && colorReplacements.containsKey(color) ) {
                if( LOGGING ) Log.d(TAG,"mapColor : EXACT");
                return colorReplacements.get(color);
            } else {
                if( colorReplacements!=null && colorReplacementTolerance>0 ) {
                    for( Map.Entry<Integer, Integer> entry: colorReplacements.entrySet() ) {
                        int keycolor = entry.getKey();
                        int diff = Math.max(Math.max(
                                Math.abs(Color.red(keycolor)-Color.red(color)),
                                Math.abs(Color.green(keycolor)-Color.green(color))),
                                Math.abs(Color.blue(keycolor)-Color.blue(color)));
                        if( diff <= colorReplacementTolerance ) {
                            if( LOGGING ) Log.d(TAG,"mapColor : SIMILAR");
                            int replaceColor = entry.getValue();
                            return Color.rgb(
                                    Math.max(0,Math.min(255,Color.red(color)-Color.red(keycolor)+Color.red(replaceColor))),
                                    Math.max(0,Math.min(255,Color.green(color)-Color.green(keycolor)+Color.green(replaceColor))),
                                    Math.max(0,Math.min(255,Color.blue(color)-Color.blue(keycolor)+Color.blue(replaceColor)))
                            );
                        }
                    }
                }
                if( LOGGING ) Log.d(TAG,"mapColor : NO CHANGE");
                return color;
            }
        }
    }

    /**
     * Parse SVG data from an input stream.
     *
     * @param svgData the input stream, with SVG XML data in UTF-8 character encoding.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     */
    public static SVG getSVGFromInputStream(InputStream svgData) throws SVGParseException {
        return SVGParser.parse(svgData, null);
    }

    /**
     * Parse SVG data from an input stream.
     *
     * @param svgData the input stream, with SVG XML data in UTF-8 character encoding.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     */
    public static SVG getSVGFromInputStream(InputStream svgData, Map<Integer,Integer> colorReplacements, int tolerance) throws SVGParseException {
        ParseParams parseParams = new ParseParams();
        parseParams.setColorReplacements(colorReplacements,tolerance);
        return SVGParser.parse(svgData, parseParams);
    }

    /**
     * Parse SVG data from a string.
     *
     * @param svgData the string containing SVG XML data.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     */
    public static SVG getSVGFromString(String svgData) throws SVGParseException {
        return SVGParser.parse(new ByteArrayInputStream(svgData.getBytes()), null);
    }

    /**
     * Parse SVG data from an Android application resource.
     *
     * @param resources the Android context resources.
     * @param resId     the ID of the raw resource SVG.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     */
    public static SVG getSVGFromResource(Resources resources, int resId) throws SVGParseException {
        return SVGParser.parse(resources.openRawResource(resId), null);
    }

    /**
     * Parse SVG data from an Android application resource.
     *
     * @param resources the Android context resources.
     * @param resId     the ID of the raw resource SVG.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     */
    public static SVG getSVGFromResource(Resources resources, int resId, ParseParams parseParams) throws SVGParseException {
        return SVGParser.parse(resources.openRawResource(resId), parseParams);
    }

    /**
     * Parse SVG data from an Android application asset.
     *
     * @param assetMngr the Android asset manager.
     * @param svgPath   the path to the SVG file in the application's assets.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     * @throws IOException       if there was a problem reading the file.
     */
    public static SVG getSVGFromAsset(AssetManager assetMngr, String svgPath) throws SVGParseException, IOException {
        InputStream inputStream = assetMngr.open(svgPath);
        SVG svg = getSVGFromInputStream(inputStream);
        inputStream.close();
        return svg;
    }

    /**
     * Parse SVG data from an input stream, replacing a single color with another color.
     *
     * @param svgData      the input stream, with SVG XML data in UTF-8 character encoding.
     * @param searchColor  the color in the SVG to replace.
     * @param replaceColor the color with which to replace the search color.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     */
    public static SVG getSVGFromInputStream(InputStream svgData, int searchColor, int replaceColor) throws SVGParseException {
        if( LOGGING ) Log.d(TAG,"getSVGFromInputStream");
        ParseParams parseParams = new ParseParams();
        parseParams.setReplaceColor(searchColor,replaceColor);
        return SVGParser.parse(svgData, parseParams);
    }

    /**
     * Parse SVG data from a string.
     *
     * @param svgData      the string containing SVG XML data.
     * @param searchColor  the color in the SVG to replace.
     * @param replaceColor the color with which to replace the search color.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     */
    public static SVG getSVGFromString(String svgData, int searchColor, int replaceColor) throws SVGParseException {
        ParseParams parseParams = new ParseParams();
        parseParams.setReplaceColor(searchColor,replaceColor);
        return SVGParser.parse(new ByteArrayInputStream(svgData.getBytes()), parseParams);
    }

    /**
     * Parse SVG data from an Android application resource.
     *
     * @param resources    the Android context
     * @param resId        the ID of the raw resource SVG.
     * @param searchColor  the color in the SVG to replace.
     * @param replaceColor the color with which to replace the search color.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     */
    public static SVG getSVGFromResource(Resources resources, int resId, int searchColor, int replaceColor) throws SVGParseException {
        ParseParams parseParams = new ParseParams();
        parseParams.setReplaceColor(searchColor,replaceColor);
        return SVGParser.parse(resources.openRawResource(resId), parseParams);
    }

    /**
     * Parse SVG data from an Android application asset.
     *
     * @param assetMngr    the Android asset manager.
     * @param svgPath      the path to the SVG file in the application's assets.
     * @param searchColor  the color in the SVG to replace.
     * @param replaceColor the color with which to replace the search color.
     * @return the parsed SVG.
     * @throws SVGParseException if there is an error while parsing.
     * @throws IOException       if there was a problem reading the file.
     */
    public static SVG getSVGFromAsset(AssetManager assetMngr, String svgPath, int searchColor, int replaceColor) throws SVGParseException, IOException {
        InputStream inputStream = assetMngr.open(svgPath);
        SVG svg = getSVGFromInputStream(inputStream, searchColor, replaceColor);
        inputStream.close();
        return svg;
    }

    /**
     * Parses a single SVG path and returns it as a <code>android.graphics.Path</code> object.
     * An example path is <code>M250,150L150,350L350,350Z</code>, which draws a triangle.
     *
     * @param pathString the SVG path, see the specification <a href="http://www.w3.org/TR/SVG/paths.html">here</a>.
     */
    public static Path parsePath(String pathString) {
        return doPath(pathString);
    }

    private static SVG parse(InputStream in, ParseParams parseParams) throws SVGParseException {
        if( LOGGING ) Log.d(TAG,"Parse IN");
//        Util.debug("Parsing SVG...");
        try {
            long start = System.currentTimeMillis();
            SAXParserFactory spf = SAXParserFactory.newInstance();
            SAXParser sp = spf.newSAXParser();
            XMLReader xr = sp.getXMLReader();
            final Picture picture = new Picture();
            SVGHandler handler = new SVGHandler(picture,parseParams);
            xr.setContentHandler(handler);
            xr.parse(new InputSource(in));
//        Util.debug("Parsing complete in " + (System.currentTimeMillis() - start) + " millis.");
            SVG result = new SVG(picture, handler.bounds, handler.getParseParams().originalColors);
            // Skip bounds if it was an empty pic
            if (!Float.isInfinite(handler.limits.top)) {
                result.setLimits(handler.limits);
            }
            if( LOGGING ) Log.d(TAG,"Parse OUT");
//            throw new SVGParseException("TEST");
            return result;
        } catch (Exception e) {
            if( LOGGING ) Log.w(TAG,"Parse Error",e);
            throw new SVGParseException(e);
        }
    }

    private static NumberParse parseNumbers(String s) {
        //Util.debug("Parsing numbers from: '" + s + "'");
        int n = s.length();
        int p = 0;
        ArrayList<Float> numbers = new ArrayList<Float>();
        boolean skipChar = false;
        for (int i = 1; i < n; i++) {
            if (skipChar) {
                skipChar = false;
                continue;
            }
            char c = s.charAt(i);
            switch (c) {
                // This ends the parsing, as we are on the next element
                case 'M':
                case 'm':
                case 'Z':
                case 'z':
                case 'L':
                case 'l':
                case 'H':
                case 'h':
                case 'V':
                case 'v':
                case 'C':
                case 'c':
                case 'S':
                case 's':
                case 'Q':
                case 'q':
                case 'T':
                case 't':
                case 'a':
                case 'A':
                case ')': {
                    String str = s.substring(p, i);
                    if (str.trim().length() > 0) {
                        //Util.debug("  Last: " + str);
                        Float f = Float.parseFloat(str);
                        numbers.add(f);
                    }
                    p = i;
                    return new NumberParse(numbers, p);
                }
                case '\n':
                case '\t':
                case ' ':
                case ',':
                /*case '-':*/ {
                    String str = s.substring(p, i);
                    // Just keep moving if multiple whitespace
                    if (str.trim().length() > 0) {
                        //Util.debug("  Next: " + str);
                        Float f = Float.parseFloat(str);
                        numbers.add(f);
                        if (c == '-') {
                            p = i;
                        } else {
                            p = i + 1;
                            skipChar = true;
                        }
                    } else {
                        p++;
                    }
                    break;
                }
            }
        }
        String last = s.substring(p);
        if (last.length() > 0) {
            //Util.debug("  Last: " + last);
            try {
                numbers.add(Float.parseFloat(last));
            } catch (NumberFormatException nfe) {
                // Just white-space, forget it
            }
            p = s.length();
        }
        return new NumberParse(numbers, p);
    }

    private static class TransformParser {

        private final String s;
        private Matrix matrix = null;

        public TransformParser( String s ) {
            this.s = s;
        }

        public Matrix parse() {
            if( matrix==null ) {
                parse_internal();
            }
            return matrix;
        }

        private void parse_internal() {
            matrix = new Matrix();

            int len = s.length();
            for( int i=0; i<len; i++ ) {
                char c = s.charAt(i);
                if( c>='a' && c<='z' ) {
                    for( Prefix prefix: Prefix.values() ) {
                        String name = prefix.name();
                        if (s.startsWith(name, i) && s.charAt(i + name.length()) == '(') {
                            i += name.length();
                            NumberParse np = parseNumbers(s.substring(i+1));
                            i += np.getNextCmd();
                            Matrix partialMatrix = parseTransformElement(prefix,np);
                            if( partialMatrix!=null )
                                matrix.preConcat(partialMatrix);
                        }
                    }
                }
            }
        }

    }

    private enum Prefix {
        matrix,
        translate,
        scale,
        skewX,
        skewY,
        rotate
    };

    private static Matrix parseTransformElement(Prefix prefix, NumberParse np) {
        switch(prefix) {
            case matrix:
                if (np.numbers.size() == 6) {
                    Matrix matrix = new Matrix();
                    matrix.setValues(new float[]{
                            // Row 1
                            np.numbers.get(0),
                            np.numbers.get(2),
                            np.numbers.get(4),
                            // Row 2
                            np.numbers.get(1),
                            np.numbers.get(3),
                            np.numbers.get(5),
                            // Row 3
                            0,
                            0,
                            1,
                    });
                    return matrix;
                }
                break;
            case translate:
                if (np.numbers.size() > 0) {
                    float tx = np.numbers.get(0);
                    float ty = 0;
                    if (np.numbers.size() > 1) {
                        ty = np.numbers.get(1);
                    }
                    Matrix matrix = new Matrix();
                    matrix.postTranslate(tx, ty);
                    return matrix;
                }
                break;
            case scale:
                if (np.numbers.size() > 0) {
                    float sx = np.numbers.get(0);
                    float sy = 0;
                    if (np.numbers.size() > 1) {
                        sy = np.numbers.get(1);
                    }
                    Matrix matrix = new Matrix();
                    matrix.postScale(sx, sy);
                    return matrix;
                }
                break;
            case skewX:
                if (np.numbers.size() > 0) {
                    float angle = np.numbers.get(0);
                    Matrix matrix = new Matrix();
                    matrix.postSkew((float) Math.tan(angle), 0);
                    return matrix;
                }
                break;
            case skewY:
                if (np.numbers.size() > 0) {
                    float angle = np.numbers.get(0);
                    Matrix matrix = new Matrix();
                    matrix.postSkew(0, (float) Math.tan(angle));
                    return matrix;
                }
                break;
            case rotate:
                if (np.numbers.size() > 0) {
                    float angle = np.numbers.get(0);
                    float cx = 0;
                    float cy = 0;
                    if (np.numbers.size() > 2) {
                        cx = np.numbers.get(1);
                        cy = np.numbers.get(2);
                    }
                    Matrix matrix = new Matrix();
                    matrix.postTranslate(cx, cy);
                    matrix.postRotate(angle);
                    matrix.postTranslate(-cx, -cy);
                    return matrix;
                }
                break;
        }
        return null;
    }

    private static Matrix parseTransform(String s) {
        TransformParser transformParser = new TransformParser(s);
        return transformParser.parse();
    }

//    private static Matrix parseTransform(String s) {
//        if (s.startsWith("matrix(")) {
//            NumberParse np = parseNumbers(s.substring("matrix(".length()));
//            if (np.numbers.size() == 6) {
//                Matrix matrix = new Matrix();
//                matrix.setValues(new float[]{
//                        // Row 1
//                        np.numbers.get(0),
//                        np.numbers.get(2),
//                        np.numbers.get(4),
//                        // Row 2
//                        np.numbers.get(1),
//                        np.numbers.get(3),
//                        np.numbers.get(5),
//                        // Row 3
//                        0,
//                        0,
//                        1,
//                });
//                return matrix;
//            }
//        } else if (s.startsWith("translate(")) {
//            NumberParse np = parseNumbers(s.substring("translate(".length()));
//            if (np.numbers.size() > 0) {
//                float tx = np.numbers.get(0);
//                float ty = 0;
//                if (np.numbers.size() > 1) {
//                    ty = np.numbers.get(1);
//                }
//                Matrix matrix = new Matrix();
//                matrix.postTranslate(tx, ty);
//                return matrix;
//            }
//        } else if (s.startsWith("scale(")) {
//            NumberParse np = parseNumbers(s.substring("scale(".length()));
//            if (np.numbers.size() > 0) {
//                float sx = np.numbers.get(0);
//                float sy = 0;
//                if (np.numbers.size() > 1) {
//                    sy = np.numbers.get(1);
//                }
//                Matrix matrix = new Matrix();
//                matrix.postScale(sx, sy);
//                return matrix;
//            }
//        } else if (s.startsWith("skewX(")) {
//            NumberParse np = parseNumbers(s.substring("skewX(".length()));
//            if (np.numbers.size() > 0) {
//                float angle = np.numbers.get(0);
//                Matrix matrix = new Matrix();
//                matrix.postSkew((float) Math.tan(angle), 0);
//                return matrix;
//            }
//        } else if (s.startsWith("skewY(")) {
//            NumberParse np = parseNumbers(s.substring("skewY(".length()));
//            if (np.numbers.size() > 0) {
//                float angle = np.numbers.get(0);
//                Matrix matrix = new Matrix();
//                matrix.postSkew(0, (float) Math.tan(angle));
//                return matrix;
//            }
//        } else if (s.startsWith("rotate(")) {
//            NumberParse np = parseNumbers(s.substring("rotate(".length()));
//            if (np.numbers.size() > 0) {
//                float angle = np.numbers.get(0);
//                float cx = 0;
//                float cy = 0;
//                if (np.numbers.size() > 2) {
//                    cx = np.numbers.get(1);
//                    cy = np.numbers.get(2);
//                }
//                Matrix matrix = new Matrix();
//                matrix.postTranslate(cx, cy);
//                matrix.postRotate(angle);
//                matrix.postTranslate(-cx, -cy);
//                return matrix;
//            }
//        }
//        return null;
//    }

    /**
     * This is where the hard-to-parse paths are handled.
     * Uppercase rules are absolute positions, lowercase are relative.
     * Types of path rules:
     * <p/>
     * <ol>
     * <li>M/m - (x y)+ - Move to (without drawing)
     * <li>Z/z - (no params) - Close path (back to starting point)
     * <li>L/l - (x y)+ - Line to
     * <li>H/h - x+ - Horizontal ine to
     * <li>V/v - y+ - Vertical line to
     * <li>C/c - (x1 y1 x2 y2 x y)+ - Cubic bezier to
     * <li>S/s - (x2 y2 x y)+ - Smooth cubic bezier to (shorthand that assumes the x2, y2 from previous C/S is the x1, y1 of this bezier)
     * <li>Q/q - (x1 y1 x y)+ - Quadratic bezier to
     * <li>T/t - (x y)+ - Smooth quadratic bezier to (assumes previous control point is "reflection" of last one w.r.t. to current point)
     * </ol>
     * <p/>
     * Numbers are separate by whitespace, comma or nothing at all (!) if they are self-delimiting, (ie. begin with a - sign)
     *
     * @param s the path string from the XML
     */
    private static Path doPath(String s) {
        int n = s.length();
        ParserHelper ph = new ParserHelper(s, 0);
        ph.skipWhitespace();
        Path p = new Path();
        float lastX = 0;
        float lastY = 0;
        float lastX1 = 0;
        float lastY1 = 0;
        float subPathStartX = 0;
        float subPathStartY = 0;
        char prevCmd = 0;
        while (ph.pos < n) {
            char cmd = s.charAt(ph.pos);
            switch (cmd) {
                case '-':
                case '+':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (prevCmd == 'm' || prevCmd == 'M') {
                        cmd = (char) (((int) prevCmd) - 1);
                        break;
                    } else if (prevCmd == 'c' || prevCmd == 'C') {
                        cmd = prevCmd;
                        break;
                    } else if (prevCmd == 'l' || prevCmd == 'L') {
                        cmd = prevCmd;
                        break;
                    }
                default: {
                    ph.advance();
                    prevCmd = cmd;
                }
            }

            boolean wasCurve = false;
            switch (cmd) {
                case 'M':
                case 'm': {
                    float x = ph.nextFloat();
                    float y = ph.nextFloat();
                    if (cmd == 'm') {
                        subPathStartX += x;
                        subPathStartY += y;
                        p.rMoveTo(x, y);
                        lastX += x;
                        lastY += y;
                    } else {
                        subPathStartX = x;
                        subPathStartY = y;
                        p.moveTo(x, y);
                        lastX = x;
                        lastY = y;
                    }
                    break;
                }
                case 'Z':
                case 'z': {
                    p.close();
                    p.moveTo(subPathStartX, subPathStartY);
                    lastX = subPathStartX;
                    lastY = subPathStartY;
                    lastX1 = subPathStartX;
                    lastY1 = subPathStartY;
                    wasCurve = true;
                    break;
                }
                case 'L':
                case 'l': {
                    float x = ph.nextFloat();
                    float y = ph.nextFloat();
                    if (cmd == 'l') {
                        p.rLineTo(x, y);
                        lastX += x;
                        lastY += y;
                    } else {
                        p.lineTo(x, y);
                        lastX = x;
                        lastY = y;
                    }
                    break;
                }
                case 'H':
                case 'h': {
                    float x = ph.nextFloat();
                    if (cmd == 'h') {
                        p.rLineTo(x, 0);
                        lastX += x;
                    } else {
                        p.lineTo(x, lastY);
                        lastX = x;
                    }
                    break;
                }
                case 'V':
                case 'v': {
                    float y = ph.nextFloat();
                    if (cmd == 'v') {
                        p.rLineTo(0, y);
                        lastY += y;
                    } else {
                        p.lineTo(lastX, y);
                        lastY = y;
                    }
                    break;
                }
                case 'C':
                case 'c': {
                    wasCurve = true;
                    float x1 = ph.nextFloat();
                    float y1 = ph.nextFloat();
                    float x2 = ph.nextFloat();
                    float y2 = ph.nextFloat();
                    float x = ph.nextFloat();
                    float y = ph.nextFloat();
                    if (cmd == 'c') {
                        x1 += lastX;
                        x2 += lastX;
                        x += lastX;
                        y1 += lastY;
                        y2 += lastY;
                        y += lastY;
                    }
                    p.cubicTo(x1, y1, x2, y2, x, y);
                    lastX1 = x2;
                    lastY1 = y2;
                    lastX = x;
                    lastY = y;
                    break;
                }
                case 'S':
                case 's': {
                    wasCurve = true;
                    float x2 = ph.nextFloat();
                    float y2 = ph.nextFloat();
                    float x = ph.nextFloat();
                    float y = ph.nextFloat();
                    if (cmd == 's') {
                        x2 += lastX;
                        x += lastX;
                        y2 += lastY;
                        y += lastY;
                    }
                    float x1 = 2 * lastX - lastX1;
                    float y1 = 2 * lastY - lastY1;
                    p.cubicTo(x1, y1, x2, y2, x, y);
                    lastX1 = x2;
                    lastY1 = y2;
                    lastX = x;
                    lastY = y;
                    break;
                }
                case 'A':
                case 'a': {
                    float rx = ph.nextFloat();
                    float ry = ph.nextFloat();
                    float theta = ph.nextFloat();
                    int largeArc = (int) ph.nextFloat();
                    int sweepArc = (int) ph.nextFloat();
                    float x = ph.nextFloat();
                    float y = ph.nextFloat();
                    drawArc(p, lastX, lastY, x, y, rx, ry, theta, largeArc, sweepArc);
                    lastX = x;
                    lastY = y;
                    break;
                }
            }
            if (!wasCurve) {
                lastX1 = lastX;
                lastY1 = lastY;
            }
            ph.skipWhitespace();
        }
        return p;
    }

    private static void drawArc(Path p, float lastX, float lastY, float x, float y, float rx, float ry, float theta, int largeArc, int sweepArc) {
        // todo - not implemented yet, may be very hard to do using Android drawing facilities.
    }

    private static NumberParse getNumberParseAttr(String name, Attributes attributes) {
        int n = attributes.getLength();
        for (int i = 0; i < n; i++) {
            if (attributes.getLocalName(i).equals(name)) {
                return parseNumbers(attributes.getValue(i));
            }
        }
        return null;
    }

    private static String getStringAttr(String name, Attributes attributes) {
        int n = attributes.getLength();
        for (int i = 0; i < n; i++) {
            if (attributes.getLocalName(i).equals(name)) {
                return attributes.getValue(i);
            }
        }
        return null;
    }

    private static Float getFloatAttr(String name, Attributes attributes) {
        return getFloatAttr(name, attributes, null);
    }

    private static Float getFloatAttr(String name, Attributes attributes, Float defaultValue) {
        String v = getStringAttr(name, attributes);
        if (v == null) {
            return defaultValue;
        } else {
            if (v.endsWith("%")) {
                v = v.substring(0, v.length() - 1);
                return Float.parseFloat(v)/100f;
            }
            if (v.endsWith("px")) {
                v = v.substring(0, v.length() - 2);
            }
//            Log.d(TAG, "Float parsing '" + name + "=" + v + "'");
            return Float.parseFloat(v);
        }
    }

    private static Integer getHexAttr(String name, Attributes attributes) {
        String v = getStringAttr(name, attributes);
        //Util.debug("Hex parsing '" + name + "=" + v + "'");
        if (v == null) {
            return null;
        } else {
            try {
                return Integer.parseInt(v.substring(1), 16);
            } catch (NumberFormatException nfe) {
                // todo - parse word-based color here
                return null;
            }
        }
    }

    private static class NumberParse {
        private ArrayList<Float> numbers;
        private int nextCmd;

        public NumberParse(ArrayList<Float> numbers, int nextCmd) {
            this.numbers = numbers;
            this.nextCmd = nextCmd;
        }

        public int getNextCmd() {
            return nextCmd;
        }

        public float getNumber(int index) {
            return numbers.get(index);
        }

    }

    private static class Gradient {
        String id;
        String xlink;
        boolean isLinear;
        float x1, y1, x2, y2;
        float x, y, radius;
        ArrayList<Float> positions = new ArrayList<Float>();
        ArrayList<Integer> colors = new ArrayList<Integer>();
        Matrix matrix = null;

        public Gradient createChild(Gradient g) {
            Gradient child = new Gradient();
            child.id = g.id;
            child.xlink = id;
            child.isLinear = g.isLinear;
            child.x1 = g.x1;
            child.x2 = g.x2;
            child.y1 = g.y1;
            child.y2 = g.y2;
            child.x = g.x;
            child.y = g.y;
            child.radius = g.radius;
            child.positions = positions;
            child.colors = colors;
            child.matrix = matrix;
            if (g.matrix != null) {
                if (matrix == null) {
                    child.matrix = g.matrix;
                } else {
                    Matrix m = new Matrix(matrix);
                    m.preConcat(g.matrix);
                    child.matrix = m;
                }
            }
            return child;
        }
    }

    private static class StyleSet {
        HashMap<String, String> styleMap = new HashMap<String, String>();

        private StyleSet(String string) {
            String[] styles = string.split(";");
            for (String s : styles) {
                String[] style = s.split(":");
                if (style.length == 2) {
                    styleMap.put(style[0], style[1]);
                }
            }
        }

        public String getStyle(String name) {
            return styleMap.get(name);
        }
    }

    private static class Properties {
        StyleSet styles = null;
        Attributes atts;
        Set<String> blocked = null;

        private Properties(Attributes atts) {
            this.atts = atts;
            String styleAttr = getStringAttr("style", atts);
            if (styleAttr != null) {
                styles = new StyleSet(styleAttr);
            }
        }

        public void block(String name) {
            if( blocked==null ) blocked = new HashSet<>();
            blocked.add(name);
        }

        public String getAttr(String name) {
            if( blocked!=null && blocked.contains(name) ) return null;
            String v = null;
            if (styles != null) {
                v = styles.getStyle(name);
            }
            if (v == null) {
                v = getStringAttr(name, atts);
            }
            return v;
        }

        public String getString(String name) {
            return getAttr(name);
        }

        public Integer getHex(String name) {
            String v = getAttr(name);
            if (v == null || !v.startsWith("#")) {
                return null;
            } else {
                try {
                    return Integer.parseInt(v.substring(1), 16);
                } catch (NumberFormatException nfe) {
                    // todo - parse word-based color here
                    return null;
                }
            }
        }

        public Float getFloat(String name, float defaultValue) {
            Float v = getFloat(name);
            if (v == null) {
                return defaultValue;
            } else {
                return v;
            }
        }

        public Float getFloat(String name) {
            String v = getAttr(name);
            if (v == null) {
                return null;
            } else {
                try {
                    return Float.parseFloat(v);
                } catch (NumberFormatException nfe) {
                    return null;
                }
            }
        }
    }

    private static class Style {

        private int fillColor = 0xFF000000;
        private int strokeColor = 0xFF000000;
        private Shader fillShader = null;
        private float strokeWidth = 1;
        private Paint.Cap strokeCap = Paint.Cap.BUTT;
        private Paint.Join strokeJoin = Paint.Join.MITER;
        private boolean strokeSet = false;
        private boolean fillSet = false;
        private Path.FillType fillRule;

        public Style() {

        }

        public Style(Style other) {
            fillColor = other.fillColor;
            strokeColor = other.strokeColor;
            fillShader = other.fillShader;
            strokeWidth = other.strokeWidth;
            strokeCap = other.strokeCap;
            strokeJoin = other.strokeJoin;
            strokeSet = other.strokeSet;
            fillSet = other.fillSet;
        }

        public void setFillColor(int fillColor) {
            this.fillColor = fillColor;
            this.fillShader = null;
            fillSet = true;
        }

        public void setStrokeColor(int strokeColor) {
            this.strokeColor = strokeColor;
            strokeSet = true;
        }

        public boolean applyFillStyle(Paint paint) {
            if(!fillSet)
                return false;
            paint.setStyle(Paint.Style.FILL);
            paint.setShader(fillShader);
            paint.setColor(fillColor);
            return true;
        }

        public boolean applyStrokeStyle(Paint paint) {
            if(!strokeSet)
                return false;
            paint.setStyle(Paint.Style.STROKE);
            paint.setShader(null);
            paint.setColor(strokeColor);
            paint.setStrokeWidth(strokeWidth);
            paint.setStrokeCap(strokeCap);
            paint.setStrokeJoin(strokeJoin);
            return true;
        }

        public void setStrokeWidth(float strokeWidth) {
            this.strokeWidth = strokeWidth;
        }

        public void setFillShader(Shader fillShader) {
            this.fillShader = fillShader;
            this.fillColor = 0xFF000000;
            fillSet = true;
        }

        public void setStrokeCap(Paint.Cap strokeCap) {
            this.strokeCap = strokeCap;
        }

        public void setStrokeJoin(Paint.Join strokeJoin) {
            this.strokeJoin = strokeJoin;
        }

        public boolean hasStroke() {
            return strokeSet;
        }
        public boolean hasFill() {
            return fillSet;
        }

        public void setFillRule(Path.FillType fillRule) {
            this.fillRule = fillRule;
        }

        public Path.FillType getFillRule() {
            return fillRule==null? Path.FillType.WINDING:fillRule;
        }
    }


    private static class SVGHandler extends DefaultHandler {

        private static final String LOG_TAG = "SVGHandler";
        Picture picture;
        Canvas canvas;
        Paint paint;
        // Scratch rect (so we aren't constantly making new ones)
        RectF rect = new RectF();
        RectF bounds = null;
        RectF limits = new RectF(Float.POSITIVE_INFINITY, Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY, Float.NEGATIVE_INFINITY);
        Deque<Style> styleStack = new ArrayDeque<>();

        ParseParams parseParams;

        boolean pushed = false;

        HashMap<String, Shader> gradientMap = new HashMap<String, Shader>();
        HashMap<String, Gradient> gradientRefMap = new HashMap<String, Gradient>();
        Gradient gradient = null;

        private SVGHandler(Picture picture, ParseParams parseParams) {
            if( LOGGING ) Log.d(TAG,"SVGHandler Constructed");
            this.picture = picture;
            paint = new Paint();
            paint.setAntiAlias(true);
            Style style = new Style();
            style.fillSet = true;
            styleStack.push(style);
            this.parseParams = new ParseParams(parseParams);
        }

        public ParseParams getParseParams() {
            return parseParams;
        }

        //        public void setColorSwap(Integer searchColor, Integer replaceColor) {
//            this.searchColor = searchColor;
//            this.replaceColor = replaceColor;
//        }
//
//        public void setWhiteMode(boolean whiteMode) {
//            this.whiteMode = whiteMode;
//        }

        @Override
        public void startDocument() throws SAXException {
            // Set up prior to parsing a doc
        }

        @Override
        public void endDocument() throws SAXException {
            // Clean up after parsing a doc
        }

        private boolean doFill(Properties atts, HashMap<String, Shader> gradients, Style style) {
            if( LOGGING ) Log.d(TAG,"doFill : IN");
            if ("none".equals(atts.getString("display"))) {
                return false;
            }
//            if (whiteMode) {
//                style.setFillColor(0xFFFFFFFF);
//                return true;
//            }
            String fillRule = atts.getString("fill-rule");
            if (fillRule != null) {
                if ("nonzero".equals(fillRule)) {
                    style.setFillRule(Path.FillType.WINDING);
                } else if( "evenodd".equals(fillRule)) {
                    style.setFillRule(Path.FillType.EVEN_ODD);
                }
            }
            if( parseParams.fillOverride ) {
                style.setFillColor(parseParams.fillOverrideColor);
                return true;
            }
            String fillString = atts.getString("fill");
            if (fillString != null && fillString.startsWith("url(#")) {
                if( LOGGING ) Log.d(TAG,"doFill :   gradient>>" + fillString);
                // It's a gradient fill, look it up in our map
                String id = fillString.substring("url(#".length(), fillString.length() - 1);
                Shader shader = gradients.get(id);
                if (shader != null) {
                    //Util.debug("Found shader!");
                    style.setFillShader(shader);
                    return true;
                } else {
                    //Util.debug("Didn't find shader!");
                    return false;
                }
            } else if (fillString!=null && fillString.equals("none")) {
                if( LOGGING ) Log.d(TAG,"doFill :   none");
                style.fillSet = false;
            } else {
                Integer color = atts.getHex("fill");
                if (color != null) {
                    if( LOGGING ) Log.d(TAG,"doFill :   c=" + ColorUtil.colorString(color));
                    style.setFillColor(getColor(atts, color, true));
                    return true;
                } else if (atts.getString("fill") == null && atts.getString("stroke") == null) {
                    if( LOGGING ) Log.d(TAG,"doFill :   no fill & no stroke");
                    return true;
                }
            }
            if( LOGGING ) Log.d(TAG,"doFill :   no fill");
            return false;
        }

        private boolean doStroke(Properties atts, Style style) {
            if( LOGGING ) Log.d(TAG,"doStroke : IN");
//            if (whiteMode) {
//                // Never stroke in white mode
//                return false;
//            }
            if ("none".equals(atts.getString("display"))) {
                return false;
            }
            if( parseParams.strokeOverride ) {
                style.setStrokeColor(parseParams.strokeOverrideColor);
                style.setStrokeWidth(parseParams.strokeOverrideWidth);
                return true;
            }
            String strokeStrong = atts.getString("stroke");
            if (strokeStrong!=null && strokeStrong.equals("none")) {
                style.strokeSet = false;
                return true;
            }
            Integer color = atts.getHex("stroke");
            if (color != null) {
                style.setStrokeColor(getColor(atts, color, false));
                // Check for other stroke attributes
                Float width = atts.getFloat("stroke-width");
                // Set defaults

                if (width != null) {
                    style.setStrokeWidth(width);
                }
                String linecap = atts.getString("stroke-linecap");
                if ("round".equals(linecap)) {
                    style.setStrokeCap(Paint.Cap.ROUND);
                } else if ("square".equals(linecap)) {
                    style.setStrokeCap(Paint.Cap.SQUARE);
                } else if ("butt".equals(linecap)) {
                    style.setStrokeCap(Paint.Cap.BUTT);
                }
                String linejoin = atts.getString("stroke-linejoin");
                if ("miter".equals(linejoin)) {
                    style.setStrokeJoin(Paint.Join.MITER);
                } else if ("round".equals(linejoin)) {
                    style.setStrokeJoin(Paint.Join.ROUND);
                } else if ("bevel".equals(linejoin)) {
                    style.setStrokeJoin(Paint.Join.BEVEL);
                }
                return true;
            }
            return false;
        }

        private Gradient doGradient(boolean isLinear, Attributes atts) {
            if( LOGGING ) Log.d(TAG,"doGradient : IN");
            Gradient gradient = new Gradient();
            gradient.id = getStringAttr("id", atts);
            gradient.isLinear = isLinear;
            if (isLinear) {
                gradient.x1 = getFloatAttr("x1", atts, 0f);
                gradient.x2 = getFloatAttr("x2", atts, 0f);
                gradient.y1 = getFloatAttr("y1", atts, 0f);
                gradient.y2 = getFloatAttr("y2", atts, 0f);
            } else {
                gradient.x = getFloatAttr("cx", atts, 0f);
                gradient.y = getFloatAttr("cy", atts, 0f);
                gradient.radius = getFloatAttr("r", atts, 0f);
            }
            String transform = getStringAttr("gradientTransform", atts);
            if (transform != null) {
                gradient.matrix = parseTransform(transform);
            }
            String xlink = getStringAttr("href", atts);
            if (xlink != null) {
                if (xlink.startsWith("#")) {
                    xlink = xlink.substring(1);
                }
                gradient.xlink = xlink;
            }
            return gradient;
        }

        private int getColor(Properties atts, Integer color, boolean fillMode) {
            if( LOGGING ) Log.d(TAG,"getColor : fillMode=" + fillMode + " color=" + ColorUtil.colorString(color));
            if(fillMode&&parseParams.fillOverride) {
                if( LOGGING ) Log.d(TAG,"getColor : FILL OVERRIDE");
                return parseParams.fillOverrideColor;
            }
            int c = (0xFFFFFF & color) | 0xFF000000;
            if(parseParams.originalColors!=null)
                parseParams.originalColors.add(c);
            if (parseParams.searchColor == c) {
                if( LOGGING ) Log.d(TAG,"getColor : REPLACE COLOR");
                c = parseParams.replaceColor;
            }
            if( parseParams.colorReplacements!=null) {
                if( LOGGING ) Log.d(TAG,"getColor : MAP COLOR");
                c = parseParams.mapColor(c);
            }
            Float opacity = atts.getFloat("opacity");
            if (opacity == null) {
                opacity = atts.getFloat(fillMode ? "fill-opacity" : "stroke-opacity");
            }
            if (opacity != null) {
                c = (0xFFFFFF&c) | (0xFF000000&(((int) (255 * opacity))<<24));
            }
            return c;
        }

        private boolean hidden = false;
        private int hiddenLevel = 0;
        private boolean boundsMode = false;

        private void doLimits(float x, float y) {
            if (x < limits.left) {
                limits.left = x;
            }
            if (x > limits.right) {
                limits.right = x;
            }
            if (y < limits.top) {
                limits.top = y;
            }
            if (y > limits.bottom) {
                limits.bottom = y;
            }
        }

        private void doLimits(float x, float y, float width, float height) {
            doLimits(x, y);
            doLimits(x + width, y + height);
        }

        private void doLimits(Path path) {
            path.computeBounds(rect, false);
            doLimits(rect.left, rect.top);
            doLimits(rect.right, rect.bottom);
        }

        private void pushTransform(Attributes atts) {
            final String transform = getStringAttr("transform", atts);
            pushed = transform != null;
            if (pushed) {
                final Matrix matrix = parseTransform(transform);
                canvas.save();
                canvas.concat(matrix);
            }
        }

        private void popTransform() {
            if (pushed) {
                canvas.restore();
            }
        }

        @Override
        public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
            if( LOGGING ) Log.d(TAG,"startElement : " + localName + " atts=" + atts.toString());
            // Reset paint opacity
//            paint.setAlpha(255);
            // Ignore everything but rectangles in bounds mode
            if (boundsMode) {
                if (localName.equals("rect")) {
                    Float x = getFloatAttr("x", atts);
                    if (x == null) {
                        x = 0f;
                    }
                    Float y = getFloatAttr("y", atts);
                    if (y == null) {
                        y = 0f;
                    }
                    Float width = getFloatAttr("width", atts);
                    Float height = getFloatAttr("height", atts);
                    bounds = new RectF(x, y, x + width, y + width);
                }
                return;
            }
            if (localName.equals("svg")) {
                Float widthAttr = getFloatAttr("width", atts);
                Float heightAttr = getFloatAttr("height", atts);
                float dx=0, dy=0;
                int width, height;
                if( widthAttr==null || heightAttr==null ) {
                    String viewBox = getStringAttr("viewBox",atts);
                    if( viewBox==null ) {
                        width=height=100;
                        Log.e(LOG_TAG,"Missing SVG bounds! Defaulting to 100x100");
                    } else {
                        NumberParse nums = parseNumbers(viewBox);
                        if( nums.numbers.size() < 4 ) {
                            width=height=100;
                            Log.e(LOG_TAG,"Malformed SVG bounds! Defaulting to 100x100");
                        } else {
                            dx = nums.numbers.get(0);
                            dy = nums.numbers.get(1);
                            width = (int) Math.ceil(nums.numbers.get(2));
                            height = (int) Math.ceil(nums.numbers.get(3));
                            if( LOGGING ) Log.d(TAG,"Used viewBox");
                        }
                    }
                } else {
                    width = (int) Math.ceil(widthAttr);
                    height = (int) Math.ceil(heightAttr);
                    if( LOGGING ) Log.d(TAG,"Used width, height");
                }
                if( LOGGING ) Log.d(TAG,"BEGIN REC: dx,dy=" + dx + "," + dy + "  w,h=" + width + "," + height);
                canvas = picture.beginRecording(width, height);
                canvas.translate(dx,dy);
            } else if (localName.equals("defs")) {
                // Ignore
            } else if (localName.equals("linearGradient")) {
                gradient = doGradient(true, atts);
            } else if (localName.equals("radialGradient")) {
                gradient = doGradient(false, atts);
            } else if (localName.equals("stop")) {
                if (gradient != null) {
                    float offset = getFloatAttr("offset", atts);
                    String styles = getStringAttr("style", atts);
//                    StyleSet styleSet = new StyleSet(styles);
                    Properties properties = new Properties(atts);
                    String colorStyle = properties.getAttr("stop-color");
                    int color = Color.BLACK;
                    if (colorStyle != null) {
                        if (colorStyle.startsWith("#")) {
                            color = Integer.parseInt(colorStyle.substring(1), 16);
                        } else {
                            color = Integer.parseInt(colorStyle, 16);
                        }
                    }
                    String opacityStyle = properties.getAttr("stop-opacity");
                    if (opacityStyle != null) {
                        float alpha = Float.parseFloat(opacityStyle);
                        int alphaInt = Math.round(255 * alpha);
                        if( alphaInt > 255 ) alphaInt = 255;
                        color |= (alphaInt << 24);
                    } else {
                        color |= 0xFF000000;
                    }
                    gradient.positions.add(offset);
                    if(parseParams.originalColors!=null)
                        parseParams.originalColors.add(color);
                    color = parseParams.mapColor(color);
                    gradient.colors.add(color);
                }
            } else if (localName.equals("g")) {
                // Check to see if this is the "bounds" layer
                if ("bounds".equalsIgnoreCase(getStringAttr("id", atts))) {
                    boundsMode = true;
                }
                if (hidden) {
                    hiddenLevel++;
                    //Util.debug("Hidden up: " + hiddenLevel);
                }
                Style style = new Style(styleStack.peek());
                Properties props = new Properties(atts);
                Float floatOpacity = props.getFloat("opacity");
                int opacity;
                if( floatOpacity==null ) {
                    opacity = 255;
                } else {
                    opacity = Math.max(0, Math.min(255, (int) (floatOpacity * 255)));

                }
                if( opacity<255 && opacity>0 ) {
                    canvas.saveLayerAlpha(null,opacity,Canvas.ALL_SAVE_FLAG);
                    props.block("opacity");
                } else {
                    canvas.save();
                }
                doFill(props, gradientMap, style);
                doStroke(props, style);
                styleStack.push(style);
                // Go in to hidden mode if display is "none"
                if (opacity<=0 || "none".equals(getStringAttr("display", atts))) {
                    if (!hidden) {
                        hidden = true;
                        hiddenLevel = 1;
                        //Util.debug("Hidden up: " + hiddenLevel);
                    }
                }
            } else if (!hidden && localName.equals("rect")) {
                Float x = getFloatAttr("x", atts);
                if (x == null) {
                    x = 0f;
                }
                Float y = getFloatAttr("y", atts);
                if (y == null) {
                    y = 0f;
                }
                Float rx = getFloatAttr("rx", atts);
                Float ry = getFloatAttr("ry", atts);
                if( rx==null && ry==null ) {
                    rx=0f;
                    ry=0f;
                } else if( rx==null ) {
                    rx = ry;
                } else if( ry==null ) {
                    ry = rx;
                }
                Float width = getFloatAttr("width", atts);
                Float height = getFloatAttr("height", atts);
                pushTransform(atts);
                Properties props = new Properties(atts);
                Style style = new Style(styleStack.peek());
                doFill(props,gradientMap,style);
                doStroke(props,style);
                if (style.applyFillStyle(paint)) {
                    doLimits(x, y, width, height);
                    if( rx>0 && ry>0 ) {
                        canvas.drawRoundRect(new RectF(x, y, x + width, y + height), rx, ry, paint);
                    } else {
                        canvas.drawRect(x, y, x + width, y + height, paint);
                    }
                }
                if (style.applyStrokeStyle(paint)) {
                    if( rx>0 && ry>0 ) {
                        canvas.drawRoundRect(new RectF(x, y, x + width, y + height), rx, ry, paint);
                    } else {
                        canvas.drawRect(x, y, x + width, y + height, paint);
                    }
                }
                popTransform();
            } else if (!hidden && localName.equals("line")) {
                Float x1 = getFloatAttr("x1", atts);
                Float x2 = getFloatAttr("x2", atts);
                Float y1 = getFloatAttr("y1", atts);
                Float y2 = getFloatAttr("y2", atts);
                Properties props = new Properties(atts);
                Style style = new Style(styleStack.peek());
                doStroke(props,style);
                if (style.applyStrokeStyle(paint)) {
                    pushTransform(atts);
                    doLimits(x1, y1);
                    doLimits(x2, y2);
                    canvas.drawLine(x1, y1, x2, y2, paint);
                    popTransform();
                }
            } else if (!hidden && localName.equals("circle")) {
                Float centerX = getFloatAttr("cx", atts);
                Float centerY = getFloatAttr("cy", atts);
                Float radius = getFloatAttr("r", atts);
                if (centerX != null && centerY != null && radius != null) {
                    pushTransform(atts);
                    Properties props = new Properties(atts);
                    Style style = new Style(styleStack.peek());
                    doFill(props,gradientMap,style);
                    doStroke(props, style);
                    if (style.applyFillStyle(paint)) {
                        doLimits(centerX - radius, centerY - radius);
                        doLimits(centerX + radius, centerY + radius);
                        canvas.drawCircle(centerX, centerY, radius, paint);
                    }
                    if (style.applyStrokeStyle(paint)) {
                        canvas.drawCircle(centerX, centerY, radius, paint);
                    }
                    popTransform();
                }
            } else if (!hidden && localName.equals("ellipse")) {
                Float centerX = getFloatAttr("cx", atts);
                Float centerY = getFloatAttr("cy", atts);
                Float radiusX = getFloatAttr("rx", atts);
                Float radiusY = getFloatAttr("ry", atts);
                if (centerX != null && centerY != null && radiusX != null && radiusY != null) {
                    pushTransform(atts);
                    Properties props = new Properties(atts);
                    Style style = new Style(styleStack.peek());
                    doFill(props,gradientMap,style);
                    doStroke(props, style);
                    rect.set(centerX - radiusX, centerY - radiusY, centerX + radiusX, centerY + radiusY);
                    if (style.applyFillStyle(paint)) {
                        doLimits(centerX - radiusX, centerY - radiusY);
                        doLimits(centerX + radiusX, centerY + radiusY);
                        canvas.drawOval(rect, paint);
                    }
                    if (style.applyStrokeStyle(paint)) {
                        canvas.drawOval(rect, paint);
                    }
                    popTransform();
                }
            } else if (!hidden && (localName.equals("polygon") || localName.equals("polyline"))) {
                NumberParse numbers = getNumberParseAttr("points", atts);
                if (numbers != null) {
                    Path p = new Path();
                    ArrayList<Float> points = numbers.numbers;
                    if (points.size() > 1) {
                        pushTransform(atts);
                        Properties props = new Properties(atts);
                        Style style = new Style(styleStack.peek());
                        doFill(props,gradientMap,style);
                        doStroke(props, style);
                        p.moveTo(points.get(0), points.get(1));
                        for (int i = 2; i < points.size(); i += 2) {
                            float x = points.get(i);
                            float y = points.get(i + 1);
                            p.lineTo(x, y);
                        }
                        // Don't close a polyline
                        if (localName.equals("polygon")) {
                            p.close();
                        }
                        if (style.applyFillStyle(paint)) {
                            p.setFillType(style.getFillRule());
                            doLimits(p);
                            canvas.drawPath(p, paint);
                        }
                        if (style.applyStrokeStyle(paint)) {
                            canvas.drawPath(p, paint);
                        }
                        popTransform();
                    }
                }
            } else if (!hidden && localName.equals("path")) {
                Path p = doPath(getStringAttr("d", atts));
                pushTransform(atts);
                Properties props = new Properties(atts);
                Style style = new Style(styleStack.peek());
                doFill(props,gradientMap,style);
                doStroke(props,style);
                if (style.applyFillStyle(paint)) {
                    p.setFillType(style.getFillRule());
                    doLimits(p);
                    canvas.drawPath(p, paint);
                }
                if (style.applyStrokeStyle(paint)) {
                    canvas.drawPath(p, paint);
                }
                popTransform();
            } else if (!hidden) {
                Log.w(TAG, "UNRECOGNIZED SVG COMMAND: " + localName);
            }
        }

        @Override
        public void characters(char ch[], int start, int length) {
            // no-op
        }

        @Override
        public void endElement(String namespaceURI, String localName, String qName)
                throws SAXException {
            if (localName.equals("svg")) {
                picture.endRecording();
            } else if (localName.equals("linearGradient")) {
                if (gradient.id != null) {
                    if (gradient.xlink != null) {
                        Gradient parent = gradientRefMap.get(gradient.xlink);
                        if (parent != null) {
                            gradient = parent.createChild(gradient);
                        }
                    }
                    int[] colors = new int[gradient.colors.size()];
                    for (int i = 0; i < colors.length; i++) {
                        colors[i] = gradient.colors.get(i);
                    }
                    float[] positions = new float[gradient.positions.size()];
                    for (int i = 0; i < positions.length; i++) {
                        positions[i] = gradient.positions.get(i);
                    }
                    if (colors.length == 0) {
                        Log.d("BAD", "BAD");
                    }
                    LinearGradient g = new LinearGradient(gradient.x1, gradient.y1, gradient.x2, gradient.y2, colors, positions, Shader.TileMode.CLAMP);
                    if (gradient.matrix != null) {
                        g.setLocalMatrix(gradient.matrix);
                    }
                    gradientMap.put(gradient.id, g);
                    gradientRefMap.put(gradient.id, gradient);
                }
            } else if (localName.equals("radialGradient")) {
                if (gradient.id != null) {
                    if (gradient.xlink != null) {
                        Gradient parent = gradientRefMap.get(gradient.xlink);
                        if (parent != null) {
                            gradient = parent.createChild(gradient);
                        }
                    }
                    int[] colors = new int[gradient.colors.size()];
                    for (int i = 0; i < colors.length; i++) {
                        colors[i] = gradient.colors.get(i);
                    }
                    float[] positions = new float[gradient.positions.size()];
                    for (int i = 0; i < positions.length; i++) {
                        positions[i] = gradient.positions.get(i);
                    }
                    if (gradient.xlink != null) {
                        Gradient parent = gradientRefMap.get(gradient.xlink);
                        if (parent != null) {
                            gradient = parent.createChild(gradient);
                        }
                    }
                    RadialGradient g = new RadialGradient(gradient.x, gradient.y, gradient.radius, colors, positions, Shader.TileMode.CLAMP);
                    if (gradient.matrix != null) {
                        g.setLocalMatrix(gradient.matrix);
                    }
                    gradientMap.put(gradient.id, g);
                    gradientRefMap.put(gradient.id, gradient);
                }
            } else if (localName.equals("g")) {
                styleStack.pop();
                if (boundsMode) {
                    boundsMode = false;
                }
                canvas.restore();
                // Break out of hidden mode
                if (hidden) {
                    hiddenLevel--;
                    //Util.debug("Hidden down: " + hiddenLevel);
                    if (hiddenLevel == 0) {
                        hidden = false;
                    }
                }
                // Clear gradient map
//                gradientMap.clear(); // [Matthew] This seems suspicious & doesn't match the spec, so I removed it
            }
        }
    }
}
