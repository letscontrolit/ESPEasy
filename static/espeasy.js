// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

var commonAtoms = ["And", "Or"];
var commonKeywords = ["If", "Else", "Elseif", "Endif"];
var commonCommands = ["AccessInfo", "Background", "Build", "ClearAccessBlock", "ClearRTCam", "Config", "ControllerDisable",
  "ControllerEnable", "DateTime", "Debug", "DeepSleep", "DNS", "DST", "EraseSDKWiFi", "ExecuteRules", "Gateway", "I2Cscanner",
  "IP", "Let", "Load", "LogEntry", "LogPortStatus", "LoopTimerSet", "LoopTimerSet_ms", "MemInfo", "MemInfoDetail", "Name", "Password", "Publish",
  "Reboot", "Reset", "ResetFlashWriteCounter", "Save", "SendTo", "SendToHTTP", "SendToUDP", "Settings", "Subnet", "Subscribe", "TaskClear", "TaskClearAll",
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
  "MCPGPIO", "MCPGPIOToggle", "MCPLongPulse", "MCPLongPulse_ms", "MCPPulse", "Status,MCP", "Monitor,MCP", "MonitorRange,MCP", "UnMonitorRange,MCP", "UnMonitor,MCP", "MCPGPIORange", "MCPGPIOPattern", "MCPMode", "MCPModeRange",
  //P012
  "LCDCmd", "LCD",
  //P019
  "PCFGPIO", "PCFGPIOToggle", "PCFLongPulse", "PCFLongPulse_ms", "PCFPulse", "Status,PCF", "Monitor,PCF",
  "MonitorRange,PCF", "UnMonitorRange,PCF", "UnMonitor,PCF", "PCFGPIORange", "PCFGPIOpattern", "PCFMode", "PCFmodeRange",
  //P036
  "OledFramedCmd", "OledFramedCmd,Display", "OledFramedCmd,Frame", "OledFramedCmd,linecount", "OledFramedCmd,leftalign",
  //P038
  "NeoPixel", "NeoPixelAll", "NeoPixelLine", "NeoPixelHSV", "NeoPixelAllHSV", "NeoPixelLineHSV",
  //P048
  "MotorShieldCmd,DCMotor", "MotorShieldCmd,Stepper",
  //P052
  "Sensair_SetRelay",
  //P053
  "PMSX003", "PMSX003,Wake", "PMSX003,Sleep", "PMSX003,Reset",
  //P065
  "Play", "Vol", "Eq", "Mode", "Repeat",
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
  "DotMatrix", "DotMatrix,clear", "DotMatrix,update", "DotMatrix,size", "DotMatrix,txt", "DotMatrix,settxt", "DotMatrix,content", "DotMatrix,alignment", "DotMatrix,anim.in", "DotMatrix,anim.out", "DotMatrix,speed", "DotMatrix,pause", "DotMatrix,font", "DotMatrix,layout", "DotMatrix,inverted", "DotMatrix,specialeffect", "DotMatrix,offset", "DotMatrix,brightness", "DotMatrix,repeat", "DotMatrix,bar", "DotMatrix,bar",
  //P115
  "Max1704xclearalert",
  //P116
  //P117
  "scdgetabc", "scdgetalt", "scdgettmp", "scdsetcalibration", "scdsetfrc", "scdgetinterval",
  //P124
  "multirelay", "multirelay,on", "multirelay,off", "multirelay,set", "multirelay,get", "multirelay,loop",
  //P126
  "ShiftOut", "ShiftOut,Set", "ShiftOut,SetNoUpdate", "ShiftOut,Update", "ShiftOut,SetAll", "ShiftOut,SetAllNoUpdate", "ShiftOut,SetAllLow", "ShiftOut,SetChipCount", "ShiftOut,SetHexBin",
  //P127
  "cdmrst"
];
var pluginDispKind = [
  //P095
  "tft", "ili9341", "ili9342", "ili9481", "ili9486", "ili9488",
  //P096
  "eink", "epaper", "il3897", "uc8151d", "ssd1680", "ws2in7", "ws1in54",
  //P116
  /*"tft",*/ "st77xx", "st7735", "st7789", "st7796",
  //P131
  "neomatrix", "neo"
];
var pluginDispCmd = [
  "cmd,on", "cmd,off", "cmd,clear", "cmd,backlight", "cmd,bright", "cmd,deepsleep", "cmd,seq_start", "cmd,seq_end", "cmd,inv", "cmd,rot",
  ",clear", ",rot", ",tpm", ",txt", ",txp", ",txz", ",txc", ",txs", ",txtfull", ",asciitable", ",font",
  ",l", ",lh", ",lv", ",lm", ",lmr", ",r", ",rf", ",c", ",cf", ",rf", ",t", ",tf", ",rr", ",rrf", ",px", ",pxh", ",pxv", ",bmp", ",btn"
];
var commonTag = ["On", "Do", "Endon"];
var commonNumber = ["toBin", "toHex", "Constrain", "XOR", "AND:", "OR:", "Ord", "bitRead", "bitSet", "bitClear", "bitWrite", "urlencode"];
var commonMath = ["Log", "Ln", "Abs", "Exp", "Sqrt", "Sq", "Round", "Sin", "Cos", "Tan", "aSin", "aCos", "aTan", "Sind_d", "Cos_d", "Tan_d", "aSin_d", "aCos_d", "sTan_d"];
var commonWarning = ["delay", "Delay"];
var AnythingElse = [];

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

