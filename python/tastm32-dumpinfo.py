#!/usr/bin/env python3
import serial, time
import serial_helper
import argparse_helper
import tastm32

parser = argparse_helper.audio_parser()
args = parser.parse_args()

if args.serial == None:
    dev = tastm32.TAStm32(serial_helper.select_serial_port())
else:
    dev = tastm32.TAStm32(args.serial)

def readUntil(dev, char):
  while True:
    try:
      c = dev.read(1)
      if c == '':
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
      if c == '':
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
      if c == '':
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
    cb = readByte(dev)
    value |= ( cb & 0x7f << (7*byte) )
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
      chunk = readBytes(1024)
    else:
      chunk = readBytes(arraysize)
      arraysize = 0
    splitchunks = (chunkremainder+chunk).split(b'\x00')
    chunkremainder = splitchunks.pop()
    for header in splitchunks:
      items.push(header.decode('utf8'))
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
fields = dict(zip(headers, values))
print("---fields", fields)

