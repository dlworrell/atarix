; SPDX-FileCopyrightText: Donovan Worrell
; SPDX-License-Identifier: Apache-2.0
;
; ATARIX ROM monitor, milestone M0 (ATX-SPEC-022 EP-A, ADR-0016).
;
; Runs boot-sequence-v1 phase 4:
;   - enters native mode;
;   - sets the stack, direct page and data bank;
;   - prints the banner on the UART;
;   - sizes CPU-local RAM;
;   - echoes console input.
;
; The image is 64 KiB at $FF0000. Its last 8 KiB is also visible at
; $00E000-$00FFFF, and all code runs from that bank-0 window, so every
; address here is a bank-0 address.

        .p816
        .smart  +

; ---------------------------------------------------------------- registers
UART_DATA       = $C000
UART_STATUS     = $C001
UART_TX_READY   = $02
UART_RX_READY   = $01
BOARD_REVISION  = $C304

; ---------------------------------------------------------- direct page
dp_bank         = $00           ; RAM sizing: bank under test
dp_save         = $01           ; RAM sizing: saved byte
dp_num          = $02           ; print_dec16: value (2 bytes)
dp_started      = $04           ; print_dec16: leading-zero flag (2 bytes)

FIRST_RAM_BANK  = $01
END_RAM_BANK    = $F0           ; CPU-local RAM ends at $EFFFFF
BANK0_RAM_KIB   = 48            ; $000000-$00BFFF

        .segment "CODE"

reset:
        sei
        cld
        clc
        xce                     ; native mode
        rep     #$30            ; 16-bit A, X, Y
        ldx     #$01FF
        txs
        lda     #$0000
        tcd                     ; D = $0000
        sep     #$20            ; 8-bit A, 16-bit X/Y
        lda     #$00
        pha
        plb                     ; DB = $00

        ldx     #msg_banner
        jsr     puts
        ldx     #msg_version
        jsr     puts
        ldx     #msg_reset
        jsr     puts
        ldx     #msg_mode
        jsr     puts

        ldx     #msg_board
        jsr     puts
        lda     BOARD_REVISION
        jsr     print_hex8
        ldx     #msg_crlf
        jsr     puts

        jsr     size_ram        ; Y = number of 64 KiB banks above bank 0
        ldx     #msg_ram
        jsr     puts
        rep     #$20
        tya
        asl
        asl
        asl
        asl
        asl
        asl                     ; banks * 64
        clc
        adc     #BANK0_RAM_KIB
        jsr     print_dec16     ; leaves A 8-bit
        .a8
        ldx     #msg_kib
        jsr     puts

        ldx     #msg_ready
        jsr     puts

; Echo console input. CR is echoed as CR LF.
echo:
        .a8
        jsr     getc
        cmp     #$0D
        bne     @plain
        jsr     putc
        lda     #$0A
@plain:
        jsr     putc
        bra     echo

; ------------------------------------------------------------------ console
; putc: A (8-bit) to the UART. Preserves A.
putc:
        .a8
        pha
@wait:
        lda     UART_STATUS
        and     #UART_TX_READY
        beq     @wait
        pla
        sta     UART_DATA
        rts

; getc: waits for a byte; returns it in A (8-bit).
getc:
        .a8
        lda     UART_STATUS
        and     #UART_RX_READY
        beq     getc
        lda     UART_DATA
        rts

; puts: prints the NUL-terminated bank-0 string at X. A is 8-bit.
puts:
        .a8
        lda     a:0,x
        beq     @done
        jsr     putc
        inx
        bra     puts
@done:
        rts

; print_hex8: prints A (8-bit) as two hex digits.
print_hex8:
        .a8
        pha
        lsr
        lsr
        lsr
        lsr
        jsr     @nibble
        pla
        and     #$0F
@nibble:
        cmp     #10
        bcc     @digit
        adc     #6              ; carry is set: adds 7, 'A' - '0' - 10
