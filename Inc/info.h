/**
@file info.h
@brief Generates the device info blob and writes it over serial.
*/

#ifndef INFO_H
#define INFO_H

#pragma once
#include "TASRun.h"
#include <string.h>
#include <stdlib.h>

/*****Preprocessor shenanigans*****/
#define STRINGIFY2( x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define ARRLEN(x) (sizeof(x) / sizeof(x[0]))

/*****This should be defined by the makefile at compile-time.*****/
#ifndef GIT_HASH
#define GIT_HASH No Git hash defined at compile time - this might be a TrueStudio dev build. Are teehats to blame?
#endif

/*****Edit the below fields to adjust the info blob parameters.*****/
static const char* info_field_names[8] =
{
  "deviceName",
  "firmwareVersion",
  "protocolVersion",
  "maxSize",
  "maxControllers",
  "maxDataLanes",
  "maxTransitions",
  "supportedConsoles"
};

static const char* info_field_vals[8] =
{
  "TAStm32",
  STRINGIFY(GIT_HASH),
  "1.0.0",
  STRINGIFY(MAX_SIZE),
  STRINGIFY(MAX_CONTROLLERS),
  STRINGIFY(MAX_DATA_LANES),
  STRINGIFY(MAX_TRANSITIONS),
  "nes,snes,n64,gcn,genesis"
};

uint8_t* outbuf;
int bufsize;
#define writeptr (outbuf+bufsize)

//! Append an integer as a varint (up to 15 bits) to the output buffer.
static inline void buffer_write_varint(int n)
{
  if(n<=127)
  {
    outbuf = realloc(outbuf, bufsize+1);
    *writeptr=(uint8_t) n;
    bufsize++;
  }
  else
  {
    outbuf = realloc(outbuf, bufsize+2);
    writeptr[0]=(uint8_t)(n | 0x80);
    writeptr[1]=(uint8_t)(n >> 7);
    bufsize+=2;
  }
}

//! Construct the infoblob in a buffer, feed it to serial output, and free the buffer.
static inline void serial_write_InfoBlob()
{
  int names_array_bytes = 0;
  for(int i=0; i<ARRLEN(info_field_names); i++) names_array_bytes += strlen(info_field_names[i])+1;
  int vals_array_bytes = 0;
  for(int i=0; i<ARRLEN(info_field_vals); i++) vals_array_bytes += strlen(info_field_vals[i])+1;

  outbuf=malloc(1);
  outbuf[0]='I';
  bufsize=1;

  buffer_write_varint(ARRLEN(info_field_names));
  buffer_write_varint(names_array_bytes);

  //Write field names
  for(int i=0; i< ARRLEN(info_field_names); i++)
  {
    outbuf = realloc(outbuf, bufsize+strlen(info_field_names[i])+1);
    memcpy(writeptr, info_field_names[i], strlen(info_field_names[i])+1);
    bufsize+=(strlen(info_field_names[i])+1);
  }

  buffer_write_varint(vals_array_bytes);
  for(int i=0; i< ARRLEN(info_field_vals); i++)
  {
    outbuf = realloc(outbuf, bufsize+strlen(info_field_vals[i])+1);
    memcpy(writeptr, info_field_vals[i], strlen(info_field_vals[i])+1);
    bufsize+=(strlen(info_field_vals[i])+1);
  }

  serial_interface_output(outbuf, bufsize);
  free(outbuf);
}
#endif //INFO_H