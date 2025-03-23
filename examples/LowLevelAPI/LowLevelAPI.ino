#include <ESP32-SpeexDSP.h>

#define SAMPLE_RATE 16000
#define FRAME_SIZE 256
#define FILTER_LENGTH 1024
#define BUFFER_SIZE 2048
#define JITTER_STEP_MS 20
#define RESAMPLE_RATE 8000

SpeexEchoState* echoState = nullptr;
SpeexPreprocessState* preprocessState = nullptr;
JitterBuffer* jitterBuffer = nullptr;
SpeexResamplerState* resamplerState = nullptr;
SpeexBuffer* bufferState = nullptr;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Starting Low-Level SpeexDSP Test...");

  // Low-level AEC initialization
  if (echoState) speex_echo_state_destroy(echoState);
  echoState = speex_echo_state_init(FRAME_SIZE, FILTER_LENGTH);
  if (echoState) {
    int sampleRate = SAMPLE_RATE;
    int result = speex_echo_ctl(echoState, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
    if (result == 0) {
      Serial.println("AEC initialized");
    } else {
      Serial.println("AEC sample rate setting failed!");
      while (1) delay(1000);
    }
  } else {
    Serial.println("AEC initialization failed!");
    while (1) delay(1000);
  }

  // Low-level preprocessing initialization
  if (preprocessState) speex_preprocess_state_destroy(preprocessState);
  preprocessState = speex_preprocess_state_init(FRAME_SIZE, SAMPLE_RATE);
  if (preprocessState) {
    int denoise = 1;
    speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_DENOISE, &denoise);
    int noiseSuppress = -15; // Less aggressive noise suppression
    speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noiseSuppress);
    int agc = 1;
    speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_AGC, &agc);
    int level = (int)(0.9f * 32768);
    speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_AGC_LEVEL, &level);
    int vad = 1;
    speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_VAD, &vad);
    Serial.println("Preprocessing initialized (denoise -15 dB, AGC, VAD)");
  } else {
    Serial.println("Preprocessing initialization failed!");
    while (1) delay(1000);
  }

  // Low-level jitter buffer initialization
  if (jitterBuffer) jitter_buffer_destroy(jitterBuffer);
  jitterBuffer = jitter_buffer_init(JITTER_STEP_MS);
  if (jitterBuffer) {
    Serial.println("Jitter buffer initialized");
  } else {
    Serial.println("Jitter buffer initialization failed!");
    while (1) delay(1000);
  }

  // Low-level resampler initialization
  if (resamplerState) speex_resampler_destroy(resamplerState);
  int err;
  resamplerState = speex_resampler_init(1, SAMPLE_RATE, RESAMPLE_RATE, 5, &err);
  if (resamplerState && err == 0) {
    Serial.println("Resampler initialized (16 kHz -> 8 kHz)");
  } else {
    Serial.println("Resampler initialization failed!");
    while (1) delay(1000);
  }

  // Low-level ring buffer initialization
  if (bufferState) speex_buffer_destroy(bufferState);
  bufferState = speex_buffer_init(BUFFER_SIZE);
  if (bufferState) {
    Serial.println("Ring buffer initialized");
  } else {
    Serial.println("Ring buffer initialization failed!");
    while (1) delay(1000);
  }

  testAudioProcessing();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("NS=")) {
      bool enable = input.substring(3).toInt() == 1;
      if (preprocessState) {
        int val = enable ? 1 : 0;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_DENOISE, &val);
        if (enable) {
          int noiseSuppress = -15;
          speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noiseSuppress);
        }
        Serial.println(enable ? "Noise suppression enabled (-15 dB)" : "Noise suppression disabled");
      }
    } else if (input.startsWith("AGC=")) {
      float level = input.substring(4).toFloat();
      if (level >= 0.0f && level <= 1.0f && preprocessState) {
        int val = 1;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_AGC, &val);
        int agcLevel = (int)(level * 32768);
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_AGC_LEVEL, &agcLevel);
        Serial.print("AGC target level set to: ");
        Serial.println(level, 2);
      } else {
        Serial.println("Invalid AGC level (0.0-1.0)");
      }
    } else if (input.startsWith("VAD=")) {
      bool enable = input.substring(4).toInt() == 1;
      if (preprocessState) {
        int val = enable ? 1 : 0;
        speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_VAD, &val);
        Serial.println(enable ? "VAD enabled" : "VAD disabled");
      }
    } else {
      Serial.println("Commands: NS=0/1, AGC=0.0-1.0, VAD=0/1");
    }
  }
  delay(100);
}

