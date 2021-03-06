#!/usr/bin/env python3
import serial_helper
import argparse_helper
import tastm32
import time
from inputs import devices

#Some sort of sync command every time it changes?

#Triggers: 0-255
#ABS_Z is left trigger
#ABS_RZ is right trigger

#BTN_TL / BTN_TR are bumpers, 0 or 1

#BTN_EAST, NORTH, SOUTH, and WEST are face buttons. 0 or 1.

# ABS_HAT0X tristate: -1 = left, 0 = neutral, 1 = right
# ABS_HAT0Y tristate: -1 = UP, 0 = neutral, 1 = DOWN

#BTN_SELECT and BTN_START are the two middle buttons for whatever xbox family symbol thingy, 0 or 1.

#Left analog stick:
#ABS_X POSITIVE = RIGHT
#ABS_Y POSITIVE = UP

#Right analog stick:
#ABS_RX POSITIVE = RIGHT
#ABS_RY POSITIVE = UP

#analog sticks pushed in = BTN_THUMBL, BTN_THUMBR, 0 or 1. also registering as "KEY" event for some reason
#- to + 32k

# try and connect to the TAStm32
parser = argparse_helper.audio_parser()
args = parser.parse_args()

if args.serial == None:
    dev = tastm32.TAStm32(serial_helper.select_serial_port())
else:
    dev = tastm32.TAStm32(args.serial)
ser = dev

# initialize tastm32
ser.write(b'R')
time.sleep(0.1)
cmd = ser.read(2)
print(bytes(cmd))

# set up the N64 correctly
ser.write(b'SAM\x80\x00')
time.sleep(0.1)
cmd = ser.read(2)
print(bytes(cmd))

# no bulk transfer
ser.write(b'QA0')
time.sleep(0.1)

# clear the input buffer
ser.ser.reset_input_buffer()

TRIGGER_THRESHOLD = 125
XBOX_ANALOG_THRESHOLD = 20000

btn_codes = {
    'BTN_NORTH' : 0x00020000, #Y
    'BTN_SOUTH' : 0x80000000, #A
    'BTN_EAST' : 0x00040000, #B
    'BTN_WEST' : 0x40000000, #X
    'BTN_THUMBL' : 0x00000000, # L stick in
    'BTN_THUMBR' : 0x00000000, # R stick in
    'BTN_SELECT' : 0x10000000, # Select
    'BTN_START' : 0x10000000, # Start
    'BTN_TL' : 0x00200000, # L bumper
    'BTN_TR' : 0x00100000, # R bumper
}

N64_Z = 0x20000000

frame_start = time.time()
data_to_tastm32 = 0
prev_data = 0

ser.write(bytes([65,0,0,0,0]))

numGamepads = len(devices.gamepads)
chosenGamepad = 0
if len == 1:
    chosenGamepad = 0
else:
    for i in range(len(devices.gamepads)):
        print(f"{i}) --{devices.gamepads[i]}--")
    chosenGamepad = int(input("Which device is your gamepad? "))

while 1:
    events = devices.gamepads[chosenGamepad].read()
    for event in events:
        if event.ev_type == 'Key': # handle button presses
            if event.state == 1: # pressed
                data_to_tastm32 |= btn_codes[event.code]
            elif event.state == 0: # released
                data_to_tastm32 &= ~btn_codes[event.code]
        elif event.ev_type == 'Absolute': # handle analog inputs
            if event.code == 'ABS_Z' or event.code == 'ABS_RZ':
                if event.state >= TRIGGER_THRESHOLD:
                    data_to_tastm32 |= N64_Z
                else:
                    data_to_tastm32 &= ~N64_Z
            # ABS_HAT0X tristate: -1 = left, 0 = neutral, 1 = right
            # ABS_HAT0Y tristate: -1 = UP, 0 = neutral, 1 = DOWN
            if event.code == 'ABS_HAT0Y':
                if event.state == -1: # up
                    print(event.ev_type, event.code, event.state)
                    data_to_tastm32 |= 0x01000000
                elif event.state == 1: #down
                    data_to_tastm32 |= 0x02000000
                elif event.state == 0: # neutral
                    data_to_tastm32 &= 0xFCFFFFFF
            elif event.code == 'ABS_HAT0X':
                if event.state == -1: # left
                    data_to_tastm32 |= 0x04000000
                elif event.state == 1: #right
                    data_to_tastm32 |= 0x08000000
                elif event.state == 0: # neutral
                    data_to_tastm32 &= 0xF3FFFFFF
            elif event.code == 'ABS_X':
                N64_X_val = (event.state) // 384
                abs_X_val = abs(N64_X_val)
                sign_X_val = 0
                if N64_X_val < 0:
                    sign_X_val = 1
                    abs_X_val = 128 - abs_X_val
                data_to_tastm32 &= 0xFFFF00FF
                data_to_tastm32 |= ((abs_X_val << 8) | (sign_X_val << 15))
            elif event.code == 'ABS_Y':
                N64_Y_val = (event.state) // 384
                abs_Y_val = abs(N64_Y_val)
                sign_Y_val = 0
                if N64_Y_val < 0:
                    sign_Y_val = 1
                    abs_Y_val = 128 - abs_Y_val
                data_to_tastm32 &= 0xFFFFFF00
                data_to_tastm32 |= (abs_Y_val | (sign_Y_val << 7))
            elif event.code == 'ABS_RX':
                abs_X_val = abs(event.state)
                if abs_X_val < XBOX_ANALOG_THRESHOLD: # Neutral position
                    data_to_tastm32 &= 0xFFFCFFFF
                else:
                    if event.state < 0: # Left
                        data_to_tastm32 |= 0x00020000
                    else: # Right
                        data_to_tastm32 |= 0x00010000
            elif event.code == 'ABS_RY':
                abs_Y_val = abs(event.state)
                if abs_Y_val < XBOX_ANALOG_THRESHOLD: # Neutral position
                    data_to_tastm32 &= 0xFFF3FFFF
                else:
                    if event.state < 0: # Down
                        data_to_tastm32 |= 0x00040000
                    else: # Up
                        data_to_tastm32 |= 0x00080000
        
        # prepare and send message to the replay device
        new_time = time.time()
        if new_time > frame_start+(1/30): # slightly less than 30 hz polling to ensure no input queuing
            if data_to_tastm32 != prev_data:
                output_string = b'A' + data_to_tastm32.to_bytes(4, "big")
                ser.write(output_string)
                frame_start = new_time
                prev_data = data_to_tastm32
