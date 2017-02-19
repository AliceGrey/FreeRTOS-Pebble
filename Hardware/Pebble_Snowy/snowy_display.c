/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "display.h"
#include "vibrate.h"
#include "snowy_display.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_tim.h>

// pointer to the place in flash where the FPGA image resides
extern unsigned char _binary_Resources_FPGA_4_3_snowy_dumped_bin_start;
extern unsigned char _binary_Resources_FPGA_4_3_snowy_dumped_bin_size;

void snowy_display_init_dma(void);

// Display configuration for the Pebble TIME
display_t display = {
    .PortDisplay    = GPIOG,
    .PinReset       = GPIO_Pin_15,
    .PinCs          = GPIO_Pin_8, 
    .PinBacklight   = GPIO_Pin_14,
    .PortBacklight  = GPIOB,
    .PinMiso        = GPIO_Pin_12,
    .PinMosi        = GPIO_Pin_14,
    .PinSck         = GPIO_Pin_13,
    
    .PinResetDone   = GPIO_Pin_9,
    .PinIntn        = GPIO_Pin_10,
    
    .NumRows        = 168,
    .NumCols        = 144,
    .NumBorderRows  = 2,
    .NumBorderCols  = 2,
};


/*
 * Initialise the hardware. This means all GPIOs and SPI for the display
 */
void hw_display_init(void)
{   

    // init display variables
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
        
    printf("Display Init\n");
    //GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure_Disp;
    GPIO_InitTypeDef GPIO_InitStructure_Disp_o;
    
    // these are set. not sure why, but will set them for completeness for now
    /*GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // init the ??? (pins 2 & 3 are for something. not wure yet)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    // bootloader pulls port D2 + D4 high
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    */
    
    // init the portG display pins (inputs)
    GPIO_InitStructure_Disp.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure_Disp.GPIO_Pin =  display.PinResetDone;
    GPIO_InitStructure_Disp.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Disp.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure_Disp.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(display.PortDisplay, &GPIO_InitStructure_Disp);

    // init the portG display pins (inputs)
    GPIO_InitStructure_Disp.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure_Disp.GPIO_Pin =  display.PinIntn;
    GPIO_InitStructure_Disp.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Disp.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure_Disp.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOG, &GPIO_InitStructure_Disp);
    
    // init the portG display pins (outputs)
    GPIO_InitStructure_Disp_o.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure_Disp_o.GPIO_Pin =  display.PinReset; // | display.PinPower;
    GPIO_InitStructure_Disp_o.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Disp_o.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure_Disp_o.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(display.PortDisplay, &GPIO_InitStructure_Disp_o);
       
        
    // start SPI
    snowy_display_init_SPI6();
    
    snowy_display_init_dma();
     
//     // bootloader does this too. Why??
//     GPIO_SetBits(GPIOD, GPIO_Pin_2);
//     GPIO_SetBits(GPIOD, GPIO_Pin_4);
//     
//     GPIO_SetBits(GPIOF, GPIO_Pin_3);
//     GPIO_SetBits(GPIOF, GPIO_Pin_2);

}

/*
 * We use the Done INTn for the display. This is asserted
 * after every successful command write. I.t. 0x5 to star a frame 
 * will assert done
 */
void snowy_display_init_intn(void)
{
    // Snowy uses two interrupts for the display
    // Done (G10) signals the drawing is done
    // INTn (G9) I suspect is for device readyness after flash
    //
    // PinSource9 uses IRQ (EXTI9_5_IRQn) and 10 uses (EXTI15_10_IRQn)
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
        
    // Wait for external interrupts when the FPGA is done with a command
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource10);
    
    EXTI_InitStruct.EXTI_Line = EXTI_Line10;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStruct);

    // display used PinSource10 which is connected to EXTI15_10
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 6;  // must be > 5
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/*
 * The display hangs off SPI6. Initialise it
 */
void snowy_display_init_SPI6(void)
{	
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_InitTypeDef SPI_InitStruct;

    // enable clock for used IO pins
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    /* configure pins used by SPI6
        * PG13 = SCK
        * PG12 = MISO
        * PG14 = MOSI
        */
    GPIO_InitStruct.GPIO_Pin = display.PinMiso | display.PinMosi | display.PinSck;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(display.PortDisplay, &GPIO_InitStruct);

    // connect SPI6 pins to SPI alternate function
    GPIO_PinAFConfig(display.PortDisplay, GPIO_PinSource13, GPIO_AF_SPI6);
    GPIO_PinAFConfig(display.PortDisplay, GPIO_PinSource12, GPIO_AF_SPI6);
    GPIO_PinAFConfig(display.PortDisplay, GPIO_PinSource14, GPIO_AF_SPI6);

    GPIO_InitStruct.GPIO_Pin = display.PinCs;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(display.PortDisplay, &GPIO_InitStruct);

    // enable peripheral clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI6, ENABLE);

    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;        // clock is low when idle
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // SPI frequency is APB2 frequency / 2
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted LSB first
    // pebble additionally has these Set

    SPI_Init(SPI6, &SPI_InitStruct); 

    SPI_Cmd(SPI6, ENABLE); // enable SPI
}

