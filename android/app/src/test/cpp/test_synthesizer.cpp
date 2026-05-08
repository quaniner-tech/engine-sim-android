#include <gtest/gtest.h>
#include <engine_sim/synthesizer.h>
#include <cstring>

using namespace engine_sim;

TEST(SynthesizerTest, Construction) {
    Synthesizer synth;
    // No crash on construction
}

TEST(SynthesizerTest, Initialize) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);
    // No crash
}

TEST(SynthesizerTest, InitializeDualChannel) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 2;
    params.inputBufferSize = 512;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);
    // No crash
}

TEST(SynthesizerTest, Destroy) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);
    synth.destroy();
    // No crash
}

TEST(SynthesizerTest, ReadAudioOutputEmpty) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);

    // Read from empty buffer
    int16_t buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));
    int samplesRead = synth.readAudioOutput(1024, buffer);

    // May return 0 if no audio has been rendered yet
    EXPECT_GE(samplesRead, 0);
    EXPECT_LE(samplesRead, 1024);
}

TEST(SynthesizerTest, ReadAudioOutputPartial) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);

    int16_t buffer[256];
    int samplesRead = synth.readAudioOutput(256, buffer);
    EXPECT_GE(samplesRead, 0);
    EXPECT_LE(samplesRead, 256);
}

TEST(SynthesizerTest, ReadAudioOutputLargerThanBuffer) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);

    // Request more than available
    int16_t buffer[1024];
    int samplesRead = synth.readAudioOutput(1024, buffer);
    EXPECT_GE(samplesRead, 0);
}

TEST(SynthesizerTest, SampleRates) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);

    EXPECT_DOUBLE_EQ(synth.getInputSampleRate(), 10000.0);

    synth.setInputSampleRate(20000.0);
    EXPECT_DOUBLE_EQ(synth.getInputSampleRate(), 20000.0);
}

TEST(SynthesizerTest, AudioParameters) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;

    Synthesizer::AudioParameters audioParams;
    audioParams.volume = 0.8f;
    audioParams.convolution = 0.5f;
    audioParams.dF_F_mix = 0.02f;
    audioParams.inputSampleNoise = 0.3f;
    audioParams.airNoise = 0.7f;
    audioParams.levelerTarget = 25000.0f;
    audioParams.levelerMaxGain = 2.0f;
    audioParams.levelerMinGain = 0.0001f;
    params.initialAudioParameters = audioParams;

    synth.initialize(params);

    auto retrieved = synth.getAudioParameters();
    EXPECT_FLOAT_EQ(retrieved.volume, 0.8f);
    EXPECT_FLOAT_EQ(retrieved.convolution, 0.5f);
    EXPECT_FLOAT_EQ(retrieved.dF_F_mix, 0.02f);
}

TEST(SynthesizerTest, SetAudioParameters) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);

    Synthesizer::AudioParameters newParams;
    newParams.volume = 0.5f;
    newParams.convolution = 0.3f;
    newParams.dF_F_mix = 0.005f;
    newParams.inputSampleNoise = 0.1f;
    newParams.airNoise = 0.2f;
    newParams.levelerTarget = 20000.0f;
    newParams.levelerMaxGain = 1.5f;
    newParams.levelerMinGain = 0.001f;

    synth.setAudioParameters(newParams);

    auto retrieved = synth.getAudioParameters();
    EXPECT_FLOAT_EQ(retrieved.volume, 0.5f);
    EXPECT_FLOAT_EQ(retrieved.levelerTarget, 20000.0f);
}

TEST(SynthesizerTest, Latency) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);

    double latency = synth.getLatency();
    EXPECT_GE(latency, 0.0);
}

TEST(SynthesizerTest, ImpulseResponseInitialization) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);

    // Create a simple impulse response (just a few samples)
    int16_t impulse[64];
    for (int i = 0; i < 64; ++i) {
        impulse[i] = (i == 0) ? 32767 : 0; // unit impulse
    }

    // Should not crash
    synth.initializeImpulseResponse(impulse, 64, 1.0f, 0);

    synth.destroy();
}

TEST(SynthesizerTest, InputWrite) {
    Synthesizer synth;
    Synthesizer::Parameters params;
    params.inputChannelCount = 1;
    params.inputBufferSize = 1024;
    params.audioBufferSize = 44100;
    params.inputSampleRate = 10000;
    params.audioSampleRate = 44100;
    synth.initialize(params);

    double inputData[256];
    for (int i = 0; i < 256; ++i) {
        inputData[i] = 0.5;
    }

    synth.writeInput(inputData);
    synth.endInputBlock();

    // Should not crash
}

TEST(SynthesizerTest, InputDelta) {
    // With no input, delta should be 0
    EXPECT_EQ(synth.inputDelta(100, 50), 50);
    EXPECT_EQ(synth.inputDelta(50, 100), -50);
}

TEST(SynthesizerTest, InputDistance) {
    double dist = synth.inputDistance(1.0, 0.5);
    EXPECT_DOUBLE_EQ(dist, 0.5);
}