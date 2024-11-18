// #include "stm32f0xx.h"
// #include <string.h> 
// #include <stdio.h>
// #include <math.h>   // for M_PI
// #include <stdint.h>
// #include <stdlib.h>
// #include <time.h>

// void set_char_msg(int, char);
// void nano_wait(unsigned int);
// void internal_clock();
// void playSound();
// void set_freq(int chan, float f);

// //===========================================================================
// // Configure GPIOC
// //===========================================================================
// void enable_portsNathan(void) {
//     // Only enable port C for the keypad
//     RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
//     GPIOC->MODER &= ~0xffff;
//     GPIOC->MODER |= 0x55 << (4*2);
//     GPIOC->OTYPER &= ~0xff;
//     GPIOC->OTYPER |= 0xf0;
//     GPIOC->PUPDR &= ~0xff;
//     GPIOC->PUPDR |= 0x55;
// }

// int scoree = 0;
// int currentLevel = 1;
// char curScore[9];
// uint8_t col; // the column being scanned

// void drive_column(int);   // energize one of the column outputs
// int  read_rows();         // read the four row inputs
// void update_history(int col, int rows); // record the buttons of the driven column
// char get_key_event(void); // wait for a button event (press or release)
// char get_keypress(void);  // wait for only a button press event.
// float getfloat(void);     // read a floating-point number from keypad
// void show_keys(void);     // demonstrate get_key_event()
// void print(const char str[]);
// void update_game_speed(int scoree);


// //===========================================================================
// // Bit Bang SPI LED Array
// //===========================================================================
// int msg_index = 0;
// uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };
// extern const char font[];

// void small_delay(void) {
//     nano_wait(50000);
// }

// void init_tim15(void) {
//     RCC -> APB2ENR |= 0x00010000;
//     TIM15 -> DIER |= 0x00000100;
//     TIM15 -> PSC = 24000 - 1;
//     TIM15 -> ARR = 2 - 1;
//     TIM15 -> CR1 |= 0x00000001;

// }

// void init_tim7(void) {
//   RCC->APB1ENR |= 0x00000020;
   
//   TIM7 -> PSC = 4800000 - 1;
//   TIM7 -> ARR = 60 - 1;

//   TIM7 -> DIER |= TIM_DIER_UIE;

//   NVIC -> ISER[0] = 1 << TIM7_IRQn;

//   TIM7 -> CR1 |= TIM_CR1_CEN;
// }

// void TIM7_IRQHandler(void)
// {
//     TIM7->SR &= ~TIM_SR_UIF;
//     int rows = read_rows();
//     update_history(col, rows);
//     col = (col + 1) & 3;
//     drive_column(col);

//     scoree = scoree + 1;
//     snprintf(curScore, 9, "%8d", scoree);
//     print(curScore);
//     // set_freq(0, 10000);
//     if(scoree == 120)
//     {
//         TIM7->CR1 &= ~TIM_CR1_CEN; //Stop main game system
//         // nano_wait(500000000);
//         // set_freq(0, 200);
//         // nano_wait(500000000);
//         // set_freq(0, 900);
//         // nano_wait(500000000);
//         currentLevel++;
//         update_game_speed(currentLevel);
//         //playSound();
//         TIM7->CR1 |= TIM_CR1_CEN; //Resume timing system

//     }
    
// }

// void update_game_speed(int level) {  //Updtes the game speed
    
//     if (level == 1) {
//         TIM7->PSC = 24000 - 1;  // Example values for level 1
//         TIM7 -> ARR = 50 - 1;
//     }
//     else {
//         TIM7->PSC = 24000 - 1;  // Higher frequency by reducing PSC
//         TIM7->ARR = 1000 - 1;
//     }

// }


