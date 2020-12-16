import kedlparser
from zipfile import ZipFile
import argparse
import os
import hmac
import hashlib
import json
from keyczar import keyczar
import copy
import time
import shutil
from datetime import datetime
import subprocess
import re

def common_prefix(a,b):
    if a==b or b.startswith(a):
        return a
    if a.startswith(b):
        return b
    return a[:[x[0]==x[1] for x in zip(a,b)].index(False)]

def kedltool(cmd,input=[],
                      output=os.path.join(os.getcwd(),"output"),
                      resources=None,
                      keys=None,
                      autoclean=False,
                      sdkroot=None):
    global search_path
    global key_path
    global file_digest_cache
    if resources is None:
        search_path = os.path.dirname(input[0])
    else:
        search_path = resources
    key_path = keys
    file_digest_cache = {}

    if cmd=="package":
        package(input, output)
    elif cmd=="dep":
        dep(input)
    elif cmd=="abbr":
        abbr(input)
    elif cmd=="summary":
        summary(input)
    elif cmd=="index":
        index(input)
    elif cmd=="strip":
        strip(input)
    elif cmd=="test":
        test(input, output, autoclean, sdkroot)

def kedlcmd():
    global search_path
    global key_path
    global file_digest_cache
    global outputfolder
    file_digest_cache = {}
    parser = argparse.ArgumentParser(
                description="Tools for working with KEDL files")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--package", "-p",
                        action="store_const",
                        const="package",
                        dest="cmd",
                        help="Package the specified XML theme and " +
                             "effect files, along with their dependencies.")
    group.add_argument("--signfile", "-e",
                        action="store_const",
                        const="signfile",
                        dest="cmd",
                        help="Sign the specified file and write the result to stdout.")
    group.add_argument("--fontsummary", "-n",
                        action="store_const",
                        const="fontsummary",
                        dest="cmd",
                        help="Show the summary of the specified font file")
    group.add_argument("--fontpackage", "-f",
                        action="store_const",
                        const="fontpackage",
                        dest="cmd",
                        help="Package the specified font file")
    group.add_argument("--dependencies", "-d",
                       action="store_const",
                       const="dep",
                       dest="cmd",
                       help="List dependencies for the specified XML theme " +
                            "and effect files.")
    group.add_argument("--test-on-device","-t",
                       action="store_const",
                       const="test",
                       dest="cmd",
                       help="Package and test the specified XML files on the device")
#    group.add_argument("--abbr", "-a",
#                       action="store_const",
#                       const="abbr",
#                       dest="cmd")
    group.add_argument("--summary", "-u",
                       action="store_const",
                       const="summary",
                       dest="cmd",
                       help="Display the JSON summary data for the " +
                            "specified XML themes and effects.")
    group.add_argument("--strip", "-s",
                       action="store_const",
                       const="strip",
                       dest="cmd",
                       help="Strip unnecessary data from specified " +
                            "XML files and output them to the console.")
    group.add_argument("--index", "-i",
                       action="store_const",
                       const="index",
                       dest="cmd",
                       help="Build an index for any .kmt files in the" +
                       " specified path.")
    parser.add_argument("--sign", "-k",
                       dest="keypath",
                        default=None,
                        help="Sign the package with keys at the specified " +
                        "path.  Only useful with --package")
    #parser.add_argument("path", default=os.getcwd(), nargs="?")
    parser.add_argument("inpaths", metavar="FILE", nargs="+",
                        help="Files to operate on")
    parser.add_argument("--output","-o",
                        dest="outpath",
                        default=os.path.join(os.getcwd(),"output"),
                        help="Folder in which to place output " +
                        "(will be created if it does not exist)")
    parser.add_argument("--res","-r",
                        dest="respath",
                        default=None,
                        help="Specifies an alternate path where resources " +
                        "can be found (if they are not in the same folder " +
                        "with the input XML files)")

    parser.add_argument("--outputfolder","-l",
                        dest="outputfolder",
                        default=os.path.join(os.getcwd(),"output/km"),
                        help="Specifies output folder for xml and other files")
                        
    # output
    args = parser.parse_args()
    
    outputfolder = args.outputfolder
    if args.respath is None:
        search_path = os.path.dirname(args.inpaths[0])
    else:
        search_path = args.respath
    
    key_path = args.keypath
    
    if args.cmd=="package":
        package(args.inpaths, args.outpath)
    elif args.cmd=="dep":
        dep(args.inpaths)
    elif args.cmd=="abbr":
        abbr(args.inpaths)
    elif args.cmd=="summary":
        summary(args.inpaths)
    elif args.cmd=="fontsummary":
        fontsummary(args.inpaths)
    elif args.cmd=="fontpackage":
        fontpackage(args.inpaths, args.outpath)
    elif args.cmd=="index":
        index(args.inpaths)
    elif args.cmd=="strip":
        strip(args.inpaths)
    elif args.cmd=="test":
        test(args.inpaths, args.outpath)
    elif args.cmd=="signfile":
        signfile(args.inpaths)

