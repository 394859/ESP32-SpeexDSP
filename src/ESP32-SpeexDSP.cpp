#include "ESP32-SpeexDSP.h"
#include <cstring>
#include <cmath>
#include <Arduino.h>

ESP32SpeexDSP::ESP32SpeexDSP() 
    : echoState(nullptr), micPreprocessState(nullptr), speakerPreprocessState(nullptr), 
      jitterBuffer(nullptr), resampler(nullptr), ringBuffer(nullptr), frameSize(0), 
      sampleRate(0), jitterStepSize(0), aecEnabled(false), resamplerInputRate(0), 
      resamplerOutputRate(0), resamplerQuality(5) {}

ESP32SpeexDSP::~ESP32SpeexDSP() {
    if (echoState) speex_echo_state_destroy(echoState);
    if (micPreprocessState) speex_preprocess_state_destroy(micPreprocessState);
    if (speakerPreprocessState) speex_preprocess_state_destroy(speakerPreprocessState);
    if (jitterBuffer) jitter_buffer_destroy(jitterBuffer);
    if (resampler) speex_resampler_destroy(resampler);
    if (ringBuffer) speex_buffer_destroy(ringBuffer);
}

// AEC (unchanged)
bool ESP32SpeexDSP::beginAEC(int frameSize, int filterLength, int sampleRate, int channels) {
    if (echoState) {
        speex_echo_state_destroy(echoState);
        echoState = nullptr;
    }
    this->frameSize = frameSize;
    this->sampleRate = sampleRate;
    echoState = speex_echo_state_init_mc(frameSize, filterLength, channels, channels);
    if (!echoState) return false;
    speex_echo_ctl(echoState, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
    aecEnabled = true;
    return true;
}

void ESP32SpeexDSP::enableAEC(bool enable) {
    aecEnabled = enable;
}

void ESP32SpeexDSP::processAEC(int16_t *mic, int16_t *speaker, int16_t *out) {
    if (echoState && aecEnabled) {
        speex_echo_cancellation(echoState, mic, speaker, out);
    } else {
        memcpy(out, mic, frameSize * sizeof(int16_t));
    }
}

SpeexEchoState* ESP32SpeexDSP::getEchoState() {
    return echoState;
}

// Preprocessing - Mic (unchanged)
bool ESP32SpeexDSP::beginMicPreprocess(int frameSize, int sampleRate) {
    if (micPreprocessState) {
        speex_preprocess_state_destroy(micPreprocessState);
        micPreprocessState = nullptr;
    }
    this->frameSize = frameSize;
    this->sampleRate = sampleRate;
    micPreprocessState = speex_preprocess_state_init(frameSize, sampleRate);
    return micPreprocessState != nullptr;
}

void ESP32SpeexDSP::preprocessMicAudio(int16_t *inOut) {
    if (micPreprocessState) {
        speex_preprocess_run(micPreprocessState, inOut);
    }
}

void ESP32SpeexDSP::enableMicNoiseSuppression(bool enable) {
    if (micPreprocessState) {
        int i = enable ? 1 : 0;
        speex_preprocess_ctl(micPreprocessState, SPEEX_PREPROCESS_SET_DENOISE, &i);
    }
}

void ESP32SpeexDSP::setMicNoiseSuppressionLevel(int dB) {
    if (micPreprocessState) {
        speex_preprocess_ctl(micPreprocessState, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &dB);
    }
}

void ESP32SpeexDSP::enableMicAGC(bool enable, float targetLevel) {
    if (micPreprocessState) {
        int i = enable ? 1 : 0;
        speex_preprocess_ctl(micPreprocessState, SPEEX_PREPROCESS_SET_AGC, &i);
        if (enable) {
            float level = targetLevel * 32768.0f;
            speex_preprocess_ctl(micPreprocessState, SPEEX_PREPROCESS_SET_AGC_LEVEL, &level);
        }
    }
}

void ESP32SpeexDSP::enableMicVAD(bool enable) {
    if (micPreprocessState) {
        int i = enable ? 1 : 0;
        speex_preprocess_ctl(micPreprocessState, SPEEX_PREPROCESS_SET_VAD, &i);
    }
}

void ESP32SpeexDSP::setMicVADThreshold(int probability) {
    if (micPreprocessState && probability >= 0 && probability <= 100) {
        speex_preprocess_ctl(micPreprocessState, SPEEX_PREPROCESS_SET_PROB_START, &probability);
    }
}

bool ESP32SpeexDSP::isMicVoiceDetected() {
    if (micPreprocessState) {
        int vad = 0;
        speex_preprocess_ctl(micPreprocessState, SPEEX_PREPROCESS_GET_VAD, &vad);
        return vad != 0;
    }
    return false;
}

// Preprocessing - Speaker (unchanged)
bool ESP32SpeexDSP::beginSpeakerPreprocess(int frameSize, int sampleRate) {
    if (speakerPreprocessState) {
        speex_preprocess_state_destroy(speakerPreprocessState);
        speakerPreprocessState = nullptr;
    }
    this->frameSize = frameSize;
    this->sampleRate = sampleRate;
    speakerPreprocessState = speex_preprocess_state_init(frameSize, sampleRate);
    return speakerPreprocessState != nullptr;
}

void ESP32SpeexDSP::preprocessSpeakerAudio(int16_t *inOut) {
    if (speakerPreprocessState) {
        speex_preprocess_run(speakerPreprocessState, inOut);
    }
}

void ESP32SpeexDSP::enableSpeakerNoiseSuppression(bool enable) {
    if (speakerPreprocessState) {
        int i = enable ? 1 : 0;
        speex_preprocess_ctl(speakerPreprocessState, SPEEX_PREPROCESS_SET_DENOISE, &i);
    }
}

void ESP32SpeexDSP::setSpeakerNoiseSuppressionLevel(int dB) {
    if (speakerPreprocessState) {
        speex_preprocess_ctl(speakerPreprocessState, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &dB);
    }
}

void ESP32SpeexDSP::enableSpeakerAGC(bool enable, float targetLevel) {
    if (speakerPreprocessState) {
        int i = enable ? 1 : 0;
        speex_preprocess_ctl(speakerPreprocessState, SPEEX_PREPROCESS_SET_AGC, &i);
        if (enable) {
            float level = targetLevel * 32768.0f;
            speex_preprocess_ctl(speakerPreprocessState, SPEEX_PREPROCESS_SET_AGC_LEVEL, &level);
        }
    }
}

// Jitter Buffer (unchanged)
bool ESP32SpeexDSP::beginJitterBuffer(int stepSizeMs) {
    jitterStepSize = (sampleRate * stepSizeMs) / 1000;
    jitterBuffer = jitter_buffer_init(jitterStepSize);
    return jitterBuffer != nullptr;
}

void ESP32SpeexDSP::putJitterPacket(int16_t *data, int len, int timestamp) {
    if (jitterBuffer) {
        JitterBufferPacket packet;
        packet.data = (char*)data;
        packet.len = len * sizeof(int16_t);
        packet.timestamp = timestamp;
        packet.span = jitterStepSize;
        jitter_buffer_put(jitterBuffer, &packet);
    }
}

int ESP32SpeexDSP::getJitterPacket(int16_t *out, int len) {
    if (jitterBuffer) {
        JitterBufferPacket packet;
        packet.data = (char*)out;
        packet.len = len * sizeof(int16_t);
        int ret = jitter_buffer_get(jitterBuffer, &packet, jitterStepSize, nullptr);
        jitter_buffer_tick(jitterBuffer);
        if (ret == JITTER_BUFFER_OK) {
            return packet.len / sizeof(int16_t);
        }
    }
    return 0;
}

// Resampler
bool ESP32SpeexDSP::beginResampler(int inputRate, int outputRate, int quality) {
    if (resampler) {
        speex_resampler_destroy(resampler);
        resampler = nullptr;
    }
    int err = 0;
    resampler = speex_resampler_init(1, inputRate, outputRate, quality, &err);
    resamplerInputRate = inputRate;
    resamplerOutputRate = outputRate;
    resamplerQuality = quality;
    return resampler != nullptr && err == 0;
}

void ESP32SpeexDSP::setResamplerQuality(int quality) {
    if (resampler && quality >= 0 && quality <= 10) {
        speex_resampler_destroy(resampler);
        int err = 0;
        resampler = speex_resampler_init(1, resamplerInputRate, resamplerOutputRate, quality, &err);
        resamplerQuality = quality;
    }
}

int ESP32SpeexDSP::resample(int16_t *in, int inLen, int16_t *out, int outLenMax) {
    if (resampler) {
        uint32_t in_len = inLen;
        uint32_t out_len = outLenMax;
        int err = speex_resampler_process_int(resampler, 0, in, &in_len, out, &out_len);
        if (err == 0) return out_len;
    }
    return 0;
}

// Ring Buffer (unchanged)
bool ESP32SpeexDSP::beginBuffer(int bufferSize) {
    if (ringBuffer) {
        speex_buffer_destroy(ringBuffer);
        ringBuffer = nullptr;
    }
    ringBuffer = speex_buffer_init(bufferSize * sizeof(int16_t));
    return ringBuffer != nullptr;
}

bool ESP32SpeexDSP::resizeBuffer(int newBufferSize) {
    if (ringBuffer) {
        speex_buffer_destroy(ringBuffer);
        ringBuffer = speex_buffer_init(newBufferSize * sizeof(int16_t));
        return ringBuffer != nullptr;
    }
    return false;
}

void ESP32SpeexDSP::writeBuffer(int16_t *data, int len) {
    if (ringBuffer) {
        speex_buffer_write(ringBuffer, (char*)data, len * sizeof(int16_t));
    }
}

int ESP32SpeexDSP::readBuffer(int16_t *out, int len) {
    if (ringBuffer) {
        int bytesRead = speex_buffer_read(ringBuffer, (char*)out, len * sizeof(int16_t));
        return bytesRead / sizeof(int16_t);
    }
    return 0;
}

bool ESP32SpeexDSP::setSampleRate(int newSampleRate, int aecFrameSize, int aecFilterLength) {
    bool success = true;
    int oldFrameSize = frameSize;
    int oldSampleRate = sampleRate;

    sampleRate = newSampleRate;

    if (echoState) {
        int channels = 1; // Default, could store as member
        speex_echo_state_destroy(echoState);
        echoState = nullptr;
        echoState = speex_echo_state_init_mc(aecFrameSize ? aecFrameSize : oldFrameSize,
                                             aecFilterLength ? aecFilterLength : oldFrameSize * 2,
                                             channels, channels);
        if (echoState) {
            speex_echo_ctl(echoState, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
        } else {
            success = false;
        }
    }

    // Update both mic and speaker preprocessing states
    if (micPreprocessState) {
        speex_preprocess_state_destroy(micPreprocessState);
        micPreprocessState = nullptr;
        micPreprocessState = speex_preprocess_state_init(oldFrameSize, sampleRate);
        if (!micPreprocessState) success = false;
    }
    if (speakerPreprocessState) {
        speex_preprocess_state_destroy(speakerPreprocessState);
        speakerPreprocessState = nullptr;
        speakerPreprocessState = speex_preprocess_state_init(oldFrameSize, sampleRate);
        if (!speakerPreprocessState) success = false;
    }

    if (jitterBuffer) {
        jitterStepSize = (sampleRate * jitterStepSize) / oldSampleRate;
        jitter_buffer_destroy(jitterBuffer);
        jitterBuffer = jitter_buffer_init(jitterStepSize);
        if (!jitterBuffer) success = false;
    }

    return success;
}

bool ESP32SpeexDSP::setFrameSize(int newFrameSize) {
    bool success = true;
    if (echoState) {
        int channels = 1;
        int filterLength = newFrameSize * 2; // Example scaling
        speex_echo_state_destroy(echoState);
        echoState = speex_echo_state_init_mc(newFrameSize, filterLength, channels, channels);
        if (echoState) {
            speex_echo_ctl(echoState, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
        } else {
            success = false;
        }
    }
    // Update both mic and speaker preprocessing states
    if (micPreprocessState) {
        speex_preprocess_state_destroy(micPreprocessState);
        micPreprocessState = speex_preprocess_state_init(newFrameSize, sampleRate);
        if (!micPreprocessState) success = false;
    }
    if (speakerPreprocessState) {
        speex_preprocess_state_destroy(speakerPreprocessState);
        speakerPreprocessState = speex_preprocess_state_init(newFrameSize, sampleRate);
        if (!speakerPreprocessState) success = false;
    }
    frameSize = newFrameSize;
    return success;
}