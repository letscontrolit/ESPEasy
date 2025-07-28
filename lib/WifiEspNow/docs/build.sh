#!/bin/bash
set -euo pipefail
cd "$( dirname "${BASH_SOURCE[0]}" )"

doxygen Doxyfile 2>&1 | ./filter-Doxygen-warning.awk 1>&2

find html -name '*.html' | xargs sed -i '/<\/head>/ i\ <script>(function(e,t,n,i,s,a,c){e[n]=e[n]||function(){(e[n].q=e[n].q||[]).push(arguments)};a=t.createElement(i);c=t.getElementsByTagName(i)[0];a.async=true;a.src=s;c.parentNode.insertBefore(a,c)})(window,document,"galite","script","https://cdn.jsdelivr.net/npm/ga-lite@2/dist/ga-lite.min.js"); if (location.hostname.endsWith(".yoursunny.dev")) { galite("create", "UA-935676-14", "auto"); galite("send", "pageview"); }</script>'
cp _redirects html/
