#!/usr/bin/env python3
import serial, sys, time, os, gc
import serial_helper
import argparse_helper
import tastm32
import psutil
import struct
import mido
import time
import numpy as np
from itertools import chain

data_to_tastm32 = 0
prev_data = 0
MOVEMENT_VELOCITY_FILTER = 18
toggle = 0
aup = 0
adown = 0
aleft = 0
aright = 0
A_time = -1
B_time = -1
R_toggle = 0

NOTE_AUP = 45
NOTE_ADOWN = 38
NOTE_ALEFT = 48
NOTE_ARIGHT = 43

velocity_values = [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130]
stick_values =    [0, 20, 24, 72, 80, 80, 80, 80, 81, 82,  83,  84,  85,  86]

tmap = {
  NOTE_AUP: 0x00000018, # analog up
  NOTE_ADOWN: 0x000000E8, # analog down
  NOTE_ALEFT: 0x0000E800, # analog left
  NOTE_ARIGHT: 0x00001800, # analog right
}

bmap = {
  NOTE_AUP: 0x00000050, # analog up
  NOTE_ADOWN: 0x000000B0, # analog down
  NOTE_ALEFT: 0x0000B000, # analog left
  NOTE_ARIGHT: 0x00005000, # analog right

  2: 0x01000000, # d-pad up
  1: 0x02000000, # d-pad left
  3: 0x04000000, # d-pad down
  4: 0x08000000, # d-pad right
  36: 0x80000000, # A
  30: 0x40000000, # B
  27: 0x40000000, # B
  52: 0x40000000, # B
  63: 0x10000000, # Start
  47: 0x00080000, # C-up 
  40: 0x00040000, # C-down
  50: 0x00020000, # C-left
  58: 0x00010000, # C-right
  8: 0x00200000, # L
  51: 0x00100000, # R
  59: 0x00100000, # R
  12: 0x40000000, # B
  
  53: 0x00100000, # R toggle
}

cmap = {
  4: 0x20000000, # Z
  64: 0x80000000, # A
}

pmap = {
  12: 0x40000000, # B
  51: 0x80000000, # A
  18: 0x04000000, # d-pad left
  19: 0x08000000, # d-pad right
  20: 0x01000000, # d-pad up
  21: 0x02000000, # d-pad down
  57: 0x00200000, # L
}

def velocity_to_stick(vel):
    return int(np.interp(vel, velocity_values, stick_values))
    
def twos_comp(val, bits):
    return val & ((2 ** bits) - 1)

def center_analog_stick():
    global data_to_tastm32
    data_to_tastm32 &= 0xFFFF0000 # clear all analog directions
    
def center_vertical_axis():
    global data_to_tastm32
    data_to_tastm32 &= 0xFFFFFF00 # clear vertical axis
    
def center_horizontal_axis():
    global data_to_tastm32
    data_to_tastm32 &= 0xFFFF00FF # clear horizontal axis

def set_analog_stick(velocity, direction):
    global data_to_tastm32
    stick_val = velocity_to_stick(velocity)
    if(direction == 'up'):
        data_to_tastm32 |= stick_val
    elif(direction == 'down'):
        data_to_tastm32 |= (twos_comp(-stick_val,8))
    elif(direction == 'left'):
        data_to_tastm32 |= ((twos_comp(-stick_val,8)) << 8)
    elif(direction == 'right'):
        data_to_tastm32 |= (stick_val << 8)
    else:
        pass
        
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
ser.write(b'V1')
time.sleep(0.1)

# disable bulk transfer mode
ser.write(b'QA0')
time.sleep(0.1)

ser.ser.reset_input_buffer()

# try and connect to the MIDI drumset
device_list = mido.get_input_names()

choice = -1
choice2 = -1

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
        choice2 = input("Which device is your second drumset (please enter the number and hit <enter>)? ")

frame_start = time.time()

