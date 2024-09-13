#include "../PluginStructs/P113_data_struct.h"

#ifdef USES_P113

P113_data_struct::P113_data_struct(uint8_t i2c_addr, int timing, bool range) : i2cAddress(i2c_addr), timing(timing), range(range) {
  sensor = new (std::nothrow) SFEVL53L1X();
}

P113_data_struct::~P113_data_struct() {
  delete sensor;
}

// **************************************************************************/
// Initialize VL53L1X
// **************************************************************************/
bool P113_data_struct::begin(struct EventStruct *event) {
  initState = nullptr != sensor;

  if (initState) {
    const uint16_t id = sensor->getID();

    // FIXME 2023-08-11 tonhuisman: Disabled, as it seems to mess up the sensor
    // sensor->setI2CAddress(i2cAddress); // Initialize for configured address

    const bool res = sensor->begin();

    if (res) { // 0/false is NO-ERROR
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLogMove(LOG_LEVEL_ERROR, strformat(F("VL53L1X: Sensor not found, init failed for 0x%02x, id: 0x%04X status: %d"),
                                              i2cAddress, id, res));
      }
      initState = false;
      return initState;
    }

    sensor->setTimingBudgetInMs(timing);

    if (range) {
      sensor->setDistanceModeLong();
    } else {
      sensor->setDistanceModeShort();
    }

    # if P113_USE_ROI

    if (initState) {
      if ((0 != P113_ROI_X) && (0 != P113_ROI_Y) && (0 != P113_OPT_CENTER)) {
        sensor->setROI(P113_ROI_X, P113_ROI_Y, P113_OPT_CENTER);
      }
    }
    # endif // if P113_USE_ROI

    # ifdef P113_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, strformat(F("VL53L1X: Sensor initialized at address 0x%02x, id: 0x%04x"), i2cAddress, id));
    }
    # endif // ifdef P113_DEBUG
  }

  return initState;
}

bool P113_data_struct::startRead() {
  if (initState && !readActive && (nullptr != sensor)) {
    sensor->startRanging();
    readActive = true;
    distance   = -1;
  }
  return readActive;
}

bool P113_data_struct::readAvailable() {
  bool ready = (nullptr != sensor) && sensor->checkForDataReady();

  if (ready) {
    distance = sensor->getDistance();
    sensor->clearInterrupt();
    sensor->stopRanging();

    // readActive = false;
  }
  return ready;
}

uint16_t P113_data_struct::readDistance() {
  # ifdef P113_DEBUG_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, strformat(F("VL53L1X: idx: 0x%02x init: %d"), i2cAddress, initState));
  }
  # endif // P113_DEBUG_DEBUG

  success    = true;
  readActive = false;

  if (distance >= 8190u) {
    # ifdef P113_DEBUG_DEBUG
    addLog(LOG_LEVEL_DEBUG, concat(F("VL53L1X: NO MEASUREMENT"), distance));
    # endif // P113_DEBUG_DEBUG
    success = false;
  }

  # ifdef P113_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, strformat(F("VL53L1X: Address: 0x%02x / Timing: %d / Long Range: %d / Distance: %d"),
                                         i2cAddress, timing, range, distance));
  }
  # endif // P113_DEBUG

  return distance;
}

uint16_t P113_data_struct::readAmbient() {
  if (nullptr == sensor) {
    return 0u;
  }
  return sensor->getAmbientRate();
}

bool P113_data_struct::isReadSuccessful() {
  return success;
}

# if P113_USE_ROI
uint8_t P113_data_struct::getSPAD(uint8_t y, uint8_t x) {
  const uint8_t offset = ((x & 0x0F) << 3) + (y & 0x07);

  return (y > 7) ? 127 - offset : 128 + offset;
}

void P113_data_struct::opticalIndexToXy(uint8_t oi, uint8_t& y, uint8_t& x) {
  x = 0;
  y = 0;

  if (oi < 128) {
    y += 8;
    oi = 127 - oi;
  } else {
    oi -= 128;
  }
  x  = oi >> 3;
  y += oi & 0x7;
}

