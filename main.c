//* ECE243: Final Project *//
//* Stickman Volleyball *//
//* By Hui Lee & Alex Yeh *//

/* This files provides address values that exist in the system */
#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00
#define BLACK 0x0000

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240
	
/* Game object parameters */
#define BALL_SIZE 15

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

void enable_A9_interrupts();
void disable_A9_interrupts();
void config_GIC();
void config_interrupt (int N, int CPU_target);
void config_PS2s();
void PS2_ISR();
void HEX_PS2(char b1, char b2, char b3); 
void __attribute__ ((interrupt)) __cs3_isr_irq ();
void __attribute__ ((interrupt)) __cs3_reset ();
void __attribute__ ((interrupt)) __cs3_isr_undef ();
void __attribute__ ((interrupt)) __cs3_isr_swi ();
void __attribute__ ((interrupt)) __cs3_isr_pabort ();
void __attribute__ ((interrupt)) __cs3_isr_dabort ();
void __attribute__ ((interrupt)) __cs3_isr_fiq ();
void set_A9_IRQ_stack();

void clear_screen();
void wait_for_vsync();
void draw_circle(int x_centre, int y_centre, int radius, short int color);
	
volatile int pixel_buffer_start; // global variable
volatile char keyPressed; 
volatile char byte1, byte2, data;


int main(void) {
     int deltaOneX = 0; 
     int deltaOneY = 0; 
	 int locOneX = 100;
	 int locOneY = 100; 
	 int deltaTwoX = 0; 
     int deltaTwoY = 0; 
	 int locTwoX = 200;
	 int locTwoY = 200; 
     byte1 = 0;
     byte2 = 0; 
     data = 0;
     set_A9_IRQ_stack();
     config_GIC();
     config_PS2s();
     enable_A9_interrupts();
	
     volatile int* pixel_ctrl_ptr = (int*)0xFF203020; // addr of front buffer reg
     
    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
	
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
	
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    while(1){
	    deltaOneX = 0; 
     	deltaOneY = 0;
	    deltaTwoX = 0; 
     	deltaTwoY = 0;
	    clear_screen();
	    if(keyPressed=='W'){
			deltaOneX = 0;
			deltaOneY = -10;
			locOneY+=deltaOneY;
		}
	    if(keyPressed=='A'){
			deltaOneX = -10; 
			deltaOneY = 0;
			locOneX+=deltaOneX; 
		}
	    if(keyPressed=='S'){
			deltaOneX = 0;
			deltaOneY = 10;
			locOneY+=deltaOneY;
		}
	    if(keyPressed=='D'){
			deltaOneX = 10;
			deltaOneY = 0;
			locOneX+=deltaOneX; 
		}
	    if(keyPressed=='u'){
			deltaTwoX = 0;
			deltaTwoY = -10;
			locTwoY+=deltaTwoY;
		}
	    if(keyPressed=='l'){
			deltaTwoX = -10; 
			deltaTwoY = 0;
			locTwoX+=deltaTwoX; 
		}
	    if(keyPressed=='d'){
			deltaTwoX = 0;
			deltaTwoY = 10;
			locTwoY+=deltaTwoY;
		}
	    if(keyPressed=='r'){
			deltaTwoX = 10;
			deltaTwoY = 0;
			locTwoX+=deltaTwoX; 
		}
	    draw_circle(locOneX, locOneY, BALL_SIZE, ORANGE);
		draw_circle(locTwoX, locTwoY, BALL_SIZE, GREEN);
	    wait_for_vsync();
	    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
		keyPressed = '~';
   }
}

void plot_pixel(int x, int y, short int color) {
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = color;
}

void clear_screen() {
	for (int x = 0; x < RESOLUTION_X; x++) {
		for (int y = 0; y < RESOLUTION_Y; y++) plot_pixel(x, y, BLACK);
	}
}

void wait_for_vsync() {
	volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
	volatile int* status = (int*) 0xFF20302C;
	// lauch swap process
	(*pixel_ctrl_ptr) = 1; // sets S = 1
	// poll status bit
	while (*status & 0x01) {
		continue; 
	}
	// exit when status bit S = 0
}

void draw_circle(int x_centre, int y_centre, int radius, short int color) {
	for (int x = (x_centre-radius); x <= (x_centre+radius); x++) {
		for (int y = (y_centre-radius); y <= (y_centre+radius); y++) {
			// Use circle equation: (x-a)^2 + (y-b)^2 = r^2
			double check_r = sqrt(pow((x - x_centre), 2) + pow((y - y_centre), 2));
			if (check_r <= radius) plot_pixel(x, y, color);
		}
	}
	
}

void enable_A9_interrupts() {
	int status = 0b01010011;
	asm("msr cpsr, %[ps]" : : [ps]"r"(status));
}

void config_GIC() {
	config_interrupt(79, 1); //configure the PS2 keyboard parallel port

	// Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all priorities
	*((int *) 0xFFFEC104) = 0xFFFF;

	// Set CPU Interface Control Register (ICCICR). Enable signaling of interrupts
	*((int *) 0xFFFEC100) = 1;

	// Configure the Distributor Control Register (ICDDCR) to send pending interrupts to CPUs
	*((int *) 0xFFFED000) = 1;
}

