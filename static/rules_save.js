function saveRulesFile() {
    "use strict";
    let size = document.getElementById("size");
    let ruleTextNew = document.getElementById("rules").value;
    ruleTextNew = ruleTextNew.replace(/\\r?\\n/g, "\\r\\n");
    let ruleNumber = document.getElementById("set").value;
    let ruleTextFileData = new File([ruleTextNew], "rules" + ruleNumber + ".txt", {
        type: "text/plain"
    });
    let formData = new FormData();
    formData.append("file", ruleTextFileData);
    formData.append("enctype", "multipart/form-data");
    let url = "/rules" + ruleNumber + ".txt?callback=" + Date.now();
    fetch(url).then(res => res.text()).then(ruleTextOld => {
        if (ruleTextNew === ruleTextOld) {
            console.log("nothing to save...");
        } else {
            fetch("/upload", {
                method: "POST",
                body: formData
            }).then(response => response.text()).then(l => {
                let url = "/rules" + ruleNumber + ".txt?callback=" + Date.now();
                fetch(url).then(res => res.text()).then(ruleTextNewCheck => {
                    if (ruleTextNew === ruleTextNewCheck) {
                        toasting();
                        size.innerHTML = ruleTextNew.length;
                    } else {
                        console.log("error when saving...");
                    }
                });
            });
        }
    });
}