/* tintin_flash.c
 * flash driver for tintin's Micron N250xxA range
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 *         Barry Carter <barry.carter@gmail.com>
 */

#include <stm32f2xx.h>
#include "tintin.h"
#include <debug.h>

#include <stm32f2xx_gpio.h>
#include <stm32f2xx_rcc.h>
#include <stm32f2xx_spi.h>
#include <stm32f2xx_syscfg.h>
#include <misc.h>
#include "rebbleos.h"

#include "stm32_power.h"
#include "stm32_spi.h"

static void _spi_flash_tx_done(void);
static void _spi_flash_rx_done(void);

static const stm32_spi_config_t _spi1_config = {
    .spi                  = SPI1,
    .spi_periph_bus       = STM32_POWER_APB2,
    .gpio_pin_miso_num    = 6,
    .gpio_pin_mosi_num    = 7,
    .gpio_pin_sck_num     = 5,
    .gpio_ptr             = GPIOA,
    .gpio_clock           = RCC_AHB1Periph_GPIOA,
    .spi_clock            = RCC_APB2Periph_SPI1,
    .af                   = GPIO_AF_SPI1,
    .txrx_dir             = STM32_SPI_DIR_RXTX,
    .spi_prescaler        = SPI_BaudRatePrescaler_2,
    .crc_poly             = 7, /* Um */
    .line_polarity        = SPI_CPOL_Low,
};

/* In theory we can use this DMA. The settings below will work on device,
 * but I can't get them working in qemu. 
 * TX RX DMA code has been removed for now.
 * - ginge
 */
/*
static const stm32_dma_t _spi1_dma = {
    .dma_clock            = RCC_AHB1Periph_DMA2,
    .dma_tx_stream        = DMA2_Stream5,
    .dma_rx_stream        = DMA2_Stream0,
    .dma_tx_channel       = DMA_Channel_3,
    .dma_rx_channel       = DMA_Channel_3,
    .dma_irq_tx_pri       = 8,
    .dma_irq_rx_pri       = 8,
    .dma_irq_tx_channel   = DMA2_Stream5_IRQn,
    .dma_irq_rx_channel   = DMA2_Stream0_IRQn,
    .dma_tx_channel_flags = STM32_DMA_MK_FLAGS(5),
    .dma_rx_channel_flags = STM32_DMA_MK_FLAGS(0),
    .dma_tx_irq_flag      = DMA_IT_TCIF5,
    .dma_rx_irq_flag      = DMA_IT_TCIF0
};
*/
/* rcc, dma, tx_str, rx_str, tx_chan, rx_chan, tx_pri, rx_pri */
// static const stm32_dma_t _spi1_dma = STM32_DMA_MK_INIT(RCC_AHB1Periph_DMA2, 
//                                                        2 /* DMA2 */, 
//                                                        5, 0, /* Stream Tx 5, Rx 0 */ 
//                                                        3, 3, /* Channel 3 */
//                                                        8, 8);
    
static stm32_spi_t _spi1 = {
    &_spi1_config,
    NULL, //&_spi1_dma, /* dma */
};

// STM32_SPI_MK_TX_IRQ_HANDLER(&_spi1, 2, 5, _spi_flash_tx_done)
// STM32_SPI_MK_RX_IRQ_HANDLER(&_spi1, 2, 0, _spi_flash_rx_done)

#define JEDEC_READ 0x03
#define JEDEC_RDSR 0x05
#define JEDEC_IDCODE 0x9F
#define JEDEC_DUMMY 0xA9
#define JEDEC_WAKE 0xAB
#define JEDEC_SLEEP 0xB9

#define JEDEC_RDSR_BUSY 0x01

#define JEDEC_IDCODE_MICRON_N25Q032A11 0x20BB16 /* bianca / qemu / ev2_5 */
#define JEDEC_IDCODE_MICRON_N25Q064A11 0x20BB17 /* v1_5 */


static void _hw_flash_enable(int i) {
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);

    GPIO_WriteBit(GPIOA, 1 << 4, !i);
    delay_us(1);
    
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
}

static void _hw_flash_wfidle() {
    _hw_flash_enable(1);
    stm32_spi_write_read(&_spi1, JEDEC_RDSR);
    while (stm32_spi_write_read(&_spi1, JEDEC_DUMMY) & JEDEC_RDSR_BUSY)
        ;
    _hw_flash_enable(0);
}

void hw_flash_init(void) {
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SPI1);
    
    /* Set up the pins. */
    GPIO_WriteBit(GPIOA, 1 << 4, 0); /* nCS */
   
    GPIO_InitTypeDef gpioinit;
    
    gpioinit.GPIO_Pin = (1 << 4);
    gpioinit.GPIO_Mode = GPIO_Mode_OUT;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioinit.GPIO_OType = GPIO_OType_PP;
    gpioinit.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpioinit);
    
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);

    /* Set up the SPI controller, SPI1. */
    stm32_spi_init_device(&_spi1);
    
    /* In theory, SPI is up.  Now let's see if we can talk to the part. */
    _hw_flash_enable(1);
    stm32_spi_write_read(&_spi1, JEDEC_WAKE);
    _hw_flash_enable(0);
    delay_us(100);
    
    _hw_flash_wfidle();
     
    uint32_t part_id = 0;
    
    _hw_flash_enable(1);
    stm32_spi_write_read(&_spi1, JEDEC_IDCODE);
    part_id |= stm32_spi_write_read(&_spi1, JEDEC_DUMMY) << 16;
    part_id |= stm32_spi_write_read(&_spi1, JEDEC_DUMMY) << 8;
    part_id |= stm32_spi_write_read(&_spi1, JEDEC_DUMMY) << 0;
    _hw_flash_enable(0);

    DRV_LOG("Flash", APP_LOG_LEVEL_INFO, "tintin flash: JEDEC ID %08lx", part_id);

    if (part_id != JEDEC_IDCODE_MICRON_N25Q032A11 && part_id != JEDEC_IDCODE_MICRON_N25Q064A11) {
        panic("tintin flash: unsupported part ID");
    }
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SPI1);
}

void hw_flash_read_bytes(uint32_t addr, uint8_t *buf, size_t len) {
    assert(addr < 0x1000000 && "address too large for JEDEC_READ command");
    
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SPI1);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    _hw_flash_wfidle();
    
    _hw_flash_enable(1);
    stm32_spi_write_read(&_spi1, JEDEC_READ);
    stm32_spi_write_read(&_spi1, (addr >> 16) & 0xFF);
    stm32_spi_write_read(&_spi1, (addr >>  8) & 0xFF);
    stm32_spi_write_read(&_spi1, (addr >>  0) & 0xFF);
    
    /* XXX: could use DMA, conceivably */
    for (int i = 0; i < len; i++) {
        buf[i] = stm32_spi_write_read(&_spi1, JEDEC_DUMMY);
    }
    _hw_flash_enable(0);
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SPI1);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
}
