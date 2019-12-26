# TAStm32
Please see the "Projects" section for an overview of the current status of this project!

Please see the "Wiki" section for the technical details of the serial communication protocol!

### Overview

This project aims to create an easy-to-use, multi-system TAS replay device to be used on actual consoles. The name of this project comes from the fact that it is powered by a STM32F4 microcontroller.

### Installation

#### Windows

Install the [latest Python 3 release](https://www.python.org/downloads/windows/)

Log out/log in to refresh the PATH variable

    py -3 -m pip install pyserial
    py -3 -m pip install psutil

clone this repo

### Usage

Dumped TAS movies are available in the [TASBot projects repo](https://github.com/dwangoac/TASBot-Projects/tree/master/replayfiles)

[Scripts to dump your own TASes](https://github.com/dwangoac/TASBot-Projects/blob/master/Dump_Scripts/fceux_dump_latches.lua)

Once you've prepared a movie dump, use it with the TAStm32 as such:

    tastm32.py --console nes --players 1,5 --dpcm --clock 14 lordtom-maru-tompa-smb3-warps-improvement_-_subs_changes.frame.r08

### Special Thanks
#### TheMas3212 - The main contributor to this project. Aided with all things python, designing the serial communication protocol, helping with PCB design, and debugging throughout the whole process.
#### CraftedCart - Designed the awesome-looking 3D case
#### total (@tewtal) - Helped with NES/SNES protocol and debugging. Provided his code as a reference
#### booto (@booto) - Helped fix N64/GC protocols, serial-over-USB, and debugging

DwangoAC (@dwangoac) - warmly accepting me into the TASBot community

micro500 (@micro500) - spent a lot of time teaching me how console replay devices work at a low level

true - for teaching me hardware concepts, helping me choose the perfect MCU for the job, and explaining a lot of low-level things to me

Tien Majerle (@MaJerle) - the owner of https://stm32f4-discovery.net/ helped me with the function which jumps to the DFU bootloader code

Ilari - for being an extraordinary wealth of knowledge on most topics

Serisium (@serisium) - helped with debugging and PCB design

rcombs (@rcombs) - helped with optimizing the N64 protocol

#### If I accidentally forgot to credit you, please let me know so I can fix it!

 - Ownasaurus
