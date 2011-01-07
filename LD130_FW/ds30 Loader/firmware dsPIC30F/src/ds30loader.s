;------------------------------------------------------------------------------
;
; Title:			ds30 Loader for dsPIC30F
;
; File description:	Main firmwarefile
;
; Copyright: 		Copyright © 08-10, Mikael Gustafsson
;
; Version			3.0.0 october 2010
;
; Webpage: 			http://mrmackey.no-ip.org/elektronik/ds30loader/
;
; Thanks to:		Daniel Chia, for distributing the sourcecode that this code is based on
;
; History:			3.0.0 Improvement: CAN and UART firmwares now share the same MPLAB project
;					2.1.0 New feature: auto baudrate detection
;					      Improvement: more versatile bootloader protection
;						  Change: size is 7 rows
;					2.0.4 Bugfix: nop inserted before check of trmt in Send()
;						  Change: size is 5 rows
;					      New feature: assigns goto at 0x00 when writing row to increase safety
;					      New feature: bootloader protect, refuses to write to bootloader location
;					2.0.2 -
;					2.0.1 Fixed baudrate error check
;					2.0.0 Added flash & eeprom verify
;						  Size of bl is now 4 rows instead of 3
;						  Command decides what to do instead of address
;					1.0.1 Removed initialization of stack limit register
;						  Added baudrate error check
;						  BRG is rounded instead of truncated
;					1.0.0 Added watchdog clear
;						  Sends firmware version
;						  Disables uart transmit on exit
;						  Clear uart interupt flags on exit
;						  Tweaked code
;					0.9.4 Added support for programming of eeprom
;						  Cleaner/smaller code for loading latches
;					0.9.3 Simpler setup for the end user
;						  All settings moved to settings.inc
;					0.9.2 New algoritm where only 96 bytes are received for each row instead of 128, ~25% performance improvement
;					0.9.1 Code improvement: flash was written 8 times for each row, only 1 time is needed
;					0.9.0 Initial release
;
;------------------------------------------------------------------------------

;-----------------------------------------------------------------------------
;    This file is part of ds30 Loader.
;
;    ds30 Loader is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation.
;
;    ds30 Loader is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with ds30 Loader. If not, see <http://www.gnu.org/licenses/>.
;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
;
; USAGE  USAGE  USAGE  USAGE  USAGE  USAGE  USAGE  USAGE  USAGE  USAGE  USAGE
;
;------------------------------------------------------------------------------
;
; This MPLAB-project comes preconfigured for dsPIC30F4011 using internal RC-
; oscillator w/PLL to achieve 30MIPS.
;
; To use other chip and/or configuration you need to do:
; * Select correct device in MPLAB IDE, menu Configure->Select Device
; * Include correct linker script
; * Search for xxx in settings.inc and check/alter those lines
;
; * Tabsize used is 4
;
;------------------------------------------------------------------------------



;------------------------------------------------------------------------------
; Registers usage
;------------------------------------------------------------------------------
		;.equ	MIXED,		W0		;immediate
		;.equ	MIXED,		W1		;immediate
		.equ	WBUFPTR,	W2		;buffer pointer
		.equ	WCNT,		W3		;loop counter
		.equ	WADDR2,		W4		;memory pointer
		.equ	WADDR,		W5		;memory pointer
		;.equ	UNUSED,		W6		;
		;.equ	UNUSED,		W7		;
		.equ	WTEMP1,		W8		;
		.equ	WTEMP2,		W9		;
		.equ	WDEL1,		W10		;delay outer
		.equ	WDEL2,		W11		;delay inner
		;.equ	UNUSED,		W12		;
		.equ	WCMD,		W13		;command
		.equ 	WCRC, 		W14		;checksum
		.equ	WSTPTR,		W15		;stack pointer


;------------------------------------------------------------------------------
; Includes
;------------------------------------------------------------------------------
		.include "settings.inc"


;-----------------------------------------------------------------------------
; UARTs
;------------------------------------------------------------------------------
		.ifndef USE_UART1
			.ifdef	USE_ALTIO
				.error "Use of alternate i/o is only valid for uart1"
			.endif
		.endif

		.ifdef USE_UART1
			.equ	USE_UART,	1

			.equ    UMODE,	    U1MODE					;uart mode
		   	.equ    USTA,  		U1STA					;uart status
		   	.equ    UBRG,		U1BRG					;uart baudrate
		   	.equ    UTXREG,		U1TXREG					;uart transmit
		   	.equ	URXREG,		U1RXREG					;uart receive
			.equ	URXIF,		U1RXIF					;uart received interupt flag
			.equ	UTXIF,		U1TXIF					;uart transmit interupt flag
		.endif

		.ifdef USE_UART2
			.ifdef USE_UART1
				.error "Both uarts are specified"
			.endif

			.ifndef HAS_UART2
				.error "UART2 specified for a device that only has uart1"
			.endif

			.equ	USE_UART,	1

		   	.equ    UMODE,	    U2MODE					;uart mode
		   	.equ    USTA,  		U2STA					;uart status
		   	.equ    UBRG,		U2BRG					;uart baudrate
		   	.equ    UTXREG,		U2TXREG					;uart transmit
		   	.equ	URXREG,		U2RXREG					;uart receive
			.equ	URXIF,		U2RXIF					;uart received interupt flag
			.equ	UTXIF,		U2TXIF					;uart transmit interupt flag
		.endif


