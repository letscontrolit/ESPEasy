// Javascript functions for updating the Digit drawn in the P165 Device Configuration page as an 11x9 subtable
// Should be minified and loaded from CDN or filesystem
// Change-handlers start with 'ch', value/checked is the content, id is the table-id that should change
function elId(e) {
  return document.getElementById(e);
}
function elTR(e) {
  return elId(e)?.getElementsByTagName('TR');
}
function sh_col(tbl, col, show) { // Show/Hide column, minimal checks
  s = elTR(tbl);
  for (r of s) { // row of rows
    c = r?.children[col]; // Cell
    if (c?.tagName == 'TD') {
      c.style.width = 20;
      c.style.display = show ? 'inline-block' : 'none';
    }
  }
};
function sh_row(tbl, row, show) { // Show/Hide row, minimal checks
  r = elTR(tbl)[row];
  if (r) r.style.display = show ? 'block' : 'none';
};
function set_td(tbl, fill, row, col, td) { // Set <TD> content if fill is true, else non-breaking space, minimal checks
  c = elTR(tbl)[row].children[col];
  if (c?.tagName == 'TD') c.innerHTML = fill ? td : '&nbsp;';
};
// Next functions: 4th & 5th parameters are used only in 1 function, but to keep the calling logic simple, leave it there
function chWdth(value, base, count, max, colr) { // Change width
  for (t = base; t < base + count; t++)
    for (i = 1; i <= 7; i++)
      sh_col('dgtbl' + t, i, i <= value);
};
function chHght(value, base, count, max, colr) { // Change height
  for (t = base; t < base + count; t++) {
    b = 'dgtbl' + t; // Table
    for (i = 1; i < 8; i++) {
      sh_row(b, 8 - i, i <= value);
      sh_row(b, 8 + i, i <= value);
    };
  }
};
function chCrnr(checked, base, count, max, colr) { // Change Corner overlap
  if (colr) z = '<span style=\"color:' + colr + ';\">&#x2638;</span>'; else z = '&#x2638;';
  for (t = base; t < base + count; t++) {
    b = 'dgtbl' + t; // Table
    for (i = 0; i < 18; i += 8) {
      set_td(b, checked, i, 0, z); // P165_PIXEL_CHARACTER
      set_td(b, checked, i, 8, z);
    }
  }
};
function chDecp(value, base, count, max, colr) { // Change Decimal point pixels
  for (t = base; t < base + count; t++)
    set_td('dgtbl' + t, value > 0, 16, 9, '' + value);
};
function chAddn(value, base, count, max, colr) { // Change Additional pixels
  set_td('dxtbl' + base + count, value > 0 && max, 8, 0, '' + value);
};
function dgts(gps, flds) { // Update pixel counts per group and total pixels
  z = 0;
  for (q = 0; q < gps; q++) {
    v = [];
    for (f of flds) {
      v.push(parseInt(elId(f + q * 10).value));
    }
    c = elId('crnr' + q * 10).checked;
    x = v[5] * (3 * v[0] + 4 * v[1] + (c ? 6 : 0) + v[2]) + v[3];
    z += x + v[4];
    elId('totp' + q * 10).value = x;
  }
  elId('totpx').value = z;
};
function cbDis(c, s) { // change checkbox disabled state
  l = 'disabled';
  elId(c).disabled = s;
  c = elId('cs' + c);
  if (s)
    c.classList.add(l);
  else
    c.classList.remove(l);
}
function stSeg(e, g) {
  s = e.value == 2;
  ['spltg', 'dend'].forEach((a) => {
    cbDis(a + g, s)
  });
}
