// WARNING:  This file contains code that is more than likely already 
// exposed from the Esp32 Arduino API.  It will be removed once integration is complete.
//
// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#if defined(ARDUINO_ARCH_ESP32) 

#include "sdkconfig.h" // this sets useful config symbols, like CONFIG_IDF_TARGET_ESP32C3

// ESP32 C3 & S3 I2S is not supported yet due to significant changes to interface
#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)



#include <string.h>
#include <stdio.h>
#include "stdlib.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"


#if ESP_IDF_VERSION_MAJOR>=4
#include "esp_intr_alloc.h"
#else
#include "esp_intr.h"
#endif

#include "rom/lldesc.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"
#include "soc/io_mux_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/i2s_struct.h"
#if defined(CONFIG_IDF_TARGET_ESP32)
/* included here for ESP-IDF v4.x compatibility */
#include "soc/dport_reg.h"
#endif
#include "soc/sens_reg.h"
#include "driver/gpio.h"
#include "driver/i2s.h"

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
#include "driver/dac.h"
#endif

#include "Esp32_i2s.h"
#include "esp32-hal.h"

esp_err_t i2sSetClock(uint8_t bus_num, uint8_t div_num, uint8_t div_b, uint8_t div_a, uint8_t bck, uint8_t bits_per_sample);
esp_err_t i2sSetSampleRate(uint8_t bus_num, uint32_t sample_rate, bool parallel_mode, size_t bytes_per_sample);

#define MATRIX_DETACH_OUT_SIG 0x100

#if ESP_IDF_VERSION_MAJOR<=4
#define I2S_BASE_CLK (160000000L)
#endif

#define I2S_DMA_BLOCK_COUNT_DEFAULT      0
// 20 bytes gives us enough time if we use single stage idle
// But it can't be longer due to non-parrallel mode and 50us reset time
// there just isn't enough silence at the end to fill more than 20 bytes
#define I2S_DMA_SILENCE_SIZE     20 // 4 byte increments
#define I2S_DMA_SILENCE_BLOCK_COUNT_FRONT  2 // two front
#define I2S_DMA_SILENCE_BLOCK_COUNT_BACK  1 // one back, required for non parallel

typedef struct 
{
    i2s_dev_t* bus;
    int8_t  ws;
    int8_t  bck;
    int8_t  out;
    int8_t  in;

    intr_handle_t isr_handle;
    lldesc_t* dma_items;
    size_t dma_count;

    volatile uint32_t is_sending_data;
} i2s_bus_t;

// is_sending_data values
#define I2s_Is_Idle 0
#define I2s_Is_Pending 1
#define I2s_Is_Sending 2

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)
// (I2S_NUM_MAX == 2)
static i2s_bus_t I2S[I2S_NUM_MAX] = 
{
    {&I2S0, -1, -1, -1, -1, NULL, NULL, I2S_DMA_BLOCK_COUNT_DEFAULT, I2s_Is_Idle},
    {&I2S1, -1, -1, -1, -1, NULL, NULL, I2S_DMA_BLOCK_COUNT_DEFAULT, I2s_Is_Idle}
};
#else
static i2s_bus_t I2S[I2S_NUM_MAX] = 
{
    {&I2S0, -1, -1, -1, -1, NULL, NULL, I2S_DMA_BLOCK_COUNT_DEFAULT, I2s_Is_Idle}
};
#endif

void IRAM_ATTR i2sDmaISR(void* arg);

inline void dmaItemInit(lldesc_t* item, uint8_t* posData, size_t sizeData, lldesc_t* itemNext)
{
    item->eof = 0;
    item->owner = 1;
    item->sosf = 0;
    item->offset = 0;
    item->buf = posData;
    item->size = sizeData;
    item->length = sizeData;
    item->qe.stqe_next = itemNext;
}

