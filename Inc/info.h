/**
@file vport.h
@brief Generates the device info string (readable over serial) at compile-time using preprocessor magic.
*/

#pragma once
#include "TASRun.h"
#include <string.h>
#include <stdlib.h>

#define ARRLEN(x) (sizeof x / sizeof x[0])

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
  "MAKEFILE_SHOULD_REPLACE_ME_WITH_GIT_HASH",
  "1.0.0",
  "MAX_SIZE",
  "MAX_CONTROLLERS",
  "MAX_DATA_LANES",
  "MAX_TRANSITIONS",
  "nes,snes,n64,gcn,genesis"
};

static inline void serial_write_varint(int n)
{
  uint8_t buf;
  if(n<=127)
  {
    buf=(uint8_t) n;
    serial_interface_output((uint8_t*)&buf, 1);
  }
  else
  {
    buf=(uint8_t)(n & 0x8f);
    serial_interface_output((uint8_t*)&buf, 1);
    buf=(uint8_t)(n >> 7);
    serial_interface_output((uint8_t*)&buf, 1);
  }
}

uint8_t* outbuf;
uint8_t* writeptr;
#define BUFSIZE (writeptr-outbuf)

static inline void buffer_write_varint(int n)
{
  if(n<=127)
  {
    realloc(outbuf, BUFSIZE+1);
    *writeptr=(uint8_t) n;
    writeptr++;
  }
  else
  {
    realloc(outbuf, BUFSIZE+2);
    writeptr[0]=(uint8_t)(n & 0x8f);
    writeptr[1]=(uint8_t)(n >> 7);
    writeptr+=2;
  }
}

static inline void serial_write_InfoBlob()
{
  int names_array_bytes = 0;
  for(int i=0; i<ARRLEN(info_field_names); i++) names_array_bytes += strlen(info_field_names[i])+1;
  int vals_array_bytes = 0;
  for(int i=0; i<ARRLEN(info_field_vals); i++) vals_array_bytes += strlen(info_field_vals[i])+1;

  outbuf=malloc(1);
  writeptr=outbuf;
  outbuf[0]="I";
  writeptr++;


  buffer_write_varint(ARRLEN(info_field_names));
  buffer_write_varint(names_array_bytes);
  //Write field names
  for(int i=0; i< ARRLEN(info_field_names); i++)
  {
    realloc(outbuf, BUFSIZE+strlen(info_field_names[i])+1);
    memcpy(info_field_names[i], writeptr, strlen(info_field_names[i])+1);
    writeptr+=(strlen(info_field_names[i])+1);
  }

  buffer_write_varint(vals_array_bytes);
  for(int i=0; i< ARRLEN(info_field_vals); i++)
  {
    realloc(outbuf, BUFSIZE+strlen(info_field_vals[i])+1);
    memcpy(info_field_vals[i], writeptr, strlen(info_field_vals[i])+1);
    writeptr+=(strlen(info_field_vals[i])+1);
  }

  serial_interface_output(outbuf, BUFSIZE);
  free(outbuf);

  /*serial_interface_output((uint8_t*)"I", 1);
  serial_write_varint(ARRLEN(info_field_names));
  serial_write_varint(names_array_bytes);
  //Write field names
  for(int i=0; i< ARRLEN(info_field_names); i++)
    serial_interface_output(info_field_names[i], strlen(info_field_names[i])+1);

  serial_write_varint(vals_array_bytes);
  for(int i=0; i< ARRLEN(info_field_vals); i++)
    serial_interface_output(info_field_vals[i], strlen(info_field_vals[i])+1);
    */
}


/*
DV > 'i'
DV > <field count>
DV > <field names array is x bytes>
DV > <field names array>
DV > <field values array is y bytes>
DV > <field values array>
*/
