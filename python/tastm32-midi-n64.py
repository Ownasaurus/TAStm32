#!/usr/bin/env python3
import serial, sys, time, os, gc
import serial_helper
import argparse_helper
import tastm32
import psutil
import struct
import mido
import time

current_milli_time = lambda: int(round(time.time() * 1000))

if(os.name == 'nt'):
    psutil.Process().nice(psutil.REALTIME_PRIORITY_CLASS)
else:
    psutil.Process().nice(20)

gc.disable()

parser = argparse_helper.audio_parser()
args = parser.parse_args()

DEBUG = args.debug

# try and connect to the TAStm32
if args.serial == None:
    dev = tastm32.TAStm32(serial_helper.select_serial_port())
else:
    dev = tastm32.TAStm32(args.serial)
ser = dev

MOVEMENT_VELOCITY_FILTER = 18

tmap = {
  45: 0x00000018, # analog up
  38: 0x000000E8, # analog down
  48: 0x0000E800, # analog left
  43: 0x00001800 # analog right
}

bmap = {
  45: 0x00000050, # analog up
  38: 0x000000B0, # analog down
  48: 0x0000B000, # analog left
  43: 0x00005000, # analog right
  0: 0x01000000, # d-pad up
  1: 0x02000000, # d-pad down
  2: 0x04000000, # d-pad left
  3: 0x08000000, # d-pad right
  36: 0x80000000, # A
  57: 0x40000000, # B
  52: 0x40000000, # B
  26: 0x10000000, # Start
  47: 0x00080000, # C-up 
  40: 0x00040000, # C-down
  50: 0x00020000, # C-left
  58: 0x00010000, # C-right
  5: 0x00200000, # L
  51: 0x00100000, # R
  59: 0x00100000 # R
}

cmap = {
  4: 0x20000000, # Z
}

pmap = {
  51: 0x80000000, # A
  18: 0x04000000, # d-pad left
  19: 0x08000000, # d-pad right
  20: 0x01000000, # d-pad up
  21: 0x02000000, # d-pad down
  12: 0x40000000, # B
  57: 0x00200000 # L
}

toggle = 0
aup = 0
adown = 0
aleft = 0
aright = 0
A_time = -1

int_to_byte_struct = struct.Struct('B')
def int_to_byte(interger):
    return int_to_byte_struct.pack(interger)

ser.write(b'R')
time.sleep(0.1)
cmd = ser.read(2)
print(bytes(cmd))

# set up the N64 correctly
ser.write(b'SAM\x80\x00')
time.sleep(0.1)
cmd = ser.read(2)
print(bytes(cmd))

# enable controller mode
ser.write(b'C1')
time.sleep(0.1)

# disable bulk transfer mode
ser.write(b'QA0')
time.sleep(0.1)

ser.ser.reset_input_buffer()

# try and connect to the MIDI drumset
device_list = mido.get_input_names()

choice = -1

if len(device_list) == 1:
    choice = 0
else:
    for i in range(len(device_list)):
        if device_list[i] == "4- UMC404HD 192k MIDI In 0":
            choice = 0
    if choice != 0:
        for i in range(len(device_list)):
            print(f"{i}) --{device_list[i]}--")
        choice = input("Which device is your drumset (please enter the number and hit <enter>)? ")

frame_start = time.time()
data_to_tastm32 = 0
prev_data = 0

