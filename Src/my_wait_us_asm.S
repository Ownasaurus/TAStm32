.syntax unified
.globl my_wait_us_asm
.type  my_wait_us_asm, %function

my_wait_us_asm:
WAIT_1_US:
    ADD     R1, R2, #0      // clock cycle 1
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 10
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 20
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 30
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 40
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 50
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 60
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 70
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 80
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 90
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 100
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 110
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 120
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 130
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 140
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 150
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 160
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0		// 166

    SUBS    R0, R0, #1      // clock cycle 167
    BGT     WAIT_1_US       // clock cycle 168

    BX      LR      // return
