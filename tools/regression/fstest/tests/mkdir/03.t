#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/mkdir/03.t,v 1.1.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $

desc="mkdir returns ENAMETOOLONG if an entire path name exceeded 1023 characters"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..11"

expect 0 mkdir ${name255} 0755
expect 0 mkdir ${name255}/${name255} 0755
expect 0 mkdir ${name255}/${name255}/${name255} 0755
expect 0 mkdir ${path1021} 0755
expect 0 mkdir ${path1023} 0755
expect 0 rmdir ${path1023}
expect ENAMETOOLONG mkdir ${path1024} 0755
expect 0 rmdir ${path1021}
expect 0 rmdir ${name255}/${name255}/${name255}
expect 0 rmdir ${name255}/${name255}
expect 0 rmdir ${name255}
