# TAStm32
Please see the "Projects" section for an overview of the current status of this project!

Please see the "Wiki" section for the technical details of the serial communication protocol!

### Overview

This project aims to create an easy-to-use, multi-system TAS replay device to be used on actual consoles. The name of this project comes from the fact that it is powered by a STM32F4 microcontroller.

## How to use this device

#### Get Python3!
Install the [latest Python 3 release](https://www.python.org/downloads/)

(You may need to log out/log in to refresh the PATH variable)

Then install the required python modules with the following command:

    python3 -m pip install -r python/requirements.txt
    
#### Linux users -- make sure you are in the dialout and plugdev groups!

### Get the python scripts

Download the latest python scripts from https://github.com/Ownasaurus/TAStm32/releases/download/latest/python.zip

### Usage

Dumped TAS movies are available in the [TASBot projects repo](https://github.com/dwangoac/TASBot-Projects/tree/master/replayfiles)

[Scripts to dump your own TASes](https://github.com/dwangoac/TASBot-Projects/blob/master/Dump_Scripts/)

Once you've prepared a movie dump, use it with the TAStm32. Here is one example example is here:

    python3 tastm32.py --console snes --players 1,5 --dpcm --clock 14 --transition 2123 A --blank 1 smb3_agdq.r16m
    
Another option is to use python to run main.py, allowing you to utilize a GUI crated by Zayitskin. You can find more information on his repo here: https://github.com/Zayitskin/TAStm32GUI

## Upgrading firmware 

#### Linux
    apt install dfu-util
OR the equivalent in your distribution. Then run:

    make v3 && make flash

Or :

    make v4 && make flash

#### Windows
1. Install STM32CubeProgrammer
1. Run the tastm32-dfu.py script to put the device in DFU mode
1. Use the STM32CubeProgrammer software to flash the latest firmware. The latest firmware can always be found here: https://github.com/Ownasaurus/TAStm32/releases/download/latest/tastm32v3.hex
or
https://github.com/Ownasaurus/TAStm32/releases/download/latest/tastm32v4.hex

## Development (advanced users)

Do you want to fork the codebase and make your own tweaks? Here is some information that may help you.

### Compilation

#### Linux
    apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
OR the equivalent in your distribution. Then run:

    make v3
    
Or : 

    make v4
    
Depending on if you have a v3 or v4 board. (v4 boards were released after July 2021)

#### Windows
1. Install Atollic TrueStudio
1. Right-click on the project > Build Configurations > Set Active, then click "Release v3" or "Release v4" depending on your board version. (v4s came out around July 2021)
1. Project > Build Project and check that the console shows no errors. The last line before "build finished" should be "arm-atollic-eabi-objcopy -O ihex TAStm32.elf TAStm32.hex".
1. Feel free to use the helper scripts "setenv.bat" and either "flashv3.bat" or "flashv4.bat" to assist in flashing firmware. These scripts will work if you have compiled using TrueStudio and if you have STM32CubeProgrammer installed.

### Special Thanks
#### rasteri (@rasteri) - Standalone USB playback, SNES multitap, v4 PCB, and more!
#### TheMas3212 (@TheMas3212) - Aided with all things python, designing the serial communication protocol, helping with PCB design, and debugging throughout the whole process.
#### total (@tewtal) - Helped with NES/SNES protocol and debugging. Provided his code as a reference
#### booto (@booto) - Helped fix N64/GC protocols, serial-over-USB, and debugging
#### Skippy - Pretty much helped a little bit with everything: including software, hardware, advice on proper tools, and more!

dwangoAC (@dwangoac) - warmly accepting me into the TASBot community

micro500 (@micro500) - spent a lot of time teaching me how console replay devices work at a low level

true - for teaching me hardware concepts, helping me choose the perfect MCU for the job, and explaining a lot of low-level things to me

Tien Majerle (@MaJerle) - the owner of https://stm32f4-discovery.net/ helped me with the function which jumps to the DFU bootloader code

CraftedCart - Designed the awesome-looking 3D case

Ilari - for being an extraordinary wealth of knowledge on most topics

Serisium (@serisium) - helped with debugging and PCB design

rcombs (@rcombs) - helped with optimizing the N64 protocol

Sauraen (@sauraen) - Linux setup and installation notes, multiplayer GC/N64 support, misc. GC/N64 improvements

Derf - worked on debugging the genesis protocol together

sspx (@SuperSpyTX) - For help with finding a bug in the N64 controller identity code.

#### If I accidentally forgot to credit you, please let me know so I can fix it!

 - Ownasaurus
