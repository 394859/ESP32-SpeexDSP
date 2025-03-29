# ESP32-SpeexDSP Library 
 
A library for audio processing on the ESP32 using SpeexDSP, providing both high-level and low-level APIs for acoustic echo cancellation (AEC), noise suppression (NS), automatic gain control (AGC), voice activity detection (VAD), jitter buffering, resampling, and ring buffering. 
 
## Features 
- **High-Level API**: Easy-to-use class methods for common audio processing tasks. 
- **Low-Level API**: Direct access to SpeexDSP structures for advanced control. 
- **Components**: 
  - **AEC**: Removes echo from microphone input, toggleable on/off. 
  - **Preprocessing**: Separate NS, AGC, and VAD for mic and speaker paths. 
  - **Jitter Buffer**: Handles packetized audio timing. 
  - **Resampler**: Converts between sample rates (e.g., 16 kHz to 8 kHz), adjustable quality. 
  - **Ring Buffer**: Stores and retrieves audio samples, resizable. 
  - **RMS Calculation**: Measures audio signal strength. 
 
## Installation 
1. Clone or download this repository: 
   ```bash 
   git clone https://github.com/rjsachse/ESP32-SpeexDSP.git 
   ``` 
2. Move the `ESP32-SpeexDSP` folder to your Arduino libraries directory (e.g., `~/Documents/Arduino/libraries/`). 
3. Open Arduino IDE, and the library will be available under *Sketch > Include Library*. 
 
## Examples 
- **HighLevelAPI**: Demonstrates the `ESP32SpeexDSP` class with serial commands to adjust settings (`NS=`, `AGC=`, `VAD=`). 
- **LowLevelAPI**: Uses raw SpeexDSP functions for the same functionality, bypassing the class. 
 
### Running Examples 
1. Open an example via *File > Examples > ESP32-SpeexDSP > [HighLevelAPI or LowLevelAPI]*. 
2. Upload to an ESP32 board. 
3. Open Serial Monitor (115200 baud) to view logs and send commands (HighLevelAPI only). 
 
## Usage 
### High-Level API 
#### Basic Setup 
```cpp 
ESP32SpeexDSP dsp; 
void setup() { 
  dsp.beginAEC(256, 1024, 16000);           // AEC: 256-sample frame, 1024-sample filter 
  dsp.beginMicPreprocess(256, 16000);       // Mic preprocessing 
  dsp.beginSpeakerPreprocess(256, 16000);   // Speaker preprocessing 
} 
``` 
 
#### Dynamic Settings 
```cpp 
void loop() { 
  // AEC 
  dsp.enableAEC(true);                     // Enable/disable AEC 
 
  // Mic Preprocessing 
  dsp.enableMicNoiseSuppression(true);     // Enable NS for mic 
  dsp.setMicNoiseSuppressionLevel(-20);    // NS level (-10 to -40 dB) 
  dsp.enableMicAGC(true, 0.75f);           // Enable AGC for mic, target 75% 
  dsp.enableMicVAD(true);                  // Enable VAD for mic 
  dsp.setMicVADThreshold(80);              // VAD sensitivity (0-100) 
 
  // Speaker Preprocessing 
  dsp.enableSpeakerNoiseSuppression(false); // Disable NS for speaker 
  dsp.setSpeakerNoiseSuppressionLevel(-15); // NS level if enabled 
  dsp.enableSpeakerAGC(true, 0.9f);        // Enable AGC for speaker, target 90% 
 
  // Other Adjustments 
  dsp.setSampleRate(8000, 128, 256);       // Change sample rate (reinitializes) 
  dsp.setFrameSize(128);                   // Change frame size (reinitializes) 
  dsp.setResamplerQuality(7);              // Resampler quality (0-10) 
  dsp.resizeBuffer(2048);                  // Resize ring buffer (samples) 
} 
``` 
 
#### Processing Audio 
```cpp 
int16_t mic[256], speaker[256], out[256]; 
dsp.processAEC(mic, speaker, out);         // Apply AEC if enabled 
dsp.preprocessMicAudio(out);               // Mic NS/AGC/VAD 
dsp.preprocessSpeakerAudio(speaker);       // Speaker NS/AGC 
``` 
 
### Low-Level API 
```cpp 
SpeexEchoState* echoState; 
SpeexPreprocessState* micPreprocess; 
void setup() { 
  echoState = speex_echo_state_init(256, 1024); 
  micPreprocess = speex_preprocess_state_init(256, 16000); 
  int rate = 16000; 
  speex_echo_ctl(echoState, SPEEX_ECHO_SET_SAMPLING_RATE, &rate); 
} 
``` 
 
## Settings 
- **AEC**: `enableAEC(bool)` toggles echo cancellation runtime. 
- **Mic Preprocessing**: 
  - `enableMicNoiseSuppression(bool)`: On/off NS. 
  - `setMicNoiseSuppressionLevel(int)`: -10 to -40 dB. 
  - `enableMicAGC(bool, float)`: On/off AGC, target 0.0-1.0. 
  - `enableMicVAD(bool)`: On/off VAD. 
  - `setMicVADThreshold(int)`: Sensitivity 0-100 (higher = stricter). 
- **Speaker Preprocessing**: 
  - `enableSpeakerNoiseSuppression(bool)`: On/off NS. 
  - `setSpeakerNoiseSuppressionLevel(int)`: -10 to -40 dB. 
  - `enableSpeakerAGC(bool, float)`: On/off AGC, target 0.0-1.0. 
- **Dynamic Adjustments** (reinitialize components): 
  - `setSampleRate(int, int, int)`: Sample rate, AEC frame, filter length. 
  - `setFrameSize(int)`: Frame size for AEC and preprocess. 
  - `setResamplerQuality(int)`: Resampler quality 0-10. 
  - `resizeBuffer(int)`: Ring buffer size in samples. 
 
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
