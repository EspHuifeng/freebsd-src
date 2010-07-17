/*
 * Copyright (c) 1983, 1993
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
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getproto.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/net/getproto.c,v 1.7.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $");

#include <netdb.h>
#include <nsswitch.h>
#include "netdb_private.h"
#ifdef NS_CACHING
#include "nscache.h"
#endif
#include "nss_tls.h"

static const ns_src defaultsrc[] = {
	{ NSSRC_FILES, NS_SUCCESS },
	{ NULL, 0 }
};

#ifdef NS_CACHING
extern int __proto_id_func(char *, size_t *, va_list, void *);
extern int __proto_marshal_func(char *, size_t *, void *, va_list, void *);
extern int __proto_unmarshal_func(char *, size_t, void *, va_list, void *);
#endif

static int
files_getprotobynumber(void *retval, void *mdata, va_list ap)
{
	struct protoent pe;
	struct protoent_data *ped;
	int error;

	int number;
	struct protoent	*pptr;
	char *buffer;
	size_t buflen;
	int *errnop;

	number = va_arg(ap, int);
	pptr = va_arg(ap, struct protoent *);
	buffer = va_arg(ap, char *);
	buflen = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);

	if ((ped = __protoent_data_init()) == NULL) {
		*errnop = -1;
		return (NS_NOTFOUND);
	}

	__setprotoent_p(ped->stayopen, ped);
	while ((error = __getprotoent_p(&pe, ped)) == 0)
		if (pe.p_proto == number)
			break;
	if (!ped->stayopen)
		__endprotoent_p(ped);
	if (error != 0) {
		*errnop = -1;
		return (NS_NOTFOUND);
	}
	if (__copy_protoent(&pe, pptr, buffer, buflen) != 0) {
		*errnop = -1;
		return (NS_NOTFOUND);
	}

	*((struct protoent **)retval) = pptr;
	return (NS_SUCCESS);
}

int
getprotobynumber_r(int proto, struct protoent *pptr, char *buffer,
    size_t buflen, struct protoent **result)
{
#ifdef NS_CACHING
	static const nss_cache_info cache_info =
    		NS_COMMON_CACHE_INFO_INITIALIZER(
		protocols, (void *)nss_lt_id,
		__proto_id_func, __proto_marshal_func, __proto_unmarshal_func);
#endif

	static const ns_dtab dtab[] = {
		{ NSSRC_FILES, files_getprotobynumber, NULL },
#ifdef NS_CACHING
		NS_CACHE_CB(&cache_info)
#endif
		{ NULL, NULL, NULL }
	};
	int	rv, ret_errno;

	ret_errno = 0;
	*result = NULL;
	rv = nsdispatch(result, dtab, NSDB_PROTOCOLS, "getprotobynumber_r",
		defaultsrc, proto, pptr, buffer, buflen, &ret_errno);

	if (rv == NS_SUCCESS)
		return (0);
	else
		return (ret_errno);
}

struct protoent *
getprotobynumber(int proto)
{
	struct protodata *pd;
	struct protoent *rval;

	if ((pd = __protodata_init()) == NULL)
		return (NULL);
	if (getprotobynumber_r(proto, &pd->proto, pd->data, sizeof(pd->data),
	    &rval) != 0)
		return (NULL);
	return (rval);
}