// void playSound() {
//     set_freq(0, 400);
//     nano_wait(50000000); // Use a timer-based delay for better consistency
//     set_freq(0, 600);
//     nano_wait(50000000);
//     set_freq(0, 900);
//     nano_wait(50000000);
//     set_freq(0, 440);
//     nano_wait(50000000);
//     set_freq(0, 600);
//     nano_wait(50000000);
//     set_freq(0, 0); // Ensure sound is stopped after playing
// }


// void setup_tim1(void) {
//     // Generally the steps are similar to those in setup_tim3
//     // except we will need to set the MOE bit in BDTR. 
//     // Be sure to do so ONLY after enabling the RCC clock to TIM1.

//     // Enable clocks
//     RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
//     RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

//     //PA8-11 being set to 10 for alternative funtion mode
//     GPIOA -> MODER &= ~0x00ff0000;
//     GPIOA -> MODER |= 0x00aa0000;

//     //Set GPIOA8-11, contingent on last one working
//     // GPIOA->AFRH_AFR2[3:0] = 0001;
//     // GPIOA->AFRH_AFR3[3:0] = 0001;
//     // GPIOA->AFRH_AFR0[3:0] = 0001;
//     // GPIOA->AFRH_AFR1[3:0] = 0001;

//     GPIOA-> AFR[1] &= ~0x0000ffff;
//     GPIOA-> AFR[1] |= 0x00002222;

//     //Set MOE
//     TIM1 -> BDTR |= 0x8000;

//     //Set freq
//     TIM1 -> PSC = 1 - 1;
//     TIM1 -> ARR = 2400 - 1;

//     //Enable channel outputs
//     TIM1 -> CCMR1 &= 0x8f8f;
//     TIM1 -> CCMR1 |= 0x6060;
//     TIM1 -> CCMR2 &= 0x878f;
//     TIM1 -> CCMR2 |= 0x6860;
//     //Enabled the OC4CE in channel 4
//     TIM1 -> CCER |= TIM_CCER_CC3E | TIM_CCER_CC4E | TIM_CCER_CC2E | TIM_CCER_CC1E;

//     //enable timer
//     TIM1 -> CR1 |= TIM_CR1_CEN;








// }





// // Part 3: Analog-to-digital conversion for a volume level.
// uint32_t volume = 2400;

// // Variables for boxcar averaging.
// #define BCSIZE 32
// int bcsum = 0;
// int boxcar[BCSIZE];
// int bcn = 0;

// void dialer(void);

// // Parameters for the wavetable size and expected synthesis rate.
// #define N 1000
// #define RATE 20000
// short int wavetable[N];
// int step0 = 0;
// int offset0 = 0;
// int step1 = 0;
// int offset1 = 0;



// // void init_tim15(void) {
// //     RCC -> APB2ENR |= 0x00010000;
// //     TIM15 -> DIER |= 0x00000100;
// //     TIM15 -> PSC = 24000 - 1;
// //     TIM15 -> ARR = 2 - 1;
// //     TIM15 -> CR1 |= 0x00000001;

// // }

// uint8_t col; // the column being scanned

// // void drive_column(int);   // energize one of the column outputs
// // int  read_rows();         // read the four row inputs
// // void update_history(int col, int rows); // record the buttons of the driven column
// // char get_key_event(void); // wait for a button event (press or release)
// // char get_keypress(void);  // wait for only a button press event.
// // float getfloat(void);     // read a floating-point number from keypad
// // void show_keys(void);     // demonstrate get_key_event()

// void TIM6_DAC_IRQHandler(void)
//     {
//         TIM6 -> SR &= ~TIM_SR_UIF;
//         offset0 += step0;
//         offset1 += step1;
//         if (offset0 >= (N << 16))
//             offset0 -= (N << 16);
//         if (offset1 >= (N << 16))
//             offset1 -= (N << 16);

//         int samp = wavetable[offset0>>16] + wavetable[offset1>>16];
//         int sample = ((samp * volume)>>18) + 1200;
//         // samp *= volume;TIM1_CCR4 
//         // samp = samp >> 17;
//         // samp += 2048;
//         TIM1 -> CCR4  = sample;
//     }


