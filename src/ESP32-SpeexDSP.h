#ifndef ESP32_SPEEXDSP_H
#define ESP32_SPEEXDSP_H

#include "config.h"
#include "speex/speexdsp_types.h"
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"
#include "speex/speex_jitter.h"
#include "speex/speex_resampler.h"
#include "speex/speex_buffer.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "codecs/g7xx/g72x.h"
#ifdef __cplusplus
}
#endif

class ESP32SpeexDSP {
public:
    ESP32SpeexDSP();
    ~ESP32SpeexDSP();

    // AEC
    bool beginAEC(int frameSize, int filterLength, int sampleRate, int channels = 1);
    void enableAEC(bool enable);
    void processAEC(int16_t *mic, int16_t *speaker, int16_t *out);
    SpeexEchoState* getEchoState();

    // Preprocessing - Mic
    bool beginMicPreprocess(int frameSize, int sampleRate);
    void preprocessMicAudio(int16_t *inOut); // Mic-specific
    void enableMicNoiseSuppression(bool enable);
    void setMicNoiseSuppressionLevel(int dB);
    void enableMicAGC(bool enable, float targetLevel = 0.9f);
    void enableMicVAD(bool enable);
    void setMicVADThreshold(int probability);
    bool isMicVoiceDetected();

    // Preprocessing - Speaker
    bool beginSpeakerPreprocess(int frameSize, int sampleRate);
    void preprocessSpeakerAudio(int16_t *inOut); // Speaker-specific
    void enableSpeakerNoiseSuppression(bool enable);
    void setSpeakerNoiseSuppressionLevel(int dB);
    void enableSpeakerAGC(bool enable, float targetLevel = 0.9f);

    // Jitter Buffer
    bool beginJitterBuffer(int stepSizeMs);
    void putJitterPacket(int16_t *data, int len, int timestamp);
    int getJitterPacket(int16_t *out, int len);

    // Resampler
    bool beginResampler(int inputRate, int outputRate, int quality = 5);
    void setResamplerQuality(int quality);
    int resample(int16_t *in, int inLen, int16_t *out, int outLenMax);

    // Ring Buffer
    bool beginBuffer(int bufferSize);
    bool resizeBuffer(int newBufferSize);
    void writeBuffer(int16_t *data, int len);
    int readBuffer(int16_t *out, int len);

    // G.711 Codec
    void decodeG711(uint8_t* inG711, int16_t* out, int numSamples, bool ulaw = true);
    void encodeG711(int16_t* in, uint8_t* outG711, int numSamples, bool ulaw = true);

    // RTP Parsing
    struct RTPPacket {
        uint8_t version;
        bool padding;
        bool extension;
        uint8_t csrcCount;
        bool marker;
        uint8_t payloadType;
        uint16_t sequenceNumber;
        uint32_t timestamp;
        uint32_t ssrc;
        uint8_t* payload;
        int payloadLen;
    };
    bool parseRTPPacket(uint8_t* packet, int packetLen, RTPPacket& rtp);

    // Utility
    float computeRMS(int16_t *data, int len);
    bool setSampleRate(int newSampleRate, int aecFrameSize = 0, int aecFilterLength = 0);
    bool setFrameSize(int newFrameSize);

private:
    SpeexEchoState *echoState;
    SpeexPreprocessState *micPreprocessState; // Mic-specific
    SpeexPreprocessState *speakerPreprocessState; // Speaker-specific
    JitterBuffer *jitterBuffer;
    SpeexResamplerState *resampler;
    SpeexBuffer *ringBuffer;
    int frameSize;
    int sampleRate;
    int jitterStepSize;
    bool aecEnabled;
    int resamplerInputRate;
    int resamplerOutputRate;
    int resamplerQuality;
};

#endif