#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "usb_device_uac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/i2s_common.h"

#define TAG "UAC_PCM"

// PCM1808 pinleri (ESP32-S3)
#define PIN_BCK   1   // BCKI
#define PIN_WS    2   // LRCKI
#define PIN_MCLK  3   // SCKI
#define PIN_DIN   4   // DOUT (PCM -> ESP RX)

// Ses parametreleri
#define SAMPLE_RATE       48000
#define MIC_INTERVAL_MS   10

static i2s_chan_handle_t s_rx_chan = NULL;

static void pcm1808_i2s_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num  = 8;
    chan_cfg.dma_frame_num = 240;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &s_rx_chan));

    i2s_std_clk_config_t clk_cfg = {
        .sample_rate_hz = SAMPLE_RATE,
        .clk_src        = I2S_CLK_SRC_DEFAULT,
        .mclk_multiple  = I2S_MCLK_MULTIPLE_256,
    };

    i2s_std_slot_config_t slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO);
    slot_cfg.slot_mask      = I2S_STD_SLOT_LEFT;
    slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT;
    slot_cfg.data_bit_width = I2S_DATA_BIT_WIDTH_32BIT;
    slot_cfg.bit_shift      = false;

    i2s_std_gpio_config_t gpio_cfg = {
        .mclk = PIN_MCLK,
        .bclk = PIN_BCK,
        .ws   = PIN_WS,
        .dout = I2S_GPIO_UNUSED,
        .din  = PIN_DIN,
    };

    i2s_std_config_t std_cfg = {
        .clk_cfg  = clk_cfg,
        .slot_cfg = slot_cfg,
        .gpio_cfg = gpio_cfg,
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(s_rx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(s_rx_chan));
}

static esp_err_t uac_device_output_cb(uint8_t *buf, size_t len, void *cb_ctx)
{
    (void)buf; (void)len; (void)cb_ctx;
    return ESP_OK;
}

// PCM1808 -> 32 bit word -> 16 bit UAC (host'a gönder)
static esp_err_t uac_device_input_cb(uint8_t *buf, size_t len, size_t *bytes_read, void *cb_ctx)
{
    (void)cb_ctx;
    const size_t nsamp_16 = len / sizeof(int16_t);
    static int32_t i2s_tmp[1024];
    size_t need_words = nsamp_16;
    size_t got_words = 0;
    uint8_t *dst8 = (uint8_t *)i2s_tmp;

    while (got_words < need_words) {
        size_t to_read_bytes = (need_words - got_words) * sizeof(int32_t);
        size_t rcvd = 0;
        esp_err_t err = i2s_channel_read(s_rx_chan, dst8 + got_words * sizeof(int32_t), to_read_bytes, &rcvd, pdMS_TO_TICKS(20));
        if (err == ESP_OK && rcvd > 0) {
            got_words += rcvd / sizeof(int32_t);
        } else {
            break;
        }
    }

    int16_t *out16 = (int16_t *)buf;
    size_t produced = 0;
    for (; produced < got_words && produced < nsamp_16; produced++) {
        int32_t s32 = i2s_tmp[produced];
        out16[produced] = (int16_t)(s32 >> 8);
    }
    for (; produced < nsamp_16; produced++) {
        out16[produced] = 0;
    }

    *bytes_read = len;
    return ESP_OK;
}

static void uac_device_set_mute_cb(uint32_t mute, void *cb_ctx)
{
    (void)cb_ctx;
    ESP_LOGI(TAG, "Mute:%lu", (unsigned long)mute);
}
static void uac_device_set_volume_cb(uint32_t volume, void *cb_ctx)
{
    (void)cb_ctx;
    ESP_LOGI(TAG, "Volume:%lu", (unsigned long)volume);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Init I2S (PCM1808) @ %d Hz...", SAMPLE_RATE);
    pcm1808_i2s_init();

    ESP_LOGI(TAG, "Init USB UAC device...");
    uac_device_config_t cfg = {
        .skip_tinyusb_init = false,
        .output_cb         = uac_device_output_cb,
        .input_cb          = uac_device_input_cb,
        .set_mute_cb       = uac_device_set_mute_cb,
        .set_volume_cb     = uac_device_set_volume_cb,
        .cb_ctx            = NULL,
    };
    ESP_ERROR_CHECK(uac_device_init(&cfg));

    ESP_LOGI(TAG, "Ready: PC > Sound > Input: 'ESP USB Audio' (MIC). Dinlemeyi aç.");
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}