void P113_data_struct::drawSelectionArea(uint8_t roix, uint8_t roiy, uint8_t opticalCenter) {
  addHtml(F("<TD rowspan='16' style='padding:0;'>"));
  addHtml(F("<section id='vi'>"
            "<div id='sL'></div>"));
  uint8_t lx  = 0;
  uint8_t ly  = 0;
  uint8_t rx  = 15;
  uint8_t ry  = 15;
  uint8_t ocx = 0;
  uint8_t ocy = 0;

  P113_data_struct::opticalIndexToXy(opticalCenter, ocy, ocx);
  const uint8_t hx = roix / 2;
  const uint8_t hy = roiy / 2;
  const uint8_t dx = roix % 2 != 0 ? 0 : 1;
  const uint8_t dy = roiy % 2 != 0 ? 0 : 1;

  if ((roix < 16) || (roiy < 16)) {
    lx = ocx - hx;
    rx = ocx + hx - dx;
    ly = ocy - hy + dy;
    ry = ocy + hy;

    if ((rx + 1) - (lx + 1) < 4) { // check width >= 4 SPADs
      const uint8_t drx = 3 - ((rx + 1) - (lx + 1));

      if (rx < (15 - drx)) {
        rx += drx;
      } else {
        lx  -= drx;
        ocx -= drx / 2;
      }
    }

    if (rx > 15) { // do we fall off from the right?
      const uint8_t drx = rx - 15;
      rx  -= drx;
      lx  -= drx;
      ocx -= drx;
    }

    if ((ry + 1) - (ly + 1) < 4) { // check height >= 4 SPADs
      const uint8_t dry = 3 - ((ry + 1) - (ly + 1));

      if (ry < ((15 - dry))) {
        ry += dry;
      } else {
        ly  -= dry;
        ocy -= dry / 2;
      }
    }

    if (ry > 15) { // do we fall off from the bottom?
      const uint8_t dry = ry - 15;
      ry  -= dry;
      ly  -= dry;
      ocy -= dry;
    }
  }

  // addLog(LOG_LEVEL_INFO, strformat(F("drawSelectionArea E lx:%d ly:%d rx:%d ry:%d opt.center:%d ocx:%d ocy:%d hx:%d hy:%d dx:%d dy:%d"),
  //                                  lx, ly, rx, ry, opticalCenter, ocx, ocy, hx, hy, dx, dy));

  for (uint8_t y = 0; y < 16; ++y) {
    for (uint8_t x = 0; x < 16; ++x) {
      const bool sel = x >= lx && x <= rx && y >= ly && y <= ry;
      const bool oc  = x == ocx && y == ocy;

      addHtml(F("<div class='cel"));

      if (sel) { addHtml(F(" sel")); }

      if (oc) { addHtml(F(" oc")); }

      addHtml('\'', '>');
      addHtmlInt(getSPAD(y, x));
      addHtml(F("</div>"));
    }
  }

  addHtml(F("</section>"));
}

