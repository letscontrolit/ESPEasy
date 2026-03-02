//---------------no need to be included into WebStaticData.h---------------------------------------------
// if no filter input exists, create it
document.getElementById("logfilter") ||
    (document.querySelector("section").innerHTML +=
        '<label>Filter: <input id="logfilter" size="35"></label>');
//------------------------------------------------------------
function elId(e) {
    return document.getElementById(e);
}

const ct1 = elId('copyText_1');

if (!window.fetch) {
    ct1.textContent = 'Error: This browser is not supported. Please use a modern browser.';
} else {
    ct1.textContent = 'Fetching log entries...';
}

const logLevel = {
    0: 'Unused',
    1: 'Error',
    2: 'Info',
    3: 'Debug',
    4: 'Debug More',
    9: 'Debug Dev'
};

loopDeLoop(1000);

function loopDeLoop(timeForNext = 1000) {
    const url = '/logjson';

    setTimeout(function () {
        fetch(url)
            .then(response => {
                if (!response.ok) {
                    throw new Error(response.status);
                }
                return response.json();
            })
            .then(data => {
                let logEntriesChunk = '';

                for (let c = 0; c < data.Log.nrEntries; c++) {
                    const entry = data.Log.Entries[c];
                    if (!entry || !entry.timestamp) continue;

                    logEntriesChunk +=
                        `<div class="level_${entry.level}">
                            <font color="gray">${entry.timestamp}:</font> ${entry.text}
                        </div>`;
                }

                timeForNext = data.Log.TTL || timeForNext;

                if (logEntriesChunk) {
                    if (ct1.textContent === 'Fetching log entries...') {
                        ct1.innerHTML = '';
                    }
                    ct1.innerHTML += logEntriesChunk;
                }

                applyLogFilter();

                if (elId('autoscroll')?.checked) {
                    ct1.scrollTo({
                        top: ct1.scrollHeight,
                        behavior: timeForNext <= 500 ? 'auto' : 'smooth'
                    });
                }

                const level = data.Log.SettingsWebLogLevel;
                const levelName = logLevel[level] ?? 'Undefined';

                elId('current_loglevel').textContent =
                    `Logging: ${levelName} (${level})`;

                loopDeLoop(timeForNext);
            })
            .catch(err => {
                ct1.innerHTML += `<div>>> ${err.message} <<</div>`;
                ct1.scrollTop = ct1.scrollHeight;
                loopDeLoop(5000);
            });
    }, timeForNext);
}

function applyLogFilter() {
    const filterGroups = elId('logfilter').value
        .split(';').map(s => s.trim()).filter(Boolean);

    const entries = document.querySelectorAll('#copyText_1 > div');
    const hasPositiveFilter = filterGroups.some(g => !g.startsWith('!'));

    for (const entry of entries) {
        if (!entry.dataset.originalHtml) entry.dataset.originalHtml = entry.innerHTML;
        let html = entry.dataset.originalHtml;

        const textLower = entry.textContent.toLowerCase();
        let matched = [];
        let hiddenByExclusion = false;

        let isMatch = !hasPositiveFilter;

        for (const group of filterGroups) {

            // exclusion rule with ordered AND
            if (group.startsWith('!')) {
                const g = group.slice(1);

                if (g.includes('&') && !g.startsWith('&') && !g.endsWith('&')) {
                    const parts = g.split('&').map(s => s.trim()).filter(Boolean);
                    let lastIndex = -1;
                    let ok = true;

                    for (const p of parts) {
                        const idx = textLower.indexOf(p.toLowerCase(), lastIndex + 1);
                        if (idx === -1) {
                            ok = false;
                            break;
                        }
                        lastIndex = idx;
                    }

                    if (ok) {
                        hiddenByExclusion = true;
                        break;
                    }
                } else {
                    if (g && textLower.includes(g.toLowerCase())) {
                        hiddenByExclusion = true;
                        break;
                    }
                }
                continue;
            }

            // ordered AND: a&b&c
            if (group.includes('&') && !group.startsWith('&') && !group.endsWith('&')) {
                const parts = group.split('&').map(s => s.trim()).filter(Boolean);
                if (parts.length >= 2) {
                    let lastIndex = -1;
                    let ok = true;

                    for (const p of parts) {
                        const idx = textLower.indexOf(p.toLowerCase(), lastIndex + 1);
                        if (idx === -1) {
                            ok = false;
                            break;
                        }
                        lastIndex = idx;
                    }
                    if (ok) {
                        matched.push(...parts);
                        isMatch = true;
                        break;
                    }
                }
                continue;
            }
            // normal include
            if (textLower.includes(group.toLowerCase())) {
                matched.push(group);
                isMatch = true;
                break;
            }
        }
        if (hiddenByExclusion || !isMatch) {
            entry.style.display = 'none';
            continue;
        }
        entry.style.display = '';
        entry.innerHTML = highlightOutsideTags(html, matched);
    }
}

function highlightOutsideTags(html, terms) {
    if (!terms.length) return html;
    let result = '';
    let insideTag = false;
    let i = 0;

    while (i < html.length) {
        const char = html[i];
        if (char === '<') insideTag = true;
        if (char === '>') {
            insideTag = false;
            result += char;
            i++;
            continue;
        }

        if (!insideTag) {
            let matched = false;
            for (const term of terms) {
                const slice = html.slice(i, i + term.length);
                if (slice.toLowerCase() === term.toLowerCase()) {
                    result += `<span style="background-color: #bb9300;color: black;">${slice}</span>`;
                    i += term.length;
                    matched = true;
                    break;
                }
            }
            if (matched) continue;
        }
        result += char;
        i++;
    }
    return result;
}

const input = document.getElementById("logfilter");
input && (input.placeholder = '"!"=exclude ";"=or "&"=and (e.g. !act)');