package com.nexstreaming.app.common.localprotocol;

import android.util.Base64;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Random;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.KeyGenerator;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;
import javax.crypto.spec.SecretKeySpec;

/**
 * Created by jeongwook.yoon on 2016-10-05.
 */
public class nexCipher {
    private static final String algorithm = "AES";
    private static final String transformation = algorithm + "/ECB/PKCS5Padding";
    private Key key;
    private OutputStream outputStream;
    Cipher mCipher;

    public nexCipher(){
    }

    public nexCipher(Key key){
        this.key = key;
    }

    public nexCipher(String byte16 ){
        SecretKeySpec key = new SecretKeySpec(byte16.getBytes(), algorithm);
        this.key = key;
    }

    public void openEncOutputFile(File dest) throws Exception {
        mCipher = Cipher.getInstance(transformation);
        mCipher.init(Cipher.ENCRYPT_MODE, key);
        outputStream = new BufferedOutputStream(new FileOutputStream(dest));
    }

    public void writeEncOutputFile(byte[] buffer){
        try {
            outputStream.write(mCipher.update(buffer, 0, buffer.length));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void closeEncOutputFile(){
        try {
            outputStream.write(mCipher.doFinal());
        } catch (IOException e) {
            e.printStackTrace();
        } catch (IllegalBlockSizeException e) {
            e.printStackTrace();
        } catch (BadPaddingException e) {
            e.printStackTrace();
        }

        try {
            outputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void printZipEntrys(File zipfile) throws Exception {
        //FileOutputStream fileOutputStream = null;
        final Cipher cipher = Cipher.getInstance(transformation);
        cipher.init(Cipher.DECRYPT_MODE, key);

        FileInputStream fis = new FileInputStream(zipfile);
        CipherInputStream  cis = new CipherInputStream(fis, cipher);
        ZipInputStream zipinputstream =new ZipInputStream(cis);

        ZipEntry zipentry;

        zipentry = zipinputstream.getNextEntry();
        while (zipentry != null) {
//            Log.d("Zip", "Zip entry name="+zipentry.getName());
            zipinputstream.closeEntry();
            zipentry = zipinputstream.getNextEntry();
        }
        zipinputstream.close();
        cis.close();
        fis.close();
    }

    public static void encrypt(File source, File dest, String strkey) throws Exception {
        InputStream input = new BufferedInputStream(new FileInputStream(source));
        OutputStream output = new BufferedOutputStream(new FileOutputStream(dest));
        SecretKeySpec enckey = new SecretKeySpec(strkey.getBytes(), algorithm);

        crypt(Cipher.ENCRYPT_MODE, input, output, enckey);
    }

    public static void decrypt(InputStream input, OutputStream output, byte[] strkey) throws Exception {

        SecretKeySpec deckey = new SecretKeySpec(strkey, algorithm);

        crypt(Cipher.DECRYPT_MODE, input, output, deckey);
    }

    public static byte[] decrypt(byte[] input, byte[] strkey) throws Exception {

        SecretKeySpec deckey = new SecretKeySpec(strkey, algorithm);
        Cipher cipher = Cipher.getInstance(transformation);
        cipher.init(Cipher.DECRYPT_MODE, deckey);
        return cipher.doFinal(input);
    }

    public static void crypt(int mode, InputStream input, OutputStream output, Key crykey) throws Exception {
        Cipher cipher = Cipher.getInstance(transformation);
        cipher.init(mode, crykey);
        try {
            byte[] buffer = new byte[1024];
            int read = -1;
            while ((read = input.read(buffer)) != -1) {
                output.write(cipher.update(buffer, 0, read));
            }
            output.write(cipher.doFinal());
        } finally {
            if (output != null) {
                try {
                    output.close();
                } catch (IOException ie) {
                }
            }
            if (input != null) {
                try {
                    input.close();
                } catch (IOException ie) {
                }
            }
        }
    }

    public static String makeKeyString(){
        Random rnd =new Random();
        StringBuffer buf =new StringBuffer();

        for(int i=0;i<16;i++){
            if(rnd.nextBoolean()){
                buf.append((char)((int)(rnd.nextInt(26))+97));
            }else{
                buf.append((rnd.nextInt(10)));
            }
        }
        return buf.toString();
    }

    public static byte[] hexToByteArray(String hex) {
        if (hex == null || hex.length() == 0) {
            return null;
        }
        byte[] ba = new byte[hex.length() / 2];
        for (int i = 0; i < ba.length; i++) {
            ba[i] = (byte) Integer.parseInt(hex.substring(2 * i, 2 * i + 2), 16);
        }
        return ba;
    }

    // byte[] to hex sting
    public static String byteArrayToHex(byte[] ba) {
        if (ba == null || ba.length == 0) {
            return null;
        }
        StringBuffer sb = new StringBuffer(ba.length * 2);
        String hexNumber;
        for (int x = 0; x < ba.length; x++) {
            hexNumber = "0" + Integer.toHexString(0xff & ba[x]);

            sb.append(hexNumber.substring(hexNumber.length() - 2));
        }
        return sb.toString();
    }

    private String RSApubKeyHex = null;
    private String RSAprivKeyHex = null;

    private final boolean fastRSA = true;

    public void fastRSAGenKey(){
        KeyPairGenerator generator = null; // OK
        try {
//            Log.d("RSA","RSAGenKey test1");
            generator = KeyPairGenerator.getInstance("RSA");
//            Log.d("RSA","RSAGenKey test2");
            generator.initialize(2048);
//            Log.d("RSA", "RSAGenKey test3");
            KeyPair keyPair = generator.genKeyPair();
//            Log.d("RSA","RSAGenKey test4");
            Key pubKey = keyPair.getPublic();  // Kb(pub) 공개??
            Key privKey = keyPair.getPrivate();// Kb(pri) 개인??

            RSApubKeyHex = byteArrayToHex(pubKey.getEncoded());
            RSAprivKeyHex = byteArrayToHex(privKey.getEncoded());
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
    }

    public byte[] fastRSAEncrypt(byte[] input ) throws NoSuchAlgorithmException, NoSuchProviderException, NoSuchPaddingException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException {
        Cipher cipher = null;
        cipher = Cipher.getInstance("RSA");

        X509EncodedKeySpec ukeySpec = new X509EncodedKeySpec(hexToByteArray(RSApubKeyHex));
        KeyFactory ukeyFactory = KeyFactory.getInstance("RSA");
        PublicKey publicKey = null;
        try {
            publicKey = ukeyFactory.generatePublic(ukeySpec);
//            Log.d("RSA","pubKeyHex:" + byteArrayToHex(publicKey.getEncoded()));
        } catch (InvalidKeySpecException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        return cipher.doFinal(input);
    }

    public byte[] fastRSADecrypt(byte[] input) throws NoSuchAlgorithmException, NoSuchProviderException, NoSuchPaddingException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException {
        Cipher cipher = null;
        cipher = Cipher.getInstance("RSA");

        PKCS8EncodedKeySpec rkeySpec = new PKCS8EncodedKeySpec(hexToByteArray(RSAprivKeyHex));
        KeyFactory rkeyFactory  = KeyFactory.getInstance("RSA");
        PrivateKey privateKey  = null;
        try {
            privateKey  = rkeyFactory.generatePrivate(rkeySpec);
            //Log.d("RSA","privKeyHex:" + byteArrayToHex(privateKey.getEncoded()));
        } catch (InvalidKeySpecException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        cipher.init(Cipher.DECRYPT_MODE, privateKey);
        return cipher.doFinal(input);
    }

    public void RSAGenKey(){
        SecureRandom random = new SecureRandom();
        //KeyPairGenerator generator = KeyPairGenerator.getInstance("DiffieHellman", "SunJCE"); Not an RSA key: DH
        //KeyPairGenerator generator = KeyPairGenerator.getInstance("RSA", "SunRsaSign"); // OK
        KeyPairGenerator generator = null; // OK
        try {
            generator = KeyPairGenerator.getInstance("RSA", "BC");
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (NoSuchProviderException e) {
            e.printStackTrace();
        }

        generator.initialize(2048, random);
        KeyPair pair = generator.generateKeyPair();
        Key pubKey = pair.getPublic();  // Kb(pub) 공개??
        Key privKey = pair.getPrivate();// Kb(pri) 개인??



        RSApubKeyHex = byteArrayToHex(pubKey.getEncoded());
        RSAprivKeyHex = byteArrayToHex(privKey.getEncoded());
        /*
        Log.d("RSA","pubKeyHex:"+RSApubKeyHex);
        Log.d("RSA","pubKey getAlgorithm:"+pubKey.getAlgorithm());
        Log.d("RSA","pubKey getFormat:"+pubKey.getFormat());
        Log.d("RSA","privKeyHex:"+RSAprivKeyHex);

        Log.d("RSA","privKey getAlgorithm:"+privKey.getAlgorithm());
        Log.d("RSA","privKey getFormat:"+privKey.getFormat());
        */
    }

    public byte[] RSAEncrypt(byte[] input ) throws NoSuchAlgorithmException, NoSuchProviderException, NoSuchPaddingException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException {
        Cipher cipher = Cipher.getInstance("RSA/None/NoPadding", "BC");

        X509EncodedKeySpec ukeySpec = new X509EncodedKeySpec(hexToByteArray(RSApubKeyHex));
        KeyFactory ukeyFactory = KeyFactory.getInstance("RSA");
        PublicKey publicKey = null;
        try {
            publicKey = ukeyFactory.generatePublic(ukeySpec);
//            Log.d("RSA","pubKeyHex:" + byteArrayToHex(publicKey.getEncoded()));
        } catch (InvalidKeySpecException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        return cipher.doFinal(input);
    }

    public byte[] RSADecrypt(byte[] input) throws NoSuchAlgorithmException, NoSuchProviderException, NoSuchPaddingException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException {
        Cipher cipher = Cipher.getInstance("RSA/None/NoPadding", "BC");

        PKCS8EncodedKeySpec rkeySpec = new PKCS8EncodedKeySpec(hexToByteArray(RSAprivKeyHex));
        KeyFactory rkeyFactory  = KeyFactory.getInstance("RSA");
        PrivateKey privateKey  = null;
        try {
            privateKey  = rkeyFactory.generatePrivate(rkeySpec);
            //Log.d("RSA","privKeyHex:" + byteArrayToHex(privateKey.getEncoded()));
        } catch (InvalidKeySpecException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        cipher.init(Cipher.DECRYPT_MODE, privateKey);
        return cipher.doFinal(input);
    }

    public void SHAChecksum(File encFile) throws Exception {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        FileInputStream fis = new FileInputStream(encFile);
        byte[] dataBytes = new byte[1024];
        int nread = 0;
        while ((nread = fis.read(dataBytes)) != -1) {
            md.update(dataBytes, 0, nread);
        };
        byte[] mdbytes = md.digest();
        String str = Base64.encodeToString(mdbytes,Base64.DEFAULT);
//        Log.d("SHA","Base64 enc="+str);
    }

    public byte[] getRSApubKey(){
        return hexToByteArray(RSApubKeyHex);
    }
    public byte[] getRSAprivKey(){
        return hexToByteArray(RSAprivKeyHex);
    }
}
