#include <ESP32-SpeexDSP.h>
#include <driver/i2s.h>

#define FRAME_SIZE 256
#define SAMPLE_RATE 16000
#define BUFFER_SIZE (FRAME_SIZE * 2 * 10)

ESP32SpeexDSP dsp;

void setup() {
  Serial.begin(115200);
  i2s_config_t i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX,
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = FRAME_SIZE,
  };
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

  dsp.beginAEC(FRAME_SIZE, 1024, SAMPLE_RATE);
  dsp.beginPreprocess(FRAME_SIZE, SAMPLE_RATE);
  dsp.enableNoiseSuppression(true);
  dsp.enableAGC(true, 0.9);
  dsp.beginBuffer(BUFFER_SIZE);
}

void loop() {
  int16_t mic[FRAME_SIZE], speaker[FRAME_SIZE], out[FRAME_SIZE];
  i2s_read(I2S_NUM_0, mic, FRAME_SIZE * 2, NULL, portMAX_DELAY);
  i2s_read(I2S_NUM_0, speaker, FRAME_SIZE * 2, NULL, portMAX_DELAY);
  dsp.processAEC(mic, speaker, out);
  dsp.preprocessAudio(out);
  float rms = dsp.computeRMS(out, FRAME_SIZE);
  Serial.printf("RMS: %.3f\n", rms);
  dsp.writeBuffer(out, FRAME_SIZE);
  int16_t buffered[FRAME_SIZE];
  int samples = dsp.readBuffer(buffered, FRAME_SIZE);
  if (samples > 0) {
    i2s_write(I2S_NUM_0, buffered, samples * 2, NULL, portMAX_DELAY);
  }
}