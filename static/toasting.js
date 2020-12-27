function toasting() {
    var x = document.getElementById('toastmessage');
    x.innerHTML = 'Submitted';
    x.className = 'show';
    setTimeout(function() {
        x.innerHTML = '';
        x.className = x.className.replace('show', '');
    }, 2000);
}
