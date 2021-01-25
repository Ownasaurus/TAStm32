#!/usr/bin/env python3
import serial, time
import serial_helper
import argparse_helper
import tastm32

parser = argparse_helper.audio_parser()
args = parser.parse_args()

if args.serial == None:
    dev = tastm32.TAStm32(serial_helper.select_serial_port())
else:
    dev = tastm32.TAStm32(args.serial)

# connect to device
ser = dev

print("--- Sending Ping Command")

def ping(attempt = 0):
  if (attempt < 5):
    dev.ping()
    result = dev.waitForPong()
    if (result == 0):
        print("--- Pong Received")
    elif (result == -1):
        print("--- Ping Timeout")
        ping(attempt+1)
    elif (result == -2):
        print("--- Greater than 1000 bytes read with no response")
    elif (result == -3):
        print("--- Serial Error")
    elif (result == -4):
        print("--- Keyboard Interupt")
    else:
        print("--- Unhandled Error")


