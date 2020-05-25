#!/usr/bin/env python3
import struct
import sys

# Header format
#   000 16-byte signature and format version: "Gens Movie TEST9"
#   00F ASCII-encoded GMV file format version. The most recent is 'A'. (?)
#   010 4-byte little-endian unsigned int: rerecord count
#   014 ASCII-encoded controller config for player 1. '3' or '6'.
#   015 ASCII-encoded controller config for player 2. '3' or '6'.
#   016 special flags (Version A and up only):
#      bit 7(most significant): if "1", movie runs at 50 frames per second; if "0", movie runs at 60 frames per second
#      bit 6: if "1", movie requires a savestate.
#      bit 5: if "1", movie is 3-player movie; if "0", movie is 2-player movie
#   018 40-byte zero-terminated ASCII movie name string
#   040 frame data

def read_header(data):
    signature, = struct.unpack('<15s', data[:0xf])
    if signature != b'Gens Movie TEST':
        print(signature)
        print(len(signature))
        raise RuntimeError('Bad movie signature')
    header = {'signature': signature}
    values = struct.unpack('<cIccB40s', data[0xf:0x3f])
    header["version"] = values[0]
    header["rerecord_count"] = values[1]
    header["p1"] = values[2]
    header["p2"] = values[3]
    header["fps"] = "50" if (bool(int(values[4]) & 0x80)) else "60"
    header["savestate_required"] = bool(int(values[4]) & 0x40)
    header["3players"] = (bool(int(values[4]) & 0x20))
    header["movie_name"] = values[5].decode("ascii'").rstrip()
    return header

def read_input(data, header=None):
    if header == None:
        header = read_header(data)
    start = 0x40
    input_struct = struct.Struct('3s')
    input_iter = input_struct.iter_unpack(data[start:])
    input_data = []
    for frame in input_iter:
      input_data.append(frame)
    return input_data

def main():
    try:
        file = sys.argv[1]
    except:
        print(f'Usage {sys.argv[0]} <movie file>')
        sys.exit()
    with open(file, 'rb') as f:
        data = f.read()
    header = read_header(data)
    for k, v in header.items():
        if 'unused' in k:
            continue
        else:
            print('{}: {}'.format(k, v))
    inputs = read_input(data, header)
    print(inputs[:20])

if __name__ == '__main__':
    main()