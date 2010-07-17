#!/bin/sh
# $FreeBSD: src/tools/regression/usr.bin/pkill/pgrep-i.t,v 1.1.22.1.4.1 2010/06/14 02:09:06 kensmith Exp $

base=`basename $0`

echo "1..1"

name="pgrep -i"
sleep=`mktemp /tmp/$base.XXXXXX` || exit 1
usleep="${sleep}XXX"
touch $usleep
lsleep="${sleep}xxx"
ln -sf /bin/sleep $usleep
$usleep 5 &
sleep 0.3
chpid=$!
pid=`pgrep -f -i $lsleep`
if [ "$pid" = "$chpid" ]; then
	echo "ok - $name"
else
	echo "not ok - $name"
fi
kill $chpid
rm -f $sleep $usleep
