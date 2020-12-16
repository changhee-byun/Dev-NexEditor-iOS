package com.nexstreaming.app.general.nexasset.assetpackage.packager;

import com.google.gson.Gson;
import com.google.gson.stream.JsonWriter;
import com.nexstreaming.app.general.util.CloseUtil;
import com.nexstreaming.app.general.util.StreamUtil;

import org.keyczar.Crypter;
import org.keyczar.exceptions.KeyczarException;
import org.keyczar.interfaces.KeyczarReader;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;
import java.util.zip.ZipFile;
import java.util.Random;

public class PackageCrypter {

    private enum MP4BoxCommand {
        VERSION, RUN
    }

    private static String xmlContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "<GPACDRM type=\"CENC AES-CTR\">\n" +
            "<DRMInfo type=\"pssh\" version=\"1\" cypherOffset=\"20\" cypherKey=\"0x00000000000000000000000000000000\" cypherIV=\"0x00000000000000000000000000000001\">\n" +
            "<BS ID128=\"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\"/>\n" +
            "<BS value=\"2\" bits=\"32\"/>\n" +
            "<BS ID128=\"0x11111111111111111111111111111111\"/>\n" +
            "<BS ID128=\"0x22222222222222222222222222222222\"/>\n" +
            "<BS string=\"CID=ASSET00034821492\"/>\n" +
            "</DRMInfo>\n" +
            "<CrypTrack trackID=\"1\" IsEncrypted=\"1\" IV_size=\"16\" first_IV=\"0x00000000000000000000000000000002\" selectiveType=\"RAP\" saiSavedBox=\"senc\" >\n" +
            "<key KID=\"0x11111111111111111111111111111111\" value=\"0x11111111111111111111111111111100\"/>\n" +
            "</CrypTrack>\n" +
            "<CrypTrack trackID=\"2\" IsEncrypted=\"1\" IV_size=\"16\" first_IV=\"0x00000000000000000000000000000003\" selectiveType=\"RAP\" saiSavedBox=\"senc\" >\n" +
            "<key KID=\"0x22222222222222222222222222222222\" value=\"0x22222222222222222222222222222200\"/>\n" +
            "</CrypTrack>\n" +
            "</GPACDRM>";
    private static final String TEMP_IN_FILE = "tempIN.tmp";
    private static final String TEMP_OUT_FILE = "tempOut.tmp";
    private static final String TEMP_XML_FILE = "drm.xml";

    private static final boolean LOGGING = false;
    private static final String ENCRYPTION_INFO_FILE = "e.json";

    private String[] hexakeys=null;
    private String providerid=null;

    private static void log(String s) {
        System.out.println(s);
    }

    public interface OutputWriter {
        // Entry
        void setEntry(String name) throws IOException;
        boolean isDirectory();
        String getName();
        String getPath();
        long getCompressedSize();
        long getSize();
        void closeEntry() throws  IOException;

        // Stream
        void write(byte[] data) throws IOException;
        OutputStream getOutputStream();
        void finish() throws IOException;
    }

    private static class ZipWriter implements OutputWriter {
        private final ZipOutputStream out;
        private ZipEntry entry;

        public ZipWriter(File file) throws IOException {
            out = new ZipOutputStream(new FileOutputStream(file));
            entry = null;
        }

        @Override
        public void setEntry(String name) throws  IOException {
            entry = new ZipEntry(name);
            out.putNextEntry(entry);
            return;
        }

        @Override
        public boolean isDirectory() {
            return entry.isDirectory();
        }

        @Override
        public String getName() {
            return entry.getName();
        }

        @Override
        public String getPath() {
            return entry.getName();
        }

        @Override
        public long getCompressedSize() {
            return entry.getCompressedSize();
        }

        @Override
        public long getSize() {
            return entry.getSize();
        }

        @Override
        public void closeEntry() throws  IOException {
            out.closeEntry();
            return;
        }

        @Override
        public void write(byte[] data) throws IOException {
            out.write(data);
            return;
        }

