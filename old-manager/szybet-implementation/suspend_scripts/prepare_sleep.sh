#!/bin/bash

LOCKSCREEN=$(cat /opt/config/12-lockscreen/config 2>/dev/null)
DEVICE=$(cat /opt/inkbox_device)

#echo "preparing" > /tmp/sleep_status

sleep 1
chroot /kobo /usr/bin/fbgrab "/external_root/tmp/dump.png"

/opt/bin/fbink/fbink -k -f -q
/opt/bin/fbink/fbink -t regular=/etc/init.d/splash.d/fonts/resources/inter-b.ttf,size=20 "Sleeping" -m -M -q

echo "true" > /tmp/sleep_standby # Idk if its needed anymore
> /tmp/power

echo "true" > /tmp/sleep_mode # Idk if its needed anymore


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
if [ "${DEVICE}" != "n613" ]; then 
	# /tmp/savedBrightness is deleted in after_sleep.sh. The if is for avoiding restoring wrong brightness in after_sleep.sh
	if [[ ! -f "/tmp/savedBrightness" ]]; then
		CURRENT_BRIGHTNESS=$(cat /kobo/var/run/brightness)
		echo "${CURRENT_BRIGHTNESS}" > /tmp/savedBrightness
	fi
else
	if [[ ! -f "/tmp/savedBrightness" ]]; then
		CURRENT_BRIGHTNESS=$(cat /opt/config/03-brightness/config)
		echo "${CURRENT_BRIGHTNESS}" > /tmp/savedBrightness
	fi
fi
/opt/bin/cinematic-brightness.sh 0 1

if [ -d "/sys/class/net/${WIFI_DEV}" ]; then
	if [ "${DEVICE}" == "n873" ] || [ "${DEVICE}" == "n236" ] || [ "${DEVICE}" == "n306" ]; then
		WIFI_MODULE="/modules/wifi/8189fs.ko"
		SDIO_WIFI_PWR_MODULE="/modules/drivers/mmc/card/sdio_wifi_pwr.ko"
		WIFI_DEV="eth0"
	elif [ "${DEVICE}" == "n705" ] || [ "${DEVICE}" == "n905b" ] || [ "${DEVICE}" == "n905c" ] || [ "${DEVICE}" == "n613" ]; then
		WIFI_MODULE="/modules/dhd.ko"
		SDIO_WIFI_PWR_MODULE="/modules/sdio_wifi_pwr.ko"
		WIFI_DEV="eth0"
	elif [ "${DEVICE}" == "n437" ]; then
		WIFI_MODULE="/modules/wifi/bcmdhd.ko"
		SDIO_WIFI_PWR_MODULE="/modules/drivers/mmc/card/sdio_wifi_pwr.ko"
		WIFI_DEV="wlan0"
	else
		WIFI_MODULE="/modules/dhd.ko"
		SDIO_WIFI_PWR_MODULE="/modules/sdio_wifi_pwr.ko"
		WIFI_DEV="eth0"
	fi

	# Checking if we have a fully configured Wi-Fi interface
	# same as for /tmp/savedBrightness, avoid confussion
	if [[ ! -f "/run/was_connected_to_wifi" ]]; then
		if grep -q "up" "/sys/class/net/${WIFI_DEV}/operstate"; then
			echo "true" > /run/was_connected_to_wifi
		fi
	fi
	
	# killall -q dhcpcd wpa_supplicant
	# Uncomment this later
	ifconfig "${WIFI_DEV}" down 2>/dev/null
	if [ "${DEVICE}" == "n705" ] || [ "${DEVICE}" == "n905b" ] || [ "${DEVICE}" == "n905c" ] || [ "${DEVICE}" == "n613" ] || [ "${DEVICE}" == "n437" ]; then
		wlarm_le down
	fi
	rmmod "${WIFI_MODULE}" 2> /dev/null
	rmmod "${SDIO_WIFI_PWR_MODULE}" 2> /dev/null
fi

echo "false" > /kobo/inkbox/remount
> /tmp/power

/etc/init.d/suspend_scripts/go_sleep.sh
