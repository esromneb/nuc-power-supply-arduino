# nuc-power-supply-arduino


# Schematic

NUC Fan      + goes through resistor to A0
Nuc Fan Ground goes to gnd


# DC Fan Control Reylay
Connected to NC so that if arduino is unplugged, default is fans ON
Yellow jumper goes in position farthest from relay module


# Notes
Arduino projects cannot have a hyphen.


# Reboot issues
when rebooting, the USB goes low and that is our first sign. The Arduino must now glitch the drive off and on with a delay.
This must be timed right so that the BIOS? catches the PCI cards and makes them avail to truenas. In the case of a poweroff signal, this USB goes low
and the ardiuno can't tell the difference. So what we do is the same glitch, and then wait for the USB (aka the cpu) to come back. if the USB stays low
then the drives shut off for good, until the next button press.

Note these were taken with printing on, so I will leave it on

Inorder to find this time, I added a debug button and so far:
50 bad

150 good, after the 50 was bad

150 and 250 are every other reboot

at 350/450/650 they became de-synched from eachother

same with 1050

I will try 150 again, incase it must be after a bad reboot to be good. In this case I could just ssh in and reboot twice which would be annoying, but not end of the world.
