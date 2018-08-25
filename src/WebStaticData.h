#ifndef WEBSTATICDATA_h
#define WEBSTATICDATA_h

#define PGMT( pgm_ptr ) ( reinterpret_cast< const __FlashStringHelper * >( pgm_ptr ) )

/*********************************************************************************************\
 * ESP Easy logo Favicon.ico 16x16 8 bit
\*********************************************************************************************/
// Generated using xxd:   xxd -i favicon.ico > favicon.ino
static const char favicon_8b_ico[] PROGMEM = {
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x00, 0x00, 0x01, 0x00,
  0x20, 0x00, 0x68, 0x04, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x12, 0x0b,
  0x00, 0x00, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xef, 0xc0, 0x89, 0x11, 0xfe, 0xfb, 0xf8, 0xac, 0xff, 0xff,
  0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xf3,
  0xe9, 0xac, 0xef, 0xc0, 0x89, 0x11, 0xfe, 0xfb, 0xf8, 0xac, 0xf1, 0xc8,
  0x95, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xf1, 0xc8, 0x95, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xef, 0xc2,
  0x8a, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfc, 0xf3, 0xe9, 0xac, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd9, 0x69,
  0x00, 0xff, 0xd9, 0x69, 0x00, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xc2,
  0x8a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xd9, 0x69, 0x00, 0xff, 0xd9, 0x69, 0x00, 0xff, 0xef, 0xc2,
  0x8a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xf1, 0xc8, 0x95, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xc2,
  0x8a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xc2,
  0x8a, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xc8, 0x95, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xc2,
  0x8a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xc8,
  0x95, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xf1, 0xc8, 0x95, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xc2,
  0x8a, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xc2,
  0x8a, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xd8, 0x63, 0x00, 0xff, 0xef, 0xbc,
  0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xbc, 0x80, 0xff, 0xd8, 0x63,
  0x00, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xf1,
  0xe6, 0xac, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xc8, 0x95, 0xff, 0xf1, 0xc8,
  0x95, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xc2,
  0x8a, 0xff, 0xf1, 0xc8, 0x95, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xef, 0xc2, 0x8a, 0xff, 0xf1, 0xc8, 0x95, 0xff, 0xfe, 0xfb,
  0xf8, 0xac, 0xef, 0xc0, 0x89, 0x11, 0xfb, 0xf1, 0xe6, 0xac, 0xfe, 0xfa,
  0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfa,
  0xf5, 0xff, 0xfe, 0xfa, 0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfb,
  0xf8, 0xac, 0xef, 0xc0, 0x89, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned int favicon_8b_ico_len = 1150;

static const char githublogo[] PROGMEM = {
  "<button class='button link' onclick='setGithubClipboard()' style='padding: 0px 2px; position: relative; top: 4px;'>"
  "<svg width='24' height='24' viewBox='-1 -1 26 26' style='position: relative; top: 2px;'>"
  "<path d='M12 .297c-6.63 0-12 5.373-12 12 0 5.303 3.438 9.8 8.205 11.385.6.113.82-.258.82-.577 0-.285-.01-1.04-.015-2.04-3.338.724-4.042-1.61-4.042-1.61C4.422 18.07 3.633 17.7 3.633 17.7c-1.087-.744.084-.729.084-.729 1.205.084 1.838 1.236 1.838 1.236 1.07 1.835 2.809 1.305 3.495.998.108-.776.417-1.305.76-1.605-2.665-.3-5.466-1.332-5.466-5.93 0-1.31.465-2.38 1.235-3.22-.135-.303-.54-1.523.105-3.176 0 0 1.005-.322 3.3 1.23.96-.267 1.98-.399 3-.405 1.02.006 2.04.138 3 .405 2.28-1.552 3.285-1.23 3.285-1.23.645 1.653.24 2.873.12 3.176.765.84 1.23 1.91 1.23 3.22 0 4.61-2.805 5.625-5.475 5.92.42.36.81 1.096.81 2.22 0 1.606-.015 2.896-.015 3.286 0 .315.21.69.825.57C20.565 22.092 24 17.592 24 12.297c0-6.627-5.373-12-12-12' stroke='white' fill='white'/></svg>"
  "</button>"
  "<script>"
  "function setGithubClipboard() { var clipboard = 'ESP Easy | Information |\\n -----|-----|\\n';"
  " max_loop = 100;"
  " for (var i = 1; i < max_loop; i++){"
  " var cur_id = 'copyText_' + i;"
  " var test = document.getElementById(cur_id);"
  " if (test == null){ i = max_loop + 1;"
  "  } else {"
  "var separatorSymbol = '|';"
  "if (i % 2 == 0) {separatorSymbol += '\\n'}"
  " clipboard += test.innerHTML.replace(/<[Bb][Rr]\\s*\\/?>/gim,'\\n') + separatorSymbol; } }"
  "clipboard = clipboard.replace(/<\\/[Dd][Ii][Vv]\\s*\\/?>/gim,'\\n');"
  "clipboard = clipboard.replace(/<[^>]*>/gim,'');"
  "var tempInput = document.createElement('textarea');"
  "tempInput.style = 'position: absolute;"
  "left: -1000px; top: -1000px'; tempInput.innerHTML = clipboard;"
  "document.body.appendChild(tempInput);"
  "tempInput.select();"
  "document.execCommand('copy');"
  "document.body.removeChild(tempInput);"
  "alert('Copied: \"' + clipboard + '\" to clipboard!') }"
  "</script>"
};

static const char pgDefaultCSS[] PROGMEM = {
    //color scheme: #07D #D50 #DB0 #A0D
    "* {font-family: sans-serif; font-size: 12pt; margin: 0px; padding: 0px; box-sizing: border-box; }"
    "h1 {font-size: 16pt; color: #07D; margin: 8px 0; font-weight: bold; }"
    "h2 {font-size: 12pt; margin: 0 -4px; padding: 6px; background-color: #444; color: #FFF; font-weight: bold; }"
    "h3 {font-size: 12pt; margin: 16px -4px 0 -4px; padding: 4px; background-color: #EEE; color: #444; font-weight: bold; }"
    "h6 {font-size: 10pt; color: #07D; }"
    // buttons
    ".button {margin: 4px; padding: 4px 16px; background-color: #07D; color: #FFF; text-decoration: none; border-radius: 4px; border: none;}"
    ".button.link { }"
    ".button.link.wide {display: inline-block; width: 100%; text-align: center;}"
    ".button.link.red {background-color: red;}"
    ".button.help {padding: 2px 4px; border-style: solid; border-width: 1px; border-color: gray; border-radius: 50%; }"
    ".button:hover {background: #369; }"
    // inputs, select, textarea general
    "input, select, textarea {margin: 4px; padding: 4px 8px; border-radius: 4px; background-color: #eee; border-style: solid; border-width: 1px; border-color: gray;}"
    // inputs
    "input:hover {background-color: #ccc; }"
    "input.wide {max-width: 500px; width:80%; }"
    "input.widenumber {max-width: 500px; width:100px; }"
    // select
    "#selectwidth {max-width: 500px; width:80%; padding: 4px 8px;}"
    "select:hover {background-color: #ccc; }"
    // custom checkboxes
    ".container {display: block; padding-left: 35px; margin-left: 4px; margin-top: 0px; position: relative; cursor: pointer; font-size: 12pt; -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; }"
    // Hide the browser's default checkbox
    ".container input {position: absolute; opacity: 0; cursor: pointer;  }"
    // Create a custom checkbox
    ".checkmark {position: absolute; top: 0; left: 0; height: 25px;  width: 25px;  background-color: #eee; border-style: solid; border-width: 1px; border-color: gray;  border-radius: 4px;}"
    // On mouse-over, add a grey background color
    ".container:hover input ~ .checkmark {background-color: #ccc; }"
    // When the checkbox is checked, add a blue background
    ".container input:checked ~ .checkmark { background-color: #07D; }"
    // Create the checkmark/indicator (hidden when not checked)
    ".checkmark:after {content: ''; position: absolute; display: none; }"
    // Show the checkmark when checked
    ".container input:checked ~ .checkmark:after {display: block; }"
    // Style the checkmark/indicator
    ".container .checkmark:after {left: 7px; top: 3px; width: 5px; height: 10px; border: solid white; border-width: 0 3px 3px 0; -webkit-transform: rotate(45deg); -ms-transform: rotate(45deg); transform: rotate(45deg); }"

    // custom radio buttons
    ".container2 {display: block; padding-left: 35px; margin-left: 9px; margin-bottom: 20px; position: relative; cursor: pointer; font-size: 12pt; -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; }"
    // Hide the browser's default radio button
    ".container2 input {position: absolute; opacity: 0; cursor: pointer;  }"
    // Create a custom radio button
    ".dotmark {position: absolute; top: 0; left: 0; height: 26px;  width: 26px;  background-color: #eee; border-style: solid; border-width: 1px; border-color: gray; border-radius: 50%;}"
    // On mouse-over, add a grey background color
    ".container2:hover input ~ .dotmark {background-color: #ccc; }"
    // When the radio button is checked, add a blue background
    ".container2 input:checked ~ .dotmark { background-color: #07D;}"
    // Create the dot/indicator (hidden when not checked)
    ".dotmark:after {content: ''; position: absolute; display: none; }"
    // Show the dot when checked
    ".container2 input:checked ~ .dotmark:after {display: block; }"
    // Style the dot/indicator
    ".container2 .dotmark:after {top: 8px; left: 8px; width: 8px; height: 8px;	border-radius: 50%;	background: white; }"

    // toast messsage
    "#toastmessage {visibility: hidden; min-width: 250px; margin-left: -125px; background-color: #07D;"
        "color: #fff;  text-align: center;  border-radius: 4px;  padding: 16px;  position: fixed;"
        "z-index: 1; left: 282px; bottom: 30%;  font-size: 17px;  border-style: solid; border-width: 1px; border-color: gray;}"
    "#toastmessage.show {visibility: visible; -webkit-animation: fadein 0.5s, fadeout 0.5s 2.5s; animation: fadein 0.5s, fadeout 0.5s 2.5s; }"
    // fade in
    "@-webkit-keyframes fadein {from {bottom: 20%; opacity: 0;} to {bottom: 30%; opacity: 0.9;} }"
    "@keyframes fadein {from {bottom: 20%; opacity: 0;} to {bottom: 30%; opacity: 0.9;} }"
    // fade out
    "@-webkit-keyframes fadeout {from {bottom: 30%; opacity: 0.9;} to {bottom: 0; opacity: 0;} }"
    "@keyframes fadeout {from {bottom: 30%; opacity: 0.9;} to {bottom: 0; opacity: 0;} }"
    // web log viewer and log levels
    //low level? Not used? white
    ".level_0 { color: #F1F1F1; }"
    //ERROR yellow
    ".level_1 { color: #FCFF95; }"
    //INFO blue
    ".level_2 { color: #9DCEFE; }"
    //DEBUG green
    ".level_3 { color: #A4FC79; }"
    //DEBUG_MORE orange
    ".level_4 { color: #F2AB39; }"
    //DEBUG_DEV dark orange
    ".level_9 { color: #FF5500; }"
    //the cmd window
    ".logviewer {	color: #F1F1F1; background-color: #272727; 	font-family: 'Lucida Console', Monaco, monospace; "
                " height:  530px; max-width: 1000px; width: 80%; padding: 4px 8px;  overflow: auto;   border-style: solid; border-color: gray; }"
    // text textarea
    "textarea {max-width: 1000px; width:80%; padding: 4px 8px; font-family: 'Lucida Console', Monaco, monospace; }"
    "textarea:hover {background-color: #ccc; }"
    // tables
    "table.normal th {padding: 6px; background-color: #444; color: #FFF; border-color: #888; font-weight: bold; }"
    "table.normal td {padding: 4px; height: 30px;}"
    "table.normal tr {padding: 4px; }"
    "table.normal {color: #000; width: 100%; min-width: 420px; border-collapse: collapse; }"
    //every second row
    "table.multirow th {padding: 6px; background-color: #444; color: #FFF; border-color: #888; font-weight: bold; }"
    "table.multirow td {padding: 4px; text-align: center;  height: 30px;}"
    "table.multirow tr {padding: 4px; }"
      "table.multirow tr:nth-child(even){background-color: #DEE6FF; }"
    "table.multirow {color: #000; width: 100%; min-width: 420px; border-collapse: collapse; }"
    // inside a form
    ".note {color: #444; font-style: italic; }"
    //header with title and menu
    ".headermenu {position: fixed; top: 0; left: 0; right: 0; height: 90px; padding: 8px 12px; background-color: #F8F8F8; border-bottom: 1px solid #DDD; z-index: 1;}"
    ".apheader {padding: 8px 12px; background-color: #F8F8F8;}"
    ".bodymenu {margin-top: 96px;}"
    // menu
    ".menubar {position: inherit; top: 55px; }"
    ".menu {float: left; padding: 4px 16px 8px 16px; color: #444; white-space: nowrap; border: solid transparent; border-width: 4px 1px 1px; border-radius: 4px 4px 0 0; text-decoration: none; }"
    ".menu.active {color: #000; background-color: #FFF; border-color: #07D #DDD #FFF; }"
    ".menu:hover {color: #000; background: #DEF; }"
    ".menu_button {display: none;}"
    // symbols for enabled
    ".on {color: green; }"
    ".off {color: red; }"
    // others
    ".div_l {float: left; }"
    ".div_r {float: right; margin: 2px; padding: 1px 10px; border-radius: 4px; background-color: #080; color: white; }"
    ".div_br {clear: both; }"
    // The alert message box
    ".alert {padding: 20px; background-color: #f44336; color: white; margin-bottom: 15px; }"
    ".warning {padding: 20px; background-color: #ffca17; color: white; margin-bottom: 15px; }"
    // The close button
    ".closebtn {margin-left: 15px; color: white; font-weight: bold; float: right; font-size: 22px; line-height: 20px; cursor: pointer; transition: 0.3s; }"
    // When moving the mouse over the close button
    ".closebtn:hover {color: black; }"
    "section{overflow-x: auto; width: 100%; }"
    // For screens with width less than 960 pixels
    "@media screen and (max-width: 960px) {"
//      "header:hover .menubar {display: block;}"
//      ".menu_button {display: block; text-align: center;}"
//      ".bodymenu{  margin-top: 0px;  }"
//      ".menubar{ display: none; top: 0px;   position: relative;   float: left;   width: 100%;}"
//      ".headermenu{  position: relative;   height: auto;   float: left;   width: 100%;   padding: 5px; z-index: 1;}"
//      ".headermenu h1{  padding: 8px 12px; }"
//      ".headermenu  a{  text-align: center;  width: 100%;  padding:7px 10px;  height: auto;   border: 0px;   border-radius:0px; }; }"
        "span.showmenulabel { display: none; }"
        ".menu { max-width: 11vw; max-width: 48px; }"

    "\0"
};


// JavaScript blobs



static const char jsReboot[] PROGMEM = {
  "<script>"
    "i=document.getElementById('rbtmsg');"
    "i.innerHTML=\"Please reboot: <input id='reboot' class='button link' value='Reboot' type='submit' onclick='r()'>\";"
    "var x = new XMLHttpRequest();"

    //done
    "function d(){"
      "i.innerHTML='';"
      "clearTimeout(t);"
    "}"


    //keep requesting mainpage until no more errors
    "function c(){"
      "i.innerHTML+='.';"
      "x.onload=d;"
      "x.open('GET', window.location.origin);"
      "x.send();"
    "}"

    //rebooting
    "function b(){"
      "i.innerHTML='Rebooting..';"
      "t=setInterval(c,2000);"
    "}"


    //request reboot
    "function r(){"
      "i.innerHTML+=' (requesting)';"
      "x.onload=b;"
      "x.open('GET', window.location.origin+'/?cmd=reboot');"
      "x.send();"
    "}"

  "</script>"
};

static const char jsToastMessageBegin[] PROGMEM = {
  "<script>"
  "function toasting() {"
    "var x = document.getElementById('toastmessage');"
    "x.innerHTML = '"
};

static const char jsToastMessageEnd[] PROGMEM = {
  "'; x.className = 'show';"
  " setTimeout(function(){x.innerHTML = '';"
  " x.className = x.className.replace('show', '');"
  " }, 2000);"
  "} </script>"
};

static const char jsClipboardCopyPart1[] PROGMEM = {
  "<script>function setClipboard() { var clipboard = '';"
  " max_loop = 100;"
  " for (var i = 1; i < max_loop; i++){"
  " var cur_id = '"
};

static const char jsClipboardCopyPart2[] PROGMEM = {
  "_' + i;"
  " var test = document.getElementById(cur_id);"
  " if (test == null){ i = max_loop + 1;"
  "  } else {"
  " clipboard += test.innerHTML.replace(/<[Bb][Rr]\\s*\\/?>/gim,'\\n') + '"
};

//Fix HTML
static const char jsClipboardCopyPart3[] PROGMEM = {
  "'; } }"
  "clipboard = clipboard.replace(/<\\/[Dd][Ii][Vv]\\s*\\/?>/gim,'\\n');"
  "clipboard = clipboard.replace(/<[^>]*>/gim,'');"
  "var tempInput = document.createElement('textarea');"
  "tempInput.style = 'position: absolute;"
  "left: -1000px; top: -1000px'; tempInput.innerHTML = clipboard;"
  "document.body.appendChild(tempInput);"
  "tempInput.select();"
  "document.execCommand('copy');"
  "document.body.removeChild(tempInput);"
  "alert('Copied: \"' + clipboard + '\" to clipboard!') }"
  "</script>"
};


static const char jsUpdateSensorValuesDevicePage[] PROGMEM = {
  "<script defer>"
  "loopDeLoop(1000, 0);"

  "function loopDeLoop(timeForNext, activeRequests) {"
        "const maximumRequests = 1;"
        "var c;"
        "var k;"
        "var err = '';"
        "const url = '/json?view=sensorupdate';"
        "var check = 0;"
        "if (isNaN(activeRequests)){activeRequests = maximumRequests;}"
      	"if (timeForNext == null){timeForNext = 1000;}"
      	"var i = setInterval(function() {"
          "if (check > 0) {"
              "clearInterval(i);"
              "return;"
              "}"
            "++activeRequests;"
            // to handle requests pending...
            "if (activeRequests > maximumRequests) {"
            		 "check = 1;"
            "} else {"
            "fetch(url).then(function(response) {"
                "var valueEntry;"
                "if (response.status !== 200) {"
                    "console.log('Looks like there was a problem. Status Code: ' + response.status);"
                    "return;"
                "}"
                "response.json().then(function(data) {"
                    "timeForNext = data.TTL;"
                    "for (c = 0; c < data.Sensors.length; c++) {"
                      "if (data.Sensors[c].hasOwnProperty('TaskValues')) {"
                        "for (k = 0; k < data.Sensors[c].TaskValues.length; k++) {"
                            "try {"
                                "valueEntry = data.Sensors[c].TaskValues[k].Value;"
                            "} catch (err) {"
                                "valueEntry = err.name;"
                            "} finally {"
                                "if (valueEntry !== 'TypeError') {"
                                    //to fix trailing zeros (fixed decimals)
                                    "tempValue = data.Sensors[c].TaskValues[k].Value;"
                                    "decimalsValue = data.Sensors[c].TaskValues[k].NrDecimals;"
                                    "tempValue = parseFloat(tempValue).toFixed(decimalsValue);"
                                    "document.getElementById('value_' + (data.Sensors[c].TaskNumber - 1) + '_' + (data.Sensors[c].TaskValues[k].ValueNumber - 1)).innerHTML = tempValue;"
                                    "document.getElementById('valuename_' + (data.Sensors[c].TaskNumber - 1) + '_' + (data.Sensors[c].TaskValues[k].ValueNumber - 1)).innerHTML = data.Sensors[c].TaskValues[k].Name + ':';"
                                "}"
                            "}"
                        "}"
                      "}"
                    "}"
                    "timeForNext = data.TTL;"
                    //"console.log('Next fetch in: ' + timeForNext + 'mSec, active requests: ' + activeRequests);"
                    "clearInterval(i);"
                    "loopDeLoop(timeForNext, 0);"
                    "return;"
                "});"
            "})"
            ".catch(function(err) {"
                "console.log(err.message);"
                "timeForNext = 5000;"
                "clearInterval(i);"
                "loopDeLoop(timeForNext, 0);"
                "return;"
            "});"
      	"check = 1;"
      	"}}, timeForNext);"
        "}"
    "</script>"
};

static const char jsFetchAndParseLog[] PROGMEM = {

  "<script defer>"
      "function getBrowser() {"
          "var ua=navigator.userAgent,tem,M=ua.match(/(opera|chrome|safari|firefox|msie|trident(?=\\/))\\/?\\s*(\\d+)/i) || [];"
          "if(/trident/i.test(M[1])){"
              "tem=/\\brv[ :]+(\\d+)/g.exec(ua) || [];"
              "return {name:'IE',version:(tem[1]||'')};"
              "}"
          "if(M[1]==='Chrome'){"
              "tem=ua.match(/\\bOPR|Edge\\/(\\d+)/);"
              "if(tem!=null)   {return {name:'Opera', version:tem[1]};}"
              "}"
          "M=M[2]? [M[1], M[2]]: [navigator.appName, navigator.appVersion, '-?'];"
          "if((tem=ua.match(/version\\/(\\d+)/i))!=null) {M.splice(1,1,tem[1]);}"
          "return {"
            "name: M[0],"
            "version: M[1]"
          "};"
       "}"
    "var browser=getBrowser();"
    "var currentBrowser = browser.name + browser.version;"
    //"console.log(browser.name + browser.version);"
    "if (browser.name = 'IE' && browser.version < 12) {"
      "textToDisplay = 'Error: ' + currentBrowser + ' is not supported! Please try a modern web browser.'"
    "} else {"
      "textToDisplay = 'Fetching log entries...';"
    "}"
    "document.getElementById('copyText_1').innerHTML = textToDisplay;"
    "loopDeLoop(1000, 0);"

    "const logLevel = new Array('Unused','Error','Info','Debug','Debug More','Undefined','Undefined','Undefined','Undefined','Debug Dev');"

    "function loopDeLoop(timeForNext, activeRequests) {"
      "const maximumRequests = 1;"
      "const url = '/logjson';"
      "if (isNaN(activeRequests)){activeRequests = maximumRequests;}"
    	"if (timeForNext == null){timeForNext = 1000;}"
      //to make sure we don't run to often... JS seems to like it that way.
      "if (timeForNext <= 500){"
          "scrolling_type = 'auto';"
        "} else {"
          "scrolling_type = 'smooth';"
        "}"
    	//"console.log('Next fetch in: ' + timeForNext + 'mSec');"
    	"var c;"
      "var logEntriesChunk;"
      "var currentIDtoScrollTo = '';"
      "var check = 0;"
    	"var i = setInterval(function() {"
      // to handle runs that didn't find any json
        "if (check > 0) {"
            "clearInterval(i);"
            "return;"
            "}"
        "++activeRequests;"
        // to handle requests pending...
        "if (activeRequests > maximumRequests) {"
        		 "check = 1;"
        "} else {"
    		"fetch(url).then(function(response) {"
    			"if (response.status !== 200) {"
    				"console.log('Looks like there was a problem. Status Code: ' + response.status);"
    				"return;"
    			"}"
    			"response.json().then(function(data) {"
            "var logEntry;"
            "if (logEntriesChunk == null) {"
              "logEntriesChunk = '';"
            "}"
    				"for (c = 0; c < data.Log.nrEntries; ++c) {"
    					"try {"
    						"logEntry = data.Log.Entries[c].timestamp;"
    					"} catch (err) {"
    						"logEntry = err.name;"
    					"} finally {"
    						"if (logEntry !== \"TypeError\") {"
                    "currentIDtoScrollTo = data.Log.Entries[c].timestamp;"
                    "logEntriesChunk += '<div class=level_' + data.Log.Entries[c].level + ' id=' + currentIDtoScrollTo + '><font color=\"gray\">' + data.Log.Entries[c].timestamp + ':</font> ' + data.Log.Entries[c].text + '</div>';"
    							"}"
    						"}"
    				"}"
    				"timeForNext = data.Log.TTL;"
            "if (logEntriesChunk !== '') {"
                "if (document.getElementById('copyText_1').innerHTML == 'Fetching log entries...') {"
                  "document.getElementById('copyText_1').innerHTML = '';"
                "}"
              "document.getElementById('copyText_1').innerHTML += logEntriesChunk;"
            "}"
            "logEntriesChunk = '';"
            "autoscroll_on = document.getElementById('autoscroll').checked;"
              "if (autoscroll_on == true && currentIDtoScrollTo !== '') {"
                "document.getElementById(currentIDtoScrollTo).scrollIntoView({"
                  "behavior: scrolling_type"
                  "});"
                "}"
            "document.getElementById('current_loglevel').innerHTML = 'Logging: ' + logLevel[data.Log.SettingsWebLogLevel] + ' (' + data.Log.SettingsWebLogLevel + ')';"
            //"console.log('Next fetch in: ' + timeForNext + 'mSec, active requests: ' + activeRequests);"
    				"clearInterval(i);"
    				"loopDeLoop(timeForNext, 0);"
    				"return;"
    			"})"
    		"})"
    	".catch(function(err) {"
    			"document.getElementById('copyText_1').innerHTML += '<div>>> ' + err.message + ' <<</div>';"
          "autoscroll_on = document.getElementById('autoscroll').checked;"
          "document.getElementById('copyText_1').scrollTop = document.getElementById('copyText_1').scrollHeight;"
          "timeForNext = 5000;"
          "clearInterval(i);"
          "loopDeLoop(timeForNext, 0);"
          "return;"
  			"})};"
  	"check = 1;"
  	"}, timeForNext);"
    "}"
    "</script>"
};

#endif // WEBSTATICDATA_h