;------------------------------------------------------------------------------
; CAN stuff
;------------------------------------------------------------------------------
		.ifdef USE_CAN1
			.ifdef USE_CAN2
				.error "Both CAN ports are specified"
			.endif

			.ifndef HAS_CAN1
				.error "CAN is specified for a device that don't have CAN"
			.endif

			.equ	USE_CAN,	1


			.equ	CiCFG1, 	C1CFG1
			.equ	CiCFG2, 	C1CFG2
			.equ	CiCTRL, 	C1CTRL
			.equ	CiRX0CON, 	C1RX0CON
			.equ	CiRXM0SID, 	C1RXM0SID
			.equ	CiRXF0SID, 	C1RXF0SID
			.equ	CiRXF1SID, 	C1RXF1SID
			.equ	CiRX1CON, 	C1RX1CON
			.equ	CiRXM1SID, 	C1RXM1SID
			.equ	CiRXF2SID, 	C1RXF2SID
			.equ	CiRXF3SID, 	C1RXF3SID
			.equ	CiRXF4SID, 	C1RXF4SID
			.equ	CiRXF5SID, 	C1RXF5SID

			.equ	CiTX0B1,	C1TX0B1
			.equ	CiTX0SID,	C1TX0SID
			.equ	CiCTRLH,	C1CTRLH
			.equ	CiCTRLL,	C1CTRLL
			.equ	CiTX0DLC,	C1TX0DLC
			.equ	CiTX0CON,	C1TX0CON
			.equ	CiINTF,		C1INTF
			.equ	CiRX0DLC,	C1RX0DLC
			.equ	CiRX0CON,	C1RX0CON
			.equ	CiRX0B1,	C1RX0B1
		.endif

		.ifdef USE_CAN2
			.ifndef HAS_CAN2
				.error "CAN2 specified for a device that don't have CAN2"
			.endif

			.equ	USE_CAN,	1

			.equ	CiCFG1, 	C2CFG1
			.equ	CiCFG2, 	C2CFG2
			.equ	CiCTRL, 	C2CTRL
			.equ	CiRX0CON, 	C2RX0CON
			.equ	CiRXM0SID, 	C2RXM0SID
			.equ	CiRXF0SID, 	C2RXF0SID
			.equ	CiRXF1SID, 	C2RXF1SID
			.equ	CiRX1CON, 	C2RX1CON
			.equ	CiRXM1SID, 	C2RXM1SID
			.equ	CiRXF2SID, 	C2RXF2SID
			.equ	CiRXF3SID, 	C2RXF3SID
			.equ	CiRXF4SID, 	C2RXF4SID
			.equ	CiRXF5SID, 	C2RXF5SID

			.equ	CiTX0B1,	C2TX0B1
			.equ	CiTX0SID,	C2TX0SID
			.equ	CiCTRLH,	C2CTRLH
			.equ	CiCTRLL,	C2TRLL
			.equ	CiTX0DLC,	C2TX0DLC
			.equ	CiTX0CON,	C2TX0CON
			.equ	CiINTF,		C2INTF
			.equ	CiRX0DLC,	C2RX0DLC
			.equ	CiRX0CON,	C2RX0CON
			.equ	CiRX0B1,	C2RX0B1
		.endif