        @Override
        public OutputStream getOutputStream() {
            return out;
        }

        @Override
        public void finish() throws IOException {
            out.finish();
        }
    }

    private static class FolderWriter implements OutputWriter {
        private final String folderPath;
        private FileOutputStream out;
        private File outf;
        private boolean isFolder;
        private String fullPath;
        private String entryName;

        public FolderWriter(String folder) throws IOException {
            folderPath = folder;
            entryName = null;
            out = null;
            outf = null;
            isFolder = false;
            fullPath = null;

            File f = new File(folderPath);
            if ( f.mkdirs() == false ) {
                if( LOGGING ) log("Root folder create fail! "+folder);
            }
        }

        @Override
        public void setEntry(String name) throws IOException {
            CloseUtil.closeSilently(out);
            outf = null;

            entryName = name;
            fullPath = folderPath + File.separator + entryName;
            if (fullPath.endsWith("/")) {
                fullPath = fullPath.substring(0, fullPath.length() - 1);
                isFolder = true;
            } else {
                isFolder = false;
                out = new FileOutputStream(fullPath);
            }

            outf = new File(fullPath);
            if (isFolder==true) {
                if ( outf.mkdirs() == false ) {
                    if( LOGGING ) log("Sub folder create fail! "+fullPath);
                }
            }
            return;
        }

        @Override
        public boolean isDirectory() {
            return isFolder;
        }

        @Override
        public String getName() {
            return entryName;
        }

        @Override
        public String getPath() {
            return fullPath;
        }

        @Override
        public long getCompressedSize() {
            return outf.length();
        }

        @Override
        public long getSize() {
            return outf.length();
        }

        @Override
        public void closeEntry() throws  IOException {
            if ( out != null ) {
                out.close();
                out = null;
            }
            outf = null;
            isFolder = false;
            fullPath = null;
            return;
        }

        @Override
        public void write(byte[] data) throws IOException {
            out.write(data);
            return;
        }

        @Override
        public OutputStream getOutputStream() {
            return out;
        }

        @Override
        public void finish() throws IOException {
            closeEntry();
            return;
        }
    }

    public interface InputReader {
        InputStream openFile(String path) throws FileNotFoundException, IOException;
        Iterable<String> listFiles();
        String getEntryName(String path);
        long getSize(String path);
        public void close() throws IOException;
    }

    private static class ZipInputReader implements InputReader {
        private final ZipFile zipFile;
        private final File zipPath;

        public ZipInputReader(File path) throws IOException {
            zipFile = new ZipFile(path);
            zipPath = path;
        }

        @Override
        public InputStream openFile(String path) throws FileNotFoundException, IOException {
            ZipEntry entry = zipFile.getEntry(path);
            if( entry==null ) {
                //entry = zipFile.getEntry(PathUtil.combinePaths(PathUtil.getBasePath(path),PathUtil.getName(path).toLowerCase(Locale.ENGLISH)));
                if( entry==null ) {
                    throw new FileNotFoundException("File '" + path + "' not found in '" + zipFile.getName() + "'");
                }
            }
            return zipFile.getInputStream(entry);
        }

        @Override
        public Iterable<String> listFiles() {
            return new Iterable<String>() {
                @Override
                public Iterator<String> iterator() {
                    final Enumeration<? extends ZipEntry> entries = zipFile.entries();
                    return new Iterator<String>() {
                        @Override
                        public boolean hasNext() {
                            return entries.hasMoreElements();
                        }

                        @Override
                        public String next() {
                            return entries.nextElement().getName();
                        }

                        @Override
                        public void remove() {
                            throw new UnsupportedOperationException();
                        }
                    };
                }
            };
        }

        @Override
        public void close() throws IOException {
            zipFile.close();
        }

        @Override
        public String getEntryName(String path) {
            return zipFile.getEntry(path).getName();
        }

        @Override
        public long getSize(String path) {
            return zipFile.getEntry(path).getSize();
        }
    }

    private static class FolderInputReader implements InputReader {

