function getBrowser() {
    var ua = navigator.userAgent,
        tem, M = ua.match(/(opera|chrome|safari|firefox|msie|trident(?=\/))\/?\s*(\d+)/i) || [];
    if (/trident/i.test(M[1])) {
        tem = /\brv[ :]+(\d+)/g.exec(ua) || [];
        return {
            name: 'IE',
            version: (tem[1] || '')
        };
    }
    if (M[1] === 'Chrome') {
        tem = ua.match(/\bOPR|Edge\/(\d+)/);
        if (tem != null) {
            return {
                name: 'Opera',
                version: tem[1]
            };
        }
    }
    M = M[2] ? [M[1], M[2]] : [navigator.appName, navigator.appVersion, '-?'];
    if ((tem = ua.match(/version\/(\d+)/i)) != null) {
        M.splice(1, 1, tem[1]);
    }
    return {
        name: M[0],
        version: M[1]
    };
}
var browser = getBrowser();
var currentBrowser = browser.name + browser.version;
if (browser.name = 'IE' && browser.version < 12) {
    textToDisplay = 'Error: ' + currentBrowser + ' is not supported! Please try a modern web browser.'
} else {
    textToDisplay = 'Fetching log entries...';
}
document.getElementById('copyText_1').innerHTML = textToDisplay;
loopDeLoop(1000, 0);
var logLevel = new Array('Unused', 'Error', 'Info', 'Debug', 'Debug More', 'Undefined', 'Undefined', 'Undefined', 'Undefined', 'Debug Dev');

function loopDeLoop(timeForNext, activeRequests) {
    var maximumRequests = 1;
    var url = '/logjson';
    if (isNaN(activeRequests)) {
        activeRequests = maximumRequests;
    }
    if (timeForNext == null) {
        timeForNext = 1000;
    }
    if (timeForNext <= 500) {
        scrolling_type = 'auto';
    } else {
        scrolling_type = 'smooth';
    }
    var c;
    var logEntriesChunk;
    var currentIDtoScrollTo = '';
    var check = 0;
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
                if (response.status !== 200) {
                    console.log('Looks like there was a problem. Status Code: ' + response.status);
                    return;
                }
                response.json().then(function(data) {
                    var logEntry;
                    if (logEntriesChunk == null) {
                        logEntriesChunk = '';
                    }
                    for (c = 0; c < data.Log.nrEntries; ++c) {
                        try {
                            logEntry = data.Log.Entries[c].timestamp;
                        } catch (err) {
                            logEntry = err.name;
                        } finally {
                            if (logEntry !== "TypeError") {
                                currentIDtoScrollTo = data.Log.Entries[c].timestamp;
                                logEntriesChunk += '<div class=level_' + data.Log.Entries[c].level + ' id=' + currentIDtoScrollTo + '><font color="gray">' + data.Log.Entries[c].timestamp + ':</font> ' + data.Log.Entries[c].text + '</div>';
                            }
                        }
                    }
                    timeForNext = data.Log.TTL;
                    if (logEntriesChunk !== '') {
                        if (document.getElementById('copyText_1').innerHTML == 'Fetching log entries...') {
                            document.getElementById('copyText_1').innerHTML = '';
                        }
                        document.getElementById('copyText_1').innerHTML += logEntriesChunk;
                    }
                    logEntriesChunk = '';
                    autoscroll_on = document.getElementById('autoscroll').checked;
                    if (autoscroll_on == true && currentIDtoScrollTo !== '') {
                        document.getElementById(currentIDtoScrollTo).scrollIntoView({
                            behavior: scrolling_type
                        });
                    }
                    document.getElementById('current_loglevel').innerHTML = 'Logging: ' + logLevel[data.Log.SettingsWebLogLevel] + ' (' + data.Log.SettingsWebLogLevel + ')';
                    clearInterval(i);
                    loopDeLoop(timeForNext, 0);
                    return;
                })
            }).catch(function(err) {
                document.getElementById('copyText_1').innerHTML += '<div>>> ' + err.message + ' <<</div>';
                autoscroll_on = document.getElementById('autoscroll').checked;
                document.getElementById('copyText_1').scrollTop = document.getElementById('copyText_1').scrollHeight;
                timeForNext = 5000;
                clearInterval(i);
                loopDeLoop(timeForNext, 0);
                return;
            })
        };
        check = 1;
    }, timeForNext);
}