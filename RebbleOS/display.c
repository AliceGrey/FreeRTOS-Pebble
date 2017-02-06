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
#include "FreeRTOS.h"
#include "stdio.h"
#include "string.h"
#include "platform.h"
#include "display.h"
#include "snowy_display.h"
#include "task.h"
#include "semphr.h"
#include "logo.h"

#include "menu.h"

static TaskHandle_t xDisplayCommandTask;
static xQueueHandle xQueue;
static SemaphoreHandle_t xMutex;

extern display_t display;

/*
 * Start the display driver and tasks. Show splash
 */
void display_init(void)
{   
    // init variables
    display.BacklightEnabled = 0;
    display.Brightness = 0;
    display.PowerOn = 0;
    display.State = DISPLAY_STATE_BOOTING;
    
    printf("Display Init\n");
    
    // initialise device specific display
    hw_display_init();
    
    hw_display_start();
        
    // set up the RTOS tasks
    xTaskCreate(vDisplayCommandTask, "Display", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 5UL, &xDisplayCommandTask);
    
    xQueue = xQueueCreate( 10, sizeof(uint8_t) );
        
    printf("Display Tasks Created!\n");
    
    // turn on the LCD draw
    display.DisplayMode = DISPLAY_MODE_BOOTLOADER;   
    
    display_cmd(DISPLAY_CMD_DRAW, NULL);
    
    xMutex = xSemaphoreCreateMutex();
}

/*
 * When the display driver has responded to something
 * 1) frame frame accepted
 * 2) frame completed
 * We get notified here. We can now let the prcessing complete or continue
 */
void display_done_ISR(uint8_t cmd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Notify the task that the transmission is complete.
    //vTaskNotifyGiveFromISR(xDisplayISRTask, &xHigherPriorityTaskWoken);
    vTaskNotifyGiveFromISR(xDisplayCommandTask, &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


/*
 * Brutally and forcefully reset the display. This will
 * reset, but it will leave you dead in the water. Make sure to init.
 * or, instead, use init
 */
void display_reset(uint8_t enabled)
{
    hw_display_reset();
}

/*
 * We can command the screen on and off. but only in bootloader mode. hrmph
 */
void display_on()
{    
//    display.State = DISPLAY_STATE_TURNING_ON;
}

/*
 * Begin rendering a frame from the framebuffer into the display
 */
void display_start_frame(uint8_t xoffset, uint8_t yoffset)
{
    display.State = DISPLAY_STATE_FRAME_INIT;
    printf("Mtake\n");
    xSemaphoreTake(xMutex, portMAX_DELAY);
    printf("Mgrant\n");
    hw_display_start_frame(xoffset, yoffset);
    xSemaphoreGive(xMutex);
}

/*
 * Given a frame of data, convert it and draw it to the display
 */
void display_logo(char *frameData)
{
    for(uint16_t i = 0; i < 24192; i++)
    {
        frameData[i] = rebbleOS[i];
    }

    return;
}

/*
 * Display a checkerboard picture for testing
 */
uint16_t display_checkerboard(char *frameData, uint8_t invert)
{
    int gridx = 0;
    int gridy = 0;
    int count = 0;

    uint8_t forCol = RED;
    uint8_t backCol = GREEN;
    
    // display a checkboard    
    if (invert)
    {
        forCol = GREEN;
        backCol = RED;
    }
    
    for (int x = 0; x < display.NumCols; x++) {
        gridy = 0;
        if ((x % 21) == 0)
            gridx++;
        
        for (int y = 0; y < display.NumRows; y++) {
            if ((y % 21) == 0)
                gridy++;
            
            if (gridy%2 == 0)
            {
                if (gridx%2 == 0)
                    frameData[count] = forCol;
                    //display_SPI6_send(RED);
                else
                    frameData[count] = backCol;
                    //display_SPI6_send(GREEN);
            }
            else
            {
                if (gridx%2 == 0)    
                    frameData[count] = backCol;
                    //display_SPI6_send(GREEN);
                else
                    //display_SPI6_send(RED);
                    frameData[count] = forCol;
            }
            
            
            count++;
        }
    }
    return count;
}


/*
 * Request a command from the display driver. 
 * Such as DISPLAY_CMD_DRAW
 */
void display_cmd(uint8_t cmd, char *data)
{
    xQueueSendToBack(xQueue, &cmd, 0);
}

void display_draw(void)
{
    display_cmd(DISPLAY_CMD_DRAW, 0);
}

/*
 * Main task processing for the display. Manages locking
 * state machine control and command management
 */
void vDisplayCommandTask(void *pvParameters)
{
    uint8_t data;
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);
    display.State = DISPLAY_STATE_BOOTING;

    while(1)
    {
        if (/*display.State != DISPLAY_STATE_IDLE &&*/
            display.State == DISPLAY_STATE_BOOTING)
        {
            vTaskDelay(5 / portTICK_RATE_MS);
            continue;
        }
        
        // commands to be exectuted are send to this queue and processed
        // one at a time
        if (xQueueReceive(xQueue, &data, xMaxBlockTime))
        {
            switch(data)
            {
                // in the case of draw, we are going to leave locking to
                // the outer laters. If someone calls an overlapping draw into here
                // it's just going to fail
                case DISPLAY_CMD_DRAW:
                    // all we are responsible for is starting a frame draw
                    display_start_frame(0, 0);
                    break;
            }
        }
        else
        {
            // nothing emerged from the buffer
            //hw_get_time_str(buf);
            //UG_ConsolePutString(buf);

            //display_cmd(DISPLAY_CMD_DRAW, 0);
            
        }        
    }
}

