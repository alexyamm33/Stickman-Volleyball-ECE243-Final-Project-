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

void clear_screen();
void wait_for_vsync();
void draw_circle(int x_centre, int y_centre, int radius, short int color);
	
volatile int pixel_buffer_start; // global variable

int main(void) {
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
	clear_screen(); // pixel_buffer_start points to the pixel buffer
	
	draw_circle(100, 100, BALL_SIZE, ORANGE);
	wait_for_vsync();
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
	int status;
	// lauch swap process
	*pixel_ctrl_ptr = 1; // sets S = 1
	// poll status bit
	status = *(pixel_ctrl_ptr + 3); // dereference status reg
	while ((status & 0x01)!=0) {
		status = *(pixel_ctrl_ptr+3);
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
	
