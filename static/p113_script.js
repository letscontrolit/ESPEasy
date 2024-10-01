function p113_main() {
  function elId(e) {
    return document.getElementById(e);
  }
  function elVal(e) {
    return elId(e).value;
  }
  function getCel() {
    return vi.querySelectorAll('.cel');
  }
  function cntr(e) {
    if (elVal('roix') > 10 || elVal('roiy') > 10) return;
    let sc = [];
    getCel().forEach((c) => {
      const b = c.getBoundingClientRect();
      if (
        e.x >= b.left &&
        e.y >= b.top &&
        e.x - scx <= b.right &&
        e.y - scy <= b.bottom
      ) sc.push(c);
    });
    if (sc.length === 1) { elId('optc').value = parseInt(sc[0].textContent); upDsp(); }
  }
  const vi = elId('vi');
  const sl = elId('sL');
  let imd = 0;
  let ims = 0;
  let st = {},
    se = {};
  let sr = {};
  let oC = 199;
  let xr = 16;
  let yr = 16;
  let ptg = 0;
  let scx = vi.offsetWidth - vi.clientWidth;
  let scy = vi.offsetHeight - vi.clientHeight;
  let lck = 0;
  let sm = {
    w: 96,
    h: 96,
  };
  // console.log('p113_main', vi, sl);

  function clr() {
    getCel()
      .forEach((c) => {
        c.classList.remove('sel');
        c.classList.remove('oc');
      });
  }
  vi.addEventListener('mousedown', (e) => {
    if (e.altKey) {
      cntr(e);
    } else {
      lck = elId('lck').checked;
      if (imd && lck) {
        lck = 0;
        return;
      }
      imd = 1;
      if (!ims) {
        se = {
          x: null,
          y: null
        };
        clr();
      }
      st = {
        x: e.x + window.scrollX - vi.offsetLeft,
        y: e.y + window.scrollY - vi.offsetTop
      };
      // console.log('mousedown', e.x, e.y, st);
    }
  });
  vi.addEventListener('mouseup', (e) => {
    if (lck || !imd) return;
    imd = 0;
    const vc = vi.getBoundingClientRect();
    se = {
      x: e.x + window.scrollX - vi.offsetLeft,
      y: e.y + window.scrollY - vi.offsetTop
    };
    // console.log('mouseup', sr, se, vc.left, vc.top, window.scrollY);
    slt = sl.style;
    slt.top = `0px`;
    slt.left = `0px`;
    slt.bottom = 'unset';
    slt.right = 'unset';
    slt.visibility = 'hidden';
    if (
      Math.abs(se.x - st.x) < sm.w ||
      Math.abs(se.y - st.y) < sm.h
    ) { upDsp(); return; }
    gSelCls();
    pntSel(xr, yr, oC);
  });
  vi.addEventListener('dblclick', (e) => {
    lck = 0;
    imd = 0;
    cntr(e);
    // console.log('dblclick', e.x, e.y);
  });
  vi.addEventListener('mousemove', (e) => {
    if (!imd) return;
    se = {
      x: e.x + window.scrollX,
      y: e.y + window.scrollY
    };
    sr = {
      x1: Math.min(st.x, se.x),
      y1: Math.min(st.y, se.y),
      x2: Math.max(st.x, se.x) + scx,
      y2: Math.max(st.y, se.y) + scy
    };
    slt = sl.style;
    slt.top = `${scx + Math.min(
      sr.y1,
      sr.y2
    )}px`;
    slt.left = `${scy + Math.min(
      sr.x1,
      sr.x2
    )}px`;
    slt.bottom = `${vi.offsetHeight +
      vi.offsetTop -
      Math.max(sr.y1, sr.y2)
      }px`;
    slt.right = `${vi.offsetWidth +
      vi.offsetLeft -
      Math.max(sr.x1, sr.x2)
      }px`;
    slt.visibility = 'visible';
    if (!ims)
      clr();
  });
  function gSelCls() {
    let sc = [];
    sl.style.visibility = 'visible';
    getCel().forEach((c) => {
      const b = c.getBoundingClientRect();
      wx = window.scrollX;
      wy = window.scrollY;
      if (
        sr.x1 - wx <= ((b.left) - vi.offsetLeft + 16) &&
        sr.y1 - wy <= ((b.top) - vi.offsetTop + 16) &&
        (sr.x2 - wx - scx + 16) >= (b.right) &&
        (sr.y2 - wy - scy + 16) >= b.bottom
      ) sc.push(c);
    });
    if (sc.length > 0) {
      [ly, lx] = oiToXy(sc[0].textContent);
      [ry, rx] = oiToXy(sc[sc.length - 1].textContent);
      nx = lx + Math.floor((rx - lx) / 2 + 0.5);
      ny = ly + Math.floor((ry - ly) / 2);
      oC = gSP(ny, nx);
      xr = 1 + rx - lx;
      yr = 1 + ry - ly;
      // console.log('getSelectedCells', sc[0], sc[sc.length - 1], lx, ly, rx, ry, nx, ny, 'optc:', oC);
      ptg = 1;
      elId('roix').value = xr;
      elId('roiy').value = yr;
      elId('optc').value = oC;
      ptg = 0;
    }
    return sc;
  }
  function oiToXy(oidx) {
    x = 0;
    y = 0;
    o = parseInt(oidx);
    if (o < 128) {
      y += 8;
      o = 127 - o;
    } else {
      o -= 128;
    }
    x = o >> 3;
    y += o & 7;
    return [y, x];
  }
  function pntSel(roix, roiy, oCen) {
    if (!ptg) {
      lx = 0;
      ly = 0;
      rx = 15;
      ry = 15;
      oo = oCen;
      if (roix > 10 || roiy > 10) { oCen = 199; }
      [ocy, ocx] = oiToXy(oCen);
      hx = Math.floor(roix / 2);
      hy = Math.floor(roiy / 2);
      dx = roix % 2 != 0 ? 0 : 1;
      dy = roiy % 2 != 0 ? 0 : 1;
      if ((roix < 16) || (roiy < 16)) {
        lx = ocx - hx;
        rx = ocx + hx - dx;
        ly = ocy - hy + dy;
        ry = ocy + hy;
        if ((rx + 1) - (lx + 1) < 4) {
          d = 3 - ((rx + 1) - (lx + 1));
          if (rx < (15 - d)) {
            rx += d;
          } else {
            lx -= d;
            ocx -= d / 2;
          }
        }
        if (rx > 15 || lx < 0) {
          d = lx < 0 ? lx : rx - 15;
          rx -= d;
          lx -= d;
          ocx -= d;
        }
        if ((ry + 1) - (ly + 1) < 4) {
          d = 3 - ((ry + 1) - (ly + 1));
          if (ry < ((15 - d))) {
            ry += d;
          } else {
            ly -= d;
            ocy -= d / 2;
          }
        }
        if (ry > 15 || ly < 0) {
          d = ly < 0 ? ly : ry - 15;
          ry -= d;
          ly -= d;
          ocy -= d;
        }
      }
      // console.log('dsp oCen:', getSPAD(ocy, ocx), oCen);
      cs = getCel();
      for (y = 0; y < 16; ++y) {
        for (x = 0; x < 16; ++x) {
          cc = cs[y * 16 + x].classList;
          if (x >= lx && x <= rx && y >= ly && y <= ry)
            cc.add('sel');
          else
            cc.remove('sel');
          if (x == ocx && y == ocy)
            cc.add('oc');
          else
            cc.remove('oc');
        }
      }
      oc = gSP(ocy, ocx);
      if (oc != oo) {
        ptg = 1;
        elId('optc').value = oc;
        ptg = 0;
      }
    }
  }
  function upDsp() {
    xr = elVal('roix');
    yr = elVal('roiy');
    optc = elVal('optc');
    pntSel(xr, yr, optc);
  }
  p113_main.upDsp = upDsp;
  function gSP(y, x) {
    o = ((x & 15) << 3) + (y & 7);
    return (y > 7) ? 127 - o : 128 + o;
  }
  // Needed for multi-select
  // document.addEventListener('keydown', (e) => {
  //   if (e.ctrlKey) isMultiSelectionOn = true;
  // });
  // document.addEventListener('keyup', (e) => {
  //   if (!e.ctrlKey) isMultiSelectionOn = false;
  // });
  document.addEventListener('resize', () => {
    scx = vi.offsetWidth - vi.clientWidth;
    scy = vi.offsetHeight - vi.clientHeight;
  });
}
