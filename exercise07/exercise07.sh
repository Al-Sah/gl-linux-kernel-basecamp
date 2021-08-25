#!/bin/bash


if [ -e "/sys/class/gpio/gpio26" ]; then echo 26 > /sys/class/gpio/unexport; fi
echo 26 > /sys/class/gpio/export
echo high > /sys/class/gpio/gpio26/direction
echo 1 > /sys/class/gpio/gpio26/active_low
echo "GPIO26 is ready !!"; echo ""

current_state=0
previous_state=0
duration=0

while true; do
	current_state=$(cat /sys/class/gpio/gpio26/value)
	if (( current_state == 1)); then
	  if ((previous_state == 0)); then
	    previous_state=1
	    echo "Time: $(i2cget -y 1 0x68 0x02) $(i2cget -y 1 0x68 0x01) $(i2cget -y 1 0x68 0x00)"
	    echo "Temperature: $(i2cget -y 1 0x68 0x11)"; echo ""
    else
      ((duration++))
      if (( duration >= 20 )); then
        break;
      fi
    fi
	else
	  duration=0
	  previous_state=0
	fi
	sleep 0.05
done

echo 26 > /sys/class/gpio/unexport