bool i2sInitDmaItems(uint8_t bus_num, uint8_t* data, size_t dataSize)
{
    if (bus_num >= I2S_NUM_MAX) 
    {
        return false;
    }

    size_t dmaCount = I2S[bus_num].dma_count;

    if (I2S[bus_num].dma_items == NULL) 
    {
        I2S[bus_num].dma_items = (lldesc_t*)heap_caps_malloc(dmaCount * sizeof(lldesc_t), MALLOC_CAP_DMA);
        if (I2S[bus_num].dma_items == NULL) 
        {
            log_e("MEM ERROR!");
            return false;
        }
    }

    lldesc_t* itemFirst = &I2S[bus_num].dma_items[0];
    lldesc_t* item = itemFirst;
//    lldesc_t* itemsEnd = itemFirst + I2S[bus_num].dma_count;
    lldesc_t* itemNext = item + 1;
    size_t dataLeft = dataSize;
    uint8_t* pos = data;
    // at the end of the data is the encoded silence reset
    uint8_t* posSilence = data + dataSize - I2S_DMA_SILENCE_SIZE;

    // front two are silent items used for looping to micmic single fire
    //  default to looping
    dmaItemInit(item, posSilence, I2S_DMA_SILENCE_SIZE, itemNext);
    dmaItemInit(itemNext, posSilence, I2S_DMA_SILENCE_SIZE, item);
    item = itemNext;
    itemNext++;

    // init blocks with avialable data
    //
    while (dataLeft)
    {
        item = itemNext;
        itemNext++;

        size_t blockSize = dataLeft;
        if (blockSize > I2S_DMA_MAX_DATA_LEN)
        {
            blockSize = I2S_DMA_MAX_DATA_LEN;
        }
        dataLeft -= blockSize;

        dmaItemInit(item, pos, blockSize, itemNext);

        pos += blockSize;
    }

    // last data item is EOF to manage send state using EOF ISR
    item->eof = 1;

    // last block, the back silent item, loops to front
    item = itemNext;
    dmaItemInit(item, posSilence, I2S_DMA_SILENCE_SIZE, itemFirst);

    return true;
}

bool i2sDeinitDmaItems(uint8_t bus_num) 
{
    if (bus_num >= I2S_NUM_MAX) 
    {
        return false;
    }

    heap_caps_free(I2S[bus_num].dma_items);
    I2S[bus_num].dma_items = NULL;

    return true;
}

// normal 4, 10, 63, 12, 16

esp_err_t i2sSetClock(uint8_t bus_num, 
        uint8_t div_num, // 4     13
        uint8_t div_b,   // 10    20
        uint8_t div_a,   // 63    63
        uint8_t bck,     // 12    60 or 7
        uint8_t bits)    // 16    8
{
    if (bus_num >= I2S_NUM_MAX || div_a > 63 || div_b > 63 || bck > 63) 
    {
        return ESP_FAIL;
    }

    //log_i("i2sSetClock bus %u, clkm_div_num %u, clk_div_a %u, clk_div_b %u, bck_div_num %u, bits_mod %u",
    //    bus_num,
    //    div_num,
    //    div_a,
    //    div_b,
    //    bck,
    //    bits);

    i2s_dev_t* i2s = I2S[bus_num].bus;

    typeof(i2s->clkm_conf) clkm_conf;

    clkm_conf.val = 0;

#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)
    clkm_conf.clk_sel = 2; // APPL = 1 APB = 2
    clkm_conf.clk_en = 1; // examples of i2s show this being set if sel is set to 2
#else
    clkm_conf.clka_en = 0;
#endif

    clkm_conf.clkm_div_a = div_a;
    clkm_conf.clkm_div_b = div_b;
    clkm_conf.clkm_div_num = div_num;
    i2s->clkm_conf.val = clkm_conf.val;

    typeof(i2s->sample_rate_conf) sample_rate_conf;
    sample_rate_conf.val = 0;
    sample_rate_conf.tx_bck_div_num = bck;
    sample_rate_conf.rx_bck_div_num = bck;
    sample_rate_conf.tx_bits_mod = bits;
    sample_rate_conf.rx_bits_mod = bits;
    i2s->sample_rate_conf.val = sample_rate_conf.val;

    return ESP_OK;
}

