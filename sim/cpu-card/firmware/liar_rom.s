; SPDX-FileCopyrightText: Donovan Worrell
; SPDX-License-Identifier: Apache-2.0
;
; Negative-control ROM: claims success without exercising the mailbox.
; The test bench must reject it, because the Northbridge audit log is empty.

.p816
.smart -

UART = $C000

.segment "RODATA"
msg_pass: .byte "ATARIX CPU CARD TEST: PASS", $0D, $0A, 0

.segment "CODE"
reset:
    sei
    clc
    xce
    rep #$10
    .i16
    sep #$20
    .a8
    ldx #$0000
@loop:
    lda msg_pass,x
    beq @done
    sta UART
    inx
    bra @loop
@done:
    stp

.segment "VECTORS"
    .res 28, $FF
    .word reset                     ; $FFFC RESET
    .word $FFFF
