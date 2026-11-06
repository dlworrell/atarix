; SPDX-FileCopyrightText: Donovan Worrell
; SPDX-License-Identifier: Apache-2.0
;
; ATARIX CPU card test ROM (W65C816, ca65).
;
; Boot path:   reset (emulation mode) -> native mode, 16-bit registers,
;              stack at $01FF, direct page at $0000.
; Self-checks: native mode entered, RAM in bank $00 and long RAM in banks
;              $01/$7F, ROM write protection, board identity readable.
; Binding:     finds every service it uses by service type through Discovery
;              Services ($00D500), the kernel's ring-filtered directory the
;              supervisor published before releasing reset. No service node
;              id is compiled in; only the deliberately unlisted probe nodes
;              (supervisor, fabric, an empty node) are.
; Mailbox:     builds Mailbox v1 frames (16-byte little-endian header +
;              payload), computes CRC32 (reflected 0xEDB88320) in firmware,
;              submits through the Northbridge Mailbox Engine at $00D200 and
;              checks the decision for each case, verifying every PONG field.
; Supervisor:  requests through the Supervisor Request Gateway ($00C200) and
;              checks each ATX-SPEC-016 result code.
; Result:      prints "ATARIX CPU CARD TEST: PASS" or "FAIL Tnn", then STP.
;
; The host test bench checks the Northbridge audit log independently; this
; firmware's PASS line is necessary but not sufficient.

.p816
.smart -

UART        = $C000
IDENT       = $C300
MB          = $D200
MB_STATUS   = MB + $00
MB_CONTROL  = MB + $01
MB_LEN      = MB + $02
MB_SRC      = MB + $03
MB_DST      = MB + $04
MB_TYPE     = MB + $05
MB_RESULT   = MB + $06
MB_DST_HI   = MB + $07
MB_SRC_HI   = MB + $08
MB_TYPE_HI  = MB + $09
TX          = $D280

; Interrupt controller (docs/interrupt-architecture-v1.md "preferred v1" layout)
INT         = $D100
INT_PEND_LO = INT + $00
INT_PEND_HI = INT + $01
INT_MASK_LO = INT + $02
INT_MASK_HI = INT + $03
INT_PRIO    = INT + $04
INT_ACK     = INT + $05
INT_ROUTE   = INT + $06
INT_STATUS  = INT + $07
SRC_SUPER   = 1
SRC_MAILBOX = 3

; Supervisor Request Gateway (docs/register-map-v1.md block, ATX-SPEC-016 results)
SUP         = $C200
SUP_CMD     = SUP + $00
SUP_STATUS  = SUP + $01
SUP_ARG     = SUP + $02
SUP_RESULT  = SUP + $06
SUP_RESET   = $01
SUP_RECOVER = $02
SUP_READ_ID = $03
SUP_FAULTLOG = $04
SUP_FPGA    = $05
SUP_REPORT  = $06
R_EXECUTED  = 0
R_DENIED    = 1
R_APPROVAL  = 3
R_UNSUPP    = 5
R_UNKNOWN   = 6

; Discovery Services: ring-filtered service directory (simulator profile)
DISC        = $D500
DISC_COUNT  = DISC + $00
DISC_SELECT = DISC + $01
DISC_CAP    = DISC + $06
DISC_RING   = DISC + $07
DISC_SVC    = DISC + $08
DISC_NODE   = DISC + $0A
DISC_STATE  = DISC + $0C
SVC_VALIDATION = $0004
SVC_BLOCK   = $4000
SVC_GPU     = $6000
EP_ACTIVE   = 1                     ; atarix_mailbox_endpoint_state_t
EP_SUSPENDED = 2
EP_QUARANTINED = 6
DIR_COUNT   = 5                     ; cards in the simulated chassis
RX          = $D2C0
HDR         = 16

ST_RX_READY = $02
CTL_SUBMIT  = $01
CTL_ACK     = $02

; Mailbox v1 values (include/atarix/mailbox.h)
MAGIC_LO    = $4D41                 ; 0x58424D41, little-endian
MAGIC_HI    = $5842
T_PING      = $0001
T_PONG      = $0002
T_WRITE_REG = $0008
T_UNKNOWN   = $0099
T_GPU_REQ   = $0040                 ; GPU_TRANSFORM_REQUEST (SERVICE_GPU_COMPUTE)
T_GPU_REP   = $0041                 ; GPU_TRANSFORM_REPLY
S_OK        = 0
S_CRC       = 4
S_SEQ       = 5
S_DUP       = 6
S_UNKNOWN   = 8
S_RING      = 9
S_POLICY    = 10
S_CAP       = 11
S_QUAR      = 12
S_BUSY      = 13
S_UNAVAIL   = 15

