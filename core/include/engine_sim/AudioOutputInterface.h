#ifndef ENGINE_SIM_AUDIO_OUTPUT_INTERFACE_H
#define ENGINE_SIM_AUDIO_OUTPUT_INTERFACE_H

#include <cstdint>

namespace engine_sim {

/**
 * @brief Audio output abstraction for platform-specific implementations
 * 
 * This pure virtual interface allows the Synthesizer to output audio
 * to any platform (Android AAudio, Windows DirectSound, etc.) without
 * coupling to any specific audio API.
 */
class AudioOutputInterface {
public:
    virtual ~AudioOutputInterface() = default;

    /**
     * @brief Initialize the audio output device
     * @param sampleRate Sample rate in Hz (e.g., 44100)
     * @param channelCount Number of audio channels (1 = mono, 2 = stereo)
     * @param bufferSizeFrames Buffer size in frames
     * @return true if initialization succeeded
     */
    virtual bool initialize(int sampleRate, int channelCount, int bufferSizeFrames) = 0;

    /**
     * @brief Write PCM audio data to the output device
     * @param data Pointer to interleaved PCM data (int16_t)
     * @param sampleCount Number of samples (not frames)
     */
    virtual void writeAudio(const int16_t* data, int sampleCount) = 0;

    /**
     * @brief Set the output volume (0.0 to 1.0)
     */
    virtual void setVolume(float volume) = 0;

    /**
     * @brief Check if audio output is active
     */
    virtual bool isActive() const = 0;

    /**
     * @brief Shutdown and release audio resources
     */
    virtual void shutdown() = 0;

    /**
     * @brief Get the current sample rate
     */
    virtual int getSampleRate() const = 0;

    /**
     * @brief Get the number of channels
     */
    virtual int getChannelCount() const = 0;
};

} // namespace engine_sim

#endif // ENGINE_SIM_AUDIO_OUTPUT_INTERFACE_H