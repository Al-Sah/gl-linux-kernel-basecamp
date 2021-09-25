#! /bin/bash


current_dir="$(pwd)"
target="time_manager"

sys_settings="/sys/class/${target}/settings"
proc_timestamps="/proc/${target}/timestamps"
proc_last_access="/proc/${target}/last_access"
proc_set_time="/proc/${target}/time_controller"


# output messages
start="$(tput bold)"
end="$(tput sgr0)"
subtest="$start * subtest "
test_info="$(tput setaf 6) + test:"
passed="$(tput setaf 2) PASSED $end"
failed="$(tput setaf 1) FAILED $end"
red="$(tput setaf 1)"


run_test_comparison(){
  if [[ "$1" == "$2" ]]; then
    echo "$subtest $5 $passed"
    echo "  $3"
  else
    echo "$subtest $5 $failed"
    echo "  $4"
  fi
  echo "-------------------------------------"
}

sudo dmesg -C &> /dev/null


if [[ -f "$(pwd)/Makefile" ]]; then
  make all &> "$current_dir/make-all.log"
  echo "$test_info run 'make all. See log at make-all.log$end"

  if [[ ! -e "${current_dir}/build/${target}.ko" ]]; then
    echo "$red Probably build failed ... 'build/${target}.ko' is not accessible $end";exit 1
  else
    module_dir="${current_dir}/build/"
  fi

else
  if [[ ! -e "${current_dir}/${target}.ko" ]]; then
      echo "$red Failed to find ${target}.ko $end";exit 1
  else
    module_dir="${current_dir}"
  fi
fi

echo "$test_info Module location: ${module_dir}/${target}.ko $end"
chmod ugo+rwx "${module_dir}/${target}.ko"


# module loading
if sudo lsmod | grep -q ${target}; then sudo rmmod ${target}.ko; fi
sudo insmod "${module_dir}/${target}.ko"

if sudo lsmod | grep -q ${target}; then
  echo "$test_info Module loaded successfully $end"
else
  echo "$red Failed to load module $end"; exit 1
fi






if [[ -e "$sys_settings" ]] && [[ -e "$proc_timestamps" ]] && [[ -e "$proc_last_access" ]] && [[ -e "$proc_set_time" ]]; then
  echo "$test_info all interfaces are exists $end"
else
  echo "$red interfaces are not exists $end"
  exit 1
fi


test_number=1
# -------------------------------------------------------------------------------------
src="$(cat $proc_last_access)"
res="First time accessing file"
failure_msg="Output is '$src', but had to be: '$res'"
success_msg="Output is: '$src'"
run_test_comparison "$src" "$res" "$success_msg" "$failure_msg" "$test_number"; ((test_number++))

# -------------------------------------------------------------------------------------
src="$(cat $proc_last_access)"
res=" 0 seconds have passed since last file access"
failure_msg="Output is '$src', but had to be: '$res'"
success_msg="Output is: '$src'"
run_test_comparison "$src" "$res" "$success_msg" "$failure_msg" "$test_number"; ((test_number++))


echo "$test_info Updating sys_settings $end (write 1)"
echo "1" | sudo tee $sys_settings &> /dev/null
sleep 1
# -------------------------------------------------------------------------------------
src="$(cat $proc_last_access)"
res=" 00:00:01 have passed since last file access"
failure_msg="Output is '$src', but had to be: '$res'"
success_msg="Output is: '$src'"
run_test_comparison "$src" "$res" "$success_msg" "$failure_msg" "$test_number"; ((test_number++))




# -------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------
test_number=1
echo "$test_info reading $proc_timestamps file $end"
echo " $start MANUAL TEST $test_number RESULT: $end"; ((test_number++))
cat $proc_timestamps
echo "-------------------------------------"


echo "$test_info sleeping  5 seconds"; sleep 5
echo "$test_info reading $proc_timestamps file $end"
echo " $start MANUAL TEST $test_number RESULT: $end"; ((test_number++))
cat $proc_timestamps
echo "-------------------------------------"


time="05:10:10"
echo "$test_info setting new time $time via $proc_set_time file $end"
echo $time | sudo tee $proc_set_time &> /dev/null
echo " $start MANUAL TEST $test_number RESULT: $end"; ((test_number++))
cat $proc_timestamps
echo "-------------------------------------"


time="10:59:16"
echo "$test_info setting new time $time via $proc_set_time file $end"
echo $time | sudo tee $proc_set_time &> /dev/null
echo " $start MANUAL TEST $test_number RESULT: $end"; ((test_number++))
cat $proc_timestamps
echo "-------------------------------------"

sudo rmmod ${target}.ko
echo "$test_info module ${target}.ko removed $end"

echo "$start $test_info last 10 lines of dmesg: $end"
dmesg | tail -10
