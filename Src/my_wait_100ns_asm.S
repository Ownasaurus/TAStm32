.syntax unified
.globl my_wait_100ns_asm
.type  my_wait_100ns_asm, %function

my_wait_100ns_asm:
WAIT_100_NS: //168 MHz = about 17 cycles
    ADD     R1, R2, #0      // clock cycle 1
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0		// 10
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0		// 14

    SUBS    R0, R0, #1      // 15
    BGT     WAIT_100_NS       // takes probably 2 cycles: 16 and 17

    BX      LR      // return
