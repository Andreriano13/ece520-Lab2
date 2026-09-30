/*
 * ECE 520/L - Lab 2: AXI GPIO
 * Zybo Z7, bare metal on the ARM core inside the Zynq
 *
 * Reads the four slide switches and drives the four LEDs and the RGB LED,
 * including a binary counter and a ring counter.
 *
 * Written by Andres Riano.
 * Claude assisted with debugging code and guidance through using Vitis.
 */

#include "xparameters.h"   /* addresses of the blocks I built in Vivado */
#include "xgpio.h"         /* the AXI GPIO driver */
#include "sleep.h"         /* usleep */

XGpio led_gpio; /*the 4 bit LEDs*/
XGpio rgb_gpio; /*the 3 bit RGB LED*/
XGpio sw_gpio;  /*the 4 bit switches*/

int main(void)
{
    u32 switches;

    /* These two have to live out here, not inside the loop. If they were
     * declared inside, they'd be reset to their starting value every pass
     * and would never get anywhere. The ring starts at 0x1 because it needs
     * a lit bit to walk - shifting zero left just gives you zero. */
    u32 binary_count = 0x0;
    u32 ring_count   = 0x1;

    XGpio_Initialize(&led_gpio, XPAR_XGPIO_0_BASEADDR);
    XGpio_Initialize(&rgb_gpio, XPAR_XGPIO_1_BASEADDR);
    XGpio_Initialize(&sw_gpio,  XPAR_XGPIO_2_BASEADDR);

    /* One bit per pin, and it reads backwards from what you'd guess:
     * 0 means the chip drives the wire (output), 1 means it lets go so
     * something else can (input). You have to let go before you can read,
     * otherwise the chip and the switch are both fighting over one wire. */
    XGpio_SetDataDirection(&led_gpio, 1, 0x0);
    XGpio_SetDataDirection(&rgb_gpio, 1, 0x0);
    XGpio_SetDataDirection(&sw_gpio,  1, 0xF);

    while (1) {

        switches = XGpio_DiscreteRead(&sw_gpio, 1);

        /* The value isn't a count of switches - each switch is one bit,
         * so the whole row reads as a pattern. 0x3 is SW0 and SW1 together,
         * 0xC is SW2 and SW3 together. */
        switch (switches) {

        case 0x1:
            XGpio_DiscreteWrite(&led_gpio, 1, 0x1);
            XGpio_DiscreteWrite(&rgb_gpio, 1, 0x1);
            break;

        case 0x2:
            XGpio_DiscreteWrite(&led_gpio, 1, 0x2);
            XGpio_DiscreteWrite(&rgb_gpio, 1, 0x2);
            break;

        case 0x4:
            XGpio_DiscreteWrite(&led_gpio, 1, 0x4);
            XGpio_DiscreteWrite(&rgb_gpio, 1, 0x4);
            break;

        case 0x8:
            /* 0x8 is LED3. On the RGB, white is all three colours at once,
             * which is 0b111 - there is no white pin. 0x8 wouldn't do
             * anything here anyway, the block is only 3 bits wide. */
            XGpio_DiscreteWrite(&led_gpio, 1, 0x8);
            XGpio_DiscreteWrite(&rgb_gpio, 1, 0x7);
            break;

        case 0x3:
            /* Masking with 0xF keeps the bottom four bits, so when the count
             * hits 0x10 the fifth bit gets wiped and it lands back on 0x0.
             * The wrap is free - no comparison needed. */
            binary_count = (binary_count + 1) & 0xF;
            XGpio_DiscreteWrite(&led_gpio, 1, binary_count);
            XGpio_DiscreteWrite(&rgb_gpio, 1, 0x0);
            break;

        case 0xC:
            /* A ring counter isn't counting, it's one lit bit sliding left:
             * 1, 2, 4, 8 - it skips 3, 5, 6, 7 because only one LED is ever
             * on. After LED3 the bit shifts off the end and the mask leaves
             * nothing, so the if puts it back at the start. */
            ring_count = (ring_count << 1) & 0xF;
            if (ring_count == 0x0) {
                ring_count = 0x1;
            }
            XGpio_DiscreteWrite(&led_gpio, 1, ring_count);
            XGpio_DiscreteWrite(&rgb_gpio, 1, 0x0);
            break;

        default:
            /* Catches every combination not listed, including no switches
             * at all. Turning things off has to be an instruction - leave
             * this empty and the LEDs keep whatever the last case showed. */
            XGpio_DiscreteWrite(&led_gpio, 1, 0x0);
            XGpio_DiscreteWrite(&rgb_gpio, 1, 0x0);
            break;
        }

        /* Without this the loop runs millions of times a second and the
         * counters are just a blur. 200 ms is slow enough to watch and
         * still quick enough that flipping a switch feels instant. */
        usleep(200000);
    }

    return 0;   /* never reached - there's no OS to return to */
}