// void init_tim6(void) {
//     RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
   
//     // TIM6 -> PSC = (24000 / (RATE)) - 1;
//     // TIM6 -> ARR = 2 - 1;

//     TIM6 -> PSC = 48 - 1;
//     TIM6 -> ARR = 1000000 / RATE - 1;

//     TIM6 -> DIER |= TIM_DIER_UIE;

//     NVIC -> ISER[0] = 1 << TIM6_DAC_IRQn;

//     //TIM6 -> CR2 |= 0x00000020;
//     //Bits 6-4 = 001

//     TIM6 -> CR1 |= TIM_CR1_CEN;
// }


// void init_wavetable(void) {
//     for(int i=0; i < N; i++)
//         wavetable[i] = 32767 * sin(2 * M_PI * i / N);
// }

// void set_freq(int chan, float f) {
//     if (chan == 0) {
//         if (f == 0.0) {
//             step0 = 0;
//             offset0 = 0;
//         } else
//             step0 = (f * N / RATE) * (1<<16);
//     }
//     if (chan == 1) {
//         if (f == 0.0) {
//             step1 = 0;
//             offset1 = 0;
//         } else
//             step1 = (f * N / RATE) * (1<<16);
//     }
// }

// void init_spi1(void) {
//      RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
//         GPIOA -> MODER &= ~0xc000cc00;
//         GPIOA->MODER |= 0x80008800;// << (4*6); //Set 5, 7, 15 to alternative function
//         GPIOA -> AFR[1] &= ~0xf0000000;
//         GPIOA -> AFR[0] &= ~0xf0f00000;
//         RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;
//     SPI1 -> CR1 &= ~(0x1 << 6);
//     // Set the baud rate as low as possible (maximum divisor for BR).
//     SPI1 -> CR1 |= 0x7 << 3; //For bits 3-5 = 1
// // Configure the interface for a 8-bit word size.
    
//     SPI1->CR2 = (0xF << 8); // Set 11-8 to 1001
// // Configure the SPI channel to be in "master configuration".
//     SPI1 -> CR1 |= 0x1 << 2; //Set bit 2 to 1
// // Set the SS Output enable bit and enable NSSP.
//     SPI1 -> CR2 |= 0x3 << 2; //Set 2nd and 3rd bit to 1
// // Set the TXDMAEN bit to enable DMA transfers on transmit buffer empty
//     SPI1 -> CR2 |= 0x1 << 1; // Set 1st bit to 1
// // Enable the SPI channel.
//     SPI1 -> CR1 |= 0x1 << 6; //Set bit 6 to 1

// }


// void spi1_setup_dma(void) {
    
//     //Turn off enable
//     DMA1_Channel3 -> CCR &= ~DMA_CCR_EN;
//     //0xfffffffe

//     //Activate
//     RCC -> AHBENR |= RCC_AHBENR_DMA1EN;
//     //CMAR
//     DMA1_Channel3 -> CMAR = (uint32_t) &msg;
//     //CPAR
//     DMA1_Channel3 -> CPAR = (uint32_t) &(SPI1->DR);
//     //CNDTR
//     DMA1_Channel3 -> CNDTR = 8;
//     //DIR
//      DMA1_Channel3 -> CCR |= DMA_CCR_DIR;//page 209 manual
//     //0x00000010

//     //MINC
//     DMA1_Channel3 -> CCR |= DMA_CCR_MINC;
//     DMA1_Channel3 -> CCR &= ~DMA_CCR_PINC;
//     //0x00000080

//     //Memory size
//     DMA1_Channel3 -> CCR |= DMA_CCR_MSIZE_0;
//     //0x00000400

//     //P datum
//     DMA1_Channel3 -> CCR |= DMA_CCR_PSIZE_0;
//     //0x00000100

