#include "ESP32-SpeexDSP.h"

// Include all SpeexDSP .c files directly
extern "C" {
#include "speex/buffer.c"
#include "speex/fftwrap.c"
#include "speex/filterbank.c"
#include "speex/jitter.c"
#include "speex/kiss_fft.c"
#include "speex/kiss_fftr.c"
#include "speex/mdf.c"
#include "speex/preprocess.c"
#include "speex/resample.c"
#include "speex/scal.c"
#include "speex/smallft.c"
}

ESP32SpeexDSP::ESP32SpeexDSP() : echoState(nullptr), preprocessState(nullptr), 
                                 jitterBuffer(nullptr), resamplerState(nullptr), 
                                 bufferState(nullptr) {}

ESP32SpeexDSP::~ESP32SpeexDSP() {
    if (echoState) speex_echo_state_destroy(echoState);
    if (preprocessState) speex_preprocess_state_destroy(preprocessState);
    if (jitterBuffer) jitter_buffer_destroy(jitterBuffer);
    if (resamplerState) speex_resampler_destroy(resamplerState);
    if (bufferState) speex_buffer_destroy(bufferState);
}

bool ESP32SpeexDSP::beginAEC(int frameSize, int filterLength, int sampleRate) {
    if (echoState) speex_echo_state_destroy(echoState);
    echoState = speex_echo_state_init(frameSize, filterLength);
    if (!echoState) return false;
    int result = speex_echo_ctl(echoState, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
    return result == 0;
}

void ESP32SpeexDSP::processAEC(spx_int16_t* mic, spx_int16_t* speaker, spx_int16_t* out) {
    if (echoState) speex_echo_cancellation(echoState, mic, speaker, out);
}

bool ESP32SpeexDSP::beginPreprocess(int frameSize, int sampleRate) {
    if (preprocessState) speex_preprocess_state_destroy(preprocessState);
    preprocessState = speex_preprocess_state_init(frameSize, sampleRate);
    return preprocessState != nullptr;
}

void ESP32SpeexDSP::enableNoiseSuppression(bool enable) {
    if (preprocessState) {
        int val = enable ? 1 : 0;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_DENOISE, &val);
    }
}

void ESP32SpeexDSP::enableAGC(bool enable, float targetLevel) {
    if (preprocessState) {
        int val = enable ? 1 : 0;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_AGC, &val);
        if (enable) {
            int level = (int)(targetLevel * 32768);  // Corrected to SPEEX_PREPROCESS_SET_AGC_LEVEL
            speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_AGC_LEVEL, &level);
        }
    }
}

void ESP32SpeexDSP::enableVAD(bool enable) {
    if (preprocessState) {
        int val = enable ? 1 : 0;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_VAD, &val);
    }
}

bool ESP32SpeexDSP::isVoiceDetected() {
    if (preprocessState) {
        int vad;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_GET_VAD, &vad);
        return vad != 0;
    }
    return false;
}

void ESP32SpeexDSP::preprocessAudio(spx_int16_t* audio) {
    if (preprocessState) speex_preprocess_run(preprocessState, audio);
}

bool ESP32SpeexDSP::beginJitterBuffer(int stepSizeMs) {
    if (jitterBuffer) jitter_buffer_destroy(jitterBuffer);
    jitterBuffer = jitter_buffer_init(stepSizeMs);
    return jitterBuffer != nullptr;
}

void ESP32SpeexDSP::putJitterPacket(spx_int16_t* data, int len, int timestamp) {
    if (jitterBuffer) {
        JitterBufferPacket packet = {(char*)data, len * sizeof(spx_int16_t), timestamp, len, 0};
        jitter_buffer_put(jitterBuffer, &packet);
    }
}

int ESP32SpeexDSP::getJitterPacket(spx_int16_t* data, int maxLen) {
    if (jitterBuffer) {
        JitterBufferPacket packet = {(char*)data, maxLen * sizeof(spx_int16_t), 0, maxLen, 0};
        spx_int32_t offset;
        int ret = jitter_buffer_get(jitterBuffer, &packet, 0, &offset);
        if (ret == JITTER_BUFFER_OK) return packet.len / sizeof(spx_int16_t);
    }
    return 0;
}

bool ESP32SpeexDSP::beginResampler(int inRate, int outRate, int quality) {
    if (resamplerState) speex_resampler_destroy(resamplerState);
    int err;
    resamplerState = speex_resampler_init(1, inRate, outRate, quality, &err);
    return resamplerState != nullptr && err == 0;
}

int ESP32SpeexDSP::resample(spx_int16_t* in, int inLen, spx_int16_t* out, int outLen) {
    if (resamplerState) {
        spx_uint32_t in_len = inLen, out_len = outLen;
        speex_resampler_process_int(resamplerState, 0, in, &in_len, out, &out_len);
        return out_len;
    }
    return 0;
}

bool ESP32SpeexDSP::beginBuffer(int maxSizeBytes) {
    if (bufferState) speex_buffer_destroy(bufferState);
    bufferState = speex_buffer_init(maxSizeBytes);
    return bufferState != nullptr;
}

void ESP32SpeexDSP::writeBuffer(spx_int16_t* data, int len) {
    if (bufferState) speex_buffer_write(bufferState, (char*)data, len * sizeof(spx_int16_t));
}

int ESP32SpeexDSP::readBuffer(spx_int16_t* data, int maxLen) {
    if (bufferState) {
        int bytesRead = speex_buffer_read(bufferState, (char*)data, maxLen * sizeof(spx_int16_t));
        return bytesRead / sizeof(spx_int16_t);
    }
    return 0;
}

float ESP32SpeexDSP::computeRMS(spx_int16_t* audio, int len) {
    if (len <= 0) return 0.0f;
    float sum = 0;
    for (int i = 0; i < len; i++) {
        float sample = audio[i] / 32768.0f;
        sum += sample * sample;
    }
    return sqrtf(sum / len);
}