        private final File folderPath;

        private FolderInputReader(File folderPath) {
            this.folderPath = folderPath;
        }

        @Override
        public InputStream openFile(String path) throws FileNotFoundException, IOException {
            return new FileInputStream(getAbsolutePath(path));
        }

        private String getContainerRelativePath(File absPath) {
            String full = absPath.getAbsolutePath();
            String base = folderPath.getAbsolutePath();
            if( !full.startsWith(base) ) {
                throw new IllegalStateException();
            }
            if( full.length()<=base.length() ) {
                return "";
            }
            if( full.charAt(base.length())=='/' ) {
                return full.substring(base.length()+1).replaceAll("\\\\", "/");
            } else {
                return full.substring(base.length()).replaceAll("\\\\", "/");
            }
        }

        private File getAbsolutePath(String containerRelativePath) {
            return new File(folderPath,containerRelativePath);
        }

        @Override
        public Iterable<String> listFiles() {
            return new Iterable<String>() {
                @Override
                public Iterator<String> iterator() {

                    final List<File> toDoList = new ArrayList<>();
                    File[] files = folderPath.listFiles();
                    if( files!=null ) {
                        for (File f:files) {
                            toDoList.add(f);
                        }
                    }

                    return new Iterator<String>() {
                        @Override
                        public boolean hasNext() {
                            return !toDoList.isEmpty();
                        }

                        @Override
                        public String next() {
                            File nextfile = toDoList.remove(0);
                            if( nextfile.isDirectory() ) {
                                File[] files = nextfile.listFiles();
                                if( files!=null ) {
                                    for (File f:files) {
                                        toDoList.add(f);
                                    }
                                }
                            }
                            return getContainerRelativePath(nextfile);
                        }

                        @Override
                        public void remove() {

                        }
                    };
                }
            };
        }

        @Override
        public void close() throws IOException {
            // Nothing to do
        }

        @Override
        public String getEntryName(String path) {
            File file=getAbsolutePath(path);
            String ret = file.isDirectory() ? path.substring(1)+"/" : path.substring(1);
            return ret;//path.substring(1);
        }

        @Override
        public long getSize(String path) {
            File file=getAbsolutePath(path);
            return file.length();
        }
    }

    public interface EntryFilter {
        boolean shouldEncryptEntry(OutputWriter entry);
    }

    private static class ProviderSpecificDataJSON {
        int v;          // Provider vesion
        List<String> f; // Paths of encrypted files
    }

    private static class EncryptionInfoJSON {
        public String provider;
        public String psd;          // Provider-Specific Data
    }

    private String getProviderId(int[] provider) {
        char[] id = new char[provider.length];
        for( int i=0; i<provider.length; i++) id[i] = (char) (provider[i] ^ 90);
        return new String(id);
    }

    private String getRandomIV(int length) {
        Random rnd = new Random();
        byte[] iv = new byte[length];
        rnd.nextBytes(iv);
        StringBuilder sb = new StringBuilder();
        for(byte b: iv) {
            sb.append(String.format("%02X", b&0xff));
        }
        return sb.toString();
    }

    private void updateXML() {
        String[] info = getEInfo();
        int infoCnt = info.length;
        Random rnd = new Random();
        int idx = 0;

        //if ( LOGGING ) log("XML content = " +xmlContent);
        xmlContent = xmlContent.replaceAll("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", toHex(getDecryptedString(info[infoCnt-1])));//systemid

        idx = rnd.nextInt(infoCnt);
        idx = idx - idx%2;
        xmlContent = xmlContent.replaceAll("11111111111111111111111111111111", toHex(getDecryptedString(info[idx])));//first kid
        xmlContent = xmlContent.replaceAll("11111111111111111111111111111100", toHex(getDecryptedString(info[idx+1])));//first key

        idx = rnd.nextInt(infoCnt);
        idx = idx - idx%2;
        xmlContent = xmlContent.replaceAll("22222222222222222222222222222222", toHex(getDecryptedString(info[idx])));//second kid
        xmlContent = xmlContent.replaceAll("22222222222222222222222222222200", toHex(getDecryptedString(info[idx+1])));//second key

        xmlContent = xmlContent.replaceAll("00000000000000000000000000000000", getRandomIV(16));
        xmlContent = xmlContent.replaceAll("00000000000000000000000000000001", getRandomIV(16));
        xmlContent = xmlContent.replaceAll("00000000000000000000000000000002", getRandomIV(16));
        xmlContent = xmlContent.replaceAll("00000000000000000000000000000003", getRandomIV(16));

        //if ( LOGGING ) log("XML content2 = " +xmlContent);
    }

