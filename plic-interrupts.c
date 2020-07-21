/* Copyright 2019 SiFive, Inc */

/* SPDX-License-Identifier: Apache-2.0 */

#include <metal/button.h>
#include <metal/cpu.h>
#include <metal/drivers/riscv_plic0.h>
#include <metal/interrupt.h>
#include <metal/led.h>
#include <metal/machine/platform.h>
#include <metal/switch.h>
#include <stdbool.h>
#include <stdio.h>

#define RTC_FREQ	32768

#ifndef metal_led_ld0red
#define metal_led_ld0red metal_led_none
#endif

#ifndef metal_led_ld0green
#define metal_led_ld0green metal_led_none
#endif

#ifndef metal_led_ld0blue
#define metal_led_ld0blue metal_led_none
#endif

#define led0_red metal_led_ld0red
#define led0_green metal_led_ld0green
#define led0_blue metal_led_ld0blue

#ifndef metal_switch_sw1
#define metal_switch_sw1 metal_switch_none
#endif

#ifndef metal_switch_sw2
#define metal_switch_sw2 metal_switch_none
#endif

#define swch1 metal_switch_sw1
#define swch2 metal_switch_sw2

void display_instruction (void) {
    printf("\n");
    printf("SIFIVE, INC.\n!!\n");
    printf("\n");
    printf("E76/S76 Coreplex IP Eval Kit 'plic-interrupts' Example.\n\n");
    printf("Switch 1 and 2 are enabled as external global interrupt sources.\n");
    printf("Toggling switches 1 and 2 will activate interrupt handlers.\n");
    printf("\n");
}

void metal_riscv_cpu_intc_mtip_handler(void) {
    static bool state = false;
    if (state) {
        printf("#### Giving Switch 2 Priority for 10 seconds ####\n");
        printf("#### Even if Switch 1 is on, Switch 2 has higher priority right now ####\n");
        metal_switch_set_interrupt_priority(swch1, 2);
        metal_switch_set_interrupt_priority(swch2, 3);
    } else {
        printf("**** Giving Switch 1 Priority for 10 seconds ****\n");
        printf("#### Even if Switch 2 is on, Switch 1 has higher priority right now ####\n");
        metal_switch_set_interrupt_priority(swch1, 3);
        metal_switch_set_interrupt_priority(swch2, 2);
    }    

    state = !state;

    struct metal_cpu cpu = metal_cpu_get(metal_cpu_get_current_hartid());
    metal_cpu_set_mtimecmp(cpu, metal_cpu_get_mtime(cpu) + 10*RTC_FREQ);
}

void metal_switch_sw1_interrupt_handler(void) {
    metal_switch_clear_interrupt(swch1);
    printf("Switch 1 is on!\n");
    metal_led_on(led0_green);
    metal_led_off(led0_red);
}

void metal_switch_sw2_interrupt_handler(void) {
    metal_switch_clear_interrupt(swch2);
    printf("Switch 2 is on!\n");
    metal_led_off(led0_green);
    metal_led_on(led0_red);
}

int main (void)
{
    metal_led_enable(led0_red);
    metal_led_enable(led0_green);
    metal_led_enable(led0_blue);
    metal_led_on(led0_red);
    metal_led_off(led0_green);
    metal_led_off(led0_blue);
 
    // Check we this target has a plic. If not gracefull exit
#ifndef METAL_RISCV_PLIC0
    printf("Exit. This example need a plic interrupt controller for SW1 and SW2.\n");
    return 0;
#endif
    /* Get a handle for the PLIC */
    struct metal_interrupt plic = (struct metal_interrupt) { 0 };

    /* Set the PLIC threshold to 1 */
    riscv_plic0_set_threshold(plic, 1);

    if (metal_switch_enable_interrupt(swch1) == -1) {
        printf("SW1 interrupt enable failed\n");
        return 5;
    }
    if (metal_switch_enable_interrupt(swch2) == -1) {
        printf("SW2 interrupt enable failed\n");
        return 5;
    }

    display_instruction();

    struct metal_cpu cpu = metal_cpu_get(metal_cpu_get_current_hartid());
    metal_cpu_set_mtimecmp(cpu, metal_cpu_get_mtime(cpu));
    metal_cpu_enable_timer_interrupt();
    metal_cpu_enable_interrupts();

    while (1) {
        __asm__ volatile ("wfi");
    }

    return 0;
}
