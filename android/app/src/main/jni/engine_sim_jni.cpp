#include "engine_sim_jni.h"
#include "../../../../core/include/engine_sim/piston_engine_simulator.h"
#include "../../../../core/include/engine_sim/engine.h"
#include "../../../../core/include/engine_sim/vehicle.h"
#include "../../../../core/include/engine_sim/transmission.h"

#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

namespace {
    // Engine preset types (matches Kotlin enum)
    enum class EnginePreset {
        Inline4 = 0,
        V6 = 1,
        V8 = 2,
        V12 = 3
    };

    // Internal state handle
    struct EngineSimHandle {
        PistonEngineSimulator* simulator = nullptr;
        Engine* engine = nullptr;
        Vehicle* vehicle = nullptr;
        Transmission* transmission = nullptr;
        bool isRunning = false;
        int sampleRate = 44100;
        float volume = 1.0f;

        ~EngineSimHandle() {
            if (simulator) {
                simulator->destroy();
                delete simulator;
            }
            delete engine;
            delete vehicle;
            delete transmission;
        }
    };

    // Simple JSON-like config parsing (minimal implementation)
    // For production, use a proper JSON library
    void parseEngineConfig(Engine::Parameters& params, const char* json) {
        // Default values for a basic 4-cylinder engine
        params.cylinderBanks = 1;
        params.cylinderCount = 4;
        params.crankshaftCount = 1;
        params.exhaustSystemCount = 1;
        params.intakeCount = 1;
        params.name = "4-Cylinder Engine";
        params.starterTorque = 90.0 * 1.3558179483314; // ft_lb to N*m
        params.starterSpeed = 200.0 * 2.0 * 3.14159265359 / 60.0; // rpm to rad/s
        params.redline = 6500.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMinSpeed = 1000.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMaxSpeed = 6500.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoHoldStep = 100.0 * 2.0 * 3.14159265359 / 60.0;
        params.initialSimulationFrequency = 10000.0;
        params.initialHighFrequencyGain = 0.3;
        params.initialNoise = 0.05;
        params.initialJitter = 0.001;
    }

    void setupInline4(Engine::Parameters& params) {
        params.cylinderBanks = 1;
        params.cylinderCount = 4;
        params.crankshaftCount = 1;
        params.exhaustSystemCount = 1;
        params.intakeCount = 1;
        params.name = "Inline-4 Engine";
        params.starterTorque = 90.0 * 1.3558179483314;
        params.starterSpeed = 200.0 * 2.0 * 3.14159265359 / 60.0;
        params.redline = 7000.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMinSpeed = 1000.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMaxSpeed = 7000.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoHoldStep = 100.0 * 2.0 * 3.14159265359 / 60.0;
        params.initialSimulationFrequency = 10000.0;
        params.initialHighFrequencyGain = 0.3;
        params.initialNoise = 0.05;
        params.initialJitter = 0.001;
    }

    void setupV6(Engine::Parameters& params) {
        params.cylinderBanks = 2;
        params.cylinderCount = 6;
        params.crankshaftCount = 1;
        params.exhaustSystemCount = 2;
        params.intakeCount = 2;
        params.name = "V6 Engine";
        params.starterTorque = 120.0 * 1.3558179483314;
        params.starterSpeed = 180.0 * 2.0 * 3.14159265359 / 60.0;
        params.redline = 6500.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMinSpeed = 800.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMaxSpeed = 6500.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoHoldStep = 100.0 * 2.0 * 3.14159265359 / 60.0;
        params.initialSimulationFrequency = 10000.0;
        params.initialHighFrequencyGain = 0.35;
        params.initialNoise = 0.05;
        params.initialJitter = 0.001;
    }

    void setupV8(Engine::Parameters& params) {
        params.cylinderBanks = 2;
        params.cylinderCount = 8;
        params.crankshaftCount = 1;
        params.exhaustSystemCount = 2;
        params.intakeCount = 2;
        params.name = "V8 Engine";
        params.starterTorque = 150.0 * 1.3558179483314;
        params.starterSpeed = 150.0 * 2.0 * 3.14159265359 / 60.0;
        params.redline = 7000.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMinSpeed = 700.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMaxSpeed = 7000.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoHoldStep = 100.0 * 2.0 * 3.14159265359 / 60.0;
        params.initialSimulationFrequency = 10000.0;
        params.initialHighFrequencyGain = 0.4;
        params.initialNoise = 0.04;
        params.initialJitter = 0.001;
    }

