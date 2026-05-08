#ifndef ANDROID_AUDIO_OUTPUT_H
#define ANDROID_AUDIO_OUTPUT_H

#include "engine_sim/AudioOutputInterface.h"
#include "aaudio_engine.h"

#include <memory>

namespace engine_sim {

/**
 * @brief Android audio output implementation
 * 
 * Implements AudioOutputInterface using AAudio for Android 8.0+
 * and provides fallback capability for older versions.
 */
class AndroidAudioOutput : public AudioOutputInterface {
public:
    AndroidAudioOutput();
    ~AndroidAudioOutput() override;

    // AudioOutputInterface implementation
    bool initialize(int sampleRate, int channelCount, int bufferSizeFrames) override;
    void writeAudio(const int16_t* data, int sampleCount) override;
    void setVolume(float volume) override;
    bool isActive() const override;
    void shutdown() override;
    int getSampleRate() const override;
    int getChannelCount() const override;

private:
    std::unique_ptr<AAudioEngine> m_aaudioEngine;
    int m_sampleRate = 44100;
    int m_channelCount = 1;
    int m_bufferSizeFrames = 512;
    float m_volume = 1.0f;
    bool m_initialized = false;
};

} // namespace engine_sim

#endif // ANDROID_AUDIO_OUTPUT_H