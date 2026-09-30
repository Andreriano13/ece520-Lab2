# ECE 520 Lab 2 — AXI GPIO

Andres Riano · Zybo Z7

The ARM processor on the Zybo reads the 4 switches and lights up the LEDs and the RGB LED through three AXI GPIO blocks.

- **SW0 / SW1 / SW2 / SW3 alone:** turns on that LED, and the RGB goes red / green / blue / white
- **SW0 + SW1:** the LEDs count in binary (0 to 15)
- **SW2 + SW3:** one LED walks left in a loop
- **Anything else:** everything off

The Vivado project is in the zip. The C code is `main.c` (built in Vitis 2023.2).

**AI use:** Claude assisted with debugging code and guidance through using Vitis.