with mido.open_input(device_list[int(choice)]) as midi_device:
    with mido.open_input(device_list[int(choice2)]) as midi_device2:
        while True:
            # release A if timer is active and expired
            if A_time != -1 and current_milli_time() > (A_time + 10):
                data_to_tastm32 &= ~pmap[51]
                A_time = -1 # un-initialize A_time
            
            # release B if timer is active and expired
            if B_time != -1 and current_milli_time() > (B_time + 32):
                data_to_tastm32 &= ~pmap[12]
                B_time = -1 # un-initialize B_time
                
            # process the midi message
            for message in chain(midi_device.iter_pending(),midi_device2.iter_pending()):
                if toggle == 0 and message.type != "clock": #alesis spams clock messages
                    print(message) # fixes stuff cause of artificial delay, apparently, causing a smoothing effect
                if message.type == "note_on":
                    try:
                        if message.velocity == 0: # treat this as a note off for the weird midi device that doesnt do note off
                            if message.note == 36:
                                B_time = current_milli_time()
                        else:
                            if message.note == 49: # toggle switch
                                toggle = 1 - toggle
                                aup = 0
                                adown = 0
                                aleft = 0
                                aright = 0
                            elif message.note == NOTE_AUP: # analog up
                                if toggle == 1 or message.velocity == 127:
                                    center_analog_stick()
                                    if aup == 0:
                                        set_analog_stick(70, 'up') # velocity of 70 corresponds to an n64 value of 80
                                    aup = 1 - aup
                                    adown = 0
                                    aleft = 0
                                    aright = 0
                                else:
                                    aup = 0
                                    set_analog_stick(message.velocity, 'up')
                            elif message.note == NOTE_ADOWN: # analog down
                                if toggle == 1 or message.velocity == 127:
                                    center_analog_stick()
                                    if adown == 0:
                                        set_analog_stick(70, 'down') # velocity of 70 corresponds to an n64 value of 80
                                    adown = 1 - adown
                                    aup = 0
                                    aleft = 0
                                    aright = 0
                                else:
                                    adown = 0
                                    set_analog_stick(message.velocity, 'down')
                            elif message.note == NOTE_ALEFT: # analog left
                                if toggle == 1 or message.velocity == 127:
                                    center_analog_stick()
                                    if aleft == 0:
                                        set_analog_stick(70, 'left') # velocity of 70 corresponds to an n64 value of 80
                                    aleft = 1 - aleft
                                    adown = 0
                                    aup = 0
                                    aright = 0
                                else:
                                    aleft = 0
                                    set_analog_stick(message.velocity, 'left')
                            elif message.note == NOTE_ARIGHT: # analog right
                                if toggle == 1 or message.velocity == 127:
                                    center_analog_stick()
                                    if aright == 0:
                                        set_analog_stick(70, 'right') # velocity of 70 corresponds to an n64 value of 80
                                    aright = 1 - aright
                                    adown = 0
                                    aleft = 0
                                    aup = 0
                                else:
                                    aright = 0
                                    set_analog_stick(message.velocity, 'right')
                            elif message.note == 36: # A
                                data_to_tastm32 |= bmap[message.note]
                                if toggle == 0 and message.velocity > 50:
                                    A_time = current_milli_time()
                            elif message.note == 53: # A
                                R_toggle = 1 - R_toggle
                                data_to_tastm32 |= bmap[message.note]
                            else:
                                data_to_tastm32 |= bmap[message.note]
                    except KeyError:
                        pass
                elif message.type == "control_change":
                    try:
                        if message.control == 4:
                            if message.value >= 45:
                                data_to_tastm32 |= cmap[message.control]
                            else:
                                data_to_tastm32 &= ~cmap[message.control]
                        elif message.control == 64:
                            if message.value >= 90:
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
                        elif toggle == 1 and (message.note == NOTE_AUP or message.note == NOTE_ARIGHT or message.note == NOTE_ADOWN or message.note == NOTE_ALEFT): # ignore analog UDLR release in toggle mode
                            pass
                        elif (aup == 1 and message.note == NOTE_AUP) or (aright == 1 and message.note == NOTE_ARIGHT) or (adown == 1 and message.note == NOTE_ADOWN) or (aleft == 1 and message.note == NOTE_ALEFT) or (R_toggle == 1 and message.note == 53):
                            pass
                        elif message.note == 36 and A_time != -1: # A is pressed and its timer is set
                            pass
                        elif message.note == NOTE_AUP or message.note == NOTE_ADOWN:
                            center_vertical_axis()
                        elif message.note == NOTE_ARIGHT or message.note == NOTE_ALEFT:
                            center_horizontal_axis()
                        else:
                            data_to_tastm32 &= ~bmap[message.note]
                    except KeyError:
                        pass
            # prepare and send message to the replay device
            new_time = time.time()
            if new_time > frame_start+(1/30): # slightly less than 30 hz polling to ensure no input queuing
                if data_to_tastm32 != prev_data:
                    output_string = b'A' + data_to_tastm32.to_bytes(4, "big")
                    ser.write(output_string)
                    frame_start = new_time
                    prev_data = data_to_tastm32