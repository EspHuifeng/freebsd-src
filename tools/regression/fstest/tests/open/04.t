#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/open/04.t,v 1.1.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $

desc="open returns ENOENT if a component of the path name that must exist does not exist or O_CREAT is not set and the named file does not exist"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..4"

n0=`namegen`
n1=`namegen`

expect 0 mkdir ${n0} 0755
expect ENOENT open ${n0}/${n1}/test O_CREAT 0644
expect ENOENT open ${n0}/${n1} O_RDONLY
expect 0 rmdir ${n0}
