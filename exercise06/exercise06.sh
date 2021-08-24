#!/bin/bash

if [ -e "/sys/class/gpio/gpio26" ]; then
    echo "GPIO26 was active ! "
    echo 26 > /sys/class/gpio/unexport
fi

echo 26 > /sys/class/gpio/export
echo high > /sys/class/gpio/gpio26/direction
echo 1 > /sys/class/gpio/gpio26/active_low
echo "GPIO26 is ready !!"


current_state=0
previous_state=0
times=0

while true; do
	current_state=$(cat /sys/class/gpio/gpio26/value)
	if (( current_state == 1)); then
	  if ((previous_state == 0)); then
	    previous_state=1
	    ((times++))
      echo "Clicked times $times"
	  fi
	else
	  previous_state=0
	fi
	sleep 0.05
done