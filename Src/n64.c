#include <stdint.h>
#include <string.h>
#include "n64.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "TASRun.h"

void my_wait_us_asm(int n);
void my_wait_100ns_asm(int n);

static uint8_t GetMiddleOfPulse(uint8_t player);
static void SendByte(uint8_t player, unsigned char b);
uint8_t GCN64_ReadBit(uint8_t player);
void N64_SendIdentity(uint8_t player);
void write_0(uint8_t player);
void write_1(uint8_t player);
void SendStop(uint8_t player);

inline uint8_t GCN64_ReadBit(uint8_t player)
{
	if(player == 1)
	{
		return (P1_DATA_2_GPIO_Port->IDR & P1_DATA_2_Pin) ? 1U : 0U;
	}
	else if(player == 2)
	{
		return (P2_DATA_2_GPIO_Port->IDR & P2_DATA_2_Pin) ? 1U : 0U;
	}
	else
	{
		return 2U; // unsupported
	}
}

uint32_t GCN64_ReadCommand(uint8_t player)
{
	uint8_t retVal;

	// we are already at the first falling edge
	// get middle of first pulse, 2us later
	// HOWEVER, shorten the delay slightly to account for function call
	// overhead, ISR overhead, etc.
	// Fix by Sauraen
	my_wait_100ns_asm(15);

	uint32_t command = GCN64_ReadBit(player), bits_read = 1;

    while(1) // read at least 9 bits (1 byte + stop bit)
    {
        command = command << 1; // make room for the new bit
        retVal = GetMiddleOfPulse(player);
        if(retVal == 5) // timeout
        {
        	if(bits_read >= 8)
        	{
				command = command >> 2; // get rid of the stop bit AND the room we made for an additional bit
				return command;
        	}
        	else // there is no possible way this can be a real command
        	{
        		return 5; // dummy value
        	}
        }
        command += retVal;

        bits_read++;

        if(bits_read >= 25) // this is the longest known command length
        {
        	command = command >> 1; // get rid of the stop bit (which is always a 1)
        	return command;
        }
    }
}


static uint8_t GetMiddleOfPulse(uint8_t player)
{
	uint8_t ct = 0;
    // wait for line to go high
    while(1)
    {
        if(GCN64_ReadBit(player)) break;

        ct++;
        if(ct == 200) // failsafe limit TBD
        	return 5; // error code
    }

    ct = 0;

    // wait for line to go low
    while(1)
    {
        if(!GCN64_ReadBit(player)) break;

        ct++;
		if(ct == 200) // failsafe limit TBD
			return 5; // error code
    }

    // now we have the falling edge

    // wait 2 microseconds to be in the middle of the pulse, and read. high --> 1.  low --> 0.
    my_wait_us_asm(2);

    return GCN64_ReadBit(player);
}

inline void SendStop(uint8_t player)
{

	#ifdef BOARDV3
	if(player == 1)
	{
		P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin<<16;
		my_wait_us_asm(1);
		P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin;
	}
	else if(player == 2)
	{
		P2_DATA_2_GPIO_Port->BSRR = P2_DATA_2_Pin<<16;
		my_wait_us_asm(1);
		P2_DATA_2_GPIO_Port->BSRR = P2_DATA_2_Pin;
	}
	#endif

	#ifdef BOARDV4
	if(player == 1)
	{
		ENABLE_P1D2D3_GPIO_Port->BSRR = ENABLE_P1D2D3_Pin<<16;
		my_wait_us_asm(1);
		ENABLE_P1D2D3_GPIO_Port->BSRR = ENABLE_P1D2D3_Pin;
	}
	else if(player == 2)
	{
		ENABLE_P2D2D3_GPIO_Port->BSRR = ENABLE_P2D2D3_Pin<<16;
		my_wait_us_asm(1);
		ENABLE_P2D2D3_GPIO_Port->BSRR = ENABLE_P2D2D3_Pin;
	}
	#endif

}

inline void N64_SendIdentity(uint8_t player)
{
    // reply 0x05, 0x00, 0x02
    SendByte(player, 0x05);
    SendByte(player, 0x00);
    SendByte(player, 0x02);
    SendStop(player);
}

inline void write_1(uint8_t player)
{
	#ifdef BOARDV3
	if(player == 1)
	{
		P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin<<16;
		my_wait_us_asm(1);
		P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin;
		my_wait_us_asm(3);
	}
	else if(player == 2)
	{
		P2_DATA_2_GPIO_Port->BSRR = P2_DATA_2_Pin<<16;
		my_wait_us_asm(1);
		P2_DATA_2_GPIO_Port->BSRR = P2_DATA_2_Pin;
		my_wait_us_asm(3);
	}
	#endif
	#ifdef BOARDV4
	if(player == 1)
	{
		ENABLE_P1D2D3_GPIO_Port->BSRR = ENABLE_P1D2D3_Pin<<16;
		my_wait_us_asm(1);
		ENABLE_P1D2D3_GPIO_Port->BSRR = ENABLE_P1D2D3_Pin;
		my_wait_us_asm(3);
	}
	else if(player == 2)
	{
		ENABLE_P2D2D3_GPIO_Port->BSRR = ENABLE_P2D2D3_Pin<<16;
		my_wait_us_asm(1);
		ENABLE_P2D2D3_GPIO_Port->BSRR = ENABLE_P2D2D3_Pin;
		my_wait_us_asm(3);
	}
	#endif
}

