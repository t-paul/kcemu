;--------------------------------------------------------------------------
; Autostart for KC85/3, KC84/4 EPROM-Module
; (c) 2006 by Torsten Paul
;
; This code is supposed to be used for the KCemu autostart module and
; was never tested on a real KC.
;--------------------------------------------------------------------------

			CPU	Z80

PV1			EQU	0f003h
UP_CRT			EQU	00h
UP_KBDS			EQU	0ch
UP_WAIT			EQU	14h
UP_OSTR			EQU	23h
UP_MODU			EQU	26h
UP_CRLF			EQU	2ch
UP_ZKOUT		EQU	45h
BASE			EQU	0c000h
TRAMPOLINE		EQU	0b700h		; tape buffer

;--------------------------------------------------------------------------

			org	BASE

			jr	START

TEXT:			db	14 dup 0

START_ADDR:		dw	0

LOAD_ADDR:		dw	0ffffh, 0, 0
			dw	0ffffh, 0, 0
			dw	0ffffh, 0, 0
			dw	0ffffh, 0, 0

LOADER_CMP:		db	011h, 0bah, 000h, 0bah
LOADER_PATCH:		db	0cdh, 003h, 0f0h, 010h

START:

;--------------------------------------------------------------------------

			PHASE	$ - BASE + 4000h
			;
			; check module control byte
			;
			ld	a, 1
			ld	l, 8
			call	PV1
			db	UP_MODU

			ld	a, d
			cp	0c1h
			jr	z, SWITCH_OK

			;
			; not running at c000h, switch of BASIC
			; and change  module address via
			; trampoline code copied to 200h
			;
			ld	a, 2
			ld	l, 2
			ld	d, 0
			call	PV1
			db	UP_MODU

			ld	hl, TRAMPOLINE_START
			ld	de, TRAMPOLINE
			ld	bc, TRAMPOLINE_END - TRAMPOLINE_START
			ldir

			ld	hl, 0c000h
			push	hl

			ld	a, 2
			ld	l, 8
			ld	d, 0c1h
			jp	TRAMPOLINE

TRAMPOLINE_START:
			;
			; tampoline code to switch module to c000h
			; used again later when starting the program
			;
			call	PV1
			db	UP_MODU
			pop	hl
			jp	(hl)
TRAMPOLINE_END:

SWITCH_OK:	
			DEPHASE

;--------------------------------------------------------------------------

			;
			; now running at c000h
			;
			call	PV1
			db	UP_OSTR
			db	0ch			; CLS
			db	"Autostart von Modul im Schacht 8..."
			db	0dh, 0ah
			db	0dh, 0ah
			db	"Programm: "
			db	0

			ld	hl, TEXT
			call	PV1
			db	UP_ZKOUT

			call	PV1
			db	UP_CRLF
			call	PV1
			db	UP_CRLF

			ld	a, 0ffh
			call	PV1
			db	UP_WAIT

			;
			; switch on RAM at 4000h
			;
			ld	a, 2
			ld	l, 4			; RAM at 4000h
			ld	d, 03h
			call	PV1
			db	UP_MODU
			
			ld	a, 0ch
			ld	d, 43h
			call	ENABLE_RAM

			ld	hl, LOAD_ADDR
			ld	iy, 0c100h
LOOP:
			ld	e, (HL)
			inc	hl
			ld	d, (HL)
			inc	hl
			ld	c, (HL)
			inc	hl
			ld	b, (HL)
			inc	hl
			ld	a, c			; check for size == 0
			or	b
			jr	z, DONE

			ld	a, (HL)
			inc	hl
			or	a

			jr	z, NO_CAOSE_OFF
			call	CAOSE_OFF
NO_CAOSE_OFF:
     			ld	a, (HL)
			inc	hl
			or	a
			jr	z, NO_IRM_OFF
			call	IRM_OFF