@digit:
        adc     #'0'            ; carry is clear here
        jmp     putc

; print_dec16: prints A (16-bit) in decimal with no leading zeros.
; Returns with A 8-bit.
print_dec16:
        .a16
        sta     z:dp_num
        stz     z:dp_started
        ldx     #0
@place:
        ldy     #0
@subtract:
        lda     z:dp_num
        cmp     pow10,x
        bcc     @emit
        sbc     pow10,x         ; carry is set by cmp
        sta     z:dp_num
        iny
        bra     @subtract
@emit:
        tya
        ora     z:dp_started
        bne     @print
        cpx     #8              ; always print the units digit
        bne     @next
@print:
        lda     #1
        sta     z:dp_started
        tya
        clc
        adc     #'0'
        sep     #$20
        jsr     putc
        rep     #$20
@next:
        inx
        inx
        cpx     #10
        bne     @place
        sep     #$20
        rts

; ---------------------------------------------------------------- RAM size
; size_ram: counts consecutive banks from $01 that hold both $A5 and $5A at
; offset $0000. The byte under test is restored. Unmapped space ignores
; writes and reads $FF (ATX-SPEC-022), so the count stops at the first hole.
; Returns the count in Y (16-bit). A is 8-bit on entry and exit.
size_ram:
        .a8
        ldy     #0
        lda     #FIRST_RAM_BANK
        sta     z:dp_bank
@bank:
        lda     z:dp_bank
        cmp     #END_RAM_BANK
        bcs     @done
        pha
        plb                     ; DB = bank under test
        lda     a:$0000
        sta     z:dp_save       ; direct page is always bank 0
        lda     #$A5
        sta     a:$0000
        cmp     a:$0000
        bne     @restore
        lda     #$5A
        sta     a:$0000
        cmp     a:$0000
        bne     @restore
        lda     z:dp_save
        sta     a:$0000
        iny
        inc     z:dp_bank
        bra     @bank
@restore:
        lda     z:dp_save
        sta     a:$0000
@done:
        lda     #$00
        pha
        plb                     ; DB = $00
        rts

; --------------------------------------------------------------- interrupts
; M0 enables no interrupt source. Any interrupt that does arrive returns.
ignore_interrupt:
        rti

        .segment "RODATA"

pow10:          .word   10000, 1000, 100, 10, 1

msg_banner:     .byte   $0D, $0A, "ATARIX ROM Monitor", $0D, $0A, 0
msg_version:    .byte   "ROM Version: 0.1.0-m0", $0D, $0A, 0
msg_reset:      .byte   "Reset Reason: unknown", $0D, $0A, 0
msg_mode:       .byte   "CPU Mode: native 65C816", $0D, $0A, 0
msg_board:      .byte   "Board Revision: $", 0
msg_ram:        .byte   "Detected RAM: ", 0
msg_kib:        .byte   " KiB", $0D, $0A, 0
msg_ready:      .byte   "Ready.", $0D, $0A, 0
msg_crlf:       .byte   $0D, $0A, 0

        .segment "VECTORS"

; $FFE0: native-mode vectors
        .word   0, 0
        .word   ignore_interrupt        ; $FFE4 COP
        .word   ignore_interrupt        ; $FFE6 BRK
        .word   ignore_interrupt        ; $FFE8 ABORT
        .word   ignore_interrupt        ; $FFEA NMI
        .word   0                       ; $FFEC reserved
        .word   ignore_interrupt        ; $FFEE IRQ
; $FFF0: emulation-mode vectors
        .word   0, 0
        .word   ignore_interrupt        ; $FFF4 COP
        .word   0                       ; $FFF6 reserved
        .word   ignore_interrupt        ; $FFF8 ABORT
        .word   ignore_interrupt        ; $FFFA NMI
        .word   reset                   ; $FFFC RESET
        .word   ignore_interrupt        ; $FFFE IRQ/BRK
