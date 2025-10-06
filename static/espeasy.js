//espeasy.js written by chromoxdor based on shell.js 
// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

var commonAtoms = ["And", "Or"];
var commonKeywords = ["If", "Else", "Elseif", "Endif"];
var commonCommands = ["AccessInfo", "Background", "Build", "ClearAccessBlock", "ClearRTCam", "Config", "ControllerDisable",
  "ControllerEnable", "DateTime", "Debug", "Dec", "DeepSleep", "DisablePriorityTask", "DNS", "DST", "EraseSDKWiFi", "ExecuteRules", "FactoryReset", "Gateway", "I2Cscanner", "Inc",
  "IP", "Let", "LetStr", "Load", "LogEntry", "LogPortStatus", "LoopTimerSet", "LoopTimerSet_ms", "LoopTimerSetAndRun", "LoopTimerSetAndRun_ms", "MemInfo", "MemInfoDetail", "Name", "Password", "PostToHTTP", "Publish", "PublishR",
  "Reboot", "Save", "SendTo", "SendToHTTP", "SendToUDP", "SendToUDPMix", "Settings", "Subnet", "Subscribe", "TaskClear", "TaskClearAll",
  "TaskDisable", "TaskEnable", "TaskRun", "TaskValueSet", "TaskValueSetAndRun", "TaskValueSetDerived", "TaskValueSetPresentation", "TimerPause", "TimerResume", "TimerSet", "TimerSet_ms", "TimeZone",
  "UdpPort", "UdpTest", "Unit", "UseNTP", "WdConfig", "WdRead", "WiFi", "WiFiAllowAP", "WiFiAPMode", "WiFiConnect", "WiFiDisconnect", "WiFiKey",
  "WiFiKey2", "WiFiMode", "WiFiScan", "WiFiSSID", "WiFiSSID2", "WiFiSTAMode",
  "Event", "AsyncEvent",
  "GPIO", "GPIOToggle", "LongPulse", "LongPulse_mS", "Monitor", "Pulse", "PWM", "Servo", "Status", "Tone", "RTTTL", "UnMonitor",
  "Provision", "Provision,Config", "Provision,Security", "Provision,Notification", "Provision,Provision", "Provision,Rules", "Provision,CustomCdnUrl", "Provision,Firmware"];
var commonEvents = ["Clock#Time", "Login#Failed", "MQTT#Connected", "MQTT#Disconnected", "MQTTimport#Connected", "MQTTimport#Disconnected", "Rules#Timer", "System#Boot",
  "System#BootMode", "System#Sleep", "System#Wake", "TaskExit#", "TaskInit#", "ThingspeakReply", "Time#Initialized", "Time#Set", "WiFi#APmodeDisabled", "WiFi#APmodeEnabled",
  "WiFi#ChangedAccesspoint", "WiFi#ChangedWiFichannel", "WiFi#Connected", "WiFi#Disconnected"];
