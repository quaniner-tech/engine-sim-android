package com.enginesim.app

import android.content.Context
import android.content.res.AssetManager
import android.util.Log
import org.json.JSONObject
import java.io.InputStreamReader

/**
 * Native library interface for Engine-Sim
 */
class EngineSimLibrary {
    companion object {
        init {
            System.loadLibrary("engine_sim_jni")
        }

        const val TAG = "EngineSimLibrary"

        // Preset names
        const val PRESET_INLINE4 = "inline4"
        const val PRESET_V6 = "v6"
        const val PRESET_V8 = "v8"
        const val PRESET_V12 = "v12"
    }

    // Native handle
    private var nativeHandle: Long = 0

    // Audio engine - using native AAudio only (no Kotlin AudioEngine)

    // Configuration
    var sampleRate: Int = 44100
        private set
    var channelCount: Int = 1
        private set
    var bufferSizeFrames: Int = 512
        private set

    // Engine state
    private var engineThrottle: Float = 0f
    private var engineRpm: Float = 800f
    private var engineTorque: Float = 0f
    private var enginePower: Float = 0f

    /**
     * Create and initialize the engine
     */
    fun create(): Boolean {
        nativeHandle = nativeCreate()
        if (nativeHandle == 0L) {
            Log.e(TAG, "Failed to create engine handle")
            return false
        }

        // Set default engine parameters
        nativeSetThrottle(nativeHandle, 0f)

        return true
    }

    /**
     * Release resources
     */
    fun destroy() {
        shutdown()
        if (nativeHandle != 0L) {
            nativeDestroy(nativeHandle)
            nativeHandle = 0
        }
    }

    /**
     * Initialize audio playback
     */
    fun initializeAudio(
        sampleRate: Int = 44100,
        channelCount: Int = 1,
        bufferSizeFrames: Int = 512,
        assetManager: AssetManager
    ): Boolean {
        this.sampleRate = sampleRate
        this.channelCount = channelCount
        this.bufferSizeFrames = bufferSizeFrames

        if (nativeHandle == 0L) {
            Log.e(TAG, "Cannot initialize audio: no engine handle")
            return false
        }

        // Initialize native audio engine (AAudio) with Synthesizer + impulse response
        nativeInitializeAudio(nativeHandle, sampleRate, channelCount, bufferSizeFrames, assetManager)
        Log.i(TAG, "Native audio initialized")
        return true
    }

    /**
     * Set engine volume (0.0 to 1.0)
     */
    fun setVolume(volume: Float) {
        val clampedVolume = volume.coerceIn(0.0f, 1.0f)

        if (nativeHandle != 0L) {
            nativeSetVolume(nativeHandle, clampedVolume)
        }
    }

    /**
     * Get current RPM
     */
    fun getRpm(): Float {
        return if (nativeHandle != 0L) {
            nativeGetRpm(nativeHandle)
        } else {
            800f
        }
    }

    /**
     * Get current throttle position
     */
    fun getThrottle(): Float {
        return engineThrottle
    }

    /**
     * Get current torque (Nm)
     */
    fun getTorque(): Float {
        return engineTorque
    }

    /**
     * Get current power (HP)
     */
    fun getPower(): Float {
        return enginePower
    }

    /**
     * Set throttle position (0.0 to 1.0)
     */
    fun setThrottle(position: Float) {
        val clampedPosition = position.coerceIn(0.0f, 1.0f)
        engineThrottle = clampedPosition

        if (nativeHandle != 0L) {
            nativeSetThrottle(nativeHandle, clampedPosition)
        }
    }

    /**
     * Start the engine simulation
     */
    fun start() {
        if (nativeHandle != 0L) {
            nativeStart(nativeHandle)
        }
    }

    /**
     * Stop the engine simulation
     */
    fun stop() {
        if (nativeHandle != 0L) {
            nativeStop(nativeHandle)
        }
    }

    /**
     * Execute one simulation step
     */
    fun simulateStep(): Boolean {
        if (nativeHandle == 0L) return false

        // Run simulation step
        val result = nativeSimulateStep(nativeHandle)

        if (result) {
            // Update engine state
            engineRpm = nativeGetRpm(nativeHandle)
            engineTorque = nativeGetTorque(nativeHandle)
            enginePower = nativeGetPower(nativeHandle)
        }

        return result
    }

    /**
     * Load engine preset from assets
     */
    fun loadEnginePreset(presetName: String, context: Context? = null): Boolean {
        if (nativeHandle == 0L) {
            Log.e(TAG, "No engine handle")
            return false
        }

        // Try to load preset JSON from assets
        var presetJson: String? = null

        if (context != null) {
            try {
                val assetManager: AssetManager = context.assets
                val fileName = "presets/${presetName}.json"
                val inputStream = assetManager.open(fileName)
                val reader = InputStreamReader(inputStream)
                val buffer = StringBuilder()
                var char: Int
                while (reader.read().also { char = it } != -1) {
                    buffer.append(char.toChar())
                }
                reader.close()
                presetJson = buffer.toString()
            } catch (e: Exception) {
                Log.w(TAG, "Could not load preset from assets: ${e.message}")
            }
        }

        // Convert to JSON and extract parameters
        return if (presetJson != null) {
            try {
                val json = JSONObject(presetJson)
                val result = nativeLoadEnginePreset(
                    nativeHandle,
                    presetName,
                    json.optInt("cylinder_count", 4),
                    json.optDouble("bore_mm", 86.0).toFloat(),
                    json.optDouble("stroke_mm", 86.0).toFloat(),
                    json.optDouble("compression_ratio", 10.5).toFloat()
                )
                Log.i(TAG, "Loaded preset: $presetName")
                result
            } catch (e: Exception) {
                Log.e(TAG, "Failed to parse preset JSON", e)
                // Fall through to built-in preset
                nativeLoadEnginePresetByName(nativeHandle, presetName)
            }
        } else {
            // Use built-in preset
            nativeLoadEnginePresetByName(nativeHandle, presetName)
        }
    }

    /**
     * Read audio samples from synthesizer
     */
    fun readAudio(buffer: ShortArray, samples: Int): Int {
        if (nativeHandle == 0L) return 0

        // Read from native synthesizer
        val nativeRead = nativeReadAudio(nativeHandle, buffer, samples)

        return nativeRead
    }

    /**
     * Shutdown audio and release resources
     */
    fun shutdown() {
        // Native AAudio handles its own shutdown
    }

    // Native methods
    private external fun nativeCreate(): Long
    private external fun nativeDestroy(handle: Long)
    private external fun nativeStart(handle: Long)
    private external fun nativeStop(handle: Long)
    private external fun nativeSetThrottle(handle: Long, throttle: Float)
    private external fun nativeSetVolume(handle: Long, volume: Float)
    private external fun nativeGetRpm(handle: Long): Float
    private external fun nativeGetTorque(handle: Long): Float
    private external fun nativeGetPower(handle: Long): Float
    private external fun nativeReadAudio(handle: Long, buffer: ShortArray, samples: Int): Int
    private external fun nativeSimulateStep(handle: Long): Boolean
    private external fun nativeLoadEnginePreset(
        handle: Long,
        presetName: String,
        cylinderCount: Int,
        boreMm: Float,
        strokeMm: Float,
        compressionRatio: Float
    ): Boolean
    private external fun nativeLoadEnginePresetByName(handle: Long, presetName: String): Boolean
    private external fun nativeInitializeAudio(handle: Long, sampleRate: Int, channelCount: Int, bufferSizeFrames: Int, assetManager: android.content.res.AssetManager)
}