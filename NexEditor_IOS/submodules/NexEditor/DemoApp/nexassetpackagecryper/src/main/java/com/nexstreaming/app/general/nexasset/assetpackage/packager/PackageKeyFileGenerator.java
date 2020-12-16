package com.nexstreaming.app.general.nexasset.assetpackage.packager;

import com.nexstreaming.app.general.util.CloseUtil;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;

/**
 * Created by robin.lee on 2017-02-06.
 */

public class PackageKeyFileGenerator {
    private static final String ENCRYPT_KEY_FILE = "keys.txt";
    private static final String ENCRYPT_KEY_META_FILE = "meta";

    private static void log(String s) {
        System.out.println(s);
    }

    private static final boolean LOGGING = true;

    private static String[] encodeByteKeys(byte[][] keys) {
        Random random = new Random();
        String[] hexaKeys=null;
        int arrayIndex=0;

        hexaKeys = new String[keys.length];

        for (byte[] arrayb : keys) {
            if ( arrayb == null ) {
                break;
            }
            int n = random.nextInt(0xEE)+0x11;
            String pKeyelement=String.format("%02X", n^32);

            for ( byte org : arrayb ) {
                byte b = (byte)((int)org^n);
                b = (byte) ((((b>>4)&0x0F) | ((b<<4)&0xF0)));
                pKeyelement += String.format("%02X", b);
            }
            hexaKeys[arrayIndex++] = pKeyelement;
        }

        if( LOGGING ) log("Key file hexa encoded");

        return hexaKeys;
    }

    private static boolean writeKeyInfo(String filePath, String customer, int[] hexaProviderID, String[] keys) {
        String string=new String();

        // write customer name
        string += "customer name = \""+customer+"\"\r\n\r\n";

        // write provider id
        string+="Provider ID = {";
        for ( int ii : hexaProviderID ) {
            string+= ii+", ";
        }
        string=string.substring(0, string.length()-2);
        string+="}\r\n\r\n";

        // write key array
        string += "Keys = {";
        for (String pStr : keys) {
            //if( LOGGING ) log("Hexa Key String : "+pStr);
            if (pStr != null) {
                string += "\r\n\""+pStr+"\",";
            } else {
                if( LOGGING ) log("Invalid key string!");
            }
        }
        string=string.substring(0, string.length()-1);
        string += "\r\n}";

        try {
            File keyFile = new File(filePath);
            keyFile.createNewFile();
            FileWriter fw = new FileWriter(keyFile);
            fw.write(string);
            fw.close();
            if( LOGGING ) log("Key file writed!");
        } catch (IOException e) {
            if( LOGGING ) log("Key output file IOException!");
            return false;
        }

        return true;
    }

    private static byte[][] loadByteKeyFromDir(String keyDirPath) {
        // Read each key and meta files. Then make key array.
        byte[][] keys=null;
        File keyDir = new File(keyDirPath);
        File[] pFileList = keyDir.listFiles();
        int keyFileCnt=0;
        int arrayIndex=0;

        if ( pFileList.length < 2 ) {
            if( LOGGING ) log("No key files!");
            return null;
        }

        keys=new byte[(int)pFileList.length][];

        for (File pFile : pFileList) {
            if ( pFile.getName().equals(ENCRYPT_KEY_FILE) ) {
                continue;
            }

            if ( pFile.getName().equals(ENCRYPT_KEY_META_FILE) ) {
                arrayIndex=0;
            } else {
                arrayIndex = ++keyFileCnt;
            }

            try {
                FileInputStream in = new FileInputStream(pFile);
                keys[arrayIndex] = new byte[(int)pFile.length()];

                in.read(keys[arrayIndex]);

                CloseUtil.closeSilently(in);
            } catch (FileNotFoundException exception) {
                if( LOGGING ) log("FileNotFoundException : Invalid file! : "+pFile);
            } catch (IOException exception) {
                if( LOGGING ) log("Input file IOException : IO error!");
            }
        }

        if ( keys[0] == null ) {
            if( LOGGING ) log("No key meta file! Key file loading fail.");
            return null;
        }

        if( LOGGING ) log("Key file loading success!");
        return keys;
    }

     private static int[] generateHexaProviderID(String customer) {
        int[] providerID = new int[customer.length()];

        for ( int i=0 ; i<customer.length() ; i++ ) {
            providerID[i] = customer.charAt(i) ^ 90;
        }

        return providerID;
    }

    public static boolean generateKeyInfoFile(String customer) throws IOException {
        byte[][] keys=null;
        String[] hexaKeys=null;
        String provider=null;
        String keyDirPath = System.getProperty("user.dir") + "\\" + customer.toUpperCase();

        // Veryfy Key folder
        File keyDirectory = new File(keyDirPath);
        if ( keyDirectory.exists() == false ) {
            if (new File(keyDirPath).mkdir() == false) {
                throw new IOException("Directory make fail! ");
            }
        } else if ( keyDirectory.isDirectory() == false ) {
            if (LOGGING) log("Key keyDirPath is not directory! : " + keyDirPath);
            throw new IOException("Key keyDirPath is not directory!");
        }

        // Make provider string
        provider = customer.toLowerCase() + ".bep";

        // Generate provider id hex value
        int[] hexaProviderID = generateHexaProviderID(provider);

        // Make key file keyDirPath
        String keyFilePath = keyDirPath + "\\" + ENCRYPT_KEY_FILE;

        // Delete Hexa Key file
        File keyFile = new File(keyFilePath);
        if (keyFile.exists()) {
            if (keyFile.delete()) {
                if (LOGGING) log("Key file deleted!");
            } else {
                if (LOGGING) log("Key file delte fail!");
                throw new IOException("Old key file isn't deleted!");
            }
        }

        // Key file loading from key directory
        keys = loadByteKeyFromDir(keyDirPath);
        if ( keys == null ) {
            if ( LOGGING ) log("Source key files read fail! Please verify key keyDirPath : " + keyDirPath);
            throw new IOException("Key info isn't loaded!");
        }

        // Make hexa encoding
        if ( (hexaKeys = encodeByteKeys(keys)) == null ) {
            if( LOGGING ) log("Hexa encoding error!");
            throw new IOException("Key hexa encoding error!");
        }

        // Write hexa key file
        if ( !writeKeyInfo(keyFilePath, customer.toUpperCase(), hexaProviderID, hexaKeys) ) {
            if( LOGGING ) log("Write hexa keys error!");
            return false;
        }

        return true;
    }
}
