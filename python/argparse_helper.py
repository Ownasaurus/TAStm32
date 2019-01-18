#!/usr/bin/env python3
import argparse
import sys

def setup_parser():
    parser = argparse.ArgumentParser(description='...')
    parser.add_argument('--serial', help='Preselect the serial port')
    parser.add_argument('--blank', help='Number of blank frames to prepend to input', type=int, default=0)
    parser.add_argument('movie', help='Path to the movie file to play')

    return parser
    