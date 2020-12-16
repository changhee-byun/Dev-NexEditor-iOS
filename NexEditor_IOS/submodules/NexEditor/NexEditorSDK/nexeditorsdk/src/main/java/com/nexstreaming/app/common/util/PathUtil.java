package com.nexstreaming.app.common.util;


public class PathUtil {
    private PathUtil(){} // Prevent instantiation

    public static String combinePaths( String a, String b ) {
        if( b.startsWith("..") || b.contains( "/..") )
            throw new SecurityException("Parent Path References Not Allowed");
        if(a.endsWith("/")) {
            return a + b;
        } else {
            return a + "/" + b;
        }
    }

    public static String relativePath( String a, String b ) {
        if( b.startsWith("..") || b.contains( "/..") )
            throw new SecurityException("Parent Path References Not Allowed");
        if(a.endsWith("/")) {
            return a + b;
        } else {
            int lastSlash = a.lastIndexOf('/');
            if( lastSlash<0 )
                return b;
            return a.substring(0,lastSlash+1) + b;
        }
    }

    public static String getExt(String path) {
        int lastSlash = path.lastIndexOf('/');
        int lastDot = path.lastIndexOf('.');
        if( lastDot < lastSlash || lastDot<0 )
            return "";
        return path.substring(lastDot+1);
    }

    public static String getParent(String path) {

        if( path==null || path.length()<1 )
            return null;

        if( path.charAt(path.length()-1)=='/' ) {
            path = path.substring(0,path.length()-1);
        }

        int lastSlash = path.lastIndexOf('/');
        if( lastSlash<0 )
            return null;

        return path.substring(0,lastSlash+1);
    }

    public static String getName(String path) {
        int lastSlash = path.lastIndexOf('/');
        if( lastSlash<0 )
            return path;
        return path.substring(lastSlash+1);
    }

    public static String getBasePath(String path) {
        int lastSlash = path.lastIndexOf('/');
        if( lastSlash<0 )
            return "";
        return path.substring(0,lastSlash+1);
    }


}
