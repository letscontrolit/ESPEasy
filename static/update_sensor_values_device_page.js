loopDeLoop(1000, 0);

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
    var i = setInterval(function() {
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
                    for (c = 0; c < data.Sensors.length; c++) {
                        if (data.Sensors[c].hasOwnProperty('TaskValues')) {
                            for (k = 0; k < data.Sensors[c].TaskValues.length; k++) {
                                try {
                                    valueEntry = data.Sensors[c].TaskValues[k].Value;
                                } catch (err) {
                                    valueEntry = err.name;
                                } finally {
                                    if (valueEntry !== 'TypeError') {
                                        tempValue = data.Sensors[c].TaskValues[k].Value;
                                        decimalsValue = data.Sensors[c].TaskValues[k].NrDecimals;
                                        tempValue = parseFloat(tempValue).toFixed(decimalsValue);
                                        var valueID = 'value_' + (data.Sensors[c].TaskNumber - 1) + '_' + (data.Sensors[c].TaskValues[k].ValueNumber - 1);
                                        var valueNameID = 'valuename_' + (data.Sensors[c].TaskNumber - 1) + '_' + (data.Sensors[c].TaskValues[k].ValueNumber - 1);
                                        var valueElement = document.getElementById(valueID);
                                        var valueNameElement = document.getElementById(valueNameID);
                                        if (valueElement !== null) {
                                            valueElement.innerHTML = tempValue;
                                        }
                                        if (valueNameElement !== null) {
                                            valueNameElement.innerHTML = data.Sensors[c].TaskValues[k].Name + ':';
                                        }
                                    }
                                }
                            }
                        }
                    }
                    timeForNext = data.TTL;
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
