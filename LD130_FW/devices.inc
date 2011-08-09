;------------------------------------------------------------------------------
;
; Title:			ds30 Loader for dsPIC30F
;
; File description:	device specific constants
;
; Copyright: 		Copyright © 09-10, Mikael Gustafsson
;
; Webpage: 			http://mrmackey.no-ip.org/elektronik/ds30loader/
;
; History:			3.0.0 Improved CAN defines
;					2.1.0 -
;					2.0.4 -
;					2.0.3 -
;					2.0.2 Fixed incorrect flashsize of 50xx devices
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
;    along with ds30 Loader. If not, see <http:;www.gnu.org/licenses/>.
;------------------------------------------------------------------------------ .ifdef	__30F1010
.ifdef	__30F1010
	.include	"p30F1010.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x1000
	.equ		DEVICEID,		30
.endif
.ifdef	__30F2010
	.include	"p30F2010.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x2000
	.equ		DEVICEID,		10
.endif
.ifdef	__30F2011
	.include	"p30F2011.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x2000
	.equ		DEVICEID,		20
.endif
.ifdef	__30F2012
	.include	"p30F2012.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x2000
	.equ		DEVICEID,		22
.endif
.ifdef	__30F2020
	.include	"p30F2020.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x2000
	.equ		DEVICEID,		31
.endif
.ifdef	__30F2023
	.include	"p30F2023.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x2000
	.equ		DEVICEID,		32
.endif
.ifdef	__30F3010
	.include	"p30F3010.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x4000
	.equ		DEVICEID,		11
.endif
.ifdef	__30F3011
	.include	"p30F3011.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x4000
	.equ		DEVICEID,		13
	.equ		HAS_UART2,		1
.endif
.ifdef	__30F3012
	.include	"p30F3012.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x4000
	.equ		DEVICEID,		21
.endif
.ifdef	__30F3013
	.include	"p30F3013.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x4000
	.equ		DEVICEID,		23
	.equ		HAS_UART2,		1
.endif
.ifdef	__30F3014
	.include	"p30F3014.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x4000
	.equ		DEVICEID,		1
	.equ		HAS_UART2,		1
.endif
.ifdef	__30F4011
	.include	"p30F4011.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x8000
	.equ		DEVICEID,		14
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
.endif
.ifdef	__30F4012
	.include	"p30F4012.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x8000
	.equ		DEVICEID,		12
	.equ		HAS_CAN1,		1
.endif
.ifdef	__30F4013
	.include	"p30F4013.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x8000
	.equ		DEVICEID,		2
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
.endif
.ifdef	__30F5011
	.include	"p30F5011.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0xB000
	.equ		DEVICEID,		3
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F5013
	.include	"p30F5013.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0xB000
	.equ		DEVICEID,		6
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F5015
	.include	"p30F5015.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0xB000
	.equ		DEVICEID,		15
	.equ		HAS_CAN1,		1
.endif
.ifdef	__30F5016
	.include	"p30F5016.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0xB000
	.equ		DEVICEID,		33
	.equ		HAS_CAN1,		1
.endif
.ifdef	__30F6010
	.include	"p30F6010.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x18000
	.equ		DEVICEID,		16
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F6011
	.include	"p30F6011.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x16000
	.equ		DEVICEID,		4
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F6012
	.include	"p30F6012.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x18000
	.equ		DEVICEID,		5
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F6013
	.include	"p30F6013.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x16000
	.equ		DEVICEID,		7
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F6014
	.include	"p30F6014.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x18000
	.equ		DEVICEID,		8
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F6015
	.include	"p30F6015.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x18000
	.equ		DEVICEID,		34
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
.endif
.ifdef	__30F6010A
	.include	"p30F6010A.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x18000
	.equ		DEVICEID,		24
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F6011A
	.include	"p30F6011A.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x16000
	.equ		DEVICEID,		25
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F6012A
	.include	"p30F6012A.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x18000
	.equ		DEVICEID,		26
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F6013A
	.include	"p30F6013A.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x16000
	.equ		DEVICEID,		27
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif
.ifdef	__30F6014A
	.include	"p30F6014A.inc"
	.equ		VALID_DEV,		1
	.equ		FLASHSIZE,		0x18000
	.equ		DEVICEID,		28
	.equ		HAS_UART2,		1
	.equ		HAS_CAN1,		1
	.equ		HAS_CAN2,		1
.endif


;------------------------------------------------------------------------------
;
;-----------------------------------------------------------------------------
.ifndef VALID_DEV
	.error "Unknown device specified"
.endif