//     //Circular mode

//     DMA1_Channel3 -> CCR |= DMA_CCR_CIRC;
//     //0x00000020

//     SPI1 -> CR2 |= 0x1 << 1; //Sets bit 1



// }



// //===========================================================================
// // Enable the DMA channel.
// //===========================================================================
// void spi1_enable_dma(void) {
//     DMA1_Channel3 -> CCR |= 0x00000001;
// }

// //===========================================================================
// // Main function
// //===========================================================================

// int main(void) {
//     internal_clock();

//     msg[0] |= font[' '];
//     msg[1] |= font[' '];
//     msg[2] |= font[' '];
//     msg[3] |= font[' '];
//     msg[4] |= font[' '];
//     msg[5] |= font[' '];
//     msg[6] |= font[' '];
//     msg[7] |= font[' '];

//     // GPIO enable
//     enable_portsNathan();
//     // setup keyboard
//     init_tim7();


//     init_spi1();
//     spi1_setup_dma();
//     spi1_enable_dma();
//     init_tim15();
//     //show_keys();
//     init_wavetable();
//     init_tim6();

//     setup_tim1();
//     //set_freq(0, 400);

// }

#include "stm32f0xx.h"
#include <string.h> 
#include <stdio.h>
#include <math.h>   // for M_PI
#include <stdint.h>
#include <stdlib.h>
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

    int floorBlocks[10][13] = {
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0}
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
        currentPieces[6][11] = 1;
        currentPieces[5][11] = 1;
        currentPieces[4][11] = 1;
        currentPieces[5][12] = 1;
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



void set_char_msg(int, char);
void nano_wait(unsigned int);
void internal_clock();
void playSound();
void set_freq(int chan, float f);

//===========================================================================
// Configure GPIOC
//===========================================================================
void enable_portsNathan(void) {
    // Only enable port C for the keypad
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER &= 0xffff0000; 
    GPIOC -> MODER |= 0x00005500;
    GPIOC -> PUPDR |= 0x000000aa; 
    // GPIOC->MODER &= ~0xffff;
    // GPIOC->MODER |= 0x55 << (4*2);
    // GPIOC->OTYPER &= ~0xff;
    // GPIOC->OTYPER |= 0xf0;
    // GPIOC->PUPDR &= ~0xff;
    // GPIOC->PUPDR |= 0x55;
}

int scoree = 1;
int currentLevel = 1;
char curScore[9];
uint8_t col; // the column being scanned

void drive_column(int c);
int read_rows();   // energize one of the column outputs
//int  read_rows();         // read the four row inputs
// void update_history(int col, int rows); // record the buttons of the driven column
// char get_key_event(void); // wait for a button event (press or release)
// char get_keypress(void);  // wait for only a button press event.
float getfloat(void);     // read a floating-point number from keypad
void handle_key(char key, int* left, int* right);
void show_keys(void);     // demonstrate get_key_event()
void print(const char str[]);
void update_game_speed(int scoree);
char rows_to_key(int rows);


//===========================================================================
// Bit Bang SPI LED Array
//===========================================================================
int msg_index = 0;
uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };
extern const char font[];

void small_delay(void) {
    nano_wait(50000);
}
int left = 0;
int right = 0;

// void TIM15_IRQHandler(void)
// {
//     TIM15->SR &= ~TIM_SR_UIF;
//     int rows = read_rows();
//     if (rows != 0)
//     {
//         char newKey = rows_to_key(rows);
//         handle_key(newKey, &left, &right);
//     }
//     snprintf(curScore, 9, "%8s", "LEUEL   ");
//     print(curScore);
//     nano_wait(1000000000 / 4);
//     // int rows = read_rows();
//     // update_history(0, rows);
//     //col = (col + 1) & 3;
//     //col = 0;
//     //drive_column(col);
// }

