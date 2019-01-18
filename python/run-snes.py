#!/usr/bin/env python3
import sys
import serial
import r16m

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

def serial_wait_for(ser, flag, err=None):
    data = b''
    while len(data) < len(flag):
        data += serial_read(ser, len(flag)-len(data))
    if data != flag:
        raise RuntimeError(err)

def main():
    if not len(sys.argv) in [3,4]:
        sys.stderr.write('Usage: ' + sys.argv[0] + ' <interface> <movie file>\n\n')
        sys.exit(0)
    try:
        ser = serial.Serial(sys.argv[1], 115200, timeout=0.1)
    except serial.SerialException:
        print ('ERROR: the specified interface (' + sys.argv[1] + ') is in use')
        sys.exit(0)
    try:
        with open(sys.argv[2], 'rb') as f:
            data = f.read()
    except:
        print('ERROR: the specified file (' + sys.argv[2] + ') failed to open')
        sys.exit(0)
    try:
        blanks = int(sys.argv[3])
    except:
        blanks = 0
    buffer = r16m.read_input(data, [0])
    serial_write(ser, b'R') # send RESET command
    err = serial_read(2)
    if err == b'\x01R':
        pass
    elif err == b'\xFF':
        raise RuntimeError('Prefix not recognised')
    else:
        raise RuntimeError('Unknown error during reset')
    serial_write(ser, b'S' + run_id + b'S\x01')
    err = serial_read(2)
    if err == b'\x01S':
        pass
    elif err == b'\xFF':
        raise RuntimeError('Prefix not recognised')
    elif err == b'\xFE':
        raise RuntimeError('Run number not recognised')
    elif err == b'\xFD':
        raise RuntimeError('Number of controllers not recognised')
    elif err == b'\xFC':
        raise RuntimeError('Console Type not recognised')
    else:
        raise RuntimeError('Unknown error during run setup')
    for blank in range(blanks):
        data = run_id + b'\x00\x00'
        serial_write(ser, data)
        err = serial_read(1)
        if err == b'\xB0':
            break
        print('Sending Blank Latch: {}'.format(blank))
    fn = 0
    for latch in range(int_buffer-blanks):
        data = run_id + buffer[fn]
        serial_write(ser, data)
        err = serial_read(1)
        if err == b'\xB0':
            break
        print('Sending Latch: {}'.format(fn))
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
                    print ("WARNING: High latch rate detected: " + str(numBytes))
            if DEBUG:
                print('R: {}'.format(c))
            latches = c.count(run_id)
            for latch in range(latches):
                data = run_id + buffer[fn]
                serial_write(ser, data)
                err = serial_read(1)
                if err == b'\xB0':
                    break
                print('Sending Latch: {}'.format(fn))
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