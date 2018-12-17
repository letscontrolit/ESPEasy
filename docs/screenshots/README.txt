These scripts are used to render screenshots of the GUI.
We use phantomJS (available on Win/Mac/Linux http://phantomjs.org/quick-start.html)
and you control the script by passing arguments to the JavaScript files:

########## rasterize.js ##########
argument1: url (ampersand & need to be closed with "&")
argument2: output file name (you can use .png or .pdf as output format)
argument3: horisontalXvertical size (OPTIONAL)
argument4: zoom ("entire page" or "window" or <value> OPTIONAL)
....Example CLI....

phantomjs rasterize.js http://95.143.204.227/devices?index=1"&"page=1 P001_Setup_Switch_1.png

or

phantomjs rasterize.js http://95.143.204.227/devices?index=1"&"page=1 P001_Setup_Switch_1.png 1200px 2

WE USE THE SECOND EXAMPLE by default (you might need to change zoomFactor if the page is too big).

If the rendering is not working you can use this CLI (arg --debug=true):

phantomjs --debug=true rasterize.js http://95.143.204.227/devices?index=1"&"page=1 P001_Setup_Switch_1.png 1200px 2