    void setupV12(Engine::Parameters& params) {
        params.cylinderBanks = 2;
        params.cylinderCount = 12;
        params.crankshaftCount = 1;
        params.exhaustSystemCount = 2;
        params.intakeCount = 2;
        params.name = "V12 Engine";
        params.starterTorque = 200.0 * 1.3558179483314;
        params.starterSpeed = 120.0 * 2.0 * 3.14159265359 / 60.0;
        params.redline = 8000.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMinSpeed = 600.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoMaxSpeed = 8000.0 * 2.0 * 3.14159265359 / 60.0;
        params.dynoHoldStep = 100.0 * 2.0 * 3.14159265359 / 60.0;
        params.initialSimulationFrequency = 10000.0;
        params.initialHighFrequencyGain = 0.45;
        params.initialNoise = 0.03;
        params.initialJitter = 0.001;
    }
}

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_enginesim_EngineSimLibrary_nativeCreate(JNIEnv *env, jobject thiz) {
    EngineSimHandle* handle = new EngineSimHandle();

    handle->simulator = new PistonEngineSimulator();
    handle->engine = new Engine();
    handle->vehicle = new Vehicle();
    handle->transmission = new Transmission();

    // Initialize with default inline-4
    Engine::Parameters params;
    setupInline4(params);
    handle->engine->initialize(params);

    handle->simulator->loadSimulation(handle->engine, handle->vehicle, handle->transmission);

    handle->simulator->setSimulationFrequency(10000);
    handle->simulator->startAudioRenderingThread();

    handle->sampleRate = 44100;
    handle->isRunning = true;

    return reinterpret_cast<jlong>(handle);
}

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeDestroy(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return;

    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    if (simHandle->simulator) {
        simHandle->simulator->endAudioRenderingThread();
    }
    delete simHandle;
}

JNIEXPORT jboolean JNICALL
Java_com_enginesim_EngineSimLibrary_nativeLoadEngineConfig(JNIEnv *env, jobject thiz,
    jlong handle, jstring configJson) {
    if (handle == 0) return JNI_FALSE;

    const char* json = env->GetStringUTFChars(configJson, nullptr);
    if (!json) return JNI_FALSE;

    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);

    // Re-initialize with new config
    Engine::Parameters params;
    parseEngineConfig(params, json);

    // Clean up old engine
    if (simHandle->engine) {
        delete simHandle->engine;
    }
    simHandle->engine = new Engine();
    simHandle->engine->initialize(params);
    simHandle->simulator->loadSimulation(simHandle->engine, simHandle->vehicle, simHandle->transmission);

    env->ReleaseStringUTFChars(configJson, json);
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_enginesim_EngineSimLibrary_nativeLoadEnginePreset(JNIEnv *env, jobject thiz,
    jlong handle, jint presetType) {
    if (handle == 0) return JNI_FALSE;

    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);

    Engine::Parameters params;

    switch (static_cast<EnginePreset>(presetType)) {
        case EnginePreset::Inline4:
            setupInline4(params);
            break;
        case EnginePreset::V6:
            setupV6(params);
            break;
        case EnginePreset::V8:
            setupV8(params);
            break;
        case EnginePreset::V12:
            setupV12(params);
            break;
        default:
            setupInline4(params);
    }

    if (simHandle->engine) {
        delete simHandle->engine;
    }
    simHandle->engine = new Engine();
    simHandle->engine->initialize(params);
    simHandle->simulator->loadSimulation(simHandle->engine, simHandle->vehicle, simHandle->transmission);

    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeStart(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    simHandle->isRunning = true;
}

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeStop(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    simHandle->isRunning = false;
}

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeSetThrottle(JNIEnv *env, jobject thiz,
    jlong handle, jfloat throttle) {
    if (handle == 0) return;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    if (simHandle->engine) {
        simHandle->engine->setThrottle(static_cast<double>(throttle));
    }
}

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeSetSpeedControl(JNIEnv *env, jobject thiz,
    jlong handle, jfloat speed) {
    if (handle == 0) return;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    if (simHandle->engine) {
        simHandle->engine->setSpeedControl(static_cast<double>(speed));
    }
}

