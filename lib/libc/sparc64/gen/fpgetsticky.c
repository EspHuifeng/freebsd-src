/*	$NetBSD: fpgetsticky.c,v 1.2 2002/01/13 21:45:50 thorpej Exp $	*/

/*
 * Written by J.T. Conklin, Apr 10, 1995
 * Public domain.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/sparc64/gen/fpgetsticky.c,v 1.1.36.1.4.1 2010/06/14 02:09:06 kensmith Exp $");

#include <machine/fsr.h>
#include <ieeefp.h>

fp_except_t
fpgetsticky()
{
	unsigned int x;

	__asm__("st %%fsr,%0" : "=m" (x));
	return (FSR_GET_AEXC(x));
}
