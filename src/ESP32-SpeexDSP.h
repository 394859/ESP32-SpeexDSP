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

    /** @brief Initialize Acoustic Echo Cancellation (AEC)
     *  @param frameSize Number of samples per frame (e.g., 256)
     *  @param filterLength Length of the echo tail filter (e.g., 1024)
     *  @param sampleRate Audio sampling rate in Hz (e.g., 16000)
     *  @return true if initialization succeeds, false otherwise
     */
    bool beginAEC(int frameSize, int filterLength, int sampleRate);

    /** @brief Process audio to remove echo
     *  @param mic Microphone input samples
     *  @param speaker Speaker output samples (echo reference)
     *  @param out Output buffer for echo-cancelled audio
     */
    void processAEC(spx_int16_t* mic, spx_int16_t* speaker, spx_int16_t* out);

    /** @brief Initialize audio preprocessing (denoise, AGC, VAD)
     *  @param frameSize Number of samples per frame
     *  @param sampleRate Audio sampling rate in Hz
     *  @return true if initialization succeeds, false otherwise
     */
    bool beginPreprocess(int frameSize, int sampleRate);

    /** @brief Enable or disable noise suppression
     *  @param enable true to enable, false to disable
     */
    void enableNoiseSuppression(bool enable);

    /** @brief Enable or disable Automatic Gain Control (AGC)
     *  @param enable true to enable, false to disable
     *  @param targetLevel Desired output level (0.0 to 1.0, default 0.9)
     */
    void enableAGC(bool enable, float targetLevel = 0.9f);

    /** @brief Enable or disable Voice Activity Detection (VAD)
     *  @param enable true to enable, false to disable
     */
    void enableVAD(bool enable);

    /** @brief Check if voice is detected in the last processed frame
     *  @return true if voice is detected, false otherwise
     */
    bool isVoiceDetected();

    /** @brief Apply preprocessing to an audio frame
     *  @param audio Input/output buffer for preprocessing
     */
    void preprocessAudio(spx_int16_t* audio);

    /** @brief Initialize jitter buffer for packetized audio
     *  @param stepSizeMs Time step per frame in milliseconds (e.g., 20)
     *  @return true if initialization succeeds, false otherwise
     */
    bool beginJitterBuffer(int stepSizeMs);

    /** @brief Add a packet to the jitter buffer
     *  @param data Audio samples
     *  @param len Number of samples
     *  @param timestamp Packet timestamp in milliseconds
     */
    void putJitterPacket(spx_int16_t* data, int len, int timestamp);

    /** @brief Retrieve a packet from the jitter buffer
     *  @param data Output buffer for samples
     *  @param maxLen Maximum samples to retrieve
     *  @return Number of samples retrieved
     */
    int getJitterPacket(spx_int16_t* data, int maxLen);

    /** @brief Initialize sample rate resampling
     *  @param inRate Input sample rate (e.g., 16000)
     *  @param outRate Output sample rate (e.g., 8000)
     *  @param quality Resampling quality (0-10, default 5)
     *  @return true if initialization succeeds, false otherwise
     */
    bool beginResampler(int inRate, int outRate, int quality = 5);

    /** @brief Resample audio data
     *  @param in Input samples
     *  @param inLen Number of input samples
     *  @param out Output buffer for resampled audio
     *  @param outLen Maximum output samples
     *  @return Number of output samples produced
     */
    int resample(spx_int16_t* in, int inLen, spx_int16_t* out, int outLen);

    /** @brief Initialize a ring buffer
     *  @param maxSizeBytes Maximum buffer size in bytes
     *  @return true if initialization succeeds, false otherwise
     */
    bool beginBuffer(int maxSizeBytes);

    /** @brief Write samples to the ring buffer
     *  @param data Audio samples
     *  @param len Number of samples
     */
    void writeBuffer(spx_int16_t* data, int len);

    /** @brief Read samples from the ring buffer
     *  @param data Output buffer for samples
     *  @param maxLen Maximum samples to read
     *  @return Number of samples read
     */
    int readBuffer(spx_int16_t* data, int maxLen);

    /** @brief Compute the Root Mean Square (RMS) of an audio buffer
     *  @param audio Input samples
     *  @param len Number of samples
     *  @return RMS value (0.0 to 1.0)
     */
    float computeRMS(spx_int16_t* audio, int len);

    // Low-level access
    SpeexEchoState* getEchoState() { return echoState; }
    SpeexPreprocessState* getPreprocessState() { return preprocessState; }
    JitterBuffer* getJitterBuffer() { return jitterBuffer; }
    SpeexResamplerState* getResamplerState() { return resamplerState; }
    SpeexBuffer* getBufferState() { return bufferState; }

private:
    SpeexEchoState* echoState;
    SpeexPreprocessState* preprocessState;
    JitterBuffer* jitterBuffer;
    SpeexResamplerState* resamplerState;
    SpeexBuffer* bufferState;
};

#endif