def signfile(inpaths,testsign=False):

    for f in inpaths:
        sign_one_file(f,testsign)

def sign_one_file(filename,testsign=False):
    with open(filename) as f:
        data = f.readlines()
    data = "".join(data)
    if key_path is not None:
        if testsign:
            s = keyczar.Signer.Read(os.path.join(key_path,"testsign"))
        else:
            s = keyczar.Signer.Read(os.path.join(key_path,"sign"))
        print data + "\n" + s.Sign(data.replace("\n","").replace("\r",""))
    else:
        print "KEY NEEDED"

def test(inpaths, outpath, autoclean=False, sdkroot=None):
    pushfiles = package(inpaths,outpath,testsign=True)
    if sdkroot is None:
        adbpath = "adb"
    else:
        adbpath = os.path.join(sdkroot,"platform-tools","adb")
    subprocess.check_output([adbpath,"shell","am",
                             "force-stop",
                             "com.nexstreaming.app.kinemasterfree"])
    for f in pushfiles:
        dst = f
        if dst.startswith(outpath):
            dst = dst[len(outpath):]
        if dst.startswith("/ext/"):
            dst = "/externals/" + dst[5:]
        else:
            rmv =  "/sdcard/KineMaster/Plugins" + re.sub("\.[0-9]*\.kmt",".*.kmt",dst)
            subprocess.check_output([adbpath,"shell","rm",rmv])
        dst = "/sdcard/KineMaster/Plugins" + dst
        subprocess.check_output([adbpath,"push",f,dst])
    if autoclean:
        for f in pushfiles:
            os.remove(f)
    android_pkg = ("com.nexstreaming.app.kinemasterfree" + "/" +
                    "com.nextreaming.nexeditorui.MainActivity")
    subprocess.check_output([adbpath,"shell","am",
                             "start","-n",
                             android_pkg])

def abbr(inpaths):
    for path in inpaths:
        kf = kedlparser.KEDLFile(path)
        if len(inpaths)>1:
            print( "--------- " + path + " ---------" )
        print(kf.minixml)

def index(inpaths):
    for f in inpaths:
        index_dir(f)

