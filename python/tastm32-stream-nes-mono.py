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
    c = ser.read(100) # read up to 100 bytes, though we shouldn't ever get that close
    
    if c.count(b'\xB0'): # this should not ever occur based on the protocol
        print("overflow!")
        continue
    
    if c.count(b'a'): # we want 28 latches
        for set in range(4): # send 4 sets of 7 latches
            data = f.read(56)
            b = [0, 0, 0, 0, 0, 0, 0, 0]
            output_buffer = []
            
            for i in range(0, 56, 8):
                for n in range(0, 8, 2):
                    outp = [0, 0]

                    d = (data[n+i]>>1)
                    data0 = d&1
                    data1 = (d>>1) & 1
                    data2 = (d>>2) & 1
                    data3 = (d>>3) & 1
                    data4 = (d>>4) & 1
                    data5 = (d>>5) & 1
                    data6 = (d>>6) & 1

                    outp[0] = data3
                    outp[1] = (data6 ^ 1)

                    outp[0] = (outp[0]<<1) + data2
                    outp[1] = (outp[1]<<1) + data5

                    outp[0] = (outp[0]<<1) + data1
                    outp[1] = (outp[1]<<1) + data4

                    outp[0] = (outp[0]<<1) + data0
                    outp[1] = (outp[1]<<1) + 0
                    
                    d = (data[n+i+1]>>1)
                    data0 = d&1
                    data1 = (d>>1) & 1
                    data2 = (d>>2) & 1
                    data3 = (d>>3) & 1
                    data4 = (d>>4) & 1
                    data5 = (d>>5) & 1
                    data6 = (d>>6) & 1

                    outp[0] = (outp[0]<<1) + data3
                    outp[1] = (outp[1]<<1) + (data6 ^ 1)

                    outp[0] = (outp[0]<<1) + data2
                    outp[1] = (outp[1]<<1) + data5

                    outp[0] = (outp[0]<<1) + data1
                    outp[1] = (outp[1]<<1) + data4

                    outp[0] = (outp[0]<<1) + data0
                    outp[1] = (outp[1]<<1) + 0		
                    
                    b[n] = outp[0]
                    b[n+1] = outp[1]
                    
                output_buffer = output_buffer + [65, b[0],b[2],b[1],b[3],b[4],b[6],b[5],b[7]] 
            
            ser.write(bytes(output_buffer))
        ser.write(b'a')