;------------------------------------------------------------------------------
; Constants, don´t change
;------------------------------------------------------------------------------
		.equ	VERMAJ,		3										/*firmware version major*/
		.equ	VERMIN,		0										/*firmware version minor*/
		.equ	VERREV,		0										/*firmware version revision*/

		.equ 	HELLO, 		0xC1
		.equ 	OK, 		'K'										/*erase/write ok*/
		.equ 	CHECKSUMERR,'N'										/*checksum error*/
		.equ	VERFAIL,	'V'										/*verification failed*/
		.equ   	BLPROT,     'P'                              		/*bl protection tripped*/
		.equ   	UCMD,     	'U'                              		/*unknown command*/

		.equ	BLDELAY,	( BLTIME * (FCY / 1000) / (65536 * 7) )	/*delay before user application is loaded*/
		.equ	UARTBR,		( (((FCY / BAUDRATE) / 8) - 1) / 2 )	/*brg calculation with rounding*/


		.equ	PAGESIZER,	1										/*pagesize [rows]*/
		.equ	ROWSIZEW,	32										/*rowsize [words]*/
		.equ	STARTADDR,	( FLASHSIZE - BLPLR * PAGESIZER * ROWSIZEW * 2 )	/*bootloader placement*/
		.equ	BLSTARTROW,	(STARTADDR / ROWSIZEW / 2)
		.equ	BLENDROW,	(BLSTARTROW + BLSIZER - 1)


		.ifdef USE_CAN
			.equ	ECAN_MODE_NORMAL,			0x00
			.equ	ECAN_MODE_CONFIG,			0x04
		.endif

		.ifdef DEV_MODE
			.equ LED0,                LATE0
			.equ LED1,                LATE1
			.equ LED2,                LATE2
			.equ LED3,                LATE3
			.equ LED4,                LATE4
			.equ LED5,                LATE5
		.endif


;------------------------------------------------------------------------------
; Validate user settings
;------------------------------------------------------------------------------
		;
		.ifdef	USE_CAN
			.ifdef USE_UART
				.error "Both UART and CAN is specified"
			.endif
		.endif
		.ifndef	USE_CAN
			.ifndef USE_UART
				.error "Neither UART nor CAN is specified"
			.endif
		.endif
				;  Internal cycle clock
		.if FCY > 30000000
			.error "Fcy specified is out of range"
		.endif

		; Baudrate error
		.equ REALBR,	( FCY / (16 * (UARTBR+1)) )
		.equ BAUDERR,	( (1000 * (REALBR - BAUDRATE)) / BAUDRATE )
		.if ( BAUDERR > 25) || (BAUDERR < -25 )
			.error "Baudrate error is more than 2.5%. Remove this check or try another baudrate and/or clockspeed."
		.endif


;------------------------------------------------------------------------------
; Global declarations
;------------------------------------------------------------------------------
        .global	__reset          	;the label for the first line of code, needed by the linker script

		.global __StackError
    	.global __OscillatorFail
    	.global __AddressError
	   	.global __MathError


;------------------------------------------------------------------------------
; Uninitialized variables in data memory
;------------------------------------------------------------------------------
 		.bss	ptrRead, ( 2 )
		.bss	ptrWrite, ( 2 )
		.bss	iUnread, ( 2 )
		.bss	rxBuffer, ( 128 )
		.bss	buffera, ( 16 )
		.bss	buffer, ( ROWSIZEW * 3 + 2/*checksum*/ )
		.bss	bufferb, ( 16 )
		.bss	buffer0, ( ROWSIZEW * 3 + 2/*checksum*/ )


;------------------------------------------------------------------------------
; Send macro
;------------------------------------------------------------------------------
		.macro SendL char
			mov 	#\char, W0
			rcall Send
		.endm

		.ifdef USE_CAN
			.macro SetCANMode char
				mov 	#\char, W1
				rcall SetCANMode_
			.endm
		.endif


;------------------------------------------------------------------------------
; Traps
;------------------------------------------------------------------------------
 		.ifdef DEV_MODE
 		.text
__StackError:
       	bset	LATE, #LED0
       	bclr  	INTCON1, #STKERR  ;reset stack error trap flag
		bra 	__StackError
__MathError:
       	bset	LATE, #LED1
       	bclr  	INTCON1, #MATHERR  ;reset math error trap flag
		bra 	__MathError
__OscillatorFail:
       	bset	LATE, #LED2
       	bclr  	INTCON1, #OSCFAIL  ;reset oscillator fail trap flag
		bra 	__OscillatorFail
__AddressError:
       	bset	LATE, #LED3
       	bclr 	INTCON1, #ADDRERR  ;reset address error trap flag
		bra 	__AddressError
		.endif


;------------------------------------------------------------------------------
; Start of code section in program memory
;------------------------------------------------------------------------------
		.section *, code,address(STARTADDR-4)
usrapp:	nopr						;these two instructions will be replaced
		nopr						; with a goto to the user app. by the pc program


;------------------------------------------------------------------------------
; Reset vector
;------------------------------------------------------------------------------
		.section *, code,address(STARTADDR)
__reset:mov 	#__SP_init, WSTPTR	;initalize the Stack Pointer


;------------------------------------------------------------------------------
; Init
;------------------------------------------------------------------------------
		.ifdecl ADPCFGL
			.ifndef DEV_MODE
;******************************************
				mov 	#buffera, WBUFPTR
				mov.b 	#'A',W0

				mov 	#0xE, WCNT
