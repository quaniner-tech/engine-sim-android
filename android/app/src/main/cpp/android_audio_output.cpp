#include "android_audio_output.h"

#include <android/log.h>

#define LOG_TAG "AndroidAudioOutput"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace engine_sim {

AndroidAudioOutput::AndroidAudioOutput() {
    LOGI("AndroidAudioOutput constructed");
}

AndroidAudioOutput::~AndroidAudioOutput() {
    shutdown();
}

bool AndroidAudioOutput::initialize(int sampleRate, int channelCount, int bufferSizeFrames) {
    if (m_initialized) {
        LOGW("Already initialized, shutting down first");
        shutdown();
    }
    
    m_sampleRate = sampleRate;
    m_channelCount = channelCount;
    m_bufferSizeFrames = bufferSizeFrames;
    
    // Create and initialize AAudio engine
    m_aaudioEngine = std::make_unique<AAudioEngine>();
    
    if (!m_aaudioEngine->initialize(sampleRate, channelCount, bufferSizeFrames)) {
        LOGE("Failed to initialize AAudio engine");
        m_aaudioEngine.reset();
        return false;
    }
    
    m_aaudioEngine->setVolume(m_volume);
    m_initialized = true;
    
    LOGI("AndroidAudioOutput initialized: rate=%d, channels=%d, buffer=%d",
         sampleRate, channelCount, bufferSizeFrames);
    
    return true;
}

void AndroidAudioOutput::writeAudio(const int16_t* data, int sampleCount) {
    if (!m_aaudioEngine || !m_aaudioEngine->isActive()) {
        return;
    }
    
    m_aaudioEngine->writeAudio(data, sampleCount);
}

void AndroidAudioOutput::setVolume(float volume) {
    m_volume = volume;
    if (m_aaudioEngine) {
        m_aaudioEngine->setVolume(volume);
    }
}

bool AndroidAudioOutput::isActive() const {
    return m_initialized && m_aaudioEngine && m_aaudioEngine->isActive();
}

void AndroidAudioOutput::shutdown() {
    if (m_aaudioEngine) {
        m_aaudioEngine->shutdown();
        m_aaudioEngine.reset();
    }
    m_initialized = false;
    LOGI("AndroidAudioOutput shutdown complete");
}

int AndroidAudioOutput::getSampleRate() const {
    return m_sampleRate;
}

int AndroidAudioOutput::getChannelCount() const {
    return m_channelCount;
}

} // namespace engine_sim