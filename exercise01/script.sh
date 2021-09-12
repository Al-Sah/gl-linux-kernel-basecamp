#! /bin/bash

dir=$(pwd)
delete_flag="-delete"
maxdepth="-maxdepth 1"
force=""

print_help(){
  echo "
    $(tput bold)* HELP *$(tput sgr0)

  $(tput bold)RUN$(tput sgr0)
    ./script [OPTIONS]

  $(tput bold)DESCRIPTION$(tput sgr0)

    Bash script deletes files in the directory received as a parameter:
      1) all files ending in .tmp.
      2) all files whose names begin with -, _, or ~.

  $(tput bold)OPTIONS$(tput sgr0)

      -h --help:       Print help
      -r --recursive:  Enable recursive search
      -t --test:       Just search, without deletion
      -y --yes:        Force mode
      "
}


# Extracting flags from positional parameters
options=$(getopt -l "help,recursive,yes,test" -o "hryt" -- "$@")
eval set -- "$options"; # Sorting flags and positional parameters with "--" delimiter

# Flags processing
while true; do

  case $1 in
    -h|--help) print_help; exit 0;;
    -r|--recursive) maxdepth="" ;;
    -t|--test) delete_flag="" ;;
    -y|--yes) force="-f";;
    --) break ;;
  esac

  shift
done

if [ $# -ge 1 ] && [ -d "$1" ]; then
    dir="$1"
fi

find $dir $force $maxdepth -type f -name "_*" $delete_flag
find $dir $force $maxdepth -type f -name "-*" $delete_flag
find $dir $force $maxdepth -type f -name "~*" $delete_flag
find $dir $force $maxdepth -type f -name "*.tmp" $delete_flag
find $dir $maxdepth -type d -empty $delete_flag



