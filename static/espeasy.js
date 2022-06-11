// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE
function initCM() {
  CodeMirror.commands.autocomplete = function (cm) { cm.showHint({ hint: CodeMirror.hint.anyword }); }
  var rEdit = CodeMirror.fromTextArea(document.getElementById('rules'), { lineNumbers: true, extraKeys: { 'Alt-Space': 'autocomplete' } });
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

    var commonAtoms = ["And", "Or"];
    var commonKeywords = ["If", "Else", "Elseif", "Endif"];
    var commonCommands = ["AcessInfo", "Background", "Build", "ClearAccessBlock", "ClearRTCam", "Config", "ControllerDisable",
      "ControllerEnable", "DateTime", "Debug", "DeepSleep", "DNS", "DST", "EraseSDKwifi", "ExecuteRules", "Gateway", "I2Cscanner",
      "IP", "Let", "Load", "LogEntry", "LogPortStatus", "LoopTimerSet", "LoopTimerSet_ms", "MemInfo", "MemInfoDetail", "Name", "Password", "Publish",
      "Reboot", "Reset", "ResetFlashWriteCounter", "Save", "SendTo", "SendToHTTP", "SendToUDP", "Settings", "Subnet", "Subscribe", "TaskClear", "TaskClearAll",
      "TaskDisable", "TaskEnable", "TaskRun", "TaskValueSet", "TaskValueSetAndRun", "TimerPause", "TimerResume", "TimerSet", "TimerSet_ms", "TimeZone",
      "UdpPort", "UdpTest", "Unit", "UseNTP", "WdConfig", "WdRead", "WifiAPkey", "WifiAllowAP", "WifiAPMode", "WifiConnect", "WifiDisconnect", "WifiKey",
      "WifiKey", "WifiScan", "WifiSSID", "WifiSSID2", "WifiSTAMode",
      "Event", "AsyncEvent", "/control?cmd",
      "GPIO", "GPIOToggle", "LongPulse", "LongPulse_mS", "Monitor", "Pulse", "PWM", "Servo", "Status", "MCPGPIO", "MCPGPIOToggle", "MCPLongPulse",
      "MCPLongPulse_ms", "MCPPulse", "Status,MCP", "Monitor,MCP", "UnMonitor,MCP", "MonitorRange,MCP", "UnMonitorRange,MCP", "MCPGPIORange", "MCPGPIOPattern",
      "MCPMode", "MCPModeRange", "PCFGPIO", "PCFGPIOToggle", "PCFLongPulse", "PCFLongPulse_ms", "PCFPulse", "Status,PCF", "Monitor,PCF", "UnMonitor,PCF",
      "MonitorRange,PCF", "UnMonitorRange,PCF", "PCFGPIORange", "PCFMode", "Tone", "RTTTL", "UnMonitor",];
    var commonString2 = ["Clock#Time", "Login#Failed", "MQTT#Connected", "MQTT#Disconnected", "MQTTimport#Connected", "MQTTimport#Disconnected", "Rules#Timer", "System#Boot",
      "System#BootMode", "System#Sleep", "System#Wake", "TaskExit#", "TaskInit#", "Time#Initialized", "Time#Set", "WiFi#APmodeDisabled", "WiFi#APmodeEnabled",
      "WiFi#ChangedAccesspoint", "WiFi#ChangedWiFichannel", "WiFi#Connected"];
    var commonString3 = ["ResetPulseCounter", "SetPulseCounterTotal", "LogPulseStatistic", "LCDCmd", "LCD", "OledFramedCmd", "OledFramedCmd,Display", "OledFramedCmd,Frame",
      "MotorShieldCmd,DCMotor", "MotorShieldCmd,Stepper", "Sensair_SetRelay", "PMSX003,Wake", "PMSX003,Sleep", "PMSX003,Reset", "Play", "Vol", "Eq", "Mode", "Repeat",
      "HLWCalibrate", "HLWReset", "WemosMotorShieldCMD", "LolinMotorShieldCMD", "HeatPumpir", "MitsubishiHP,temperature", "MitsubishiHP,power", "MitsubishiHP,mode",
      "MitsubishiHP,fan", "MitsubishiHP,vane", "MitsubishiHP,widevane", "Culreader_write", "TFTcmd", "TFT&", "Touch,Rot", "WakeOnLan", "Max1704xclearalert"];
    var commonTag = ["On", "Do", "Endon"];
    var commonNumber = ["toBin", "toHex", "Constrain", "XOR", "AND:", "OR:", "Ord", "bitRead", "bitSet", "bitClear", "bitWrite", "urlencode"];
    var commonMath = ["Log", "Ln", "Abs", "Exp", "Sqrt", "Sq", "Round", "Sin", "Cos", "Tan", "aSin", "aCos", "aTan", "Sind_d", "Cos_d", "Tan_d", "aSin_d", "aCos_d", "sTan_d"];
    var commonWarning = ["delay", "Delay"]

    var lCcommonCommands = commonCommands.map(name => name.toLowerCase());
    commonCommands = commonCommands.concat(lCcommonCommands);

    var lCcommonString2 = commonString2.map(name => name.toLowerCase());
    commonString2 = commonString2.concat(lCcommonString2);

    var lCcommonString3 = commonString3.map(name => name.toLowerCase());
    commonString3 = commonString3.concat(lCcommonString3);

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

    CodeMirror.registerHelper("hintWords", "shell", commonAtoms.concat(commonKeywords, commonCommands));

    define('atom', commonAtoms);
    define('keyword', commonKeywords);
    define('builtin', commonCommands);
    define('hr', commonString2);
    define('def', commonString3);
    define('tag', commonTag);
    define('number', commonNumber);
    define('bracket', commonMath);
    define('warning', commonWarning);


    function tokenBase(stream, state) {
      if (stream.eatSpace()) return null;

      var sol = stream.sol();
      var ch = stream.next();

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

      if (/\d/.test(ch)) {
        stream.eatWhile(/\d|\./);
        if (stream.eol() || !/\d/.test(stream.peek())) {
          return 'number';
        }
      }

      if (ch == "%") {
        stream.eatWhile(/[^\s\%]/);
        if (stream.match("%")) return 'hr';
      }

      if (ch == "[") {
        stream.eatWhile(/[^\s\]]/);
        if (stream.eat("]")) return 'string-2';
      }

      if (/\w|\//.test(ch)) {
        for (const element of commonCommands) {
          let klausi = element.substring(1);
          if (stream.match(klausi)) void (0)
        }
        for (const element of commonString2) {
          let klausi = element.substring(1);
          if (stream.match(klausi)) void (0)
        }
        for (const element of commonString3) {
          let klausi = element.substring(1);
          if (stream.match(klausi)) void (0)
        }
        for (const element of commonNumber) {
          let klausi = element.substring(1);
          if (stream.match(klausi)) void (0)
        }
        for (const element of commonMath) {
          let klausi = element.substring(1);
          if (stream.match(klausi)) void (0)
        }
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