var EXTRAWORDS = commonAtoms.concat(commonPlugins, commonKeywords, commonCommands, commonString2, commonTag, commonNumber, commonMath, commonWarning, AnythingElse);

function initCM() {
  CodeMirror.commands.autocomplete = function (cm) { cm.showHint({ hint: CodeMirror.hint.anyword }); }
  var rEdit = CodeMirror.fromTextArea(document.getElementById('rules'), { lineNumbers: true, extraKeys: { 'Ctrl-Space': 'autocomplete' } });
  rEdit.on('change', function () { rEdit.save() });
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

    define('atom', commonAtoms);
    define('keyword', commonKeywords);
    define('builtin', commonCommands);
    define('string-2', commonString2);
    define('def', commonPlugins);
    define('tag', commonTag);
    define('number', commonNumber);
    define('bracket', commonMath);
    define('warning', commonWarning);

    function tokenBase(stream, state) {
      if (stream.eatSpace()) return null;

      var sol = stream.sol();
      var ch = stream.next();

      if (/\d/.test(ch)) {
        if (!/d|o/.test(stream.peek())) {
        stream.eatWhile(/\d|\./);
        if (!stream.match("dt") && !stream.match("output")) {
          if (stream.eol() || /\D/.test(stream.peek())) {
            return 'number';
          }
        }
      }}

      if (/\w/.test(ch)) {
        for (const element of EXTRAWORDS) {
          let WinDB = element.substring(1);
          if (element.includes(",") && stream.match(WinDB)) void (0)
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
        else if (stream.match("control?cmd")) {
          return "builtin";
        }
        else {
          return 'string-2';
        }
      }

      if (ch === '+' || ch === '=' || ch === '<' || ch === '>' || ch === '-' || ch === ',' || ch === '*' || ch === '!') {
        return 'qualifier'
      }

      if (ch == "%") {
        stream.eatWhile(/[^\s\%]/);
        if (stream.match("%")) return 'hr';
      }

      if (ch == "[") {
        stream.eatWhile(/[^\s\]]/);
        if (stream.eat("]")) return 'string-2';
      }

      stream.eatWhile(/\w/);
      var cur = stream.current();
      if (stream.peek() === '#' && /\w/.test(cur)) return 'hr';
      if (ch === '#') { if (!/\w/.test(stream.peek()) && /[^#]/.test(cur)) return 'hr'; }
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
      closeBrackets: "[]{}''\"\"``",
      lineComment: '//',
      fold: "brace"
    };
  });
});


