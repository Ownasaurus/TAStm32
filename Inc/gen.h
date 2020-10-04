#ifndef __GEN__H
#define __GEN__H

typedef struct __attribute__((packed))
{
	// main controller byte
	unsigned int up : 1; // low bit of 1st byte
	unsigned int down : 1;
	unsigned int left : 1;
	unsigned int right : 1;
	unsigned int a : 1;
	unsigned int b : 1;
	unsigned int c : 1;
	unsigned int start : 1; // high bit of 1st byte

	// second byte, only used for 6 button controllers
	unsigned int x : 1;
	unsigned int y : 1;
	unsigned int z : 1;
	unsigned int mode : 1;
	unsigned int unused : 4;

} GENControllerData;

#endif
