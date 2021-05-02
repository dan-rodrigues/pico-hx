# PicoHX (WIP, not fully tested)

This is an FPGA board that combines an iCE40 HX FPGA with the Raspberry Pi Pico. The Pico can program the FPGA over USB using a script included in this repo and then interact with it using GPIO.

![PCB rendering](doc/render.png)

## Features

* iCE40 HX FPGA (TQFP144, 1k pinout assumed)
* 10 Pico GPIOs connected to FPGA
    * 2 are connected to `GBIN` inputs
* 4x PMODs in a 2x double-PMOD arrangement
* 8x user LEDs for FPGA
* 1x CDONE LED
* 2x users buttons for FPGA
* 1x user button for Pico
* `RUN` button for Pico
* Headers for Pico SWD and hardware UART
* Footprint for QSPI PSRAM or flash
* Micro SD card slot

## Design

### Programming

All iCE40 configuration pins are routed to the RP2040 GPIO so the details of how or when it is programmed are up to the software. The included software in this repo will make the Pico present itself is a USB device with these interfaces:

* Vendor programmer interface: This is how the user can program the iCE40 at any time over USB, using the included picoprog.py script.
* CDC UART: This UART is separate to the hardware UART that is available on the dedicated pin header. By default, `stdout` is sent to the CDC UART for convenience.

### FPGA clock

The pico exposes one pin that can directly output a clock from one of its PLLs with an optional divider. In the demo software, the USB PLL (48MHz) is divided by 4 to output a 12MHz clock for the FPGA. On the FPGA side, the `GBIN5` pin is used for clock input so that the  `SB_PLL40_2F_PAD` can be used.

### Power

There is only a 1.2v regulator on the board as the Pico provides a 3.3v output. The `2V5_VPP` input is tied to 3.3v as the NVCM isn't used in this project.

### PIO support

The 10 GPIOs connected to the FPGA and 4 GPIOs connected to the SD card slot are in consecutive order, so PIO can be used.
 
## Usage

### Building Pico software

It's assumed that the [pico-sdk](https://github.com/raspberrypi/pico-sdk) is installed and the required environment variables are set.

```
cd software
mkdir build
cd build
cmake ..
make
```

The software can then be flashed using `picotool` for example. Default behaviour of the software is to immediately flash the iCE40 with an included bitstream that runs a counter with output to the 8 user LEDs.

### Programming iCE40 bitstream

After the Pico software is flashed, it's ready to program the iCE40 using a custom USB driver script. The script is written in Python3 and requires [pyusb](https://github.com/pyusb/pyusb).

```
pip3 install pyusb
```

It can then be programmed with similar usage to [iceprog](https://github.com/YosysHQ/icestorm/tree/master/iceprog). The programmer script is [scripts/picoprog.py](scripts/picoprog.py).

```
./picoprog.py bitstream.bit
```
 
