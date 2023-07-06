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

print("--- Sending command power the console on")

if args.relayreset:
    dev.enable_relay()

dev.power_on()
time.sleep(0.1)
