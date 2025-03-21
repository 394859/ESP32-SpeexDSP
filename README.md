# ESP32-SpeexDSP Library 
 
A library for audio processing on the ESP32 using SpeexDSP, providing both high-level and low-level APIs for acoustic echo cancellation (AEC), noise suppression, automatic gain control (AGC), voice activity detection (VAD), jitter buffering, resampling, and ring buffering. 
 
## Features 
- **High-Level API**: Easy-to-use class methods for common audio processing tasks. 
- **Low-Level API**: Direct access to SpeexDSP structures for advanced control. 
- **Components**: 
  - AEC: Removes echo from microphone input. 
  - Preprocessing: Noise suppression, AGC, VAD. 
  - Jitter Buffer: Handles packetized audio timing. 
  - Resampler: Converts between sample rates (e.g., 16 kHz to 8 kHz). 
  - Ring Buffer: Stores and retrieves audio samples. 
  - RMS Calculation: Measures audio signal strength. 
 
## Installation 
1. Clone or download this repository: 
   ```bash 
   git clone https://github.com/rjsachse/ESP32-SpeexDSP.git 
   ``` 
2. Move the `ESP32-SpeexDSP` folder to your Arduino libraries directory (e.g., `~/Documents/Arduino/libraries/`). 
3. Open Arduino IDE, and the library will be available under *Sketch  Library*. 
 
## Examples 
- **HighLevelAPI**: Demonstrates the `ESP32SpeexDSP` class with serial commands to adjust settings (`NS=`, `AGC=`, `VAD=`). 
- **LowLevelAPI**: Uses raw SpeexDSP functions for the same functionality, bypassing the class. 
 
### Running Examples 
1. Open an example via *File  or LowLevelAPI]*. 
2. Upload to an ESP32 board. 
3. Open Serial Monitor (115200 baud) to view logs and send commands (HighLevelAPI only). 
 
## Usage 
### High-Level API 
```cpp 
ESP32SpeexDSP dsp; 
void setup() { 
  dsp.beginAEC(256, 1024, 16000); 
  dsp.beginPreprocess(256, 16000); 
  dsp.enableNoiseSuppression(true); 
} 
``` 
 
### Low-Level API 
```cpp 
SpeexEchoState* echoState; 
void setup() { 
  echoState = speex_echo_state_init(256, 1024); 
  int rate = 16000; 
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
