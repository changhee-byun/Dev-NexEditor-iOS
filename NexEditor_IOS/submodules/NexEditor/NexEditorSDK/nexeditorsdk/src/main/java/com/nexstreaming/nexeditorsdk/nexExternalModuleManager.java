package com.nexstreaming.nexeditorsdk;

import com.nexstreaming.nexeditorsdk.module.UserField;
import com.nexstreaming.nexeditorsdk.module.nexFaceDetectionProvider;
import com.nexstreaming.nexeditorsdk.module.nexExternalExportProvider;
import com.nexstreaming.nexeditorsdk.module.nexModuleProvider;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


/**
 *
 * @since 2.0.0
 */
public class nexExternalModuleManager {
    private static nexExternalModuleManager single;
    private static final String TAG = "nexModuleManager";

    private static final Class[] s_supportedModuleClass ={
            nexExternalExportProvider.class,
            nexFaceDetectionProvider.class
    };

    private List<ModuleInfo> moduleInfos = new ArrayList<ModuleInfo>();

    private Map<String, Class<? extends nexModuleProvider>> moduleProviders = new HashMap<>();

    private nexExternalModuleManager(){

    }

    static nexExternalModuleManager getInstance(){
        if( single == null ){
            single = new nexExternalModuleManager();
        }
        return single;
    }

    /**
     *
     * @param className
     * @since 2.0.0
     */
    public void registerModule(String className){
        try {
            Class test = Class.forName(className);
            registerModule(test);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    /**
     *
     * @param module
     * @since 2.0.0
     */
    public void registerModule(Class< ? extends nexModuleProvider> module){

        Constructor constructor = null;

        try {
            constructor = module.getConstructor();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }

        if( constructor != null ){
            try {
                nexModuleProvider moduleinst = (nexModuleProvider)constructor.newInstance();
                for(nexModuleProvider m : moduleInfos){
                    if( m.uuid().compareTo(moduleinst.uuid()) == 0 ){
                        return;
                    }
                }
                int flags = getSubClassType(moduleinst);
                if( flags == 0 ){
                    throw new RuntimeException("not supported Provider interface. uuid="+moduleinst.uuid());
                }
                moduleInfos.add(new ModuleInfo(moduleinst,flags));
                moduleProviders.put(moduleinst.uuid(), module);
            } catch (InstantiationException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     *
     * @param module
     * @since 2.0.0
     */
    public void unregisterModule(nexModuleProvider module){
        unregisterModule(module.uuid());
    }

    /**
     *
     * @param uuid
     * @since 2.0.0
     */
    public void unregisterModule(String uuid){
        nexModuleProvider find = null;
        for(nexModuleProvider m : moduleInfos){
            if( m.uuid().compareTo(uuid) == 0 ){
                find = m;
                break;
            }
        }
        if( find != null ){
            moduleInfos.remove(find);
            moduleProviders.remove(uuid);
        }

    }

    void clearModule(){
        moduleInfos.clear();
        moduleProviders.clear();
    }

    private final class ModuleInfo implements nexModuleProvider{
        private final String name;
        private final String uuid;
        private final String description;
        private final String auth;
        private final String format;
        private final int version;
        private final UserField[] userFields;
        private final int mSubClassFlags;

        private ModuleInfo(nexModuleProvider moduleProvider, int mSubClassFlags){
            this.name = moduleProvider.name();
            this.uuid = moduleProvider.uuid();
            this.description = moduleProvider.description();
            this.auth = moduleProvider.auth();
            this.version = moduleProvider.version();
            this.userFields = moduleProvider.userFields();
            this.format = moduleProvider.format();
            this.mSubClassFlags = mSubClassFlags;
        }

        @Override
        public String name() {
            return name;
        }

        @Override
        public String uuid() {
            return uuid;
        }

        @Override
        public String description() {
            return description;
        }

        @Override
        public String auth() {
            return auth;
        }

        @Override
        public String format() {
            return format;
        }

        @Override
        public int version() {
            return version;
        }

        @Override
        public UserField[] userFields() {
            return userFields;
        }
    }

    private int getSubClassType(Object object){
        int flags = 0;

        for( int i = 0; i < s_supportedModuleClass.length ; i++ ){
            if( s_supportedModuleClass[i].isInstance(object) ){
                flags |= i+1;
            }
        }
        return flags;
    }

    private int getSubClassType(Class <? extends nexModuleProvider> T){
        for( int i = 0; i < s_supportedModuleClass.length ; i++ ){
            if( s_supportedModuleClass[i].isAssignableFrom(T) ){
                return i+1;
            }
        }
        return 0;
    }

    /**
     *
     * @param T
     * @return
     * @since 2.0.0
     */
    public List<nexModuleProvider> getModules(Class <? extends nexModuleProvider> T ){
        List<nexModuleProvider> list = new ArrayList<>();
        int type = getSubClassType(T);
        for( ModuleInfo module : moduleInfos ){
            if( (module.mSubClassFlags & type) != 0 ){
                list.add(module);
            }
        }
        return list;
    }

    Object getModule(String name, Class <? extends nexModuleProvider> T ){
        int type = getSubClassType(T);

        for(ModuleInfo module : moduleInfos){
            if( module.name().compareTo(name) == 0 ){
                if( (module.mSubClassFlags & type) != 0 ){
                    return getModule(module.uuid());
                }
            }
        }
        return null;
    }

    Object getModule(String uuid){
        Class module = moduleProviders.get(uuid);

        Constructor constructor = null;

        try {
            constructor = module.getConstructor();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }

        if( constructor != null ){
            try {
                return constructor.newInstance();
            } catch (InstantiationException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }
        }
        return null;
    }
}
