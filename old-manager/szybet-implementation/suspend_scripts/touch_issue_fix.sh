#!/bin/bash 
# This file fixes suspend issues related with touch on the kobo nia

LOCKSCREEN=$(cat /opt/config/12-lockscreen/config 2>/dev/null)

kill -CONT $(pidof inkbox-bin 2>/dev/null) 2>/dev/null
kill -CONT $(pidof oobe-inkbox-bin 2>/dev/null) 2>/dev/null
kill -CONT $(pidof calculator-bin 2>/dev/null) 2>/dev/null
kill -CONT $(pidof scribble 2>/dev/null) 2>/dev/null
kill -CONT $(pidof lightmaps 2>/dev/null) 2>/dev/null
kill -CONT $(pidof qreversi-bin 2>/dev/null) 2>/dev/null
kill -CONT $(pidof 2048-bin 2>/dev/null) 2>/dev/null

sleep 2

if [ "${LOCKSCREEN}" == "true" ]; then
	killall -q inkbox-bin
	killall -q oobe-inkbox-bin
	killall -q lockscreen-bin
	killall -q calculator-bin
	killall -q qreversi-bin
	killall -q 2048-bin
	killall -q scribble
	killall -q lightmaps
else
	kill -STOP $(pidof inkbox-bin 2>/dev/null) 2>/dev/null
	kill -STOP $(pidof oobe-inkbox-bin 2>/dev/null) 2>/dev/null
	kill -9 $(pidof lockscreen-bin 2>/dev/null) 2>/dev/null
	kill -STOP $(pidof calculator-bin 2>/dev/null) 2>/dev/null
	kill -STOP $(pidof scribble 2>/dev/null) 2>/dev/null
	kill -STOP $(pidof lightmaps 2>/dev/null) 2>/dev/null
	kill -STOP $(pidof qreversi-bin 2>/dev/null) 2>/dev/null
	kill -STOP $(pidof 2048-bin 2>/dev/null) 2>/dev/null
fi

sleep 1

/opt/bin/fbink/fbink -m -q -B WHITE -t regular=/etc/init.d/splash.d/fonts/resources/inter-b.ttf,size=18,top=800,bottom=0,left=0,right=0 "don't touch the screen"
/opt/bin/fbink/fbink -m -q -B WHITE -t regular=/etc/init.d/splash.d/fonts/resources/inter-b.ttf,size=14,top=870,bottom=0,left=0,right=0 "Nia can't go to sleep if it's touched"
/opt/bin/fbink/fbink -m -q -B WHITE -t regular=/etc/init.d/splash.d/fonts/resources/inter-b.ttf,size=13,top=930,bottom=0,left=0,right=0 "Retrying"
