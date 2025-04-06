#include <ESP32-SpeexDSP.h>

#define SAMPLE_RATE 16000
#define FRAME_SIZE 256

ESP32SpeexDSP dsp;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Starting RTP Example...");

  // Initialize basic preprocessing for payload
  if (!dsp.beginMicPreprocess(FRAME_SIZE, SAMPLE_RATE)) {
    Serial.println("Preprocessing failed!");
    while (1) delay(1000);
  }
  dsp.enableMicNoiseSuppression(true);
  dsp.setMicNoiseSuppressionLevel(-15);
  Serial.println("Preprocessing initialized");

  testRTPProcessing();
}

void loop() {
  delay(1000);
}

/** @brief Generate dummy RTP packet with audio payload */
void generateDummyRTP(uint8_t* packet, int16_t* audio, int audioLen) {
  packet[0] = 0x80; // Version 2, no padding/extension
  packet[1] = 0x00; // Payload type 0 (PCMU), no marker
  packet[2] = 0x00; packet[3] = 0x01; // Sequence number
  packet[4] = 0x00; packet[5] = 0x00; packet[6] = 0x00; packet[7] = 0x01; // Timestamp
  packet[8] = 0x12; packet[9] = 0x34; packet[10] = 0x56; packet[11] = 0x78; // SSRC
  dsp.encodeG711(audio, packet + 12, audioLen, true); // u-law payload
}

/** @brief Generate sine wave audio */
void generateSineWave(int16_t* buffer, int length, float frequency, int sampleRate) {
  for (int i = 0; i < length; i++) {
    float t = (float)i / sampleRate;
    buffer[i] = (int16_t)(32767.0f * sin(2.0f * M_PI * frequency * t));
  }
}

/** @brief Test RTP parsing and processing */
void testRTPProcessing() {
  // Generate dummy audio and RTP packet
  int16_t audio[FRAME_SIZE];
  generateSineWave(audio, FRAME_SIZE, 440.0f, SAMPLE_RATE);
  uint8_t rtpPacket[12 + FRAME_SIZE]; // Header + payload
  generateDummyRTP(rtpPacket, audio, FRAME_SIZE);

  // Parse RTP packet
  ESP32SpeexDSP::RTPPacket rtp;
  if (dsp.parseRTPPacket(rtpPacket, sizeof(rtpPacket), rtp)) {
    Serial.println("RTP Packet Parsed:");
    Serial.print("  Version: "); Serial.println(rtp.version);
    Serial.print("  Payload Type: "); Serial.println(rtp.payloadType);
    Serial.print("  Sequence: "); Serial.println(rtp.sequenceNumber);
    Serial.print("  Timestamp: "); Serial.println(rtp.timestamp);
    Serial.print("  SSRC: "); Serial.println(rtp.ssrc);
    Serial.print("  Payload Length: "); Serial.println(rtp.payloadLen);
  } else {
    Serial.println("RTP parsing failed!");
    return;
  }

  // Decode G.711 payload
  int16_t decoded[FRAME_SIZE];
  dsp.decodeG711(rtp.payload, decoded, FRAME_SIZE, true);
  Serial.print("Decoded RMS: ");
  Serial.println(dsp.computeRMS(decoded, FRAME_SIZE), 6);

  // Process decoded audio
  dsp.preprocessMicAudio(decoded);
  Serial.print("Processed RMS: ");
  Serial.println(dsp.computeRMS(decoded, FRAME_SIZE), 6);

  Serial.println("RTP test completed!");
}