function saveRulesFile() {
    'use strict';
    let e = document.getElementById('size');
    let t = document
        .getElementById('rules')
        .value;
    t = t.replace(/\\r?\\n/g, '\\r\\n');
    let n = document
        .getElementById('set')
        .value;
    let l = new File([t], 'rules' + n + '.txt', {type: 'text/plain'});
    let a = new FormData();
    a.append('file', l);
    a.append('enctype', 'multipart/form-data');
    let o = '/rules' + n + '.txt?callback=' + Date.now();
    fetch(o)
        .then(e => e.text())
        .then(l => {
            if (t === l) 
                console.log('nothing to save...');
            else {
                setTimeout(() => fetch('/upload', {
                    method: 'POST',
                    body: a,
                    mode: 'no-cors'
                }).then(e => e.text()).then(l => {
                    let a = '/rules' + n + '.txt?callback=' + Date.now();
                    setTimeout(() => fetch(a).then(e => e.text()).then(n => {
                        if (t === n) {
                            toasting();
                            e.innerHTML = t.length;
                        } else {
                            console.log('error when saving...');
                        }
                    }), 200);
                }), 100);
            }
        });
}
