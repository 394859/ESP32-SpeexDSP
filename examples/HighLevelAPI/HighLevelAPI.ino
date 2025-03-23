#include <ESP32-SpeexDSP.h>

#define SAMPLE_RATE 16000
#define FRAME_SIZE 256
#define FILTER_LENGTH 1024
#define BUFFER_SIZE 2048
#define JITTER_STEP_MS 20
#define RESAMPLE_RATE 8000

ESP32SpeexDSP dsp;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Starting ESP32 SpeexDSP Test...");

  if (!dsp.beginAEC(FRAME_SIZE, FILTER_LENGTH, SAMPLE_RATE)) {
    Serial.println("AEC initialization failed!");
    while (1) delay(1000);
  }
  Serial.println("AEC initialized");

  if (!dsp.beginPreprocess(FRAME_SIZE, SAMPLE_RATE)) {
    Serial.println("Preprocessing initialization failed!");
    while (1) delay(1000);
  }
  dsp.enableNoiseSuppression(true);
  dsp.setNoiseSuppressionLevel(-15); // Less aggressive NS
  Serial.println("Noise suppression enabled (-15 dB)");
  dsp.enableAGC(true, 0.9f);
  Serial.println("AGC enabled with target level 0.9");
  dsp.enableVAD(true);
  Serial.println("VAD enabled");
  Serial.println("Preprocessing initialized (denoise, AGC, VAD)");

  if (!dsp.beginJitterBuffer(JITTER_STEP_MS)) {
    Serial.println("Jitter buffer initialization failed!");
    while (1) delay(1000);
  }
  Serial.println("Jitter buffer initialized");

  if (!dsp.beginResampler(SAMPLE_RATE, RESAMPLE_RATE, 5)) {
    Serial.println("Resampler initialization failed!");
    while (1) delay(1000);
  }
  Serial.println("Resampler initialized (16 kHz -> 8 kHz)");

  if (!dsp.beginBuffer(BUFFER_SIZE)) {
    Serial.println("Ring buffer initialization failed!");
    while (1) delay(1000);
  }
  Serial.println("Ring buffer initialized");

  SpeexEchoState* rawEchoState = dsp.getEchoState();
  if (rawEchoState) {
    int rate = SAMPLE_RATE;
    speex_echo_ctl(rawEchoState, SPEEX_ECHO_SET_SAMPLING_RATE, &rate);
    Serial.println("Low-level AEC access successful");
  } else {
    Serial.println("Low-level AEC access failed");
  }

  testAudioProcessing();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("NS=")) {
      bool enable = input.substring(3).toInt() == 1;
      dsp.enableNoiseSuppression(enable);
      if (enable) dsp.setNoiseSuppressionLevel(-15); // Re-apply level
      Serial.println(enable ? "Noise suppression enabled (-15 dB)" : "Noise suppression disabled");
    } else if (input.startsWith("AGC=")) {
      float level = input.substring(4).toFloat();
      if (level >= 0.0f && level <= 1.0f) {
        dsp.enableAGC(true, level);
        Serial.print("AGC target level set to: ");
        Serial.println(level, 2);
      } else {
        Serial.println("Invalid AGC level (0.0-1.0)");
      }
    } else if (input.startsWith("VAD=")) {
      bool enable = input.substring(4).toInt() == 1;
      dsp.enableVAD(enable);
      Serial.println(enable ? "VAD enabled" : "VAD disabled");
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

  dsp.processAEC(mic, speaker, aecOut);
  float rmsAEC = dsp.computeRMS(aecOut, FRAME_SIZE);
  Serial.print("AEC RMS: ");
  Serial.println(rmsAEC, 6);

  // Test preprocessing with adjustments
  memcpy(preprocessed, aecOut, FRAME_SIZE * sizeof(spx_int16_t));
  dsp.preprocessAudio(preprocessed);
  Serial.print("Preprocessed samples (first 5): ");
  for (int i = 0; i < 5 && i < FRAME_SIZE; i++) {
    Serial.print(preprocessed[i]);
    Serial.print(" ");
  }
  Serial.println();
  float rmsPreprocessed = dsp.computeRMS(preprocessed, FRAME_SIZE);
  Serial.print("Preprocessed RMS (NS on, AGC 0.9, VAD on): ");
  Serial.println(rmsPreprocessed, 6);
  bool vad = dsp.isVoiceDetected();
  Serial.print("Voice detected: ");
  Serial.println(vad ? "Yes" : "No");

  // Adjust NS and retest
  dsp.enableNoiseSuppression(false);
  memcpy(preprocessed, aecOut, FRAME_SIZE * sizeof(spx_int16_t));
  dsp.preprocessAudio(preprocessed);
  float rmsNoNS = dsp.computeRMS(preprocessed, FRAME_SIZE);
  Serial.print("Preprocessed RMS (NS off): ");
  Serial.println(rmsNoNS, 6);
  dsp.enableNoiseSuppression(true);  // Restore
  dsp.setNoiseSuppressionLevel(-15); // Restore level

  // Adjust AGC and retest
  dsp.enableAGC(true, 0.5f);
  memcpy(preprocessed, aecOut, FRAME_SIZE * sizeof(spx_int16_t));
  dsp.preprocessAudio(preprocessed);
  float rmsAGC05 = dsp.computeRMS(preprocessed, FRAME_SIZE);
  Serial.print("Preprocessed RMS (AGC 0.5): ");
  Serial.println(rmsAGC05, 6);
  dsp.enableAGC(true, 0.9f);  // Restore

  int timestamp = 0;
  dsp.putJitterPacket(preprocessed, FRAME_SIZE, timestamp);
  Serial.println("Added packet to jitter buffer");
  int jitterOutSamples = dsp.getJitterPacket(bufferOut, FRAME_SIZE);
  Serial.print("Retrieved ");
  Serial.print(jitterOutSamples);
  Serial.println(" samples from jitter buffer");
  float rmsJitter = dsp.computeRMS(bufferOut, jitterOutSamples);
  Serial.print("Jitter buffer RMS: ");
  Serial.println(rmsJitter, 6);

  int resampledSamples = dsp.resample(preprocessed, FRAME_SIZE, resampled, FRAME_SIZE / 2);
  Serial.print("Resampled ");
  Serial.print(resampledSamples);
  Serial.println(" samples (16 kHz -> 8 kHz)");
  float rmsResampled = dsp.computeRMS(resampled, resampledSamples);
  Serial.print("Resampled RMS: ");
  Serial.println(rmsResampled, 6);

  dsp.writeBuffer(preprocessed, FRAME_SIZE);
  Serial.print("Wrote ");
  Serial.print(FRAME_SIZE);
  Serial.println(" samples to ring buffer");
  int bufferReadSamples = dsp.readBuffer(bufferOut, FRAME_SIZE);
  Serial.print("Read ");
  Serial.print(bufferReadSamples);
  Serial.println(" samples from ring buffer");
  float rmsBuffer = dsp.computeRMS(bufferOut, bufferReadSamples);
  Serial.print("Ring buffer RMS: ");
  Serial.println(rmsBuffer, 6);

  Serial.println("Test completed!");
}