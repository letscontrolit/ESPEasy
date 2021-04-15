i = document.getElementById("rbtmsg"),
    i.innerHTML = "Please reboot: <input id='reboot' class='button link' value='Reboot' type='submit' onclick='r()'>";
var x = new XMLHttpRequest;

function d() {
    i.innerHTML = "",
        clearTimeout(t)
}

function c() {
    i.innerHTML += ".",
        x.onload = d,
        x.open("GET", window.location.origin),
        x.send()
}

function b() {
    i.innerHTML = "Rebooting..",
        t = setInterval(c, 2e3)
}

function r() {
    i.innerHTML += " (requesting)",
        x.onload = b,
        x.open("GET", window.location.origin + "/?cmd=reboot"),
        x.send()
}