void config_interrupt (int N, int CPU_target) {
	int reg_offset, index, value, address;

	/* Configure the Interrupt Set-Enable Registers (ICDISERn).
	* reg_offset = (integer_div(N / 32) * 4
	* value = 1 << (N mod 32) */
	reg_offset = (N >> 3) & 0xFFFFFFFC;
	index = N & 0x1F;
	value = 0x1 << index;
	address = 0xFFFED100 + reg_offset;

	/* Now that we know the register address and value, set the appropriate bit */
	*(int *)address |= value;

	/* Configure the Interrupt Processor Targets Register (ICDIPTRn)
	* reg_offset = integer_div(N / 4) * 4
	* index = N mod 4 */
	reg_offset = (N & 0xFFFFFFFC);
	index = N & 0x3;
	address = 0xFFFED800 + reg_offset + index;

	/* Now that we know the register address and value, write to (only) the appropriate byte */
	*(char *)address = (char) CPU_target;
}

void config_PS2s() {
	volatile int* PS2_ptr_interrupt = (int*)0xFF200104;
	*(PS2_ptr_interrupt) = 0x1; // enable interrupts for PS/2 by writing 1 to RE field at address 0xFF200104
}

void HEX_PS2(char b1, char b2, char b3) {
	volatile int * HEX3_HEX0_ptr = (int *)0xFF200020; 
	volatile int * HEX5_HEX4_ptr = (int *)0xFF200030; 
	
	unsigned char seven_seg_decode_table[] = {
		0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
	unsigned char hex_segs[] = {0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int shift_buffer, nibble; 
	unsigned char code;
	int i; 
	shift_buffer = (b1 << 16) | (b2 << 8) | b3;
	for(i = 0; i < 6; ++i) {
		nibble = shift_buffer & 0x0000000F;
		code   = seven_seg_decode_table[nibble];
		hex_segs[i]  = code;
		shift_buffer = shift_buffer >> 4;
	}
	*(HEX3_HEX0_ptr) =*(int*)(hex_segs);
	*(HEX5_HEX4_ptr) =*(int*)(hex_segs + 4);
}
void PS2_ISR(){
   //clean interrupt
   volatile int* PS2_ptr_interrupt = (int*)0xFF200104;
   *(PS2_ptr_interrupt) = 0b100000001;

	/* PS2 base address */
	volatile int *PS2_ptr = (int *) 0xFF200100;


	int PS2_data, RAVAIL, LED;
	//const int W = 0x1D, A = 0x1C, S = 0x1B, D = 0x23;

	PS2_data = *(PS2_ptr);
	RAVAIL = (PS2_data & 0x8000);
	
	LED = 0x01;
	if(RAVAIL > 0) {

		byte1 = byte2;
		byte2 = data;
		data = PS2_data & 0xFF;

		  //determine P1 movement (W/A/S/D)
			if(data == 0x1D) {
				LED = 0x1D;
				keyPressed = 'W';
			} else if(data == 0x1C) {
				LED = 0x1C;
				keyPressed = 'A';
			} else if(data == 0x1B) {
				LED = 0x1B;
				keyPressed = 'S';
			} else if(data == 0x23) {
				LED = 0x23;
				keyPressed = 'D';

			//determine P2 movement (up, down, left, right)
			} else if(data == 0x75) {
				LED = 0x75;
				keyPressed = 'u';
			} else if(data == 0x6B) {
				LED = 0x6B;
				keyPressed = 'l';
			} else if(data == 0x74) {
				LED = 0x74;
				keyPressed = 'r';
			} else if(data == 0x72) {
				LED = 0x72;
				keyPressed = 'd';
			//error handling
			} else {
				LED = 0xCFFF;
				keyPressed = '?';
			}
		//}

	}

		//printf("data: %c\n", data);
	HEX_PS2(byte1, byte2, data); 

	//printf("%c\n", keyPressed);
	return;
}


void __attribute__ ((interrupt)) __cs3_isr_irq () {
	int interrupt_ID = *((int *) 0xFFFEC10C); //Read the ICCIAR from the CPU Interface in the GIC
	if (interrupt_ID == 79) { // check if interrupt is from the PS/2
		PS2_ISR();
	}else {
		while(1);
	}

	// Write to the End of Interrupt Register (ICCEOIR)
	*((int *) 0xFFFEC110) = interrupt_ID;
}

void __attribute__ ((interrupt)) __cs3_reset () {
	while(1);
}

void __attribute__ ((interrupt)) __cs3_isr_undef () {
	while(1);
}

void __attribute__ ((interrupt)) __cs3_isr_swi () {
	while(1);
}

void __attribute__ ((interrupt)) __cs3_isr_pabort () {
	while(1);
}

void __attribute__ ((interrupt)) __cs3_isr_dabort () {
	while(1);
}

void __attribute__ ((interrupt)) __cs3_isr_fiq () {
	while(1);
}

void set_A9_IRQ_stack() {
	int stack, mode;
	stack = 0xFFFFFFFF - 7; // top of A9 onchip memory, aligned to 8 bytes

	/* change processor to IRQ mode with interrupts disabled */
	mode = 0b11010010;
	asm("msr cpsr, %[ps]" : : [ps] "r" (mode));

	/* set banked stack pointer */
	asm("mov sp, %[ps]" : : [ps] "r" (stack));

	/* go back to SVC mode before executing subroutine return! */
	mode = 0b11010011;
	asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
}
