i = document.getElementById('rbtmsg');
i.innerHTML = "Please reboot: <input id='reboot' class='button link' value='Reboot' type='submit' onclick='r()'>";
var x = new XMLHttpRequest();

//done
function d() {
    i.innerHTML = '';
    clearTimeout(t);
}


//keep requesting mainpage until no more errors
function c() {
    i.innerHTML += '.';
    x.onload = d;
    x.open('GET', window.location.origin);
    x.send();
}

//rebooting
function b() {
    i.innerHTML = 'Rebooting..';
    t = setInterval(c, 2000);
}


//request reboot
function r() {
    i.innerHTML += ' (requesting)';
    x.onload = b;
    x.open('GET', window.location.origin + '/?cmd=reboot');
    x.send();
}
