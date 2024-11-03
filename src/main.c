#include "stm32f0xx.h"

// Define the TFT width and height
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

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
    GPIOB->AFR[1] |= (0x00 << 4); // Set alternate function for SCK
    GPIOB->AFR[1] |= (0x00 << 12); // Set alternate function for MOSI
}

void init_spi(void) {
    // Enable SPI2 clock
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    // Configure SPI2 for master mode, prescaler, software slave management, and enable SPI
    SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI2->CR1 |= SPI_CR1_SPE; // Enable SPI
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
    SPI2->DR = data;

    // Wait for transmission to complete
    while (SPI2->SR & SPI_SR_BSY);
    for (volatile int i = 0; i < 10; i++); // Small delay
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

    // Additional commands (based on common TFT initializations)
    tft_send_command(0x36); // Memory Access Control
    tft_send_data(0x48);    // Row/column addressing, RGB order

    tft_send_command(0x29); // Display ON
}

int main(void) {
    enable_ports();    // Enable GPIO ports for control pins
    init_spi();        // Initialize SPI peripheral
    tft_init();        // Initialize the TFT display

    // Fill the screen with a color (e.g., blue)
    tft_fill_screen(RGB565(255, 0, 0)); // Fill with red
    for (volatile int i = 0; i < 1000000; i++); // Delay
    tft_fill_screen(RGB565(0, 255, 0)); // Fill with green
    for (volatile int i = 0; i < 1000000; i++); // Delay
    tft_fill_screen(RGB565(0, 0, 255));

    while (1) {
        // Loop infinitely - nothing else needs to be done here
    }
}