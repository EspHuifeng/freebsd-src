/*
 * lifted from fs/ncpfs/getopt.c
 *
 * $FreeBSD: src/sys/contrib/rdma/krping/getopt.h,v 1.1.4.1.4.1 2010/06/14 02:09:06 kensmith Exp $
 */
#ifndef _KRPING_GETOPT_H
#define _KRPING_GETOPT_H

#define OPT_NOPARAM	1
#define OPT_INT		2
#define OPT_STRING	4
struct krping_option {
	const char *name;
	unsigned int has_arg;
	int val;
};

extern int krping_getopt(const char *caller, char **options, const struct krping_option *opts,
		      char **optopt, char **optarg, unsigned long *value);

#endif /* _KRPING_GETOPT_H */