def index_dir(path):
    if not os.path.isdir(path):
        print("Not a folder: " + path)
        return
    
    print("Indexing: " + path)

    jsonfiles = [f for f in os.listdir(path)
                if f.endswith(".json")
                and os.path.isfile(os.path.join(path,f)) ]

    kmtfiles = [f for f in os.listdir(path)
                if f.endswith(".kmt")
                and os.path.isfile(os.path.join(path,f)) ]

    fontlists = {}
    for f in jsonfiles:
        summary = json.load(open(os.path.join(path,f)))
        if summary.has_key("package_type") and summary["package_type"]=="fontlist":
		    fontlists[summary[u"id"]] = summary

    all_effects = []
    all_themes = []
    extpath = os.path.join(path,"ext");
    for f in kmtfiles:
        zfpath = os.path.join(path,f);
        zfsize = os.path.getsize(zfpath)
        with ZipFile(zfpath, 'r') as zf:
            summary = json.load(zf.open("summary.json"))
            for e in summary["effects"]:
                e["package_size"] = zfsize
                all_effects.append(e)
                dsize = 0
                for d in e["dependencies"]:
                    dsize += os.path.getsize(os.path.join(extpath,d))
                e["dependency_size"] = dsize
                dsize = 0
                for d in e["optional_dependencies"]:
                    dsize += os.path.getsize(os.path.join(extpath,d))
                e["optional_dependency_size"] = dsize
            for t in summary["themes"]:
                t["package_size"] = zfsize;
                all_themes.append(t)
                dsize = 0
                for d in t["dependencies"]:
                    dsize += os.path.getsize(os.path.join(extpath,d))
                t["dependency_size"] = dsize
                dsize = 0
                for d in t["optional_dependencies"]:
                    dsize += os.path.getsize(os.path.join(extpath,d))
                t["optional_dependency_size"] = dsize

    effects = []
    themes = []

    idver = {}
    for item in all_themes:
        id = item["id"]
        ver = item["version"]
        if id in idver:
            idver[id] = max(idver[id],ver)
        else:
            idver[id]=ver
    for item in all_themes:
        id = item["id"]
        ver = item["version"]
        if idver[id]==ver:
            themes.append(item)

    idver = {}
    for item in all_effects:
        id = item["id"]
        ver = item["version"]
        if id in idver:
            idver[id] = max(idver[id],ver)
        else:
            idver[id]=ver
    for item in all_effects:
        id = item["id"]
        ver = item["version"]
        if idver[id]==ver:
            effects.append(item)

    result = json.dumps({"themes":themes,"effects":effects,"fontlists":fontlists},indent=4)

    while True:
        index_path = os.path.join(path,"index." + utcstamp() + ".json")
        if not os.path.isfile(index_path):
            break

    with open(index_path,"w") as f:
        f.write(result)
    #print(result)

def strip(inpaths):
    for path in inpaths:
        kf = kedlparser.KEDLFile(path)
        if len(inpaths)>1:
            print( "--------- " + path + " ---------" )
        print(kf.xml)

def calc_dependencies(kedlfiles):
    internal_files = set()
    external_files = set()
    opt_external_files = set()
    for kf in kedlfiles:
        internal_files |= kf.internal_files
        external_files |= kf.external_files
        opt_external_files |= kf.opt_external_files
    external_files -= internal_files
    opt_external_files -= internal_files
    opt_external_files -= external_files
    return (internal_files,external_files,opt_external_files)

def summary(inpaths):
    kedlfiles = [kedlparser.KEDLFile(path) for path in inpaths]
    print(make_summary(kedlfiles))