var commonPlugins = [
  //P003
  "ResetPulseCounter", "SetPulseCounterTotal", "LogPulseStatistic",
  //P007
  "analogout",
  //P009
  "MCPGPIO", "MCPGPIOToggle", "MCPLongPulse", "MCPLongPulse_ms", "MCPPulse", "Status,MCP", "Monitor,MCP", "MonitorRange,MCP",
  "UnMonitorRange,MCP", "UnMonitor,MCP", "MCPGPIORange", "MCPGPIOPattern", "MCPMode", "MCPModeRange",
  //P011
  "ExtGpio", "ExtPwm", "ExtPulse", "ExtLongPulse", "Status,EXT,",
  //P012
  "LCDCmd", "LCD",
  //P019
  "PCFGPIO", "PCFGPIOToggle", "PCFLongPulse", "PCFLongPulse_ms", "PCFPulse", "Status,PCF", "Monitor,PCF",
  "MonitorRange,PCF", "UnMonitorRange,PCF", "UnMonitor,PCF", "PCFGPIORange", "PCFGPIOpattern", "PCFMode", "PCFmodeRange",
  //P020/P044
  "SerialSend", "SerialSendMix", "Ser2NetClientSend", "SerialSend_test",
  //P022
  "pcapwm", "pcafrq", "mode2",
  //P023
  "OLED", "OLEDCMD", "OLEDCMD,on", "OLEDCMD,off", "OLEDCMD,clear",
  //P035
  "IRSEND", "IRSENDAC",
  //P036
  "OledFramedCmd", "OledFramedCmd,Display", "OledFramedCmd,low", "OledFramedCmd,med", "OledFramedCmd,high", /*"OledFramedCmd,user",*/ "OledFramedCmd,Frame", "OledFramedCmd,linecount", "OledFramedCmd,leftalign", "OledFramedCmd,align", "OledFramedCmd,userDef1", "OledFramedCmd,userDef2",
  //P038
  "NeoPixel", "NeoPixelAll", "NeoPixelLine", "NeoPixelHSV", "NeoPixelAllHSV", "NeoPixelLineHSV", "NeoPixelBright",
  //P048
  "MotorShieldCmd,DCMotor", "MotorShieldCmd,Stepper",
  //P049
  "MHZCalibrateZero", "MHZReset", "MHZABCEnable", "MHZABCDisable",
  //P052
  "Sensair_SetRelay",
  //P053
  "PMSX003", "PMSX003,Wake", "PMSX003,Sleep", "PMSX003,Reset",
  //P059
  "encwrite",
  //P065
  "Play", "Vol", "Eq", "Mode", "Repeat",
  //P067
  "tareChanA", "tareChanB",
  //P073
  "7dn", "7dst", "7dsd", "7dtext", "7ddt", "7dt", "7dtfont", "7dtbin", "7don", "7doff", "7output",
  //P076
  "HLWCalibrate", "HLWReset",
  //P077
  "csecalibrate", "cseclearpulses", "csereset",
  //P079
  "WemosMotorShieldCMD", "LolinMotorShieldCMD",
  //P082
  "GPS", "GPS,Sleep", "GPS,Wake", "GPS#GotFix", "GPS#LostFix", "GPS#Travelled",
  //P086
  "homieValueSet",
  //P087
  "SerialProxy_Write", "SerialProxy_WriteMix", "SerialProxy_Test",
  //P088
  "HeatPumpir",
  //P093
  "MitsubishiHP", "MitsubishiHP,temperature", "MitsubishiHP,power", "MitsubishiHP,mode", "MitsubishiHP,fan", "MitsubishiHP,vane", "MitsubishiHP,widevane",
  //P094
  "Culreader_Write",
  //P099 & P123
  "Touch", "Touch,Rot", "Touch,Flip", "Touch,Enable", "Touch,Disable", "Touch,On", "Touch,Off", "Touch,Toggle", "Touch,Setgrp", "Touch,Incgrp", "Touch,Decgrp", "Touch,Incpage", "Touch,Decpage", "Touch,Updatebutton",
  //P101
  "WakeOnLan",
  //P104
  "DotMatrix", "DotMatrix,clear", "DotMatrix,update", "DotMatrix,size", "DotMatrix,txt", "DotMatrix,settxt", "DotMatrix,content", "DotMatrix,alignment", "DotMatrix,anim.in", "DotMatrix,anim.out", "DotMatrix,speed", "DotMatrix,pause", "DotMatrix,font", "DotMatrix,layout", "DotMatrix,inverted", "DotMatrix,specialeffect", "DotMatrix,offset", "DotMatrix,brightness", "DotMatrix,repeat", "DotMatrix,setbar", "DotMatrix,bar",
  //P109
  "Thermo", "Thermo,Up", "Thermo,Down", "Thermo,Mode", "Thermo,ModeBtn", "Thermo,Setpoint",
  //P115
  "Max1704xclearalert",
  //P116
  //P117
  "scdgetabc", "scdgetalt", "scdgettmp", "scdsetcalibration", "scdsetfrc", "scdgetinterval",
  //P124
  "multirelay", "multirelay,on", "multirelay,off", "multirelay,set", "multirelay,get", "multirelay,loop",
  //P126
  "ShiftOut", "ShiftOut,Set", "ShiftOut,SetNoUpdate", "ShiftOut,Update", "ShiftOut,SetAll", "ShiftOut,SetAllNoUpdate", "ShiftOut,SetAllLow", "ShiftOut,SetAllHigh", "ShiftOut,SetChipCount", "ShiftOut,SetHexBin",
  //P127
  "cdmrst",
  //P128
  "nfx", "nfx,off", "nfx,on", "nfx,dim", "nfx,line,", "nfx,hsvline,", "nfx,one,", "nfx,hsvone,", "nfx,all,", "nfx,rgb,", "nfx,fade,", "nfx,hsv,", "nfx,colorfade,", "nfx,rainbow", "nfx,kitt,", "nfx,comet,", "nfx,theatre,", "nfx,scan,", "nfx,dualscan,", "nfx,twinkle,", "nfx,twinklefade,", "nfx,sparkle,", "nfx,wipe,", "nfx,dualwipe", "nfx,fire", "nfx,fireflicker", "nfx,faketv", "nfx,simpleclock", "nfx,stop", "nfx,statusrequest", "nfx,fadetime,", "nfx,fadedelay,", "nfx,speed,", "nfx,count,", "nfx,bgcolor",
  //P129
  "ShiftIn", "ShiftIn,PinEvent", "ShiftIn,ChipEvent", "ShiftIn,SetChipCount", "ShiftIn,SampleFrequency", "ShiftIn,EventPerPin",
  //P135
  "scd4x", "scd4x,storesettings", "scd4x,facoryreset", "scd4x,selftest", "scd4x,setfrc,",
  //P137
  "axp", "axp,ldo2", "axp,ldo3", "axp,ldoio", "axp,gpio0", "axp,gpio1", "axp,gpio2", "axp,gpio3", "axp,gpio4", "axp,dcdc2", "axp,dcdc3", "axp,ldo2map", "axp,ldo3map", "axp,ldoiomap", "axp,dcdc2map", "axp,dcdc3map", "axp,ldo2perc", "axp,ldo3perc", "axp,ldoioperc", "axp,dcdc2perc", "axp,dcdc3perc",
  //P143
  "I2CEncoder", "I2CEncoder,bright", "I2CEncoder,led1", "I2CEncoder,led2", "I2CEncoder,gain", "I2CEncoder,set",
  //P146
  "cachereader", "cachereader,readpos", "cachereader,sendtaskinfo", "cachereader,flush",
  //P148
  "tm1621", "tm1621,write,", "tm1621,writerow,", "tm1621,voltamp,", "tm1621,energy,", "tm1621,celcius,", "tm1621,fahrenheit,", "tm1621,humidity,", "tm1621,raw,",
  //P152
  "dac", "dac,1", "dac,2",
  //P153
  "sht4x", "sht4x,startup",
  //P159
  "ld2410", "ld2410,factoryreset", "ld2410,logall",
  //P162
  "digipot", "digipot,reset", "digipot,shutdown", "digipot,",
  //P165
  "7dextra", "7dbefore", "7dgroup", "7digit", "7color", "7digitcolor", "7groupcolor",
  //P166
  "gp8403", "gp8403,volt,", "gp8403,mvolt,", "gp8403,range,", "gp8403,preset,", "gp8403,init,",
  //P167
  "sen5x", "sen5x,startclean", "sen5x,techlog,",
  //P169
  "as3935", "as3935,clearstats", "as3935,calibrate", "as3935,setgain,", "as3935,setnf,", "as3935,setwd,", "as3925,setsrej,",
  //P175(P053)
  /*"pmsx003", "pmsx003,wake", "pmsx003,sleep", "pmsx003,reset",*/
  //P178
  "lu9685", "lu9685,servo,", "lu9685,enable,", "lu9685,disable,", "lu9685,setrange,",
  //P180
  "geni2c", "geni2c,cmd,", "geni2c,exec,", "geni2c,log,",
];
var pluginDispKind = [
  //P095
  "tft", "ili9341", "ili9342", "ili9481", "ili9486", "ili9488",
  //P096
  "epd", "eink", "epaper", "il3897", "uc8151d", "ssd1680", "ws2in7", "ws1in54",
  //P116
  /*"tft",*/ "st77xx", "st7735", "st7789", "st7796",
  //P131
  "neomatrix", "neo",
  //P141
  /*"lcd",*/ "pcd8544",
];
var pluginDispCmd = [
  "cmd,on", "cmd,off", "cmd,clear", "cmd,backlight", "cmd,bright", "cmd,deepsleep", "cmd,seq_start", "cmd,seq_end", "cmd,inv", "cmd,rot",
  ",clear", ",rot", ",tpm", ",txt", ",txp", ",txz", ",txc", ",txs", ",txtfull", ",asciitable", ",font",
  ",l", ",lh", ",lv", ",lm", ",lmr", ",r", ",rf", ",c", ",cf", ",rf", ",t", ",tf", ",rr", ",rrf", ",px", ",pxh", ",pxv", ",bmp", ",btn",
  ",win", ",defwin", ",delwin",
];
var commonTag = ["On", "Do", "Endon"];
var commonNumber = ["toBin", "toHex", "Constrain", "XOR", "AND:", "OR:", "Ord", "bitRead", "bitSet", "bitClear", "bitWrite", "urlencode"];
var commonMath = ["Log", "Ln", "Abs", "Exp", "Sqrt", "Sq", "Round", "Sin", "Cos", "Tan", "aSin", "aCos", "aTan", "Sin_d", "Cos_d", "Tan_d", "aSin_d", "aCos_d", "aTan_d", "map", "mapc"];
var commonWarning = ["delay", "Delay", "ResetFlashWriteCounter"];
var taskSpecifics = [
  //Task settings
  "settings.Enabled", "settings.Interval", "settings.ValueCount",
  "settings.Controller1.Enabled", "settings.Controller2.Enabled", "settings.Controller3.Enabled",
  "settings.Controller1.Idx", "settings.Controller2.Idx", "settings.Controller3.Idx"
];
//things that do not fit in any other catergory (for now)
var AnythingElse = [
  //System Variables
  "%eventvalue%", "%eventpar%", "%eventname%", "%sysname%", "%bootcause%", "%systime%", "%systm_hm%",
  "%systm_hm_0%", "%systm_hm_sp%", "%systime_am%", "%systime_am_0%", "%systime_am_sp%", "%systm_hm_am%", "%systm_hm_am_0%", "%systm_hm_am_sp%",
  "%lcltime%", "%sunrise%", "%s_sunrise%", "%m_sunrise%", "%sunset%", "%s_sunset%", "%m_sunset%", "%lcltime_am%",
  "%syshour%", "%syshour_0%", "%sysmin%", "%sysmin_0%", "%syssec%", "%syssec_0%", "%sysday%", "%sysday_0%", "%sysmonth%",
  "%sysmonth_0%", "%systzoffset%", "%systzoffset_s%", "%sysyear%", "%sysyear_0%", "%sysyears%", "%sysweekday%", "%sysweekday_s%", "%unixtime%", "%unixtime_lcl%", "%uptime%", "%uptime_ms%",
  "%rssi%", "%ip%", "%unit%", "%unit_0%", "%ssid%", "%bssid%", "%wi_ch%", "%iswifi%", "%vcc%", "%mac%", "%mac_int%", "%isntp%", "%ismqtt%",
  "%dns%", "%dns1%", "%dns2%", "%flash_freq%", "%flash_size%", "%flash_chip_vendor%", "%flash_chip_model%", "%fs_free%", "%fs_size%",
  "%cpu_id%", "%cpu_freq%", "%cpu_model%", "%cpu_rev%", "%cpu_cores%", "%board_name%", "%inttemp%", "%islimited_build%", "%isvar_double%",
  //String Functions
  "substring", "lookup", "indexOf", "indexOf_ci", "equals", "equals_ci", "strtol", "timeToMin", "timeToSec",
  //Ethernet
  "%ethwifimode%", "%ethconnected%", "%ethduplex%", "%ethspeed%", "%ethstate%", "%ethspeedstate%",
  //Standard Conversions
  "%c_w_dir%", "%c_c2f%", "%c_ms2Bft%", "%c_dew_th%", "%c_alt_pres_sea%", "%c_sea_pres_alt%", "%c_cm2imp%", "%c_isnum%", "%c_mm2imp%",
  "%c_m2day%", "%c_m2dh%", "%c_m2dhm%", "%c_s2dhms%", "%c_ts2date%", "%c_ts2isodate%", "%c_ts2wday%", "%c_random%", "%c_2hex%", "%c_u2ip%", "%c_uname%", "%c_uage%", "%c_ubuild%", "%c_ubuildstr%",
  "%c_uload%", "%c_utype%", "%c_utypestr%", "%c_strf%",
  //Variables
  "var", "int", "str", "length"
];