Init1:
				mov.b 	W0, [WBUFPTR++]
				add		#1, W0
				dec 	WCNT, WCNT
				bra 	nz, Init1

				mov.b 	#'*',W0,
				mov.b 	W0, [WBUFPTR++]

				mov 	#bufferb, WBUFPTR
				mov.b 	#'1',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'2',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'3',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'4',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'5',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'6',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'7',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'8',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'9',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'A',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'B',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'C',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'D',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'E',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'F',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'*',W0,
				mov.b 	W0, [WBUFPTR++]

				mov 	#buffer0, WBUFPTR
				mov.b 	#'A',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'S',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'D',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'F',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'G',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'H',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'J',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'K',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'L',W0,
				mov.b 	W0, [WBUFPTR++]
				mov.b 	#'*',W0,
				mov.b 	W0, [WBUFPTR++]
;******************************************


 				bclr.b	_ADCON1bits+1,#7	;	// disable analog module

				setm	w0
 				mov	w0,_ADPCFG				;	// all pins are digital input/ouput pins


;				bclr ADCON1,#ADON
;				mov #0xFFFF, W0
;				mov W0,ADPCFG	;	// all pins are digital input/ouput pins
;				.error "Do you need to configura uart pins to be digital? If not, remove this line"
			.endif
		.endif

		.ifdef DEV_MODE
			clr		TRISE		;led outputs
			clr		LATE
		.endif


;------------------------------------------------------------------------------
; Init UART
;------------------------------------------------------------------------------
		.ifdef USE_UART
			; Make uart pins digital
			clr	w0
 			mov	w0,_TRISF				; // use all pins as output


			; UART
			.ifndef USE_ABAUD
				mov		#UARTBR, W0
		       	mov 	W0, UBRG			;set baudrate
				.ifdef 	USE_ALTIO
					bset	U1MODE, #ALTIO
				.endif
				bset 	UMODE, #UARTEN		;enable UART
				bset 	USTA, #UTXEN		;enable TX

			; UART witht auto baudrate detection
			.else
		       	.ifdef 	USE_ALTIO
					bset	U1MODE, #ALTIO
				.endif
				bset 	UMODE, #UARTEN		;enable UART
				bset 	U1MODE, #ABAUD      ;enable auto baudrate detection
				; Setp timer 3
		        clr 	T3CON             	;
		        bclr 	IEC0, #T3IE         ;disable timer 3 interrupt
		        setm 	PR3                 ;maximum period time
		        mov 	#0x8000, W0         ;start, 1:1 prescaler, internal cycle as clock source
		        mov 	W0, T3CON
		        ; Setp input capture
				mov		#3, W0  	        ;capture mode, every rising edge
		        mov 	W0, IC1CON        	;
		        bclr 	IFS0, #IC1IF        ;clear input capture interrupt flag
		        ; Begin autobaud detection
				mov 	#0x0004, W0			;detected 4 edges after first 4 edge, total 5 for 0x55
		        rcall 	WaitIC          	;wait for first rising edge
		        clr 	TMR3                ;reset timer 3
	edgeloop:   rcall 	WaitIC
		        dec 	W0, W0
		        bra 	nz, edgeloop
		        ; Do calculation
		        mov 	TMR3, W0
		        add 	#0x40, W0           ;add 0.5(64/128) => rounding
		        asr 	W0, #7, W0          ;W0 = (Tend - Tstart/ 128) + 0.5
		        dec 	W0, W0
				; Detection comleted, clean up
				bclr 	T3CON, #TON         ;disable timer 3
		        bclr 	IFS0, #IC1IF        ;clear input capture interrupt flag
		        ; Init uart
		        mov 	W0, U1BRG			;use calculated brg
		        bclr 	UMODE, #ABAUD      	;disable auto baudrate detection
		        bclr 	UMODE, #UARTEN		;disable uart
		        clr		USTA				;clear uart status
		        bset 	UMODE, #UARTEN		;enable uart
		        bset 	USTA, #UTXEN       	;enable tx
				;confirm baudrate detection to application
				SendL	OK
			.endif

			; TX enable, make tx enable pin output and set to 0
			.ifdef USE_TXENABLE
				bclr	TRISR_TXE, #TRISB_TXE
				bclr	LATR_TXE, #LATB_TXE
			.endif
		.endif