    public PackageCrypter(int[] provider, String[] keys) {
        providerid = getProviderId(provider);
        hexakeys = keys;
        updateXML();
    }

    private boolean deleteFolder(File src) {
        if(!src.exists()) {
            return false;
        }
        File[] files = src.listFiles();

        for (File file : files) {
            if (file.isDirectory()) {
                deleteFolder(file);
            } else {
                file.delete();
            }
        }
        return src.delete();
    }

    // Added to support JAVA 7
    boolean isAlive(Process process) {
        try {
            process.exitValue();
            return false;
        } catch (Exception e) {
            return true;
        }
    }

    private void encryptMedia(InputStream ins, OutputWriter out) throws FileNotFoundException, IOException {
        // copy temp file to current directory by input stream. - Don't need full path.
        FileOutputStream tout=new FileOutputStream(TEMP_IN_FILE);
        StreamUtil.copy(ins, tout);
        tout.close();

        // Encrypt source file to temp path by external application.
        String command = createMP4BoxCommand(MP4BoxCommand.RUN);
        Process p = Runtime.getRuntime().exec(command);

        // Discard ErrorStream, InputStream of external application.
        while ( isAlive(p) == true ) {
            InputStream is;
            if ( p.getErrorStream().available()>0 ) {
                is = p.getErrorStream();
            }
            else if ( p.getInputStream().available()>0 ) {
                is = p.getInputStream();
            }
            else {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {}
                finally {
                    continue;
                }
            }

            BufferedReader r=new BufferedReader(new InputStreamReader(is));
            String line = null;

            while ((line = r.readLine()) != null) {
                if (LOGGING) log(line);
            }
        }

        p.destroy();

        // Open cipher file to inputstream
        FileInputStream tins = new FileInputStream(TEMP_OUT_FILE);

        // Copy cipher file by to output stream by StreamUtil.copy()
        StreamUtil.copy(tins, out.getOutputStream());
        if (LOGGING) log("DRM Encrypted the file="+out.getName());

        // close and remove
        tins.close();
        File f=new File(TEMP_IN_FILE);
        if ( f.exists() ) {
            f.delete();
        }
        f = new File(TEMP_OUT_FILE);
        if ( f.exists() ) {
            f.delete();
        }
    }

    private String createMP4BoxCommand(MP4BoxCommand type) {
        //boolean isMac = (System.getProperty("os.name").toLowerCase().indexOf("mac") >= 0);
        String command = null;

        if (type == MP4BoxCommand.RUN) {
            command = String.format("MP4Box -crypt %s %s -out %s", TEMP_XML_FILE, TEMP_IN_FILE, TEMP_OUT_FILE);
        }
        else if (type == MP4BoxCommand.VERSION) {
            command = "MP4Box -version";
        }
        return command;
    }