; Probe nodes: deliberately addressed although the directory does not list
; them, to show that an unlisted node is still checked, not merely hidden.
N_SUPER     = $0000                 ; supervisor (ring -2)
N_FABRIC    = $0001                 ; fabric control (ring -1)
N_NONE      = $0404                 ; no endpoint

; Direct-page variables
crc_lo      = $00
crc_hi      = $02
ptr         = $04
len         = $06
dstv        = $08
typv        = $0A
seqv        = $0C
corruptv    = $0E
tmp         = $10
test_no     = $12
irq_count   = $14                   ; interrupts taken (16-bit)
expp        = $16                   ; pointer to the expected GPU reply
want_type   = $18                   ; find_service in: service type
want_index  = $1A                   ;                  instance (nth match)
found_node  = $1C                   ; find_service out: node id
found_state = $1E                   ;                   lifecycle state (8-bit)
found_cap   = $1F                   ;                   capability id (8-bit)
disc_n      = $20                   ; 8-bit work: entry count, index, matches
disc_i      = $21
match_n     = $22
n_echo      = $24                   ; nodes bound through Discovery Services
n_echo2     = $26
n_gpu       = $28
n_gpu2      = $2A
n_store     = $2C
sres        = $2E                   ; gateway result (8-bit) and status (8-bit)
irq_log     = $0300                 ; dispatch level of each interrupt taken
sup_args    = $0310                 ; supervisor ARG0-3 captured by the level-0 handler

; Long conditional branches (8-bit relative branches cannot reach far labels).
.macro JNE target
    beq :+
    jmp target
:
.endmacro
.macro JEQ target
    bne :+
    jmp target
:
.endmacro
.macro JCC target
    bcs :+
    jmp target
:
.endmacro
.macro JCS target
    bcc :+
    jmp target
:
.endmacro

.segment "RODATA"
banner:     .byte "ATARIX CPU CARD TEST ROM v1", $0D, $0A, 0
msg_native: .byte "native mode OK", $0D, $0A, 0
msg_ram:    .byte "RAM OK", $0D, $0A, 0
msg_rom:    .byte "ROM write-protect OK", $0D, $0A, 0
msg_board:  .byte "board ", 0
msg_slash:  .byte "/", 0
msg_crlf:   .byte $0D, $0A, 0
msg_t:      .byte "T", 0
msg_ok:     .byte " OK", $0D, $0A, 0
msg_pass:   .byte "ATARIX CPU CARD TEST: PASS", $0D, $0A, 0
msg_fail:   .byte "FAIL T", 0
msg_irq:    .byte "FAIL unexpected interrupt", $0D, $0A, 0
hexdigits:  .byte "0123456789ABCDEF"
payload:    .byte "ATARIX"
PAYLOAD_LEN = 6
.include "gpu_vectors.inc"          ; generated: requests and independently computed results

.segment "CODE"

; ---------------------------------------------------------------------------
; Reset and self-checks
; ---------------------------------------------------------------------------
reset:
    sei
    clc
    xce                     ; enter native mode
    rep #$30
    .a16
    .i16
    ldx #$01FF
    txs
    lda #$0000
    tcd
    sta test_no

    clc
    xce                     ; carry <- previous E; must be 0 (native)
    bcc @native
    jmp fail
@native:
    ldx #banner
    jsr puts
    ldx #msg_native
    jsr puts

    ; RAM: bank $00, then long addressing into banks $01 and $7F.
    lda #$0001
    sta test_no
    lda #$A55A
    sta $0200
    lda #$0000
    lda $0200
    cmp #$A55A
    beq :+
    jmp fail
:   lda #$5AA5
    sta f:$010000
    lda #$C33C
    sta f:$7FFFFE
    lda f:$010000
    cmp #$5AA5
    beq :+
    jmp fail
:   lda f:$7FFFFE
    cmp #$C33C
    beq :+
    jmp fail
:   ldx #msg_ram
    jsr puts

    ; ROM write protection: the write must be ignored (and faulted by the card).
    lda #$0002
    sta test_no
    sep #$20
    .a8
    lda $E000
    sta tmp
    eor #$FF
    sta $E000
    lda $E000
    cmp tmp
    rep #$20
    .a16
    beq :+
    jmp fail
