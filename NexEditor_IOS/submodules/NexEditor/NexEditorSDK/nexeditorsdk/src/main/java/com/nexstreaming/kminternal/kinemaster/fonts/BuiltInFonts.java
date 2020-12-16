package com.nexstreaming.kminternal.kinemaster.fonts;

import android.content.Context;
import android.graphics.Typeface;
import android.os.Build;

import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

class BuiltInFonts {
    private BuiltInFonts(){} // Prevent instantiation

    private static final char lowerStart   = 97; 
    private static final char lowerEnd     = 122;  

    private static String makeLabel(String baseid){
        StringBuffer sb= new StringBuffer();
        boolean upperCase = true;
        for( char ch : baseid.toCharArray() ){
            if( upperCase ) {
                if(ch>=lowerStart && ch<=lowerEnd)
                    ch = (char) (ch-32);
                sb.append(ch);
                upperCase = false;
                continue;
            }

            if( ch == '_' || ch == '-'){
                sb.append(' ');
                upperCase = true;
                continue;
            }
            sb.append(ch);
        }
        return sb.toString();
    }

    private static String makeId(String baseid){
        StringBuffer sb= new StringBuffer();
        String base = baseid.toLowerCase();
        for( char ch : base.toCharArray() ){
            if( ch == '_' || ch == '-' || ch == ' '){
                continue;
            }
            sb.append(ch);
        }
        return sb.toString();
    }