NO_IRM_OFF:

			push	hl
			ld	hl, 0c200h
			ldir
			call	IRM_ON
			call	CAOSE_ON

			call	CHECK_LOADER

			;
			; switch to next page
			;
			push	iy
			pop	de
			ld	a, d
			add	a, 4
			cp	0d1h			; reached last page
			jr	z, DONE

			ld	d, a
			push	de
			pop	iy

			ld	a, 2
			ld	l, 8
			call	PV1
			db	UP_MODU

			pop	hl
			jr	LOOP

DONE:
			;
			; reuse trampoline code to switch BASIC on
			; and jump to program start address
			;
			ld	hl, (START_ADDR)
			push	hl

			ld	a, 2
			ld	l, a
			ld	d, 1
			jp	TRAMPOLINE

;--------------------------------------------------------------------------

ENABLE_RAM:
			;
			; check RAM module at slot given in register a and
			; enable at with control byte in register d if found
			;
			; M035 can't be used as it is not possible to switch it to
			; a base address other than 8000h
			;
			ld	l, a
			ld	a, 1
			call	PV1
			db	UP_MODU
			ld	a, h
			cp	0f4h			; M022, 16k RAM
			jr	z, ENABLE_RAM_MODULE
			cp	0f6h			; M011, 64k RAM
			jr	z, ENABLE_RAM_MODULE
			cp	078h			; M036, 128k segmented RAM
			jr	z, ENABLE_RAM_MODULE
			cp	079h			; M032, 256k segmented RAM
			jr	z, ENABLE_RAM_MODULE
			cp	07ah			; M034, 512k segmented RAM
			jr	z, ENABLE_RAM_MODULE
			jr	NO_RAM_MODULE

ENABLE_RAM_MODULE:
			ld	a, 2
			call	PV1
			db	UP_MODU

NO_RAM_MODULE:
			ret

;--------------------------------------------------------------------------

			;
			; first check if loader is at ba00h and
			; starts at ba11h
			;
CHECK_LOADER:
			ld	b, 4
			ld	hl, START_ADDR
			ld	de, LOADER_CMP
CHECK_LOADER_M1:
			ld	a, (de)
			cp	(hl)
			ret	nz
			inc	hl
			inc	de
			djnz	CHECK_LOADER_M1

			;
			; addresses match, check if we find the
			; call to the tape routine in the memory
			; between ba00h and be00h
			;
			ld	hl, 0ba00h
CHECK_LOADER_M2:
			ld	b, 4
			ld	de, LOADER_PATCH

CHECK_LOADER_M3:
			ld	a, h
			cp	0beh
			ret	z

			ld	a, (de)
			cp	(hl)
			inc	hl
			jr	nz, CHECK_LOADER_M2
			inc	de
			djnz	CHECK_LOADER_M3

			;
			; found, patch tape load with call to wait
			;
			dec	hl
			ld	a, UP_WAIT
			ld	(hl), a

			;
			; show that we disabled the loader
			;
			call	PV1
			db	UP_OSTR
			db	"Nachladeroutine ausgeschaltet."
			db	0dh, 0ah
			db	0dh, 0ah
			db	0

			ld	a, 0ffh
			call	PV1
			db	UP_WAIT

			ret

;--------------------------------------------------------------------------

CAOSE_ON:
			;
			; switch on CAOS E
			;
			in	a, (88h)
			or	1
			out	(88h), a
			ret

;--------------------------------------------------------------------------

CAOSE_OFF:
			;
			; switch off CAOS E
			;
			in	a, (88h)
			and	0feh
			out	(88h), a
			ret

;--------------------------------------------------------------------------

IRM_ON:
			;
			; switch on IRM
			;
			in	a, (88h)
			or	4
			out	(88h), a
			ret

;--------------------------------------------------------------------------

IRM_OFF:
			;
			; switch off IRM
			;
			in	a, (88h)
			and	0fbh
			out	(88h), a
			ret

;--------------------------------------------------------------------------

			end
