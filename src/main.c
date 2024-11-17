#include "stm32f0xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Define the TFT width and height
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define BLOCK_SIZE 20      // Size of the block (width and height in pixels)
#define BACKGROUND_COLOR RGB565(0, 0, 0)  // Background color (black)
#define BLOCK_COLOR RGB565(255, 0, 0)

// Define color format for 16-bit RGB565
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// Pin macros for control signals
#define TFT_CS_HIGH()   (GPIOB->BSRR = GPIO_BSRR_BS_0)    // Set CS high
#define TFT_CS_LOW()    (GPIOB->BSRR = GPIO_BSRR_BR_0)    // Set CS low
#define TFT_DC_HIGH()   (GPIOA->BSRR = GPIO_BSRR_BS_1)    // Set DC high
#define TFT_DC_LOW()    (GPIOA->BSRR = GPIO_BSRR_BR_1)    // Set DC low
#define TFT_RST_HIGH()  (GPIOA->BSRR = GPIO_BSRR_BS_2)    // Set RESET high
#define TFT_RST_LOW()   (GPIOA->BSRR = GPIO_BSRR_BR_2)    // Set RESET low

void enable_ports(void) {
    // Enable GPIOA and GPIOB for control pins and SPI
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;

    // Configure GPIOB0 as CS (output)
    GPIOB->MODER &= ~GPIO_MODER_MODER0;
    GPIOB->MODER |= GPIO_MODER_MODER0_0;

    // Configure GPIOA1 as DC and GPIOA2 as RESET (outputs)
    GPIOA->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER2);
    GPIOA->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0;

    // Configure SPI pins (PB13 for SCK, PB15 for MOSI)
    GPIOB->MODER &= ~(GPIO_MODER_MODER13 | GPIO_MODER_MODER15);
    GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1; // Set as alternate function
    GPIOB->AFR[1] &= ~(0xF << 4); // Clear alternate function for SCK (PB13)
    GPIOB->AFR[1] |= (0x0 << 4);  // Set AF0 for SCK
    GPIOB->AFR[1] &= ~(0xF << 12); // Clear alternate function for MOSI (PB15)
    GPIOB->AFR[1] |= (0x0 << 12);  // Set AF0 for MOSI
}

void init_spi(void) {
    // Enable SPI2 clock
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    // Configure SPI2 for master mode, prescaler, software slave management, and enable SPI
    SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_SPE;
    SPI2->CR2 = SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0; // 8-bit data frame
}

void tft_reset(void) {
    // Hardware reset sequence for the TFT
    TFT_RST_LOW();
    for (volatile int i = 0; i < 200000; i++); // Longer delay
    TFT_RST_HIGH();
    for (volatile int i = 0; i < 200000; i++); // Longer delay
}

void spi_write(uint8_t data) {
    // Wait until TXE (Transmit buffer empty) flag is set
    while (!(SPI2->SR & SPI_SR_TXE));
    *(volatile uint8_t *)&SPI2->DR = data; // Write data to SPI data register

    // Wait for transmission to complete
    while (SPI2->SR & SPI_SR_BSY);
}

void tft_send_command(uint8_t cmd) {
    TFT_DC_LOW();   // Command mode
    TFT_CS_LOW();
    spi_write(cmd);
    TFT_CS_HIGH();
}

void tft_send_data(uint8_t data) {
    TFT_DC_HIGH();  // Data mode
    TFT_CS_LOW();
    spi_write(data);
    TFT_CS_HIGH();
}

void tft_set_address_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    tft_send_command(0x2A); // Column address set
    tft_send_data(x0 >> 8);
    tft_send_data(x0 & 0xFF);
    tft_send_data(x1 >> 8);
    tft_send_data(x1 & 0xFF);

    tft_send_command(0x2B); // Row address set
    tft_send_data(y0 >> 8);
    tft_send_data(y0 & 0xFF);
    tft_send_data(y1 >> 8);
    tft_send_data(y1 & 0xFF);

    tft_send_command(0x2C); // Memory write
}

