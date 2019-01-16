#ifndef __N64__H
#define __N64__H

#include <stdint.h>

typedef struct __attribute__((packed))
{
	unsigned int right : 1;
	unsigned int left : 1;
	unsigned int down : 1;
	unsigned int up : 1;
	unsigned int start : 1;
	unsigned int z : 1;
	unsigned int b : 1;
	unsigned int a : 1;

	unsigned int c_right : 1;
	unsigned int c_left : 1;
	unsigned int c_down : 1;
	unsigned int c_up : 1;
	unsigned int r : 1;
    unsigned int l : 1;
    unsigned int dummy1 : 1;
    unsigned int dummy2 : 1;

    char x_axis;

    char y_axis;

} N64ControllerData; // all bits are in the correct order... except for the analog

void initialize_n64_buffer();
uint32_t readCommand();
uint8_t GetMiddleOfPulse();
void SendIdentityN64();
void SetN64DataInputMode();
void SetN64DataOutputMode();
void write_1();
void write_0();
void SendStop();
void SendByte(unsigned char b);
void SendControllerDataN64();

#endif
