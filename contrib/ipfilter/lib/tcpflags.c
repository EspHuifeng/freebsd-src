/*	$FreeBSD: src/contrib/ipfilter/lib/tcpflags.c,v 1.4.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $	*/

/*
 * Copyright (C) 2001-2002 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: tcpflags.c,v 1.3.4.1 2006/06/16 17:21:17 darrenr Exp $
 */

#include "ipf.h"


/*
 * ECN is a new addition to TCP - RFC 2481
 */
#ifndef TH_ECN
# define	TH_ECN  0x40
#endif
#ifndef TH_CWR
# define	TH_CWR  0x80
#endif

extern	char	flagset[];
extern	u_char	flags[];


u_char tcpflags(flgs)
char *flgs;
{
	u_char tcpf = 0;
	char *s, *t;

	for (s = flgs; *s; s++) {
		if (*s == 'W')
			tcpf |= TH_CWR;
		else {
			if (!(t = strchr(flagset, *s))) {
				return 0;
			}
			tcpf |= flags[t - flagset];
		}
	}
	return tcpf;
}
