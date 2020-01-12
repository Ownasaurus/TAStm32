# TAStm32
Please see the "Projects" section for an overview of the current status of this project!

Please see the "Wiki" section for the technical details of the serial communication protocol!

### Overview

This project aims to create an easy-to-use, multi-system TAS replay device to be used on actual consoles. The name of this project comes from the fact that it is powered by a STM32F4 microcontroller.

### Installation

Install the [latest Python 3 release](https://www.python.org/downloads/windows/)

Log out/log in to refresh the PATH variable

    py -3 -m pip install pyserial
    py -3 -m pip install psutil

clone this repo

### Linux installation

1. Install Atollic TrueStudio (not STM32CubeIDE).
1. In the project properties, under C/C++ Build > Settings > Build Steps > Post-build steps, delete the ".exe" from arm-atollic-eabi-objcopy.exe. Then switch to the other configuration at the top (Debug/Release) and do the same there. Click Apply / OK.
1. Right-click on the project > Build Configurations > Set Active > 2 Release.
1. Open Src/usbd_desc.c and change USBD_MANUFACTURER_STRING to something unique (e.g. "Ownasaurus & your_name"); this is so you can be sure that your modified firmware is running on the TAStm32.
1. Project > Build Project and check that the console shows no errors. The last line before "build finished" should be "arm-atollic-eabi-objcopy -O ihex TAStm32.elf TAStm32.hex".
1. sudo apt install dfu-util (not STM32CubeProgrammer)
1. Set your board into DFU mode (switch on) and plug it in.
1. cd Release/
1. dfu-util -a 0 -s 0x08000000:leave -D TAStm32.binary (omit ":leave" if you are using the hardware switch to select DFU mode)
1. Unplug the board, turn off the switch, plug it in again. ls /dev should show /dev/ttyACM0.
1. Optional: sudo apt install usbutils; (then run) usb-devices. Among all the devices you should see "TAStm32 Virtual ComPort" with your custom manufacturer string.
1. sudo usermod -a -G dialout your_username; then log out of ALL shells/sessions (if you're not a power user, just reboot).
1. Set up your Python environment configuration system of choice (I use pyenv) to use a reasonably recent version of python3 in the TAStm32 directory.
1. pip install pyserial psutil


### Usage

Dumped TAS movies are available in the [TASBot projects repo](https://github.com/dwangoac/TASBot-Projects/tree/master/replayfiles)

[Scripts to dump your own TASes](https://github.com/dwangoac/TASBot-Projects/blob/master/Dump_Scripts/)

Once you've prepared a movie dump, use it with the TAStm32. Here is one example example is here:

    py -3 tastm32.py --console snes --players 1,5 --dpcm --clock 14 --transition 2123 A --blank 1 smb3_agdq.r16m

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

Sauraen (@sauraen) - N64 four-player support and Linux notes

#### If I accidentally forgot to credit you, please let me know so I can fix it!

 - Ownasaurus
