#!/usr/bin/env python3
import serial, time, json
import serial_helper
import argparse_helper
import tastm32

parser = argparse_helper.audio_parser()
args = parser.parse_args()

if args.serial == None:
    dev = tastm32.TAStm32(serial_helper.select_serial_port())
else:
    dev = tastm32.TAStm32(args.serial)

tastm32.DEBUG = args.debug

def readUntil(dev, char):
  while True:
    try:
      c = dev.read(1)
      if c == b'':
        continue
      if c == char:
        return
    except serial.SerialException:
      print('ERROR: Serial Exception caught!')
      raise
    except KeyboardInterrupt:
      print('^C Exiting')
      raise

def readByte(dev):
  while True:
    try:
      c = dev.read(1)
      if c == b'':
        continue
      return c
    except serial.SerialException:
      print('ERROR: Serial Exception caught!')
      raise
    except KeyboardInterrupt:
      print('^C Exiting')
      raise

def readBytes(dev, size):
  chunks = []
  remaining = size
  while True:
    try:
      c = dev.read(1)
      if c == b'':
        continue
      remaining -= 1
      numBytes = dev.ser.inWaiting()
      if numBytes > 0:
        c += dev.read(min(numBytes, remaining))
        remaining -= min(numBytes, remaining)
      chunks.append(c)
      if remaining == 0:
        return b"".join(chunks)
    except serial.SerialException:
      print('ERROR: Serial Exception caught!')
      raise
    except KeyboardInterrupt:
      print('^C Exiting')
      raise

def readVarInt(dev):
  value = 0
  byte = 0
  while True:
    cb = readByte(dev)[0]
    value |= ( (cb & 0x7f) << (7*byte) )
    if cb & 0x80:
      byte += 1
      continue
    return value

def readVarArray(dev):
  arraysize = readVarInt(dev)
  print(f"--- array size: {arraysize}")
  items = []
  chunkremainder = b''
  while True:
    chunk = b''
    if arraysize > 1024:
      arraysize -= 1024
      chunk = readBytes(dev, 1024)
    else:
      chunk = readBytes(dev, arraysize)
      arraysize = 0
    splitchunks = (chunkremainder+chunk).split(b'\x00')
    chunkremainder = splitchunks.pop()
    for item in splitchunks:
      items.append(item.decode('utf8'))
       #items.append(item)
    if arraysize == 0:
      return items

print("--- Sending command dump info block")

dev.write(b'I')
readUntil(dev, b'I')
fields = readVarInt(dev)
print(f"--- fields: {fields}")
print("--- reading headers")
headers = readVarArray(dev)
print("--- reading values")
values = readVarArray(dev)
#print(headers)
#print(values)
#print([*zip(headers,values)])
fields = dict([*zip(headers, values)])
print("---fields", json.dumps(fields, indent=2))
