/*	$FreeBSD: src/contrib/ipfilter/lib/kmemcpywrap.c,v 1.3.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $	*/

/*
 * Copyright (C) 2002 by Darren Reed.
 * 
 * See the IPFILTER.LICENCE file for details on licencing.  
 *   
 * $Id: kmemcpywrap.c,v 1.1.4.1 2006/06/16 17:21:05 darrenr Exp $ 
 */     

#include "ipf.h"
#include "kmem.h"

int kmemcpywrap(from, to, size)
void *from, *to;
size_t size;
{
	int ret;

	ret = kmemcpy((caddr_t)to, (u_long)from, size);
	return ret;
}

