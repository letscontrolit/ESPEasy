#!/bin/bash


DESCRIPTION=""

# $@ is all command line parameters passed to the script.
# -o is for short options like -v
# -l is for long options with double dash like --version
# the comma separates different long options
# -a is for long options with single dash like -version
options=$(getopt -l "help,description:" -o ":hd:" -a -- "$@")

# set --:
# If no arguments follow this option, then the positional parameters are unset. Otherwise, the positional parameters 
# are set to the arguments, even if some of them begin with a ‘-’.
eval set -- "$options"

while true
do
case $1 in
-h|--help) HELP=1;;
-d|--description) 
  shift
  DESCRIPTION=$1
  ;;
\?) echo "Invalid option: $2" 1>&2
   HELP=1;;
--)
    shift
    break;;
esac
shift
done

echo "<!DOCTYPE html>
<html>
<body>

<h1>Flash ESPEasy ${DESCRIPTION}</h1>

<script
  type=\"module\"
  src=\"https://unpkg.com/esp-web-tools@3.6.0/dist/web/install-button.js?module\"
></script>"


find . |grep manifest.json|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<li><esp-web-install-button manifest=\"{}\" ></esp-web-install-button> {}</li>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/-factory</</g'

echo "</body></html>"
