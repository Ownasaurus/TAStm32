#!/usr/bin/env python3
import serial_helper
import argparse_helper
import tastm32
import time
from inputs import devices

#Triggers: 0-255
#ABS_Z is left trigger
#ABS_RZ is right trigger

#Analogs:
#ABS_X, ABS_Y, ABS_RX, ABS_RX -32768 to 32767
#ABS_HAT0X tristate: -1 = left, 0 = neutral, 1 = right
#ABS_HAT0Y tristate: -1 = UP, 0 = neutral, 1 = DOWN

#Buttons:
#BTN_TL / BTN_TR are bumpers, 0 or 1
#BTN_EAST, NORTH, SOUTH, and WEST are face buttons. 0 or 1.
#BTN_SELECT and BTN_START are the two middle buttons for whatever xbox family symbol thingy, 0 or 1.
#analog sticks pushed in = BTN_THUMBL, BTN_THUMBR
#Left analog stick:

TRIGGER_THRESHOLD = 125
XBOX_R_ANALOG_THRESHOLD = 20000
N64_DEADZONE = 20

# bit masks and button maps
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

chosenGamepad = 0

def init():
    global ser
    # try and connect to the TAStm32
    parser = argparse_helper.audio_parser()
    args = parser.parse_args()

    if args.serial == None:
        dev = tastm32.TAStm32(serial_helper.select_serial_port())
    else:
        dev = tastm32.TAStm32(args.serial)
    ser = dev

    # BEGIN player selection
    # END player selection

    # BEGIN USB controller selection
    numGamepads = len(devices.gamepads)
    if numGamepads == 1:
        chosenGamepad = 0
    elif numGamepads > 1:
        for i in range(len(devices.gamepads)):
            print(f"{i}) --{devices.gamepads[i]}--")
        chosenGamepad = int(input("Which device is your gamepad? "))
    else:
        print("ERROR: No gamepads found! Exiting....")
        exit()
    # END USB controller selection

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
    
    # start with gamepad neutral
    ser.write(bytes([65,0,0,0,0]))

def main():
    global ser
    # keep track of controller state
    data_to_tastm32 = 0
    prev_data = 0
    
    # BEGIN main loop
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
                if event.code == 'ABS_HAT0Y':
                    if event.state == -1: # up
                        data_to_tastm32 |= 0x08000000
                    elif event.state == 1: # down
                        data_to_tastm32 |= 0x04000000
                    elif event.state == 0: # neutral
                        data_to_tastm32 &= 0xF3FFFFFF
                elif event.code == 'ABS_HAT0X':
                    if event.state == -1: # left
                        data_to_tastm32 |= 0x02000000
                    elif event.state == 1: # right
                        data_to_tastm32 |= 0x01000000
                    elif event.state == 0: # neutral
                        data_to_tastm32 &= 0xFCFFFFFF
                elif event.code == 'ABS_X':
                    N64_X_val = (event.state) // 384
                    abs_X_val = abs(N64_X_val)
                    data_to_tastm32 &= 0xFFFF00FF
                    if abs_X_val >= N64_DEADZONE: # out of neutral position
                        sign_X_val = 0
                        if N64_X_val < 0:
                            sign_X_val = 1
                            abs_X_val = 128 - abs_X_val
                        data_to_tastm32 |= ((abs_X_val << 8) | (sign_X_val << 15))
                elif event.code == 'ABS_Y':
                    N64_Y_val = (event.state) // 384
                    abs_Y_val = abs(N64_Y_val)
                    data_to_tastm32 &= 0xFFFFFF00
                    if abs_Y_val >= N64_DEADZONE: # out of neutral position
                        sign_Y_val = 0
                        if N64_Y_val < 0:
                            sign_Y_val = 1
                            abs_Y_val = 128 - abs_Y_val
                        data_to_tastm32 |= (abs_Y_val | (sign_Y_val << 7))
                elif event.code == 'ABS_RX':
                    abs_X_val = abs(event.state)
                    if abs_X_val < XBOX_R_ANALOG_THRESHOLD: # Neutral position
                        data_to_tastm32 &= 0xFFFCFFFF
                    else:
                        if event.state < 0: # Left
                            data_to_tastm32 |= 0x00020000
                        else: # Right
                            data_to_tastm32 |= 0x00010000
                elif event.code == 'ABS_RY':
                    abs_Y_val = abs(event.state)
                    if abs_Y_val < XBOX_R_ANALOG_THRESHOLD: # Neutral position
                        data_to_tastm32 &= 0xFFF3FFFF
                    else:
                        if event.state < 0: # Down
                            data_to_tastm32 |= 0x00040000
                        else: # Up
                            data_to_tastm32 |= 0x00080000
            # prepare and send message to the replay device
            if data_to_tastm32 != prev_data:
                output_string = b'A' + data_to_tastm32.to_bytes(4, "big")
                ser.write(output_string)
                prev_data = data_to_tastm32
    # END main loop

if __name__ == "__main__":
    # execute only if run as a script
    init()
    main()