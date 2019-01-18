#!/usr/bin/env python3
import struct
import sys

def read_header(data):
    return None

def read_input(data, players=[0,1]):
    frame_struct = struct.Struct('ss')
    frame_iter = frame_struct.iter_unpack(data)
    input_data = []
    for frame in frame_iter:
        fd = b''
        player = 0
        for pd in frame:
            if player in players:
                fd += pd
            player += 1
        input_data.append(fd)
    return input_data

def main():
    try:
        file = sys.argv[1]
    except:
        print('Usage {} <movie file>')
        sys.exit()
    with open(file, 'rb') as f:
        data = f.read()
    try:
        players = sys.arv[2].split(',')
    except:
        players = [0,1]
    inputs = read_input(data, players)
    print(inputs[20])

if __name__ == '__main__':
    main()