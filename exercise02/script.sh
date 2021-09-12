#! /bin/bash

dir=$(pwd)
test_mode=0
maxdepth="-maxdepth 1"
recursive_flag=""
force=""

print_help(){
  echo "
    $(tput bold)* HELP *$(tput sgr0)

  $(tput bold)RUN$(tput sgr0)
    ./script [OPTIONS]

  $(tput bold)DESCRIPTION$(tput sgr0)

  $(tput bold)OPTIONS$(tput sgr0)

      -h --help:       Print help
      -r --recursive:  Enable recursive search
      -t --test:       Just search, without  deletion
      "
}


# Extracting flags from positional parameters
options=$(getopt -l "help,recursive,test" -o "hrt" -- "$@")
eval set -- "$options"; # Sorting flags and positional parameters with "--" delimiter

# Flags processing
while true; do

  case $1 in
    -h|--help) print_help; exit 0;;
    -r|--recursive) maxdepth=""; recursive_flag="r" ;;
    -t|--test) test_mode=1 ;;
    --) break ;;
  esac

  shift
done

if [ $# -ge 1 ] && [ -d "$1" ]; then
    dir="$1"
fi


files=$(find $dir $force $maxdepth -type f -mtime +30)
found=0

for file in $files; do

if [[ "$file" == "$0" ]]; then
  echo "x has the value 'valid'"
  continue;
fi
  ((found++))

  if (( test_mode == 1 )); then
    echo " * File: $file"
  else
    file_basename=$(basename "$file")
    file_dirname=$(dirname "$file")
    echo "file_basename $file_basename"
    echo "file_dirname $file_dirname"
    mv "$file_dirname/$file_basename" "$file_dirname/~$file_basename"
  fi

done

if (( found > 0 )) && ((test_mode == 0 )); then
  echo "Running /exercise01/script.sh"
  ./../exercise01/script.sh $recursive_flag -y
else
  echo " Fond files to delete: $found"
fi