// void init_tim15(void) {
//     RCC -> APB2ENR |= 0x00010000;
//     TIM15 -> DIER |= 0x00000100;
//     TIM15 -> PSC = 4800000 - 1;
//     TIM15 -> ARR = 50 - 1;
//     NVIC -> ISER[0] = 1 << TIM15_IRQn;
//     TIM15 -> CR1 |= 0x00000001;

// }

void init_tim7(void) {
  RCC->APB1ENR |= 0x00000020;
   
  TIM7 -> PSC = 4800000 - 1;
  TIM7 -> ARR = 200 - 1;

  TIM7 -> DIER |= TIM_DIER_UIE;

  NVIC -> ISER[0] = 1 << TIM7_IRQn;

  TIM7 -> CR1 |= TIM_CR1_CEN;
}

void TIM7_IRQHandler(void)
{
    TIM7->SR &= ~TIM_SR_UIF;
    // int rows = read_rows();
    // update_history(col, rows, &left, &right);
    // col = (col + 1) & 3;
    // drive_column(col);
    int rows = read_rows();
    if (rows != 0)
    {
        char newKey = rows_to_key(rows);
        handle_key(newKey, &left, &right);
    }
    else
    {
        right = 0;
        left = 0;
    }


    scoree++;
    snprintf(curScore, 9, "%8d", scoree);
    print(curScore);
    // set_freq(0, 10000);
    if(scoree % 100 == 0)
    {
        TIM7->CR1 &= ~TIM_CR1_CEN; //Stop main game system
        // nano_wait(500000000);
        // set_freq(0, 200);
        // nano_wait(500000000);
        // set_freq(0, 900);
        // nano_wait(500000000);
        snprintf(curScore, 9, "%8s", "LEUEL   ");
        print(curScore);
        nano_wait(1000000000 / 4);
        snprintf(curScore, 9, "%8s", "UP      ");
        print(curScore);
        nano_wait(1000000000 /4);
        currentLevel++;
        update_game_speed(currentLevel);
        playSound();
        TIM7->CR1 |= TIM_CR1_CEN; //Resume timing system

    }
    int track = runGameUpdate(left,right,floorBlocks, currentPiece, &r);
    if(track)
    {
       TIM7->CR1 &= ~TIM_CR1_CEN; 
       while(1)
       {
        snprintf(curScore, 9, "%8s", "LOSER   ");
        print(curScore);
        nano_wait(1000000000 / 4);
        snprintf(curScore, 9, "%8s", "SCORE   ");
        print(curScore);
        nano_wait(1000000000 / 4);
        snprintf(curScore, 9, "%8d", scoree);
        print(curScore);
        nano_wait(1000000000 / 4);

       }

    }
    
}

void update_game_speed(int level) {  //Updtes the game speed
    
    if (level == 1) {
        TIM7->PSC = 24000 - 1;  // Example values for level 1
        TIM7 -> ARR = 500 - 1;
    }
    else if(level == 2)
    {
        TIM7->PSC = 24000 - 1;  // Example values for level 1
        TIM7 -> ARR = 100 - 1;
    }
    else {
        TIM7->PSC = 24000 - 1;  // Higher frequency by reducing PSC
        TIM7->ARR = 60 - 1;
    }

}


void playSound() {
    TIM1 -> CR1 |= TIM_CR1_CEN;
    set_freq(0, 400);
    nano_wait(200000000); // Use a timer-based delay for better consistency
    TIM1 -> CR1 &= ~TIM_CR1_CEN;
}


