#!/bin/bash
# /tmp/sleep_error stores errors when going to sleep. For kobo nia, if missing permission error will show up it will try again
DEVICE=$(cat /opt/inkbox_device)

# Race condition; going to sleep
echo "false" > /tmp/sleep_standby # Idk if its needed
echo "sleeping" > /tmp/sleep_status

go_sleep() {
    rm -rf /tmp/sleep_error 2>/dev/null
    sleep 9
    echo "1" > /sys/power/state-extended
    sleep 2
    echo "mem" > /sys/power/state 2>/tmp/sleep_error 
    sleep 2
}

# Only for testing
sync
go_sleep

if [ "${DEVICE}" == "n306" ]; then
    # Separate if to avoid writing many times to the e-ink. idk about the syntax / look of the code here
    if grep -q "Operation not permitted" /tmp/sleep_error; then
        /opt/bin/fbink/fbink -m -q -B WHITE -t regular=/etc/init.d/splash.d/fonts/resources/inter-b.ttf,size=18,top=800,bottom=0,left=0,right=0 "don't touch the screen"
        /opt/bin/fbink/fbink -m -q -B WHITE -t regular=/etc/init.d/splash.d/fonts/resources/inter-b.ttf,size=14,top=870,bottom=0,left=0,right=0 "Nia can't go to sleep if it's touched"
    fi
    while grep -q "Operation not permitted" /tmp/sleep_error; do
        /etc/init.d/suspend_scripts/touch_issue_fix.sh
        sync
        sleep 5
        go_sleep
    done
fi

# only for testing
/etc/init.d/suspend_scripts/after_sleep.sh
