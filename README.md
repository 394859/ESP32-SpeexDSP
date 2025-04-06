# ESP32-SpeexDSP Library

A comprehensive library for audio processing on the ESP32 using SpeexDSP. It provides high-level and low-level APIs for tasks such as acoustic echo cancellation (AEC), noise suppression (NS), automatic gain control (AGC), voice activity detection (VAD), jitter buffering, resampling, ring buffering, G.711 codec, RTP parsing, and RMS calculation.

## Features
- **Acoustic Echo Cancellation (AEC)**: Removes echo from microphone input using speaker output as a reference.
- **Noise Suppression (NS)**: Reduces background noise in microphone and speaker audio.
- **Automatic Gain Control (AGC)**: Adjusts audio levels to maintain consistent volume.
- **Voice Activity Detection (VAD)**: Detects whether voice is present in the audio.
- **Jitter Buffer**: Handles timing issues in packetized audio streams.
- **Resampler**: Converts audio between different sample rates with adjustable quality.
- **Ring Buffer**: Efficiently stores and retrieves audio samples.
- **G.711 Codec**: Encodes and decodes audio using u-law or A-law formats.
- **RTP Parsing**: Parses RTP packets for audio streaming.
- **RMS Calculation**: Computes the root mean square of audio data for signal strength measurement.

## Installation
1. Clone or download this repository:
   ```bash
   git clone https://github.com/rjsachse/ESP32-SpeexDSP.git
   ```
2. Move the `ESP32-SpeexDSP` folder to your Arduino libraries directory (e.g., `~/Documents/Arduino/libraries/`).
3. Open Arduino IDE, and the library will be available under *Sketch > Include Library*.

## Usage
### High-Level API
The high-level API provides easy-to-use methods for common audio processing tasks.

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

#### Noise Suppression (NS)
```cpp
void setup() {
  dsp.beginMicPreprocess(256, 16000); // Initialize mic preprocessing
  dsp.enableMicNoiseSuppression(true); // Enable noise suppression
  dsp.setMicNoiseSuppressionLevel(-20); // Set suppression level in dB
}
```

#### Automatic Gain Control (AGC)
```cpp
void setup() {
  dsp.enableMicAGC(true, 0.9f); // Enable AGC with target level (0.0-1.0)
}
```

#### Voice Activity Detection (VAD)
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

#### Jitter Buffer
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

#### Resampler
```cpp
void setup() {
  dsp.beginResampler(16000, 8000, 5); // Initialize resampler (16kHz to 8kHz, quality 5)
}

void loop() {
  int16_t input[256], output[128];
  dsp.resample(input, 256, output, 128); // Resample audio
}
```

#### Ring Buffer
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

#### G.711 Codec
```cpp
void loop() {
  int16_t pcm[256];
  uint8_t g711[256];
  dsp.encodeG711(pcm, g711, 256, true);  // Encode PCM to G.711 (u-law)
  dsp.decodeG711(g711, pcm, 256, true); // Decode G.711 (u-law) to PCM
}
```

#### RTP Parsing
```cpp
void loop() {
  uint8_t packet[512];
  ESP32SpeexDSP::RTPPacket rtp;
  if (dsp.parseRTPPacket(packet, sizeof(packet), rtp)) {
    Serial.println("RTP packet parsed successfully!");
  }
}
```

#### RMS Calculation
```cpp
void loop() {
  int16_t audio[256];
  float rms = dsp.computeRMS(audio, 256); // Compute RMS of audio data
  Serial.println(rms);
}
```

## Dependencies
- None (SpeexDSP source is included in `src/speex/`).

## License
MIT License (see SpeexDSP licensing for included files).

## Authors
- **RjSachse (KookyMarvin)** 
- **Grok (xAI)** - Code assistance and optimization

## Acknowledgments
- Built with SpeexDSP, an open-source audio processing library.
- Thanks to xAI for providing Grok, an AI assistant that helped refine this library.
