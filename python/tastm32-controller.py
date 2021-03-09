#!/usr/bin/env python3
import serial_helper
import argparse_helper
import tastm32
import time
import multiprocessing as mp
import queue
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

def init():
    global players
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
    numPlayers = int(input("How many players (1 or 2)? "))
    if numPlayers != 1 and numPlayers != 2:
        print("ERROR: Invalid number of players! Exiting....")
        exit()
    # END player selection

    # BEGIN USB controller selection
    players = []
    numGamepads = len(devices.gamepads)
    if numGamepads >= numPlayers:
        for p in range(numPlayers):
            for i in range(len(devices.gamepads)):
                print(f"{i}) --{devices.gamepads[i]}--")
            chosenGamepad = int(input(f"Player {p+1}: Which device is your gamepad? "))
            players.insert(p, chosenGamepad)
    else:
        print(f"ERROR: Not enough gamepads found (only found {numGamepads})! Exiting....")
        exit()
    # make sure p1 and p2 are not mapped to the same gamepad
    if len(players) == 2 and players[0] == players[1]:
        print("ERROR: P1 and P2 cannot use the same gamepad! Exiting....")
        exit()
    # END USB controller selection

    # initialize tastm32
    ser.write(b'R')
    time.sleep(0.1)
    cmd = ser.read(2)
    print(bytes(cmd))

    # make sure nothing is buffered
    ser.enable_controller()

    # set up the N64 correctly
    if numPlayers == 1:
        ser.write(b'SAM\x80\x00')
    elif numPlayers == 2:
        ser.write(b'SAM\x88\x00')
    else:
        print("ERROR: It should be impossible to reach this line of code! Exiting....")
        exit(0)
    time.sleep(0.1)
    cmd = ser.read(2)
    print(bytes(cmd))

    # no bulk transfer
    ser.write(b'QA0')
    time.sleep(0.1)

    # clear the input buffer
    ser.ser.reset_input_buffer()
    
    # start with gamepad neutral
    if len(players) == 1:
        ser.write(bytes([65,0,0,0,0]))
    elif len(players) == 2:
        ser.write(bytes([65,0,0,0,0,0,0,0,0]))

def readController(controller_id, queue):
    controller = devices.gamepads[controller_id]
    while True:
        events = []
        for event in controller.read():
            events.append({"ev_type": event.ev_type,
                           "code": event.code,
                           "state": event.state})
        queue.put(events)

def main():
    global ser
    global players
    # keep track of controller state
    data_to_tastm32 = []
    for p in range(len(players)):
        data_to_tastm32.insert(p, 0)
    
    # set up multiprocessing
    processes = []
    controllers = []
    
    for controller_id in players:
        q = mp.Queue()
        p = mp.Process(target = readController,
                                   args = (controller_id, q,))
        p.start()
        processes.append(p)
        controllers.append(q)

    # BEGIN main loop
    try:
        while 1:
            output_string = b'A' # reset output string to just have the prefix
            for p, controller in enumerate(controllers):
                try:
                    events = controllers[p].get_nowait()
                except queue.Empty:
                    output_string += data_to_tastm32[p].to_bytes(4, "big")
                    continue
                for event in events:
                    if event["ev_type"] == 'Key': # handle button presses
                        if event["state"] == 1: # pressed
                            data_to_tastm32[p] |= btn_codes[event["code"]]
                        elif event["state"] == 0: # released
                            data_to_tastm32[p] &= ~btn_codes[event["code"]]
                    elif event["ev_type"] == 'Absolute': # handle analog inputs
                        if event["code"] == 'ABS_Z' or event["code"] == 'ABS_RZ':
                            if event["state"] >= TRIGGER_THRESHOLD:
                                data_to_tastm32[p] |= N64_Z
                            else:
                                data_to_tastm32[p] &= ~N64_Z
                        if event["code"] == 'ABS_HAT0Y':
                            if event["state"] == -1: # up
                                data_to_tastm32[p] |= 0x08000000
                            elif event["state"] == 1: # down
                                data_to_tastm32[p] |= 0x04000000
                            elif event["state"] == 0: # neutral
                                data_to_tastm32[p] &= 0xF3FFFFFF
                        elif event["code"] == 'ABS_HAT0X':
                            if event["state"] == -1: # left
                                data_to_tastm32[p] |= 0x02000000
                            elif event["state"] == 1: # right
                                data_to_tastm32[p] |= 0x01000000
                            elif event["state"] == 0: # neutral
                                data_to_tastm32[p] &= 0xFCFFFFFF
                        elif event["code"] == 'ABS_X':
                            N64_X_val = (event["state"]) // 384
                            abs_X_val = abs(N64_X_val)
                            data_to_tastm32[p] &= 0xFFFF00FF
                            if abs_X_val >= N64_DEADZONE: # out of neutral position
                                sign_X_val = 0
                                if N64_X_val < 0:
                                    sign_X_val = 1
                                    abs_X_val = 128 - abs_X_val
                                data_to_tastm32[p] |= ((abs_X_val << 8) | (sign_X_val << 15))
                        elif event["code"] == 'ABS_Y':
                            N64_Y_val = (event["state"]) // 384
                            abs_Y_val = abs(N64_Y_val)
                            data_to_tastm32[p] &= 0xFFFFFF00
                            if abs_Y_val >= N64_DEADZONE: # out of neutral position
                                sign_Y_val = 0
                                if N64_Y_val < 0:
                                    sign_Y_val = 1
                                    abs_Y_val = 128 - abs_Y_val
                                data_to_tastm32[p] |= (abs_Y_val | (sign_Y_val << 7))
                        elif event["code"] == 'ABS_RX':
                            abs_X_val = abs(event["state"])
                            if abs_X_val < XBOX_R_ANALOG_THRESHOLD: # Neutral position
                                data_to_tastm32[p] &= 0xFFFCFFFF
                            else:
                                if event["state"] < 0: # Left
                                    data_to_tastm32[p] |= 0x00020000
                                else: # Right
                                    data_to_tastm32[p] |= 0x00010000
                        elif event["code"] == 'ABS_RY':
                            abs_Y_val = abs(event["state"])
                            if abs_Y_val < XBOX_R_ANALOG_THRESHOLD: # Neutral position
                                data_to_tastm32[p] &= 0xFFF3FFFF
                            else:
                                if event["state"] < 0: # Down
                                    data_to_tastm32[p] |= 0x00040000
                                else: # Up
                                    data_to_tastm32[p] |= 0x00080000
                # prepare message to the replay device
                output_string += data_to_tastm32[p].to_bytes(4, "big")
            ser.write(output_string) # send it!
        # END main loop
    except KeyboardInterrupt:
        pass
    for process in processes:
        process.terminate()

if __name__ == "__main__":
    # execute only if run as a script
    init()
    main()