:   ldx #msg_rom
    jsr puts

    ; Board identity (read-only block at $00C300).
    ldx #msg_board
    jsr puts
    lda IDENT
    jsr print_hex16
    ldx #msg_slash
    jsr puts
    lda IDENT + 2
    jsr print_hex16
    ldx #msg_crlf
    jsr puts

    ; I1: interrupt controller boot defaults (all masked, nothing pending).
    lda #$0021
    sta test_no
    sep #$20
    .a8
    lda INT_PEND_LO
    ora INT_PEND_HI
    JNE fail8
    lda INT_MASK_LO
    and INT_MASK_HI
    cmp #$FF
    JNE fail8
    lda INT_STATUS
    and #$01
    JNE fail8
    lda INT_PRIO
    cmp #$FF
    JNE fail8
    lda #$01
    sta INT_ROUTE                   ; deliver to the primary CPU
    rep #$20
    .a16
    jsr report_ok

; ---------------------------------------------------------------------------
; T20: bind services by type through Discovery Services. The directory must
; hold exactly the five chassis services, none in a platform ring, with the
; lifecycle state and capability the supervisor's boot chain produced.
; ---------------------------------------------------------------------------
.macro BIND type, index, var, state, hascap
    lda #type
    sta want_type
    lda #index
    sta want_index
    jsr find_service
    JCS fail
    lda found_node
    sta var
    sep #$20
    .a8
    lda found_state
    cmp #state
    JNE fail8
    lda found_cap
.if hascap
    JEQ fail8
.else
    JNE fail8
.endif
    rep #$20
    .a16
.endmacro

    lda #$0020
    sta test_no
    sep #$20
    .a8
    lda DISC_COUNT
    cmp #DIR_COUNT
    JNE fail8
    ldx #$0000
@ring:
    txa
    sta DISC_SELECT
    lda DISC_RING                   ; wire value: $FE/$FF would be a platform ring
    cmp #$04
    JCS fail8
    inx
    cpx #DIR_COUNT
    bne @ring
    rep #$20
    .a16
    BIND SVC_VALIDATION, 0, n_echo,  EP_ACTIVE,      1
    BIND SVC_VALIDATION, 1, n_echo2, EP_SUSPENDED,   1
    BIND SVC_GPU,        0, n_gpu,   EP_ACTIVE,      1
    BIND SVC_GPU,        1, n_gpu2,  EP_ACTIVE,      0   ; homebrew: no capability
    BIND SVC_BLOCK,      0, n_store, EP_QUARANTINED, 0   ; identity failed at boot
    jsr report_ok

; ---------------------------------------------------------------------------
; Mailbox cases. Each: destination, type, sequence, corrupt-CRC flag, expected
; Northbridge status. Order and values must match the host's expected audit.
; ---------------------------------------------------------------------------
.macro MBCASE num, dst, typ, seq, corrupt, expect
    lda #num
    sta test_no
    lda dst
    sta dstv
    lda #typ
    sta typv
    lda #seq
    sta seqv
    lda #corrupt
    sta corruptv
    lda #payload
    sta ptr
    lda #PAYLOAD_LEN
    sta len
    jsr send
    cmp #expect
    beq :+
    jmp fail
:
.endmacro

.macro SUPCASE num, cmd, expect
    lda #num
    sta test_no
    lda #cmd
    jsr sup_request
    cmp #expect
    JNE fail
    jsr report_ok
