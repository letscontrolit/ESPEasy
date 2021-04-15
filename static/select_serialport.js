document.getElementById('serPort').onload = function() {
  var element = document.getElementById('serPort');
  var event = new Event('change');
  element.dispatchEvent(event);
};

function serialPortChanged(elem){
  var style = elem.value == 0 ? '' : 'none';
  document.getElementById('tr_taskdevicepin1').style.display = style;
  document.getElementById('tr_taskdevicepin2').style.display = style;
}
