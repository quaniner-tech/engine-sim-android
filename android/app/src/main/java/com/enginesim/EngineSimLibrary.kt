package com.enginesim

import android.content.Context

/**
 * EngineSimLibrary - Kotlin wrapper for the native engine simulation library.
 *
 * Provides JNI bridge to libengine_sim_core for Android.
 */
class EngineSimLibrary {

    companion object {
        init {
            System.loadLibrary("engine_sim_core")
            System.loadLibrary("engine_sim_jni")
        }
    }

    // Engine preset types
    enum class EnginePreset(val value: Int) {
        Inline4(0),
        V6(1),
        V8(2),
        V12(3)
    }

    // Native handle (pointer to EngineSimHandle in C++)
    private var nativeHandle: Long = 0

    val isCreated: Boolean
        get() = nativeHandle != 0L

    /**
     * Create a new engine simulator instance.
     * @return true if creation succeeded
     */
    fun create(): Boolean {
        if (nativeHandle != 0L) {
            destroy()
        }
        nativeHandle = nativeCreate()
        return nativeHandle != 0L
    }

    /**
     * Destroy the engine simulator and release resources.
     */
    fun destroy() {
        if (nativeHandle != 0L) {
            nativeDestroy(nativeHandle)
            nativeHandle = 0L
        }
    }

    /**
     * Load engine configuration from JSON string.
     * @param configJson JSON configuration string
     * @return true if loaded successfully
     */
    fun loadEngineConfig(configJson: String): Boolean {
        if (nativeHandle == 0L) return false
        return nativeLoadEngineConfig(nativeHandle, configJson)
    }

    /**
     * Load a preset engine configuration.
     * @param preset The preset type to load
     * @return true if loaded successfully
     */
    fun loadEnginePreset(preset: EnginePreset): Boolean {
        if (nativeHandle == 0L) return false
        return nativeLoadEnginePreset(nativeHandle, preset.value)
    }

    /**
     * Start the simulation.
     */
    fun start() {
        if (nativeHandle != 0L) {
            nativeStart(nativeHandle)
        }
    }

    /**
     * Stop the simulation.
     */
    fun stop() {
        if (nativeHandle != 0L) {
            nativeStop(nativeHandle)
        }
    }

    /**
     * Set the throttle position (0.0 to 1.0).
     */
    fun setThrottle(throttle: Float) {
        if (nativeHandle != 0L) {
            nativeSetThrottle(nativeHandle, throttle)
        }
    }

    /**
     * Set speed control for dynamometer mode.
     */
    fun setSpeedControl(speed: Float) {
        if (nativeHandle != 0L) {
            nativeSetSpeedControl(nativeHandle, speed)
        }
    }

    /**
     * Get current engine RPM.
     */
    fun getRpm(): Float {
        if (nativeHandle == 0L) return 0f
        return nativeGetRpm(nativeHandle)
    }

    /**
     * Get current engine torque in N*m.
     */
    fun getTorque(): Float {
        if (nativeHandle == 0L) return 0f
        return nativeGetTorque(nativeHandle)
    }

    /**
     * Get current engine power in watts.
     */
    fun getPower(): Float {
        if (nativeHandle == 0L) return 0f
        return nativeGetPower(nativeHandle)
    }

    /**
     * Get vehicle speed in m/s.
     */
    fun getSpeed(): Float {
        if (nativeHandle == 0L) return 0f
        return nativeGetSpeed(nativeHandle)
    }

    /**
     * Check if engine is spinning.
     */
    fun isSpinning(): Boolean {
        if (nativeHandle == 0L) return false
        return nativeIsSpinning(nativeHandle)
    }

    /**
     * Read audio samples from the synthesizer.
     * @param buffer ShortArray to receive PCM samples
     * @param samples Number of samples to read
     * @return Number of samples actually read
     */
    fun readAudio(buffer: ShortArray, samples: Int): Int {
        if (nativeHandle == 0L) return 0
        return nativeReadAudio(nativeHandle, buffer, samples)
    }

    /**
     * Set audio volume (0.0 to 1.0).
     */
    fun setVolume(volume: Float) {
        if (nativeHandle != 0L) {
            nativeSetVolume(nativeHandle, volume)
        }
    }

    /**
     * Get audio sample rate.
     */
    fun getSampleRate(): Int {
        if (nativeHandle == 0L) return 44100
        return nativeGetSampleRate(nativeHandle)
    }

    /**
     * Execute one simulation step.
     * @return true if simulation step succeeded
     */
    fun simulateStep(): Boolean {
        if (nativeHandle == 0L) return false
        return nativeSimulateStep(nativeHandle)
    }

    /**
     * Get total exhaust flow rate.
     */
    fun getExhaustFlow(): Double {
        if (nativeHandle == 0L) return 0.0
        return nativeGetExhaustFlow(nativeHandle)
    }

    // ============================================
    // Native JNI methods (loaded from libengine_sim_jni.so)
    // ============================================

    private external fun nativeCreate(): Long
    private external fun nativeDestroy(handle: Long)
    private external fun nativeLoadEngineConfig(handle: Long, configJson: String): Boolean
    private external fun nativeLoadEnginePreset(handle: Long, presetType: Int): Boolean
    private external fun nativeStart(handle: Long)
    private external fun nativeStop(handle: Long)
    private external fun nativeSetThrottle(handle: Long, throttle: Float)
    private external fun nativeSetSpeedControl(handle: Long, speed: Float)
    private external fun nativeGetRpm(handle: Long): Float
    private external fun nativeGetTorque(handle: Long): Float
    private external fun nativeGetPower(handle: Long): Float
    private external fun nativeGetSpeed(handle: Long): Float
    private external fun nativeIsSpinning(handle: Long): Boolean
    private external fun nativeReadAudio(handle: Long, buffer: ShortArray, samples: Int): Int
    private external fun nativeSetVolume(handle: Long, volume: Float)
    private external fun nativeGetSampleRate(handle: Long): Int
    private external fun nativeSimulateStep(handle: Long): Boolean
    private external fun nativeGetExhaustFlow(handle: Long): Double
}