def fontpackage(inpaths,outpath):

    kedlfiles = [kedlparser.KMFontList(path) for path in inpaths]
    (internal_files,external_files,opt_external_files) = calc_dependencies(kedlfiles)

    rename_rule = {}
    extpath = os.path.join(outpath,"ext")

    if not os.path.isdir(outpath):
        os.mkdir(outpath)
    if not os.path.isdir(extpath):
        os.mkdir(extpath)

    for f in external_files:
        digest = digest_file(os.path.join(search_path,f))
        (base,ext) = os.path.splitext(f.lower())
        rename_rule[f] = filtername(base) + "." + digest + ext
    for f in opt_external_files:
        digest = digest_file(os.path.join(search_path,f))
        (base,ext) = os.path.splitext(f.lower())
        rename_rule[f] = filtername(base) + "." + digest + ext

    for f in external_files:
        if f in rename_rule:
            fname = rename_rule[f]
        else:
            fname = f
        dst = os.path.join(extpath,fname)
        if not os.path.isfile(dst):
            shutil.copy(os.path.join(search_path,f),dst)

    for f in opt_external_files:
        if f in rename_rule:
            fname = rename_rule[f]
        else:
            fname = f
        dst = os.path.join(extpath,fname)
        if not os.path.isfile(dst):
            shutil.copy(os.path.join(search_path,f),dst)

    kedlfiles = [kedlparser.KMFontList(path,rename_rule) for path in inpaths]
    (internal_files,external_files,opt_external_files) = calc_dependencies(kedlfiles)

    for fontlist in kedlfiles:
        (base,ext) = os.path.splitext(os.path.split(fontlist.path)[1].lower())
        with open(  os.path.join(outpath,"f_" + filtername(base) + "." + fontlist.version + ".json"),"w") as f:
            f.write(json.dumps(fontlist.summary,indent=4))

def fontsummary(inpaths):

    kedlfiles = [kedlparser.KMFontList(path) for path in inpaths]
    (internal_files,external_files,opt_external_files) = calc_dependencies(kedlfiles)

    rename_rule = {}

    for f in external_files:
        digest = digest_file(os.path.join(search_path,f))
        (base,ext) = os.path.splitext(f.lower())
        rename_rule[f] = filtername(base) + "." + digest + ext
    for f in opt_external_files:
        digest = digest_file(os.path.join(search_path,f))
        (base,ext) = os.path.splitext(f.lower())
        rename_rule[f] = filtername(base) + "." + digest + ext

    for path in inpaths:
        print(json.dumps(kedlparser.KMFontList(path,rename_rule).summary,indent=4))


def filtername(s):
    return ("".join([c for c in s.lower().replace(" ","_")
       if (c >= "a" and c <= "z") or (c >= "0" and c <= "0" ) or c in "_-."]))

def utcstamp():
    return tstamp(datetime.utcnow())

def tstamp(t):
    return str.format("{:04}{:02}{:02}{:02}{:02}{:02}",
                      t.year,t.month,t.day,t.hour,t.minute,t.second)

def make_summary(kedlfiles,package_file=None,rename_rule=None):
    global search_path
    (internal_files,external_files,opt_external_files) = calc_dependencies(kedlfiles)
    effects = []
    themes = []
    for f in kedlfiles:
        s = copy.deepcopy(f.summary)
#        if rename_rule is not None:
#            s["dependencies"] = [rename_rule[ef] for ef in external_files]
#        else:
        s["dependencies"] = list(external_files)
        s["optional_dependencies"] = list(opt_external_files)
        if package_file is not None:
            s["package"]=package_file
        if f.type == "theme":
            themes.append(s)
        elif f.type == "effect" and not f.themeonly:
            effects.append(s)
    return json.dumps({"themes":themes,"effects":effects},indent=4)

def dep(inpaths):
    global search_path
    kedlfiles = [kedlparser.KEDLFile(path) for path in inpaths]
    (internal_files,external_files,opt_external_files) = calc_dependencies(kedlfiles)
    for g in (("Internal Dependencies:",internal_files),
              ("External Dependencies:",external_files),
              ("Optional External Dependencies:",opt_external_files)):
        print("")
        print(g[0])
        totalsize = 0
        for f in g[1]:
            p = os.path.join(search_path,f)
            if os.path.isfile(p):
                state = "OK"
                size = os.path.getsize(p)
                sizestr = str(size)
                totalsize += size
            else:
                state = "MISSING"
                sizestr = ""
            print("    " + f.ljust(30)
                  + " " + state.ljust(8)
                  + " " + sizestr.rjust(10) )
        print("    TOTAL SIZE: " + str(totalsize))
    print("")

