#! /bin/bash

# Global variables
upper_range_limit=100
guessing_tries=3
user_number=0
random_number=0
is_guessed=0
first_play=1

get_user_number(){
  local un=""
  read -rp " Enter your number: " un
  # Check. Is user_number variable a type of integer ?
  until [[ -n "$un" ]] || [[ $un =~ ^[0-9]+$ ]]; do
      read -rp " error: \"$un\"cannot be type of int. Try again: " un
  done
  user_number=$un
}

get_random_number(){
	random_number=$((RANDOM % upper_range_limit))
	echo " Generated random number in range [0-$upper_range_limit]"
}

compare(){
if (( is_guessed == 0 )); then
  if ((random_number > user_number)); then
	    echo " Randomly generated number is greater than $user_number"; return 1
    elif ((random_number < user_number)); then
	    echo " Randomly generated number is less than $user_number"; return 1
    else
      is_guessed=1
	    echo " Bingo !!! Randomly generated number is $random_number"; return 0
  fi
else return 0
fi;
}

# Validation of "upper_range_limit" variable
validate_limit_number(){
  until [[ $upper_range_limit =~ ^[0-9]+$ ]] && ((upper_range_limit >= 1 && upper_range_limit <= 100)); do
    echo " $upper_range_limit is not in range from 1 to 100"
    read -rp " Enter upper range limit number (1-100): " upper_range_limit
    if [[ $upper_range_limit =~ ^[0-9]+$ ]] && ((upper_range_limit >= 1 && upper_range_limit <= 100)); then
      break
    fi
  done
}
# Validation of "$guessing_tries" variable
validate_retries_number(){
  until [[ $guessing_tries =~ ^[0-9]+$ ]] && ((guessing_tries >= 1 && guessing_tries <= 20)); do
    echo " $guessing_tries is not appropriate for the guessing tries. "
    echo " Enter number in range from 1 to 20"
    read -rp " Enter upper range limit number (1-100): " guessing_tries
    if [[ $guessing_tries =~ ^[0-9]+$ ]] && ((guessing_tries >= 1 && guessing_tries <= 20)); then
      break
    fi
  done
}

print_game_info(){
  echo ""
  echo "  You have tries: $guessing_tries"
  echo "  Your current try: $counter"
}


# shellcheck disable=SC2120
play(){
  is_guessed=0
  counter=1
  get_random_number

  if (( first_play == 1)); then
    first_play=0; print_game_info;
    if ! [[ $user_number =~ ^[0-9]+$ ]]; then get_user_number; fi; compare
  fi

  # Main loop
  until [[ $counter -gt $guessing_tries ]] || (( is_guessed == 1 )); do
    print_game_info
    ((counter++))
    get_user_number
    if compare; then break; fi
  done

  if (( is_guessed == 0 )); then
    echo "";
    echo " ## Ups. Failed. Random number was $random_number";
    echo "";
  fi
}

print_menu(){
  echo " "
  echo " ## Menu ##"
  echo " 1|s|start|play - play game"
  echo " 2|p|param  - params"
  echo " 3|q|quit   - finish and leave game"
}



process_menu_input(){
  local menu_input="0"
  read -rp "What do you want to do? " menu_input

  case $menu_input in
    1|s|start|play|"") play;;
    2|p|param) change_param ;;
    3|q|quit) exit 0;;
  esac
}

change_param(){
  echo ""
  echo " ## Changing param menu ##"
  echo " 1|u|upper   - change limit number (1-100)"
  echo " 2|r|retries - change number of tries"

  local menu_input=""
  read -rp "What do you want to do ?" menu_input

  case $menu_input in
    1|u|upper)
      read -rp "Enter upper range limit number (1-100): " upper_range_limit
      validate_limit_number ;;
    2|r|retries)
      read -rp "Enter upper range limit number (1-100): " guessing_tries
      validate_retries_number ;;
    *) echo "Wrong action [$menu_input]";;
  esac
}



# Extracting flags from positional parameters
options=$(getopt -l "upper:,retries:" -o "u:r:" -- "$@")
eval set -- "$options"; # Sorting flags and positional parameters with "--" delimiter
echo "Options $options"

# Flags processing
while true; do
  case $1 in
  -u|--upper) shift; upper_range_limit=$1 ;;
  -r|--retries) shift; guessing_tries=$1 ;;
  --) shift; break ;;
  esac
  shift
done

validate_limit_number
validate_retries_number
user_number=$1

while true; do
  print_menu
  process_menu_input
done