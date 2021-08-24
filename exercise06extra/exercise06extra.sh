#!/bin/bash

led_pins=(20 21 16)
finish_script=0

run_led_blinking(){
  echo "$$ $!"
  local current_active=0
  until (( finish_script == 1 )); do
    
    for (( i = 0; i < 3; i++ )); do
      if (( current_active == i )); then
        echo 1 > /sys/class/gpio/gpio"${led_pins[$i]}"/value
      else
        echo 0 > /sys/class/gpio/gpio"${led_pins[$i]}"/value
      fi
    done
    if (( current_active == 2 )); then current_active=0; else ((current_active++)); fi
    sleep 0.5
  done
}

setup_button(){
  if [ -e "/sys/class/gpio/gpio26" ]; then echo 26 > /sys/class/gpio/unexport; fi
  echo 26 > /sys/class/gpio/export
  echo high > /sys/class/gpio/gpio26/direction
  echo 1 > /sys/class/gpio/gpio26/active_low
  echo "Button is ready !!"
}

setup_led(){
  if [ -e "/sys/class/gpio/gpio16" ]; then echo 16 > /sys/class/gpio/unexport; fi
  if [ -e "/sys/class/gpio/gpio20" ]; then echo 20 > /sys/class/gpio/unexport; fi
  if [ -e "/sys/class/gpio/gpio21" ]; then echo 21 > /sys/class/gpio/unexport; fi

  echo 16 > /sys/class/gpio/export
  echo out > /sys/class/gpio/gpio16/direction
  echo 1 > /sys/class/gpio/gpio16/value

  echo 20 > /sys/class/gpio/export
  echo out > /sys/class/gpio/gpio20/direction
  echo 1 > /sys/class/gpio/gpio20/value

  echo 21 > /sys/class/gpio/export
  echo out > /sys/class/gpio/gpio21/direction
  echo 1 > /sys/class/gpio/gpio21/value

  echo "Led is ready !!"
}

shutdown_led(){
  echo 0 > /sys/class/gpio/gpio16/value
  echo 16 > /sys/class/gpio/unexport

  echo 0 > /sys/class/gpio/gpio20/value
  echo 20 > /sys/class/gpio/unexport

  echo 0 > /sys/class/gpio/gpio21/value
  echo 21 > /sys/class/gpio/unexport
}


setup_button
while true; do
	if (( 1 == $(cat /sys/class/gpio/gpio26/value) )); then break; fi
	sleep 0.05
done
setup_led
sleep 1

current_button_state=0
previous_button_state=0
duration=0

run_led_blinking

while true; do
	current_button_state=$(cat /sys/class/gpio/gpio26/value)
	if (( current_button_state == 1)); then
	  if ((previous_button_state == 0)); then
	    previous_button_state=1
	    if (( led_pins[0] == 16)); then
	        export led_pins=(20 21 16)
	      else
	        export led_pins=(16 21 20)
	      fi
    else
      ((duration++))
      if (( duration >= 20 )); then
        export finish_script=1
        break;
      fi
    fi
	else
	  duration=0
	  previous_button_state=0
	fi
	sleep 0.05
done

echo 26 > /sys/class/gpio/unexport
shutdown_led