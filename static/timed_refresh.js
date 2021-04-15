function timedRefresh(timeoutPeriod) {
   var timer = setInterval(function() {
   if (timeoutPeriod > 0) {
       timeoutPeriod -= 1;
       document.getElementById('countdown').innerHTML = timeoutPeriod + '..' + '<br />';
   } else {
       clearInterval(timer);
            window.location.href = window.location.href;
       };
   }, 1000);
};