void setup_tim1(void) {
    // Generally the steps are similar to those in setup_tim3
    // except we will need to set the MOE bit in BDTR. 
    // Be sure to do so ONLY after enabling the RCC clock to TIM1.

    // Enable clocks
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    //PA8-11 being set to 10 for alternative funtion mode
    GPIOA -> MODER &= ~0x00ff0000;
    GPIOA -> MODER |= 0x00aa0000;

    //Set GPIOA8-11, contingent on last one working
    // GPIOA->AFRH_AFR2[3:0] = 0001;
    // GPIOA->AFRH_AFR3[3:0] = 0001;
    // GPIOA->AFRH_AFR0[3:0] = 0001;
    // GPIOA->AFRH_AFR1[3:0] = 0001;

    GPIOA-> AFR[1] &= ~0x0000ffff;
    GPIOA-> AFR[1] |= 0x00002222;

    //Set MOE
    TIM1 -> BDTR |= 0x8000;

    //Set freq
    TIM1 -> PSC = 1 - 1;
    TIM1 -> ARR = 2400 - 1;

    //Enable channel outputs
    TIM1 -> CCMR1 &= 0x8f8f;
    TIM1 -> CCMR1 |= 0x6060;
    TIM1 -> CCMR2 &= 0x878f;
    TIM1 -> CCMR2 |= 0x6860;
    //Enabled the OC4CE in channel 4
    TIM1 -> CCER |= TIM_CCER_CC3E | TIM_CCER_CC4E | TIM_CCER_CC2E | TIM_CCER_CC1E;

    //enable timer
    //TIM1 -> CR1 |= TIM_CR1_CEN;








}





// Part 3: Analog-to-digital conversion for a volume level.
uint32_t volume = 1200;

// Variables for boxcar averaging.
#define BCSIZE 32
int bcsum = 0;
int boxcar[BCSIZE];
int bcn = 0;

void dialer(void);

// Parameters for the wavetable size and expected synthesis rate.
#define N 1000
#define RATE 20000
short int wavetable[N];
int step0 = 0;
int offset0 = 0;
int step1 = 0;
int offset1 = 0;



// void init_tim15(void) {
//     RCC -> APB2ENR |= 0x00010000;
//     TIM15 -> DIER |= 0x00000100;
//     TIM15 -> PSC = 24000 - 1;
//     TIM15 -> ARR = 2 - 1;
//     TIM15 -> CR1 |= 0x00000001;

// }

uint8_t col; // the column being scanned

// void drive_column(int);   // energize one of the column outputs
// int  read_rows();         // read the four row inputs
// void update_history(int col, int rows); // record the buttons of the driven column
// char get_key_event(void); // wait for a button event (press or release)
// char get_keypress(void);  // wait for only a button press event.
// float getfloat(void);     // read a floating-point number from keypad
// void show_keys(void);     // demonstrate get_key_event()

void TIM6_DAC_IRQHandler(void)
    {
        TIM6 -> SR &= ~TIM_SR_UIF;
        offset0 += step0;
        offset1 += step1;
        if (offset0 >= (N << 16))
            offset0 -= (N << 16);
        if (offset1 >= (N << 16))
            offset1 -= (N << 16);

        int samp = wavetable[offset0>>16] + wavetable[offset1>>16];
        int sample = ((samp * volume)>>18) + 1200;
        // samp *= volume;TIM1_CCR4 
        // samp = samp >> 17;
        // samp += 2048;
        TIM1 -> CCR4  = sample;
    }


void init_tim6(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
   
    // TIM6 -> PSC = (24000 / (RATE)) - 1;
    // TIM6 -> ARR = 2 - 1;

    TIM6 -> PSC = 48 - 1;
    TIM6 -> ARR = 1000000 / RATE - 1;

    TIM6 -> DIER |= TIM_DIER_UIE;

    NVIC -> ISER[0] = 1 << TIM6_DAC_IRQn;

    //TIM6 -> CR2 |= 0x00000020;
    //Bits 6-4 = 001

    TIM6 -> CR1 |= TIM_CR1_CEN;
}


void init_wavetable(void) {
    for(int i=0; i < N; i++)
        wavetable[i] = 32767 * sin(2 * M_PI * i / N);
}

