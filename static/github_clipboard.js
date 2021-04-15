function setGithubClipboard() {
    var clipboard = 'ESP Easy | Information |\n -----|-----|\n';
    max_loop = 100;
    for (var i = 1; i < max_loop; i++) {
        var cur_id = 'copyText_' + i;
        var test = document.getElementById(cur_id);
        if (test == null) {
            i = max_loop + 1;
        } else {
            var separatorSymbol = '|';
            if (i % 2 == 0) {
                separatorSymbol += '\n'
            }
            clipboard += test.innerHTML.replace(/<[Bb][Rr]\s*\/?>/gim, '\n') + separatorSymbol;
        }
    }
    clipboard = clipboard.replace(/<\/[Dd][Ii][Vv]\s*\/?>/gim, '\n');
    clipboard = clipboard.replace(/<[^>]*>/gim, '');
    var tempInput = document.createElement('textarea');
    tempInput.style = 'position: absolute;left: -1000px; top: -1000px';
    tempInput.innerHTML = clipboard;
    document.body.appendChild(tempInput);
    tempInput.select();
    document.execCommand('copy');
    document.body.removeChild(tempInput);
    alert('Copied: "' + clipboard + '" to clipboard!')
}