    static List<Font> makeBuiltInFontList() {

        List<Font> result = new ArrayList<>();
        result.addAll(Arrays.asList(
                new Font("system.robotothin",				"android",	Typeface.create("sans-serif-thin", Typeface.NORMAL),		  "Roboto Thin"),
                new Font("system.robotolight",				"android",	Typeface.create("sans-serif-light", Typeface.NORMAL),		  "Roboto Light"),
                new Font("system.droidsans",				"android",	Typeface.create("sans-serif", Typeface.NORMAL),				  "Roboto Regular"),
                new Font("system.droidsansb",				"android",	Typeface.create("sans-serif", Typeface.BOLD),				  "Roboto Bold"),
                new Font("system.robotonthini",				"android",	Typeface.create("sans-serif-thin", Typeface.ITALIC),		  "Roboto Thin Italic"),
                new Font("system.robotolighti",				"android",	Typeface.create("sans-serif-light", Typeface.ITALIC),		  "Roboto Light Italic"),
                new Font("system.robotoi",					"android",	Typeface.create("sans-serif", Typeface.ITALIC),				  "Roboto Regular Italic"),
                new Font("system.robotobi",					"android",	Typeface.create("sans-serif", Typeface.BOLD|Typeface.ITALIC), "Roboto Bold Italic"),
                new Font("system.robotocond",				"android",	Typeface.create("sans-serif-condensed", Typeface.NORMAL),	  "Roboto Condensed Regular"),
                new Font("system.robotocondi",				"android",	Typeface.create("sans-serif-condensed", Typeface.ITALIC),	  "Roboto Condensed Italic"),
                new Font("system.robotocondb",				"android",	Typeface.create("sans-serif-condensed", Typeface.BOLD),		  "Roboto Condensed Bold"),
                new Font("system.robotocondbi",				"android",	Typeface.create("sans-serif-condensed", Typeface.BOLD|Typeface.ITALIC),				"Roboto Condensed Bold Italic"),
                new Font("system.droidserif",				"android",	Typeface.create(Typeface.SERIF, Typeface.NORMAL),	"Noto Serif Regular"),
                new Font("system.droidserifb",				"android",	Typeface.create(Typeface.SERIF, Typeface.BOLD),		"Noto Serif Bold"),
                new Font("system.droidserifi",				"android",	Typeface.create(Typeface.SERIF, Typeface.ITALIC),	"Noto Serif Italic"),
                new Font("system.droidserifbi",				"android",	Typeface.create(Typeface.SERIF, Typeface.BOLD_ITALIC),	"Noto Serif Bold Italic")
        ));

        Context ctx = KineMasterSingleTon.getApplicationInstance().getApplicationContext();
        try {
            String [] files = ctx.getAssets().list("");
            for( String assetFile : files ){
                if (assetFile.endsWith(".ttf")) {
                    if (assetFile.compareTo("bevan.ttf") == 0)
                        result.add(new Font("builtin.font.bevan", "latin", "bevan.ttf", "Bevan"));
                    else if (assetFile.compareTo("creepster-regular.ttf") == 0)
                        result.add(new Font("builtin.font.creepster", "latin", "creepster-regular.ttf", "Creepster"));
                    else if (assetFile.compareTo("goudy_stm_italic.ttf") == 0)
                        result.add(new Font("builtin.font.sortsmillgoudyital", "latin", "goudy_stm_italic.ttf", "Sorts Mill Goudy Italic"));
                    else if (assetFile.compareTo("greatvibes-regular.ttf") == 0)
                        result.add(new Font("builtin.font.greatvibes", "latin", "greatvibes-regular.ttf", "Great Vibes"));
                    else if (assetFile.compareTo("junction.ttf") == 0)
                        result.add(new Font("builtin.font.junction", "latin", "junction.ttf", "Junction"));
                    else if (assetFile.compareTo("knewave.ttf") == 0)
                        result.add(new Font("builtin.font.knewave", "latin", "knewave.ttf", "Knewave"));
                    else if (assetFile.compareTo("lato-bold.ttf") == 0)
                        result.add(new Font("builtin.font.latobold", "latin", "lato-bold.ttf", "Lato Bold"));
                    else if (assetFile.compareTo("leaguegothic.ttf") == 0)
                        result.add(new Font("builtin.font.leaguegothic", "latin", "leaguegothic.ttf", "League Gothic"));
                    else if (assetFile.compareTo("leaguescript.ttf") == 0)
                        result.add(new Font("builtin.font.leaguescriptthin", "latin", "leaguescript.ttf", "League Script"));
                    else if (assetFile.compareTo("lindenhill.ttf") == 0)
                        result.add(new Font("builtin.font.lindenhillregular", "latin", "lindenhill.ttf", "Linden Hill"));
                    else if (assetFile.compareTo("orbitron-bold.ttf") == 0)
                        result.add(new Font("builtin.font.orbitronbold", "latin", "orbitron-bold.ttf", "Orbitron Bold"));
                    else if (assetFile.compareTo("orbitron-medium.ttf") == 0)
                        result.add(new Font("builtin.font.orbitronmedium", "latin", "orbitron-medium.ttf", "Orbitron Medium"));
                    else if (assetFile.compareTo("raleway_thin.ttf") == 0)
                        result.add(new Font("builtin.font.ralewaythin", "latin", "raleway_thin.ttf", "Raleway Thin"));
                    else if (assetFile.compareTo("redressed.ttf") == 0)
                        result.add(new Font("builtin.font.redressedregular", "latin", "redressed.ttf", "Redressed"));
                    else if (assetFile.compareTo("sniglet.ttf") == 0)
                        result.add(new Font("builtin.font.sniglet", "latin", "sniglet.ttf", "Sniglet"));
                    else {
                        if( assetFile.startsWith("_H_") ){
                            // Hidden
                        }else {
                            String baseid = assetFile.substring(0, assetFile.length() - 4);
                            result.add(new Font("builtin.font." + makeId(baseid), "latin", assetFile, makeLabel(baseid)));
                        }
                    }
                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        }

        if(Build.VERSION.SDK_INT>=21) {
            result.addAll(Arrays.asList(
                    new Font("system.robotomed",				"android",	Typeface.create("sans-serif-medium", Typeface.NORMAL),		  "Roboto Medium"),
                    new Font("system.robotomedi",				"android",	Typeface.create("sans-serif-medium", Typeface.ITALIC),		  "Roboto Medium Italic"),
                    new Font("system.robotoblk",				"android",	Typeface.create("sans-serif-black", Typeface.NORMAL),		  "Roboto Black"),
                    new Font("system.robotoblki",				"android",	Typeface.create("sans-serif-black", Typeface.ITALIC),		  "Roboto Black Italic")
            ));
            result.addAll(Arrays.asList(
                    new Font("system.cursive",					"android",	Typeface.create("cursive", Typeface.NORMAL),		  "Dancing Script Regular"),
                    new Font("system.cursiveb",					"android",	Typeface.create("cursive", Typeface.BOLD),		  "Dancing Script Bold"),
                    new Font("system.mono",						"android",	Typeface.create("monospace", Typeface.NORMAL),		  "Droid Sans Mono")
            ));
        }
        return result;
    }
}