void i2sSetPins(uint8_t bus_num, 
        int8_t out, 
        int8_t parallel,
        int8_t busSampleSize, 
        bool invert)
{
    if (bus_num >= I2S_NUM_MAX) 
    {
        return;
    }

    if (out >= 0) 
    {
        uint32_t i2sSignal;

        pinMode(out, OUTPUT);

#if defined(CONFIG_IDF_TARGET_ESP32S2)

        // S2 only has one bus
        // 
        //  in parallel mode
        //  8bit mode   : I2S0O_DATA_OUT16_IDX ~I2S0O_DATA_OUT23_IDX
        //  16bit mode  : I2S0O_DATA_OUT8_IDX ~I2S0O_DATA_OUT23_IDX
        //  24bit mode  : I2S0O_DATA_OUT0_IDX ~I2S0O_DATA_OUT23_IDX
        if (parallel == -1)
        {
            i2sSignal = I2S0O_DATA_OUT23_IDX;
        }
        else if (busSampleSize == 1)
        {
            i2sSignal = I2S0O_DATA_OUT16_IDX + parallel;
        }
        else if (busSampleSize == 2)
        {
            i2sSignal = I2S0O_DATA_OUT8_IDX + parallel;
        }
        else
        {
            i2sSignal = I2S0O_DATA_OUT0_IDX + parallel;
        }

#else
        if (bus_num == 0)
        {
            //  in parallel mode
            //  0-7 bits   : I2S0O_DATA_OUT16_IDX ~I2S0O_DATA_OUT23_IDX
            //  8-15 bits  : I2S0O_DATA_OUT8_IDX ~I2S0O_DATA_OUT23_IDX
            //  16-23 bits : I2S0O_DATA_OUT0_IDX ~I2S0O_DATA_OUT23_IDX
            if (parallel == -1)
            {
                i2sSignal = I2S0O_DATA_OUT23_IDX;
            }
            else if (parallel < 8)
            {
                i2sSignal = I2S0O_DATA_OUT16_IDX + parallel;
            }
            else if (parallel < 16)
            {
                i2sSignal = I2S0O_DATA_OUT8_IDX + parallel - 8;
            }
            else
            {
                i2sSignal = I2S0O_DATA_OUT0_IDX + parallel - 16;
            }
        }
        else
        {
            if (parallel == -1)
            {
                i2sSignal = I2S1O_DATA_OUT23_IDX;
            }
            else
            {
                i2sSignal = I2S1O_DATA_OUT0_IDX + parallel;
            }
        }
#endif
        //log_i("i2sSetPins bus %u, i2sSignal %u, pin %u, mux %u",
        //    bus_num,
        //    i2sSignal,
        //    out,
        //    parallel);
        gpio_matrix_out(out, i2sSignal, invert, false);
    } 
}

void i2sSetClkWsPins(uint8_t bus_num,
    int8_t outClk,
    bool invertClk,
    int8_t outWs,
    bool invertWs)
{

    if (bus_num >= I2S_NUM_MAX)
    {
        return;
    }

    uint32_t i2sSignalClk = I2S0O_BCK_OUT_IDX;
    uint32_t i2sSignalWs = I2S0O_WS_OUT_IDX;

#if !defined(CONFIG_IDF_TARGET_ESP32S2)
    if (bus_num == 1)
    {
        i2sSignalClk = I2S1O_BCK_OUT_IDX;
        i2sSignalWs = I2S1O_WS_OUT_IDX;
    }
#endif

    if (outClk >= 0)
    {
        pinMode(outClk, OUTPUT);
        gpio_matrix_out(outClk, i2sSignalClk, invertClk, false);
    }

    if (outWs >= 0)
    {
        pinMode(outWs, OUTPUT);
        gpio_matrix_out(outWs, i2sSignalWs, invertWs, false);
    }
}

bool i2sWriteDone(uint8_t bus_num) 
{
    if (bus_num >= I2S_NUM_MAX) 
    {
        return false;
    }

    return (I2S[bus_num].is_sending_data == I2s_Is_Idle);
}

void i2sInit(uint8_t bus_num, 
        bool parallel_mode,
        size_t bytes_per_sample, 
        uint32_t sample_rate, 
        i2s_tx_chan_mod_t chan_mod, 
        i2s_tx_fifo_mod_t fifo_mod, 
        size_t dma_count, 
        uint8_t* data, 
        size_t dataSize)
{
    if (bus_num >= I2S_NUM_MAX) 
    {
        return;
    }

    I2S[bus_num].dma_count = dma_count + 
            I2S_DMA_SILENCE_BLOCK_COUNT_FRONT +
            I2S_DMA_SILENCE_BLOCK_COUNT_BACK;

    if (!i2sInitDmaItems(bus_num, data, dataSize)) 
    {
        return;
    }

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)
// (I2S_NUM_MAX == 2)
    if (bus_num) 
    {
        periph_module_enable(PERIPH_I2S1_MODULE);
    } 
    else 