void set_freq(int chan, float f) {
    if (chan == 0) {
        if (f == 0.0) {
            step0 = 0;
            offset0 = 0;
        } else
            step0 = (f * N / RATE) * (1<<16);
    }
    if (chan == 1) {
        if (f == 0.0) {
            step1 = 0;
            offset1 = 0;
        } else
            step1 = (f * N / RATE) * (1<<16);
    }
}

void init_spi1(void) {
     RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
        GPIOA -> MODER &= ~0xc000cc00;
        GPIOA->MODER |= 0x80008800;// << (4*6); //Set 5, 7, 15 to alternative function
        GPIOA -> AFR[1] &= ~0xf0000000;
        GPIOA -> AFR[0] &= ~0xf0f00000;
        RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1 -> CR1 &= ~(0x1 << 6);
    // Set the baud rate as low as possible (maximum divisor for BR).
    SPI1 -> CR1 |= 0x7 << 3; //For bits 3-5 = 1
// Configure the interface for a 8-bit word size.
    
    SPI1->CR2 = (0xF << 8); // Set 11-8 to 1001
// Configure the SPI channel to be in "master configuration".
    SPI1 -> CR1 |= 0x1 << 2; //Set bit 2 to 1
// Set the SS Output enable bit and enable NSSP.
    SPI1 -> CR2 |= 0x3 << 2; //Set 2nd and 3rd bit to 1
// Set the TXDMAEN bit to enable DMA transfers on transmit buffer empty
    SPI1 -> CR2 |= 0x1 << 1; // Set 1st bit to 1
// Enable the SPI channel.
    SPI1 -> CR1 |= 0x1 << 6; //Set bit 6 to 1

}


void spi1_setup_dma(void) {
    
    //Turn off enable
    DMA1_Channel3 -> CCR &= ~DMA_CCR_EN;
    //0xfffffffe

    //Activate
    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;
    //CMAR
    DMA1_Channel3 -> CMAR = (uint32_t) &msg;
    //CPAR
    DMA1_Channel3 -> CPAR = (uint32_t) &(SPI1->DR);
    //CNDTR
    DMA1_Channel3 -> CNDTR = 8;
    //DIR
     DMA1_Channel3 -> CCR |= DMA_CCR_DIR;//page 209 manual
    //0x00000010

    //MINC
    DMA1_Channel3 -> CCR |= DMA_CCR_MINC;
    DMA1_Channel3 -> CCR &= ~DMA_CCR_PINC;
    //0x00000080

    //Memory size
    DMA1_Channel3 -> CCR |= DMA_CCR_MSIZE_0;
    //0x00000400

    //P datum
    DMA1_Channel3 -> CCR |= DMA_CCR_PSIZE_0;
    //0x00000100

    //Circular mode

    DMA1_Channel3 -> CCR |= DMA_CCR_CIRC;
    //0x00000020

    SPI1 -> CR2 |= 0x1 << 1; //Sets bit 1



}



//===========================================================================
// Enable the DMA channel.
//===========================================================================
void spi1_enable_dma(void) {
    DMA1_Channel3 -> CCR |= 0x00000001;
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
    update_screen(floorBlocks);
    update_screen(currentPiece);
    msg[0] |= font[' '];
    msg[1] |= font[' '];
    msg[2] |= font[' '];
    msg[3] |= font[' '];
    msg[4] |= font[' '];
    msg[5] |= font[' '];
    msg[6] |= font[' '];
    msg[7] |= font[' '];

    // GPIO enable
    enable_portsNathan();
    // setup keyboard
    init_tim7();


    init_spi1();
    spi1_setup_dma();
    spi1_enable_dma();
    //init_tim15();
    //show_keys();
    init_wavetable();
    drive_column(0);
    init_tim6();

    setup_tim1(); 
   //TIM1 -> CR1 |= TIM_CR1_CEN;
    //set_freq(0, 400);   


    while (1) {
        // Loop infinitely - nothing else needs to be done here
    }
}