with mido.open_input(device_list[int(choice)]) as midi_device:
    while True:
        # release A if timer is active and expired
        if A_time != -1 and current_milli_time() > (A_time + 400):
            data_to_tastm32 &= ~pmap[51]
            A_time = -1 # un-initialize A_time
            
        # process the midi message
        for message in midi_device.iter_pending():
            if toggle == 0:
                print(message) # fixes stuff cause of artificial delay, apparently
            if message.type == "note_on":
                try:
                    if message.note == 49: # toggle switch
                        toggle = 1 - toggle
                        aup = 0
                        adown = 0
                        aleft = 0
                        aright = 0
                    elif message.note == 45: # analog up
                        if toggle == 1:
                            data_to_tastm32 &= 0xFFFF0000 # clear all other directions
                            if aup == 0:
                                data_to_tastm32 |= bmap[message.note]
                            aup = 1 - aup
                            adown = 0
                            aleft = 0
                            aright = 0
                        else:
                            if message.velocity < MOVEMENT_VELOCITY_FILTER:
                                data_to_tastm32 |= tmap[message.note]
                            else:
                                data_to_tastm32 |= bmap[message.note]
                    elif message.note == 38: # analog down
                        if toggle == 1:
                            data_to_tastm32 &= 0xFFFF0000 # clear all other directions
                            if adown == 0:
                                data_to_tastm32 |= bmap[message.note]
                            adown = 1 - adown
                            aup = 0
                            aleft = 0
                            aright = 0
                        else:
                            if message.velocity < MOVEMENT_VELOCITY_FILTER:
                                data_to_tastm32 |= tmap[message.note]
                            else:
                                data_to_tastm32 |= bmap[message.note]
                    elif message.note == 48: # analog left
                        if toggle == 1:
                            data_to_tastm32 &= 0xFFFF0000 # clear all other directions
                            if aleft == 0:
                                data_to_tastm32 |= bmap[message.note]
                            aleft = 1 - aleft
                            adown = 0
                            aup = 0
                            aright = 0
                        else:
                            if message.velocity < MOVEMENT_VELOCITY_FILTER:
                                data_to_tastm32 |= tmap[message.note]
                            else:
                                data_to_tastm32 |= bmap[message.note]
                    elif message.note == 43: # analog right
                        if toggle == 1:
                            data_to_tastm32 &= 0xFFFF0000 # clear all other directions
                            if aright == 0:
                                data_to_tastm32 |= bmap[message.note]
                            aright = 1 - aright
                            adown = 0
                            aleft = 0
                            aup = 0
                        else:
                            if message.velocity < MOVEMENT_VELOCITY_FILTER:
                                data_to_tastm32 |= tmap[message.note]
                            else:
                                data_to_tastm32 |= bmap[message.note]
                    elif message.note == 36: # A
                        data_to_tastm32 |= bmap[message.note]
                        if toggle == 0 and message.velocity > 50:
                            A_time = current_milli_time()
                    else:
                        data_to_tastm32 |= bmap[message.note]
                except KeyError:
                    pass
            elif message.type == "control_change":
                try:
                    if message.value == 90:
                        data_to_tastm32 |= cmap[message.control]
                    else:
                        data_to_tastm32 &= ~cmap[message.control]
                except KeyError:
                    pass
            elif message.type == "polytouch":
                try:
                    if message.value == 127:
                        data_to_tastm32 |= pmap[message.note]
                    else:
                        data_to_tastm32 &= ~pmap[message.note]
                except KeyError:
                    pass
            elif message.type == "note_off":
                try:
                    if message.note == 49: # ignore toggle switch release
                        pass
                    elif toggle == 1 and (message.note == 45 or message.note == 38 or message.note == 48 or message.note == 43): # ignroe analog UDLR release in toggle mode
                        pass
                    elif message.note == 36 and A_time != -1: # A is pressed and its timer is set
                        pass
                    elif message.note == 45 or message.note == 38:
                        data_to_tastm32 &= 0xFFFFFF00
                    elif message.note == 48 or message.note == 43:
                        data_to_tastm32 &= 0xFFFF00FF
                    else:
                        data_to_tastm32 &= ~bmap[message.note]
                except KeyError:
                    pass
        # prepare and send message to the replay device
        new_time = time.time()
        #if new_time > frame_start+(1/30): # slightly less than 30 hz polling to ensure no input queuing
        if new_time > frame_start+(1/120): # try 120 hz polling
            if data_to_tastm32 != prev_data:
                output_string = b'A' + data_to_tastm32.to_bytes(4, "big")
                ser.write(output_string)
                frame_start = new_time
                prev_data = data_to_tastm32