void generateSineWave(spx_int16_t* buffer, int length, float frequency, int sampleRate) {
  for (int i = 0; i < length; i++) {
    float t = (float)i / sampleRate;
    buffer[i] = (spx_int16_t)(32767.0f * sin(2.0f * M_PI * frequency * t));
  }
}

float computeRMS(spx_int16_t* audio, int len) {
  if (len <= 0) return 0.0f;
  float sum = 0;
  for (int i = 0; i < len; i++) {
    float sample = audio[i] / 32768.0f;
    sum += sample * sample;
  }
  return sqrtf(sum / len);
}

void testAudioProcessing() {
  spx_int16_t mic[FRAME_SIZE];
  spx_int16_t speaker[FRAME_SIZE];
  spx_int16_t aecOut[FRAME_SIZE];
  spx_int16_t preprocessed[FRAME_SIZE];
  spx_int16_t resampled[FRAME_SIZE / 2];
  spx_int16_t bufferOut[FRAME_SIZE];

  generateSineWave(mic, FRAME_SIZE, 440.0f, SAMPLE_RATE);
  generateSineWave(speaker, FRAME_SIZE, 880.0f, SAMPLE_RATE);
  Serial.println("Generated dummy audio data (mic: 440 Hz, speaker: 880 Hz)");

  if (echoState) {
    speex_echo_cancellation(echoState, mic, speaker, aecOut);
    float rmsAEC = computeRMS(aecOut, FRAME_SIZE);
    Serial.print("AEC RMS: ");
    Serial.println(rmsAEC, 6);
  }

  if (preprocessState) {
    memcpy(preprocessed, aecOut, FRAME_SIZE * sizeof(spx_int16_t));
    speex_preprocess_run(preprocessState, preprocessed);
    Serial.print("Preprocessed samples (first 5): ");
    for (int i = 0; i < 5 && i < FRAME_SIZE; i++) {
      Serial.print(preprocessed[i]);
      Serial.print(" ");
    }
    Serial.println();
    float rmsPreprocessed = computeRMS(preprocessed, FRAME_SIZE);
    Serial.print("Preprocessed RMS: ");
    Serial.println(rmsPreprocessed, 6);
    int vad;
    speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_GET_VAD, &vad);
    Serial.print("Voice detected: ");
    Serial.println(vad ? "Yes" : "No");
  }

  if (jitterBuffer) {
    spx_uint32_t timestamp = 0;
    JitterBufferPacket packet = {(char*)preprocessed, FRAME_SIZE * sizeof(spx_int16_t), timestamp, FRAME_SIZE, 0};
    jitter_buffer_put(jitterBuffer, &packet);
    Serial.println("Added packet to jitter buffer");

    JitterBufferPacket outPacket = {(char*)bufferOut, FRAME_SIZE * sizeof(spx_int16_t), 0, FRAME_SIZE, 0};
    spx_int32_t offset;
    int ret = jitter_buffer_get(jitterBuffer, &outPacket, 0, &offset);
    jitter_buffer_tick(jitterBuffer); // Added for proper timing
    int jitterOutSamples = (ret == JITTER_BUFFER_OK) ? outPacket.len / sizeof(spx_int16_t) : 0;
    Serial.print("Retrieved ");
    Serial.print(jitterOutSamples);
    Serial.println(" samples from jitter buffer");
    float rmsJitter = computeRMS(bufferOut, jitterOutSamples);
    Serial.print("Jitter buffer RMS: ");
    Serial.println(rmsJitter, 6);
  }

  if (resamplerState) {
    spx_uint32_t in_len = FRAME_SIZE, out_len = FRAME_SIZE / 2;
    speex_resampler_process_int(resamplerState, 0, preprocessed, &in_len, resampled, &out_len);
    Serial.print("Resampled ");
    Serial.print(out_len);
    Serial.println(" samples (16 kHz -> 8 kHz)");
    float rmsResampled = computeRMS(resampled, out_len);
    Serial.print("Resampled RMS: ");
    Serial.println(rmsResampled, 6);
  }

  if (bufferState) {
    speex_buffer_write(bufferState, (char*)preprocessed, FRAME_SIZE * sizeof(spx_int16_t));
    Serial.print("Wrote ");
    Serial.print(FRAME_SIZE);
    Serial.println(" samples to ring buffer");

    int bytesRead = speex_buffer_read(bufferState, (char*)bufferOut, FRAME_SIZE * sizeof(spx_int16_t));
    int bufferReadSamples = bytesRead / sizeof(spx_int16_t);
    Serial.print("Read ");
    Serial.print(bufferReadSamples);
    Serial.println(" samples from ring buffer");
    float rmsBuffer = computeRMS(bufferOut, bufferReadSamples);
    Serial.print("Ring buffer RMS: ");
    Serial.println(rmsBuffer, 6);
  }

  Serial.println("Test completed!");
}