JNIEXPORT jfloat JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetRpm(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return 0.0f;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    if (simHandle->engine) {
        double rpm = simHandle->engine->getRpm();
        // Convert rad/s to RPM
        return static_cast<jfloat>(rpm * 60.0 / (2.0 * 3.14159265359));
    }
    return 0.0f;
}

JNIEXPORT jfloat JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetTorque(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return 0.0f;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    if (simHandle->engine && simHandle->engine->getOutputCrankshaft()) {
        // Torque in N*m (from SCS physics)
        return static_cast<jfloat>(0.0);
    }
    return 0.0f;
}

JNIEXPORT jfloat JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetPower(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return 0.0f;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    if (simHandle->engine) {
        double rpm = simHandle->engine->getRpm();
        double torque = simHandle->engine->getOutputCrankshaft() ?
            0.0 : 0.0;
        // Power = torque * angular_velocity (watts)
        return static_cast<jfloat>(torque * rpm);
    }
    return 0.0f;
}

JNIEXPORT jfloat JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetSpeed(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return 0.0f;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    if (simHandle->vehicle) {
        return static_cast<jfloat>(simHandle->vehicle->getSpeed());
    }
    return 0.0f;
}

JNIEXPORT jboolean JNICALL
Java_com_enginesim_EngineSimLibrary_nativeIsSpinning(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return JNI_FALSE;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    if (simHandle->engine) {
        return simHandle->engine->isSpinningCw() ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

JNIEXPORT jint JNICALL
Java_com_enginesim_EngineSimLibrary_nativeReadAudio(JNIEnv *env, jobject thiz,
    jlong handle, jshortArray buffer, jint samples) {
    if (handle == 0 || buffer == nullptr) return 0;
    if (samples <= 0 || samples > 32768) return 0;

    jsize bufferLen = env->GetArrayLength(buffer);
    if (bufferLen < samples) return 0;

    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);

    jshort* nativeBuffer = env->GetShortArrayElements(buffer, nullptr);
    if (!nativeBuffer) return 0;

    int samplesRead = simHandle->simulator->readAudioOutput(samples, nativeBuffer);

    // Apply volume
    if (simHandle->volume != 1.0f) {
        float vol = simHandle->volume;
        for (int i = 0; i < samplesRead; i++) {
            float sample = static_cast<float>(nativeBuffer[i]) * vol;
            sample = std::max(-32768.0f, std::min(32767.0f, sample));
            nativeBuffer[i] = static_cast<jshort>(sample);
        }
    }

    env->ReleaseShortArrayElements(buffer, nativeBuffer, 0);
    return samplesRead;
}

JNIEXPORT void JNICALL
Java_com_enginesim_EngineSimLibrary_nativeSetVolume(JNIEnv *env, jobject thiz,
    jlong handle, jfloat volume) {
    if (handle == 0) return;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    simHandle->volume = volume;
}

JNIEXPORT jint JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetSampleRate(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return 44100;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    return simHandle->sampleRate;
}

JNIEXPORT jboolean JNICALL
Java_com_enginesim_EngineSimLibrary_nativeSimulateStep(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return JNI_FALSE;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);

    if (!simHandle->isRunning) return JNI_FALSE;

    double dt = simHandle->simulator->getTimestep();
    simHandle->simulator->startFrame(dt);
    bool result = simHandle->simulator->simulateStep();
    simHandle->simulator->endFrame();

    return result ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jdouble JNICALL
Java_com_enginesim_EngineSimLibrary_nativeGetExhaustFlow(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) return 0.0;
    EngineSimHandle* simHandle = reinterpret_cast<EngineSimHandle*>(handle);
    return simHandle->simulator->getTotalExhaustFlow();
}

} // extern "C"