;------------------------------------------------------------------------------
; Init CAN
;------------------------------------------------------------------------------
		.ifdef USE_CAN
			;----------------------------------------------------------------------
			; Set CAN mode: configuration
			;----------------------------------------------------------------------
			SetCANMode	ECAN_MODE_CONFIG


			;----------------------------------------------------------------------
			; CAN module
			;----------------------------------------------------------------------

			; Baud rate prescalar
			; SJW
			mov		#( (CAN_BRP-1) + ((CAN_SJW-1)<<6) ), W0
			mov		W0, CiCFG1

			; Propagation
			; Segment 1
			;SJW
			; Segment2
			mov		#( (CAN_PROP-1) + ((CAN_SEG1-1)<<3) + (1<<6)/*SAM*/  + (1<<7)/*SEG2PHTS*/ + ((CAN_SEG2-1)<<8) ), W0
			mov		W0, CiCFG2

		    ;CAN control and status register
		    bset	CiCTRL, #CANCKS		;Fcan clock is Fcy
		    bset	CiCTRL, #ABAT		;abort pending transmissions in all TX buffers, clears TXREQ in tx buffer control registers

		    ; Clear interupt flags
		    clr		CiINTF


			;----------------------------------------------------------------------
			; CAN tx
			;----------------------------------------------------------------------
			; CAN transmit 0 SID
			;bset	CiTX0SID, #TXIDE		;extended identifier


			;----------------------------------------------------------------------
			; CAN rx buffer 0
			;----------------------------------------------------------------------
			; Receive buffer 0 control and status register
			;bset	CiRX0CON, #DBEN			;receive buffer 0 overflow will write to Receive Buffer 1
			bclr	CiRX0CON, #RXFUL		;receive full status

			; Acceptance mask 0 SID/EID
			setm	CiRXM0SID
			;bclr	CiRXM0SID, #MIDE		;0=match either standard or extended address message if the filters match
			;clr		CiRXM0EIDH
			;clr		CiRXM0EIDL

			; Acceptance filter  0 SID/EID
			mov		#( (ID_PIC<<2) & 0x1FFC), W0
			mov		W0, CiRXF0SID
			;bset	CiRXF0SID, #EXIDE		;1=enable filter for extended identifier
			;clr		CiRXF0EIDH
			;clr		CiRXF0EIDL

			; Acceptance filter  1 SID/EID, not used
			clr 	CiRXF1SID
			;bset	CiRXF1SID, #EXIDE		;1=enable filter for extended identifier
			;clr		CiRXF1EIDH
			;clr		CiRXF1EIDL


			;----------------------------------------------------------------------
			; CAN rx buffer 1
			;----------------------------------------------------------------------
			; Receive buffer 1 control and status register
			bclr	CiRX1CON, #RXFUL		;receive full status

			; Acceptance mask 1 SID/EID
			setm	CiRXM1SID
			;bclr	CiRXM1SID, #MIDE		;0=match either standard or extended address message if the filters match
			;clr		CiRXM1EIDH
			;clr		CiRXM1EIDL

			; Acceptance filter  2 SID/EID
			clr		CiRXF2SID
			;bset	CiRXF2SID, #EXIDE		;1=enable filter for extended identifier
			;clr		CiRXF2EIDH
			;clr		CiRXF2EIDL

			; Acceptance filter  3 SID/EID, not used
			clr 	CiRXF3SID
			;bset	CiRXF3SID, #EXIDE		;1=enable filter for extended identifier
			;clr		CiRXF3EIDH
			;clr		CiRXF3EIDL

			; Acceptance filter  4 SID/EID, not used
			clr 	CiRXF4SID
			;bset	CiRXF4SID, #EXIDE		;1=enable filter for extended identifier
			;clr		CiRXF4EIDH
			;clr		CiRXF4EIDL

			; Acceptance filter  5 SID/EID, not used
			clr 	CiRXF5SID
			;bset	CiRXF5SID, #EXIDE		;1=enable filter for extended identifier
			;clr		CiRXF5EIDH
			;clr		CiRXF5EIDL


			;----------------------------------------------------------------------
			; Set CAN mode: normal
			;----------------------------------------------------------------------
			SetCANMode ECAN_MODE_NORMAL


			;----------------------------------------------------------------------
			; Init can
			;----------------------------------------------------------------------
			mov		#rxBuffer, W0
			mov		W0, ptrRead
			mov		W0, ptrWrite
			clr		iUnread
		.endif


;------------------------------------------------------------------------------
; Receive hello
;------------------------------------------------------------------------------
		rcall 	Receive
		sub 	#HELLO, W0
		bra 	nz, exit			;unknown prompt


;------------------------------------------------------------------------------
; Send device id and firmware version
;------------------------------------------------------------------------------
		SendL 	DEVICEID
		SendL	VERMAJ
		SendL	(VERMIN*16 + VERREV)


;------------------------------------------------------------------------------
; Main
;------------------------------------------------------------------------------
Main:	SendL 	OK

		; Init checksum
