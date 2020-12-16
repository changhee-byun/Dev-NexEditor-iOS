//package com.nexstreaming.app.general.nexasset.assetpackage.packager;
//
//
//import android.util.Log;
//
//import com.google.gson.Gson;
//import com.nexstreaming.app.general.util.CloseUtil;
//import com.nexstreaming.app.general.util.StreamUtil;
//
//import org.keyczar.Crypter;
//import org.keyczar.exceptions.KeyczarException;
//import org.keyczar.interfaces.KeyczarReader;
//
//import java.io.ByteArrayOutputStream;
//import java.io.File;
//import java.io.FileInputStream;
//import java.io.FileOutputStream;
//import java.io.IOException;
//import java.util.ArrayList;
//import java.util.List;
//import java.util.zip.ZipEntry;
//import java.util.zip.ZipInputStream;
//import java.util.zip.ZipOutputStream;
//
//public class PackageCrypter {
//
//    private static final boolean LOGGING = true;
//    private static final String ENCRYPTION_INFO_FILE = "e.json";
//
//    private PackageCrypter(){} // Prevent instantiation
//
//    private static void log(String s) {
//        Log.d("PackageCrypter",s);
////        System.out.println(s);
//    }
//
//    public interface EntryFilter {
//        boolean shouldEncryptEntry(ZipEntry entry);
//    }
//
//    private static class ProviderSpecificDataJSON {
//        int v;              // Provider vesion
//        List<String> f;     // Paths of encrypted files
//    }
//
//    private static class EncryptionInfoJSON {
//        public String provider;
//        public String psd;          // Provider-Specific Data
//    }
//
//    public static void encryptPackage(File inputPath, File outputPath, EntryFilter entryFilter) throws IOException, KeyczarException {
//
//        if( LOGGING ) log("Encrypting from '" + inputPath + "' to '" + outputPath + "'");
//        ZipInputStream in = new ZipInputStream(new FileInputStream(inputPath));
//        ZipOutputStream out = new ZipOutputStream(new FileOutputStream(outputPath));
//        Crypter crypter = new Crypter(keyczarReader);
//        ProviderSpecificDataJSON psd = new ProviderSpecificDataJSON();
//        psd.v = 1;
//        psd.f = new ArrayList<>();
//        try {
//            ZipEntry zipEntry;
//            while ((zipEntry = in.getNextEntry()) != null) {
//                String name = zipEntry.getName();
//                if (name.contains("..")) {
//                    throw new IOException("Relative paths not allowed"); // For security purposes
//                }
////                out.putNextEntry(zipEntry);
//                if (!zipEntry.isDirectory()) {
//                    out.putNextEntry(new ZipEntry(zipEntry.getName()));
//                    if( LOGGING ) log("  - processing file '" + name + "' " + zipEntry.getCompressedSize() + "->" + zipEntry.getSize());
//                    if( entryFilter.shouldEncryptEntry(zipEntry) ) {
//                        ByteArrayOutputStream data = new ByteArrayOutputStream();
//                        StreamUtil.copy(in,data);
//                        out.write(crypter.encrypt(data.toByteArray()));
//                        psd.f.add(zipEntry.getName());
//                    } else {
//                        StreamUtil.copy(in, out);
//                    }
//                    out.closeEntry();
//                }
//            }
//            ZipEntry ejson = new ZipEntry(ENCRYPTION_INFO_FILE);
//            out.putNextEntry(ejson);
//            EncryptionInfoJSON eij = new EncryptionInfoJSON();
//            eij.provider = "nex.bep";
//            eij.psd = new Gson().toJson(psd);
//            out.write(new Gson().toJson(eij).getBytes());
//            out.closeEntry();
//            out.finish();
//        } finally {
//            CloseUtil.closeSilently(in);
//            CloseUtil.closeSilently(out);
//        }
//        if( LOGGING) log("Encrypting DONE for: '" + inputPath + "' to '" + outputPath + "'");
//
//
//    }
//
//    private static final String[] KEYS = {
//        "{\"encrypted\": false, \"versions\": [{\"status\": \"PRIMARY\", \"versionNumber\": 1, \"exportable\": false}, {\"status\": \"ACTIVE\", \"versionNumber\": 2, \"exportable\": false}, {\"status\": \"ACTIVE\", \"versionNumber\": 3, \"exportable\": false}, {\"status\": \"ACTIVE\", \"versionNumber\": 4, \"exportable\": false}, {\"status\": \"ACTIVE\", \"versionNumber\": 5, \"exportable\": false}, {\"status\": \"ACTIVE\", \"versionNumber\": 6, \"exportable\": false}], \"type\": \"AES\", \"name\": \"assetcrypt\", \"purpose\": \"DECRYPT_AND_ENCRYPT\"}",
//        "{\"hmacKey\": {\"hmacKeyString\": \"9iskB9Q5rqOyVTrou-FTKn3KLme8OIHpzGptzARLhmo\", \"size\": 256}, \"aesKeyString\": \"sRrkHI3zxlAEvPNTB15eLQ\", \"mode\": \"CBC\", \"size\": 128}",
//        "{\"hmacKey\": {\"hmacKeyString\": \"K0hdv4hHK8j50bwG25pfSGkPBAQBAAEnT5I3my8QlpM\", \"size\": 256}, \"aesKeyString\": \"jlhf9wsmtrq9Z8i03V38xA\", \"mode\": \"CBC\", \"size\": 128}",
//        "{\"hmacKey\": {\"hmacKeyString\": \"bv60n4Tg8zUN0UxZgXd8n3QIS3CKgeVQ7hnY3o6Fkvc\", \"size\": 256}, \"aesKeyString\": \"Yshj6FRLIKh_beCWHr5KNg\", \"mode\": \"CBC\", \"size\": 128}",
//        "{\"hmacKey\": {\"hmacKeyString\": \"I3LH52TugogYgvnx0MJBYp1PSmfaVzE8VeMd-_jgZ6k\", \"size\": 256}, \"aesKeyString\": \"UpOKqjAmKEqgOo3_pYsjgw\", \"mode\": \"CBC\", \"size\": 128}",
//        "{\"hmacKey\": {\"hmacKeyString\": \"seFKSCOtTOfP11gfsa99yXaSNICQ615pnTMj1wpRyP4\", \"size\": 256}, \"aesKeyString\": \"f-jvka_J5OCQprg0SOp1QQ\", \"mode\": \"CBC\", \"size\": 128}",
//        "{\"hmacKey\": {\"hmacKeyString\": \"PQYbXTT91GtHFvbsDtzIdTD18J5_Hj0JKSMNhiHa8Jk\", \"size\": 256}, \"aesKeyString\": \"qx5ja4ccKlagtNGZon6NZg\", \"mode\": \"CBC\", \"size\": 128}"
//    };
//
//    private static KeyczarReader keyczarReader = new KeyczarReader() {
//        @Override
//        public String getKey(int version) throws KeyczarException {
//            return KEYS[version];
//        }
//
//        @Override
//        public String getKey() throws KeyczarException {
//            return KEYS[1];
//        }
//
//        @Override
//        public String getMetadata() throws KeyczarException {
//            return KEYS[0];
//        }
//    };
//
//
//}