/*
 * Initialise DMA for sending frames.
 * DMA is only used for doing a full frame transfer
 * once frame mode select is sent
 */
void snowy_display_init_dma(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    
    // spi6 dma config:
    // SPI6 	DMA2 	DMA Stream 5 	DMA Channel 1 	DMA Stream 6 	DMA Channel 0
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    // De-init DMA configuration just to be sure. No Boom
    DMA_DeInit(DMA2_Stream5);
    // Configure DMA controller to manage TX DMA requests
    DMA_Cmd(DMA2_Stream5, DISABLE);
    while (DMA2_Stream5->CR & DMA_SxCR_EN);

    DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_FEIF5|DMA_FLAG_DMEIF5|DMA_FLAG_TEIF5|DMA_FLAG_HTIF5|DMA_FLAG_TCIF5);
    
    DMA_StructInit(&DMA_InitStructure);
    // set the pointer to the SPI6 DR register
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(SPI6->DR);
    DMA_InitStructure.DMA_Channel = DMA_Channel_1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0; // set this to bypass assert
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_FIFOMode  = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream5, &DMA_InitStructure);
    
    // enable the tx interrupt.
    DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);
     
    // Enable dma
    SPI_I2S_DMACmd(SPI6, SPI_I2S_DMAReq_Tx, ENABLE);
    
    // tell the NVIC to party
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
 * Start a new Dma transfer
 */
void snowy_display_reinit_dma(uint32_t *data, uint32_t length)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    // Configure DMA controller to manage TX DMA requests
    DMA_Cmd(DMA2_Stream5, DISABLE);
    while (DMA2_Stream5->CR & DMA_SxCR_EN);

    DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_FEIF5|DMA_FLAG_DMEIF5|DMA_FLAG_TEIF5|DMA_FLAG_HTIF5|DMA_FLAG_TCIF5);
    
    DMA_StructInit(&DMA_InitStructure);
    // set the pointer to the SPI6 DR register
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &SPI6->DR;
    DMA_InitStructure.DMA_Channel = DMA_Channel_1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)display.DisplayBuffer; // set this to bypass assert
    DMA_InitStructure.DMA_BufferSize = 24192;
    DMA_InitStructure.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_FIFOMode  = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;

    DMA_Init(DMA2_Stream5, &DMA_InitStructure);
    
    // enable the tx interrupt.
    DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);
     
    // Enable dma
    SPI_I2S_DMACmd(SPI6, SPI_I2S_DMAReq_Tx, ENABLE);
}

/*
 * DMA2 handler for SPI6
 */
void DMA2_Stream5_IRQHandler()
{
    if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5))
    {
        //printf("DMA BEGIN end frame\n");
        DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);

        // check the tx finished
        while (SPI_I2S_GetFlagStatus(SPI6, SPI_I2S_FLAG_TXE) == RESET)
        {
        };
        
        // make sure we are not busy
        while (SPI_I2S_GetFlagStatus(SPI6, SPI_I2S_FLAG_BSY) == SET)
        {
        };

        // done. We are still in control of the SPI select, so lets let go
        snowy_display_cs(0);
        display.State = DISPLAY_STATE_IDLE;
    }
}

/*
 * Interrupt handler for the INTn Done interrupt on the FPGA
 * When we send a command and it is successfully acked, we get a 
 * GPIO interrupt. We then chain call the ISR handler from here
 * and clear the interrupt
 */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {   
        //display_done_ISR(1);
       
        EXTI_ClearITPendingBit(EXTI_Line10);
    }
}

/* When reset goes high, sample the CS input to see what state we should be in
 * if CS is low, expect new FPGA programming to arrive
 * if CS is high, assume we will be using the bootloader configuration
 * 
 *  *NOTE* CS is inverted below
 */
void snowy_display_cs(uint8_t enabled)
{
    // CS bit is inverted
    if (!enabled)
        GPIO_SetBits(display.PortDisplay, display.PinCs);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinCs);
}

/*
 * Request the version from the FPGA in bootloader mode
 */
