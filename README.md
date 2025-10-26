# ESP32-SpeexDSP Library

A comprehensive library for audio processing on the ESP32 using SpeexDSP. It provides high-level and low-level APIs for tasks such as acoustic echo cancellation (AEC), noise suppression (NS), automatic gain control (AGC), voice activity detection (VAD), jitter buffering, resampling, ring buffering, G.711 codec, RTP parsing, and RMS calculation.
一个使用 SpeexDSP 在 ESP32 上进行音频处理的综合库。它为声学回声消除 (AEC)、噪声抑制 (NS)、自动增益控制 (AGC)、语音活动检测 (VAD)、抖动缓冲、重采样、环形缓冲、G.711 编解码器、RTP 解析和 RMS 计算等任务提供高级和低级 API。
## Features
- **声学回声消除 (AEC)：使用扬声器输出作为参考，消除麦克风输入的回声。Acoustic Echo Cancellation (AEC)**: Removes echo from microphone input using speaker output as a reference.
- **噪声抑制 (NS)：降低麦克风和扬声器音频中的背景噪音。Noise Suppression (NS)**: Reduces background noise in microphone and speaker audio.
- **自动增益控制 (AGC)：调整音频级别以保持一致的音量。Automatic Gain Control (AGC)**: Adjusts audio levels to maintain consistent volume.
- **语音活动检测 (VAD)：检测音频中是否存在语音。Voice Activity Detection (VAD)**: Detects whether voice is present in the audio.
- **抖动缓冲区：处理分组音频流中的时间问题。Jitter Buffer**: Handles timing issues in packetized audio streams.
- **重采样器：在可调节质量的不同采样率之间转换音频。Resampler**: Converts audio between different sample rates with adjustable quality.
- **环形缓冲区：有效地存储和检索音频样本。Ring Buffer**: Efficiently stores and retrieves audio samples.
- **G.711 编解码器：使用 u-law 或 A-law 格式对音频进行编码和解码。G.711 Codec**: Encodes and decodes audio using u-law or A-law formats.
- **RTP 解析：解析音频流的 RTP 数据包。RTP Parsing**: Parses RTP packets for audio streaming.
- **RMS 计算：计算音频数据的均方根以测量信号强度。RMS Calculation**: Computes the root mean square of audio data for signal strength measurement.


## Installation
1. 克隆或下载此存储库： Clone or download this repository:
   ```bash
   git clone https://github.com/rjsachse/ESP32-SpeexDSP.git
   ```
2. 将文件夹移动ESP32-SpeexDSP到您的 Arduino 库目录（例如~/Documents/Arduino/libraries/）。Move the `ESP32-SpeexDSP` folder to your Arduino libraries directory (e.g., `~/Documents/Arduino/libraries/`).
3. 打开 Arduino IDE，该库将在Sketch > Include Library下可用。Open Arduino IDE, and the library will be available under *Sketch > Include Library*.

## Usage
### High-Level API
The high-level API provides easy-to-use methods for common audio processing tasks.
高级 API
高级 API 为常见的音频处理任务提供了易于使用的方法。

声学回声消除 (AEC)

#### Acoustic Echo Cancellation (AEC)
```cpp
ESP32SpeexDSP dsp;
void setup() {
  dsp.beginAEC(256, 1024, 16000); // Initialize AEC with frame size, filter length, and sample rate
  dsp.enableAEC(true);            // Enable AEC
}

void loop() {
  int16_t mic[256], speaker[256], out[256];
  dsp.processAEC(mic, speaker, out); // Process audio with AEC
}
```

#### Noise Suppression (NS) 噪声抑制（NS）

```cpp
void setup() {
  dsp.beginMicPreprocess(256, 16000); // Initialize mic preprocessing
  dsp.enableMicNoiseSuppression(true); // Enable noise suppression
  dsp.setMicNoiseSuppressionLevel(-20); // Set suppression level in dB
}
```

#### Automatic Gain Control (AGC) 自动增益控制（AGC）

```cpp
void setup() {
  dsp.enableMicAGC(true, 0.9f); // Enable AGC with target level (0.0-1.0)
}
```

#### Voice Activity Detection (VAD) 语音活动检测 (VAD)

```cpp
void setup() {
  dsp.enableMicVAD(true); // Enable VAD
  dsp.setMicVADThreshold(50); // Set VAD threshold (0-100)
}

void loop() {
  if (dsp.isMicVoiceDetected()) {
    Serial.println("Voice detected!");
  }
}
```

#### Jitter Buffer 抖动缓冲器

```cpp
void setup() {
  dsp.beginJitterBuffer(20); // Initialize jitter buffer with 20ms step size
}

void loop() {
  int16_t packet[256];
  dsp.putJitterPacket(packet, 256, millis()); // Add packet to jitter buffer
  dsp.getJitterPacket(packet, 256);          // Retrieve packet from jitter buffer
}
```

#### Resampler 重采样器

```cpp
void setup() {
  dsp.beginResampler(16000, 8000, 5); // Initialize resampler (16kHz to 8kHz, quality 5)
}

void loop() {
  int16_t input[256], output[128];
  dsp.resample(input, 256, output, 128); // Resample audio
}
```

#### Ring Buffer 环形缓冲区

```cpp
void setup() {
  dsp.beginBuffer(1024); // Initialize ring buffer with size 1024 samples
}

void loop() {
  int16_t data[256];
  dsp.writeBuffer(data, 256); // Write data to ring buffer
  dsp.readBuffer(data, 256);  // Read data from ring buffer
}
```

#### G.711 Codec G.711编解码器

```cpp
void loop() {
  int16_t pcm[256];
  uint8_t g711[256];
  dsp.encodeG711(pcm, g711, 256, true);  // Encode PCM to G.711 (u-law)
  dsp.decodeG711(g711, pcm, 256, true); // Decode G.711 (u-law) to PCM
}
```

#### RTP Parsing RTP解析

```cpp
void loop() {
  uint8_t packet[512];
  ESP32SpeexDSP::RTPPacket rtp;
  if (dsp.parseRTPPacket(packet, sizeof(packet), rtp)) {
    Serial.println("RTP packet parsed successfully!");
  }
}
```

#### RMS Calculation 有效值计算

```cpp
void loop() {
  int16_t audio[256];
  float rms = dsp.computeRMS(audio, 256); // Compute RMS of audio data
  Serial.println(rms);
}
```

## Dependencies 依赖项

- None (SpeexDSP source is included in `src/speex/`).

## License
MIT License (see SpeexDSP licensing for included files).

## Authors
- **RjSachse (KookyMarvin)** 
- **Grok (xAI)** - Code assistance and optimization

## Acknowledgments
- Built with SpeexDSP, an open-source audio processing library.
- Thanks to xAI for providing Grok, an AI assistant that helped refine this library.
