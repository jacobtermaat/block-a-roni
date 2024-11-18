#include "stm32f0xx.h"
#include <string.h> // for memmove()
#include <stdlib.h> // for srandom() and random()
#include <stdio.h>

void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

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

extern uint16_t msg[8];

void set_digit_segments(int digit, char val) {
    msg[digit] = (digit << 8) | val;
}

void print(const char str[])
{
    const char *p = str;
    for(int i=0; i<8; i++) {
        if (*p == '\0') {
            msg[i] = (i<<8);
        } else {
            msg[i] = (i<<8) | font[*p & 0x7f] | (*p & 0x80);
            p++;
        }
    }
}

void printfloat(float f)
{
    char buf[10];
    snprintf(buf, 10, "%f", f);
    for(int i=1; i<10; i++) {
        if (buf[i] == '.') {
            // Combine the decimal point into the previous digit.
            buf[i-1] |= 0x80;
            memcpy(&buf[i], &buf[i+1], 10-i-1);
        }
    }
    print(buf);
}

void append_segments(char val) {
    for (int i = 0; i < 7; i++) {
        set_digit_segments(i, msg[i+1] & 0xff);
    }
    set_digit_segments(7, val);
}

void clear_display(void) {
    for (int i = 0; i < 8; i++) {
        msg[i] = msg[i] & 0xff00;
    }
}

// 16 history bytes.  Each byte represents the last 8 samples of a button.
uint8_t hist[16];
char queue[2];  // A two-entry queue of button press/release events.
int qin;        // Which queue entry is next for input
int qout;       // Which queue entry is next for output

const char keymap[] = "DCBA#9630852*741";

void push_queue(int n) {
    queue[qin] = n;
    qin ^= 1;
}

char pop_queue() {
    char tmp = queue[qout];
    queue[qout] = 0;
    qout ^= 1;
    return tmp;
}

void update_history(int c, int rows)
{
    // We used to make students do this in assembly language.
    for(int i = 0; i < 4; i++) {
        hist[4*c+i] = (hist[4*c+i]<<1) + ((rows>>i)&1);
        if (hist[4*c+i] == 0x01)
            push_queue(0x80 | keymap[4*c+i]);
        if (hist[4*c+i] == 0xfe)
            push_queue(keymap[4*c+i]);
    }
}

void drive_column(int c)
{
    GPIOC->BSRR = 0xf00000 | ~(1 << (c + 4));
}

int read_rows()
{
    return (~GPIOC->IDR) & 0xf;
}

char get_key_event(void) {
    for(;;) {
        asm volatile ("wfi");   // wait for an interrupt
        if (queue[qout] != 0)
            break;
    }
    return pop_queue();
}

char get_keypress() {
    char event;
    for(;;) {
        // Wait for every button event...
        event = get_key_event();
        // ...but ignore if it's a release.
        if (event & 0x80)
            break;
    }
    return event & 0x7f;
}

void show_keys(void)
{
    char buf[] = "        ";
    for(;;) {
        char event = get_key_event();
        memmove(buf, &buf[1], 7);
        buf[7] = event;
        print(buf);
    }
}

// Turn on the dot of the rightmost display element.
void dot()
{
    msg[7] |= 0x80;
}

extern uint16_t display[34];

int score = 0;
char disp1[17] = "                ";
char disp2[17] = "                ";
volatile int pos = 0;



void init_spi2(void);
void spi2_setup_dma(void);
void spi2_enable_dma(void);

//Just in case

/*

//===========================================================================
// Configure PB12 (CS), PB13 (SCK), and PB15 (SDI) for outputs
//===========================================================================
// void setup_bb(void) {
//     RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
//     // PA15 (Represents CS/NSS)
//     // PA5 (Represents SCK)
//     // PA7 (Represents SDI/MOSI)
//     GPIOA->MODER &= ~0xc000cc00; //reset MODER
//     //GPIOB->MODER |=  ( (0x1 << (2*12)) | (0x1 << (2*13)) | (0x1 << (2*15)) );
//     GPIOA->MODER |= 0x40004400;// << (4*6); //Set 12, 13, 15 to otuput
//     GPIOA -> BSRR = GPIO_BSRR_BS_15 | GPIO_BSRR_BR_5;// 0x10002000; //set 12 to 1, set 13 to 0
    
// }


// //===========================================================================
// // Set the MOSI bit, then set the clock high and low.
// // Pause between doing these steps with small_delay().
// //===========================================================================
// void bb_write_bit(int val) {
//     // Set SDI to 0 or 1 based on out
//     GPIOA -> BSRR |= (val != 0 ? 0x00800000 : 0x00000080);
//     small_delay();
//     GPIOA -> BSRR |=  0x00200000;
//     small_delay();
//     GPIOA -> BSRR |=  0x00000020;
//   // Set SCK to 0
//     // CS (PB12)
//     // SCK (PB13)
//     // SDI (PB15)
    
// }

// void bb_write_halfword(int halfword) {
//     GPIOA -> BSRR |=  0x00008000; // Set CS to 0
//     for(int i = 15; i >= 0; i--)// Call bb_write_bit() for bit 15-0
//     {
//         int bitAtHand = (halfword >> i) & 0x1;
//         bb_write_bit(bitAtHand);
//     }
//     GPIOA -> BSRR |=  0x80000000; // Set CS to 1
// }


// void drive_bb(void) {
//     for(;;)
//         for(int d=0; d<8; d++) {
//             bb_write_halfword(msg[d]);
//             nano_wait(1000000); // wait 1 ms between digits
//         }
// }
*/