void tft_fill_screen(uint16_t color) {
    // Set the entire screen area
    tft_set_address_window(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);

    // Write the color to each pixel in the screen area
    TFT_DC_HIGH();
    TFT_CS_LOW();
    for (uint32_t i = 0; i < (TFT_WIDTH * TFT_HEIGHT); i++) {
        spi_write(color >> 8);     // Send high byte of color
        spi_write(color & 0xFF);   // Send low byte of color
    }
    TFT_CS_HIGH();
}

void tft_init(void) {
    tft_reset();

    tft_send_command(0x01); // Software reset
    for (volatile int i = 0; i < 100000; i++);

    tft_send_command(0x11); // Exit sleep mode
    for (volatile int i = 0; i < 100000; i++);

    tft_send_command(0x3A); // Set color format
    tft_send_data(0x55);    // 16-bit color (RGB565)

    tft_send_command(0x36); // Memory Access Control
    tft_send_data(0x48);    // Row/column addressing, RGB order

    tft_send_command(0x29); // Display ON
}

void tft_update_area(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    // Set the address window for the specified area
    tft_set_address_window(x0, y0, x1, y1);

    // Calculate the number of pixels to fill
    uint32_t pixel_count = (x1 - x0 + 1) * (y1 - y0 + 1);

    // Set to data mode and start filling pixels in the specified area
    TFT_DC_HIGH();
    TFT_CS_LOW();
    for (uint32_t i = 0; i < pixel_count; i++) {
        spi_write(color >> 8);     // Send high byte of color
        spi_write(color & 0xFF);   // Send low byte of color
    }
    TFT_CS_HIGH();
}

void update_screen(int currentState[10][13]){
    for (int j = 0; j < 13; j++){
        for (int i = 0; i < 10; i++){
            if(currentState[i][j] == 1){
                tft_update_area(i * 24, j * 24, (i + 1) * 24 - 1, (j + 1) * 24 - 1, RGB565(255, 255, 255));
            }else{
                // tft_update_area(i * 24, j * 24, (i + 1) * 24 - 1, (j + 1) * 24 - 1, RGB565(0, 0, 0));
            }
        }
    }
}

// void shiftDownPieces(int currentState[10][13]){
//     for (int j = 1; j < 13; j++){
//         for (int i = 0; i < 10; i++){
//             if(currentState[i][j] == 1 && currentState[i][j-1] == 0){
//                 currentState[i][j] = 0;
//                 currentState[i][j-1] = 1;
//                 tft_update_area(i * 24, j * 24, (i + 1) * 24 - 1, (j + 1) * 24 - 1, RGB565(0, 0, 0));
//                 tft_update_area(i * 24, (j-1) * 24, (i + 1) * 24 - 1, j * 24 - 1, RGB565(255, 255, 255));
//             }
//         }
//     }
// }

void shiftDownPieces(int currentState[10][13]){
    for (int j = 1; j < 13; j++){
        for (int i = 0; i < 10; i++){
            if(currentState[i][j] == 1){
                currentState[i][j] = 0;
                currentState[i][j-1] = 1;
                tft_update_area(i * 24, j * 24, (i + 1) * 24 - 1, (j + 1) * 24 - 1, RGB565(0, 0, 0));
                tft_update_area(i * 24, (j-1) * 24, (i + 1) * 24 - 1, j * 24 - 1, RGB565(255, 255, 255));
            }
        }
    }
}

void shiftPiecesSideways(int currentState[10][13], int left, int right){
    if(right == 1){
        for (int j = 0; j < 13; j++){
            for (int i = 0; i < 10; i++){
                if(currentState[i][j] == 1){
                    currentState[i][j] = 0;
                    currentState[i-1][j] = 1;
                    tft_update_area(i * 24, j * 24, (i + 1) * 24 - 1, (j + 1) * 24 - 1, RGB565(0, 0, 0));
                    //tft_update_area(i-1 * 24, j * 24, (i) * 24 - 1, (j + 1) * 24 - 1, RGB565(255, 255, 255));
                }
            }
        }
    }
    if(left == 1){
        for (int j = 0; j < 13; j++){
            for (int i = 9; i >= 0; i--){
                if(currentState[i][j] == 1){
                    currentState[i][j] = 0;
                    currentState[i+1][j] = 1;
                    tft_update_area(i * 24, j * 24, (i + 1) * 24 - 1, (j + 1) * 24 - 1, RGB565(0, 0, 0));
                    //tft_update_area(i-1 * 24, j * 24, (i) * 24 - 1, (j + 1) * 24 - 1, RGB565(255, 255, 255));
                }
            }
        }
    }
}

