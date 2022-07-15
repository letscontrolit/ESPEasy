// Script to split the pasted input over all fields with given class
// e.g.: split('$', ".query-input")
// Only splits on paste when checkbox with id 'splitpaste' is checked
// This will replace the newlines, tabs and spaces with '$' 
// and splits it over the fields with class 'query-input'

function split(sep, clazz) {

    var items = $(clazz);

    items.each(function (i) {

        $(this).on("paste", function () {
            var remember1 = document.getElementById('splitpaste');
            if (remember1.checked) {
              var me = $(this);
              setTimeout(function () {
                  var splitted = me.val().split(/\r?\n/).join(sep).split(/\s+/).join(sep).split(/\t/).join(sep).split(sep);
                  console.log(splitted);
                  items.each(function (i) {
                      $(this).val(splitted[i]);
                  });
              }, 1);
            }
        });
    })
}
split('$', ".query-input")