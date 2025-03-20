#include <ESP32-SpeexDSP.h>

// Constants for audio processing
#define SAMPLE_RATE 16000       // 16 kHz sample rate
#define FRAME_SIZE 256         // Frame size for AEC and preprocessing
#define FILTER_LENGTH 1024     // Filter length for AEC
#define BUFFER_SIZE 2048       // Ring buffer size in bytes
#define JITTER_STEP_MS 20      // Jitter buffer step size (20 ms)
#define RESAMPLE_RATE 8000     // Resample to 8 kHz

// Instantiate the DSP object
ESP32SpeexDSP dsp;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Starting ESP32 SpeexDSP Test...");

  // Initialize AEC
  dsp.beginAEC(FRAME_SIZE, FILTER_LENGTH, SAMPLE_RATE);
  Serial.println("AEC initialized");

  // Initialize preprocessing
  dsp.beginPreprocess(FRAME_SIZE, SAMPLE_RATE);
  dsp.enableNoiseSuppression(true);
  dsp.enableAGC(true, 0.9f);  // Target level 0.9
  dsp.enableVAD(true);
  Serial.println("Preprocessing initialized (denoise, AGC, VAD)");

  // Initialize jitter buffer
  dsp.beginJitterBuffer(JITTER_STEP_MS);
  Serial.println("Jitter buffer initialized");

  // Initialize resampler (16 kHz -> 8 kHz)
  dsp.beginResampler(SAMPLE_RATE, RESAMPLE_RATE, 5);  // Quality 5
  Serial.println("Resampler initialized (16 kHz -> 8 kHz)");

  // Initialize ring buffer
  dsp.beginBuffer(BUFFER_SIZE);
  Serial.println("Ring buffer initialized");

  // Run the test
  testAudioProcessing();
}

void loop() {
  // Nothing to do in loop for this test
  delay(1000);
}

// Simulate audio data (sine wave for simplicity)
void generateSineWave(spx_int16_t* buffer, int length, float frequency, int sampleRate) {
  for (int i = 0; i < length; i++) {
    float t = (float)i / sampleRate;
    buffer[i] = (spx_int16_t)(32767.0f * sin(2.0f * M_PI * frequency * t));
  }
}

void testAudioProcessing() {
  // Buffers for testing
  spx_int16_t mic[FRAME_SIZE];
  spx_int16_t speaker[FRAME_SIZE];
  spx_int16_t aecOut[FRAME_SIZE];
  spx_int16_t preprocessed[FRAME_SIZE];
  spx_int16_t resampled[FRAME_SIZE / 2];  // Half size due to 16 kHz -> 8 kHz
  spx_int16_t bufferOut[FRAME_SIZE];

  // Step 1: Generate dummy audio data
  generateSineWave(mic, FRAME_SIZE, 440.0f, SAMPLE_RATE);      // 440 Hz for mic
  generateSineWave(speaker, FRAME_SIZE, 880.0f, SAMPLE_RATE);  // 880 Hz for speaker
  Serial.println("Generated dummy audio data (mic: 440 Hz, speaker: 880 Hz)");

  // Step 2: Test AEC
  dsp.processAEC(mic, speaker, aecOut);
  float rmsAEC = dsp.computeRMS(aecOut, FRAME_SIZE);
  Serial.print("AEC RMS: ");
  Serial.println(rmsAEC, 6);

  // Step 3: Test preprocessing
  memcpy(preprocessed, aecOut, FRAME_SIZE * sizeof(spx_int16_t));
  dsp.preprocessAudio(preprocessed);
  float rmsPreprocessed = dsp.computeRMS(preprocessed, FRAME_SIZE);
  Serial.print("Preprocessed RMS: ");
  Serial.println(rmsPreprocessed, 6);
  bool vad = dsp.isVoiceDetected();
  Serial.print("Voice detected: ");
  Serial.println(vad ? "Yes" : "No");

  // Step 4: Test jitter buffer
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

  // Step 5: Test resampling
  int resampledSamples = dsp.resample(preprocessed, FRAME_SIZE, resampled, FRAME_SIZE / 2);
  Serial.print("Resampled ");
  Serial.print(resampledSamples);
  Serial.println(" samples (16 kHz -> 8 kHz)");
  float rmsResampled = dsp.computeRMS(resampled, resampledSamples);
  Serial.print("Resampled RMS: ");
  Serial.println(rmsResampled, 6);

  // Step 6: Test ring buffer
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
