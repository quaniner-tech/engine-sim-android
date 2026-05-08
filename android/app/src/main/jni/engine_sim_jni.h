#ifndef ENGINE_SIM_JNI_H
#define ENGINE_SIM_JNI_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

// Library lifecycle
JNIEXPORT jlong JNICALL
Java_com_enginesim_EngineSimLibrary_nativeCreate(JNIEnv *env, jobject thiz);

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeDestroy(JNIEnv *env, jobject thiz, jlong handle);

// Engine configuration
JNIEXPORT jboolean JNICALL
Java_com_enginesim_EngineSimLibrary_nativeLoadEngineConfig(JNIEnv *env, jobject thiz,
    jlong handle, jstring configJson);

JNIEXPORT jboolean JNICALL
Java_com_enginesim_EngineSimLibrary_nativeLoadEnginePreset(JNIEnv *env, jobject thiz,
    jlong handle, jint presetType);

// Simulation control
JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeStart(JNIEnv *env, jobject thiz, jlong handle);

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeStop(JNIEnv *env, jobject thiz, jlong handle);

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeSetThrottle(JNIEnv *env, jobject thiz,
    jlong handle, jfloat throttle);

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeSetSpeedControl(JNIEnv *env, jobject thiz,
    jlong handle, jfloat speed);

// Simulation state queries
JNIEXPORT jfloat JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetRpm(JNIEnv *env, jobject thiz, jlong handle);

JNIEXPORT jfloat JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetTorque(JNIEnv *env, jobject thiz, jlong handle);

JNIEXPORT jfloat JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetPower(JNIEnv *env, jobject thiz, jlong handle);

JNIEXPORT jfloat JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetSpeed(JNIEnv *env, jobject thiz, jlong handle);

JNIEXPORT jboolean JNICALL
Java_com_enginesim_EngineSimLibrary_nativeIsSpinning(JNIEnv *env, jobject thiz, jlong handle);

// Audio
JNIEXPORT jint JNICALL
Java_com_enginesim_EngineSimLibrary_nativeReadAudio(JNIEnv *env, jobject thiz,
    jlong handle, jshortArray buffer, jint samples);

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeSetVolume(JNIEnv *env, jobject thiz,
    jlong handle, jfloat volume);

JNIEXPORT jint JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetSampleRate(JNIEnv *env, jobject thiz, jlong handle);

// Simulation step
JNIEXPORT jboolean JNICALL
Java_com_enginesim_EngineSimLibrary_nativeSimulateStep(JNIEnv *env, jobject thiz, jlong handle);

JNIEXPORT jdouble JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetExhaustFlow(JNIEnv *env, jobject thiz, jlong handle);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_SIM_JNI_H