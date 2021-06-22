# SCS Update for Linux
A simple Linux tool to update the firmware of the SCS modems.

## Prerequisites
On a Debian or Debian based system simply do
```
sudo apt update
sudo apt install build-essential libusb-1.0-0-dev
```

In general you need gcc, make and libusb_1.0.

## Get the source

```
git clone https://github.com/scsptc/scsupdate.git
```

## Build
Change directory to the scsupdate source
```
cd scsupdate
```
and simply enter
```
make
```

You may copy scsupdate to /usr/local/bin for system wide use
```
sudo cp scsupdate /usr/local/bin/
```

## Function

** The modem must be powered up and connected with the PC either via USB or a serial port! **

Start with
```
./scsupdate <firmware_file>
```
e.g. for updating a P4dragon DR-7800 or DR-7400
```
./scsupdate dragon_fw_2_40_00.dr7
```

scsupdate searches for SCS modems (max. 8) on USB.
If more than on modem is found SCS Update presents you a list of the modems.
```
More than one SCS modem found! Plaese choose:
1: /dev/ttyUSB0     P4dragon DR-7400
2: /dev/ttyUSB1     PTC-IIIusb
Enter a number: 
```
Enter a number from the list and press enter.

** The firmware must match the modem! **

If you don't want the automatic search, you can enter the device and baudrate as arguments:
```
./scsupdate /dev/ttyS0 38400 <firmware_file>
```
e.g. a PTC-IIpro connected to ttyS0
```
./scsupdate /dev/ttyS0 115200 profi41r.pro
```

**Hint:** if you get a *permission denied* error, you normally have to add the user to the group dialout!
```
sudo usermod -a -G dialout username
```
replace *username* with your user name.

**Log out and log in again to activate the changes!**


Tested on:
- Debian 10
- Ubuntu 20.04
- Raspberry Pi OS Lite May 7th 2021
