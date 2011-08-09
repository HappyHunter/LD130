;------------------------------------------------------------------------------
;
; Title:			ds30 Loader for dsPIC30F
;
; File description:	user settings and configuration bits
;
; Copyright: 		Copyright © 09-10, Mikael Gustafsson
;
; Webpage: 			http://mrmackey.no-ip.org/elektronik/ds30loader/
;
; History:			3.0.0 New settings: CAN stuff
;					2.1.0 New setting: auto baudrate detection
;					2.0.4 New setting: goto protection
;						  New setting: bootloader protection;
;					2.0.3 Added tx enable setting
;					2.0.2 -
;					2.0.1 -
;					2.0.0 -
;					1.0.1 -
;					1.0.0 -
;					0.9.4 -
;					0.9.3 first version of this file
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
; Device
;------------------------------------------------------------------------------
        .equ 	__30F6010A, 	1				;xxx device


;------------------------------------------------------------------------------
; Includes
;------------------------------------------------------------------------------
        .include	"devices.inc"


;------------------------------------------------------------------------------
; User preferences
;------------------------------------------------------------------------------
		;.equ	DEV_MODE,	1					;xxx comment or delete this line
;		.equ 	FCY, 		15000000			;xxx speed of internal cycle clock[Hz], used to calculate uart brg and delay
;		config 	__FOSC, 	CSW_FSCM_OFF & XT_PLL8	;xxx oscillator


		.equ 	FCY, 		30000000			;xxx speed of internal cycle clock[Hz], used to calculate uart brg and delay
		config 	__FOSC, 	CSW_FSCM_OFF & FRC_PLL16	;xxx oscillator

;		.equ 	BAUDRATE, 	9600				;xxx baudrate
		.equ 	BAUDRATE, 	115200				;xxx baudrate
		.equ	BLTIME,		2000				;xxx time in ms before bootloader loads the user application

       	.equ    USE_UART1,	1					;xxx uncomment to use uart1
       	;.equ	USE_UART2,	1					;xxx uncomment to use uart2
        ;.equ	USE_CAN1,	1					;xxx uncomment to use CAN1
        ;.equ	USE_CAN2,	1					;xxx uncomment to use CAN2, not implemented


;------------------------------------------------------------------------------
; UART settings
;------------------------------------------------------------------------------
		;;.equ	USE_ALTIO,	1					;xxx uncomment to use uart 1 alternative i/o
        ;.equ	USE_ABAUD,	1					;xxx uncomment to use auto baud rate detection, READ ERRATA FIRST and check for problems

        ;.equ	USE_TXENABLE,	1				;xxx uncomment to use a tx enable pin
		.ifdef USE_TXENABLE
			.equ	TRISR_TXE,	TRISD			;xxx tris register containing tx enable
			.equ 	LATR_TXE,	LATD			;xxx port register containing tx enable
			.equ	TRISB_TXE,	TRISD0			;xxx tris bit for tx enable
			.equ	LATB_TXE,	RD0				;xxx port bit for tx enable
		.endif


;------------------------------------------------------------------------------
; CAN settings
;------------------------------------------------------------------------------
; Baudrate table, not confirmed
;
;	30MIPS	   BRP    PPROP	   SEG1	   SEG2
;	1M			1	     3		 6	     5
;	500k		2        3       6       5
;	250k		4		 3	     6		 5
;	125k	    8		 3		 6		 5
;	100k	   10 	     3       6       5
;
;------------------------------------------------------------------------------
		.equ	ID_PIC,			291				;xxx node number for this device
		.equ 	ID_GUI, 		2				;xxx node number of the ds30 Loader gui

		.equ	CAN_BRP,		4				;xxx baud rate prescalar, should be 1-64
		.equ	CAN_PROP,		3				;xxx propagation, should be 1-8
		.equ	CAN_SEG1,		6				;xxx segment 1, should be 1-8
		.equ	CAN_SEG2,		5				;xxx segment 2, should be 1-8, should be less than prop+seg1
		.equ	CAN_SJW,		3				;xxx sjw, should be 1-4


;------------------------------------------------------------------------------
; Advanced settings
;------------------------------------------------------------------------------
		.equ	PROT_GOTO,		1				;xxx protect goto at 0x00
		.equ 	PROT_BL, 		1				;xxx protect bootloader

		.equ	BLPLR,			8				;bootloader placement, rows from end
		.equ	BLSIZER,		8				;bootloader size [rows]


;------------------------------------------------------------------------------
; Configuration bits, these macros can be found at the end of the inc-files located in
; c:\Program files\Microchip\MPLAB ASM30 Suite\Support\dsPIC30F\inc
;
; These can also be set in MPLAB IDE instead, they are found in Configure->Configuration bits...
;------------------------------------------------------------------------------
		config 	__FWDT, 	WDT_OFF						;xxx watchdog
;		config 	__FBORPOR,	PBOR_OFF & MCLR_EN			;xxx power-on reset, brown-out reset, master clear
;		config 	__FGS, 		CODE_PROT_OFF 				;xxx code protection
		;config 	__FICD, 	ICS_NONE					;xxx comm channel select
