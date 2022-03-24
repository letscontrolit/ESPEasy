Fetch and decode bin files from the cache controller
****************************************************

The dump5.htm file should be uploaded to the file system of the ESP.
When there are cache controller bin files present on the file system,
open this htm file from the file browser in ESPEasy into a new tab in your browser.

This presents a large button "Fetch cache files".
When pressed, the JavaScript in this htm file will fetch JSON information from 
ESPEasy describing the column names and the binary cache files present on the file system.

Those bin files will be fetched and decoded in the browser.
When done, a "Download" button will be presented which generates and downloads a new CSV file.

This file can be opened in any spreadsheet program.
LibreNMS has proven to be the easiest to parse the column separators and make the best guess on the data types in each cell.


