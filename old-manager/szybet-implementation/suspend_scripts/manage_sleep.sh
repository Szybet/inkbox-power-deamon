#!/bin/bash
# /tmp/sleep_status steps in chronological order:
# waiting - nothing, its just waiting
# preparing - its turning everything off, its needed to wait
# sleeping - its sleeping already. This mode is only for user information, not used in code
# restoring - after sleeping, restoring wifi for example

echo "waiting" > /tmp/sleep_status
### TO DO: avoid launching multiple scripts becouse restart doesn't kill it


while true; do
	inotifywait -e modify /tmp/power
	if grep -q "KEY_POWER" /tmp/power || grep -q "KEY_F1" /tmp/power; then
		> /tmp/power # Idk if its needed
		# If something happens, execute:
		# I want the code to be here that it isin't needed to relaunch this script
		# Testing only
		CURRENT_STATUS=$(cat /tmp/sleep_status 2>/dev/null)
		if [ "${CURRENT_STATUS}" == "waiting" ]; then
            		/etc/init.d/suspend_scripts/prepare_sleep.sh &
		elif [ "${CURRENT_STATUS}" == "preparing" ]; then
            		killall -9 prepare_sleep.sh 2> /dev/null
            		killall -9 go_sleep.sh 2> /dev/null
            		killall -9 cinematic-brightness.sh 2> /dev/null
            		/etc/init.d/suspend_scripts/after_sleep.sh &
        	elif [ "${CURRENT_STATUS}" == "sleeping" ]; then
            		# this will maybe cause going it to sleep but with restoring status?
            		killall -9 go_sleep.sh 2> /dev/null
            		killall -9 cinematic-brightness.sh 2> /dev/null
             		/etc/init.d/suspend_scripts/after_sleep.sh &
        	elif [ "${CURRENT_STATUS}" == "restoring" ]; then
            		killall -9 after_sleep.sh 2> /dev/null
            		killall -9 cinematic-brightness.sh 2> /dev/null
            		/etc/init.d/suspend_scripts/prepare_sleep.sh &
        fi
        sleep 1.5 # Avoid clicking the power button once, but triggering inotify 2 times
	else
		> /tmp/power # Idk if its needed
		echo "continue"
		continue
	fi
done
