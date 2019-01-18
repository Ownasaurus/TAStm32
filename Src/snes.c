#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "snes.h"

void my_wait_us_asm(int n);

void nes_write_1()
{
	GPIOA->BSRR = (1 << 24);
	my_wait_us_asm(6);
	GPIOA->BSRR = (1 << 8);
    my_wait_us_asm(6);
}

void nes_write_0()
{
    my_wait_us_asm(12);
}

// TODO: this will change to an interrupt-based state machine to always match the clock as precisely as possible
void SendControllerDataNES(uint8_t data)
{
	for(int8_t b = 7;b >=0;b--) // for each bit in the byte
	{
		if((data >> b) & 1) // if the button is pressed
		{
			nes_write_0(); // send logical low
		}
		else
		{
			nes_write_1(); // send logical high
		}
	}
}
