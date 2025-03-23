#ifndef ESP32_SPEEXDSP_H
#define ESP32_SPEEXDSP_H

#include "config.h"                 // Configuration defines
#include "speex/speexdsp_types.h"   // Core type definitions
#include "speex/speex_echo.h"       // AEC
#include "speex/speex_preprocess.h" // NS, AGC, VAD, etc.
#include "speex/speex_jitter.h"     // Jitter Buffer
#include "speex/speex_resampler.h"  // Resampler
#include "speex/speex_buffer.h"     // Ring buffer
//#include "codecs/g7xx/g72x.h"        // G.711 codec
#include <stdint.h>                 // Standard types

#ifdef __cplusplus
extern "C" {
#endif
#include "codecs/g7xx/g72x.h" // Back in the game
#ifdef __cplusplus
}
#endif

class ESP32SpeexDSP {
public:
    ESP32SpeexDSP();
    ~ESP32SpeexDSP();

    // AEC
    bool beginAEC(int frameSize, int filterLength, int sampleRate, int channels = 1);
    void processAEC(int16_t *mic, int16_t *speaker, int16_t *out);
    SpeexEchoState* getEchoState();

    // Preprocessing
    bool beginPreprocess(int frameSize, int sampleRate);
    void preprocessAudio(int16_t *inOut);
    void enableNoiseSuppression(bool enable);
    void setNoiseSuppressionLevel(int dB);
    void enableAGC(bool enable, float targetLevel = 0.9f);
    void enableVAD(bool enable);
    bool isVoiceDetected();

    // Jitter Buffer
    bool beginJitterBuffer(int stepSizeMs);
    void putJitterPacket(int16_t *data, int len, int timestamp);
    int getJitterPacket(int16_t *out, int len);

    // Resampler
    bool beginResampler(int inputRate, int outputRate, int quality = 5);
    int resample(int16_t *in, int inLen, int16_t *out, int outLenMax);

    // Ring Buffer
    bool beginBuffer(int bufferSize);
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

private:
    SpeexEchoState *echoState;
    SpeexPreprocessState *preprocessState;
    JitterBuffer *jitterBuffer;
    SpeexResamplerState *resampler;
    SpeexBuffer *ringBuffer;
    int frameSize;
    int sampleRate;
    int jitterStepSize;
};

#endif