main1:	clr 	WCRC

		; Init CAN rx buffer
		.ifdef USE_CAN
			mov		#rxBuffer, W0
			mov		W0, ptrRead
			mov		W0, ptrWrite
			;clr		iUnread		;should allready be 0 when we get here, error condition if not
		.endif


		;----------------------------------------------------------------------
		; Receive address
		;----------------------------------------------------------------------
		; Upper
		rcall 	Receive
		mov 	W0, NVMADRU
		mov 	W0, TBLPAG
		; High
		rcall 	Receive
		mov.b 	WREG, NVMADR+1
		; Low
		rcall 	Receive
		mov.b 	WREG, NVMADR
		;
		mov		NVMADR, WREG
		mov		W0, WADDR
		mov		W0, WADDR2


		;----------------------------------------------------------------------
		; Receive command
		;----------------------------------------------------------------------
		rcall 	Receive
		mov		W0, WCMD


		;----------------------------------------------------------------------
		; Receive nr of data bytes that will follow
		;----------------------------------------------------------------------
		rcall 	Receive
		mov 	W0, WCNT


		;----------------------------------------------------------------------
		; Receive data
		;----------------------------------------------------------------------
		mov 	#buffer, WBUFPTR
rcvdata:rcall 	Receive
		mov.b 	W0, [WBUFPTR++]
		dec		WCNT, WCNT
		bra 	nz, rcvdata			;last byte received is checksum


		;----------------------------------------------------------------------
		; 0x00 goto protection
		;----------------------------------------------------------------------
		.ifdef	PROT_GOTO
			cp0		NVMADRU
			bra		nz, chksum
			cp0		NVMADR
			bra		nz, chksum

			;
			mov 	#buffer, WBUFPTR
			; 1st word upper byte = goto instruction
			mov.b 	#0x04, W0
			mov.b	W0, [WBUFPTR++]
			; 1st word  low byte = low address byte
			mov.b 	#(0xff & STARTADDR), W0
			mov.b 	W0, [WBUFPTR++]
			; 1st word high byte = high address byte
			mov.b 	#(0xff & (STARTADDR>>8)), W0
			mov.b 	W0, [WBUFPTR++]
			;2nd word upper byte = unused
			clr.b	[WBUFPTR++]
			; 2nd word  low byte = upper address byte
			mov.b 	#(0xff & (STARTADDR>>16)), W0
			mov.b 	W0, [WBUFPTR++]
			; 2nd word high byte = unused
			clr.b 	[WBUFPTR++]
		.endif


		;----------------------------------------------------------------------
		; Check checksum
		;----------------------------------------------------------------------
chksum:	cp0.b 	WCRC
		bra 	z, ptrinit
		SendL 	CHECKSUMERR
		bra 	main1


		;----------------------------------------------------------------------
		; Init pointer
		;----------------------------------------------------------------------
ptrinit:mov 	#buffer, WBUFPTR


		;----------------------------------------------------------------------
		; Check command
		;----------------------------------------------------------------------
		; Write row
		btsc	WCMD, #1
		bra		blprot
		; Write EEROM word
		btsc	WCMD, #2
		bra		eeprom
		; Write Config word
		btsc	WCMD, #3
		bra		config
		; Else, unknown command
		SendL   UCMD
		bra     main1


		;----------------------------------------------------------------------
		; Bootloader protection
		;----------------------------------------------------------------------
blprot:	nop
		.ifdef PROT_BL
			; Calculate row number of received address
			mov		TBLPAG, W1
			mov		WADDR, W0
			mov		#(ROWSIZEW*2), WTEMP1
			repeat	#17
			div.ud	W0, WTEMP1;		W = received address / (rowsizew*2)
			; Received row < bl start row = OK
			mov		#BLSTARTROW, WTEMP1
			cp		W0, WTEMP1
			bra		N, blprotok
			; Received row > bl end row = OK
			mov		#BLENDROW, WTEMP1
			cp		WTEMP1, W0
			bra		N, blprotok
			; Protection tripped
			SendL   BLPROT
		    bra     main1
			; Restore WADDR2
blprotok:	mov		WADDR, WADDR2
		.endif

;******************************************
;		mov 	#buffera, W2
;		mov.b 	[w2],W0
;		add   	#1, W0
;		mov.b 	W0, [W2++]
;		mov.b 	#'Z',W0
;		mov.b 	W0, [W2++]
;******************************************

		;----------------------------------------------------------------------
		; Erase & write row
		;----------------------------------------------------------------------
		; Erase row
errow:
		mov 	#0x4041, W0
		rcall 	Write
		; Load latches
		mov 	#ROWSIZEW, WCNT
latlo:	tblwth.b 	[WBUFPTR++], [WADDR] 	;upper byte
		tblwtl.b	[WBUFPTR++], [WADDR++] 	;low byte
		tblwtl.b	[WBUFPTR++], [WADDR++] 	;high byte
		dec 	WCNT, WCNT
		bra 	nz, latlo
		; Write
		mov 	#0x4001, W0
		rcall	Write

