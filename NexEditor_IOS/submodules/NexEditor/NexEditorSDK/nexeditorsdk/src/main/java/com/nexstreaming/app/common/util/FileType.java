package com.nexstreaming.app.common.util;

import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public enum FileType {

    JPEG( FileCategory.Image, new String[]{"jpeg","jpg"}, new int[]{0xFF, 0xD8, 0xFF}),
    PNG(  FileCategory.Image, new String[]{"png"}, new int[]{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}),
    SVG(  FileCategory.Image, new String[]{"svg"}, new int[]{0x3C, 0x73, 0x76, 0x67},  new int[]{0x3C, 0x53, 0x56, 0x47}),
    WEBP(  FileCategory.Image, new String[]{"webp"}, new int[]{'R', 'I', 'F', 'F', MATCH.ANY, MATCH.ANY, MATCH.ANY, MATCH.ANY, 'W', 'E', 'B', 'P'}),
    GIF(  FileCategory.Image, new String[]{"gif"}, new int[]{0x47, 0x49, 0x46, 0x38, 0x37, 0x61},  new int[]{0x47, 0x49, 0x46, 0x38, 0x39, 0x61}),
    M4A(  FileCategory.Audio, new String[]{"m4a"},
            new int[]{ 0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70, 0x4D, 0x34, 0x41, 0x20 },   // '....ftypM4A '
            new int[]{ MATCH.ANY, MATCH.ANY, MATCH.ANY, MATCH.ANY, 0x66, 0x74, 0x79, 0x70, 0x4D, 0x34, 0x41, 0x20 }   // 'ftypM4A '
    ),
    M4V(  FileCategory.Video, new String[]{"m4v"},
            new int[]{ 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x6D, 0x70, 0x34, 0x32 },  // '....ftypmp42'
            new int[]{ MATCH.ANY, MATCH.ANY, MATCH.ANY, MATCH.ANY, 0x66, 0x74, 0x79, 0x70, 0x6D, 0x70, 0x34, 0x32 }   // 'ftypmp42'
    ),
    MP4(  FileCategory.Video, new String[]{"mp4"},
            new int[]{ 0x00, 0x00, 0x00, 0x14, 0x66, 0x74, 0x79, 0x70, 0x69, 0x73, 0x6F, 0x6D },  // '....ftypisom'
            new int[]{ 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x69, 0x73, 0x6F, 0x6D },  // '....ftypisom'
            new int[]{ 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x33, 0x67, 0x70, 0x35 },  // '....ftyp3gp5'
            new int[]{ 0x00, 0x00, 0x00, 0x1C, 0x66, 0x74, 0x79, 0x70, 0x4D, 0x53, 0x4E, 0x56, 0x01, 0x29, 0x00, 0x46, 0x4D, 0x53, 0x4E, 0x56, 0x6D, 0x70, 0x34, 0x32}, // ' 	....ftypMSNV.).FMSNVmp42'
            new int[]{ MATCH.ANY, MATCH.ANY, MATCH.ANY, MATCH.ANY, 0x66, 0x74, 0x79, 0x70, 0x33, 0x67, 0x70, 0x35 },    // 'ftyp3gp5'
            new int[]{ MATCH.ANY, MATCH.ANY, MATCH.ANY, MATCH.ANY, 0x66, 0x74, 0x79, 0x70, 0x4D, 0x53, 0x4E, 0x56 },    // 'ftypMSNV'
            new int[]{ MATCH.ANY, MATCH.ANY, MATCH.ANY, MATCH.ANY, 0x66, 0x74, 0x79, 0x70, 0x69, 0x73, 0x6F, 0x6D },    // 'ftypisom'
            new int[]{ 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x6D, 0x70, 0x34, 0x31 }  // '....ftypmp41'
    ),
    F_3GP(  FileCategory.VideoOrAudio, new String[]{"3gp","3gpp","3g2"},
            new int[]{ 0x00, 0x00, 0x00, MATCH.ANY, 0x66, 0x74, 0x79, 0x70, 0x33, 0x67, 0x70 },
            new int[]{ 0x00, 0x00, 0x00, MATCH.ANY, 0x66, 0x74, 0x79, 0x70, 0x33, 0x67, 0x32 }
    ),
    K3G(  FileCategory.Video, new String[]{"k3g"}),
    ACC(  FileCategory.Video, new String[]{"acc"}),
    AVI(  FileCategory.Video, new String[]{"avi"}, new int[]{0x52, 0x49, 0x46, 0x46, MATCH.ANY, MATCH.ANY, MATCH.ANY, MATCH.ANY, 0x41, 0x56, 0x49, 0x20, 0x4C, 0x49, 0x53, 0x54}),
    MOV(  FileCategory.Video, new String[]{"mov"},
            new int[]{ 0x00, 0x00, 0x00, 0x14, 0x66, 0x74, 0x79, 0x70, 0x71, 0x74, 0x20, 0x20 },
            new int[]{ 0x66, 0x74, 0x79, 0x70, 0x71, 0x74, 0x20, 0x20 },
            new int[]{ MATCH.ANY, MATCH.ANY, MATCH.ANY, MATCH.ANY, 0x6D, 0x6F, 0x6F, 0x76 }
    ),
    WMV(  FileCategory.Video, new String[]{"wmv"}, new int[]{ 0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}),
    MP3(  FileCategory.Audio, new String[]{"mp3"}, new int[]{ 0x49, 0x44, 0x33 }, new int[]{0xff, 0xfb}),
    AAC(  FileCategory.Audio, new String[]{"aac"}, new int[]{ 0xff, 0xf1 }, new int[]{0xff, 0xf9}),
    BMP(  FileCategory.Image, new String[]{"bmp"}, new int[]{0x42, 0x4D}),
    TTF(  FileCategory.Font, new String[]{"ttf","otf"}),
    WBMP(  FileCategory.Image, new String[]{"wbmp"},false),
    ;

    private static final String LOG_TAG = "FileType";
    private final Imp imp;
    private final FileCategory category;
    private final String[] extensions;
    private final boolean extensionOnly;
    private final boolean isSupportedFormat;
    private static final Map<String,FileType> extensionMap = new HashMap<>();

    private static final int CHECK_SIZE = 32;
    private static final byte WEBP_HEADER[] = {'R', 'I', 'F', 'F', 'W', 'E', 'B', 'P'};

    private FileType(FileCategory category, String[] extensions, Imp imp) {
        this.imp = imp;
        this.category = category;
        this.extensions = extensions;
        this.extensionOnly = false;
        this.isSupportedFormat = true;
    }

    private FileType(FileCategory category, String[] extensions, boolean isSupportedFormat) {
        this.imp = new Imp() {
            @Override
            boolean matches(byte[] header) {
                return false;
            }
        };
        this.category = category;
        this.extensions = extensions;
        this.extensionOnly = false;
        this.isSupportedFormat = isSupportedFormat;
    }


    private FileType(FileCategory category, String[] extensions) {
        this.imp = new Imp() {
            @Override
            boolean matches(byte[] header) {
                return false;
            }
        };
        this.category = category;
        this.extensions = extensions;
        this.extensionOnly = true;
        this.isSupportedFormat = true;
    }

    private FileType(FileCategory category, String[] extensions, final int[]... matches) {
        this.imp = new Imp() {
            @Override
            boolean matches(byte[] header) {
                NextMatch: for( int m = 0; m<matches.length; m++ ) {
                    int[] match = matches[m];
                    if( header.length < match.length )
                        continue;
                    for( int i=0; i<match.length; i++ ) {
                        if( match[i]==MATCH.ANY )
                            continue;
                        if( header[i]!=(byte)match[i] )
                            continue NextMatch;
                    }
                    return true;
                }
                return false;
            }
        };
        this.category = category;
        this.extensions = extensions;
        this.extensionOnly = false;
        this.isSupportedFormat = true;
    }

    private static void makeExtensionMap() {
        if( !extensionMap.isEmpty() )
            return;
        for( FileType t: values() ) {
            for( String e: t.extensions ) {
                extensionMap.put(e,t);
            }
        }

    }

    public boolean isSupportedFormat() { return isSupportedFormat; }

    public boolean isImage() {
        return category==FileCategory.Image;
    }

    public boolean isVideo() {
        return category==FileCategory.Video || category==FileCategory.VideoOrAudio;
    }

    public boolean isAudio() {
        return category==FileCategory.Audio || category==FileCategory.VideoOrAudio;
    }

    public enum FileCategory {
        Audio, Video, VideoOrAudio, Image, Font;
    }

    private static abstract class Imp {
        abstract boolean matches(byte[] header);
    }

    public FileCategory getCategory() {
        return category;
    }

//    public static FileType fromPath( String path ) {
//        if( path==null )
//            return null;
//        return fromFile(new File(path));
//    }

    public static FileType fromExtension( String path ) {
        if( path==null )
            return null;
        int extension_idx = path.lastIndexOf('.');
        if( extension_idx < 0 )
            return null;
        int sep_idx = path.lastIndexOf('/');
        if( extension_idx<sep_idx )
            return null;
        String ext = path.substring(extension_idx+1).toLowerCase(Locale.US);
        if( extensionMap.isEmpty() )
            makeExtensionMap();
        return extensionMap.get(ext);
    }

    public static FileType fromExtension( File file ) {
        if( file==null )
            return null;

        String ext;
        String filename = file.getName();
        int extension_idx = filename.lastIndexOf('.');
        if( extension_idx>=0 ) {
            ext = filename.substring(extension_idx + 1).toLowerCase(Locale.US);
        } else {
            ext = null;
        }




        if( ext!=null ) {
            if( extensionMap.isEmpty() )
                makeExtensionMap();
            return extensionMap.get(ext);

//            for( FileType t: values() ) {
//                for( String e: t.extensions ) {
//                    if( e.equalsIgnoreCase(ext)) {
//                        if( LL.D ) Log.d(LOG_TAG, "FileType extension match: " + t.name());
//                        return t;
//                    }
//                }
//            }
        }
        return null;
    }

    public static FileType fromFile( String path ) {
        if( path==null )
            return null;
        return fromFile(new File(path));
    }

    public static FileType fromFile( File file ) {

        if( file==null )
            return null;

        String ext;
        String filename = file.getName();
        int extension_idx = filename.lastIndexOf('.');
        if( extension_idx>=0 ) {
            ext = filename.substring(extension_idx + 1);
        } else {
            ext = null;
        }

        // Check extension first ONLY for file types that
        // can only be identified by extension
        if( ext!=null ) {
            for( FileType t: values() ) {
                for( String e: t.extensions ) {
                    if( t.extensionOnly && e.equalsIgnoreCase(ext)) {
                        if( LL.D ) Log.d(LOG_TAG, "FileType extension match: " + t.name());
                        return t;
                    }
                }
            }
        }

        // For all other file types, attempt to identify the file
        // from the header
        byte[] header = null;

        if( file.exists() && file.length()>=CHECK_SIZE ) {
            header = new byte[CHECK_SIZE];
            int bytes_read = 0;
            try {
                FileInputStream is = new FileInputStream(file);
                try {
                    bytes_read = is.read(header);
                } finally {
                    is.close();
                }
            } catch (IOException e) {
                header=null;
            }
            if( header!=null && bytes_read>=CHECK_SIZE ) {
                FileType positiveMatch = null;
                for (FileType t : values()) {
                    if (t.imp.matches(header)) {
                        if( LL.D ) Log.d(LOG_TAG, "FileType analysis match: " + t.name());
                        if( positiveMatch!=null ) {
                            if( LL.D ) Log.d(LOG_TAG, "FileType analysis MULTIPLE match: " + t.name() + " (fall back to file extension)");
                            positiveMatch = null;
                            break;
                        }
                        positiveMatch = t;
                    }
                }
                if( positiveMatch!=null )
                    return positiveMatch;
            }
        }

        // If the header wasn't recognized, fall back
        // to checking the extension
        if( ext!=null ) {
            for( FileType t: values() ) {
                for( String e: t.extensions ) {
                    if( e.equalsIgnoreCase(ext)) {
                        if( LL.D ) Log.d(LOG_TAG, "FileType extension match: " + t.name() + " [" + (header==null?"null":StringUtil.bytesToHexStringUpper(header)) + "]");
                        return t;
                    }
                }
            }
        }
        return null;
    }

}

class MATCH{
    private MATCH(){}
    public static final int ANY = -1;
}

