# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in C:\Users\Eric\AppData\Local\Android\sdk/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

-keepattributes EnclosingMethod
-dontwarn org.apache.**
-keepattributes InnerClasses
-keepattributes Exceptions
-keepattributes *Annotation*
-keepattributes Signature
-keepattributes SourceFile,LineNumberTable
-keep public class com.nexstreaming.nexeditorsdk.** {*;}
-keep public class com.nexstreaming.nexeditorsdk.service.** {*;}
-keep public class com.nexstreaming.app.common.task.** { *;}
-keep public class com.nexstreaming.app.common.task.Task$Event { *;}
-keep class javax.**
-keep class org.*
-keep public class com.google.api.services.drive.** { *; }
-keep public class com.google.**
-keep public class com.nexstreaming.nexeditorsdk.WrapperForSurfaceTextureListener {*;}
-keep class * extends com.google.api.client.json.GenericJson { *; }
-keepclassmembers class * extends com.google.api.client.json.GenericJson {
    *;
}
-keep class * extends com.google.api.client.googleapis.services.json.AbstractGoogleJsonClientRequest { *; }
-keepclassmembers class * extends com.google.api.client.googleapis.services.json.AbstractGoogleJsonClientRequest {
     *;
 }

-keep class com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader$PackageInfoJSON { <fields>; *; }
-keep class com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader$ItemInfoJSON { <fields>; *; }
-keep class com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader$EncryptionInfoJSON { <fields>; *; }
-keep class com.nexstreaming.app.common.nexasset.assetpackage.security.provider.BasicEncryptionProvider$ProviderSpecificDataJSON { <fields>; *; }

-keep class com.nexstreaming.app.common.nexasset.store.json
-keep class com.nexstreaming.kminternal.json.** { *; }
-keep class com.nexstreaming.nexvision.** { *; }

-keep class com.nexstreaming.app.common.nexasset.overlay.** { *; }

-keep class * extends com.nexstreaming.app.common.norm.NormTable { *; }
-keep class com.nexstreaming.app.common.nexasset.store.** { *; }

-keep public class com.nexstreaming.kminternal.nexvideoeditor.NexEditorEventListener {
    public *;
}
-keep public class com.nexstreaming.kminternal.nexvideoeditor.NexImage {
    public *;
}
-keep public class com.nexstreaming.kminternal.nexvideoeditor.NexAudioClip {
    <fields>;
    public *;
}
-keep public class com.nexstreaming.kminternal.nexvideoeditor.NexVisualClip {
    <fields>;
    public *;
}
-keep public class com.nexstreaming.kminternal.nexvideoeditor.NexLayerClip {
    <fields>;
    public *;
}
-keep public class com.nexstreaming.kminternal.nexvideoeditor.NexRectangle {
    <fields>;
    public *;
}
-keep public class com.nexstreaming.kminternal.nexvideoeditor.NexClipInfo {
    <fields>;
    public *;
}
-keep public class com.nexstreaming.kminternal.nexvideoeditor.NexDrawInfo {
    <fields>;
    public *;
}

-keep public class com.nexstreaming.app.common.util.RotationGestureDetector{ *; }
-keep public class com.nexstreaming.app.common.util.RotationGestureDetector$OnRotationGestureListener{ *; }
-keep class com.nexstreaming.app.common.tracelog.*Request** { *; }
-keep class com.nexstreaming.app.common.tracelog.*Response** { *; }
-keep class com.nexstreaming.kminternal.kinemaster.kmpackage.KMTServerIndex { *; <fields>;}
-keep class com.nexstreaming.kminternal.kinemaster.kmpackage.KMTServerIndex$* { *; <fields>; }
-keep class com.nexstreaming.kminternal.kinemaster.kmpackage.ClipEffectAutoSelectContext { *; <fields>;}
-keep class com.nexstreaming.kminternal.kinemaster.kmpackage.EffectAutoSelectContext { *; <fields>;}
-keep class com.nexstreaming.kminternal.kinemaster.kmpackage.TransitionEffectAutoSelectContext { *; <fields>;}
-keep class sun.misc.Unsafe { *; }
-keep class com.google.gson_nex.** { *; }
-keep class **keyczar** {*;}
-keep class com.nexstreaming.kminternal.kinemaster.editorwrapper.KMIntentData { *; }
-keep class com.nexstreaming.kminternal.kinemaster.editorwrapper.KMIntentData$* { *; }
-keep class com.nexstreaming.app.common.nexasset.assetpackage.db.** { *; }

-keepclassmembers class com.nexstreaming.kminternal.nexvideoeditor.NexThemeRenderer {
    void setThemeInstanceHandle(long);
    long getThemeInstanceHandle();
}
-keep class **.NexImageLoader { *; }
-keep class com.nexstreaming.kminternal.nexvideoeditor.NexImageLoader { *; }
-keepclassmembers class com.nexstreaming.kminternal.nexvideoeditor.NexImageLoader {
    public com.nexstreaming.nexvideoeditor.NexImage openThemeImage(String);
}
-keepnames class * implements java.io.Serializable

-keepclassmembers class * implements java.io.Serializable {
    static final long serialVersionUID;
    private static final java.io.ObjectStreamField[] serialPersistentFields;
    !static !transient <fields>;
    !private <fields>;
    !private <methods>;
    private void writeObject(java.io.ObjectOutputStream);
    private void readObject(java.io.ObjectInputStream);
    java.lang.Object writeReplace();
    java.lang.Object readResolve();
}

-keepclassmembers enum * {
     <fields>;
     public static **[] values();
     public static ** valueOf(java.lang.String);
}
