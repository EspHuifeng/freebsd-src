/*-
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "opt_inet.h"
#include "opt_inet6.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/contrib/pf/net/pf_subr.c,v 1.11.2.1.4.1 2010/06/14 02:09:06 kensmith Exp $");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/libkern.h>
#include <sys/mbuf.h>
#include <sys/md5.h>
#include <sys/time.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/systm.h>
#include <sys/time.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/bpf.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_seq.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/if_ether.h>
#include <net/pfvar.h>

/*
 * Following is where TCP initial sequence number generation occurs.
 *
 * There are two places where we must use initial sequence numbers:
 * 1.  In SYN-ACK packets.
 * 2.  In SYN packets.
 *
 * All ISNs for SYN-ACK packets are generated by the syncache.  See
 * tcp_syncache.c for details.
 *
 * The ISNs in SYN packets must be monotonic; TIME_WAIT recycling
 * depends on this property.  In addition, these ISNs should be
 * unguessable so as to prevent connection hijacking.  To satisfy
 * the requirements of this situation, the algorithm outlined in
 * RFC 1948 is used, with only small modifications.
 *
 * Implementation details:
 *
 * Time is based off the system timer, and is corrected so that it
 * increases by one megabyte per second.  This allows for proper
 * recycling on high speed LANs while still leaving over an hour
 * before rollover.
 *
 * As reading the *exact* system time is too expensive to be done
 * whenever setting up a TCP connection, we increment the time
 * offset in two ways.  First, a small random positive increment
 * is added to isn_offset for each connection that is set up.
 * Second, the function tcp_isn_tick fires once per clock tick
 * and increments isn_offset as necessary so that sequence numbers
 * are incremented at approximately ISN_BYTES_PER_SECOND.  The
 * random positive increments serve only to ensure that the same
 * exact sequence number is never sent out twice (as could otherwise
 * happen when a port is recycled in less than the system tick
 * interval.)
 *
 * net.inet.tcp.isn_reseed_interval controls the number of seconds
 * between seeding of isn_secret.  This is normally set to zero,
 * as reseeding should not be necessary.
 *
 * Locking of the global variables isn_secret, isn_last_reseed, isn_offset,
 * isn_offset_old, and isn_ctx is performed using the TCP pcbinfo lock.  In
 * general, this means holding an exclusive (write) lock.
 */

#define ISN_BYTES_PER_SECOND 1048576
#define ISN_STATIC_INCREMENT 4096
#define ISN_RANDOM_INCREMENT (4096 - 1)

static u_char pf_isn_secret[32];
static int pf_isn_last_reseed;
static u_int32_t pf_isn_offset;

u_int32_t
pf_new_isn(struct pf_state *s)
{
	MD5_CTX isn_ctx;
	u_int32_t md5_buffer[4];
	u_int32_t new_isn;
	struct pf_state_host *src, *dst;

	/* Seed if this is the first use, reseed if requested. */
	if (pf_isn_last_reseed == 0) {
		read_random(&pf_isn_secret, sizeof(pf_isn_secret));
		pf_isn_last_reseed = ticks;
	}

	if (s->direction == PF_IN) {
		src = &s->ext;
		dst = &s->gwy;
	} else {
		src = &s->lan;
		dst = &s->ext;
	}

	/* Compute the md5 hash and return the ISN. */
	MD5Init(&isn_ctx);
	MD5Update(&isn_ctx, (u_char *) &dst->port, sizeof(u_short));
	MD5Update(&isn_ctx, (u_char *) &src->port, sizeof(u_short));
#ifdef INET6
	if (s->af == AF_INET6) {
		MD5Update(&isn_ctx, (u_char *) &dst->addr,
			  sizeof(struct in6_addr));
		MD5Update(&isn_ctx, (u_char *) &src->addr,
			  sizeof(struct in6_addr));
	} else
#endif
	{
		MD5Update(&isn_ctx, (u_char *) &dst->addr,
			  sizeof(struct in_addr));
		MD5Update(&isn_ctx, (u_char *) &src->addr,
			  sizeof(struct in_addr));
	}
	MD5Update(&isn_ctx, (u_char *) &pf_isn_secret, sizeof(pf_isn_secret));
	MD5Final((u_char *) &md5_buffer, &isn_ctx);
	new_isn = (tcp_seq) md5_buffer[0];
	pf_isn_offset += ISN_STATIC_INCREMENT +
		(arc4random() & ISN_RANDOM_INCREMENT);
	new_isn += pf_isn_offset;
	return (new_isn);
}
