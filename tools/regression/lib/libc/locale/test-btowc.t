#!/bin/sh
# $FreeBSD: src/tools/regression/lib/libc/locale/test-btowc.t,v 1.1.22.1.4.1 2010/06/14 02:09:06 kensmith Exp $

cd `dirname $0`

executable=`basename $0 .t`

make $executable 2>&1 > /dev/null

exec ./$executable
