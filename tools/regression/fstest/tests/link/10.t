#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/link/10.t,v 1.1.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $

desc="link returns EEXIST if the destination file does exist"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..14"

n0=`namegen`
n1=`namegen`

expect 0 create ${n0} 0644

expect 0 create ${n1} 0644
expect EEXIST link ${n0} ${n1}
expect 0 unlink ${n1}

expect 0 mkdir ${n1} 0755
expect EEXIST link ${n0} ${n1}
expect 0 rmdir ${n1}

expect 0 symlink test ${n1}
expect EEXIST link ${n0} ${n1}
expect 0 unlink ${n1}

expect 0 mkfifo ${n1} 0644
expect EEXIST link ${n0} ${n1}
expect 0 unlink ${n1}

expect 0 unlink ${n0}
