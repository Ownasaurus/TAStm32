#!/usr/bin/env python3
import argparse
import sys

def setup_parser():
    parser = argparse.ArgumentParser(description='...')
    parser.add_argument('--serial', help='Preselect the serial port')
    parser.add_argument('--blank', help='Number of blank frames to prepend to input', type=int, default=0)
    parser.add_argument('movie', help='Path to the movie file to play')
    return parser

def setup_parser_full():
    parser = argparse.ArgumentParser(description='...')
    parser.add_argument('--serial', help='Preselect the serial port')
    parser.add_argument('--blank', help='Number of blank frames to prepend to input', type=int, default=0)
    parser.add_argument('--console', help='Set the console', choices=['n64', 'snes', 'nes', 'gc', 'genesis'], required=True)
    parser.add_argument('--players', help='Comma seperated list of players', default='1')
    parser.add_argument('--debug', help='Enable Debug Mode', action='store_true')
    parser.add_argument('--dpcm', help='Enable dpcm fix', action='store_true')
    parser.add_argument('--hardreset', help='Perform a hard/slow reset before the run begins', action='store_true')
    parser.add_argument('--softreset', help='Perform a quick/fast reset before the run begins', action='store_true')
    parser.add_argument('--clock', help='Enable clock filter. Value must be between 0 and 63. This number gets multiplied by 0.25us')
    parser.add_argument('--overread', help='Set overread value', action='store_true')
    parser.add_argument('--transition', help='Add a transition', nargs=2, action='append')
    parser.add_argument('--latchtrain', help='Configure latch train', default='')
    parser.add_argument('--nobulk', help='Disable Bulk Transfer Mode', action='store_true')
    # parser.add_argument('--window', help='Set window mode', type=float, default=0)
    parser.add_argument('movie', help='Path to the movie file to play')
    return parser

def audio_parser():
    parser = argparse.ArgumentParser(description='...')
    parser.add_argument('--serial', help='Preselect the serial port')
    parser.add_argument('--debug', help='Enable Debug Mode', action='store_true')
    return parser
