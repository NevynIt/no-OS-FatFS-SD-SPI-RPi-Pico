/* hw_config.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use 
this file except in compliance with the License. You may obtain a copy of the 
License at

   http://www.apache.org/licenses/LICENSE-2.0 
Unless required by applicable law or agreed to in writing, software distributed 
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR 
CONDITIONS OF ANY KIND, either express or implied. See the License for the 
specific language governing permissions and limitations under the License.
*/
/*

This file should be tailored to match the hardware design.

There should be one element of the spi[] array for each hardware SPI used.

There should be one element of the sd_cards[] array for each SD card slot.
The name is should correspond to the FatFs "logical drive" identifier.
(See http://elm-chan.org/fsw/ff/doc/filename.html#vol)
The rest of the constants will depend on the type of
socket, which SPI it is driven by, and how it is wired.

*/

#include <string.h>
//
#include "my_debug.h"
//
#include "hw_config.h"
//
#include "ff.h" /* Obtains integer types */
//
#include "diskio.h" /* Declarations of disk functions */
//
#include "rp2040_sdio.pio.h"
#include "SDIO/rp2040_sdio.h"

void spi0_dma_isr(); // Forward declaration of spis[0]'s DMA ISR
void sdio0_dma_isr(); // Forward declaration of sd_cards[0].sdio_if's DMA ISR

// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.
static spi_t spis[] = {  // One for each SPI.
    {
        .hw_inst = spi1,  // SPI component
        .miso_gpio = 12,  // GPIO number (not Pico pin number)
        .mosi_gpio = 15,
        .sck_gpio = 14,
        .set_drive_strength = true,
        .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,
        .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,

        .baud_rate = 25 * 1000 * 1000, // Actual frequency: 20833333. 

        .DMA_IRQ_num = DMA_IRQ_1,
        .dma_isr = spi0_dma_isr
    }
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
    {
        .pcName = "0:",   // Name used to mount device

        .type = SD_IF_SDIO,
        .sdio_if.CLK_gpio = SDIO_CLK_GPIO, // From sd_driver/SDIO/rp2040_sdio.pio
        .sdio_if.CMD_gpio = 18,
        .sdio_if.D0_gpio = 19,
        .sdio_if.D1_gpio = 20,
        .sdio_if.D2_gpio = 21,
        .sdio_if.D3_gpio = 22,
        .sdio_if.SDIO_PIO = pio1,
        .sdio_if.DMA_IRQ_num = DMA_IRQ_1,
        .sdio_if.dma_isr = sdio0_dma_isr,

        .use_card_detect = true,    
        .card_detect_gpio = 16,   // Card detect
        .card_detected_true = 1   // What the GPIO read returns when a card is
                                  // present.
    }, {
        .pcName = "1:",   // Name used to mount device
        .type = SD_IF_SPI,
        .spi_if.spi = &spis[0],  // Pointer to the SPI driving this card
        .spi_if.ss_gpio = 9,     // The SPI slave select GPIO for this SD card
        .spi_if.set_drive_strength = true,
        .spi_if.ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,
        .use_card_detect = true,
        .card_detect_gpio = 13,   // Card detect
        .card_detected_true = 1   // What the GPIO read returns when a card is
                                  // present.
    }
};

void spi0_dma_isr() { spi_irq_handler(&spis[0]); } // spis[0]'s DMA ISR
void sdio0_dma_isr() { rp2040_sdio_tx_irq(&sd_cards[0]); } // sd_cards[0].sdio_if's DMA ISR

/* ********************************************************************** */
size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}
size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}

/* [] END OF FILE */