    public String encryptPackage(File src, File dst, EntryFilter entryFilter) throws IOException, KeyczarException {
        InputReader in=null;
        OutputWriter out=null;
        Boolean isDrm = false;
        String itemString="";

        try {
            // Check is MP4Box is installed on current OS by running process
            String command = createMP4BoxCommand(MP4BoxCommand.VERSION);
            Runtime.getRuntime().exec(command);
            isDrm = true;
        }
        catch(Exception e) {
            e.printStackTrace();
            return ( "MP4Box is not correctly installed on current OS ");
        }

        if( isDrm ) {
            File f = new File(TEMP_XML_FILE);
            if ( f.exists() ) {
                f.delete();
            }
            FileOutputStream fs = new FileOutputStream(f);
            fs.write(xmlContent.getBytes());
            fs.close();
        }

        if ( src.isDirectory() ) {
            if( LOGGING ) log("Input is folder = "+src.getPath());
            in = new FolderInputReader(src);
        } else if ( src.getName().toLowerCase().endsWith("zip") ) {
            if( LOGGING ) log("Input is zip file = "+src.getPath());
            in = new ZipInputReader(src);
        }
        if ( in == null ) {
            return ("Input is not folder and zip file! "+src.getName());
        }

        // Check the source package is not encrypted.
        for (String path : in.listFiles()) {
            String name = in.getEntryName(path);
            if (name.equals(ENCRYPTION_INFO_FILE)) {
                return ("It is already encrypted package!");
            }
        }

        if ( dst.getName().toLowerCase().endsWith("zip") ) {
            if( LOGGING ) log("Encrypting from '" + src.getName() + "' to '" + dst.getName() + "' ZIP");

            File f = new File(dst.getPath());
            if ( f.exists() ) {
                f.delete();
            }

            out = new ZipWriter(dst);
        } else {
            if( LOGGING ) log("Encrypting from '" + src.getName() + "' to '" + dst.getName() + "' Folder");

            if ( dst.exists() && dst.isDirectory() ) {
                deleteFolder(dst);
            }
            out = new FolderWriter(dst.getPath());
        }

        Crypter crypter = new Crypter(keyczarReader);
        ProviderSpecificDataJSON psd = new ProviderSpecificDataJSON();
        psd.v = 1;
        psd.f = new ArrayList<>();
        try {
            for (String path : in.listFiles()) {
                String name = in.getEntryName(path);
                if (name.contains("..")) {
                    throw new IOException("Relative paths not allowed"); // For security purposes
                } else if ( name.contains("__MACOSX") || name.contains(".DS_Store") ) {
                    continue;
                }

                out.setEntry(name);
                if (!out.isDirectory()) {
                    InputStream ins = in.openFile(path);
                    if (LOGGING) log("  - processing file '" + name + "' " + out.getCompressedSize() + "->" + out.getSize());

                    if (entryFilter.shouldEncryptEntry(out)) {
                        itemString += ("Encrypted item = "+name+"\n");
                        if ( isDrm && (name.toLowerCase().endsWith("m4a") || name.toLowerCase().endsWith("mp4")) ) {
                            encryptMedia(ins, out);
                        } else {
                            ByteArrayOutputStream data = new ByteArrayOutputStream();
                            StreamUtil.copy(ins, data);
                            out.write(crypter.encrypt(data.toByteArray()));
                            psd.f.add(out.getName());
                        }
                    } else {
                        if ( name.contains("_info.json")) {
                            if ( updateItemInfoJson(ins, out.getOutputStream(), true) == false ) {
                                log("Update _info.json is fail!");
                            }
                        } else {
                            StreamUtil.copy(ins, out.getOutputStream());
                        }
                    }
                    CloseUtil.closeSilently(ins);
                } else {
                    if (LOGGING) log("  - processing directory '" + name);
                }
                out.closeEntry();
            }
            out.setEntry(ENCRYPTION_INFO_FILE);
            EncryptionInfoJSON eij = new EncryptionInfoJSON();
            eij.provider = providerid;
            ;//"nex.bep";
            eij.psd = new Gson().toJson(psd);
            out.write(new Gson().toJson(eij).getBytes());
            out.closeEntry();
            out.finish();
        } finally {
            in.close();
            CloseUtil.closeSilently(out.getOutputStream());

            // delete temp file
            File f=new File(TEMP_IN_FILE);
            if ( f.exists() ) {
                f.delete();
            }
            f = new File(TEMP_OUT_FILE);
            if ( f.exists() ) {
                f.delete();
            }
            f = new File(TEMP_XML_FILE);
            if ( f.exists() ) {
                f.delete();
            }
        }
        return ("Encrypting Success");
    }

