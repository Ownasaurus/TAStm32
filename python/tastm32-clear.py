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

print("--- Sending command reset TAStm32")

dev.reset()
time.sleep(0.1)
