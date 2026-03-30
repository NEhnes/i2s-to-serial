// This is the I2S transmitter node.
// It generates a sine wave and sends it to the receiver node over I2S.
#include <driver/i2s.h>
#include <Arduino.h>

#define I2S_BCK_PIN 26 // AKA Bit Clock (BCLK)
#define I2S_WS_PIN 25  // AKA Word Select (LRCLK)
#define I2S_DO_PIN 14  // AKA Data Out

// SINE STUFF
#define TABLE_SIZE 256          // Resolution of one full sine cycle (256-point lookup)
int32_t sine_table[TABLE_SIZE]; // 32-bit amplitude sine lookup table
int table_index = 0;            // Current position in the sine wave (wraps at TABLE_SIZE)

/**
 * @brief Initialize I2S interface and generate a sine wave LUT
 * Configures I2S in slave mode to transmit audio data to receiver.
 * The sine table is precomputed at 44.1kHz for efficient real-time playback.
 */
void setup() {
  Serial.begin(115200);

  // 1. Precompute sine table with 32-bit signed integer scaling
  for (int n = 0; n < TABLE_SIZE; n++) {
    // Scale sine wave to full 32-bit range: ±2147483647
    sine_table[n] = (int32_t)(sin(2.0 * PI * n / TABLE_SIZE) * 2147483647.0);
  }

  // 2. Configure I2S interface (slave mode, receives clock from receiver node)
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_SLAVE | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  // Stereo mode, though mono content
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,      // High-priority interrupt
    .dma_buf_count = 8,                            // Number of DMA buffers
    .dma_buf_len = 64,                             // Samples per buffer (32-bit = 256 bytes)
    .use_apll = false,                             // Don't use APLL (use internal clock)
    .tx_desc_auto_clear = true,                    // Auto-clear transmit descriptors
    .fixed_mclk = -1                               // Use default MCLK (ignored in slave mode)
  };

  // 3. Configure I2S pins
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK_PIN,  // Bit clock input from receiver
    .ws_io_num = I2S_WS_PIN,    // Word select (LRCLK) input from receiver
    .data_out_num = I2S_DO_PIN, // Data output to receiver
    .data_in_num = I2S_PIN_NO_CHANGE // No data input — this is TX only
  };

  // 4. Install and configure I2S driver
  i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_1, &pin_config);

  Serial.println("I2S sender ready — waiting for receiver to start clock");
}

/**
 * @brief Transmit precomputed sine wave samples over I2S
 * Continuously sends 128 samples (64 stereo pairs) per loop iteration.
 * The Sine LUT is indexed in sequence, creating a clean tone.
 */
void loop() {
  int32_t samples[128]; // Buffer for 128 samples (64 stereo frames)

  // Fill buffer from sine LUT
  for (int i = 0; i < 128; i++) {
    samples[i] = sine_table[table_index];
    table_index++; // Advance index
    if (table_index >= TABLE_SIZE) {
      table_index = 0; // Wrap to start of table
    }
  }

  size_t bytes_written = 0;
  i2s_write(I2S_NUM_1, samples, sizeof(samples), &bytes_written, portMAX_DELAY);
}