def package(inpaths, outpath,testsign=False):
    global search_path
    
    print("Packaging...")
    
    pushfiles = []
    
    kedlfiles = [kedlparser.KEDLFile(path) for path in inpaths]
    (internal_files,external_files,opt_external_files) = calc_dependencies(kedlfiles)

    internal_files = list(internal_files)
    internal_files.sort()
    external_files = list(external_files)
    external_files.sort()
    opt_external_files = list(opt_external_files)
    opt_external_files.sort()
    
    rename_rule = dict(zip(internal_files,
                           ["r" + str(x)
                            for x in
                            range(len(internal_files))]))
    
    for f in external_files:
        digest = digest_file(os.path.join(search_path,f))
        (base,ext) = os.path.splitext(f.lower())
        rename_rule[f] = filtername(base) + "." + digest + ext
    for f in opt_external_files:
        digest = digest_file(os.path.join(search_path,f))
        (base,ext) = os.path.splitext(f.lower())
        rename_rule[f] = filtername(base) + "." + digest + ext

    kedlfiles = [kedlparser.KEDLFile(path,rename_rule) for path in inpaths]
    
    first_theme_id = None
    first_effect_id = None
    common_effext_prefix = None
    first_theme_version = None
    first_effect_version = None
    latest_version = None
    num_items = 0
    for f in kedlfiles:
        if f.type == "theme":
            num_items += 1
            if first_theme_id is None:
                first_theme_id=f.id
                first_theme_version=f.version
                common_effext_prefix=f.id
                latest_version=f.version
            elif common_effext_prefix is not None:
                if f.id.find(".")>=0:
                    common_effext_prefix = common_prefix(common_effext_prefix,f.id)
                    if f.version > latest_version:
                        latest_version=f.version

        elif f.type == "effect" and not f.themeonly:
            num_items += 1
            if first_effect_id is None:
                first_effect_id=f.id
                first_effect_version=f.version
                common_effext_prefix=f.id
                latest_version=f.version
            elif common_effext_prefix is not None:
                if f.id.find(".")>=0:
                    common_effext_prefix = common_prefix(common_effext_prefix,f.id)
                    if f.version > latest_version:
                        latest_version=f.version
                

    package_file="com.example." + str(int(round(time.time()))) + ".kmt"
    if num_items > 1:
        psep = ".package."
    else:
        psep = "."
    if (common_effext_prefix is not None
        and common_effext_prefix.find(".")>=0
        and len(common_effext_prefix)>2):
        if common_effext_prefix.endswith("."):
            common_effext_prefix = common_effext_prefix[:-1]
        package_file = (common_effext_prefix
                        + "."
                        + str(latest_version)
                        + ".kmt")
    elif first_theme_id is not None:
        package_file = (first_theme_id
                        + psep
                        + str(first_theme_version)
                        + ".kmt")
    elif first_effect_id is not None:
        package_file = (first_effect_id
                        + psep
                        + str(first_effect_version)
                        + ".kmt")

    summary_data = make_summary(kedlfiles,package_file,rename_rule)

    extpath = os.path.join(outpath,"ext")
    if not os.path.isdir(outpath):
        os.mkdir(outpath)
    if not os.path.isdir(extpath):
        os.mkdir(extpath)
    package_path = os.path.join(outpath,package_file)
    if os.path.isfile(package_path):
        print("Package already exists:  Please change ID or VERSION and try again")
        return
    pushfiles.append(package_path)
    with ZipFile(package_path, 'w') as zf:
        manifest = "KineMaster Manifest/0100\n"
        manifest += "summary.json=" + digest_str(summary_data) + "\n"
        zf.writestr("summary.json",summary_data)
        manifest += "[published-effects]\n"
        for f in kedlfiles:
            data = f.xml
            if key_path is not None:
                c = keyczar.Crypter.Read(os.path.join(key_path,"crypt"))
                data = c.Encrypt(data)
            zf.writestr(f.id + ".xml",data)
            manifest += f.id + "=" + digest_str(data) + "\n"
