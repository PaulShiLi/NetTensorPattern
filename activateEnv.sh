#!/bin/bash

#######################################

# Color Codes

# Reset
Color_Off='\033[0m' # Text Reset

# Regular Colors
Black='\033[0;30m'  # Black
Red='\033[0;31m'    # Red
Green='\033[0;32m'  # Green
Yellow='\033[0;33m' # Yellow
Blue='\033[0;34m'   # Blue
Purple='\033[0;35m' # Purple
Cyan='\033[0;36m'   # Cyan
White='\033[0;37m'  # White
Grey='\033[0;90m'   # Grey

# Bold
BBlack='\033[1;30m'  # Black
BRed='\033[1;31m'    # Red
BGreen='\033[1;32m'  # Green
BYellow='\033[1;33m' # Yellow
BBlue='\033[1;34m'   # Blue
BPurple='\033[1;35m' # Purple
BCyan='\033[1;36m'   # Cyan
BWhite='\033[1;37m'  # White
BGrey='\033[1;90m'   # Grey

# Underline
UBlack='\033[4;30m'  # Black
URed='\033[4;31m'    # Red
UGreen='\033[4;32m'  # Green
UYellow='\033[4;33m' # Yellow
UBlue='\033[4;34m'   # Blue
UPurple='\033[4;35m' # Purple
UCyan='\033[4;36m'   # Cyan
UWhite='\033[4;37m'  # White
UGrey='\033[4;90m'   # Grey

# Background
On_Black='\033[40m'  # Black
On_Red='\033[41m'    # Red
On_Green='\033[42m'  # Green
On_Yellow='\033[43m' # Yellow
On_Blue='\033[44m'   # Blue
On_Purple='\033[45m' # Purple
On_Cyan='\033[46m'   # Cyan
On_White='\033[47m'  # White
On_Grey='\033[100m'  # Grey

# High Intensity
IBlack='\033[0;90m'  # Black
IRed='\033[0;91m'    # Red
IGreen='\033[0;92m'  # Green
IYellow='\033[0;93m' # Yellow
IBlue='\033[0;94m'   # Blue
IPurple='\033[0;95m' # Purple
ICyan='\033[0;96m'   # Cyan
IWhite='\033[0;97m'  # White

# Bold High Intensity
BIBlack='\033[1;90m'  # Black
BIRed='\033[1;91m'    # Red
BIGreen='\033[1;92m'  # Green
BIYellow='\033[1;93m' # Yellow
BIBlue='\033[1;94m'   # Blue
BIPurple='\033[1;95m' # Purple
BICyan='\033[1;96m'   # Cyan
BIWhite='\033[1;97m'  # White

# High Intensity backgrounds
On_IBlack='\033[0;100m'  # Black
On_IRed='\033[0;101m'    # Red
On_IGreen='\033[0;102m'  # Green
On_IYellow='\033[0;103m' # Yellow
On_IBlue='\033[0;104m'   # Blue
On_IPurple='\033[0;105m' # Purple
On_ICyan='\033[0;106m'   # Cyan
On_IWhite='\033[0;107m'  # White

#######################################

# Functions

printSection() {
    echo -e "\n==============================\n"
}

# Function to accept a string parameter to set color
setColor() {
    printf "${1}"
}

printHeader() {
    setColor $Cyan
    toPrint=$1
    noNewLines=false

    # Loop through parameters and see if there are any flags and skip 1st parameter and if there are no parameters, print a new line
    for var in "${@:2}"
    do
        if [[ $var == "no lines" ]]; then
            toPrint+=""
            noNewLines=true
        elif [[ $var == "section" ]]; then
            toPrint="\n\n${toPrint}"
        fi
    done
    if [[ $noNewLines == false ]]; then
        toPrint+="\n"
    fi
    
    echo -e "${toPrint}"
    setColor $Color_Off
}

printAlreadyInstalled() {
    setColor $Yellow
    if [[ -z $2 ]]; then
        echo -e "[=] $1 is already installed"
    else
        echo -e "[=] $1 $2"
    fi
    setColor $Color_Off
}

printInstalled() {
    setColor $Green
    if [[ -z $2 ]]; then
        echo -e "[+] $1 installed"
    else
        echo -e "[+] $1 $2"
    fi
    setColor $Color_Off
}

printNotInstalled() {
    setColor $Red
    if [[ -z $2 ]]; then
        echo -e "[!] $1 not found"
    else
        echo -e "[!] $1 $2"
    fi
    setColor $Color_Off
}

printUninstall() {
    setColor $Red
    if [[ -z $2 ]]; then
        echo -e "[-] $1 uninstalled"
    else
        echo -e "[-] $1 $2"
    fi
    setColor $Color_Off
}

printInProgress() {
    setColor $White
    if [[ -z $2 ]]; then
        echo -e "[.] $1 in progress"
    else
        echo -e "[.] $1 $2"
    fi
    setColor $Color_Off
}

#######################################

rootDir=$(dirname "${BASH_SOURCE[0]}")

#######################################

# Some checks before running the script
    
# Check if script is using source to run

printSection
printHeader "Checking execution environment and privileges:" "no lines"

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printNotInstalled "Please run this script using source command!" " "
    exit 1
else
    printAlreadyInstalled "Script is running using source command" " "
fi

#######################################

VENV="nettensor_env"

printSection
printHeader "Activating ${VENV}..." "section"

if [ ! -d "$VENV" ]; then
    # Take action if $venv does exists
    printNotInstalled $VENV
    exit 1
else
    printAlreadyInstalled $VENV
fi

source $rootDir/$VENV/bin/activate

echo -e "- ${VENV} activated"