.endmacro

    MBCASE $11, n_echo,      T_PING,      1, 0, S_OK
    jsr check_pong
    jsr report_ok
    MBCASE $12, {#N_SUPER},  T_PING,      2, 0, S_RING
    jsr check_no_reply
    jsr report_ok
    MBCASE $13, {#N_FABRIC}, T_PING,      3, 0, S_RING
    jsr check_no_reply
    jsr report_ok
    MBCASE $14, n_echo,      T_WRITE_REG, 4, 0, S_CAP
    jsr check_no_reply
    jsr report_ok
    MBCASE $15, n_echo,      T_PING,      5, 1, S_CRC
    jsr check_no_reply
    jsr report_ok
    MBCASE $16, n_echo,      T_PING,      5, 0, S_OK
    jsr check_pong
    jsr report_ok
    MBCASE $17, n_echo,      T_PING,      5, 0, S_DUP
    jsr check_no_reply
    jsr report_ok
    MBCASE $18, {#N_NONE},   T_PING,      6, 0, S_POLICY
    jsr check_no_reply
    jsr report_ok
    MBCASE $19, n_echo,      T_PING,      8, 0, S_SEQ
    jsr check_no_reply
    jsr report_ok
    MBCASE $1A, n_echo,      T_UNKNOWN,   7, 0, S_UNKNOWN
    jsr check_no_reply
    jsr report_ok

; ---------------------------------------------------------------------------
; Interrupt tests: Vega816 Vector Pull Rewrite through the fabric controller.
; ---------------------------------------------------------------------------
    ; I2: a mailbox reply while masked: pending, but no IRQ asserted.
    lda #$0000
    sta irq_count
    lda #$0022
    sta test_no
    sep #$20
    .a8
    lda #SRC_MAILBOX
    sta INT_ACK                     ; clear the latch left by T11/T16 (RX already acknowledged)
    lda INT_PEND_LO
    JNE fail8
    rep #$20
    .a16
    MBCASE $22, n_echo, T_PING, 7, 0, S_OK
    sep #$20
    .a8
    lda MB_STATUS
    and #ST_RX_READY
    JEQ fail8
    lda INT_PEND_LO
    cmp #(1 << SRC_MAILBOX)
    JNE fail8
    lda INT_PRIO
    cmp #$FF
    JNE fail8
    lda INT_STATUS
    and #$01
    JNE fail8
    rep #$20
    .a16
    jsr report_ok

    ; I3: supervisor completes Read Board ID and raises its source.
    lda #$0023
    sta test_no
    sep #$20
    .a8
    lda #SUP_READ_ID
    sta SUP_CMD
    lda INT_PEND_LO
    cmp #((1 << SRC_SUPER) | (1 << SRC_MAILBOX))
    JNE fail8
    rep #$20
    .a16
    jsr report_ok

    ; I4: unmask both. Supervisor (level 0, $FFEE) must be taken before the
    ; mailbox (level 2, rewritten to $FFF2), each in its own handler.
    lda #$0024
    sta test_no
    sep #$20
    .a8
    lda #<~((1 << SRC_SUPER) | (1 << SRC_MAILBOX))
    sta INT_MASK_LO
    lda INT_PRIO
    JNE fail8                       ; level 0 selected
    lda INT_STATUS
    and #$01
    JEQ fail8
    rep #$30
    .a16
    .i16
    ldx #$2000
    cli
@wait:
    lda irq_count
    cmp #$0002
    beq @taken
    dex
    bne @wait
    sei
    jmp fail
@taken:
    sei
    sep #$20
    .a8
    lda irq_log
    JNE fail8                       ; first: level 0 (supervisor)
    lda irq_log + 1
    cmp #$02
    JNE fail8                       ; second: level 2 (mailbox)
    lda sup_args
    cmp #$A1
    JNE fail8
    lda sup_args + 1
    cmp #$A7
    JNE fail8
    lda sup_args + 2
    cmp #$16
    JNE fail8
    lda sup_args + 3
    cmp #$08
    JNE fail8
    lda INT_PEND_LO
    JNE fail8
    lda MB_STATUS
    and #ST_RX_READY
    JNE fail8
    rep #$20
    .a16
    jsr report_ok

    ; I5: mask everything again; IRQ must deassert.
    lda #$0025
    sta test_no
    sep #$20
    .a8
    lda #$FF
    sta INT_MASK_LO
    lda INT_STATUS
    and #$01
    JNE fail8
    rep #$20
    .a16
    jsr report_ok

    ; I6: a reset request is a request, not a command: the gateway answers
    ; REQUIRES_APPROVAL and nothing is reset.
    SUPCASE $26, SUP_RESET, R_APPROVAL

; ---------------------------------------------------------------------------
; GPU compute service (cglm-backed display card), 16.16 fixed point.
; ---------------------------------------------------------------------------
.macro GPUCASE num, dst, seq, req, reqlen, expect_status, exp
    lda #num
    sta test_no
    lda dst
    sta dstv
    lda #T_GPU_REQ
    sta typv
    lda #seq
    sta seqv
    lda #0
    sta corruptv
    lda #req
    sta ptr
    lda #reqlen
    sta len
    jsr send
    cmp #expect_status
    JNE fail
    lda #exp
    sta expp
.endmacro

    GPUCASE $31, n_gpu, 8, gpu_req_g1, GPU_REQ_LEN, S_OK, gpu_exp_g1
    jsr check_gpu_reply                 ; rotate 90 deg about Z, translate -> (2,4,4)
    jsr report_ok
    GPUCASE $32, n_gpu, 9, gpu_req_g2, GPU_REQ_LEN, S_OK, gpu_exp_g2
    jsr check_gpu_reply                 ; general rotation, unnormalized axis
    jsr report_ok
    GPUCASE $33, n_gpu, 10, gpu_req_badop, GPU_REQ_LEN, S_OK, gpu_exp_badop
    jsr check_gpu_reply                 ; service rejects operation 7 (status 2)
    jsr report_ok
    GPUCASE $34, n_gpu, 11, gpu_req_g1, GPU_REQ_SHORT_LEN, S_OK, gpu_exp_badlen
    jsr check_gpu_reply                 ; service rejects a 40-byte request (status 1)
    jsr report_ok
    GPUCASE $35, n_gpu2, 12, gpu_req_g1, GPU_REQ_LEN, S_CAP, gpu_exp_g1
    jsr check_no_reply                  ; homebrew GPU: policy withheld the capability
    jsr report_ok

; ---------------------------------------------------------------------------
; ATX-SPEC-005 validation after authority: lifecycle state, then resources.
; ---------------------------------------------------------------------------
    MBCASE $36, n_store, T_PING, 13, 0, S_CAP
    jsr check_no_reply                  ; listed, but quarantined at boot: no capability
    jsr report_ok
    MBCASE $37, n_echo2, T_PING, 14, 0, S_UNAVAIL
    jsr check_no_reply                  ; capability held, endpoint SUSPENDED
    jsr report_ok
    MBCASE $38, n_echo,  T_PING, 15, 0, S_OK
    MBCASE $38, n_echo,  T_PING, 16, 0, S_BUSY   ; reply to 15 not yet acknowledged
    lda #15
    sta seqv
    jsr check_pong                      ; the slot still holds the reply to 15
    jsr report_ok

; ---------------------------------------------------------------------------
; Supervisor Request Gateway: each command gets its ATX-SPEC-016 result.
; ---------------------------------------------------------------------------
    SUPCASE $41, SUP_RECOVER,  R_DENIED
    SUPCASE $42, SUP_FAULTLOG, R_UNSUPP
    SUPCASE $43, SUP_FPGA,     R_DENIED
    SUPCASE $44, $7E,          R_UNKNOWN
    lda #N_SUPER                        ; a kernel may not report the supervisor
    jsr sup_set_arg
    SUPCASE $45, SUP_REPORT,   R_DENIED
    lda n_gpu                           ; report the GPU as faulty: quarantined
    jsr sup_set_arg
    SUPCASE $46, SUP_REPORT,   R_EXECUTED

    ; T47: the directory now shows the GPU quarantined, and the mailbox
    ; refuses it although the kernel still holds its capability.
    lda #$0047
    sta test_no
    lda #SVC_GPU
    sta want_type
    lda #0
    sta want_index
    jsr find_service
    JCS fail
    sep #$20
    .a8
    lda found_state
    cmp #EP_QUARANTINED
    JNE fail8
    rep #$20
    .a16
    GPUCASE $47, n_gpu, 17, gpu_req_g1, GPU_REQ_LEN, S_QUAR, gpu_exp_g1
    jsr check_no_reply
    jsr report_ok

    ldx #msg_pass
    jsr puts
    stp

; ---------------------------------------------------------------------------
; check_gpu_reply: the reply must be a GPU_TRANSFORM_REPLY from dstv with the
; request's sequence, a CRC this firmware recomputes, the expected status, and
; x, y, z within +/-2 LSB (16.16) of the expected values at (expp).
; ---------------------------------------------------------------------------
check_gpu_reply:
    .a16
    .i16
    sep #$20
    .a8
    lda MB_STATUS
    and #ST_RX_READY
    JEQ gpu_bad
    lda MB_TYPE
    cmp #<T_GPU_REP
    JNE gpu_bad
    lda MB_TYPE_HI
    cmp #>T_GPU_REP
    JNE gpu_bad
    lda MB_SRC
    cmp dstv
    JNE gpu_bad
    lda MB_SRC_HI
    cmp dstv + 1
    JNE gpu_bad
    lda MB_LEN
    cmp #16
    JNE gpu_bad
    rep #$20
    .a16
    lda RX + 0
    cmp #MAGIC_LO
    JNE gpu_bad
    lda RX + 4
    cmp #T_GPU_REP
    JNE gpu_bad
    lda RX + 6
    cmp #16
    JNE gpu_bad
    lda RX + 8
    cmp seqv
    JNE gpu_bad
    lda #RX + HDR
    sta ptr
    lda #16
    sta len
    jsr crc32
    lda RX + 12
    cmp crc_lo
    JNE gpu_bad
    lda RX + 14
    cmp crc_hi
    JNE gpu_bad

    ; status: exact
    ldy #0
    lda RX + HDR
    cmp (expp),y
    JNE gpu_bad
    ldy #2
    lda RX + HDR + 2
    cmp (expp),y
    JNE gpu_bad

    ; x, y, z: |got - expected| <= 2 (32-bit, two's complement)
    ldy #4
@coord:
    lda RX + HDR,y
    sec
    sbc (expp),y
    sta tmp
    iny
    iny
    lda RX + HDR,y
    sbc (expp),y
    beq @hi_zero
    cmp #$FFFF
    JNE gpu_bad
    lda tmp
    cmp #$FFFE
    JCC gpu_bad
    bra @next
@hi_zero:
    lda tmp
    cmp #3
    JCS gpu_bad
@next:
    iny
    iny
    cpy #16
    bne @coord

    sep #$20
    .a8
    lda #CTL_ACK
    sta MB_CONTROL
    rep #$20
    .a16
    rts
gpu_bad:
    rep #$30
    .a16
    .i16
    jmp fail

fail8:                              ; failure path reachable from 8-bit code
    rep #$30
    .a16
    .i16
    jmp fail

; ---------------------------------------------------------------------------
; Native IRQ handlers, one per Vector Pull Rewrite level.
; ---------------------------------------------------------------------------
irq_level0:                         ; supervisor
    rep #$30
    .a16
    .i16
    pha
    phx
    sep #$20
    .a8
    lda SUP_STATUS                  ; consumes the completion, deasserts the source
    and #$82
    cmp #$02                        ; done, and executed
    JNE fail8
    lda SUP_RESULT
    cmp #R_EXECUTED
    JNE fail8
    lda SUP_ARG
    sta sup_args
    lda SUP_ARG + 1
    sta sup_args + 1
    lda SUP_ARG + 2
    sta sup_args + 2
    lda SUP_ARG + 3
    sta sup_args + 3
    lda #SRC_SUPER
    sta INT_ACK
    lda #$00
    jsr log_irq
    rep #$30
    .a16
    .i16
    plx
    pla
    rti

irq_level2:                         ; mailbox
    rep #$30
    .a16
    .i16
    pha
    phx
    sep #$20
    .a8
    lda MB_STATUS
    and #ST_RX_READY
    JEQ fail8
    lda #CTL_ACK
    sta MB_CONTROL                  ; consume the reply: source deasserts
    lda #SRC_MAILBOX
    sta INT_ACK
    lda #$02
    jsr log_irq
    rep #$30
    .a16
    .i16
    plx
    pla
    rti

log_irq:                            ; A (8-bit) = level; X 16-bit free
    .a8
    .i16
    ldx irq_count
    sta irq_log,x
    inx
    stx irq_count
    rts

; ---------------------------------------------------------------------------
; find_service: nth entry (want_index) of service type want_type in the
; Discovery Services directory. Out: carry clear and found_node,
; found_state, found_cap; carry set if there is no such entry.
; ---------------------------------------------------------------------------
find_service:
    .a16
    .i16
    sep #$20
    .a8
    lda DISC_COUNT
    sta disc_n
    stz disc_i
    stz match_n
@loop:
    lda disc_i
    cmp disc_n
    beq @none
    sta DISC_SELECT
    lda DISC_SVC
    cmp want_type
    bne @next
    lda DISC_SVC + 1
    cmp want_type + 1
    bne @next
    lda match_n
    cmp want_index
    beq @hit
    inc match_n
@next:
    inc disc_i
    bra @loop
@hit:
    lda DISC_NODE
    sta found_node
    lda DISC_NODE + 1
    sta found_node + 1
    lda DISC_STATE
    sta found_state
    lda DISC_CAP
    sta found_cap
    rep #$20
    .a16
    clc
    rts
@none:
    rep #$20
    .a16
    sec
    rts

; ---------------------------------------------------------------------------
; sup_set_arg: A (16-bit) -> gateway ARG0 (low), ARG1 (high).
; ---------------------------------------------------------------------------
sup_set_arg:
    .a16
    .i16
    sep #$20
    .a8
    sta SUP_ARG
    xba
    sta SUP_ARG + 1
    rep #$20
    .a16
    rts

; ---------------------------------------------------------------------------
; sup_request: submit gateway command A (low byte). Checks that the request
; completed (STATUS done; reading it deasserts the source), that STATUS bit 7
; ("not executed") agrees with RESULT, and acknowledges the interrupt.
; Out: A = RESULT (16-bit, high 0).
; ---------------------------------------------------------------------------
sup_request:
    .a16
    .i16
    sep #$20
    .a8
    sta SUP_CMD
    lda SUP_STATUS
    sta sres + 1
    and #$02
    JEQ fail8
    lda #SRC_SUPER
    sta INT_ACK
    lda INT_PEND_LO
    and #(1 << SRC_SUPER)
    JNE fail8                       ; completion consumed: no longer pending
    lda SUP_RESULT
    sta sres
    beq @executed
    lda sres + 1
    and #$80
    JEQ fail8                       ; not executed, so bit 7 must be set
    bra @done
@executed:
    lda sres + 1
    and #$80
    JNE fail8
@done:
    rep #$20
    .a16
    lda sres
    and #$00FF
    rts

; ---------------------------------------------------------------------------
; send: build and submit one frame. In: dstv, typv, seqv, corruptv.
; Payload is always "ATARIX". Out: A = Northbridge result (16-bit, high 0).
; ---------------------------------------------------------------------------
send:                               ; payload at (ptr), length len, set by the caller
    .a16
    .i16
    jsr crc32
    lda corruptv
    beq :+
    lda crc_lo
    eor #$0001
    sta crc_lo
:   lda #MAGIC_LO
    sta TX + 0
    lda #MAGIC_HI
    sta TX + 2
    lda typv
    sta TX + 4
    lda len
    sta TX + 6
    lda seqv
    sta TX + 8
    lda #$0000
    sta TX + 10
    lda crc_lo
    sta TX + 12
    lda crc_hi
    sta TX + 14

    sep #$20
    .a8
    ldy #$0000
@copy:
    cpy len
    beq @copied
    lda (ptr),y
    sta TX + HDR,y
    iny
    bra @copy
@copied:
    lda dstv
    sta MB_DST
    lda dstv + 1
    sta MB_DST_HI
    lda #CTL_SUBMIT
    sta MB_CONTROL
    lda MB_RESULT
    rep #$20
    .a16
    and #$00FF
    rts

; ---------------------------------------------------------------------------
; check_pong: the reply must be a PONG from the echo node with the request's
; sequence and payload, a valid header, and a CRC this firmware recomputes.
; Then acknowledge it and confirm RX-ready clears.
; ---------------------------------------------------------------------------
check_pong:
    .a16
    .i16
    sep #$20
    .a8
    lda MB_STATUS
    and #ST_RX_READY
    JEQ pong_bad
    lda MB_TYPE
    cmp #<T_PONG
    JNE pong_bad
    lda MB_TYPE_HI
    cmp #>T_PONG
    JNE pong_bad
    lda MB_SRC
    cmp dstv
    JNE pong_bad
    lda MB_SRC_HI
    cmp dstv + 1
    JNE pong_bad
    lda MB_LEN
    cmp #PAYLOAD_LEN
    JNE pong_bad
    rep #$20
    .a16
    lda RX + 0
    cmp #MAGIC_LO
    JNE pong_bad
    lda RX + 2
    cmp #MAGIC_HI
    JNE pong_bad
    lda RX + 4
    cmp #T_PONG
    JNE pong_bad
    lda RX + 6
    cmp #PAYLOAD_LEN
    JNE pong_bad
    lda RX + 8
    cmp seqv
    JNE pong_bad
    lda RX + 10
    JNE pong_bad

    ; payload echoed byte for byte
    sep #$20
    .a8
    ldy #$0000
@cmp:
    cpy #PAYLOAD_LEN
    beq @payload_ok
    lda RX + HDR,y
    cmp payload,y
    JNE pong_bad
    iny
    bra @cmp
@payload_ok:
    rep #$20
    .a16
    lda #RX + HDR
    sta ptr
    lda #PAYLOAD_LEN
    sta len
    jsr crc32
    lda RX + 12
    cmp crc_lo
    JNE pong_bad
    lda RX + 14
    cmp crc_hi
    JNE pong_bad

    sep #$20
    .a8
    lda #CTL_ACK
    sta MB_CONTROL
    lda MB_STATUS
    and #ST_RX_READY
    JNE pong_bad
    rep #$20
    .a16
    rts
pong_bad:
    rep #$20
    .a16
    jmp fail

check_no_reply:
    .a16
    .i16
    sep #$20
    .a8
    lda MB_STATUS
    and #ST_RX_READY
    rep #$20
    .a16
    beq :+
    jmp fail
:   rts

; ---------------------------------------------------------------------------
; crc32 over len bytes at (ptr) into crc_hi:crc_lo. Reflected, poly
; 0xEDB88320, init and final XOR 0xFFFFFFFF (matches atarix_mailbox_crc32).
; ---------------------------------------------------------------------------
crc32:
    .a16
    .i16
    lda #$FFFF
    sta crc_lo
    sta crc_hi
    ldy #$0000
@byte:
    cpy len
    beq @done
    sep #$20
    .a8
    lda (ptr),y
    rep #$20
    .a16
    and #$00FF
    eor crc_lo
    sta crc_lo
    ldx #$0008
@bit:
    lsr crc_hi
    ror crc_lo
    bcc @next
    lda crc_lo
    eor #$8320
    sta crc_lo
    lda crc_hi
    eor #$EDB8
    sta crc_hi
@next:
    dex
    bne @bit
    iny
    bra @byte
@done:
    lda crc_lo
    eor #$FFFF
    sta crc_lo
    lda crc_hi
    eor #$FFFF
    sta crc_hi
    rts

; ---------------------------------------------------------------------------
; Output helpers (A/X/Y 16-bit on entry and exit)
; ---------------------------------------------------------------------------
puts:                       ; X = address of NUL-terminated string in bank $00
    .a16
    .i16
    sep #$20
    .a8
@loop:
    lda a:$0000,x
    beq @done
    sta UART
    inx
    bra @loop
@done:
    rep #$20
    .a16
    rts

print_hex16:                ; A = value
    .a16
    .i16
    sta tmp
    xba
    jsr print_hex8
    lda tmp
    jsr print_hex8
    rts

print_hex8:                 ; low byte of A
    .a16
    .i16
    pha
    and #$00F0
    lsr
    lsr
    lsr
    lsr
    tax
    sep #$20
    .a8
    lda hexdigits,x
    sta UART
    rep #$20
    .a16
    pla
    and #$000F
    tax
    sep #$20
    .a8
    lda hexdigits,x
    sta UART
    rep #$20
    .a16
    rts

report_ok:
    .a16
    .i16
    ldx #msg_t
    jsr puts
    lda test_no
    jsr print_hex8
    ldx #msg_ok
    jsr puts
    rts

fail:
    rep #$30
    .a16
    .i16
    ldx #msg_fail
    jsr puts
    lda test_no
    jsr print_hex8
    ldx #msg_crlf
    jsr puts
    stp

unexpected:
    rep #$30
    .a16
    .i16
    ldx #msg_irq
    jsr puts
    stp

; ---------------------------------------------------------------------------
; Vectors: native $FFE4-$FFEF, emulation $FFF4-$FFFF
; ---------------------------------------------------------------------------
.segment "VECTORS"
    .word $FFFF, $FFFF              ; $FFE0 reserved
    .word unexpected                ; $FFE4 COP (native)
    .word unexpected                ; $FFE6 BRK (native)
    .word unexpected                ; $FFE8 ABORT (native)
    .word unexpected                ; $FFEA NMI (native)
    .word $FFFF                     ; $FFEC reserved
; With Vector Pull Rewrite, native IRQ level n fetches $FFEE + 2n. Levels 3-6
; reuse the emulation-mode COP/reserved/ABORT/NMI slots (as on Vega816), so
; those vectors serve native IRQ levels once the CPU is in native mode.
    .word irq_level0                ; $FFEE IRQ level 0 (native)
    .word unexpected                ; $FFF0 IRQ level 1
    .word irq_level2                ; $FFF2 IRQ level 2
    .word unexpected                ; $FFF4 IRQ level 3 / COP (emulation)
    .word unexpected                ; $FFF6 IRQ level 4
    .word unexpected                ; $FFF8 IRQ level 5 / ABORT (emulation)
    .word unexpected                ; $FFFA IRQ level 6 / NMI (emulation)
    .word reset                     ; $FFFC RESET
    .word unexpected                ; $FFFE IRQ/BRK (emulation)
