#ifndef __SNES__H
#define __SNES__H

#include <stdint.h>

typedef struct __attribute__((packed))
{
	uint8_t right : 1; // low bit
	uint8_t left : 1;
	uint8_t down : 1;
	uint8_t up : 1;
	uint8_t start : 1;
	uint8_t select : 1;
	uint8_t b : 1;
	uint8_t a : 1; // high bit

} NESControllerData;

typedef struct __attribute__((packed))
{
	// first byte
	uint8_t right : 1; // low bit
	uint8_t left : 1;
	uint8_t down : 1;
	uint8_t up : 1;
	uint8_t start : 1;
	uint8_t select : 1;
	uint8_t y : 1;
	uint8_t b : 1; // high bit

	// second byte
	uint8_t four : 1; // low bit
	uint8_t three : 1;
	uint8_t two : 1;
	uint8_t one : 1;
	uint8_t r : 1;
	uint8_t l : 1;
	uint8_t x : 1;
	uint8_t a : 1; // high bit

} SNESControllerData;

void SendControllerDataNES(uint8_t data);

#endif
