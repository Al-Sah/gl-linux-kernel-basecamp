# Exercise03


#### Subtask1
    git config --global user.name "Name Surname"
    git config --global user.email "mailbox@example.com"

    git config --global core.autocrlf input
    git config --global core.safecrlf true

#### Subtask2

    mkdir exercise03
    cd exercise03

#### Subtask3

    mkdir pro
    cd pro
    touch readme.md
    git add .
    git commit -m "Exercise03: Init exercise project"

#### Subtask4

    git checkout -b first_branch
    gedit readme.md
    git status
    git commit -am "Exercise03(readme): Add the command log of the 1st subtask"

#### Subtask5

    git checkout master
    gedit readme.md
    git commit -am "Exercise03(readme): Add the command log of the 2nd subtask"
    git log --oneline --decorate --graph --all

#### Subtask6

    git status
    git merge first_branch
    git mergetool
    git status
    git log --oneline --decorate --graph --all

#### Subtask7

    gedit readme.md
    git commit -am "Exercise03(readme): Add the command log of the 3d subtask"
    gedit readme.md
    git commit -am "Exercise03(readme): Add the command log of the 4th subtask"
    gedit readme.md
    git commit -am "Exercise03(readme): Add the command log of the 5th subtask"
    gedit readme.md
    git commit -am "Exercise03(readme): Add the command log of the 6th subtask"
    gedit readme.md
    git commit -am "Exercise03(readme): Add the command log of the 7th subtask"
    git log --oneline --decorate --graph --all


