loopDeLoop(1000, 0);
function elId(e) {
  return document.getElementById(e);
}

function loopDeLoop(timeForNext, activeRequests) {
    var maximumRequests = 1;
    var c;
    var k;
    var err = '';
    var url = '/json?view=sensorupdate';
    var check = 0;
    if (isNaN(activeRequests)) {
        activeRequests = maximumRequests;
    }
    if (timeForNext == null) {
        timeForNext = 1000;
    }
    i = setInterval(function() {
        if (check > 0) {
            clearInterval(i);
            return;
        }
        ++activeRequests;
        if (activeRequests > maximumRequests) {
            check = 1;
        } else {
            fetch(url).then(function(response) {
                var valueEntry;
                if (response.status !== 200) {
                    console.log('Looks like there was a problem. Status Code: ' + response.status);
                    return;
                }
                response.json().then(function(data) {
                    timeForNext = data.TTL;
                    var showUoM = data.ShowUoM === undefined || (data.ShowUoM && data.ShowUoM !== 'false');
                    for (c = 0; c < data.Sensors.length; c++) {
                        if (data.Sensors[c].hasOwnProperty('TaskValues')) {
                            for (k = 0; k < data.Sensors[c].TaskValues.length; k++) {
                                try {
                                    valueEntry = data.Sensors[c].TaskValues[k].Value;
                                } catch (err) {
                                    valueEntry = err.name;
                                } finally {
                                    if (valueEntry !== 'TypeError') {
                                        var tempValue = data.Sensors[c].TaskValues[k].Value;
                                        var decimalsValue = data.Sensors[c].TaskValues[k].NrDecimals;
                                        if (decimalsValue < 255) {
                                          tempValue = parseFloat(tempValue).toFixed(decimalsValue);
                                        }
                                        var tempUoM = data.Sensors[c].TaskValues[k].UoM;
                                        var tempPres = data.Sensors[c].TaskValues[k].Presentation;
                                        if (tempPres) {
                                          tempValue = tempPres;
                                        } else
                                        if (tempUoM && showUoM) {
                                          tempValue += ' ' + tempUoM;
                                        }
                                        var valueID = 'value_' + (data.Sensors[c].TaskNumber - 1) + '_' + (data.Sensors[c].TaskValues[k].ValueNumber - 1);
                                        var valueNameID = 'valuename_' + (data.Sensors[c].TaskNumber - 1) + '_' + (data.Sensors[c].TaskValues[k].ValueNumber - 1);
                                        var valueElement = elId(valueID);
                                        var valueNameElement = elId(valueNameID);
                                        if (valueElement) {
                                            valueElement.innerHTML = tempValue;
                                        }
                                        if (valueNameElement) {
                                            valueNameElement.innerHTML = data.Sensors[c].TaskValues[k].Name + ':';
                                        }
                                    }
                                }
                            }
                        }
                    }
                    clearInterval(i);
                    loopDeLoop(timeForNext, 0);
                    return;
                });
            }).catch(function(err) {
                console.log(err.message);
                timeForNext = 5000;
                clearInterval(i);
                loopDeLoop(timeForNext, 0);
                return;
            });
            check = 1;
        }
    }, timeForNext);
}