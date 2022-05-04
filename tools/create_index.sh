#!/bin/bash


DESCRIPTION=""

# match all when not initialized
BUILD="_"

# $@ is all command line parameters passed to the script.
# -o is for short options like -v
# -l is for long options with double dash like --version
# the comma separates different long options
# -a is for long options with single dash like -version
options=$(getopt -l "help,description:,build:" -o ":hd:b:" -a -- "$@")

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
-b|--build) 
  shift
  BUILD=$1
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
  <head>
    <style>
      body {
        font-family: sans-serif;
      }
      .pick-variant {
        margin-bottom: 16px;
      }
    </style>
    <script
      type=\"module\"
      src=\"https://unpkg.com/esp-web-tools@3.6.0/dist/web/install-button.js?module\"
    ></script>
  </head>
  <body>
    <h1>Install ESPEasy ${DESCRIPTION}</h1>

    <div class=\"pick-variant\">
      <p>
        To install ESPEasy, connect your ESP device to your computer, pick your
        selected variant and click the install button.
      </p>
      <select>
        <optgroup label=\"ESP8285\">"
find . |grep ${BUILD}|grep manifest.json|grep -i esp8285|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"ESP8266-1M\">"
find . |grep ${BUILD}|grep manifest.json|grep -iv esp32|grep -iv esp8285|grep _1M|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"ESP8266-2M\">"
find . |grep ${BUILD}|grep manifest.json|grep -iv esp32|grep -iv esp8285|grep _2M|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"ESP8266-4M\">"
find . |grep ${BUILD}|grep manifest.json|grep -iv esp32|grep -iv esp8285|grep _4M|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"ESP8266-16M\">"
find . |grep ${BUILD}|grep manifest.json|grep -iv esp32|grep -iv esp8285|grep _16M|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"ESP32-4M\">"
find . |grep ${BUILD}|grep manifest.json|grep -i esp32|grep -iv esp32s2|grep _4M|grep -v ETH|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"ESP32-16M\">"
find . |grep ${BUILD}|grep manifest.json|grep -i esp32|grep -iv esp32s2|grep _16M|grep -v ETH|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"ESP32-4M ETH\">"
find . |grep ${BUILD}|grep manifest.json|grep -i esp32|grep -iv esp32s2|grep _4M|grep ETH|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"ESP32-16M ETH\">"
find . |grep ${BUILD}|grep manifest.json|grep -i esp32|grep -iv esp32s2|grep _16M|grep ETH|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"ESP32-S2-4M\">"
find . |grep ${BUILD}|grep manifest.json|grep -i esp32s2|grep _4M|grep -v ETH|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"
echo "  <optgroup label=\"Hardware Specific\">"
find . |grep ${BUILD}|grep manifest.json|grep -i hard_|sort -n|grep -v dummy|cut -d '/' -f2-|xargs -n1 -I {} echo "<option value=\"{}\" > {}</option>" |sed 's/\ static\// /g'|sed 's/.manifest.json</</g'|sed 's/.factory</</g'|sed 's/\ bin\//\ /g'

echo "        </optgroup>"

echo "      </select>
    </div>
    <esp-web-install-button></esp-web-install-button>
    <script>
      const selectEl = document.querySelector(\".pick-variant select\");
      const installEl = document.querySelector(\"esp-web-install-button\");
      installEl.manifest = selectEl.value;
      selectEl.addEventListener(\"change\", () => {
        installEl.manifest = selectEl.value;
      });
    </script>
  </body>
</html>
"