//merging displayspecific commands of P095,P096,P116,P131 into commonPlugins
for (const element2 of pluginDispKind) {
  commonPlugins = commonPlugins.concat(element2);
}
for (const element2 of pluginDispKind) {
  for (const element3 of pluginDispCmd) {
    let mergedDisp = element2 + element3;
    commonPlugins = commonPlugins.concat(mergedDisp);
  }
}

var EXTRAWORDS = commonAtoms.concat(commonPlugins, commonKeywords, commonCommands, commonEvents, commonTag, commonNumber, commonMath, commonWarning, taskSpecifics, AnythingElse);

var rEdit;
var confirmR = true;
var android = /Android/.test(navigator.userAgent);

function initCM() {
  if (android) {
    if (confirm("Do you want to enable colored rules on your Android device?\nThis feature hasn't been fully tested yet and may still have some issues.\nIt is currently expected to work with Chrome, Firefox, and Vivaldi.\nPlease report any problems you encounter.")) {
      confirmR = true
    } else {
      confirmR = false
    }
  }
  if (confirmR) {
    CodeMirror.commands.autocomplete = function (cm) { cm.showHint({ hint: CodeMirror.hint.anyword }); }
    rEdit = CodeMirror.fromTextArea(document.getElementById('rules'), {
      tabSize: 2, indentWithTabs: false, lineNumbers: true, autoCloseBrackets: true,
      extraKeys: {
        'Ctrl-Space': 'autocomplete',
        Tab: (cm) => {
          if (cm.getMode().name === 'null') {
            cm.execCommand('insertTab');
          } else {
            if (cm.somethingSelected()) {
              cm.execCommand('indentMore');
            } else {
              cm.execCommand('insertSoftTab');
            }
          }
        },
        'Shift-Tab': (cm) => cm.execCommand('indentLess'),
      }
    });

    rEdit.on('change', function () { rEdit.save() });

    if (!android) {
      rEdit.on("inputRead", function (cm, event) {
        var letters = /[\w%,.]/; //characters for activation
        var cur = cm.getCursor();
        var token = cm.getTokenAt(cur);
        if (letters.test(event.text) && token.type != "comment") {
          cm.showHint({ completeSingle: false });
        };
      });
    }
    CodeMirror.keyMap.default["Ctrl-F"] = function (cm) {
      openFind(); // Inject your custom buttons
    };

    CodeMirror.keyMap.default["Cmd-F"] = function (cm) {
      openFind();
    };
    // Disable search-related keys to prevent conflics with other shortcuts
    CodeMirror.keyMap.default["Ctrl-G"] = disableShortcut;
    CodeMirror.keyMap.default["Cmd-G"] = disableShortcut;
    CodeMirror.keyMap.default["Shift-Ctrl-G"] = disableShortcut;
    CodeMirror.keyMap.default["Shift-Cmd-G"] = disableShortcut;
    CodeMirror.keyMap.default["Ctrl-H"] = disableShortcut;
    CodeMirror.keyMap.default["Cmd-H"] = disableShortcut;
    CodeMirror.keyMap.default["Shift-Ctrl-F"] = disableShortcut;
    CodeMirror.keyMap.default["Shift-Cmd-F"] = disableShortcut;
    CodeMirror.keyMap.default["Ctrl-Shift-R"] = disableShortcut;
    CodeMirror.keyMap.default["Cmd-Shift-R"] = disableShortcut;
  }
  function disableShortcut(cm) {
    // do nothing to disable
  }
}

