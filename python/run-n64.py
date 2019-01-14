#!/usr/bin/env python3
import sys
import serial
import m64

int_buffer = 768 # internal buffer size on replay device
run_id = b'A'

def main():
    if len(sys.argv) != 3:
        sys.stderr.write('Usage: ' + sys.argv[0] + ' <interface> <movie file>\n\n')
        sys.exit(0)
    try:
        ser = serial.Serial(sys.argv[1], 2000000, timeout=1)
    except SerialException:
        print ('ERROR: the specified interface (' + sys.argv[1] + ') is in use')
        sys.exit(0)
    try:
        with open(sys.argv[2], 'rb') as f:
            data = f.read()
    except:
        print('ERROR: the specified file (' + sys.argv[2] + ') failed to open')
    buffer = m64.read_input(data)
    ser.write(b'R') # send RESET command
    ser.write(b'S' + run_id + b'M\x01')
    for latch in buffer[:int_buffer]:
        ser.write(run_id)
        ser.write(latch)
    fn = int_buffer
    done = False
    print('Main Loop Start')
    while not done:
        try:
            c = ser.read(1)
            if c == '':
                continue
            numBytes = ser.inWaiting()
            if numBytes > 0:
                c += ser.read(numBytes)
                if numBytes > int_buffer:
                    print ("WARNING: High frame read detected: " + str(numBytes))
            latches = c.count(run_id)
            for latch in range(latches):
                ser.write(run_id)
                ser.write(buffer[fn])
                fn += 1
        except KeyboardInterrupt:
            print('^C Exiting')
            done = True
        except IndexError:
            print('End of Input Exiting')
            done = True
    ser.close()
    sys.exit(0)

if __name__ == '__main__':
    main()