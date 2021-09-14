#! /bin/bash


current_dir="$(pwd)"
module_dir=""

sys_fs_interface="/sys/class/string_processor_class/settings"
proc_fs_in="/proc/string_processor/input"
proc_fs_out="/proc/string_processor/output"


# output messages
start="$(tput bold)"
end="$(tput sgr0)"
subtest="$start * subtest "
test_info="$(tput setaf 6) + test:"
passed="$(tput setaf 2) PASSED $end"
failed="$(tput setaf 1) FAILED $end"
red="$(tput setaf 1)"

# test
test_number=1
src=""
res=""
success_msg=""
failure_msg=""

run_test(){
  if [[ "$1" == "$2" ]]; then
    echo "$subtest $5 $passed"
    echo "  $3"
  else
    echo "$subtest $5 $failed"
    echo "  $4"
  fi
  echo "-------------------------------------"
}



if [[ -d "$(pwd)/part03" ]] && [[ -f "$(pwd)/part03/Makefile" ]]; then
  # shellcheck disable=SC2164
  cd "$(pwd)/part03"
  make all &> "$current_dir/make-all.log"
  echo "$test_info run 'make all. See log at make-all.log$end"

  if [[ ! -e "${current_dir}/part03/build/string_processor.ko" ]]; then
    echo "$red Probably build failed ... 'part03/build/string_processor.ko' is not accessible $end";exit 1
  else
    module_dir="${current_dir}/part03/build/"
  fi

else
  if [[ ! -e "${current_dir}/string_processor.ko" ]]; then
      echo "$red Failed to find string_processor.ko $end";exit 1
  else
    module_dir="${current_dir}"
  fi
fi

echo "$test_info Module location: ${module_dir}/string_processor.ko $end"


# module loading
if sudo lsmod | grep -q string_processor; then sudo rmmod string_processor.ko; fi
sudo insmod "${module_dir}/string_processor.ko"

if sudo lsmod | grep -q string_processor; then
  echo "$test_info Module loaded successfully $end"
else
  echo "$red Failed to load module $end"; exit 1
fi


if [[ -e "$sys_fs_interface" ]] && [[ -e "$proc_fs_in" ]] && [[ -e "$proc_fs_out" ]]; then
  echo "$test_info all interfaces are accessible $end"
else
  echo "$red interfaces are not accessible $end"
  exit 1
fi



# -------------------------------------------------------------------------------------
src="$(cat $proc_fs_out)"
success_msg="File $proc_fs_out is empty"
failure_msg="File $proc_fs_out is not empty"
run_test "$src" "$res" "$success_msg" "$failure_msg" "$test_number"; ((test_number++))


# -------------------------------------------------------------------------------------
res="0"
src="$(cat $sys_fs_interface)"
failure_msg="File '$sys_fs_interface' mast contains '0', but contains '$src"
success_msg="Default value is correct (contains '0')"
run_test "$src" "$res" "$success_msg" "$failure_msg" "$test_number"; ((test_number++))


# -------------------------------------------------------------------------------------
res="1234 qwerty aaz zxcv"
echo "$test_info writing in $proc_fs_in '$res' $end"
echo "$res" | sudo tee $proc_fs_in &> /dev/null

src="$(cat $proc_fs_out)"
failure_msg="Output is '$src', but had to be: '$res'"
success_msg="Output is: '$src'"
run_test "$src" "$res" "$success_msg" "$failure_msg" "$test_number"; ((test_number++))


# -------------------------------------------------------------------------------------
res="4321 ytrewq zaa vcxz"
echo "$test_info writing in $sys_fs_interface '1' $end (flip words)"
echo "1" | sudo tee $sys_fs_interface > /dev/null
src="$(cat $proc_fs_out)"
failure_msg="Output is '$src', but had to be: '$res'"
success_msg="Output is: '$src'"
run_test "$src" "$res" "$success_msg" "$failure_msg" "$test_number"; ((test_number++))



# -------------------------------------------------------------------------------------
res="1234 QWERTY AAZ ZXCV"
echo "$test_info writing in $sys_fs_interface '2' $end (to uppercase)"
echo "2" | sudo tee $sys_fs_interface > /dev/null
src="$(cat $proc_fs_out)"
failure_msg="Output is '$src', but had to be: '$res'"
success_msg="Output is: '$src'"
run_test "$src" "$res" "$success_msg" "$failure_msg" "$test_number"; ((test_number++))


# -------------------------------------------------------------------------------------
res="4321 YTREWQ ZAA VCXZ"
echo "$test_info writing in $sys_fs_interface '3' $end (to uppercase and flip words)"
echo "3" | sudo tee $sys_fs_interface > /dev/null
src="$(cat $proc_fs_out)"
failure_msg="Output is '$src', but had to be: '$res'"
success_msg="Output is: '$src'"
run_test "$src" "$res" "$success_msg" "$failure_msg" "$test_number"; ((test_number++))


sudo rmmod string_processor.ko
echo "$test_info module string_processor.ko removed $end"