//----------------------------------------------------------------------- add search and formatting options


function closeSearchDialog() {
  const dlg = document.querySelectorAll('.CodeMirror-dialog');
  if (dlg.length > 0) {
    dlg.forEach(d => d.remove());
    document.body.classList.remove('dialog-opened');
  }
  rEdit.execCommand('clearSearch');
}

function removeHighlight() {
  requestAnimationFrame(() => {
    const highlights = document.querySelectorAll('.search-next-highlight');
    highlights.forEach(el => el.classList.remove('search-next-highlight'));
  });
  console.log('Removing highlights');
}

let findDialogObserver = null; // Keep one observer

function openFind() {
  // Disconnect previous observer if it exists
  if (findDialogObserver) {
    document.querySelectorAll('.CodeMirror-dialog').forEach(d => d.remove());
    findDialogObserver.disconnect();
    findDialogObserver = null;
  }

  // Create a new observer
  findDialogObserver = new MutationObserver(() => {
    if (!document.querySelector('.CodeMirror-dialog')) {
      removeHighlight();
      findDialogObserver.disconnect();
      findDialogObserver = null;
    }
  });

  findDialogObserver.observe(document.body, {
    childList: true,
    subtree: true
  });

  clearSearchNextHighlight(rEdit);
  rEdit.execCommand('findPersistent');
  addFindButtons();
}

function clearSearchNextHighlight(cm) {
  removeHighlight();
  // Also clear any markText highlights if used
  if (cm.__searchNextHighlight) {
    cm.__searchNextHighlight.clear();
    cm.__searchNextHighlight = null;
  }
}

function addFindButtons() {
  const element = document.querySelector('.CodeMirror-selected');
  const dialog = document.querySelector('.CodeMirror-dialog');
  if (!dialog || dialog.querySelector('.search-button-group')) return;

  const buttons = [
    {
      title: 'Find Previous',
      symbol: '▲',
      action: () => rEdit.execCommand('findPersistentPrev')
    },
    {
      title: 'Find Next',
      symbol: '▼',
      action: () => rEdit.execCommand('findPersistentNext')
    },
    {
      title: 'Replace',
      symbol: 'Replace',
      action: () => {
        closeSearchDialog();
        rEdit.execCommand('replace');
        addFindButtons(); // Re-add buttons after replacing
      }
    },
    {
      title: 'Close',
      symbol: '❌',
      action: closeSearchDialog
    },
    {
      title: 'Help',
      symbol: '?',
      action: () => {
        alert(`Available shortcuts:
• Ctrl+F / Cmd+F: Open search
• Enter: Find next
• Shift+Enter: Find previous
• Use /re/ syntax for regex search`);
      }
    }
  ];

  buttons.forEach(({ title, symbol, action }) => {
    const btn = document.createElement('span');
    btn.title = title;
    btn.className = title.toLowerCase() === 'help' ? 'button help' : 'button';
    btn.innerHTML = symbol;
    btn.style.cssText = `
      cursor: pointer;
      user-select: none;
    `;
    btn.addEventListener('click', (e) => {
      e.preventDefault();
      action();
    });
    dialog.appendChild(btn);
  });
}

