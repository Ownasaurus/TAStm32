@echo off
python python/tastm32-dfu.py
TIMEOUT /T 2
STM32_Programmer_CLI -c port=USB1 -d Releasev4\TAStm32.hex -s
