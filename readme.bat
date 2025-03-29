@echo off
setlocal EnableDelayedExpansion

echo Generating README.md...

REM Clear existing README.md if it exists
if exist README.md del README.md

REM Write README.md line by line
echo # ESP32-SpeexDSP Library >> README.md
echo. >> README.md
echo A library for audio processing on the ESP32 using SpeexDSP, providing both high-level and low-level APIs for acoustic echo cancellation (AEC), noise suppression (NS), automatic gain control (AGC), voice activity detection (VAD), jitter buffering, resampling, and ring buffering. >> README.md
echo. >> README.md
echo ## Features >> README.md
echo - **High-Level API**: Easy-to-use class methods for common audio processing tasks. >> README.md
echo - **Low-Level API**: Direct access to SpeexDSP structures for advanced control. >> README.md
echo - **Components**: >> README.md
echo   - **AEC**: Removes echo from microphone input, toggleable on/off. >> README.md
echo   - **Preprocessing**: Separate NS, AGC, and VAD for mic and speaker paths. >> README.md
echo   - **Jitter Buffer**: Handles packetized audio timing. >> README.md
echo   - **Resampler**: Converts between sample rates (e.g., 16 kHz to 8 kHz), adjustable quality. >> README.md
echo   - **Ring Buffer**: Stores and retrieves audio samples, resizable. >> README.md
echo   - **RMS Calculation**: Measures audio signal strength. >> README.md
echo. >> README.md
echo ## Installation >> README.md
echo 1. Clone or download this repository: >> README.md
echo    ```bash >> README.md
echo    git clone https://github.com/rjsachse/ESP32-SpeexDSP.git >> README.md
echo    ``` >> README.md
echo 2. Move the `ESP32-SpeexDSP` folder to your Arduino libraries directory (e.g., `~/Documents/Arduino/libraries/`). >> README.md
echo 3. Open Arduino IDE, and the library will be available under *Sketch ^> Include Library*. >> README.md
echo. >> README.md
echo ## Examples >> README.md
echo - **HighLevelAPI**: Demonstrates the `ESP32SpeexDSP` class with serial commands to adjust settings (`NS=`, `AGC=`, `VAD=`). >> README.md
echo - **LowLevelAPI**: Uses raw SpeexDSP functions for the same functionality, bypassing the class. >> README.md
echo. >> README.md
echo ### Running Examples >> README.md
echo 1. Open an example via *File ^> Examples ^> ESP32-SpeexDSP ^> [HighLevelAPI or LowLevelAPI]*. >> README.md
echo 2. Upload to an ESP32 board. >> README.md
echo 3. Open Serial Monitor (115200 baud) to view logs and send commands (HighLevelAPI only). >> README.md
echo. >> README.md
echo ## Usage >> README.md
echo ### High-Level API >> README.md
echo #### Basic Setup >> README.md
echo ```cpp >> README.md
echo ESP32SpeexDSP dsp; >> README.md
echo void setup() { >> README.md
echo   dsp.beginAEC(256, 1024, 16000);           // AEC: 256-sample frame, 1024-sample filter >> README.md
echo   dsp.beginMicPreprocess(256, 16000);       // Mic preprocessing >> README.md
echo   dsp.beginSpeakerPreprocess(256, 16000);   // Speaker preprocessing >> README.md
echo } >> README.md
echo ``` >> README.md
echo. >> README.md
echo #### Dynamic Settings >> README.md
echo ```cpp >> README.md
echo void loop() { >> README.md
echo   // AEC >> README.md
echo   dsp.enableAEC(true);                     // Enable/disable AEC >> README.md
echo. >> README.md
echo   // Mic Preprocessing >> README.md
echo   dsp.enableMicNoiseSuppression(true);     // Enable NS for mic >> README.md
echo   dsp.setMicNoiseSuppressionLevel(-20);    // NS level (-10 to -40 dB) >> README.md
echo   dsp.enableMicAGC(true, 0.75f);           // Enable AGC for mic, target 75%% >> README.md
echo   dsp.enableMicVAD(true);                  // Enable VAD for mic >> README.md
echo   dsp.setMicVADThreshold(80);              // VAD sensitivity (0-100) >> README.md
echo. >> README.md
echo   // Speaker Preprocessing >> README.md
echo   dsp.enableSpeakerNoiseSuppression(false); // Disable NS for speaker >> README.md
echo   dsp.setSpeakerNoiseSuppressionLevel(-15); // NS level if enabled >> README.md
echo   dsp.enableSpeakerAGC(true, 0.9f);        // Enable AGC for speaker, target 90%% >> README.md
echo. >> README.md
echo   // Other Adjustments >> README.md
echo   dsp.setSampleRate(8000, 128, 256);       // Change sample rate (reinitializes) >> README.md
echo   dsp.setFrameSize(128);                   // Change frame size (reinitializes) >> README.md
echo   dsp.setResamplerQuality(7);              // Resampler quality (0-10) >> README.md
echo   dsp.resizeBuffer(2048);                  // Resize ring buffer (samples) >> README.md
echo } >> README.md
echo ``` >> README.md
echo. >> README.md
echo #### Processing Audio >> README.md
echo ```cpp >> README.md
echo int16_t mic[256], speaker[256], out[256]; >> README.md
echo dsp.processAEC(mic, speaker, out);         // Apply AEC if enabled >> README.md
echo dsp.preprocessMicAudio(out);               // Mic NS/AGC/VAD >> README.md
echo dsp.preprocessSpeakerAudio(speaker);       // Speaker NS/AGC >> README.md
echo ``` >> README.md
echo. >> README.md
echo ### Low-Level API >> README.md
echo ```cpp >> README.md
echo SpeexEchoState* echoState; >> README.md
echo SpeexPreprocessState* micPreprocess; >> README.md
echo void setup() { >> README.md
echo   echoState = speex_echo_state_init(256, 1024); >> README.md
echo   micPreprocess = speex_preprocess_state_init(256, 16000); >> README.md
echo   int rate = 16000; >> README.md
echo   speex_echo_ctl(echoState, SPEEX_ECHO_SET_SAMPLING_RATE, ^&rate); >> README.md
echo } >> README.md
echo ``` >> README.md
echo. >> README.md
echo ## Settings >> README.md
echo - **AEC**: `enableAEC(bool)` toggles echo cancellation runtime. >> README.md
echo - **Mic Preprocessing**: >> README.md
echo   - `enableMicNoiseSuppression(bool)`: On/off NS. >> README.md
echo   - `setMicNoiseSuppressionLevel(int)`: -10 to -40 dB. >> README.md
echo   - `enableMicAGC(bool, float)`: On/off AGC, target 0.0-1.0. >> README.md
echo   - `enableMicVAD(bool)`: On/off VAD. >> README.md
echo   - `setMicVADThreshold(int)`: Sensitivity 0-100 (higher = stricter). >> README.md
echo - **Speaker Preprocessing**: >> README.md
echo   - `enableSpeakerNoiseSuppression(bool)`: On/off NS. >> README.md
echo   - `setSpeakerNoiseSuppressionLevel(int)`: -10 to -40 dB. >> README.md
echo   - `enableSpeakerAGC(bool, float)`: On/off AGC, target 0.0-1.0. >> README.md
echo - **Dynamic Adjustments** (reinitialize components): >> README.md
echo   - `setSampleRate(int, int, int)`: Sample rate, AEC frame, filter length. >> README.md
echo   - `setFrameSize(int)`: Frame size for AEC and preprocess. >> README.md
echo   - `setResamplerQuality(int)`: Resampler quality 0-10. >> README.md
echo   - `resizeBuffer(int)`: Ring buffer size in samples. >> README.md
echo. >> README.md
echo ## Dependencies >> README.md
echo - None (SpeexDSP source is included in `src/speex/`). >> README.md
echo. >> README.md
echo ## License >> README.md
echo MIT License (see SpeexDSP licensing for included files). >> README.md
echo. >> README.md
echo ## Authors >> README.md
echo - **RjSachse (KookyMarvin)** >> README.md
echo - **Grok (xAI)** - Code assistance and optimization >> README.md
echo. >> README.md
echo ## Acknowledgments >> README.md
echo - Built with SpeexDSP, an open-source audio processing library. >> README.md
echo - Thanks to xAI for providing Grok, an AI assistant that helped refine this library. >> README.md

echo README.md has been generated successfully!
pause