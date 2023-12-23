Hi!

This module can be used for P4 Executive watches. It has the micro controller on the back side.
So, double check the orientation of the chip as the PCB is shown from the top
layer in the pictures but you have to flip around the PCB, when soldering the chip.

Be aware, that you will need to **modify the plastic carrier** for the **P4 version**.
You will be in need to make the open space wider to be able to place the PCB flush on the
P4 plastic carrier. I usually do that by cutting the edges on all four sides of the open
space and create a 45 degree angle there. See the pictures.

In the header file of the main.h do not forget to set...

#define APP_WATCH_TYPE_BUILD    APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MARK_II_MOD

...when building for the P4 SiF module! You can also use the pre-compiled binary
called "pulsar_p4_sif_litronix_firmware.hex" for flashing the micro controller instead.

Kind regards

Roy
