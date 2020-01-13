#ifndef __N64__H
#define __N64__H

#include "main.h"

typedef struct __attribute__((packed))
{
	unsigned int right : 1; // low bit of 1st byte
	unsigned int left : 1;
	unsigned int down : 1;
	unsigned int up : 1;
	unsigned int start : 1;
	unsigned int z : 1;
	unsigned int b : 1;
	unsigned int a : 1; // high bit of 1st byte

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

} N64ControllerData;

typedef struct __attribute__((packed))
{
	unsigned int a : 1;
	unsigned int b : 1;
	unsigned int x : 1; // 1 bit wide
	unsigned int y : 1;
	unsigned int start : 1;
	unsigned int beginning_zeros : 3;

    unsigned int d_left : 1;
    unsigned int d_right : 1;
    unsigned int d_down : 1;
    unsigned int d_up : 1;
    unsigned int z : 1;
    unsigned int r : 1;
    unsigned int l : 1;
    unsigned int beginning_one : 1;

    uint8_t a_x_axis;
    uint8_t a_y_axis;
    uint8_t c_x_axis;
    uint8_t c_y_axis;
    uint8_t l_trigger;
    uint8_t r_trigger;

} GCControllerData; // all bits are in the correct order... except for the analog


maybe_unused static void GCN64_SetPortInput()
{
	// port C4 to input mode
	const uint32_t MODER_SLOT = (P1_DATA_2_Pin*P1_DATA_2_Pin);
	const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
	const uint32_t MODER_NEW_VALUE = GPIO_MODE_INPUT * MODER_SLOT;

	P1_DATA_2_GPIO_Port->MODER = (P1_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
}

maybe_unused static void GCN64_SetPortOutput()
{
	// port C4 to output mode
	const uint32_t MODER_SLOT = (P1_DATA_2_Pin*P1_DATA_2_Pin);
	const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
	const uint32_t MODER_NEW_VALUE = GPIO_MODE_OUTPUT_PP * MODER_SLOT;
	P1_DATA_2_GPIO_Port->MODER = (P1_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
}

maybe_unused static void GCN64_Send0()
{
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin<<16;
	my_wait_us_asm(3);
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin;
	my_wait_us_asm(1);
}
maybe_unused static void GCN64_Send1()
{
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin<<16;
	my_wait_us_asm(1);
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin;
	my_wait_us_asm(3);
}
maybe_unused static void GCN64_SendStop()
{
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin<<16;
	my_wait_us_asm(1);
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin;
}
maybe_unused static void GCN64_SendData(uint8_t *data, uint8_t bytes)
{
	while(bytes){
		uint8_t d = *data;
		for(uint8_t b=0; b<8; ++b){
			(d & 0x80) ? GCN64_Send1() : GCN64_Send0();
			d <<= 1;
		}
		++data;
		--bytes;
	}
	GCN64_SendStop();
}

uint32_t GCN64_ReadCommand();
void N64_SendIdentity();
void GCN_SendIdentity();
void GCN_SendOrigin();

#endif
