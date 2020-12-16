package com.nexstreaming.assetpackage;

import com.nexstreaming.app.general.nexasset.assetpackage.packager.BasicEncryptionProviderKeySet;
import com.nexstreaming.app.general.nexasset.assetpackage.packager.PackageCrypter;
import com.nexstreaming.app.general.nexasset.assetpackage.packager.PackageKeyFileGenerator;

import org.keyczar.exceptions.KeyczarException;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import javax.script.Bindings;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;
import javax.script.SimpleBindings;

public class NexAssetPackageCryper {
    private final static int TOOL_VERSION_Major = 1;
    private final static int TOOL_VERSION_Minor = 0;
    private final static int TOOL_VERSION_Patch = 4;

    private static String[] filters = {".json",".xml",".glsl"};
    private static String[] exception_filters={".json",".mp3",".aac", ".ttf", "renderitem.xml"};

    private static BasicEncryptionProviderKeySet keySet = new BasicEncryptionProviderKeySet();

    public static void main( String[] args ) {

//        if( args.length==1 ) {
//            try {
//                System.out.println(engine.eval(args[0]));
//            } catch (ScriptException e) {
//                e.printStackTrace();
//            }
//            return;
//        }

/* For android sutdio debugging
        if ( args.length < 4) {
            BasicEncryptionProviderKeySet keySet = new BasicEncryptionProviderKeySet();
            if ( keySet.getProviderID("NEX") == null ) {
                System.out.println("Invalid customer name! ");
                return;
            }
            PackageCrypter crypter = new PackageCrypter(keySet.getProviderID("NEX"), keySet.getKeyArray(keySet.getProviderID("NEX")));
            //String[] key=crypter.getEInfo();
            //System.out.println("Key = "+key);

            //System.out.println("Encrypting: " + args[2] + " to " + args[3] + " for " + args[1]);
            encrypt(crypter, "C:\\Working\\Asset\\Clean\\GirlsDay_L2_V1.0.1", "C:\\Temp\\Temp\\GirlsDay_Enc.zip", null);
            try {
                crypter.decryptPackage(new File("C:\\Working\\Asset\\Enc\\NEX_GirlsDay_L2_V1.0.1.zip"), new File("C:\\Working\\Asset\\Dec\\temp.zip"));
            } catch (IOException e) {
            } catch (KeyczarException e) {
            }
            //encrypt(crypter, "c:\\TempCrypter\\bokeh.zip", "c:\\TempCrypter\\bokeh_enc.zip", null);
            return;
        }
*/

        System.out.println("Tool Version : "+TOOL_VERSION_Major+"."+TOOL_VERSION_Minor+"."+TOOL_VERSION_Patch);
        if( args.length<2 || args.length>5 || (!args[0].equals("key") && !args[0].equals("drm") && args.length == 2)) {
            System.out.println("Usage1:\n\t NexAssetPackageCrypter key <keyczar key folder>");
            System.out.println("Example:\n\t NexAssetPackageCrypter key NEX");
            System.out.println("Usage2:\n\t NexAssetPackageCrypter <Mode:enc/dec> <customer name> <input_zip/input_folder> <output_zip> [include_condition]\n");
            System.out.println("Example:\n\t NexAssetPackageCrypter enc NEX clean.zip cipher.zip");
            System.out.println("Example:\n\t NexAssetPackageCrypter enc NEX ./clean cipher.zip");
            System.out.println("Example:\n\t NexAssetPackageCrypter dec NEX cipher.zip clean.zip");
            return;
        }

        if ( args[0].equals("key") ) {
            try {
                PackageKeyFileGenerator.generateKeyInfoFile(args[1]);
            } catch (IOException e) {
                e.printStackTrace();
            }
            return;
        } else if ( args[0].equals("enc") ) {
                encryptPackage(args[1], new File(args[2]), new File(args[3]), (args.length>4?args[5]:null));
        } else if ( args[0].equals("dec") ) {
            if ( args.length < 4 || !args[3].equals("robin.lee@nexstreaming.com") ) {
                System.out.println("Invalid operation!");
                return;
            }
            decryptPackage(new File(args[1]), new File(args[2]));
        } else {
            System.out.println("Invalid operation!");
        }
    }

    private static void encryptPackage(String key, File src, File dst, String condition) {
        File tmp=null;
        if ( keySet.getProviderID(key) == null ) {
            System.out.println("Invalid customer name! "+key);
            return;
        }

        System.out.println("Encrypting: " + src.getPath() + " to " + dst.getPath() + " for " + key);

        int[] providerID=getProviderIDFromPackage(src, keySet);
        if ( providerID != null ) {
            // decrypt to temporary.
            tmp= new File("./temporary");
            PackageCrypter crypter = new PackageCrypter(providerID, keySet.getKeyArray(providerID));
            try {
                crypter.decryptPackage(src, tmp);
            } catch (IOException e) {
                e.printStackTrace();
            } catch (KeyczarException e) {
                e.printStackTrace();
            }
            src = tmp;
        }

        PackageCrypter crypter = new PackageCrypter(keySet.getProviderID(key), keySet.getKeyArray(keySet.getProviderID(key)));

        encrypt(crypter, src, dst, condition);

        if ( tmp != null ) {
            deleteAllFiles(tmp);
        }

    }