// Add Searchfield button 
document.addEventListener('DOMContentLoaded', () => {
  const form = document.getElementById('rulesselect');

  if (form) {
    if (confirmR) {
      // Add search help button to CodeMirror editor
      const btn3 = document.createElement('button');
      btn3.type = 'button';     // prevent form submission
      btn3.id = 'searchBtn';
      btn3.innerHTML = "&#128270;&#xFE0E;"; // magnifier
      btn3.style.padding = "2px 5px";
      btn3.className = 'button help'; // just the class, no inline style
      form.appendChild(btn3);


      btn3.addEventListener('click', () => {
        if (typeof rEdit !== 'undefined') {
          openFind();
        }
      });
    }

    // Add format document button
    const btn = document.createElement('button');
    btn.type = 'button';     // prevent form submission
    btn.id = 'formatBtn';
    btn.textContent = 'Format';
    btn.className = 'button'; // just the class, no inline style
    form.appendChild(btn);

    btn.addEventListener('click', () => {
      console.log('Format button clicked');
      triggerFormatting();
    });
  }

  // Add hint button to CodeMirror editor
  // const form2 = document.getElementsByClassName('CodeMirror')[0];
  // if (form2) {
  //   console.log('Form2 found', form2);
  //   const btn2 = document.createElement('button');
  //   btn2.type = 'button';     // prevent form submission
  //   btn2.id = 'hintBtn';
  //   btn2.innerHTML = "&#9776;&#xFE0E;"; // ☰︎
  //   btn2.className = 'button help';
  //   btn2.style.position = 'absolute';
  //   btn2.style.right = '0';
  //   btn2.style.top = '0';

  //   form2.appendChild(btn2);

  //   btn2.addEventListener('click', () => {
  //     if (typeof rEdit !== 'undefined') {
  //       rEdit.focus(); // Optional: ensure focus
  //       rEdit.execCommand('autocomplete'); // Show hint menu
  //     }
  //   });
  // }

  let charBuffer = "";
  let findTimer = null;

  // Global keydown handler
  document.addEventListener('keydown', function (e) {
    const key = e.key;

    // disabled to prevent conflics with other shortcuts
    // const isCtrlShiftF = e.ctrlKey && e.shiftKey && key.toLowerCase() === 'f';

    // // Ctrl + Shift + F triggers formatting
    // if (isCtrlShiftF) {
    //   e.preventDefault();
    //   console.log('Keyboard shortcut detected: Formatting...');
    //   triggerFormatting();
    //   return;
    // }

    // Keys that cancel character buffering
    const nonCharKeys = [
      "Backspace", "Delete", "ArrowLeft", "ArrowRight",
      "ArrowUp", "ArrowDown", "Enter", "Tab", "Escape",
      "Shift", "Control", "Alt", "Meta"
    ];

    if (nonCharKeys.includes(key) || key.length !== 1) {
      charBuffer = "";
    }
  });


  // Workaround of showing hints for Android devices
  if (android) {
    var wasKeydown = false; // Flag to track if keydown was triggered 

    // Works on Android for some keys
    rEdit.on("keydown", (cm, e) => {
      if (["Enter", "Backspace", " "].includes(e.key)) {
        charBuffer = "";
        //alert("Buffer cleared via keydown:", e.key);
      }
      //alert("Keydown event detected: " + e.key);
      wasKeydown = true;
    });
    let lastCharBuffer = "";

    const inputField = rEdit.getInputField();

    function handleTypedChar(e, isBeforeInput = false) {
      if (!rEdit.hasFocus() || !rEdit || !e.data || !wasKeydown) return;
      wasKeydown = false;
      const data = e.data;
      const doc = rEdit.getDoc();
      const cursor = doc.getCursor(); // AFTER inserted char
      const letters = /[\w%,.]/;
      const token = rEdit.getTokenAt(cursor);

      // Reset on space
      if (data === ' ') {
        console.log("Clearing buffer due to space");
        charBuffer = "";
        lastCharBuffer = "";
        return;
      }

      // Ignore unchanged input
      if (data === lastCharBuffer && charBuffer.length > 0) return;

      if (letters.test(data) && token.type !== "comment") {
        const lastChar = cursor.ch <= 1 ? data.slice(-1) : data;
        charBuffer += lastChar;
        lastCharBuffer = charBuffer;

        // Remove line number bleed-in
        if (charBuffer.startsWith(String(cursor.line + 1)) && cursor.ch === 0) {
          charBuffer = charBuffer.slice(String(cursor.line).length);
        }

        console.log("Buffer updated:", charBuffer);

        const insertPos = {
          line: cursor.line,
          ch: cursor.ch - charBuffer.length + 1
        };

        const insertAndHint = () => {
          doc.replaceRange(charBuffer, insertPos, cursor);
          rEdit.setCursor({
            line: insertPos.line,
            ch: insertPos.ch + charBuffer.length
          });
          rEdit.showHint({ completeSingle: false });
        };

        // For `input`, delay slightly to avoid race with native insert
        if (!isBeforeInput) {
          setTimeout(insertAndHint, 0);
        } else {
          insertAndHint();
        }

      } else {
        charBuffer = "";
      }
    }

    const ua = navigator.userAgent.toLowerCase();
    const isFirefox = /firefox/.test(ua);
    const isChrome = /chrome/.test(ua) && !isFirefox;
    // Firefox (Android) – beforeinput preferred
    if (isFirefox) {
      inputField.addEventListener("beforeinput", (e) => {
        e.preventDefault();
        handleTypedChar(e, true);
      });
    } else if (isChrome) {
      document.addEventListener("input", (e) => {
        handleTypedChar(e, false);
      });
    }

    rEdit.on('endCompletion', function () {
      setTimeout(() => {
        forceKeyboardOpen();
      }, 100); // small delay may help
    });
  }

  function forceKeyboardOpen() {
    const input = document.createElement("input");
    input.type = "text";
    input.style.position = "absolute";
    input.style.opacity = "0";
    input.style.height = "0";
    input.style.width = "0";
    input.style.border = "none";
    input.style.top = "0";
    input.style.left = "-9999";
    input.style.padding = "0";
    input.style.zIndex = "-1";
    input.style.fontSize = "16px"; // prevents zoom on iOS

    document.body.appendChild(input);
    input.focus();

    setTimeout(() => {
      input.remove();
      rEdit.focus(); // bring focus back to CodeMirror
    }, 10);
  }

});

function triggerFormatting() {
  let scrollInfo, cursor, currentLine, lineText, textarea;

  if (confirmR) {
    const doc = rEdit.getDoc();
    scrollInfo = rEdit.getScrollInfo();
    cursor = doc.getCursor();

    currentLine = cursor.ch === 0 ? cursor.line - 1 : cursor.line;
    lineText = rEdit.getLine(currentLine) || "";

    textarea = rEdit.getValue();
  } else {
    textarea = document.getElementById('rules').value;
  }

  // Apply transformations
  textarea = initalAutocorrection(textarea);
  textarea = formatLogic(textarea);

  if (confirmR) {
    rEdit.setValue(textarea);

    // Restore cursor
    const newLine = currentLine;
    const newCh = cursor.ch === 0 && lineText.length > 0
      ? lineText.length
      : cursor.ch;

    rEdit.setCursor({ line: newLine, ch: newCh });

    // Delay scroll restoration slightly to ensure rendering is complete
    setTimeout(() => {
      rEdit.scrollTo(scrollInfo.left, scrollInfo.top);
      rEdit.focus();
    }, 0);

    rEdit.save();
  } else {
    document.getElementById('rules').value = textarea;
  }
}

function initalAutocorrection(text) {
  for (const word of EXTRAWORDS) {
    if (word === "Do") {
      const pattern = /(^|\s)(do)(\s*)(\/\/.*)?$/gmi;
      text = text.replace(pattern, (match, p1, p2, p3, p4) => {
        return `${p1}Do${p3}${p4 ?? ""}`;
      });
    } else {
      const pattern = new RegExp(`^\\s*\\b${word}\\b`, "gmi");
      text = text.replace(pattern, (match) => {
        // Replace the matched word with its corrected casing
        return match.replace(new RegExp(word, 'i'), word);
      });
    }
  }
  return text;
}

