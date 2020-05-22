#!/usr/bin/env python3
import serial, sys, time, os, gc
import serial_helper
import argparse_helper
import tastm32
import psutil
import struct

if(os.name == 'nt'):
    psutil.Process().nice(psutil.REALTIME_PRIORITY_CLASS)
else:
    psutil.Process().nice(20)

gc.disable()

parser = argparse_helper.audio_parser()
args = parser.parse_args()

DEBUG = args.debug

if args.serial == None:
    dev = tastm32.TAStm32(serial_helper.select_serial_port())
else:
    dev = tastm32.TAStm32(args.serial)
ser = dev

bmap = {
  "u": b"A\x08\x00A\x00\x00",
  "d": b"A\x04\x00A\x00\x00",
  "l": b"A\x02\x00A\x00\x00",
  "r": b"A\x01\x00A\x00\x00",
  "a": b"A\x00\x80A\x00\x00",
  "b": b"A\x80\x00A\x00\x00",
  "x": b"A\x00\x40A\x00\x00",
  "y": b"A\x40\x00A\x00\x00",
  "s": b"A\x20\x00A\x00\x00",
  "S": b"A\x10\x00A\x00\x00",
  "L": b"A\x00\x20A\x00\x00",
  "R": b"A\x00\x10A\x00\x00"
}

int_to_byte_struct = struct.Struct('B')
def int_to_byte(interger):
    return int_to_byte_struct.pack(interger)

ser.write(b'R')
time.sleep(0.1)
cmd = ser.read(2)
print(bytes(cmd))

# set up the SNES correctly
ser.write(b'SAS\x80\x00')
time.sleep(0.1)
cmd = ser.read(2)
print(bytes(cmd))

ser.write(b'QA0')
time.sleep(0.1)

ser.ser.reset_input_buffer()

ser.write(bytes([65,0,0]))

while True:
  line = input()
  if line in bmap:
    ser.write(bmap[line])
  else:
    try:
      ser.write(b"A" + int_to_byte(int(line[0:2], 16)) + int_to_byte(int(line[2:4], 16)))
    except:
      pass
