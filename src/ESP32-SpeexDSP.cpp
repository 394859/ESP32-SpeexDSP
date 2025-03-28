#include "ESP32-SpeexDSP.h"
#include <cstring>
#include <cmath>
#include <Arduino.h>

ESP32SpeexDSP::ESP32SpeexDSP() 
    : echoState(nullptr), preprocessState(nullptr), jitterBuffer(nullptr), 
      resampler(nullptr), ringBuffer(nullptr), frameSize(0), sampleRate(0), 
      jitterStepSize(0), aecEnabled(false), resamplerInputRate(0), 
      resamplerOutputRate(0), resamplerQuality(5) {}

ESP32SpeexDSP::~ESP32SpeexDSP() {
    if (echoState) speex_echo_state_destroy(echoState);
    if (preprocessState) speex_preprocess_state_destroy(preprocessState);
    if (jitterBuffer) jitter_buffer_destroy(jitterBuffer);
    if (resampler) speex_resampler_destroy(resampler);
    if (ringBuffer) speex_buffer_destroy(ringBuffer);
}

// AEC
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
    aecEnabled = true; // Enable by default
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

// Preprocessing
bool ESP32SpeexDSP::beginPreprocess(int frameSize, int sampleRate) {
    if (preprocessState) {
        speex_preprocess_state_destroy(preprocessState);
        preprocessState = nullptr;
    }
    this->frameSize = frameSize;
    this->sampleRate = sampleRate;
    preprocessState = speex_preprocess_state_init(frameSize, sampleRate);
    if (!preprocessState) return false;
    return true;
}

void ESP32SpeexDSP::preprocessAudio(int16_t *inOut) {
    if (preprocessState) {
        speex_preprocess_run(preprocessState, inOut);
    }
}

void ESP32SpeexDSP::enableNoiseSuppression(bool enable) {
    if (preprocessState) {
        int i = enable ? 1 : 0;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_DENOISE, &i);
    }
}

void ESP32SpeexDSP::setNoiseSuppressionLevel(int dB) {
    if (preprocessState) {
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &dB);
    }
}

void ESP32SpeexDSP::enableAGC(bool enable, float targetLevel) {
    if (preprocessState) {
        int i = enable ? 1 : 0;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_AGC, &i);
        if (enable) {
            float level = targetLevel * 32768.0f;
            speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_AGC_LEVEL, &level);
        }
    }
}

void ESP32SpeexDSP::enableVAD(bool enable) {
    if (preprocessState) {
        int i = enable ? 1 : 0;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_VAD, &i);
    }
}

void ESP32SpeexDSP::setVADThreshold(int probability) {
    if (preprocessState && probability >= 0 && probability <= 100) {
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_PROB_START, &probability);
    }
}

bool ESP32SpeexDSP::isVoiceDetected() {
    if (preprocessState) {
        int vad = 0;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_GET_VAD, &vad);
        return vad != 0;
    }
    return false;
}

// Jitter Buffer
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
        uint32_t in_len =植物inLen;
        uint32_t out_len = outLenMax;
        int err = speex_resampler_process_int(resampler, 0, in, &in_len, out, &out_len);
        if (err == 0) return out_len;
    }
    return 0;
}

// Ring Buffer
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

// G.711 Codec
void ESP32SpeexDSP::decodeG711(uint8_t* inG711, int16_t* out, int numSamples, bool ulaw) {
    for (int i = 0; i < numSamples; i++) {
        out[i] = ulaw ? ulaw2linear(inG711[i]) : alaw2linear(inG711[i]);
    }
}

void ESP32SpeexDSP::encodeG711(int16_t* in, uint8_t* outG711, int numSamples, bool ulaw) {
    for (int i = 0; i < numSamples; i++) {
        outG711[i] = ulaw ? linear2ulaw(in[i]) : linear2alaw(in[i]);
    }
}

// RTP Parsing
bool ESP32SpeexDSP::parseRTPPacket(uint8_t* packet, int packetLen, RTPPacket& rtp) {
    if (!packet || packetLen < 12) return false;

    rtp.version = (packet[0] >> 6) & 0x03;
    rtp.padding = (packet[0] & 0x20) != 0;
    rtp.extension = (packet[0] & 0x10) != 0;
    rtp.csrcCount = packet[0] & 0x0F;
    rtp.marker = (packet[1] & 0x80) != 0;
    rtp.payloadType = packet[1] & 0x7F;
    rtp.sequenceNumber = (packet[2] << 8) | packet[3];
    rtp.timestamp = (packet[4] << 24) | (packet[5] << 16) | (packet[6] << 8) | packet[7];
    rtp.ssrc = (packet[8] << 24) | (packet[9] << 16) | (packet[10] << 8) | packet[11];

    int headerLen = 12 + (rtp.csrcCount * 4);
    if (rtp.extension) {
        if (packetLen < headerLen + 4) return false;
        headerLen += 4 + ((packet[headerLen + 2] << 8) | packet[headerLen + 3]) * 4;
    }

    if (packetLen < headerLen) return false;
    rtp.payload = packet + headerLen;
    rtp.payloadLen = packetLen - headerLen;

    return rtp.version == 2;
}

// Utility
float ESP32SpeexDSP::computeRMS(int16_t *data, int len) {
    if (len <= 0) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < len; i++) {
        float sample = (float)data[i] / 32768.0f;
        sum += sample * sample;
    }
    return sqrtf(sum / len);
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

    if (preprocessState) {
        speex_preprocess_state_destroy(preprocessState);
        preprocessState = nullptr;
        preprocessState = speex_preprocess_state_init(oldFrameSize, sampleRate);
        if (!preprocessState) success = false;
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
    if (preprocessState) {
        speex_preprocess_state_destroy(preprocessState);
        preprocessState = speex_preprocess_state_init(newFrameSize, sampleRate);
        if (!preprocessState) success = false;
    }
    frameSize = newFrameSize;
    return success;
}