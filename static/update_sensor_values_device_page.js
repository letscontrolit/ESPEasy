loopDeLoop(1000);

function elId(e) {
    return document.getElementById(e);
}

const SENSOR_URL = '/json?view=sensorupdate';

function loopDeLoop(timeForNext = 1000) {
    setTimeout(function () {
        fetch(SENSOR_URL)
            .then(response => {
                if (!response.ok) {
                    throw new Error(response.status);
                }
                return response.json();
            })
            .then(data => {
                timeForNext = data.TTL || timeForNext;
                const showUoM = data.ShowUoM === undefined || (data.ShowUoM && data.ShowUoM !== 'false');

                for (let c = 0; c < data.Sensors.length; c++) {
                    const sensor = data.Sensors[c];
                    if (!sensor.TaskValues) continue;

                    for (let k = 0; k < sensor.TaskValues.length; k++) {
                        const taskValue = sensor.TaskValues[k];
                        if (!taskValue || taskValue.Value == null) continue;

                        let tempValue = taskValue.Value;
                        const decimals = taskValue.NrDecimals;

                        if (decimals < 255) {
                            tempValue = parseFloat(tempValue).toFixed(decimals);
                        }

                        if (taskValue.Presentation) {
                            tempValue = taskValue.Presentation;
                        } else if (taskValue.UoM && showUoM) {
                            tempValue += ' ' + taskValue.UoM;
                        }

                        const baseId = (sensor.TaskNumber - 1) + '_' + (taskValue.ValueNumber - 1);

                        const valueEl = elId('value_' + baseId);
                        if (valueEl) {
                            valueEl.innerHTML = tempValue;
                        }

                        const nameEl = elId('valuename_' + baseId);
                        if (nameEl) {
                            nameEl.innerHTML = taskValue.Name + ':';
                        }
                    }
                }

                loopDeLoop(timeForNext);
            })
            .catch(err => {
                console.log(err.message);
                loopDeLoop(5000);
            });
    }, timeForNext);
}