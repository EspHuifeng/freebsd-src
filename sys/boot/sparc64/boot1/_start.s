/* $FreeBSD: src/sys/boot/sparc64/boot1/_start.s,v 1.2.30.1.4.1 2010/06/14 02:09:06 kensmith Exp $ */

	.text
	.globl	_start
_start:
	call	ofw_init
	 nop
	sir
