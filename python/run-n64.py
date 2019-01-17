#!/usr/bin/env python3
import sys
import serial
import m64

int_buffer = 1024 # internal buffer size on replay device
run_id = b'A'

DEBUG = False

def serial_read(ser, count):
    data = ser.read(count)
    if len(data) != 0 and DEBUG:
        print('R: {}'.format(data))
    return data

def serial_write(ser, data):
    r = ser.write(data)
    if DEBUG:
        print('S: {}'.format(data))
    return r

def serial_wait_for(ser, flag):
    data = b''
    while len(data) < len(flag):
        data += serial_read(ser, len(flag)-len(data))
    if data != flag:
        raise RuntimeError()

def main():
    if len(sys.argv) != 3:
        sys.stderr.write('Usage: ' + sys.argv[0] + ' <interface> <movie file>\n\n')
        sys.exit(0)
    try:
        ser = serial.Serial(sys.argv[1], 115200, timeout=1)
    except serial.SerialException:
        print ('ERROR: the specified interface (' + sys.argv[1] + ') is in use')
        sys.exit(0)
    try:
        with open(sys.argv[2], 'rb') as f:
            data = f.read()
    except:
        print('ERROR: the specified file (' + sys.argv[2] + ') failed to open')
        sys.exit(0)
    buffer = m64.read_input(data)
    serial_write(ser, b'R') # send RESET command
    serial_wait_for(ser, b'\x01R')
    serial_write(ser, b'S' + run_id + b'M\x01')
    serial_wait_for(ser, b'\x01S')
    fn = 0
    for latch in range(int_buffer):
        data = run_id + buffer[fn]
        serial_write(ser, data)
        print('Sending Frame: {}'.format(fn))
        fn += 1
    done = False
    print('Main Loop Start')
    while not done:
        try:
            c = serial_read(ser, 1)
            if c == '':
                continue
            numBytes = ser.inWaiting()
            if numBytes > 0:
                c += serial_read(ser, numBytes)
                if numBytes > int_buffer:
                    print ("WARNING: High frame read detected: " + str(numBytes))
            if DEBUG:
                print('R: {}'.format(c))
            latches = c.count(run_id)
            for latch in range(latches):
                data = run_id + buffer[fn]
                serial_write(ser, data)
                print('Sending Frame: {}'.format(fn))
                fn += 1
        except serial.SerialException:
            print('ERROR: Serial Exception caught!')
            done = True
        except KeyboardInterrupt:
            print('^C Exiting')
            done = True
        except IndexError:
            print('End of Input Exiting')
            done = True
    print('trying to exit')
    try:
        ser.close()
    except serial.SerialException:
        sys.exit(0)
    ser.close()
    sys.exit(0)

if __name__ == '__main__':
    main()