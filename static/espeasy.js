//espeasy.js written by chromoxdor based on shell.js 
// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

var commonAtoms = ["And", "Or"];
var commonKeywords = ["If", "Else", "Elseif", "Endif"];
var commonCommands = ["AccessInfo", "Background", "Build", "ClearAccessBlock", "ClearRTCam", "Config", "ControllerDisable",
  "ControllerEnable", "DateTime", "Debug", "Dec", "DeepSleep", "DisablePriorityTask", "DNS", "DST", "EraseSDKWiFi", "ExecuteRules", "Gateway", "I2Cscanner", "Inc",
  "IP", "Let", "Load", "LogEntry", "LogPortStatus", "LoopTimerSet", "LoopTimerSet_ms", "MemInfo", "MemInfoDetail", "Name", "Password", "PostToHTTP", "Publish",
  "Reboot", "Reset", "Save", "SendTo", "SendToHTTP", "SendToUDP", "Settings", "Subnet", "Subscribe", "TaskClear", "TaskClearAll",
  "TaskDisable", "TaskEnable", "TaskRun", "TaskValueSet", "TaskValueSetAndRun", "TimerPause", "TimerResume", "TimerSet", "TimerSet_ms", "TimeZone",
  "UdpPort", "UdpTest", "Unit", "UseNTP", "WdConfig", "WdRead", "WiFi", "WiFiAPkey", "WiFiAllowAP", "WiFiAPMode", "WiFiConnect", "WiFiDisconnect", "WiFiKey",
  "WiFiKey2", "WiFiScan", "WiFiSSID", "WiFiSSID2", "WiFiSTAMode", "WiFi#Disconnected",
  "Event", "AsyncEvent",
  "GPIO", "GPIOToggle", "LongPulse", "LongPulse_mS", "Monitor", "Pulse", "PWM", "Servo", "Status", "Tone", "RTTTL", "UnMonitor",];
var commonString2 = ["Clock#Time", "Login#Failed", "MQTT#Connected", "MQTT#Disconnected", "MQTTimport#Connected", "MQTTimport#Disconnected", "Rules#Timer", "System#Boot",
  "System#BootMode", "System#Sleep", "System#Wake", "TaskExit#", "TaskInit#", "Time#Initialized", "Time#Set", "WiFi#APmodeDisabled", "WiFi#APmodeEnabled",
  "WiFi#ChangedAccesspoint", "WiFi#ChangedWiFichannel", "WiFi#Connected"];