#            zip.writestr(f.id + ".xml",f.minixml)
#            manifest += f.id + "=" + digest_str(f.minixml) + "\n"
        manifest += "[internal-dependencies]\n"
        for f in internal_files:
            if f in rename_rule:
                rf = rename_rule[f]
            else:
                rf = f
            manifest += rf + "=" + digest_file(os.path.join(search_path,f)) + "\n"
            zf.write(os.path.join(search_path,f),"res/" + rf)
        manifest += "[external-dependencies]\n"
        for f in external_files:
            if f in rename_rule:
                fname = rename_rule[f]
            else:
                fname = f
            manifest += fname + "=" + digest_file(os.path.join(search_path,f)) + "\n"
        manifest += "[opt-external-dependencies]\n"
        for f in opt_external_files:
            if f in rename_rule:
                fname = rename_rule[f]
            else:
                fname = f
            manifest += fname + "=" + digest_file(os.path.join(search_path,f)) + "\n"

        zf.writestr("manifest.txt",manifest)

        if key_path is not None:
            if testsign:
                s = keyczar.Signer.Read(os.path.join(key_path,"testsign"))
            else:
                s = keyczar.Signer.Read(os.path.join(key_path,"sign"))
            zf.writestr("kinemaster.dsa",s.Sign(manifest))

    for f in external_files:
        if f in rename_rule:
            fname = rename_rule[f]
        else:
            fname = f
        dst = os.path.join(extpath,fname)
        if not os.path.isfile(dst):
            shutil.copy(os.path.join(search_path,f),dst)
        pushfiles.append(dst)
    for f in opt_external_files:
        if f in rename_rule:
            fname = rename_rule[f]
        else:
            fname = f
        dst = os.path.join(extpath,fname)
        if not os.path.isfile(dst):
            shutil.copy(os.path.join(search_path,f),dst)
        pushfiles.append(dst)

    print("Packaged as: " + outpath+ package_file)

#    with ZipFile(outpath+ "/" + package_file, 'r') as zfile:
##        folder_name = os.path.splitext(package_file)
##        folder_name = os.path.split(folder_name[0])
#        if not os.path.exists(outputfolder):
#            os.mkdir(outputfolder)
##        target = outputfolder + "/" + folder_name[1]
#        target = outputfolder + "/" + package_file
#        print("OUTPUT FOLDER " + target)
#        os.mkdir(target)
#        for name in zfile.namelist():
#            (dirname, filename) = os.path.split(name)
#            print "Decompressing " + filename + " on " +  target + "/" +  dirname
#            if not os.path.exists(target+ "/"+ dirname):
#                os.mkdir(target+ "/"+ dirname)
#            fd = open(target+"/"+ name,"w")
#            fd.write(zfile.read(name))
#            fd.close()
		    
    return pushfiles

def digest_file(path):
    global file_digest_cache
    if path in file_digest_cache:
        return file_digest_cache[path]
    size = os.path.getsize(path)
    h = hashlib.new('sha1',"km " + str(size) + "\0");
    with open(path,'rb') as f:
        h.update(f.read())
    file_digest_cache[path] = h.hexdigest()
    return file_digest_cache[path]

def digest_str(s):
    return hashlib.new('sha1',"km " + str(len(s)) + "\0" + s).hexdigest()

#def sign(str):
#    k = ('r,z-}"E-S.9k2?o3g16"6x\'r/7yWKB0',
#         "}owj(,9Uh07'c)rAxY8SA7g[DZ:*97Q",
#         '_v){DL]i}996U]wFG22QY\\8768\\-2jp',
#         'B1TGPCU-&uIFh@(Ppmm-;1?\\y31GlE1')
#    kk = "".join([k[1+((ord(k[0][i])>>(i%8))%3)][i] for i in range(len(k[0]))])
#    return hmac.new(kk,str).hexdigest()

if __name__ == '__main__':
    kedlcmd()
