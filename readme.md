# Readme

This project contains the software for the main controller (situated in the pilots controller) for the GC9 wheelchair of Autak (2025 version). This is a platformio project built for an esp32 microcontroller.

# Getting started

 To upload and run this on the controller, you need to adapt the TFT_eSPI library. The round watch-like screen uses a GC9A01 driver, that needs to be selected in the "User_Setup.h" file. Additionally, the display ports in said file may need to be re-assigned to the ones listed in "NanoESP_CAN.ino".