#!/usr/bin/env python3
import struct
import sys

def read_header(data):
    return None

def read_input(data, players=[1,2,3,4,5,6,7,8]):
    frame_struct = struct.Struct('2s2s2s2s2s2s2s2s')
    frame_iter = frame_struct.iter_unpack(data)
    input_data = []
    for frame in frame_iter:
        fd = b''
        player = 1
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
        print(f'Usage {sys.argv[0]} <movie file>')
        sys.exit()
    with open(file, 'rb') as f:
        data = f.read()
    try:
        players = sys.arv[2].split(',')
    except:
        players = [1,2,3,4,5,6,7,8]
    inputs = read_input(data, players)
    print(inputs[20])

if __name__ == '__main__':
    main()