uint8_t snowy_display_SPI6_getver(uint8_t data)
{
    while( !(SPI6->SR & SPI_I2S_FLAG_TXE) ); // wait until send complete
    SPI6->DR = data; // write data to be transmitted to the SPI data register
    while( !(SPI6->SR & SPI_I2S_FLAG_RXNE) ); // wait until send complete

    return SPI6->DR; // return received data from SPI data register
}

/*
 * Send one byte over the SPI
 */
uint8_t snowy_display_SPI6_send(uint8_t data)
{
    // TODO likely DMA would be better :)
    SPI6->DR = data; // write data to be transmitted to the SPI data register
    while( !(SPI6->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while( !(SPI6->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
    while( SPI6->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
    return SPI6->DR; // return received data from SPI data register
}

/*
 * Send n bytes over SPI using the DMA engine.
 * This will async run and call the ISR when complete
 */
uint8_t snowy_display_dma_send(uint8_t *data, uint32_t length)
{
    // re-initialise the DMA controller. prep for send
    snowy_display_reinit_dma((uint32_t *)data, length);
    DMA_Cmd(DMA2_Stream5, ENABLE);
    
    return 0;
}

/*
 * Reset the FPGA. This goes through a convoluted set of steps
 * that basically pound the FPGA into submission. Ya see, sometimes
 * it just doesn't boot nicely. So you have to try a command, no dice?
 * reeeeboot. Over and over until it works.
 * Yay
 */
uint8_t snowy_display_FPGA_reset(uint8_t mode)
{
    uint16_t k = 0;
    uint8_t g9 = 0;    

    snowy_display_cs(mode);
    //snowy_display_cs(1);
    snowy_display_reset(0);
    snowy_display_cs(1);
    delay_ns(1);
    snowy_display_reset(1);
    delay_ns(1);
    
    // don't wait when we are not in bootloader mode
    // qemu doesn't like this, real pebble behaves
    //if (mode == 1)
        return 1;
   
    // The real pebble at this point will pull reset done once it has reset
    // it also (probably dangerously) "just works" without.
    // it probably isn't the danger zone, but it's the highway
    while(1)
    {
        g9 = GPIO_ReadInputDataBit(display.PortDisplay, display.PinResetDone);
        
        if (g9 > 0)
        {
            printf("FPGA Was reset\n");
            return 1;
        }
        
        delay_us(100);
            
        k++;
        
        if (k >=1001)
        {
            printf("timed out waiting for reset\n");
            //display_vibrate(1);
            vibrate_enable(1);
            return 0;
        }
    }
}

/*
 * Do a hard reset on the display. Make sure to init it!
 */
void snowy_display_reset(uint8_t enabled)
{
    if (enabled)
        GPIO_SetBits(display.PortDisplay, display.PinReset);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinReset);
}

// SPI Command related

/*
 * Shortcut function to address the display device using its
 * Chip Select and wait the "proper" amount of time for engagement
 */
void snowy_display_SPI_start(void)
{
    snowy_display_cs(1);
    delay_us(100);
}

/*
 * We're done with this device. Drop the line
 */
void snowy_display_SPI_end(void)
{
    snowy_display_cs(0);
}

/*
 * BOOTLOADER MODE ONLY:
 * When in this mode, we can request a pre-baked scene be drawn
 * Interesting but of little utiity
 */
void snowy_display_drawscene(uint8_t scene)
{
    snowy_display_SPI_start();
    snowy_display_SPI6_send(DISPLAY_CTYPE_SCENE); // set cmdset (select scene)
    snowy_display_SPI6_send(scene); // set the scene id
    snowy_display_SPI_end();
}

/*
 * Start to send a frame to the display driver
 * If it says yes, then we can then tell someone to fill the buffer
 */
void snowy_display_start_frame(uint8_t xoffset, uint8_t yoffset)
{
    scanline_convert_buffer(xoffset, yoffset);
    
    snowy_display_cs(1);
    delay_us(80);
    snowy_display_SPI6_send(DISPLAY_CTYPE_FRAME); // Frame Begin
    snowy_display_cs(0);
    delay_us(1);

    display.State = DISPLAY_STATE_FRAME;

    snowy_display_send_frame();
}

/* 
 * We can fill the framebuffer now. Depending on mode, we will DMA
 * the data directly over to the display
 */
void snowy_display_send_frame()
{
    snowy_display_cs(1);
    
    // send over DMA
    snowy_display_dma_send(display.DisplayBuffer, MAX_FRAMEBUFFER_SIZE);
    
    // we return immediately and let the system take care of the rest
}

/*
 * Bang the SPI bit by bit
 */
void snowy_display_send_frame_slow()
{
    snowy_display_cs(1);
    delay_us(50);
      
    // send via standard SPI
    for(uint16_t i = 0; i < MAX_FRAMEBUFFER_SIZE; i++)
    {
        snowy_display_SPI6_send(display.DisplayBuffer[i]);
    }   
    
    snowy_display_cs(0);
    printf("End Frame\n");
}

/*
 * Once we reset the FPGA, it takes some time to wake up
 * let's have a tea party and wait for it
 */
uint8_t snowy_display_wait_FPGA_ready(void)
{
    uint16_t i = 1000;
    
    while(GPIO_ReadInputDataBit(display.PortDisplay, display.PinIntn) != 0)
    {
        if (!--i)
        {
            printf("timed out waiting for ready\n");
            return 0;
        }        
        delay_us(100);
    }
    printf("FPGA Ready\n");
    
    return 1;
}

/*
 * Master initiation of the display. it will got through all steps
 * needed to make the screen come on. Reset it, download the firmware
 * and also kick a scene into view
 */
void snowy_display_splash(uint8_t scene)
{  
    
    if (!snowy_display_FPGA_reset(0)) // mode bootloader
    {
        return;
    }
  
        
    // Get the version of the FPGA.
    // This is done in bootloader at least once. But we are skipping this for now
    //     display_cs(1);
    //     for(int i = 0; i < 1000; i++)
    //         ;;
    //     display_SPI6_getver(0);
    //     
    //     display_cs(0);
    //     
        
    // once a scene is selected, we need to wait for the FPGA to respond.
    // it seems like sometimes it doesn't behave, so we reset it until it does 
    // I have one device that requires 5 cycles before it is alive!
    // known issue apparently (Katharine Berry confirmed)
    for (uint8_t i = 0; i < 10; i++)
    {
        delay_ns(1);
        snowy_display_cs(1);
        delay_us(100);
        snowy_display_SPI6_send(DISPLAY_CTYPE_SCENE); // Scene select
        snowy_display_SPI6_send(scene); // Select scene
        snowy_display_cs(0);
        
        IWDG_ReloadCounter();
        delay_us(100);
        
        if (snowy_display_wait_FPGA_ready())
        {
            hw_display_on();
            printf("Display Init Complete\n");
            
            break;
        }
        
        snowy_display_FPGA_reset(0);
    }
    
    return;                
}

/*
 * Reset the FPGA and send the display engine into full frame mode
 * This will allow raw frame dumps to work
 */
void snowy_display_full_init(void)
{
    printf("Going full fat\n");
    
    if (!snowy_display_FPGA_reset(1)) // full fat
    {
        return;
    }
    display_logo(display.BackBuffer);
    
    snowy_display_program_FPGA();
    
    delay_us(100);
    hw_display_on();

    delay_us(100);
    snowy_display_start_frame(0, 0);
    delay_us(10);
    //snowy_display_send_frame();
    
    IWDG_ReloadCounter();
    
    // enable interrupts now we have the splash up
    snowy_display_init_intn();
    return;
}

/*
 * Get the source for the display's FPGA, and download it to the device
 */
void snowy_display_program_FPGA(void)
{
    unsigned char *fpgaBlob = &_binary_Resources_FPGA_4_3_snowy_dumped_bin_start;
           
    // enter programming mode
    snowy_display_cs(1);
    
    // Do this with good ol manual SPI for reliability
    for (uint32_t i = 0; i < (uint32_t)&_binary_Resources_FPGA_4_3_snowy_dumped_bin_size; i++)
    {
        snowy_display_SPI6_send(*(fpgaBlob + i));
    }
    snowy_display_cs(0);
    
    printf("Sent FPGA dump\n");
}


// API (barely) implementation

/*
 * When in bootloader mode, we can command the screen to power on
 * Of little use the end developers now
 */
void hw_display_on()
{
    snowy_display_SPI_start();
    snowy_display_SPI6_send(DISPLAY_CTYPE_DISPLAY_ON); // Power on
    snowy_display_SPI_end();
}

/*
 * Start a frame render
 */
void hw_display_start_frame(uint8_t xoffset, uint8_t yoffset)
{
    snowy_display_start_frame(xoffset, yoffset);
}

/*
 * Reset the display
 * Needs work before it works (ahem) after first boot
 * as interrupts are still enabled and get all in the way
 */
void hw_display_reset(void)
{
    // disable interrupts
    // TODO
    // go for it
    hw_display_start();
}

/*
 * Start the display init sequence. from cold start to full framebuffer
 */
void hw_display_start(void)
{
    // begin the init
    snowy_display_splash(2);
    delay_us(100);
    snowy_display_full_init();
}


// Util

void delay_us(uint16_t us)
{
    for(int i = 0; i < 22 * us; i++)
            ;;
}

void delay_ns(uint16_t ns)
{
    delay_us(1000 * ns);
}