function formatLogic(text) {
  const INDENT = '  ';
  const lines = text.split('\n').map(line => {
    const trimmed = line.trimStart(); // remove leading spaces only
    return trimmed.startsWith('//') ? line : trimmed;
  });
  const result = [];
  const errors = [];

  let insideOnBlock = false;
  let onStartLine = null;
  let currentIfStack = [];
  let currentIfLines = [];

  function isComment(line) {
    return line.trim().startsWith('//');
  }

  function isEmpty(line) {
    return line.trim() === '';
  }

  function isOnLine(line) {
    return line.trim().toLowerCase().startsWith('on');
  }

  function hasDo(line) {
    return line.trim().toLowerCase().endsWith('do');
  }

  function isEndonLine(line) {
    return line.trim().toLowerCase() === 'endon';
  }

  function isIf(line) {
    return line.trim().toLowerCase().startsWith('if');
  }

  function isElse(line) {
    return line.trim().toLowerCase() === 'else';
  }

  function isElseif(line) {
    return line.trim().toLowerCase().startsWith('elseif');
  }

  function isEndif(line) {
    return line.trim().toLowerCase() === 'endif';
  }

  let indentLevel = 0;

  function flushIfErrors() {
    if (currentIfStack.length > 0) {
      errors.push(`• Missing ${currentIfStack.length} Endif(s):`);
      errors.push(`     - Unclosed If block(s) starting at line(s): ${currentIfLines.join(', ')}`);
    }
    currentIfStack = [];
    currentIfLines = [];
  }

  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const trimmed = line.trim();

    // Comments
    if (isComment(trimmed)) {
      result.push(line);
      continue;
    }

    // Empty
    if (isEmpty(trimmed)) {
      result.push('');
      continue;
    }

    // === On Line ===
    if (isOnLine(trimmed)) {
      if (!hasDo(trimmed)) {
        errors.push(`• Line ${i + 1}: "On" statement must end with "Do"`);
        // still treat as valid On block so user can see rest of structure
      }

      if (insideOnBlock) {
        errors.push(`• Line ${i + 1}: Found "On..." before previous "On...Do" (line ${onStartLine + 1}) was closed with "Endon"`);
        flushIfErrors();
      }

      insideOnBlock = true;
      onStartLine = i;
      indentLevel = 1;
      result.push(trimmed);
      continue;
    }

    // === Endon Line ===
    if (isEndonLine(trimmed)) {
      if (!insideOnBlock) {
        errors.push(`• Line ${i + 1}: Found "Endon" without a matching "On...Do"`);
      } else {
        flushIfErrors();
        insideOnBlock = false;
        onStartLine = null;
        indentLevel = 0;
      }

      result.push(INDENT.repeat(indentLevel) + trimmed);
      continue;
    }

    // === Inside On Block ===
    if (insideOnBlock) {
      if (isIf(trimmed)) {
        result.push(INDENT.repeat(indentLevel) + trimmed);
        currentIfStack.push('if');
        currentIfLines.push(i + 1);
        indentLevel++;
        continue;
      }

      if (isElse(trimmed)) {
        console.log("Else found:", trimmed);
        if (currentIfStack.length === 0) {
          errors.push(`• Line ${i + 1}: "Else" without matching "If"`);
        } else {
          indentLevel = Math.max(indentLevel - 1, 0);
        }

        result.push(INDENT.repeat(indentLevel) + trimmed);
        indentLevel++;
        continue;
      }

      if (isElseif(trimmed)) {
        console.log("Elseif found:", trimmed);
        if (currentIfStack.length === 0) {
          errors.push(`• Line ${i + 1}: "Elseif" without matching "If"`);
        } else {
          indentLevel = Math.max(indentLevel - 1, 0);
        }

        result.push(INDENT.repeat(indentLevel) + trimmed);
        indentLevel++;
        continue;
      }

      if (isEndif(trimmed)) {
        if (currentIfStack.length === 0) {
          errors.push(`• Line ${i + 1}: "Endif" without matching "If"`);
        } else {
          indentLevel = Math.max(indentLevel - 1, 0);
          currentIfStack.pop();
          currentIfLines.pop();
        }

        result.push(INDENT.repeat(indentLevel) + trimmed);
        continue;
      }

      // Any other line inside a block
      result.push(INDENT.repeat(indentLevel) + trimmed);
      continue;
    }

    // === Outside any block ===
    result.push(trimmed);
  }

  // === Final checks ===
  if (insideOnBlock) {
    errors.push(`• Missing "Endon" for "On...Do" starting at line ${onStartLine + 1}`);
    flushIfErrors();
  }

  if (errors.length > 0) {
    const firstErrorLine = extractFirstErrorLine(errors);
    alert("Errors found:\n" + errors.join('\n'));
    if (!isNaN(firstErrorLine)) {
      if (confirmR) {
        setTimeout(() => {
          jumpToLine(firstErrorLine);
        }, 50);
      } else {
        const textareaR = document.getElementById('rules');
        setTimeout(() => {
          jumpToLineInTextarea(textareaR, firstErrorLine);
        }, 50);
      }
    }
  }

  return result.join('\n');
}

function jumpToLine(lineNumber) {
  const cmLine = Math.max(0, lineNumber - 1); // Convert to 0-based index
  rEdit.setCursor({ line: cmLine, ch: 0 }); // Place cursor at beginning of the line
  rEdit.focus();                            // Ensure editor is focused
  rEdit.scrollIntoView({ line: cmLine, ch: 0 }, 100);
}

function extractFirstErrorLine(errors) {
  for (const err of errors) {
    // Match: "• Line 5: ..."
    let match = err.match(/• Line (\d+)/);
    if (match) return parseInt(match[1]);

    // Match: "starting at line 2"
    match = err.match(/starting at line (\d+)/);
    if (match) return parseInt(match[1]);

    // Match: "starting at line(s): 4, 7, 9"
    match = err.match(/starting at line\(s\):\s*(\d+)/);
    if (match) return parseInt(match[1]);
  }
  return null;
}

function jumpToLineInTextarea(textarea, lineNumber) {
  const lines = textarea.value.split('\n');
  const clampedLine = Math.max(1, Math.min(lineNumber, lines.length)); // Clamp to valid range

  // Calculate the character offset to the start of the target line
  let offset = 0;
  for (let i = 0; i < clampedLine - 1; i++) {
    offset += lines[i].length + 1; // +1 for the newline character
  }

  // Move cursor and scroll into view
  textarea.focus();
  textarea.selectionStart = textarea.selectionEnd = offset;

  // Scroll to the selection
  textarea.scrollTop = textarea.scrollHeight; // Jump to bottom
  textarea.scrollTop = textarea.scrollTop - textarea.clientHeight / 2; // Center around selection
}
//--------------------------------------------------------------------------------- end of formatting option

