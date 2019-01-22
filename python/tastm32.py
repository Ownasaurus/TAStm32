#!/usr/bin/env python3
import sys
import serial
import struct
import time

import serial_helper
import argparse_helper

import r08, r16m, m64

DEBUG = True
int_buffer = 1024 # internal buffer size on replay device

int_to_byte_struct = struct.Struct('B')
def int_to_byte(interger):
    return int_to_byte_struct.pack(interger)

class TAStm32():
    def __init__(self, ser):
        try:
            self.ser = serial.Serial(ser, 115200, timeout=0)
        except serial.SerialException:
            print ('ERROR: the specified interface (' + ser + ') is in use')
            sys.exit(0)
        self.activeRuns = {b'A': False, b'B': False, b'C': False, b'D': False}

    def write(self, data):
        count = self.ser.write(data)
        if DEBUG and data != b'':
            print('S:', data)
        return count

    def read(self, count):
        data = self.ser.read(count)
        if DEBUG and data != b'':
            print('R:', data)
        return data

    def reset(self):
        self.write(b'R')
        time.sleep(1)
        data = self.read(2)
        if data == b'\x01R':
            return True
        else:
            raise RuntimeError('Error during reset')

    def get_run_prefix(self):
        if self.activeRuns[b'A']:
            if self.activeRuns[b'B']:
                if self.activeRuns[b'C']:
                    if self.activeRuns[b'D']:
                        return None
                    else:
                        self.activeRuns[b'D'] == True
                        return b'D'
                else:
                    self.activeRuns[b'C'] == True
                    return b'C'
            else:
                self.activeRuns[b'B'] == True
                return b'B'
        else:
            self.activeRuns[b'A'] == True
            return b'A'

    def setup_run(self, console, players=[1], dpcm=False, overread=False, window=0):
        prefix = self.get_run_prefix()
        if prefix == None:
            raise RuntimeError('No Free Run')
        if console == 'n64':
            cbyte = b'M'
            pbyte = 0
            for player in players:
                p = int(player)
                if p < 1 or p > 2:
                    raise RuntimeError('Invalid player for N64')
                else:
                    pbyte = pbyte ^ 2**(8-p)
            sbyte = 0
        elif console == 'snes':
            cbyte = b'S'
            pbyte = 0
            for player in players:
                p = int(player)
                if p < 1 or p > 8:
                    raise RuntimeError('Invalid player for SNES')
                else:
                    pbyte = pbyte ^ 2**(8-p)
            sbyte = 0
            if overread:
                sbyte = sbyte ^ 0x40
            # TODO HANDLE WINDOW MODE
        elif console == 'nes':
            cbyte = b'N'
            pbyte = 0
            for player in players:
                p = int(player)
                if p < 1 or p > 8:
                    raise RuntimeError('Invalid player for NES')
                else:
                    pbyte = pbyte ^ 2**(8-p)
            sbyte = 0
            if dpcm:
                sbyte = sbyte ^ 0x80
            if overread:
                sbyte = sbyte ^ 0x40
            # TODO HANDLE WINDOW MODE
        command = b'S' + prefix + cbyte + int_to_byte(pbyte) + int_to_byte(sbyte)
        self.write(command)
        time.sleep(1)
        data = self.read(2)
        if data == b'\x01S':
            return prefix
        else:
            self.activeRuns[prefix] = False
            raise RuntimeError('Error during setup')

def main():
    parser = argparse_helper.setup_parser_full()
    args = parser.parse_args()
    args.players = args.players.split(',')
    
    if args.serial == None:
        dev = TAStm32(serial_helper.select_serial_port())
    else:
        dev = TAStm32(args.serial)

    try:
        with open(args.movie, 'rb') as f:
            data = f.read()
    except:
        print('ERROR: the specified file (' + args.movie + ') failed to open')
        sys.exit(0)

    dev.reset()
    run_id = dev.setup_run(args.console, args.players, args.dpcm, args.overread, args.window)
    if run_id == None:
        raise RuntimeError('ERROR')
        sys.exit()
    if args.console == 'n64':
        buffer = m64.read_input(data)
        blankframe = b'\x00\x00\x00\x00' * len(args.players)
    elif args.console == 'snes':
        buffer = r16m.read_input(data)
        blankframe = b'\x00\x00' * len(args.players)
    elif args.console == 'nes':
        buffer = r08.read_input(data)
        blankframe = b'\x00' * len(args.players)

    # Send Blank Frames
    for blank in range(args.blank):
        data = run_id + blankframe
        dev.write(data)
        print('Sending Blank Latch: {}'.format(blank))
    fn = 0
    for latch in range(int_buffer-args.blank):
        data = run_id + buffer[fn]
        dev.write(data)
        print('Sending Latch: {}'.format(fn))
        fn += 1
    err = dev.read(int_buffer)
    fn -= err.count(b'\xB0')
    if err.count(b'\xB0') != 0:
        print('Buffer Overflow x{}'.format(err.count(b'\xB0')))

    print('Main Loop Start')
    while True:
        try:
            c = dev.read(1)
            if c == '':
                continue
            numBytes = dev.ser.inWaiting()
            if numBytes > 0:
                c += dev.read(numBytes)
                if numBytes > int_buffer:
                    print ("WARNING: High latch rate detected: " + str(numBytes))
            latches = c.count(run_id)
            missed = c.count(b'\xB0')
            if missed != 0:
                fn -= missed
                print('Buffer Overflow x{}'.format(err.count(b'\xB0')))
            for latch in range(latches):
                data = run_id + buffer[fn]
                dev.write(data)
                print('Sending Latch: {}'.format(fn))
                fn += 1
        except serial.SerialException:
            print('ERROR: Serial Exception caught!')
            break
        except KeyboardInterrupt:
            print('^C Exiting')
            break
        except IndexError:
            print('End of Input Exiting')
            break
    print('Exiting')
    dev.ser.close()
    sys.exit(0)

if __name__ == '__main__':
    main()