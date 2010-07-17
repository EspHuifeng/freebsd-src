/*
 * Copyright (c) 2005, David Xu <davidxu@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libthr/thread/thr_cancel.c,v 1.16.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $
 *
 */

#include "namespace.h"
#include <pthread.h>
#include "un-namespace.h"

#include "thr_private.h"

__weak_reference(_pthread_cancel, pthread_cancel);
__weak_reference(_pthread_setcancelstate, pthread_setcancelstate);
__weak_reference(_pthread_setcanceltype, pthread_setcanceltype);
__weak_reference(_pthread_testcancel, pthread_testcancel);

static inline void
testcancel(struct pthread *curthread)
{
	if (__predict_false(SHOULD_CANCEL(curthread) &&
	    !THR_IN_CRITICAL(curthread) && curthread->cancel_defer == 0))
		_pthread_exit(PTHREAD_CANCELED);
}

void
_thr_testcancel(struct pthread *curthread)
{
	testcancel(curthread);
}

int
_pthread_cancel(pthread_t pthread)
{
	struct pthread *curthread = _get_curthread();
	int ret;

	/*
	 * POSIX says _pthread_cancel should be async cancellation safe.
	 * _thr_ref_add and _thr_ref_delete will enter and leave critical
	 * region automatically.
	 */
	if ((ret = _thr_ref_add(curthread, pthread, 0)) == 0) {
		THR_THREAD_LOCK(curthread, pthread);
		if (!pthread->cancel_pending) {
			pthread->cancel_pending = 1;
			if (pthread->cancel_enable)
				_thr_send_sig(pthread, SIGCANCEL);
		}
		THR_THREAD_UNLOCK(curthread, pthread);
		_thr_ref_delete(curthread, pthread);
	}
	return (ret);
}

int
_pthread_setcancelstate(int state, int *oldstate)
{
	struct pthread *curthread = _get_curthread();
	int oldval;

	oldval = curthread->cancel_enable;
	switch (state) {
	case PTHREAD_CANCEL_DISABLE:
		THR_LOCK(curthread);
		curthread->cancel_enable = 0;
		THR_UNLOCK(curthread);
		break;
	case PTHREAD_CANCEL_ENABLE:
		THR_LOCK(curthread);
		curthread->cancel_enable = 1;
		THR_UNLOCK(curthread);
		break;
	default:
		return (EINVAL);
	}

	if (oldstate) {
		*oldstate = oldval ? PTHREAD_CANCEL_ENABLE :
			PTHREAD_CANCEL_DISABLE;
	}
	return (0);
}

int
_pthread_setcanceltype(int type, int *oldtype)
{
	struct pthread	*curthread = _get_curthread();
	int oldval;

	oldval = curthread->cancel_async;
	switch (type) {
	case PTHREAD_CANCEL_ASYNCHRONOUS:
		curthread->cancel_async = 1;
		testcancel(curthread);
		break;
	case PTHREAD_CANCEL_DEFERRED:
		curthread->cancel_async = 0;
		break;
	default:
		return (EINVAL);
	}

	if (oldtype) {
		*oldtype = oldval ? PTHREAD_CANCEL_ASYNCHRONOUS :
		 	PTHREAD_CANCEL_DEFERRED;
	}
	return (0);
}

void
_pthread_testcancel(void)
{
	struct pthread *curthread = _get_curthread();

	_thr_cancel_enter(curthread);
	_thr_cancel_leave(curthread);
}

void
_thr_cancel_enter(struct pthread *curthread)
{
	if (curthread->cancel_enable) {
		curthread->cancel_point++;
		testcancel(curthread);
	}
}

void
_thr_cancel_leave(struct pthread *curthread)
{
	if (curthread->cancel_enable)
		curthread->cancel_point--;
}

void
_thr_cancel_enter_defer(struct pthread *curthread)
{
	if (curthread->cancel_enable) {
		curthread->cancel_point++;
		testcancel(curthread);
		curthread->cancel_defer++;
	}
}

void
_thr_cancel_leave_defer(struct pthread *curthread, int check)
{
	if (curthread->cancel_enable) {
		if (!check) {
			curthread->cancel_point--;
			curthread->cancel_defer--;
		} else {
			curthread->cancel_defer--;
			testcancel(curthread);
			curthread->cancel_point--;
		}
	}
}