    public String decryptPackage(File src, File dst) throws IOException, KeyczarException {
        InputReader in=null;
        OutputWriter out=null;
        String itemString="";

        if ( src.isDirectory() ) {
            if( LOGGING ) log("This is folder = "+src.getPath());
            in = new FolderInputReader(src);
        } else if ( src.getName().toLowerCase().endsWith("zip") ) {
            if( LOGGING ) log("This is zip = "+src.getPath());
            in = new ZipInputReader(src);
        }
        if ( in == null ) {
            return ("This is not folder and zip file! "+src.getName());
        }

        if ( dst.getName().toLowerCase().endsWith("zip") ) {
            if( LOGGING ) log("Decrypting from '" + src.getName() + "' to '" + dst.getName() + "' ZIP");

            File f = new File(dst.getPath());
            if ( f.exists() ) {
                f.delete();
            }

            out = new ZipWriter(dst);
        } else {
            if( LOGGING ) log("Decrypting from '" + src.getName() + "' to '" + dst.getName() + "' Folder");

            if ( dst.exists() && dst.isDirectory() ) {
                deleteFolder(dst);
            }
            out = new FolderWriter(dst.getPath());
        }

        EncryptionInfoJSON  encinfo = null;
        ProviderSpecificDataJSON psdata = null;
        try {
            for ( String path : in.listFiles() ) {
                String name = in.getEntryName(path);
                if ( name.equals(ENCRYPTION_INFO_FILE) ) {
                    InputStream ins = in.openFile(path);
                    byte[] buf = new byte[(int)in.getSize(path)];
                    ByteArrayInputStream data = new ByteArrayInputStream(buf);
                    ins.read(buf);

                    // Parse json format string and make list.
                    Gson gson = new Gson();
                    encinfo = gson.fromJson(new String(buf), EncryptionInfoJSON.class);
                    psdata = new Gson().fromJson(encinfo.psd, ProviderSpecificDataJSON.class);
                    //if( LOGGING) log("Encryption Info : " + encinfo.provider+"\n"+encinfo.psd);
                    ins.close();
                    break;
                }
            }
        } finally {

        }

        if ( psdata == null ) {
            throw new IOException("This is not encrypted asset. There is no encryption info file.");
        }

        Crypter derypter = new Crypter(keyczarReader);
        try {
            for ( String path : in.listFiles() ) {
                String name = in.getEntryName(path);

                if ( name.equals(ENCRYPTION_INFO_FILE)) {
                    continue;
                } else if ( name.contains("..") ) {
                    throw new IOException("Relative paths not allowed"); // For security purposes
                } else if ( name.contains("__MACOSX") || name.contains(".DS_Store") ) {
                    continue;
                }

                out.setEntry(name);

                if (!out.isDirectory()) {
                    InputStream ins = in.openFile(path);

                    if ( psdata.f.contains(name) ) {
                        itemString += ("Decrypted item = "+name+"\n");
                        ByteArrayOutputStream encdata = new ByteArrayOutputStream();
                        StreamUtil.copy(ins, encdata);
                        out.write(derypter.decrypt(encdata.toByteArray()));
                    } else {
                        if ( name.contains("_info.json") ) {
                            if ( updateItemInfoJson(ins, out.getOutputStream(), false) == false ) {
                                log("Update _info.json fail!");
                            }
                        } else {
                            StreamUtil.copy(ins, out.getOutputStream());
                        }
                    }
                }
                out.closeEntry();
            }
            out.finish();
        }finally {
            in.close();
            CloseUtil.closeSilently(out.getOutputStream());
        }

        return ("Decrypting Success");
    }

    private class ItemInfoJSON {
        public String id;
        public String type;
        public String filename;
        public String thumbnail;
        public String icon;
        public String sampleText;
        public boolean hidden;
        public boolean encrypted;
        public Map<String,String> label;
        public List<String> mergePaths;
    }