var commonPlugins = [
  //P003
  "ResetPulseCounter", "SetPulseCounterTotal", "LogPulseStatistic",
  //P007
  "analogout",
  //P009
  "MCPGPIO", "MCPGPIOToggle", "MCPLongPulse", "MCPLongPulse_ms", "MCPPulse", "Status,MCP", "Monitor,MCP", "MonitorRange,MCP",
  "UnMonitorRange,MCP", "UnMonitor,MCP", "MCPGPIORange", "MCPGPIOPattern", "MCPMode", "MCPModeRange",
  //P012
  "LCDCmd", "LCD",
  //P019
  "PCFGPIO", "PCFGPIOToggle", "PCFLongPulse", "PCFLongPulse_ms", "PCFPulse", "Status,PCF", "Monitor,PCF",
  "MonitorRange,PCF", "UnMonitorRange,PCF", "UnMonitor,PCF", "PCFGPIORange", "PCFGPIOpattern", "PCFMode", "PCFmodeRange",
  //P022
  "pcapwm", "pcafrq", "mode2",
  //P023
  "OLED", "OLEDCMD", "OLEDCMD,on", "OLEDCMD,off", "OLEDCMD,clear",
  //P035
  "IRSEND", "IRSENDAC",
  //P036
  "OledFramedCmd", "OledFramedCmd,Display", "OledFramedCmd,Frame", "OledFramedCmd,linecount", "OledFramedCmd,leftalign",
  //P038
  "NeoPixel", "NeoPixelAll", "NeoPixelLine", "NeoPixelHSV", "NeoPixelAllHSV", "NeoPixelLineHSV", "NeoPixelBright",
  //P048
  "MotorShieldCmd,DCMotor", "MotorShieldCmd,Stepper",
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
  "7dn", "7dst", "7dsd", "7dtext", "7dtt", "7dt", "7dtfont", "7dtbin", "7don", "7doff", "7output",
  //P076
  "HLWCalibrate", "HLWReset",
  //P079
  "WemosMotorShieldCMD", "LolinMotorShieldCMD",
  //P082
  "GPS", "GPS,Sleep", "GPS,Wake", "GPS#GotFix", "GPS#LostFix", "GPS#Travelled",
  //P086
  "homieValueSet",
  //P088
  "HeatPumpir",
  //P093
  "MitsubishiHP", "MitsubishiHP,temperature", "MitsubishiHP,power", "MitsubishiHP,mode", "MitsubishiHP,fan", "MitsubishiHP,vane", "MitsubishiHP,widevane",
  //P094
  "Culreader_Write",
  //P099 & P123
  "Touch", "Touch,Rot", "Touch,Flip", "Touch,Enable", "Touch,Disable", "Touch,On", "Touch,Off", "Touch,Toggle", "Touch,Setgrp", "Touch,Ingrp", "Touch,Decgrp", "Touch,Incpage", "Touch,Decpage", "Touch,Updatebutton",
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
  //P129
  "ShiftIn", "ShiftIn,PinEvent", "ShiftIn,ChipEvent", "ShiftIn,SetChipCount", "ShiftIn,SampleFrequency", "ShiftIn,EventPerPin",
  //P127
  "cdmrst",
  //P137
  "axp,ldo2", "axp,ldo3", "axp,ldoio", "axp,gpio0", "axp,gpio1", "axp,gpio2", "axp,gpio3", "axp,gpio4", "axp,dcdc2", "axp,dcdc3", "axp,ldo2map", "axp,ldo3map", "axp,ldoiomap", "axp,dcdc2map", "axp,dcdc3map", "axp,ldo2perc", "axp,ldo3perc", "axp,ldoioperc", "axp,dcdc2perc", "axp,dcdc3perc",
  //P143
  "I2CEncoder", "I2CEncoder,bright", "I2CEncoder,led1", "I2CEncoder,led2", "I2CEncoder,gain", "I2CEncoder,set"
];
var pluginDispKind = [
  //P095
  "tft", "ili9341", "ili9342", "ili9481", "ili9486", "ili9488",
  //P096
  "eink", "epaper", "il3897", "uc8151d", "ssd1680", "ws2in7", "ws1in54",
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
  ",l", ",lh", ",lv", ",lm", ",lmr", ",r", ",rf", ",c", ",cf", ",rf", ",t", ",tf", ",rr", ",rrf", ",px", ",pxh", ",pxv", ",bmp", ",btn"
];
var commonTag = ["On", "Do", "Endon"];
var commonNumber = ["toBin", "toHex", "Constrain", "XOR", "AND:", "OR:", "Ord", "bitRead", "bitSet", "bitClear", "bitWrite", "urlencode"];
var commonMath = ["Log", "Ln", "Abs", "Exp", "Sqrt", "Sq", "Round", "Sin", "Cos", "Tan", "aSin", "aCos", "aTan", "Sin_d", "Cos_d", "Tan_d", "aSin_d", "aCos_d", "aTan_d"];
var commonWarning = ["delay", "Delay", "ResetFlashWriteCounter"];
var taskSpecifics = [
  //Task settings
  "settings.Enabled", "settings.Interval", "settings.ValueCount",
  "settings.Controller1.Enabled", "settings.Controller2.Enabled", "settings.Controller3.Enabled",
  "settings.Controller1.Idx", "settings.Controller2.Idx", "settings.Controller3.Idx"
];
//things that does not fit in any other catergory (for now)
var AnythingElse = [
  //System Variables
  "%eventvalue%", "%eventpar%", "%eventname%", "substring", "%sysname%", "%bootcause%", "%systime%", "%systm_hm%",
  "%systm_hm_0%", "%systm_hm_sp%", "%systime_am%", "%systime_am_0%", "%systime_am_sp%", "%systm_hm_am%", "%systm_hm_am_0%", "%systm_hm_am_sp%",
  "%lcltime%", "%sunrise%", "%s_sunrise%", "%m_sunrise%", "%sunset%", "%s_sunset%", "%m_sunset%", "%lcltime_am%",
  "%syshour%", "%syshour_0%", "%sysmin%", "%sysmin_0%", "%syssec%", "%syssec_0%", "%sysday%", "%sysday_0%", "%sysmonth%",
  "%sysmonth_0%", "%sysyear%", "%sysyear_0%", "%sysyears%", "%sysweekday%", "%sysweekday_s%", "%unixtime%", "%uptime%", "%uptime_ms%",
  "%rssi%", "%ip%", "%unit%", "%ssid%", "%bssid%", "%wi_ch%", "%iswifi%", "%vcc%", "%mac%", "%mac_int%", "%isntp%", "%ismqtt%",
  "%dns%", "%dns1%", "%dns2%", "%flash_freq%", "%flash_size%", "%flash_chip_vendor%", "%flash_chip_model%", "%fs_free%", "%fs_size%",
  "%cpu_id%", "%cpu_freq%", "%cpu_model%", "%cpu_rev%", "%cpu_cores%", "%board_name%",
  //Standard Conversions
  "%c_w_dir%", "%c_c2f%", "%c_ms2Bft%", "%c_dew_th%", "%c_alt_pres_sea%", "%c_sea_pres_alt%", "%c_cm2imp%", "%c_mm2imp%",
  "%c_m2day%", "%c_m2dh%", "%c_m2dhm%", "%c_s2dhms%", "%c_2hex%", "%c_u2ip%",
  //Variables
  "var", "int"
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

var EXTRAWORDS = commonAtoms.concat(commonPlugins, commonKeywords, commonCommands, commonString2, commonTag, commonNumber, commonMath, commonWarning, taskSpecifics, AnythingElse);

var rEdit;
function initCM() {
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
  //hinting on input
  rEdit.on("inputRead", function (cm, event) {
    var letters = /[\w%,.]/; //characters for activation
    var cur = cm.getCursor();
    var token = cm.getTokenAt(cur);
    if (letters.test(event.text) && token.type != "comment") {
      cm.showHint({ completeSingle: false });
    };
  });
}


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

    var lCcommonString2 = commonString2.map(name => name.toLowerCase());
    commonString2 = commonString2.concat(lCcommonString2);

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
    define('string-2', commonString2);
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
          return 'string-2';
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

