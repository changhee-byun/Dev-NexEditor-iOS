package com.nexstreaming.app.common.nexasset.assetpackage.security.provider;

import com.google.gson_nex.Gson;
import com.nexstreaming.app.common.nexasset.assetpackage.security.PackageSecurityManager;
import com.nexstreaming.app.common.nexasset.assetpackage.security.SecurityProvider;
import com.nexstreaming.app.common.util.StreamUtil;
import com.nexstreaming.app.common.util.StringUtil;

import org.keyczar.Crypter;
import org.keyczar.exceptions.KeyczarException;
import org.keyczar.interfaces.KeyczarReader;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Random;
import java.nio.charset.StandardCharsets;

public class BasicEncryptionProvider implements SecurityProvider {
    private final int[] ID;
    private final String[] KS;

	public BasicEncryptionProvider(int[] sec_id, String[] sec_key) {
		ID = sec_id;
		KS = sec_key;
	}
	
    private int parsehex(String s, int start, int end) {
        int len = s.length();
        int result = 0;
        for( int i=start; i<end && i<len; i++ ) {
            char c = s.charAt(i);
            result *= 16;
            if( c >= '0' && c <= '9' ) result += (c-'0');
            else if( c >='a' && c <= 'f' ) result += (c-'a'+0xa);
            else if( c >='A' && c <= 'F' ) result += (c-'A'+0xa);
        }
        return result;
    }

    private String getKS(int index) {
        String ks = KS[index];
        int len = ks.length();
        int v = parsehex(ks,0,2) ^ 32;
        byte[] buf = new byte[(len-2)/2];
        for( int i=0; i<buf.length; i++ ) {
            byte b = (byte) parsehex(ks,2+i*2,4+i*2);
            b = (byte) ((((b>>4)&0x0F) | ((b<<4)&0xF0))^v);
            buf[i] = b;
        }
        return new String(buf);
    }

    private KeyczarReader keyczarReader = new KeyczarReader() {
        @Override
        public String getKey(int version) throws KeyczarException {
            return getKS(version);
        }

        @Override
        public String getKey() throws KeyczarException {
            return getKS(1);
        }

        @Override
        public String getMetadata() throws KeyczarException {
            return getKS(0);
        }
    };

    private String getStringValue(String fullStr, String conditionStr) {
        int idx = 0;
        String str=fullStr;

        idx = str.indexOf(conditionStr);
        str = str.substring(idx);

        idx = str.indexOf(":");
        str = str.substring(idx);

        idx = str.indexOf("\"")+1;
        str = str.substring(idx);

        idx = str.indexOf("\"");
        str = str.substring(0, idx);

        return str;
    }

    private String encryptString(String str) {
        byte[] byteArray=str.getBytes(StandardCharsets.US_ASCII);
        Random random = new Random();

        int n = random.nextInt(0xEE)+0x11;
        String pKeyelement=String.format("%02X", n^32);

        for ( byte org : byteArray ) {
            byte b = (byte)((int)org^n);
            b = (byte) ((((b>>4)&0x0F) | ((b<<4)&0xF0)));
            pKeyelement += String.format("%02X", b);
        }

        return pKeyelement;
    }

    @Override
    public String[] getEncInfo() {
        String[] array=new String[12];
        String ss=null, hmacValue=null, keyStr=null;
        int j=0, i=0;
        for (i=0; i<6 ; i++) {
            try {
                ss=keyczarReader.getKey(i+1);
            } catch (KeyczarException e) {
                break;
            }
            hmacValue = getStringValue(ss, "hmacKeyString");
            hmacValue = hmacValue.length() > 16 ? hmacValue.substring(hmacValue.length() - 16, hmacValue.length()) : hmacValue;
            array[j++] = encryptString(hmacValue);
            keyStr = getStringValue(ss, "aesKeyString");
            keyStr = keyStr.length() > 16 ? keyStr.substring(keyStr.length() - 16, keyStr.length()) : keyStr;
            array[j++] = encryptString(keyStr);
        }

        return array;
    }

    @Override
    public String getProviderId() {
        char[] id = new char[ID.length];
        for( int i=0; i<ID.length; i++) id[i] = (char) (ID[i] ^ 90);
        return new String(id);
    }

    private class ProviderSpecificDataJSON {
        int v; // Provider vesion
        List<String> f; // Paths of encrypted files
    }

    @Override
    public PackageSecurityManager getSecurityManagerForPackage(String providerSpecificData) {

        ProviderSpecificDataJSON psd = new Gson().fromJson(providerSpecificData,ProviderSpecificDataJSON.class);
        final Set<String> encryptedFiles = new HashSet<>();
        encryptedFiles.addAll(psd.f);
        final int providerVersion = psd.v;
        final Crypter c;
        try {
            c = new Crypter(keyczarReader);
        } catch (KeyczarException e) {
            throw new IllegalStateException();
        }

        return new PackageSecurityManager() {

            ByteArrayOutputStream byteArray = new ByteArrayOutputStream();

            @Override
            public InputStream wrapInputStream(InputStream inputStream, String path) throws IOException {
                if( encryptedFiles.contains(path)) {
                    synchronized (this) {
                        byteArray.reset();
                        StreamUtil.copy(inputStream,byteArray);
                        inputStream.close();
                        try {
                            return new ByteArrayInputStream(c.decrypt(byteArray.toByteArray()));
                        } catch (KeyczarException e) {
                            e.printStackTrace();
                            throw new IllegalStateException();
                        }
                    }
                } else {
                    return inputStream;
                }
            }

            @Override
            public boolean plaintextAvailable(String path) {
                return !encryptedFiles.contains(path);
            }
        };

    }

}
