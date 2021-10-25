#!/usr/bin/env python3
import serial, sys, time, os, gc
import serial_helper
import argparse_helper
import tastm32
import psutil

if(os.name == 'nt'):
    psutil.Process().nice(psutil.REALTIME_PRIORITY_CLASS)
else:
    psutil.Process().nice(20)

gc.disable()

def bitswap(b):
    b = (b&0xF0) >> 4 | (b&0x0F) << 4
    b = (b&0xCC) >> 2 | (b&0x33) << 2
    b = (b&0xAA) >> 1 | (b&0x55) << 1
    return b
    
data = None

parser = argparse_helper.audio_parser()
args = parser.parse_args()

DEBUG = args.debug

if args.serial == None:
    dev = tastm32.TAStm32(serial_helper.select_serial_port())
else:
    dev = tastm32.TAStm32(args.serial)

# connect to device
ser = dev

# open file
f = sys.stdin.buffer #open(sys.argv[1], "rb")

latches = 0

cmd = None
inputs = None   

print("--- Starting read loop")

# reset to make sure there is no leftover data
ser.write(b'R')
time.sleep(0.1)
cmd = ser.read(2)
print(bytes(cmd))

# set up the SNES correctly
ser.write(b'SAS\xCC\x00')
time.sleep(0.1)
cmd = ser.read(2)
print(bytes(cmd))

ser.write(b'QA1')
time.sleep(0.1)

ser.ser.reset_input_buffer() # clear anything that might be sitting on the serial line at the moment

# seed it with an arbitrary first frame of data to get the run to be initialized
ser.write(bytes([65,1,1,1,1,1,1,1,1]))
    
while True:
    c = ser.read(1) # keep this loop as tight as possible
    
    if c.count(b'\xB0'): # this should not ever occur based on the protocol
        print("overflow!")
        continue
    if c.count(b'a'): # we want 28 latches
        for twice in range(4): # send 4 sets of 7 latches
            inputs = f.read(56) # this will result in a packet size of 63. we're trying to keep it below 64
            packed_pcm = inputs
            data = []

            for i in range(0, len(packed_pcm), 8):
                val = (packed_pcm[i+1] << 8) + (packed_pcm[i])
                val = val ^ 0xA804

                d1 =  (((val>>6))&1)
                d1 += (((val>>4)&1)<<1)
                d1 += (((val>>2)&1)<<2)
                d1 += (((val)&1)<<3)

                d2 = ((val>>7)&1)
                d2 += (((val>>5)&1)<<1)
                d2 += (((val>>3)&1)<<2)
                d2 += (((val>>1)&1)<<3)

                d3 = (((val>>14))&1)
                d3 += ((((val>>12))&1)<<1)
                d3 += (((val>>10)&1)<<2)
                d3 += ((((val>>8))&1)<<3)

                d4 = (((val>>15))&1)
                d4 += ((((val>>13))&1)<<1)
                d4 += ((((val>>11))&1)<<2)
                d4 += (((val>>9)&1)<<3)

                val = (packed_pcm[i+3] << 8) + (packed_pcm[i+2])
                val = val ^ 0xA804

                d5 =  (((val>>6))&1)
                d5 += (((val>>4)&1)<<1)
                d5 += (((val>>2)&1)<<2)
                d5 += (((val)&1)<<3)

                d6 = ((val>>7)&1)
                d6 += (((val>>5)&1)<<1)
                d6 += (((val>>3)&1)<<2)
                d6 += (((val>>1)&1)<<3)

                d7 = (((val>>14))&1)
                d7 += ((((val>>12))&1)<<1)
                d7 += (((val>>10)&1)<<2)
                d7 += ((((val>>8))&1)<<3)

                d8 = (((val>>15))&1)
                d8 += ((((val>>13))&1)<<1)
                d8 += ((((val>>11))&1)<<2)
                d8 += (((val>>9)&1)<<3)

                val = (packed_pcm[i+5] << 8) + (packed_pcm[i+4])
                val = val ^ 0xA804

                d11 =  (((val>>6))&1)
                d11 += (((val>>4)&1)<<1)
                d11 += (((val>>2)&1)<<2)
                d11 += (((val)&1)<<3)

                d21 = ((val>>7)&1)
                d21 += (((val>>5)&1)<<1)
                d21 += (((val>>3)&1)<<2)
                d21 += (((val>>1)&1)<<3)

                d31 = (((val>>14))&1)
                d31 += ((((val>>12))&1)<<1)
                d31 += (((val>>10)&1)<<2)
                d31 += ((((val>>8))&1)<<3)

                d41 = (((val>>15))&1)
                d41 += ((((val>>13))&1)<<1)
                d41 += ((((val>>11))&1)<<2)
                d41 += (((val>>9)&1)<<3)

                val = (packed_pcm[i+7] << 8) + (packed_pcm[i+6])
                val = val ^ 0xA804

                d51 =  (((val>>6))&1)
                d51 += (((val>>4)&1)<<1)
                d51 += (((val>>2)&1)<<2)
                d51 += (((val)&1)<<3)

                d61 = ((val>>7)&1)
                d61 += (((val>>5)&1)<<1)
                d61 += (((val>>3)&1)<<2)
                d61 += (((val>>1)&1)<<3)

                d71 = (((val>>14))&1)
                d71 += ((((val>>12))&1)<<1)
                d71 += (((val>>10)&1)<<2)
                d71 += ((((val>>8))&1)<<3)

                d81 = (((val>>15))&1)
                d81 += ((((val>>13))&1)<<1)
                d81 += ((((val>>11))&1)<<2)
                d81 += (((val>>9)&1)<<3)

                data = data + [65, bitswap(d1) + (bitswap(d5)>>4), bitswap(d11) + (bitswap(d51)>>4), bitswap(d2) + (bitswap(d6)>>4), bitswap(d21) + (bitswap(d61)>>4), bitswap(d3) + (bitswap(d7)>>4), bitswap(d31) + (bitswap(d71)>>4), bitswap(d4) + (bitswap(d8)>>4), bitswap(d41) + (bitswap(d81)>>4)]
            
            ser.write(bytes(data))
        ser.write(b'a') # tell the hardware that we have completed our bulk transfer
