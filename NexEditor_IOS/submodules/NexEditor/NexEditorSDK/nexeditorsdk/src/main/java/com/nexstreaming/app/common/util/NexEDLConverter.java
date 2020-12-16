package com.nexstreaming.app.common.util;

import org.xmlpull.v1.XmlPullParserFactory;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlSerializer;

import java.io.StringReader;
import java.io.StringWriter;

public class NexEDLConverter {
    public static String Transition2Title(String inputXmlString, String newid) {
        try {
            XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
            factory.setNamespaceAware(true);
            XmlPullParser xpp = factory.newPullParser();
            xpp.setInput(new StringReader(inputXmlString));
            XmlSerializer serializer = factory.newSerializer();
            StringWriter writer = new StringWriter();
            serializer.setOutput(writer);

            int eventType = xpp.getEventType();
            boolean bDone = ( eventType == XmlPullParser.END_DOCUMENT ? true:false );
            boolean isVideo2tagFinded = false;
            boolean isVideo2tagProcessing = false;
            String video2id="video2", video1id="video1";

            while (false == bDone) {
                if (eventType == XmlPullParser.START_DOCUMENT) {
                    //serializer.startDocument("UTF-8", true);
                } else if (eventType == XmlPullParser.END_DOCUMENT) {
                    serializer.endDocument();
                    bDone = true;
                } else if (eventType == XmlPullParser.START_TAG) {

                    if (xpp.getName().compareToIgnoreCase("texture") == 0) {
                        for ( int i = 0 ; i < xpp.getAttributeCount() ; i++ ) {
                            if ( xpp.getAttributeName(i).compareToIgnoreCase("video") == 0 ) {
                                if ( xpp.getAttributeValue(i).compareToIgnoreCase("2") == 0 ) {
                                    isVideo2tagProcessing = true;
                                    isVideo2tagFinded = true;
                                    video2id = "@" + xpp.getAttributeValue(null,"id");
                                }
                                else if ( xpp.getAttributeValue(i).compareToIgnoreCase("1") == 0 ) {
                                    video1id = "@" + xpp.getAttributeValue(null,"id");
                                }
                            }
                        }
                    }

                    if (isVideo2tagProcessing == false) {
                        serializer.startTag(xpp.getNamespace(), xpp.getName());

                        for (int i = 0; i < xpp.getAttributeCount(); i++) {
                            boolean wasChanged = false;

                            if ( xpp.getAttributeName(i).compareToIgnoreCase("texture") == 0 ) {
                                if ( xpp.getAttributeValue(i).compareToIgnoreCase(video2id) == 0 ) {
                                    wasChanged = true;
                                    serializer.attribute(xpp.getAttributeNamespace(i), xpp.getAttributeName(i), video1id);
                                }
                            }
                            if ( xpp.getAttributeName(i).compareToIgnoreCase("type") == 0 ) {
                                if ( xpp.getName().compareToIgnoreCase("effect") == 0 ) {
                                    wasChanged = true;
                                    serializer.attribute(xpp.getAttributeNamespace(i), xpp.getAttributeName(i), "title");
                                }
                            }
                            if ( xpp.getAttributeName(i).compareToIgnoreCase("id") == 0 ) {
                                if ( xpp.getName().compareToIgnoreCase("effect") == 0 && newid != null && newid.length() > 0 ) {
                                    wasChanged = true;
                                    serializer.attribute(xpp.getAttributeNamespace(i), xpp.getAttributeName(i), newid);
                                }
                            }
                            if ( wasChanged == false )
                                serializer.attribute(xpp.getAttributeNamespace(i), xpp.getAttributeName(i), xpp.getAttributeValue(i));
                        }
                    }
                } else if (eventType == XmlPullParser.END_TAG) {
                    if ( isVideo2tagProcessing == true && xpp.getName().compareToIgnoreCase("texture") == 0 )
                        isVideo2tagProcessing = false;
                    else
                        serializer.endTag(xpp.getNamespace(), xpp.getName());
                } else if (eventType == XmlPullParser.TEXT) {
                    serializer.text(xpp.getText());
                }
                eventType = xpp.next();
            }
            return writer.toString();

        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
}