#endif
    {
        periph_module_enable(PERIPH_I2S0_MODULE);
    }

    esp_intr_disable(I2S[bus_num].isr_handle);
    i2s_dev_t* i2s = I2S[bus_num].bus;
    i2s->out_link.stop = 1;
    i2s->conf.tx_start = 0;
    i2s->int_ena.val = 0;
    i2s->int_clr.val = 0xFFFFFFFF;
    i2s->fifo_conf.dscr_en = 0;

    // reset i2s
    i2s->conf.tx_reset = 1;
    i2s->conf.tx_reset = 0;
    i2s->conf.rx_reset = 1;
    i2s->conf.rx_reset = 0;

    // reset dma
    i2s->lc_conf.in_rst = 1;
    i2s->lc_conf.in_rst = 0;
    i2s->lc_conf.out_rst = 1;
    i2s->lc_conf.out_rst = 0;

    // reset fifo
    i2s->conf.rx_fifo_reset = 1;
    i2s->conf.rx_fifo_reset = 0;
    i2s->conf.tx_fifo_reset = 1;
    i2s->conf.tx_fifo_reset = 0;


    // set parallel (LCD) mode
    {
        typeof(i2s->conf2) conf2;
        conf2.val = 0;
        conf2.lcd_en = parallel_mode;
        conf2.lcd_tx_wrx2_en = 0; // parallel_mode; // ((parallel_mode) && (bytes_per_sample == 2));
        i2s->conf2.val = conf2.val;
    }

    // Enable and configure DMA
    {
        typeof(i2s->lc_conf) lc_conf;
        lc_conf.val = 0;
        lc_conf.out_eof_mode = 1;
        i2s->lc_conf.val = lc_conf.val;
    }

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)
    i2s->pdm_conf.pcm2pdm_conv_en = 0;
    i2s->pdm_conf.pdm2pcm_conv_en = 0;
#endif
    // SET_PERI_REG_BITS(RTC_CNTL_CLK_CONF_REG, RTC_CNTL_SOC_CLK_SEL, 0x1, RTC_CNTL_SOC_CLK_SEL_S);

    {
        typeof(i2s->fifo_conf) fifo_conf;

        fifo_conf.val = 0;
        fifo_conf.rx_fifo_mod_force_en = 1;
        fifo_conf.tx_fifo_mod_force_en = 1;
        fifo_conf.tx_fifo_mod = fifo_mod; //  0-right&left channel;1-one channel
        fifo_conf.rx_fifo_mod = fifo_mod; //  0-right&left channel;1-one channel
        fifo_conf.rx_data_num = 32; //Thresholds.
        fifo_conf.tx_data_num = 32;

        i2s->fifo_conf.val = fifo_conf.val;
    }

    // $REVIEW old code didn't set this
    { 
        typeof(i2s->conf1) conf1;
        conf1.val = 0;
        conf1.tx_stop_en = 0;
        conf1.tx_pcm_bypass = 1;
        i2s->conf1.val = conf1.val;
    }

    {
        typeof(i2s->conf_chan) conf_chan;
        conf_chan.val = 0;
        conf_chan.tx_chan_mod = chan_mod; //  0-two channel;1-right;2-left;3-righ;4-left
        conf_chan.rx_chan_mod = chan_mod; //  0-two channel;1-right;2-left;3-righ;4-left
        i2s->conf_chan.val = conf_chan.val;
    }

    {
        typeof(i2s->conf) conf;
        conf.val = 0;
        conf.tx_msb_shift = !parallel_mode; // 0:DAC/PCM, 1:I2S
        conf.tx_right_first = 0; // parallel_mode?;
        i2s->conf.val = conf.val;
    }

    i2s->timing.val = 0;

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)
    i2s->pdm_conf.rx_pdm_en = 0;
    i2s->pdm_conf.tx_pdm_en = 0;
#endif
   
    
    i2sSetSampleRate(bus_num, sample_rate, parallel_mode, bytes_per_sample);

    /* */
    //Reset FIFO/DMA -> needed? Doesn't dma_reset/fifo_reset do this?
    i2s->lc_conf.in_rst=1; i2s->lc_conf.out_rst=1; i2s->lc_conf.ahbm_rst=1; i2s->lc_conf.ahbm_fifo_rst=1;
    i2s->lc_conf.in_rst=0; i2s->lc_conf.out_rst=0; i2s->lc_conf.ahbm_rst=0; i2s->lc_conf.ahbm_fifo_rst=0;
    i2s->conf.tx_reset=1; i2s->conf.tx_fifo_reset=1; i2s->conf.rx_fifo_reset=1;
    i2s->conf.tx_reset=0; i2s->conf.tx_fifo_reset=0; i2s->conf.rx_fifo_reset=0;
    /* */

    //  enable intr in cpu // 
    int i2sIntSource;

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)
//    (I2S_NUM_MAX == 2)
    if (bus_num == 1) {
        i2sIntSource = ETS_I2S1_INTR_SOURCE;
    }
    else
