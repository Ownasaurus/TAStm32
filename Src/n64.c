#include <stdint.h>
#include <string.h>
#include "n64.h"
#include "stm32f4xx_hal.h"
#include "main.h"

void my_wait_us_asm(int n);

static uint8_t GetMiddleOfPulse(uint8_t player);
static void SendByte(unsigned char b);

uint8_t GCN64_ReadBit(uint8_t player)
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
	my_wait_us_asm(2); 	// consider shortening slightly to account for function call
						// overhead, ISR overhead, etc. Idea by Sauraen

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

void SendStop()
{
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin<<16;
	my_wait_us_asm(1);
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin;
}

void SendIdentityN64()
{
    // reply 0x05, 0x00, 0x02
    SendByte(0x05);
    SendByte(0x00);
    SendByte(0x02);
    SendStop();
}

void write_1()
{
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin<<16;
	my_wait_us_asm(1);
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin;
    my_wait_us_asm(3);
}

void write_0()
{
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin<<16;
	my_wait_us_asm(3);
	P1_DATA_2_GPIO_Port->BSRR = P1_DATA_2_Pin;
    my_wait_us_asm(1);
}

// send a byte from LSB to MSB (proper serialization)
static void SendByte(unsigned char b)
{
    for(int i = 7;i >= 0;i--) // send all 8 bits, one at a time
    {
        if((b >> i) & 1)
        {
            write_1();
        }
        else
        {
            write_0();
        }
    }
}

void SendRunDataN64(N64ControllerData n64data)
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
				write_1();
			}
			else
			{
				write_0();
			}
    	}
    }

    SendStop();
}

void SendControllerDataN64(unsigned long data)
{
    // send one byte at a time from MSB to LSB
	unsigned int size = sizeof(data); // should be 4 bytes

    for(unsigned int i = 0;i < size;i++) // for each byte
    {
    	for(int b = 7;b >=0;b--) // for each bit in the byte
    	{
			if((data >> (b+(i*8)) & 1))
			{
				write_1();
			}
			else
			{
				write_0();
			}
    	}
    }

    SendStop();
}

void SendRunDataGC(GCControllerData gcdata)
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
				write_1();
			}
			else
			{
				write_0();
			}
		}
	}

    SendStop();
}

void SendControllerDataGC(uint64_t data)
{
    unsigned int size = sizeof(data); // should be 8 bytes

    for(unsigned int i = 0;i < size;i++) // for each byte
	{
		for(int b = 7;b >=0;b--) // for each bit in the byte
		{
			if((data >> (b+(i*8)) & 1))
			{
				write_1();
			}
			else
			{
				write_0();
			}
		}
	}

    SendStop();
}

void SendIdentityGC()
{
    SendByte(0x09);
    SendByte(0x00);
    SendByte(0x30);
    SendStop();
}

void SendOriginGC()
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
				write_1();
			}
			else
			{
				write_0();
			}
		}
	}

	SendByte(0x00);
	SendByte(0x00);
	SendStop();
}