    private static void decryptPackage(File src, File  dst) {
        int[] providerID=getProviderIDFromPackage(src, keySet);
        if ( providerID == null ) {
            System.out.println("No encryption information!");
            return;
        }

        System.out.println("Decrypting: " + src.getPath() + " to " + dst.getPath());

        PackageCrypter crypter = new PackageCrypter(providerID, keySet.getKeyArray(providerID));

        try {
            System.out.println(crypter.decryptPackage(src, dst));
        } catch (IOException e) {
            e.printStackTrace();
        } catch (KeyczarException e) {
            e.printStackTrace();
        }

        return;
    }

    private static void encrypt(PackageCrypter crypter, File test_in, File test_out, final String condition) {
        ScriptEngineManager mgr = new ScriptEngineManager();
        final ScriptEngine engine = mgr.getEngineByName("JavaScript");

        try {
            engine.eval("           String.prototype.startsWith = function(str) {\n" +
                    "               return (this.indexOf(str) === 0);\n" +
                    "           }\n" +
                    "\n" +
                    "           String.prototype.endsWith = function(str) {\n" +
                    "               var lastIndex = this.lastIndexOf(str);\n" +
                    "               return (lastIndex != -1) && (lastIndex + str.length == this.length);\n" +
                    "           }");
        } catch (ScriptException e) {
            e.printStackTrace();
        }

        try {
            String ret = crypter.encryptPackage(test_in, test_out, new PackageCrypter.EntryFilter() {
                @Override
                public boolean shouldEncryptEntry(PackageCrypter.OutputWriter entry) {

                    if( condition==null ) {
                        for( String end : filters ) {
                            //if( entry.getName().endsWith(end) ){
                                for ( String except : exception_filters ) {
                                    if ( entry.getName().endsWith(except) == true ) {
                                        System.out.println("[ + Exclude ] " + entry.getName());
                                        return false;
                                    }
                                }
                            //}
                        }
                        //return entry.getSize() < 12 * 1024;
                        System.out.println("[ - INCLUDE ] " + entry.getName());
                        return true;
                    }
//                    StringBuilder sb = new StringBuilder();
//                    sb.append("path=\"").append(entry.getName()).append("\";");
//                    sb.append("size=").append(entry.getSize()).append(";");
//                    sb.append("csize=").append(entry.getCompressedSize()).append(";");
//                    sb.append(condition);
                    Object result = null;
                    Map<String,Object> params = new HashMap<String, Object>();
                    int dotidx = entry.getName().indexOf('.');
                    String ext = "";
                    if( dotidx>0 ) {
                        ext = entry.getName().substring(dotidx+1).toLowerCase();
                    }
                    params.put("path",entry.getName());
                    params.put("ext",ext);
                    params.put("size",entry.getSize());
                    params.put("csize",entry.getCompressedSize());
                    Bindings bindings = new SimpleBindings(params);
                    try {
                        result = engine.eval(condition,bindings);
                    } catch (ScriptException e) {
                        System.out.println("Script execution failed for: " + entry.getName());
                        e.printStackTrace();
                    }
                    if( result == Boolean.TRUE ) {
                        System.out.println("[ + INCLUDE ] " + entry.getName());
                        return true;
                    } else {
                        System.out.println("[ - Exclude ] " + entry.getName());
                        return false;
                    }
                }
            });
            System.out.println("Encrypt result = "+ret);
        } catch (IOException e) {
            e.printStackTrace();
        } catch (KeyczarException e) {
            e.printStackTrace();
        }
    }

    public static int[] getProviderIDFromPackage(File src, BasicEncryptionProviderKeySet keySet) {
        int[] provider=null;
        try {
            provider = PackageCrypter.getProviderID(src);
            for ( Map.Entry<int[], String[]> entry : keySet.getpKeyMap().entrySet() ) {
                if ( compareIntArrays(provider, entry.getKey()) ) {
                    provider = entry.getKey();
                    return provider;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return null;
    }

    public static boolean compareIntArrays(int[] a, int[] b) {
        boolean check = true;
        if (a!= null && b!= null){
            if (a.length != b.length){
                check= false;
            }else
                for (int i = 0; i < b.length; i++) {
                    if (b[i] != a[i]) {
                        check= false;
                    }
                }
        }else{
            check= false;
        }
        return check;
    }

    public static void deleteAllFiles(File root){
        File[] fList = root.listFiles();

        if(fList.length >0) {
            for (int i = 0; i < fList.length; i++) {
                if(fList[i].isFile()) {
                    fList[i].delete();
                } else {
                    deleteAllFiles(new File(fList[i].getPath()));
                } fList[i].delete();
            }
            root.delete();
        }
        return;
    }
}