;******************************************
;		mov 	#0xFFFF, WCNT
;delaywrite:
;		dec 	WCNT, WCNT
;		bra 	nz, delaywrite
;
;		mov 	#0xFFFF, WCNT
;delaywrite1:
;		dec 	WCNT, WCNT
;		bra 	nz, delaywrite1
;******************************************

		;----------------------------------------------------------------------
		; Verify row
		;----------------------------------------------------------------------
		mov 	#ROWSIZEW, WCNT
		mov 	#buffer, WBUFPTR
		mov		WADDR2, W0
		mov		WREG, NVMADR

		; Verify upper byte
verloop:
;******************************************
;		mov 	#buffer0, W6
;		mov.b 	#'^',W0,
;		mov.b 	W0, [W6++]
;		mov.b 	W0, [W6++]
;		mov.b 	W0, [W6++]
;		mov.b 	W0, [W6++]
;******************************************

		tblrdh.b [WADDR2], W0
;		mov.b 	W0, [W6++]
		cp.b	W0, [WBUFPTR++]
;		bra		NZ, vfail

		; Verify low byte
		tblrdl.b [WADDR2++], W0
;		mov.b 	W0, [W6++]
		cp.b	W0, [WBUFPTR++]
		bra		NZ, vfail

		; Verify high byte
		tblrdl.b [WADDR2++], W0
;		mov.b 	W0, [W6++]
		cp.b	W0, [WBUFPTR++]
		bra		NZ, vfail

		; Loop
		dec		WCNT, WCNT
		bra 	nz, verloop
		; Verify finished
		bra		Main


		;----------------------------------------------------------------------
		; Erase, write & verify eeprom word
		;----------------------------------------------------------------------
		;Erase eeprom word
eeprom:	mov 	#0x4044, W0
		rcall 	Write
		; Load latch
		tblwtl 	[WBUFPTR], [WADDR]
		; Write eeprom word
		mov 	#0x4004, W0
		rcall 	Write
		; Verify eeprom word
		tblrdl	[WADDR], W0
		cp		W0, [WBUFPTR]
		bra		Z, Main
		; Else verify fail (below)


		;----------------------------------------------------------------------
		; Verify fail
		;----------------------------------------------------------------------
vfail:	SendL	VERFAIL
		bra		main1


		;----------------------------------------------------------------------
		; Write config word, does not need erase
		;----------------------------------------------------------------------
		; Load latch
config:	tblwtl 	[WBUFPTR], [WADDR]
		; Write config word
		mov 	#0x4008, W0
		rcall 	Write
		; Write finished
		bra		Main


;------------------------------------------------------------------------------
; SetCANMode()
; Arg: mode in W1
;------------------------------------------------------------------------------
.ifdef USE_CAN
SetCANMode_:
		mov		#0xF8, W0
		and.b	CiCTRLH
		mov		W1, W0
		ior.b	CiCTRLH
scm:	mov.b	CiCTRLH, WREG
		mov		W0, W1
		and		#7, W1
		mov.b	CiCTRLL, WREG
		lsr		W0, #5, W0
		and		#7, W0
		cp		W0, W1
		bra		nz, scm
		return
.endif


;------------------------------------------------------------------------------
; WaitIC()
;------------------------------------------------------------------------------
.ifdef USE_UART
.ifdef USE_ABAUD
WaitIC: mov 	#BLDELAY, WDEL1
wicol: 	clr 	WDEL2
wicil: 	clrwdt					;clear watchdog
		btsc 	IFS0, #IC1IF	;rising edge detected?
        bra 	EdgeDet
        dec 	WDEL2, WDEL2
        nop						;nop so we can use same delay as comm.
        nop
        bra 	nz, wicil
        dec 	WDEL1, WDEL1
        bra 	nz, wicol
        ; Timeou
        goto 	usrapp
EdgeDet:bclr 	IFS0, #IC1IF    ;clear input capture interrupt flag
        return
.endif
.endif


;------------------------------------------------------------------------------
; Write()
;------------------------------------------------------------------------------
Write:
		clrwdt

		mov 	W0, NVMCON

		; Disable interrupts while the KEY sequence is written
		push 	SR
		mov 	#0x00E0,W0
		ior 	SR

		mov 	#0x55, W0
		mov 	W0, NVMKEY
		mov 	#0xAA, W0
		mov 	W0, NVMKEY
		bset 	NVMCON, #WR
		nop
		nop

		;Re-enable interrupts, if needed
		pop 	SR

		nop
		nop
		; Wait for erase/write to finish
compl:	btsc	NVMCON, #WR
		bra 	compl
		return


;------------------------------------------------------------------------------
; Send()
;------------------------------------------------------------------------------
.ifdef USE_UART
Send:	; Enable tx
		.ifdef USE_TXENABLE
			bset	LATR_TXE, #LATB_TXE
			nop
		.endif
		;Send byte
		mov 	WREG, UTXREG
		nop
		nop
		; Wait until transmit shift register is empty
