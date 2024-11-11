#include "stm32f0xx.h"
// #include <stdio.h>
// #include <stdint.h>

// Define the TFT width and height
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
int16_t gameScore          = 0;
int16_t currentLevel          = 0;

void nano_wait(unsigned int n);
void playSound(void);
void play_note(int freq, int duration);
void update_game_speed(int level);
void updateGameScoreBuffer(int score);
void init_spi1(void);
void spi1_setup_dma(void);
void spi1_enable_dma(void);

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


    //Set PB4 for Alternative function for PWM
    GPIOB -> MODER &= 0xfffffcff;
    GPIOB -> MODER |= 0x00000200;
    GPIOB-> AFR[0] &= ~0x000f0000;
    GPIOB-> AFR[0] |= 0x00010000; //Set Tim3 for AF1


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



// //Nathan's section

const char font[] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x00, // 32: space
    0x86, // 33: exclamation
    0x22, // 34: double quote
    0x76, // 35: octothorpe
    0x00, // dollar
    0x00, // percent
    0x00, // ampersand
    0x20, // 39: single quote
    0x39, // 40: open paren
    0x0f, // 41: close paren
    0x49, // 42: asterisk
    0x00, // plus
    0x10, // 44: comma
    0x40, // 45: minus
    0x80, // 46: period
    0x00, // slash
    // digits
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67,
    // seven unknown
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    // Uppercase
    0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x6f, 0x76, 0x30, 0x1e, 0x00, 0x38, 0x00,
    0x37, 0x3f, 0x73, 0x7b, 0x31, 0x6d, 0x78, 0x3e, 0x00, 0x00, 0x00, 0x6e, 0x00,
    0x39, // 91: open square bracket
    0x00, // backslash
    0x0f, // 93: close square bracket
    0x00, // circumflex
    0x08, // 95: underscore
    0x20, // 96: backquote
    // Lowercase
    0x5f, 0x7c, 0x58, 0x5e, 0x79, 0x71, 0x6f, 0x74, 0x10, 0x0e, 0x00, 0x30, 0x00,
    0x54, 0x5c, 0x73, 0x7b, 0x50, 0x6d, 0x78, 0x1c, 0x00, 0x00, 0x00, 0x6e, 0x00
};

uint8_t gameScoreBuffer[8];  // Buffer for DMA transfer to SPI

// 7-segment encoding table for digits 0-9
// Example encoding: 0x3F for "0", 0x06 for "1", etc.
const uint8_t segmentEncoding[10] = {
    0x3F,  // 0
    0x06,  // 1
    0x5B,  // 2
    0x4F,  // 3
    0x66,  // 4
    0x6D,  // 5
    0x7D,  // 6
    0x07,  // 7
    0x7F,  // 8
    0x6F   // 9
};

void updateGameScoreBuffer(int score) {
    //Convert score to an 8-digit representation, e.g., "00000019"
//     for (int i = 7; i >= 0; i--) {
//         int digit = score % 10;                 // Extract the last digit
//         gameScoreBuffer[i] = segmentEncoding[digit];  // Convert to 7-segment encoding
//         score /= 10;                            // Move to the next digit
//     }
        // gameScoreBuffer[0] &= 0x00;
        // gameScoreBuffer[1] &= 0x00;
        // gameScoreBuffer[2] &= 0x00;
        // gameScoreBuffer[3] &= 0x00;
        gameScoreBuffer[0] |= font['1'];
        gameScoreBuffer[1] |= font['5'];
        gameScoreBuffer[2] |= font['4'];
        gameScoreBuffer[3] |= font['3'];
        gameScoreBuffer[4] |= font['2'];
        gameScoreBuffer[5] |= font['2'];
        gameScoreBuffer[6] |= font['9'];
        gameScoreBuffer[7] |= font['1'];
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
    SPI1->CR2 = (0x7 << 8); // Set 11-8 to 1001
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
    DMA1_Channel3 -> CMAR = (uint32_t) &gameScoreBuffer;
    //CPAR
    DMA1_Channel3 -> CPAR = (uint32_t) &(SPI1->DR);
    //CNDTR
    DMA1_Channel3 -> CNDTR = 8;
    //DIR
     DMA1_Channel3 -> CCR |= DMA_CCR_DIR;//page 209 manual
    //0x00000010

    //MINC
    DMA1_Channel3 -> CCR |= DMA_CCR_MINC;
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

void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

void TIM7_IRQHandler(void)
{
  TIM7->SR &= ~TIM_SR_UIF;
  gameScore++;
  updateGameScoreBuffer(gameScore);
  if(gameScore == 1000)
  {
    TIM7->CR1 &= ~TIM_CR1_CEN; //Stop main game system
    currentLevel++;
    update_game_speed(currentLevel);
    playSound();
    TIM7->CR1 |= TIM_CR1_CEN; //Resume timing system

  }
}

void playSound() {
    play_note(440, 0.2);
    play_note(580, 0.1);
    nano_wait(50000);
    play_note(440, 0.2);
    play_note(600, 2);
}

void play_note(int freq, int duration){  //MAKE SURE THE JACK IS ASSOSIATED WITH CCR3, change accordingly if not
    //PB4 AF1 = Timer 3 channel 1
    TIM3->CR1 &= ~TIM_CR1_CEN;

    // Calculate the ARR and CCR3 values based on the desired frequency
    int arr_value = (48000000 / freq) - 1;
    // TIM7 -> PSC = 24000 - 1;
    // TIM7 -> ARR = 2 - 1;
    TIM3->ARR = arr_value;
    TIM3 -> CCMR1 &= 0xff8f;
    TIM3 -> CCMR1 |= 0x0060; //PWM mode 1 for channel 1
    TIM3 -> CCER |= TIM_CCER_CC1E; //Enable CCR channel
    TIM3->CR1 |= TIM_CR1_CEN; //Enable timer
    TIM3->CCR1 = (arr_value + 1) / 2;  // 50% duty cycle for a balanced square wave

    // Wait for the duration
    nano_wait(duration* 1000000);

    // Stop the PWM after the duration
    TIM3->CR1 &= ~TIM_CR1_CEN;
}



void setup_tim7() { //Sets up our main gametiming system
  
  RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
   
  TIM7 -> PSC = 2400000 - 1;
  TIM7 -> ARR = 2 - 1;

  TIM7->DIER |= TIM_DIER_UIE;

  NVIC -> ISER[0] = 1 << TIM7_IRQn;

  TIM7 -> CR1 |= TIM_CR1_CEN;
}

void update_game_speed(int level) {  //Updtes the game speed
    
    if (level == 1) {
        TIM7->PSC = 24000 - 1;  // Example values for level 1
        TIM7->ARR = 2 - 1;
    }
    else if (level ==2) {
        TIM7->PSC = 12000 - 1;  // Higher frequency by reducing PSC
        TIM7->ARR = 2 - 1;
    }

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

    // gameScoreBuffer[0] |= font['6'];
    // gameScoreBuffer[1] |= font['5'];
    // gameScoreBuffer[2] |= font['4'];
    // gameScoreBuffer[3] |= font['3'];
    // gameScoreBuffer[4] |= font['2'];
    // gameScoreBuffer[5] |= font['2'];
    // gameScoreBuffer[6] |= font['9'];
    // gameScoreBuffer[7] |= font['1'];

    init_spi1();
    setup_tim7();
    spi1_setup_dma();
    spi1_enable_dma();


    while (1) {
        // Loop infinitely - nothing else needs to be done here
    }
}


