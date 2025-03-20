#ifndef ESP32_SPEEXDSP_H
#define ESP32_SPEEXDSP_H

#include <Arduino.h>
#define HAVE_CONFIG_H 1
#include "speex/config.h"

extern "C" {
#include "speex/speex_buffer.h"
#include "speex/speex_echo.h"
#include "speex/speex_jitter.h"
#include "speex/speex_preprocess.h"
#include "speex/speex_resampler.h"
}

class ESP32SpeexDSP {
public:
    ESP32SpeexDSP();
    ~ESP32SpeexDSP();

    void beginAEC(int frameSize, int filterLength, int sampleRate);
    void processAEC(spx_int16_t* mic, spx_int16_t* speaker, spx_int16_t* out);

    void beginPreprocess(int frameSize, int sampleRate);
    void enableNoiseSuppression(bool enable);
    void enableAGC(bool enable, float targetLevel = 0.9f);
    void enableVAD(bool enable);
    bool isVoiceDetected();
    void preprocessAudio(spx_int16_t* audio);

    void beginJitterBuffer(int stepSizeMs);
    void putJitterPacket(spx_int16_t* data, int len, int timestamp);
    int getJitterPacket(spx_int16_t* data, int maxLen);

    void beginResampler(int inRate, int outRate, int quality = 5);
    int resample(spx_int16_t* in, int inLen, spx_int16_t* out, int outLen);

    void beginBuffer(int maxSizeBytes);
    void writeBuffer(spx_int16_t* data, int len);
    int readBuffer(spx_int16_t* data, int maxLen);

    float computeRMS(spx_int16_t* audio, int len);

private:
    SpeexEchoState* echoState;
    SpeexPreprocessState* preprocessState;
    JitterBuffer* jitterBuffer;
    SpeexResamplerState* resamplerState;
    SpeexBuffer* bufferState;
};

#endif