void P113_data_struct::loadJavascript() {
  html_add_script(false);
  addHtml(F( // TODO Move to WebStaticData.h
            "function p113_main(){"
            "function elId(e){return document.getElementById(e)}"
            "function elVal(e){return elId(e).value}"
            "function cntr(e){if(elVal('roix')>10||elVal('roiy')>10)return;let sc=[];const cs=vi.querySelectorAll('.cel');cs.forEach((c)=>{const b=c.getBoundingClientRect();if(e.x>=b.left&&e.y>=b.top&&e.x-scx<=b.right&&e.y-scy<=b.bottom)sc.push(c)});if(sc.length===1){elId('optc').value=parseInt(sc[0].textContent);upDsp()}}"
            "const vi=elId('vi');const sl=elId('sL');let imd=false;let ims=false;let st={},se={};let sr={};let oC=199;let xr=16;let yr=16;let ptg=0;let scx=vi.offsetWidth-vi.clientWidth;let scy=vi.offsetHeight-vi.clientHeight;let sm={w:96,h:96,};"
            "vi.addEventListener('mousedown',(e)=>{if(e.altKey){cntr(e)}else{imd=true;if(!ims){se={x:null,y:null};clr()}st={x:e.x+window.scrollX-vi.offsetLeft,y:e.y+window.scrollY-vi.offsetTop}}});"
            "vi.addEventListener('mouseup',(e)=>{if(!imd)return;imd=false;const vc=vi.getBoundingClientRect();se={x:e.x+window.scrollX-vi.offsetLeft,y:e.y+window.scrollY-vi.offsetTop};sl.style.top=`0px`;sl.style.left=`0px`;sl.style.bottom='unset';sl.style.right='unset';sl.style.visibility='hidden';if(Math.abs(se.x-st.x)<sm.w||Math.abs(se.y-st.y)<sm.h){upDsp();return}gSelCls();pntSel(xr,yr,oC)});"
            "vi.addEventListener('dblclick',(e)=>{cntr(e)});"
            "vi.addEventListener('mousemove',(e)=>{if(!imd)return;se={x:e.x+window.scrollX,y:e.y+window.scrollY};sr={x1:Math.min(st.x,se.x),y1:Math.min(st.y,se.y),x2:Math.max(st.x,se.x)+scx,y2:Math.max(st.y,se.y)+scy};sl.style.top=`${scx+Math.min(sr.y1,sr.y2)}px`;sl.style.left=`${scy+Math.min(sr.x1,sr.x2)}px`;sl.style.bottom=`${vi.offsetHeight+vi.offsetTop-Math.max(sr.y1,sr.y2)}px`;sl.style.right=`${vi.offsetWidth+vi.offsetLeft-Math.max(sr.x1,sr.x2)}px`;sl.style.visibility='visible';if(!ims)clr()});"
            "function clr(){vi.querySelectorAll('.cel').forEach((c)=>{c.classList.remove('sel');c.classList.remove('oc')})}"
            "function gSelCls(){let sc=[];const cl=vi.querySelectorAll('.cel');sl.style.visibility='visible';cl.forEach((c)=>{const b=c.getBoundingClientRect();wx=window.scrollX;wy=window.scrollY;if(sr.x1-wx<=((b.left)-vi.offsetLeft+16)&&sr.y1-wy<=((b.top)-vi.offsetTop+16)&&(sr.x2-wx-scx+16)>=(b.right)&&(sr.y2-wy-scy+16)>=b.bottom)sc.push(c)});if(sc.length>0){[ly,lx]=oiToXy(sc[0].textContent);[ry,rx]=oiToXy(sc[sc.length-1].textContent);nx=lx+Math.floor((rx-lx)/2+0.5);ny=ly+Math.floor((ry-ly)/2);oC=gSP(ny,nx);xr=1+rx-lx;yr=1+ry-ly;ptg=1;elId('roix').value=xr;elId('roiy').value=yr;elId('optc').value=oC;ptg=0}return sc}"
            "function oiToXy(oidx){x=0;y=0;o=parseInt(oidx);if(o<128){y+=8;o=127-o}else{o-=128}x=o>>3;y+=o&7;return[y,x]}"
            "function pntSel(roix,roiy,oCen){if(!ptg){lx=0;ly=0;rx=15;ry=15;oo=oCen;if(roix>10||roiy>10){oCen=199}[ocy,ocx]=oiToXy(oCen);hx=Math.floor(roix/2);hy=Math.floor(roiy/2);dx=roix%2!=0?0:1;dy=roiy%2!=0?0:1;if((roix<16)||(roiy<16)){lx=ocx-hx;rx=ocx+hx-dx;ly=ocy-hy+dy;ry=ocy+hy;if((rx+1)-(lx+1)<4){d=3-((rx+1)-(lx+1));if(rx<(15-d)){rx+=d}else{lx-=d;ocx-=d/2}}if(rx>15||lx<0){d=lx<0?lx:rx-15;rx-=d;lx-=d;ocx-=d}if((ry+1)-(ly+1)<4){d=3-((ry+1)-(ly+1));if(ry<((15-d))){ry+=d}else{ly-=d;ocy-=d/2}}if(ry>15||ly<0){d=ly<0?ly:ry-15;ry-=d;ly-=d;ocy-=d}}cl=vi.querySelectorAll('.cel');for(y=0;y<16;++y){for(x=0;x<16;++x){cc=cl[y*16+x].classList;if(x>=lx&&x<=rx&&y>=ly&&y<=ry)cc.add('sel');else cc.remove('sel');if(x==ocx&&y==ocy)cc.add('oc');else cc.remove('oc')}}oc=gSP(ocy,ocx);if(oc!=oo){ptg=1;elId('optc').value=oc;ptg=0}}}"
            "function upDsp(){xr=elVal('roix');yr=elVal('roiy');optc=elVal('optc');pntSel(xr,yr,optc)}p113_main.upDsp=upDsp;"
            "function gSP(y,x){o=((x&15)<<3)+(y&7);return(y>7)?127-o:128+o}document.addEventListener('resize',()=>{scx=vi.offsetWidth-vi.clientWidth;scy=vi.offsetHeight-vi.clientHeight})}"

            ));
  html_add_script_end();
}

void P113_data_struct::loadCss() {
  addHtml(F("<style type='text/css'>" // TODO Move to WebStaticData.h
            ":root{--bg113b:#bdaeeb45}[data-theme=dark]{--bg113b:#55304445}"
            "@media (prefers-color-scheme:dark){[data-theme=auto]{--bg113b:#55304445}}"
            "#vi{border:1px solid var(--c4);padding:0;overflow:auto;display:grid;grid-template-columns:repeat(16,2rem);gap:0;position:relative;-moz-user-select:-moz-none;-khtml-user-select:none;-webkit-user-select:none;-ms-user-select:none;user-select:none}"
            ".cel{width:2rem;height:2rem;border-right:1px solid var(--c4);border-bottom:1px solid var(--c4);text-align:center;line-height:2rem;vertical-align:middle;display:inline-block}"
            ".cel.sel{background-color:var(--bg5)}"
            ".cel.oc{font-style:italic;text-decoration:underline}"
            "#sL{position:absolute;top:0;left:0;border:1px solid var(--bg5);background-color:var(--bg113b);z-index:99999}"

            "</style>"));
}

# endif // if P113_USE_ROI

#endif // ifdef USES_P113
