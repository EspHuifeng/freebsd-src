/*
 * Copyright (c) 1995-1998 John Birrell <jb@cimlogic.com.au>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JOHN BIRRELL AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libc_r/uthread/uthread_shutdown.c,v 1.12.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $
 */
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include "pthread_private.h"

__weak_reference(_shutdown, shutdown);

int
_shutdown(int fd, int how)
{
	int             ret;

	switch (how) {
	case 0:
			if ((ret = _FD_LOCK(fd, FD_READ, NULL)) == 0) {
			ret = __sys_shutdown(fd, how);
			_FD_UNLOCK(fd, FD_READ);
		}
		break;
	case 1:
			if ((ret = _FD_LOCK(fd, FD_WRITE, NULL)) == 0) {
			ret = __sys_shutdown(fd, how);
			_FD_UNLOCK(fd, FD_WRITE);
		}
		break;
	case 2:
			if ((ret = _FD_LOCK(fd, FD_RDWR, NULL)) == 0) {
			ret = __sys_shutdown(fd, how);
			_FD_UNLOCK(fd, FD_RDWR);
		}
		break;
	default:
		errno =  EBADF;
		ret = -1;
		break;
	}
	return (ret);
}
