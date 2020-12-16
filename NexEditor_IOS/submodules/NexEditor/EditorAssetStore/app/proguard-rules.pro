# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in C:\Users\ojin.kwon\AppData\Local\Android\sdk/tools/proguard/proguard-android.txt
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


-dontwarn org.apache.**
-dontwarn android.net.http.AndroidHttpClient
-dontwarn com.google.android.gms.internal.**
-dontwarn com.google.api.client.http.apache.**
-dontwarn com.dropbox.**
-dontwarn sun.misc.Unsafe
-dontwarn com.google.common.collect.MinMaxPriorityQueue
-dontwarn okio.**
-dontwarn org.codehaus.mojo.**
-dontwarn com.google.common.**
-dontnote com.google.common.**

-keepattributes Exceptions
-keepattributes *Annotation*
-keepattributes Signature

-keep public class com.nexstreaming.app.general.task.** { *;}
-keep public class com.nexstreaming.app.general.task.Task$Event { *;}
-keep class javax.**
-keep class org.*

-keep class * extends com.google.api.client.json.GenericJson { *; }
-keepclassmembers class * extends com.google.api.client.json.GenericJson {
    *;
}
-keep class * extends com.google.api.client.googleapis.services.json.AbstractGoogleJsonClientRequest { *; }
-keepclassmembers class * extends com.google.api.client.googleapis.services.json.AbstractGoogleJsonClientRequest {
     *;
 }

 -keep class com.google.gson.** { *; }
-keepnames class * implements java.io.Serializable

-keep class com.nexstreaming.app.assetlibrary.network.assetstore.** { *; }
-keep class com.nexstreaming.app.assetlibrary.model.** { *; }