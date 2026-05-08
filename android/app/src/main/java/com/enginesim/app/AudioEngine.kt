package com.enginesim.app

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.os.Build
import android.util.Log
import kotlinx.coroutines.*
import kotlinx.coroutines.currentCoroutineContext
import kotlin.math.min

/**
 * Android Audio Engine for Engine-Sim
 * 
 * Provides audio playback using Android AudioTrack.
 * For API 26+, uses AAudio via AudioTrack for lower latency.
 */
class AudioEngine(
    private val sampleRate: Int = 44100,
    private val channelCount: Int = 1,
    private val bufferSizeFrames: Int = 512
) : AutoCloseable {
    companion object {
        private const val TAG = "AudioEngine"
        private const val BYTES_PER_SAMPLE = 2 // 16-bit PCM
    }

    private var audioTrack: AudioTrack? = null
    private var isPlaying = false
    private var audioJob: Job? = null
    private val scope = CoroutineScope(Dispatchers.Default + SupervisorJob())

    // Ring buffer for audio data from native layer
    private val audioBuffer = ShortArray(sampleRate * 2) // ~2 seconds buffer
    private var writeIndex = 0
    private var readIndex = 0
    private val bufferLock = Any()

    // Volume (0.0 to 1.0)
    @Volatile
    var volume: Float = 1.0f

    /**
     * Initialize audio playback
     */
    fun initialize(): Boolean {
        return try {
            val bufferSizeBytes = AudioTrack.getMinBufferSize(
                sampleRate,
                if (channelCount == 1) AudioFormat.CHANNEL_OUT_MONO else AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT
            )

            if (bufferSizeBytes == AudioTrack.ERROR || bufferSizeBytes == AudioTrack.ERROR_BAD_VALUE) {
                Log.e(TAG, "Invalid buffer size: $bufferSizeBytes")
                return false
            }

            val audioAttributes = AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_GAME)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build()

            val audioFormat = AudioFormat.Builder()
                .setSampleRate(sampleRate)
                .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                .setChannelMask(if (channelCount == 1) AudioFormat.CHANNEL_OUT_MONO else AudioFormat.CHANNEL_OUT_STEREO)
                .build()

            val builder = AudioTrack.Builder()
                .setAudioAttributes(audioAttributes)
                .setAudioFormat(audioFormat)
                .setBufferSizeInBytes(maxOf(bufferSizeBytes, bufferSizeFrames * channelCount * BYTES_PER_SAMPLE))
                .setTransferMode(AudioTrack.MODE_STREAM)
                .setPerformanceMode(AudioTrack.PERFORMANCE_MODE_LOW_LATENCY)

            // Session ID allocation handled automatically by AudioTrack.Builder

            audioTrack = builder.build()

            if (audioTrack?.state != AudioTrack.STATE_INITIALIZED) {
                Log.e(TAG, "AudioTrack initialization failed")
                audioTrack?.release()
                audioTrack = null
                return false
            }

            isPlaying = true
            audioTrack?.play()

            // Start audio writing job
            audioJob = scope.launch {
                writeAudioLoop()
            }

            Log.i(TAG, "AudioEngine initialized: rate=$sampleRate, channels=$channelCount, buffer=$bufferSizeBytes")
            true
        } catch (e: Exception) {
            Log.e(TAG, "Failed to initialize AudioEngine", e)
            false
        }
    }

    /**
     * Write PCM data from native synthesis
     * This is called from JNI/native code
     */
    fun writeAudio(samples: ShortArray, count: Int) {
        synchronized(bufferLock) {
            for (i in 0 until count) {
                val index = (writeIndex) % audioBuffer.size
                audioBuffer[index] = (samples[i] * volume).toInt().toShort()
                writeIndex++
            }
        }
    }

    /**
     * Get current RPM for pitch adjustment
     */
    fun setRpm(rpm: Float) {
        // Future: implement pitch-shifting based on RPM
        // For now, RPM affects synthesis in native layer
    }

    /**
     * Set playback volume
     */
    fun setVolumeLevel(vol: Float) {
        volume = vol.coerceIn(0.0f, 1.0f)
    }

    private suspend fun writeAudioLoop() {
        val buffer = ShortArray(bufferSizeFrames * channelCount)

        while (currentCoroutineContext().isActive && isPlaying) {
            // Read from ring buffer
            val samplesRead = synchronized(bufferLock) {
                var count = 0
                while (count < buffer.size && readIndex != writeIndex) {
                    buffer[count++] = audioBuffer[readIndex % audioBuffer.size]
                    readIndex++
                }
                count
            }

            // Fill remaining with silence
            for (i in samplesRead until buffer.size) {
                buffer[i] = 0
            }

            // Write to AudioTrack
            val track = audioTrack
            if (track != null) {
                track.write(buffer, 0, buffer.size)
            }

            // Small delay to prevent tight loop
            if (samplesRead == 0) {
                delay(1)
            }
        }
    }

    /**
     * Stop and release audio resources
     */
    fun shutdown() {
        close()
    }

    override fun close() {
        Log.i(TAG, "Shutting down AudioEngine")
        isPlaying = false

        audioJob?.cancel()
        scope.cancel()

        try {
            audioTrack?.stop()
        } catch (e: IllegalStateException) {
            Log.w(TAG, "AudioTrack stop failed", e)
        }
        audioTrack?.release()
        audioTrack = null

        Log.i(TAG, "AudioEngine shutdown complete")
    }
}