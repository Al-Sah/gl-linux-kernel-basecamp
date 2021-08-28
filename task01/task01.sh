#! /bin/bash

dir=""
last_command=""

# 1 - show all
# 2 - just files
# 3 - just dirs
visibly_content=1

# 1 - enable
# 2 - disable 
show_hidden=1

# 1 - short
# 2 - long
display_form=1


if [ $# -ge 1 ] && [ -d "$1" ]; then
  dir="$1"
else
	dir=$(pwd)
fi
cd "$dir"


print_menu(){
  echo " "
  echo "  **** MENU ****"
  echo " 1 -> change display mode "
  echo " 2 -> cd <path> "
  echo " 3 -> cp <new_file.tx> <file.txt> "
  echo " 4 -> mv <new_file.tx> <file.txt> "
  echo " 5 -> rm <file.txt> "
  echo " 6 -> quit "
  echo " "
}

change_display_mode(){
  echo ""
  echo " Changing display properties..."
  echo " 1|cdm -visibly_content : $visibly_content (current). Can be: (1, 2 or 3)"
  echo " 2|sh  -  show_hidden   : $show_hidden (current). Can be: (1)-enable (2)-disable"
  echo " 3|df  -  display_form  : $display_form (current). Can be: (1)-short (2)-long"
  echo ""
  read -rp "Enter property ( 1|dm  2|sh  3|df ) :" property
  case $property in
  	[1]|cdm) read -rp "Display mode: (1, 2 or 3) " visibly_content
  	  until [[ $visibly_content =~ ^[0-9]+$ ]] && ((visibly_content >= 1 && visibly_content <= 3)); do
            echo " $visibly_content is not in range from 1 to 3"
            read -rp " Enter display mode (1/2/3): " visibly_content
        done ;;
    [2]|sh) read -rp "Show hidden: (\"1\" or \"2\") " show_hidden
        until [[ $show_hidden =~ ^[0-9]+$ ]] && ((show_hidden == 1 || show_hidden == 2)); do
            echo " $show_hidden is not 1 or 3"
            read -rp "Show hidden: (\"1\" or \"2\") " show_hidden
        done ;;
    [3]|df) read -rp "Displaying form: (1-short | 2-long)" display_form
            until [[ $display_form =~ ^[0-9]+$ ]] && ((display_form == 1 || display_form == 2)); do
                echo " $display_form is not 1 or 3"
                read -rp "Displaying form: (1-short | 2-long)" display_form
            done ;;
    *) echo "Wrong parameter: $property" ;;
  esac
}


change_dir(){
  read -rp "Enter new path: " new_path
  if [ -z "$new_path" ] || [ -d "$new_path" ]; then
    cd "$new_path" || return
  else
    echo " cd: failed to change dir"
  fi
}

copy_file(){
  read -rp "Enter old filename: " old_filename
  read -rp "Enter new filename: " new_filename
  cp "$old_filename" "$new_filename"
}

move_file(){
  read -rp "Enter old filename: " old_filename
  read -rp "Enter new filename: " new_filename
  mv "$old_filename" "$new_filename"
}

remove_file(){
  read -rp "Enter filename: " filename
  rm "$filename"
}

process_user_input(){
  read -rp "Enter command (number from 1 to 6): " last_command
  case $last_command in
	  [1]|dm) change_display_mode;;
	  [2]|cd) change_dir ;;
	  [3]|cp) copy_file ;;
	  [4]|mv) move_file ;;
	  [5]|rm) remove_file ;;
	  [6]|q|exit) echo "bye !!!"; exit ;;
	  *) echo "Wrong command" ;;
  esac
}

print_files_list(){


  echo ""
  echo "Contains of $(pwd) "

  if (( show_hidden == 1 )); then
    if (( display_form == 2)); then ls_flags="-al"; else ls_flags="-a"; fi
  else
    if (( display_form == 2)); then ls_flags="-l"; else ls_flags="-1"; fi
  fi
  if (( display_form == 1)); then array=("-"); else array=(); fi

  while IFS='' read -r line; do array+=("$line"); done < <(ls $ls_flags)
  echo ""
  for ((i=1; i < ${#array[@]}; i++ )); do
      if (( i > 9 )); then
        echo "  $i  ${array[i]}"
      else
        echo "  $i   ${array[i]}"
      fi
  done
}


while true; do

read -p "Press Enter to continue"
clear;

print_files_list

print_menu
process_user_input

done;