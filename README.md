
ONX
---

An OS for ESP32-P4 boards (no FreeRTOS) and Pico-like boards (RP2040 and RP2350).

An "OS" as in both "kernel" and "shell":

 - The Kernel part provides an API for data object state handling and update propagation

 - The Shell part is an HD/WXGA screen for exploring and updating those data objects

First application I'm working on is a simple photoframe, where the data objects are
photos and galleries or collections of them. Photoframes connect so you can see each
other's latest photos as they're added.

---------------------------