void mergePieces(int floorBlocks[10][13], int currentPieces[10][13], int *r){
    for (int j = 0; j < 13; j++){
        for (int i = 0; i < 10; i++){
            if(currentPieces[i][j] == 1){
                floorBlocks[i][j] = 1;
                currentPieces[i][j] = 0;
            }
        }
    }
    if(*r == 0){
        currentPieces[5][12] = 1;
        currentPieces[5][11] = 1;
        currentPieces[4][12] = 1;
        currentPieces[4][11] = 1;
    }else if(*r == 1){  
        currentPieces[4][12] = 1;
        currentPieces[4][11] = 1;
        currentPieces[4][10] = 1;
        currentPieces[4][9] = 1;
    }else if(*r == 2){
        currentPieces[6][12] = 1;
        currentPieces[5][12] = 1;
        currentPieces[4][12] = 1;
        currentPieces[5][11] = 1;
    }else if(*r == 3){
        currentPieces[4][12] = 1;
        currentPieces[4][11] = 1;
        currentPieces[4][10] = 1;
        currentPieces[5][10] = 1;
    }else if(*r == 4){
        currentPieces[5][12] = 1;
        currentPieces[5][11] = 1;
        currentPieces[4][11] = 1;
        currentPieces[4][10] = 1;
    }
}

int runGameUpdate(int left, int right, int floorBlocks[10][13], int currentPieces[10][13], int *r){
    //Line is full and to be cleared.
    int count = 0;
    int rowFill;
    for (int j = 0; j < 13; j++){
        rowFill = 1;
        for (int i = 0; i < 10; i++){
            if(floorBlocks[i][j] == 0){
                i = 10;
                rowFill = 0;
            }
        }
        if(rowFill == 1){
            for (int i = 0; i < 10; i++){
                floorBlocks[i][j] = 0;
            }
            tft_update_area(0, 24*j, 239, 24*(j+1)-1, RGB565(0,0,0));
            count++;
        }
    }
    for (int i = 0; i < count; i++){
        shiftDownPieces(floorBlocks);
    }

    int canMoveSideways = 1;
    int canMoveDown = 1;

    for (int j = 0; j < 13; j++){
        for (int i = 0; i < 10; i++){
            if(currentPieces[i][j] == 1){
                if((j - 1 < 0) | (floorBlocks[i][j-1] == 1)){
                    canMoveDown = 0;
                }if(i == 9 && left == 1){
                    canMoveSideways = 0;
                } else if((i + left > 9) | (i-right < 0) | (floorBlocks[i+left-right][j-1] == 1)){
                    canMoveSideways = 0;
                }
            }
        }
    }

    // for (int j = 0; j < 13; j++){
    //     for (int i = 0; i < 10; i++){
    //         if(currentPieces[i][j] == 1){
    //             if(rowOne == 1){
    //                 checkRowOne = j;
    //                 rowOne = 0;
    //             }
    //             if(checkRowOne == j){
    //                 if((i+left > 9) | (i-right < 0) | (floorBlocks[i+left-right][j-1] == 1)){
    //                     canMoveSideways = 0;
    //                 }
    //                 if((j - 1 < 0) | (floorBlocks[i][j-1] == 1)){
    //                     canMoveDown = 0;
    //                 }
    //             }else{
    //                 if((i+left > 9) | (i-right < 0) | (floorBlocks[i+left-right][j-1] == 1)){
    //                     canMoveSideways = 0;
    //                 }
    //             }
    //         }
    //     }
    // }
    int endGame = 0;
    if(canMoveDown == 1){
        shiftDownPieces(currentPieces);
    }else{
        if(*r == 4){
            *r = 0;
        }else{
            *r = *r + 1;
        }
        mergePieces(floorBlocks, currentPieces, r);
        for(int i = 0; i < 10; i++){
            if(floorBlocks[i][12] == 1){
                endGame = 1;
            }
        }
    }
    if(canMoveSideways == 1){
        shiftPiecesSideways(currentPieces, left, right);
    }
    update_screen(currentPieces);
    return endGame;

    // int stop = 0;
    // int canMoveDown = 1;
    // for (int j = 0; j < 13; j++){
    //     for (int i = 0; i < 10; i++){
    //         if(currentPieces[i][j] == 1){
    //             stop = 1;
    //             if(j - 1 < 0 | floorBlocks[i][j-1] == 1){
    //                 canMoveDown = 0;
    //             }
    //         }
    //     }
    //     if(stop == 1){
    //         j = 13;
    //     }
    // }
    // if(canMoveDown == 1){
    //     shiftDownPieces(currentPieces);
    //     update_screen(currentPieces);
    // }





    //Check if piece can fall, if can fall check right and left based on input.
    // - Check if piece array intersects boundaries or the floor blocks
    // - - If array intersects with floor merge the current piece via adding
    // - - Make new piece appear in array
    // - Update the screen such that piece and floor are showing
}