    private boolean updateItemInfoJson(InputStream in, OutputStream out, boolean bEncrypted) {

        Gson gson = new Gson();
        JsonWriter writer=null;
        try {
            writer = new JsonWriter(new OutputStreamWriter(out, "UTF-8"));
            writer.setIndent(" ");
        } catch (UnsupportedEncodingException e) {
            if (LOGGING) log("UnsupportedEncodingException occurred. "+e.getLocalizedMessage());
            return false;
        }

        ItemInfoJSON info = gson.fromJson(new InputStreamReader(in), ItemInfoJSON.class);

        if ( info.encrypted == bEncrypted && info.encrypted == true ) {
            log("This asset item is already encrypted. It's invalid operation!");
        } else {
            info.encrypted = bEncrypted;
        }

        // write json file
        gson.toJson(info, ItemInfoJSON.class, writer);

        try {
            writer.flush();
        } catch (IOException e) {
            if (LOGGING) log("IOException occurred. "+e.getLocalizedMessage());
            return false;
        }

        return true;
    }

    public static int[] getProviderID(File src) throws IOException {
        InputReader in=null;
        int[] providerID=null;

        if ( src.isDirectory() ) {
            if( LOGGING ) log("This is folder = "+src.getPath());
            in = new FolderInputReader(src);
        } else if ( src.getName().toLowerCase().endsWith("zip") ) {
            if( LOGGING ) log("This is zip = "+src.getPath());
            in = new ZipInputReader(src);
        }
        if ( in == null ) {
            if( LOGGING ) log("This is not folder and zip file! "+src.getName());
            return providerID;
        }

        EncryptionInfoJSON  encinfo = null;
        ProviderSpecificDataJSON psdata = null;
        try {
            for ( String path : in.listFiles() ) {
                String name = in.getEntryName(path);
                if ( name.equals(ENCRYPTION_INFO_FILE) ) {
                    InputStream ins = in.openFile(path);
                    byte[] buf = new byte[(int)in.getSize(path)];
                    ByteArrayInputStream data = new ByteArrayInputStream(buf);
                    ins.read(buf);

                    // Parse json format string and make list.
                    Gson gson = new Gson();
                    encinfo = gson.fromJson(new String(buf), EncryptionInfoJSON.class);
                    psdata = new Gson().fromJson(encinfo.psd, ProviderSpecificDataJSON.class);
                    if( LOGGING) log("Encryption Info : " + encinfo.provider+"\n"+encinfo.psd);

                    // read provider ID
                    providerID = new int[encinfo.provider.length()];
                    for ( int i=0; i<encinfo.provider.length() ; i++) {
                        providerID[i] = (int)encinfo.provider.charAt(i)^90;
                    }
                    ins.close();
                }
            }
        } finally {
            return providerID;
        }
    }

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

    private String getDecryptedString(String in) {
        int len = in.length();
        int v = parsehex(in,0,2) ^ 32;
        byte[] buf = new byte[(len-2)/2];
        for( int i=0; i<buf.length; i++ ) {
            byte b = (byte) parsehex(in,2+i*2,4+i*2);
            b = (byte) ((((b>>4)&0x0F) | ((b<<4)&0xF0))^v);
            buf[i] = b;
        }
        return new String(buf);
    }

    public String toHex(String arg) {
        return String.format("%x", new BigInteger(1, arg.getBytes(/*YOUR_CHARSET?*/)));
    }

    public String[] getEInfo() {
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

            //log("KeyID : ["+hmacValue+"/"+toHex(hmacValue)+"]");
            //log("Key : ["+keyStr+"/"+toHex(keyStr)+"]");
        }

        //log("System ID : ["+keyStr+"/"+toHex(keyStr)+"]");

        for (i=0 ; i<j ; i++) {
            if (LOGGING) log("Index=" + i + ", Valu=" + array[i]);
        }

        return array;
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
        String ks = hexakeys[index];
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
}
