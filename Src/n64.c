#include <stdint.h>
#include <string.h>
#include "n64.h"
#include "stm32f4xx_hal.h"
#include "main.h"

// N64 data pin is p1_d2
#define N64_READ (P1_DATA_2_GPIO_Port->IDR & P1_DATA_2_Pin)

static uint8_t GetMiddleOfPulse()
{
	uint8_t ct = 0;
    // wait for line to go high
    while(1)
    {
        if(N64_READ) break;

        ct++;
        if(ct == 200) // failsafe limit TBD
        	return 5; // error code
    }

    ct = 0;

    // wait for line to go low
    while(1)
    {
        if(!N64_READ) break;

        ct++;
		if(ct == 200) // failsafe limit TBD
			return 5; // error code
    }

    // now we have the falling edge

    // wait 2 microseconds to be in the middle of the pulse, and read. high --> 1.  low --> 0.
    my_wait_us_asm(2);

    return N64_READ ? 1U : 0U;
}

uint32_t GCN64_ReadCommand()
{
	uint8_t retVal;

	// we are already at the first falling edge
	// get middle of first pulse, 2us later
	my_wait_us_asm(2);
	uint32_t command = N64_READ ? 1U : 0U, bits_read = 1;

    while(1) // read at least 9 bits (1 byte + stop bit)
    {
        command = command << 1; // make room for the new bit
        retVal = GetMiddleOfPulse();
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

void N64_SendIdentity()
{
    // reply 0x05, 0x00, 0x02
	uint32_t data = 0x00020005;
	GCN64_SendData((uint8_t*)&data, 3);
}

void GCN_SendIdentity()
{
	// reply 0x90, 0x00, 0x0C
	uint32_t data = 0x000C0090;
	GCN64_SendData((uint8_t*)&data, 3);
}

void GCN_SendOrigin()
{
	uint8_t buf[10];
	memset(buf, 0, sizeof(buf));
	GCControllerData *gc_data = (GCControllerData*)&buf[0];

	gc_data->a_x_axis = 128;
	gc_data->a_y_axis = 128;
	gc_data->c_x_axis = 128;
	gc_data->c_y_axis = 128;
	gc_data->beginning_one = 1;

	GCN64_SendData(buf, sizeof(buf));
}
