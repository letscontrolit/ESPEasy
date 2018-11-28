#!/bin/bash

#
# This script walks through the assets folder and minifys all JS, HTML, CSS and SVG files. It also generates
# the corresponding constants that are added to the data.h file on esp8266_deauther folder.
#
# @Author Erick B. Tedeschi < erickbt86 [at] gmail [dot] com >
# @Author Wandmalfarbe https://github.com/Wandmalfarbe
#
# See: https://github.com/letscontrolit/ESPEasy/issues/1671#issuecomment-415144898

cd static
outputfile="$(pwd)/data_h_temp"
rm $outputfile

function minify_html_css {
	file=$1
	post_To https://html-minifier.com/raw $file
}

function minify_js {
	file=$1
	post_To https://javascript-minifier.com/raw $file
}

function post_To {
	url=$1
	file=$2
	RESPONSE=/tmp/response.txt
	HEADERS=/tmp/headers.txt
	curl -X POST -s --data-urlencode "input@$file" $url -D $HEADERS -o $RESPONSE
	status=$(cat $HEADERS | grep '^HTTP/1' | tail -1 | awk '{print $2}')
	if [[ "$status" == "200" ]]; then
		cat $RESPONSE > /tmp/converter.temp
		rm $RESPONSE
		rm $HEADERS
	else
		echo "can not minify $file error $status"
		exit 1
	fi
}

function minify_svg {
	file=$1
	svgo -i /Users/User/Desktop/icons/tools.svg -o - > /tmp/converter.temp
}

function ascii2hexCstyle {
	file_name=$(constFileName $1)
	result=$(cat /tmp/converter.temp | hexdump -ve '1/1 "0x%.2x,"')
	result=$(echo $result | sed 's/,$//')
	echo "const char DATA_${file_name}[] PROGMEM = {$result,0};"
}

function constFileName {
	extension=$(echo $1 | egrep -io "(json|svg|css|js|html)$" | tr "[:lower:]" "[:upper:]")
	file=$(echo $1 | sed 's/\.json//' | sed 's/\.svg//' | sed 's/\.css//' | sed 's/\.html//' | sed 's/\.js//' | sed 's/\.\///' | tr '/' '_' | tr '.' '_' | tr '-' '_' | tr "[:lower:]" "[:upper:]")
	underscore="_"
	echo $file$underscore$extension
}



file_list=$(find . -type f)

for file in $file_list; do
	echo "Processing: $file"
	file_name=$(constFileName $file)
	echo "  Array Name: $file_name"

	if [[ "$file" == *.min.js ]]; then
		echo "  JS already minified"
		cat $file > /tmp/converter.temp
		ascii2hexCstyle $file >> $outputfile
	elif [[ "$file" == *.js ]]; then
		echo "  JS minify"
		minify_js $file
		ascii2hexCstyle $file >> $outputfile
	elif [[ "$file" == *.min.css ]]; then
		echo "  CSS already minified"
		cat $file > /tmp/converter.temp
		ascii2hexCstyle $file >> $outputfile
	elif [[ "$file" == *.html ]] || [[ "$file" == *.css ]]; then
		echo "  HTML and CSS minify"
		minify_html_css $file
		ascii2hexCstyle $file >> $outputfile
	elif [[ "$file" == *.svg ]]; then
		echo "  SVG minify"
		minify_svg $file
		ascii2hexCstyle $file >> $outputfile
	else
		echo "  without minifier"
		cat $file > /tmp/converter.temp
		ascii2hexCstyle $file >> $outputfile
	fi
	echo ""
	sleep 1
done
cd ..
