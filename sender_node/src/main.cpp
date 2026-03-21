// ts was vibecoded as fuck

#include <driver/i2s.h>
#include <Arduino.h>

#define I2S_BCK_PIN 26 // AKA Bit Clock (BCLK)
#define I2S_WS_PIN 25 // AKA Word Select (LRCLK)
#define I2S_DO_PIN 14

// SINE STUFF
#define TABLE_SIZE 256          // The resolution of one sine cycle
int32_t sine_table[TABLE_SIZE]; // The lookup table
int table_index = 0;            // Keeps track of our position in the wave

void setup() {
  Serial.begin(115200);

  // 1. Fill the LUT with one full sine cycle (32-bit amplitude)
  for (int n = 0; n < TABLE_SIZE; n++) {
    // 2147483647 is the max value for a signed 32-bit int
    sine_table[n] = (int32_t)(sin(2.0 * PI * n / TABLE_SIZE) * 2147483647.0);
  }

  // I2S config
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_SLAVE | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = -1
  };

  // Pin config
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_DO_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_1, &pin_config);

  Serial.println("I2S sender ready");
}

void loop() {
// Send simple alternating pattern
int32_t samples[128];

for (int i = 0; i < 128; i++) {
    // Fill buffer from the Sine LUT
    samples[i] = sine_table[table_index];
    
    // Increment and wrap the index to stay within the table bounds
    table_index++;
    if (table_index >= TABLE_SIZE) {
        table_index = 0; 
    }
}

  size_t bytes_written = 0;
  i2s_write(I2S_NUM_1, samples, sizeof(samples), &bytes_written, portMAX_DELAY);
}