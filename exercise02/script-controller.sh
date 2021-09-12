#! /bin/bash

dir=$(pwd)
test_flag=""
recursive_flag=""

print_help(){
  echo "
    $(tput bold)* HELP *$(tput sgr0)

  $(tput bold)RUN$(tput sgr0)
    ./script [OPTIONS] [DIR]

  $(tput bold)DESCRIPTION$(tput sgr0)

  Extra bash script which controls base scripts from
  exercises 1 and 2.

  $(tput bold)OPTIONS$(tput sgr0)

      -h --help:       Print help
      -r --recursive:  Enable recursive search
      -t --test:       Just search, without files movement and deletion
      "
}


options=$(getopt -l "help,recursive,test" -o "hrt" -- "$@")
eval set -- "$options";

while true; do
  case $1 in
    -h|--help) print_help; exit 0;;
    -r|--recursive) recursive_flag="-r" ;;
    -t|--test) test_flag="-t" ;;
    --) break ;;
  esac
  shift
done

if [ $# -ge 1 ] && [ -d "$1" ]; then
    dir="$1"
fi

# Exit status equals number of files to delete
./files_modifier.sh $test_flag $recursive_flag "$dir"
files_found=$?


if (( files_found > 0 )); then
  echo "Running ... /exercise01/script.sh"
  ./../exercise01/script.sh $test_flag $recursive_flag "$dir"
fi
