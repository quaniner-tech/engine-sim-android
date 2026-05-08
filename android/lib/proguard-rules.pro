# Engine-Sim Android Consumer ProGuard Rules
#
# Add this to your app's proguard-rules.pro when using engine-sim-android as AAR

# Keep JNI native methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep EngineSimLibrary class
-keep class com.enginesim.** { *; }

# Keep native method signatures
-keepclassmembers,allowobfuscation class * {
    @android.keep * *;
}

# Prevent stripping of native methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep enum classes
-keepclassmembers enum * {
    public static **[] values();
    public static ** valueOf(java.lang.String);
}

# Core library (C++) - don't optimize
-dontwarn engine_sim_core**
-keep class engine_sim_core** { *; }