inline void write_0(uint8_t player)
{
	#ifdef BOARDV3
	if(player == 1)
	{
		P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin<<16;
		my_wait_us_asm(3);
		P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin;
		my_wait_us_asm(1);
	}
	else if(player == 2)
	{
		P2_DATA_2_GPIO_Port->BSRR = P2_DATA_2_Pin<<16;
		my_wait_us_asm(3);
		P2_DATA_2_GPIO_Port->BSRR = P2_DATA_2_Pin;
		my_wait_us_asm(1);
	}
	#endif

	#ifdef BOARDV4
	if(player == 1)
	{
		ENABLE_P1D2D3_GPIO_Port->BSRR = ENABLE_P1D2D3_Pin<<16;
		my_wait_us_asm(3);
		ENABLE_P1D2D3_GPIO_Port->BSRR = ENABLE_P1D2D3_Pin;
		my_wait_us_asm(1);
	}
	else if(player == 2)
	{
		ENABLE_P2D2D3_GPIO_Port->BSRR = ENABLE_P2D2D3_Pin<<16;
		my_wait_us_asm(3);
		ENABLE_P2D2D3_GPIO_Port->BSRR = ENABLE_P2D2D3_Pin;
		my_wait_us_asm(1);
	}
	#endif

}

// send a byte from LSB to MSB (proper serialization)
inline void SendByte(uint8_t player, unsigned char b)
{
    for(int i = 7;i >= 0;i--) // send all 8 bits, one at a time
    {
        if((b >> i) & 1)
        {
            write_1(player);
        }
        else
        {
            write_0(player);
        }
    }
}

void N64_SendRunData(uint8_t player, N64ControllerData n64data)
{
	unsigned long data = 0;
	memcpy(&data,&n64data,sizeof(data));
    // send one byte at a time from MSB to LSB
	unsigned int size = sizeof(data); // should be 4 bytes

    for(unsigned int i = 0;i < size;i++) // for each byte
    {
    	for(int b = 7;b >=0;b--) // for each bit in the byte
    	{
			if((data >> (b+(i*8)) & 1))
			{
				write_1(player);
			}
			else
			{
				write_0(player);
			}
    	}
    }

    SendStop(player);
}

void N64_SendControllerData(uint8_t player, unsigned long data)
{
    // send one byte at a time from MSB to LSB
	unsigned int size = sizeof(data); // should be 4 bytes

    for(unsigned int i = 0;i < size;i++) // for each byte
    {
    	for(int b = 7;b >=0;b--) // for each bit in the byte
    	{
			if((data >> (b+(i*8)) & 1))
			{
				write_1(player);
			}
			else
			{
				write_0(player);
			}
    	}
    }

    SendStop(player);
}

void GC_SendRunData(uint8_t player, GCControllerData gcdata)
{
	uint64_t data = 0;
	memcpy(&data,&gcdata,sizeof(data));

    unsigned int size = sizeof(data); // should be 8 bytes

    for(unsigned int i = 0;i < size;i++) // for each byte
	{
		for(int b = 7;b >=0;b--) // for each bit in the byte
		{
			if((data >> (b+(i*8)) & 1))
			{
				write_1(player);
			}
			else
			{
				write_0(player);
			}
		}
	}

    SendStop(player);
}

void GC_SendControllerData(uint8_t player, uint64_t data)
{
    unsigned int size = sizeof(data); // should be 8 bytes

    for(unsigned int i = 0;i < size;i++) // for each byte
	{
		for(int b = 7;b >=0;b--) // for each bit in the byte
		{
			if((data >> (b+(i*8)) & 1))
			{
				write_1(player);
			}
			else
			{
				write_0(player);
			}
		}
	}

    SendStop(player);
}

inline void GC_SendIdentity(uint8_t player)
{
    SendByte(player, 0x09);
    SendByte(player, 0x00);
    SendByte(player, 0x30);
    SendStop(player);
}

inline void GC_SendOrigin(uint8_t player)
{
	GCControllerData gc_data;

	memset(&gc_data, 0, sizeof(gc_data));

	gc_data.a_x_axis = 128;
	gc_data.a_y_axis = 128;
	gc_data.c_x_axis = 128;
	gc_data.c_y_axis = 128;
	gc_data.beginning_one = 1;
	gc_data.l_trigger = 0;
	gc_data.r_trigger = 0;

	uint64_t data = 0;
	memcpy(&data,&gc_data,sizeof(data));

	unsigned int size = sizeof(data); // should be 8 bytes

	for(unsigned int i = 0;i < size;i++) // for each byte
	{
		for(int b = 7;b >=0;b--) // for each bit in the byte
		{
			if((data >> (b+(i*8)) & 1))
			{
				write_1(player);
			}
			else
			{
				write_0(player);
			}
		}
	}

	SendByte(player, 0x00);
	SendByte(player, 0x00);
	SendStop(player);
}
