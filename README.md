# TAStm32
Please see the "Projects" section for an overview of the current status of this project!

Please see the "Wiki" section for the technical details of the serial communication protocol!

### Overview

This project aims to create an easy-to-use, multi-system TAS replay device to be used on actual consoles. The name of this project comes from the fact that it is powered by a STM32F4 microcontroller.

### Prerequisites

First clone or download this repository

#### Get Python3!
Install the [latest Python 3 release](https://www.python.org/downloads/)

(You may need to log out/log in to refresh the PATH variable)

Then install the required python modules with the following command:

    python3 -m pip install -r python/requirements.txt
    
#### Linux users -- make sure you are in the dialout and plugdev groups!
    
### Compilation

#### Linux
    apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
OR the equivalent in your distribution

    make

#### Windows

1. Right-click on the project > Build Configurations > Set Active > Release.
1. Project > Build Project and check that the console shows no errors. The last line before "build finished" should be "arm-atollic-eabi-objcopy -O ihex TAStm32.elf TAStm32.hex".

### Flashing the firmware

#### Linux
    apt install dfu-util
OR the equivalent in your distribution

    make flash

#### Windows
1. Install Atollic TrueStudio
1. Install STM32CubeProgrammer
1. Open up the command prompt
1. Run setenv.bat included in the root directory
1. Run update.bat included in the root directory

### Usage

Dumped TAS movies are available in the [TASBot projects repo](https://github.com/dwangoac/TASBot-Projects/tree/master/replayfiles)

[Scripts to dump your own TASes](https://github.com/dwangoac/TASBot-Projects/blob/master/Dump_Scripts/)

Once you've prepared a movie dump, use it with the TAStm32. Here is one example example is here:

    python3 tastm32.py --console snes --players 1,5 --dpcm --clock 14 --transition 2123 A --blank 1 smb3_agdq.r16m

### Special Thanks
#### TheMas3212 - The main contributor to this project. Aided with all things python, designing the serial communication protocol, helping with PCB design, and debugging throughout the whole process.
#### CraftedCart - Designed the awesome-looking 3D case
#### total (@tewtal) - Helped with NES/SNES protocol and debugging. Provided his code as a reference
#### booto (@booto) - Helped fix N64/GC protocols, serial-over-USB, and debugging
#### rasteri (@rasteri) - Standalone USB playback, SNES multitap

dwangoAC (@dwangoac) - warmly accepting me into the TASBot community

micro500 (@micro500) - spent a lot of time teaching me how console replay devices work at a low level

true - for teaching me hardware concepts, helping me choose the perfect MCU for the job, and explaining a lot of low-level things to me

Tien Majerle (@MaJerle) - the owner of https://stm32f4-discovery.net/ helped me with the function which jumps to the DFU bootloader code

Ilari - for being an extraordinary wealth of knowledge on most topics

Serisium (@serisium) - helped with debugging and PCB design

rcombs (@rcombs) - helped with optimizing the N64 protocol

Sauraen (@sauraen) - Linux setup and installation notes, along with additional work on the GC/N64 protocol.

sspx (@SuperSpyTX) - For help with finding a bug in the N64 controller identity code.

#### If I accidentally forgot to credit you, please let me know so I can fix it!

 - Ownasaurus
