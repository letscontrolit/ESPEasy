function saveRulesFile() {
  let button = document.getElementById('save_button');
  let size = document.getElementById('size');
  let ruleTextNew = document.getElementById('rules').value;
  ruleTextNew = ruleTextNew.replace(/\\r?\\n/g, '\\r\\n');
  let ruleNumber = document.getElementById('set').value;
  let ruleTextFileData = new File([ruleTextNew], 'rules' + ruleNumber + '.txt', {
      type: 'text/plain'
  });
  let formData = new FormData();
  formData.append('file', ruleTextFileData);
  formData.append('enctype', 'multipart/form-data');
  let url = '/rules' + ruleNumber + '.txt?callback=' + Date.now();
  fetch(url).then(res => res.text()).then((ruleTextOld) => {
      if (ruleTextNew === ruleTextOld) {
          console.log('nothing to save...');
      } else {
          let url = '/upload';
          fetch(url, {
              method: 'POST',
              body: formData
          }).then(
              response => response.text()).then(html => {
              let url = '/rules' + ruleNumber + '.txt?callback=' + Date.now();
              fetch(url).then(res => res.text()).then((ruleTextNewCheck) => {
                  if (ruleTextNew === ruleTextNewCheck) {
                      toasting();
                      size.innerHTML = ruleTextNew.length;
                  } else {
                      console.log('error when saving...');
                  }
              });
          });
      }
  });
};