int main(void) {
    enable_ports();    // Enable GPIO ports for control pins
    init_spi();        // Initialize SPI peripheral
    tft_init();        // Initialize the TFT display

    // Fill the screen with a color (e.g., blue)
    // tft_fill_screen(RGB565(255, 0, 0)); // Fill with red
    // for (volatile int i = 0; i < 1000000; i++); // Delay
    // tft_fill_screen(RGB565(0, 255, 0)); // Fill with green
    // for (volatile int i = 0; i < 1000000; i++); // Delay
    // tft_fill_screen(RGB565(0, 0, 255)); // Fill with blue
    // for (volatile int i = 0; i < 1000000; i++); // Delay
    tft_fill_screen(RGB565(0, 0, 0));
    // tft_update_area(0, 0, 47, 47, RGB565(255, 0, 0));
    // tft_update_area(48, 0, 95, 47, RGB565(0, 0, 255));
    // tft_update_area(96, 0, 143, 47, RGB565(0, 255, 255));
    // int current_state[10][13] = {
    //     {1,0,1,0,1,0,1,0,1,0,1,0,1},
    //     {0,1,0,1,0,1,0,1,0,1,0,1,0},
    //     {1,0,1,0,1,0,1,0,1,0,1,0,1},
    //     {0,1,0,1,0,1,0,1,0,1,0,1,0},
    //     {1,0,1,0,1,0,1,0,1,0,1,0,1},
    //     {0,1,0,1,0,1,0,1,0,1,0,1,0},
    //     {1,0,1,0,1,0,1,0,1,0,1,0,1},
    //     {0,1,0,1,0,1,0,1,0,1,0,1,0},
    //     {1,0,1,0,1,0,1,0,1,0,1,0,1},
    //     {0,1,0,1,0,1,0,1,0,1,0,1,0}
    // };
    int floorBlocks[10][13] = {
        {0},
        {0},
        {1},
        {1},
        {1},
        {1},
        {1,1,1,1,},
        {1,1},
        {1,1},
        {1}
    };
    int currentPiece[10][13] = {
        {0},
        {0},
        {0},
        {0},
        {0,0,0,0,0,0,0,0,0,0,1,1,1},
        {0,0,0,0,0,0,0,0,0,0,1},
        {0},
        {0},
        {0},
        {0}
    };
    int r = 0;
    update_screen(floorBlocks);
    update_screen(currentPiece);
    // shiftDownPieces(current_state);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    for (volatile int i = 0; i < 1000000; i++); // Delay
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    for (volatile int i = 0; i < 1000000; i++); // Delay
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    for (volatile int i = 0; i < 1000000; i++); // Delay
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    for (volatile int i = 0; i < 1000000; i++); // Delay
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    for (volatile int i = 0; i < 1000000; i++); // Delay
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    runGameUpdate(0,1,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);
    runGameUpdate(0,0,floorBlocks, currentPiece, &r);



    
    // runGameUpdate(0,0,floorBlocks, currentPiece);
    // runGameUpdate(0,0,floorBlocks, currentPiece);
    // runGameUpdate(0,0,floorBlocks, currentPiece);
    // runGameUpdate(0,0,floorBlocks, currentPiece);
    //int floorBlocks[10][13];
    // runGameUpdate(1,0,floorBlocks,currentPiece);
    


    while (1) {
        // Loop infinitely - nothing else needs to be done here
    }
}



