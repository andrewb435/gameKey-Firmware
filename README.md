# gameKey Firmware #

This is the firmware designed to run on the gameKey Main Control PCB. It's a standard Arduino sketch, and only relies on built in libraries plus Matthew Heironimus's Arduino joystick library for 32u4 based Arduino boards found at https://github.com/MHeironimus/ArduinoJoystickLibrary.

The 2.0 firmware brings support for layer shifting on-device, and works with the 2.0 companion firmware for configuration.

Mode switch and LED are NYI, but the Enable jumper on the Main Control PCB must be populated and shorted with a jumper to enable ANY output.

At this time a right hand gameKey is NYI.