#endif
    {
        i2sIntSource = ETS_I2S0_INTR_SOURCE;
    }

    esp_intr_alloc(i2sIntSource, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1, &i2sDmaISR, &I2S[bus_num], &I2S[bus_num].isr_handle);
    //  enable send intr
    i2s->int_ena.out_eof = 1;
    i2s->int_ena.out_dscr_err = 1;

/*  ??? */
    // Enable and configure DMA
    {
        typeof(i2s->lc_conf) lc_conf;
        lc_conf.val = 0;
        lc_conf.out_data_burst_en = 1;
        lc_conf.indscr_burst_en = 1;
        i2s->lc_conf.val = lc_conf.val;
    }
/* */
    i2s->fifo_conf.dscr_en = 1;// enable dma
    i2s->out_link.start = 0;
    i2s->out_link.addr = (uint32_t)(&I2S[bus_num].dma_items[0]); // loads dma_struct to dma
    i2s->out_link.start = 1; // starts dma
    i2s->conf.tx_start = 1;// Start I2s module

    esp_intr_enable(I2S[bus_num].isr_handle);
}

void i2sDeinit(uint8_t bus_num) 
{
    i2sDeinitDmaItems(bus_num);
}

esp_err_t i2sSetSampleRate(uint8_t bus_num, uint32_t rate, bool parallel_mode, size_t bytes_per_sample)
{
    if (bus_num >= I2S_NUM_MAX) 
    {
        return ESP_FAIL;
    }

    uint8_t bck = 12;

    // parallel mode needs a higher sample rate
    //
    if (parallel_mode)
    {
#if defined(CONFIG_IDF_TARGET_ESP32S2)
        rate *= bytes_per_sample;
        bck *= bytes_per_sample;

        //rate /= bytes_per_sample;
        //bck /= bytes_per_sample; 
#else
        rate *= bytes_per_sample;
#endif
    }

    //               160,000,000L / (100,000 * 384)
    double clkmdiv = (double)I2S_BASE_CLK / ((rate * 384) + 1);
    if (clkmdiv > 256.0) 
    {
        log_e("rate is too low");
        return ESP_FAIL;
    } 
    else if (clkmdiv < 2.0) 
    {
        log_e("rate is too fast, clkmdiv = %f (%u, %u, %u)",
            clkmdiv,
            rate,
            parallel_mode,
            bytes_per_sample);
        return ESP_FAIL;
    }

    // calc integer and franctional for more precise timing
    // 
    uint8_t clkmInteger = clkmdiv;
    uint8_t clkmFraction = (clkmdiv - clkmInteger) * 63.0;

    i2sSetClock(bus_num, clkmInteger, clkmFraction, 63, bck, bytes_per_sample * 8);

    return ESP_OK;
}

void IRAM_ATTR i2sDmaISR(void* arg)
{
    i2s_bus_t* i2s = (i2s_bus_t*)(arg);

    if (i2s->bus->int_st.out_eof) 
    {
 //       lldesc_t* item = (lldesc_t*)(i2s->bus->out_eof_des_addr);
        if (i2s->is_sending_data != I2s_Is_Idle)
        {
            // the second item (last of the two front silent items) is 
            // silent looping item
            lldesc_t* itemLoop = &i2s->dma_items[0];
            lldesc_t* itemLoopBreaker = itemLoop + 1;
            // set to loop on silent items
            itemLoopBreaker->qe.stqe_next = itemLoop;

            i2s->is_sending_data = I2s_Is_Idle;
        }
    }

    i2s->bus->int_clr.val = i2s->bus->int_st.val;
}

bool i2sWrite(uint8_t bus_num) 
{
    if (bus_num >= I2S_NUM_MAX) 
    {
        return false;
    }

    // the second item (last of the two front silent items) is 
    // silent looping item
    lldesc_t* itemLoopBreaker = &I2S[bus_num].dma_items[1]; 
    lldesc_t* itemLoopNext = itemLoopBreaker + 1;

    // set to NOT loop on silent items
    itemLoopBreaker->qe.stqe_next = itemLoopNext;

    I2S[bus_num].is_sending_data = I2s_Is_Sending;

    return true;
}

#endif // !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)
#endif // defined(ARDUINO_ARCH_ESP32) 

