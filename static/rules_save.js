function saveRulesFile() {
    'use strict';
    let size = document.getElementById('size');
    let ruleTextNew = document
        .getElementById('rules')
        .value;
    ruleTextNew = ruleTextNew.replace(/\\r?\\n/g, '\\r\\n');
    let n = document
        .getElementById('set')
        .value;
    let ruleTextFileData = new File([ruleTextNew], 'rules' + n + '.txt', {type: 'text/plain'});
    let formData = new FormData();
    formData.append('file', ruleTextFileData);
    formData.append('enctype', 'multipart/form-data');
    let url = '/rules' + n + '.txt?callback=' + Date.now();
    fetch(url)
        .then(size => size.text())
        .then(ruleTextFileData => {
            if (ruleTextNew === ruleTextFileData) 
                console.log('nothing to save...');
            else {
                setTimeout(() => fetch('/upload', {
                    method: 'POST',
                    body: formData,
                    mode: 'no-cors'
                }).then(size => size.text()).then(ruleTextFileData => {
                    setTimeout(() => fetch(url).then(size => size.text()).then(n => {
                        if (ruleTextNew === n) {
                            toasting();
                            size.innerHTML = ruleTextNew.length;
                        } else {
                            console.log('error when saving...');
                        }
                    }), 200);
                }), 100);
            }
        });
}