(function (mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["codemirror"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function (CodeMirror) {
  "use strict";

  CodeMirror.defineMode('espeasy', function () {
    var words = {};
    function define(style, dict) {
      for (var i = 0; i < dict.length; i++) {
        words[dict[i]] = style;
      }
    };
    var lCcommonCommands = commonCommands.map(name => name.toLowerCase());
    commonCommands = commonCommands.concat(lCcommonCommands);

    var lCcommonString2 = commonEvents.map(name => name.toLowerCase());
    commonEvents = commonEvents.concat(lCcommonString2);

    var lCcommonPlugins = commonPlugins.map(name => name.toLowerCase());
    commonPlugins = commonPlugins.concat(lCcommonPlugins);

    var lCcommonAtoms = commonAtoms.map(name => name.toLowerCase());
    commonAtoms = commonAtoms.concat(lCcommonAtoms);

    var lCcommonKeywords = commonKeywords.map(name => name.toLowerCase());
    commonKeywords = commonKeywords.concat(lCcommonKeywords);

    var lCcommonTag = commonTag.map(name => name.toLowerCase());
    commonTag = commonTag.concat(lCcommonTag);

    var lCcommonNumber = commonNumber.map(name => name.toLowerCase());
    commonNumber = commonNumber.concat(lCcommonNumber);

    var lCcommonMath = commonMath.map(name => name.toLowerCase());
    commonMath = commonMath.concat(lCcommonMath);

    var lCAnythingElse = AnythingElse.map(name => name.toLowerCase());
    AnythingElse = AnythingElse.concat(lCAnythingElse);

    var lCtaskSpecifics = taskSpecifics.map(name => name.toLowerCase());
    taskSpecifics = taskSpecifics.concat(lCtaskSpecifics);

    define('atom', commonAtoms);
    define('keyword', commonKeywords);
    define('builtin', commonCommands);
    define('events', commonEvents);
    define('def', commonPlugins);
    define('tag', commonTag);
    define('number', commonNumber);
    define('bracket', commonMath);
    define('warning', commonWarning);
    define('hr', AnythingElse);
    define('comment', taskSpecifics);

    function tokenBase(stream, state) {
      if (stream.eatSpace()) return null;

      var sol = stream.sol();
      var ch = stream.next();

      if (/\d/.test(ch)) {
        if (ch == "0") {
          if (stream.next() === 'x') {
            stream.eatWhile(/\w/);
            return 'number';
          }
          else {
            stream.eatWhile(/\d|\./);
            return 'number';
          }
        }
        else {
          stream.eatWhile(/\d|\./);
          if (!stream.match("d") && !stream.match("output")) {
            if (stream.eol() || /\D/.test(stream.peek())) {
              return 'number';
            }
          }
        }
      }

      if (/\w/.test(ch)) {
        for (const element of EXTRAWORDS) {
          let WinDB = element.substring(1);
          if ((element.includes(":") || element.includes(",") || element.includes(".")) && stream.match(WinDB)) void (0)
        }
      }
      //P022 addition
      if (/\w/.test(ch)) {
        stream.eatWhile(/[\w]/);
        if (stream.match(".gpio") || stream.match(".pulse") || stream.match(".frq") || stream.match(".pwm")) {
          return 'def';
        }
      }

      if (ch === '\\') {
        stream.next();
        return null;
      }

      if (ch === '(' || ch === ')') {
        return "bracket";
      }

      if (ch === '{' || ch === '}' || ch === ':') {
        return "number";
      }

      if (ch == "/") {
        if (/\//.test(stream.peek())) {
          stream.skipToEnd();
          return "comment";
        }
        else {
          return 'operator';
        }
      }

      if (ch == "'") {
        stream.eatWhile(/[^']/);
        if (stream.match("'")) return 'attribute';
      }

      if (ch === '+' || ch === '=' || ch === '<' || ch === '>' || ch === '-' || ch === ',' || ch === '*' || ch === '!') {
        return 'operator';
      }

      if (ch == "%") {
        if (/\d/.test(stream.next())) { return 'number'; }
        else {
          stream.eatWhile(/[^\s\%]/);
          if (stream.match("%")) return 'hr';
        }
      }

      if (ch == "[") {
        stream.eatWhile(/[^\s\]]/);
        if (stream.eat("]")) return 'hr';
      }

      stream.eatWhile(/\w/);
      var cur = stream.current();

      if (/\w/.test(ch)) {
        if (stream.match("#")) {
          stream.eatWhile(/[\w.#]/);
          return 'events';
        }
      }

      if (ch === '#') {
        stream.eatWhile(/\w/);
        return 'number';
      }
      return words.hasOwnProperty(cur) ? words[cur] : null;
    }

    function tokenize(stream, state) {
      return (state.tokens[0] || tokenBase)(stream, state);
    };

    return {
      startState: function () { return { tokens: [] }; },
      token: function (stream, state) {
        return tokenize(stream, state);
      },
      //electricInput: /^\s*(\bendon\b)/i, //extra
      closeBrackets: "[]{}''\"\"``()",
      lineComment: '//',
      fold: "brace"
    };
  });
});

//------------------------closebrackets.js------------

(function (closeBrackets) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    closeBrackets(require("../../lib/codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod);
  else // Plain browser env
    closeBrackets(CodeMirror);
})(function (CodeMirror) {
  var defaults = {
    pairs: "()[]{}''\"\"",
    closeBefore: ")]}'\":;>",
    triples: "",
    explode: "[]{}"
  };

  var Pos = CodeMirror.Pos;

  CodeMirror.defineOption("autoCloseBrackets", false, function (cm, val, old) {
    if (old && old != CodeMirror.Init) {
      cm.removeKeyMap(keyMap);
      cm.state.closeBrackets = null;
    }
    if (val) {
      ensureBound(getOption(val, "pairs"))
      cm.state.closeBrackets = val;
      cm.addKeyMap(keyMap);
    }
  });

  function getOption(conf, name) {
    if (name == "pairs" && typeof conf == "string") return conf;
    if (typeof conf == "object" && conf[name] != null) return conf[name];
    return defaults[name];
  }

  var keyMap = { Backspace: handleBackspace, Enter: handleEnter };
  function ensureBound(chars) {
    for (var i = 0; i < chars.length; i++) {
      var ch = chars.charAt(i), key = "'" + ch + "'"
      if (!keyMap[key]) keyMap[key] = handler(ch)
    }
  }
  ensureBound(defaults.pairs + "`")

  function handler(ch) {
    return function (cm) { return handleChar(cm, ch); };
  }

  function getConfig(cm) {
    var deflt = cm.state.closeBrackets;
    if (!deflt || deflt.override) return deflt;
    var mode = cm.getModeAt(cm.getCursor());
    return mode.closeBrackets || deflt;
  }

  function handleBackspace(cm) {
    var conf = getConfig(cm);
    if (!conf || cm.getOption("disableInput")) return CodeMirror.Pass;

    var pairs = getOption(conf, "pairs");
    var ranges = cm.listSelections();
    for (var i = 0; i < ranges.length; i++) {
      if (!ranges[i].empty()) return CodeMirror.Pass;
      var around = charsAround(cm, ranges[i].head);
      if (!around || pairs.indexOf(around) % 2 != 0) return CodeMirror.Pass;
    }
    for (var i = ranges.length - 1; i >= 0; i--) {
      var cur = ranges[i].head;
      cm.replaceRange("", Pos(cur.line, cur.ch - 1), Pos(cur.line, cur.ch + 1), "+delete");
    }
  }

  function handleEnter(cm) {
    var conf = getConfig(cm);
    var explode = conf && getOption(conf, "explode");
    if (!explode || cm.getOption("disableInput")) return CodeMirror.Pass;

    var ranges = cm.listSelections();
    for (var i = 0; i < ranges.length; i++) {
      if (!ranges[i].empty()) return CodeMirror.Pass;
      var around = charsAround(cm, ranges[i].head);
      if (!around || explode.indexOf(around) % 2 != 0) return CodeMirror.Pass;
    }
    cm.operation(function () {
      var linesep = cm.lineSeparator() || "\n";
      cm.replaceSelection(linesep + linesep, null);
      moveSel(cm, -1)
      ranges = cm.listSelections();
      for (var i = 0; i < ranges.length; i++) {
        var line = ranges[i].head.line;
        cm.indentLine(line, null, true);
        cm.indentLine(line + 1, null, true);
      }
    });
  }

  function moveSel(cm, dir) {
    var newRanges = [], ranges = cm.listSelections(), primary = 0
    for (var i = 0; i < ranges.length; i++) {
      var range = ranges[i]
      if (range.head == cm.getCursor()) primary = i
      var pos = range.head.ch || dir > 0 ? { line: range.head.line, ch: range.head.ch + dir } : { line: range.head.line - 1 }
      newRanges.push({ anchor: pos, head: pos })
    }
    cm.setSelections(newRanges, primary)
  }

  function contractSelection(sel) {
    var inverted = CodeMirror.cmpPos(sel.anchor, sel.head) > 0;
    return {
      anchor: new Pos(sel.anchor.line, sel.anchor.ch + (inverted ? -1 : 1)),
      head: new Pos(sel.head.line, sel.head.ch + (inverted ? 1 : -1))
    };
  }

  function handleChar(cm, ch) {
    var conf = getConfig(cm);
    if (!conf || cm.getOption("disableInput")) return CodeMirror.Pass;

    var pairs = getOption(conf, "pairs");
    var pos = pairs.indexOf(ch);
    if (pos == -1) return CodeMirror.Pass;

    var closeBefore = getOption(conf, "closeBefore");

    var triples = getOption(conf, "triples");

    var identical = pairs.charAt(pos + 1) == ch;
    var ranges = cm.listSelections();
    var opening = pos % 2 == 0;

    var type;
    for (var i = 0; i < ranges.length; i++) {
      var range = ranges[i], cur = range.head, curType;
      var next = cm.getRange(cur, Pos(cur.line, cur.ch + 1));
      if (opening && !range.empty()) {
        curType = "surround";
      } else if ((identical || !opening) && next == ch) {
        if (identical && stringStartsAfter(cm, cur))
          curType = "both";
        else if (triples.indexOf(ch) >= 0 && cm.getRange(cur, Pos(cur.line, cur.ch + 3)) == ch + ch + ch)
          curType = "skipThree";
        else
          curType = "skip";
      } else if (identical && cur.ch > 1 && triples.indexOf(ch) >= 0 &&
        cm.getRange(Pos(cur.line, cur.ch - 2), cur) == ch + ch) {
        if (cur.ch > 2 && /\bstring/.test(cm.getTokenTypeAt(Pos(cur.line, cur.ch - 2)))) return CodeMirror.Pass;
        curType = "addFour";
      } else if (identical) {
        var prev = cur.ch == 0 ? " " : cm.getRange(Pos(cur.line, cur.ch - 1), cur)
        if (!CodeMirror.isWordChar(next) && prev != ch && !CodeMirror.isWordChar(prev)) curType = "both";
        else return CodeMirror.Pass;
      } else if (opening && (next.length === 0 || /\s/.test(next) || closeBefore.indexOf(next) > -1)) {
        curType = "both";
      } else {
        return CodeMirror.Pass;
      }
      if (!type) type = curType;
      else if (type != curType) return CodeMirror.Pass;
    }

    var left = pos % 2 ? pairs.charAt(pos - 1) : ch;
    var right = pos % 2 ? ch : pairs.charAt(pos + 1);
    cm.operation(function () {
      if (type == "skip") {
        moveSel(cm, 1)
      } else if (type == "skipThree") {
        moveSel(cm, 3)
      } else if (type == "surround") {
        var sels = cm.getSelections();
        for (var i = 0; i < sels.length; i++)
          sels[i] = left + sels[i] + right;
        cm.replaceSelections(sels, "around");
        sels = cm.listSelections().slice();
        for (var i = 0; i < sels.length; i++)
          sels[i] = contractSelection(sels[i]);
        cm.setSelections(sels);
      } else if (type == "both") {
        cm.replaceSelection(left + right, null);
        cm.triggerElectric(left + right);
        moveSel(cm, -1)
      } else if (type == "addFour") {
        cm.replaceSelection(left + left + left + left, "before");
        moveSel(cm, 1)
      }
    });
  }

  function charsAround(cm, pos) {
    var str = cm.getRange(Pos(pos.line, pos.ch - 1),
      Pos(pos.line, pos.ch + 1));
    return str.length == 2 ? str : null;
  }

  function stringStartsAfter(cm, pos) {
    var token = cm.getTokenAt(Pos(pos.line, pos.ch + 1))
    return /\bstring/.test(token.type) && token.start == pos.ch &&
      (pos.ch == 0 || !/\bstring/.test(cm.getTokenTypeAt(pos)))
  }
});