txwait:	btss	USTA, #TRMT
		bra		txwait
		; Disable tx
		.ifdef USE_TXENABLE
			bclr	LATR_TXE, #LATB_TXE
		.endif
		; Send complete
		return
.endif


;------------------------------------------------------------------------------
; Send()
; Arg: data in W0
;------------------------------------------------------------------------------
.ifdef USE_CAN
Send:	; Data
		mov		W0, CiTX0B1
		; Standard identifier
		mov		#( ((ID_GUI<<2)&0xfc) + ((ID_GUI<<5)&0xf800) ), W0
		mov		W0, CiTX0SID
		; Standard identifier bits 0-5
		;clr		CiTX0SID
		;mov		#NODENR_BL, W0
		;sl		W0, #2, W0
		;and		#0xFC, W0
		;mov		W0, CiTX0SID
		; Standard identifier bits 10-6
		;mov		#NODENR_BL, W0
		;sl		W0, #5, W0
		;mov		#0xF800, W1
		;and		W0, W1, W0
		;ior		CiTX0SID
		; Extended identifier
		;mov		#0, W0
		;mov		W0, CiTX0EID
		; Data length code, 1 byte
		clr		CiTX0DLC
		bset	CiTX0DLC, #3
		; Request message transmission
		bset	CiTX0CON, #TXREQ
		; Verify successfull transmission
sendw:	btsc	CiTX0CON, #TXREQ
		bra		sendw
		return
.endif


;------------------------------------------------------------------------------
; Receive()
;------------------------------------------------------------------------------
.ifdef USE_UART
		; Init delay
Receive:mov 	#BLDELAY, WDEL1
		; Check for received byte
rpt1:	clr		WDEL2
rptc:	clrwdt						;clear watchdog
		btss 	USTA, #URXDA
		bra 	notrcv
		mov 	URXREG, W0
		add 	WCRC, W0, WCRC		;add to checksum
		return
 		; Delay
notrcv:	dec 	WDEL2, WDEL2
		bra 	nz, rptc
		dec 	WDEL1, WDEL1
		bra 	nz, rpt1
		; If we get here, uart receive timed out
        mov 	#__SP_init, WSTPTR	;reinitialize the Stack Pointer
.endif


.ifdef USE_CAN
;------------------------------------------------------------------------------
; Receive(), return data from buffer
; Return: data in W0
;------------------------------------------------------------------------------
Receive:rcall	CheckRx
		; Data not available in rx buffer?
		cp0		iUnread
		bra		z, rcv
received:
		mov		ptrRead, W1
		mov.b	[W1], W0
		and		#0xFF, W0
		inc		ptrRead
		dec		iUnread
		return


;----------------------------------------------------------------------
; CheckRx()
;----------------------------------------------------------------------
CheckRx:btss 	CiINTF, #RX0IF
		return

		; Move data from rx0 to buffer
rx0buf:	mov		#CiRX0B1, W0
		mov		CiRX0DLC, WTEMP1
		and		#0xF, WTEMP1
		mov		ptrWrite, W1
		dec		WTEMP1, WTEMP1
		; Loop
		do		WTEMP1, movbyte
		;clr		WTEMP2
		add.b 	WCRC, [W0], WCRC		;add to checksum
		inc		iUnread
movbyte:mov.b	[W0++], [W1++]
		; Save write pointer
		mov		W1, ptrWrite;

		bclr	CiINTF, #RX0IF
		bclr	CiRX0CON, #RXFUL
		return


;------------------------------------------------------------------------------
; Receive, wait for data from CAN bus
;------------------------------------------------------------------------------
		; Init delay
rcv:	mov 	#BLDELAY, WDEL1
		; Check for received byte
rpt1:	clr		WDEL2
rptc:	clrwdt						;clear watchdog
		rcall	CheckRx
		cp0		iUnread
		bra 	nz, received

		;----------------------------------------------------------------------
 		; Delay
 		;----------------------------------------------------------------------
		dec 	WDEL2, WDEL2
		bra 	nz, rptc
		dec 	WDEL1, WDEL1
		bra 	nz, rpt1
		; If we get here, receive timed out
        mov 	#__SP_init, WSTPTR	;reinitialize the Stack Pointer
.endif


;------------------------------------------------------------------------------
; Exit point, clean up and load user application
;------------------------------------------------------------------------------
exit:
		.ifdef USE_UART
		bclr	UMODE, #UTXEN		;disable uart transmit
		bclr 	UMODE, #UARTEN		;disable uart
		bclr	IFS0bits, #URXIF	;clear uart received interupt flag
		bclr	IFS0bits, #UTXIF	;clear uart transmit interupt flag
		.endif
        bra 	usrapp				;change to jump code for first address


;------------------------------------------------------------------------------
; End of code
;------------------------------------------------------------------------------
.end
