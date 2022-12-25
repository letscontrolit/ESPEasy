// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE
var isSame;


// This is from the file anyword-hint.js-------------------------------------------------------------------------
(function (anyword) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    anyword(require("codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["codemirror"], mod);
  else // Plain browser env
    anyword(CodeMirror);
})(function (CodeMirror) {
  "use strict";
  var WORD = /[\w?.,%]+/, RANGE = 500;
  CodeMirror.registerHelper("hint", "anyword", function (editor, options) {
    var word = options && options.word || WORD;
    var range = options && options.range || RANGE;
    var extraWords = options && options.extraWords || EXTRAWORDS;  //CXD
    var cur = editor.getCursor(), curLine = editor.getLine(cur.line);
    var end = cur.ch, start = end;
    while (start && word.test(curLine.charAt(start - 1))) --start;
    var curWord = start != end && curLine.slice(start, end);
    if (curWord) { curWord = curWord.toLowerCase(); }
    //autocorrect with space key and add a whitespace after the word
    //look if the typed word is in the extraWord list CXD
    isSame = extraWords.some(element => {
      return element.toLowerCase() === curWord;
    });
    var list = options && options.list || [], seen = {};
    var re = new RegExp(word.source, "gi");
    for (var dir = -1; dir <= 1; dir += 2) {
      var line = cur.line, endLine = Math.min(Math.max(line + dir * range, editor.firstLine()), editor.lastLine()) + dir;
      for (; line != endLine; line += dir) {
        var text = editor.getLine(line), m;
        text = text.replace(/\/{2}.*/g, ''); //filter out comments CXD
        text = text.replace(/(=|-|\+|\*|<|>)\d+/g, ''); //filter out numbers CXD
        var replaceK = text.match(/\w+(?:,(\S+)){2}/);
        if (replaceK) {text = text.replace(replaceK[1], ' ' + replaceK[1]);} //filter out everything after second comma
        //and put a space between CXD
        if (text.includes("#")){re = new RegExp(/[\w?#.,%]+/, "gi");} //regain old bahaviour of "word1#word2" suggestions
        while (m = re.exec(text)) {
          if (line == cur.line && m[0].toLowerCase() === curWord) continue;
          if ((!curWord || m[0].toLowerCase().lastIndexOf(curWord, 0) == 0) && !Object.prototype.hasOwnProperty.call(seen, m[0])) {
            seen[m[0]] = true;
            list.push(m[0]);
          }
        }
      }
    }
    list.sort();
    list.reverse();
    let tempList = new Map(list.map(s => [s.toLowerCase(), s]));
    list = [...tempList.values()];
    list.sort();
    var list2 = extraWords.filter(el => el.toLowerCase().startsWith(curWord || ''));
    list2.sort();
    list = list.concat(list2);
    let tempList2 = new Map(list.map(s => [s.toLowerCase(), s]));
    list = [...tempList2.values()];
    list.sort(function (a, b) {
      return a.toLowerCase().localeCompare(b.toLowerCase());
    });
    return { list: list, from: CodeMirror.Pos(cur.line, start), to: CodeMirror.Pos(cur.line, end) };
  });
});


// This is from the file show-hint.js-------------------------------------------------------------------------
(function (showHint) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    showHint(require("codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["codemirror"], mod);
  else // Plain browser env
    showHint(CodeMirror);
})(function (CodeMirror) {
  "use strict";
  var HINT_ELEMENT_CLASS = "CodeMirror-hint";
  var ACTIVE_HINT_ELEMENT_CLASS = "CodeMirror-hint-active";

  // This is the old interface, kept around for now to stay
  // backwards-compatible.
  CodeMirror.showHint = function (cm, getHints, options) {
    if (!getHints) return cm.showHint(options);
    if (options && options.async) getHints.async = true;
    var newOpts = { hint: getHints };
    if (options) for (var prop in options) newOpts[prop] = options[prop];
    return cm.showHint(newOpts);
  };

  CodeMirror.defineExtension("showHint", function (options) {
    options = parseOptions(this, this.getCursor("start"), options);
    var selections = this.listSelections()
    if (selections.length > 1) return;
    // By default, don't allow completion when something is selected.
    // A hint function can have a `supportsSelection` property to
    // indicate that it can handle selections.
    if (this.somethingSelected()) {
      if (!options.hint.supportsSelection) return;
      // Don't try with cross-line selections
      for (var i = 0; i < selections.length; i++)
        if (selections[i].head.line != selections[i].anchor.line) return;
    }

    if (this.state.completionActive) this.state.completionActive.close();
    var completion = this.state.completionActive = new Completion(this, options);
    if (!completion.options.hint) return;

    CodeMirror.signal(this, "startCompletion", this);
    completion.update(true);
  });

  CodeMirror.defineExtension("closeHint", function () {
    if (this.state.completionActive) this.state.completionActive.close()
  })

  function Completion(cm, options) {
    this.cm = cm;
    this.options = options;
    this.widget = null;
    this.debounce = 0;
    this.tick = 0;
    this.startPos = this.cm.getCursor("start");
    this.startLen = this.cm.getLine(this.startPos.line).length - this.cm.getSelection().length;
    if (this.options.updateOnCursorActivity) {
      var self = this;
      cm.on("cursorActivity", this.activityFunc = function () { self.cursorActivity(); });
    }
    CodeMirror.signal(this, "startCompletion", this);
  }

  var requestAnimationFrame = window.requestAnimationFrame || function (fn) {
    return setTimeout(fn, 1000 / 60);
  };
  var cancelAnimationFrame = window.cancelAnimationFrame || clearTimeout;

  Completion.prototype = {
    close: function () {
      if (!this.active()) return;
      this.cm.state.completionActive = null;
      this.tick = null;
      if (this.options.updateOnCursorActivity) {
        this.cm.off("cursorActivity", this.activityFunc);
      }

      if (this.widget && this.data) CodeMirror.signal(this.data, "close");
      if (this.widget) this.widget.close();
      CodeMirror.signal(this.cm, "endCompletion", this.cm);
    },

    active: function () {
      return this.cm.state.completionActive == this;
    },

    pick: function (data, i) {
      var completion = data.list[i], self = this;
      this.cm.operation(function () {
        if (completion.hint)
          completion.hint(self.cm, data, completion)
        else
          self.cm.replaceRange(getText(completion), completion.from || data.from,
            completion.to || data.to, "complete");
        CodeMirror.signal(data, "pick", completion);
        self.cm.scrollIntoView();
      });
      if (this.options.closeOnPick) {
        this.close();
      }
    },

    cursorActivity: function () {
      if (this.debounce) {
        cancelAnimationFrame(this.debounce);
        this.debounce = 0;
      }

      var identStart = this.startPos;
      if (this.data) {
        identStart = this.data.from;
      }

      var pos = this.cm.getCursor(), line = this.cm.getLine(pos.line);
      if (pos.line != this.startPos.line || line.length - pos.ch != this.startLen - this.startPos.ch ||
        pos.ch < identStart.ch || this.cm.somethingSelected() ||
        (!pos.ch || this.options.closeCharacters.test(line.charAt(pos.ch - 1)))) {
        this.close();
      } else {
        var self = this;
        this.debounce = requestAnimationFrame(function () { self.update(); });
        if (this.widget) this.widget.disable();
      }
    },

    update: function (first) {
      if (this.tick == null) return
      var self = this, myTick = ++this.tick
      fetchHints(this.options.hint, this.cm, this.options, function (data) {
        if (self.tick == myTick) self.finishUpdate(data, first)
      })
    },

    finishUpdate: function (data, first) {
      if (this.data) CodeMirror.signal(this.data, "update");
      var picked = (this.widget && this.widget.picked) || (first && this.options.completeSingle);
      if (this.widget) this.widget.close();
      this.data = data;

      if (data && data.list.length) {
        if (picked && data.list.length == 1) {
          this.pick(data, 0);
        } else {
          this.widget = new Widget(this, data);
          CodeMirror.signal(data, "shown");
        }
      }
    }
  };

  function parseOptions(cm, pos, options) {
    var editor = cm.options.hintOptions;
    var out = {};
    for (var prop in defaultOptions) out[prop] = defaultOptions[prop];
    if (editor) for (var prop in editor)
      if (editor[prop] !== undefined) out[prop] = editor[prop];
    if (options) for (var prop in options)
      if (options[prop] !== undefined) out[prop] = options[prop];
    if (out.hint.resolve) out.hint = out.hint.resolve(cm, pos)
    return out;
  }

  function getText(completion) {
    if (typeof completion == "string") return completion;
    else return completion.text;
  }

  function buildKeyMap(completion, handle) {
    var baseMap = {
      Up: function () { handle.moveFocus(-1); },
      Down: function () { handle.moveFocus(1); },
      PageUp: function () { handle.moveFocus(-handle.menuSize() + 1, true); },
      PageDown: function () { handle.moveFocus(handle.menuSize() - 1, true); },
      Home: function () { handle.setFocus(0); },
      End: function () { handle.setFocus(handle.length - 1); },
      Enter: handle.pick,
      Tab: handle.pick,
      Esc: handle.close
    };
    var mac = /Mac/.test(navigator.platform);
    //autocorrect with space key and add a whitespace after the word CXD
    if (isSame) {
      baseMap["Space"] = handle.pick;
      baseMap[","] = handle.pick;
    }
    if (mac) {
      baseMap["Ctrl-P"] = function () { handle.moveFocus(-1); };
      baseMap["Ctrl-N"] = function () { handle.moveFocus(1); };
    }

    var custom = completion.options.customKeys;
    var ourMap = custom ? {} : baseMap;
    function addBinding(key, val) {
      var bound;
      if (typeof val != "string")
        bound = function (cm) { return val(cm, handle); };
      // This mechanism is deprecated
      else if (baseMap.hasOwnProperty(val))
        bound = baseMap[val];
      else
        bound = val;
      ourMap[key] = bound;
    }
    if (custom)
      for (var key in custom) if (custom.hasOwnProperty(key))
        addBinding(key, custom[key]);
    var extra = completion.options.extraKeys;
    if (extra)
      for (var key in extra) if (extra.hasOwnProperty(key))
        addBinding(key, extra[key]);
    return ourMap;
  }

  function getHintElement(hintsElement, el) {
    while (el && el != hintsElement) {
      if (el.nodeName.toUpperCase() === "LI" && el.parentNode == hintsElement) return el;
      el = el.parentNode;
    }
  }

  function Widget(completion, data) {
    this.id = "cm-complete-" + Math.floor(Math.random(1e6))
    this.completion = completion;
    this.data = data;
    this.picked = false;
    var widget = this, cm = completion.cm;
    var ownerDocument = cm.getInputField().ownerDocument;
    var parentWindow = ownerDocument.defaultView || ownerDocument.parentWindow;

    var hints = this.hints = ownerDocument.createElement("ul");
    hints.setAttribute("role", "listbox")
    hints.setAttribute("aria-expanded", "true")
    hints.id = this.id
    var theme = completion.cm.options.theme;
    hints.className = "CodeMirror-hints " + theme;
    this.selectedHint = data.selectedHint || 0;

    var completions = data.list;
    for (var i = 0; i < completions.length; ++i) {
      var elt = hints.appendChild(ownerDocument.createElement("li")), cur = completions[i];
      var className = HINT_ELEMENT_CLASS + (i != this.selectedHint ? "" : " " + ACTIVE_HINT_ELEMENT_CLASS);
      if (cur.className != null) className = cur.className + " " + className;
      elt.className = className;
      if (i == this.selectedHint) elt.setAttribute("aria-selected", "true")
      elt.id = this.id + "-" + i
      elt.setAttribute("role", "option")
      if (cur.render) cur.render(elt, data, cur);
      else elt.appendChild(ownerDocument.createTextNode(cur.displayText || getText(cur)));
      elt.hintId = i;
    }

    var container = completion.options.container || ownerDocument.body;
    var pos = cm.cursorCoords(completion.options.alignWithWord ? data.from : null);
    var left = pos.left, top = pos.bottom, below = true;
    var offsetLeft = 0, offsetTop = 0;
    if (container !== ownerDocument.body) {
      // We offset the cursor position because left and top are relative to the offsetParent's top left corner.
      var isContainerPositioned = ['absolute', 'relative', 'fixed'].indexOf(parentWindow.getComputedStyle(container).position) !== -1;
      var offsetParent = isContainerPositioned ? container : container.offsetParent;
      var offsetParentPosition = offsetParent.getBoundingClientRect();
      var bodyPosition = ownerDocument.body.getBoundingClientRect();
      offsetLeft = (offsetParentPosition.left - bodyPosition.left - offsetParent.scrollLeft);
      offsetTop = (offsetParentPosition.top - bodyPosition.top - offsetParent.scrollTop);
    }
    hints.style.left = (left - offsetLeft) + "px";
    hints.style.top = (top - offsetTop) + "px";

    // If we're at the edge of the screen, then we want the menu to appear on the left of the cursor.
    var winW = parentWindow.innerWidth || Math.max(ownerDocument.body.offsetWidth, ownerDocument.documentElement.offsetWidth);
    var winH = parentWindow.innerHeight || Math.max(ownerDocument.body.offsetHeight, ownerDocument.documentElement.offsetHeight);
    container.appendChild(hints);
    cm.getInputField().setAttribute("aria-autocomplete", "list")
    cm.getInputField().setAttribute("aria-owns", this.id)
    cm.getInputField().setAttribute("aria-activedescendant", this.id + "-" + this.selectedHint)

    var box = completion.options.moveOnOverlap ? hints.getBoundingClientRect() : new DOMRect();
    var scrolls = completion.options.paddingForScrollbar ? hints.scrollHeight > hints.clientHeight + 1 : false;

    // Compute in the timeout to avoid reflow on init
    var startScroll;
    setTimeout(function () { startScroll = cm.getScrollInfo(); });

    var overlapY = box.bottom - winH;
    if (overlapY > 0) {
      var height = box.bottom - box.top, curTop = pos.top - (pos.bottom - box.top);
      if (curTop - height > 0) { // Fits above cursor
        hints.style.top = (top = pos.top - height - offsetTop) + "px";
        below = false;
      } else if (height > winH) {
        hints.style.height = (winH - 5) + "px";
        hints.style.top = (top = pos.bottom - box.top - offsetTop) + "px";
        var cursor = cm.getCursor();
        if (data.from.ch != cursor.ch) {
          pos = cm.cursorCoords(cursor);
          hints.style.left = (left = pos.left - offsetLeft) + "px";
          box = hints.getBoundingClientRect();
        }
      }
    }
    var overlapX = box.right - winW;
    if (scrolls) overlapX += cm.display.nativeBarWidth;
    if (overlapX > 0) {
      if (box.right - box.left > winW) {
        hints.style.width = (winW - 5) + "px";
        overlapX -= (box.right - box.left) - winW;
      }
      hints.style.left = (left = pos.left - overlapX - offsetLeft) + "px";
    }
    if (scrolls) for (var node = hints.firstChild; node; node = node.nextSibling)
      node.style.paddingRight = cm.display.nativeBarWidth + "px"

    cm.addKeyMap(this.keyMap = buildKeyMap(completion, {
      moveFocus: function (n, avoidWrap) { widget.changeActive(widget.selectedHint + n, avoidWrap); },
      setFocus: function (n) { widget.changeActive(n); },
      menuSize: function () { return widget.screenAmount(); },
      length: completions.length,
      close: function () { completion.close(); },
      pick: function () { widget.pick(); },
      data: data
    }));

    if (completion.options.closeOnUnfocus) {
      var closingOnBlur;
      cm.on("blur", this.onBlur = function () { closingOnBlur = setTimeout(function () { completion.close(); }, 100); });
      cm.on("focus", this.onFocus = function () { clearTimeout(closingOnBlur); });
    }

    cm.on("scroll", this.onScroll = function () {
      var curScroll = cm.getScrollInfo(), editor = cm.getWrapperElement().getBoundingClientRect();
      if (!startScroll) startScroll = cm.getScrollInfo();
      var newTop = top + startScroll.top - curScroll.top;
      var point = newTop - (parentWindow.pageYOffset || (ownerDocument.documentElement || ownerDocument.body).scrollTop);
      if (!below) point += hints.offsetHeight;
      if (point <= editor.top || point >= editor.bottom) return completion.close();
      hints.style.top = newTop + "px";
      hints.style.left = (left + startScroll.left - curScroll.left) + "px";
    });

    CodeMirror.on(hints, "dblclick", function (e) {
      var t = getHintElement(hints, e.target || e.srcElement);
      if (t && t.hintId != null) { widget.changeActive(t.hintId); widget.pick(); }
    });

    CodeMirror.on(hints, "click", function (e) {
      var t = getHintElement(hints, e.target || e.srcElement);
      if (t && t.hintId != null) {
        widget.changeActive(t.hintId);
        if (completion.options.completeOnSingleClick) widget.pick();
      }
    });

    CodeMirror.on(hints, "mousedown", function () {
      setTimeout(function () { cm.focus(); }, 20);
    });

    // The first hint doesn't need to be scrolled to on init
    var selectedHintRange = this.getSelectedHintRange();
    if (selectedHintRange.from !== 0 || selectedHintRange.to !== 0) {
      this.scrollToActive();
    }

    CodeMirror.signal(data, "select", completions[this.selectedHint], hints.childNodes[this.selectedHint]);
    return true;
  }

  Widget.prototype = {
    close: function () {
      if (this.completion.widget != this) return;
      this.completion.widget = null;
      if (this.hints.parentNode) this.hints.parentNode.removeChild(this.hints);
      this.completion.cm.removeKeyMap(this.keyMap);
      var input = this.completion.cm.getInputField()
      input.removeAttribute("aria-activedescendant")
      input.removeAttribute("aria-owns")

      var cm = this.completion.cm;
      if (this.completion.options.closeOnUnfocus) {
        cm.off("blur", this.onBlur);
        cm.off("focus", this.onFocus);
      }
      cm.off("scroll", this.onScroll);
    },

    disable: function () {
      this.completion.cm.removeKeyMap(this.keyMap);
      var widget = this;
      this.keyMap = { Enter: function () { widget.picked = true; } };
      this.completion.cm.addKeyMap(this.keyMap);
    },

    pick: function () {
      //autocorrect with space key and add a whitespace after the word + autocompletion  CXD
      var whatisIt;
      var Xspace, Xspace2;
      var numCharA, numCharB;
      numCharA = rEdit.getCursor().ch;
      numCharB = rEdit.getCursor().ch - this.data.list[0].length;
      if (numCharA === 1 || numCharA === 0) { Xspace = ''; }
      else { Xspace = ' '.repeat(numCharA - 2); if (numCharB > 0) Xspace2 = ' '.repeat(numCharB); else Xspace2 = ''; }
      if (isSame && nameKey === 'Space') {
        if (this.data.list[0] === 'If') { this.data.list[0] = this.data.list[0] + ' '; }
        else if (this.data.list[0] != 'Do') { this.data.list[0] = this.data.list[0] + ' '; isSame = false; whatisIt = 0; }
      }
      else if (isSame && nameKey === 'Enter') {
        if (this.data.list[0] === 'If') { this.data.list[0] = this.data.list[0] + ' ' + '\n' + Xspace + 'Endif'; whatisIt = 1; }
        //else if (this.data.list[0] === 'On') { this.data.list[0] = this.data.list[0] + '  Do' + '\n\n' + Xspace + 'Endon'; whatisIt = 2; }
        else if (this.data.list[0] === 'On') { this.data.list[0] = this.data.list[0] + '  Do' + '\n\n' + 'Endon'; whatisIt = 2; }
        else if (this.data.list[0] === 'Do') { this.data.list[0] = this.data.list[0] + '\n\n' + 'Endon'; whatisIt = 2.2; }
        else { this.data.list[0] = this.data.list[0] + '\n' + Xspace2; whatisIt = 0; }
        isSame = false;
      }
      else if (isSame && nameKey === ',') {
        this.data.list[0] = this.data.list[0] + ','; whatisIt = 0;
        isSame = false;
      }
      if (this.data.list[this.selectedHint] === 'LogEntry') { this.data.list[this.selectedHint] = this.data.list[this.selectedHint] + ',\'\''; whatisIt = 3; }
      //else if (this.data.list[this.selectedHint] === 'If') { if (numCharA > 1) { Xspace = Xspace + ' '; } this.data.list[this.selectedHint] = this.data.list[this.selectedHint] + ' ' + '\n' + Xspace+ '\n' + Xspace + 'Endif'; whatisIt = 1; }

      this.completion.pick(this.data, this.selectedHint);
      //autocompletion addition CXD
      var numLine = rEdit.getCursor().line
      var numChar = rEdit.getCursor().ch
      if (whatisIt === 1) { rEdit.setCursor({ line: numLine - 1 }) }
      //else if (whatisIt === 2) { rEdit.setCursor({ line: numLine - 1 }); rEdit.execCommand('insertSoftTab'); rEdit.setCursor({ line: numLine - 2, ch: numChar-2}); }
      else if (whatisIt === 2) {
        rEdit.setCursor({ line: numLine - 1 }); rEdit.execCommand('insertSoftTab'); rEdit.setCursor({ line: numLine - 2, ch: numCharA + 1 });
        while (rEdit.getCursor().ch > 3) {
          rEdit.execCommand('indentLess');
        }
      }
      else if (whatisIt === 2.2) { rEdit.setCursor({ line: numLine - 1 }); rEdit.execCommand('insertSoftTab'); }
      else if (whatisIt === 3) { rEdit.setCursor({ line: numLine, ch: numChar - 1 }); };
    },

    changeActive: function (i, avoidWrap) {
      if (i >= this.data.list.length)
        i = avoidWrap ? this.data.list.length - 1 : 0;
      else if (i < 0)
        i = avoidWrap ? 0 : this.data.list.length - 1;
      if (this.selectedHint == i) return;
      var node = this.hints.childNodes[this.selectedHint];
      if (node) {
        node.className = node.className.replace(" " + ACTIVE_HINT_ELEMENT_CLASS, "");
        node.removeAttribute("aria-selected")
      }
      node = this.hints.childNodes[this.selectedHint = i];
      node.className += " " + ACTIVE_HINT_ELEMENT_CLASS;
      node.setAttribute("aria-selected", "true")
      this.completion.cm.getInputField().setAttribute("aria-activedescendant", node.id)
      this.scrollToActive()
      CodeMirror.signal(this.data, "select", this.data.list[this.selectedHint], node);
    },

    scrollToActive: function () {
      var selectedHintRange = this.getSelectedHintRange();
      var node1 = this.hints.childNodes[selectedHintRange.from];
      var node2 = this.hints.childNodes[selectedHintRange.to];
      var firstNode = this.hints.firstChild;
      if (node1.offsetTop < this.hints.scrollTop)
        this.hints.scrollTop = node1.offsetTop - firstNode.offsetTop;
      else if (node2.offsetTop + node2.offsetHeight > this.hints.scrollTop + this.hints.clientHeight)
        this.hints.scrollTop = node2.offsetTop + node2.offsetHeight - this.hints.clientHeight + firstNode.offsetTop;
    },

    screenAmount: function () {
      return Math.floor(this.hints.clientHeight / this.hints.firstChild.offsetHeight) || 1;
    },

    getSelectedHintRange: function () {
      var margin = this.completion.options.scrollMargin || 0;
      return {
        from: Math.max(0, this.selectedHint - margin),
        to: Math.min(this.data.list.length - 1, this.selectedHint + margin),
      };
    }
  };

  function applicableHelpers(cm, helpers) {
    if (!cm.somethingSelected()) return helpers
    var result = []
    for (var i = 0; i < helpers.length; i++)
      if (helpers[i].supportsSelection) result.push(helpers[i])
    return result
  }

  function fetchHints(hint, cm, options, callback) {
    if (hint.async) {
      hint(cm, callback, options)
    } else {
      var result = hint(cm, options)
      if (result && result.then) result.then(callback)
      else callback(result)
    }
  }

  function resolveAutoHints(cm, pos) {
    var helpers = cm.getHelpers(pos, "hint"), words
    if (helpers.length) {
      var resolved = function (cm, callback, options) {
        var app = applicableHelpers(cm, helpers);
        function run(i) {
          if (i == app.length) return callback(null)
          fetchHints(app[i], cm, options, function (result) {
            if (result && result.list.length > 0) callback(result)
            else run(i + 1)
          })
        }
        run(0)
      }
      resolved.async = true
      resolved.supportsSelection = true
      return resolved
    } else if (words = cm.getHelper(cm.getCursor(), "hintWords")) {
      return function (cm) { return CodeMirror.hint.fromList(cm, { words: words }) }
    } else if (CodeMirror.hint.anyword) {
      return function (cm, options) { return CodeMirror.hint.anyword(cm, options) }
    } else {
      return function () { }
    }
  }

  CodeMirror.registerHelper("hint", "auto", {
    resolve: resolveAutoHints
  });

  CodeMirror.registerHelper("hint", "fromList", function (cm, options) {
    var cur = cm.getCursor(), token = cm.getTokenAt(cur)
    var term, from = CodeMirror.Pos(cur.line, token.start), to = cur
    if (token.start < cur.ch && /\w/.test(token.string.charAt(cur.ch - token.start - 1))) {
      term = token.string.substr(0, cur.ch - token.start)
    } else {
      term = ""
      from = cur
    }
    var found = [];
    for (var i = 0; i < options.words.length; i++) {
      var word = options.words[i];
      if (word.slice(0, term.length) == term)
        found.push(word);
    }
    if (found.length) return { list: found, from: from, to: to };
  });

  CodeMirror.commands.autocomplete = CodeMirror.showHint;

  var defaultOptions = {
    hint: CodeMirror.hint.auto,
    completeSingle: true,
    alignWithWord: true,
    closeCharacters: /[[\s=+*\-()\[\]{};:>]/,
    closeOnPick: true,
    closeOnUnfocus: true,
    updateOnCursorActivity: true,
    completeOnSingleClick: true,
    container: null,
    customKeys: null,
    extraKeys: null,
    paddingForScrollbar: true,
    moveOnOverlap: true,
  };

  CodeMirror.defineOption("hintOptions", null);
});



// This is from the file search.js-------------------------------------------------------------------------
(function (search) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    search(require("../../lib/codemirror"), require("./searchcursor"), require("../dialog/dialog"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror", "./searchcursor", "../dialog/dialog"], mod);
  else // Plain browser env
    search(CodeMirror);
})(function (CodeMirror) {
  "use strict";

  // default search panel location
  CodeMirror.defineOption("search", { bottom: false });

  function searchOverlay(query, caseInsensitive) {
    if (typeof query == "string")
      query = new RegExp(query.replace(/[\-\[\]\/\{\}\(\)\*\+\?\.\\\^\$\|]/g, "\\$&"), caseInsensitive ? "gi" : "g");
    else if (!query.global)
      query = new RegExp(query.source, query.ignoreCase ? "gi" : "g");

    return {
      token: function (stream) {
        query.lastIndex = stream.pos;
        var match = query.exec(stream.string);
        if (match && match.index == stream.pos) {
          stream.pos += match[0].length || 1;
          return "searching";
        } else if (match) {
          stream.pos = match.index;
        } else {
          stream.skipToEnd();
        }
      }
    };
  }

  function SearchState() {
    this.posFrom = this.posTo = this.lastQuery = this.query = null;
    this.overlay = null;
  }

  function getSearchState(cm) {
    return cm.state.search || (cm.state.search = new SearchState());
  }

  function queryCaseInsensitive(query) {
    return typeof query == "string" && query == query.toLowerCase();
  }

  function getSearchCursor(cm, query, pos) {
    // Heuristic: if the query string is all lowercase, do a case insensitive search.
    return cm.getSearchCursor(query, pos, { caseFold: queryCaseInsensitive(query), multiline: true });
  }

  function persistentDialog(cm, text, deflt, onEnter, onKeyDown) {
    cm.openDialog(text, onEnter, {
      value: deflt,
      selectValueOnOpen: true,
      closeOnEnter: false,
      onClose: function () { clearSearch(cm); },
      onKeyDown: onKeyDown,
      bottom: cm.options.search.bottom
    });
  }

  function dialog(cm, text, shortText, deflt, f) {
    if (cm.openDialog) cm.openDialog(text, f, { value: deflt, selectValueOnOpen: true, bottom: cm.options.search.bottom });
    else f(prompt(shortText, deflt));
  }

  function confirmDialog(cm, text, shortText, fs) {
    if (cm.openConfirm) cm.openConfirm(text, fs);
    else if (confirm(shortText)) fs[0]();
  }

  function parseString(string) {
    return string.replace(/\\([nrt\\])/g, function (match, ch) {
      if (ch == "n") return "\n"
      if (ch == "r") return "\r"
      if (ch == "t") return "\t"
      if (ch == "\\") return "\\"
      return match
    })
  }

  function parseQuery(query) {
    var isRE = query.match(/^\/(.*)\/([a-z]*)$/);
    if (isRE) {
      try { query = new RegExp(isRE[1], isRE[2].indexOf("i") == -1 ? "" : "i"); }
      catch (e) { } // Not a regular expression after all, do a string search
    } else {
      query = parseString(query)
    }
    if (typeof query == "string" ? query == "" : query.test(""))
      query = /x^/;
    return query;
  }

  function startSearch(cm, state, query) {
    state.queryText = query;
    state.query = parseQuery(query);
    cm.removeOverlay(state.overlay, queryCaseInsensitive(state.query));
    state.overlay = searchOverlay(state.query, queryCaseInsensitive(state.query));
    cm.addOverlay(state.overlay);
    if (cm.showMatchesOnScrollbar) {
      if (state.annotate) { state.annotate.clear(); state.annotate = null; }
      state.annotate = cm.showMatchesOnScrollbar(state.query, queryCaseInsensitive(state.query));
    }
  }

  function doSearch(cm, rev, persistent, immediate) {
    var state = getSearchState(cm);
    if (state.query) return findNext(cm, rev);
    var q = cm.getSelection() || state.lastQuery;
    if (q instanceof RegExp && q.source == "x^") q = null
    if (persistent && cm.openDialog) {
      var hiding = null
      var searchNext = function (query, event) {
        CodeMirror.e_stop(event);
        if (!query) return;
        if (query != state.queryText) {
          startSearch(cm, state, query);
          state.posFrom = state.posTo = cm.getCursor();
        }
        if (hiding) hiding.style.opacity = 1
        findNext(cm, event.shiftKey, function (_, to) {
          var dialog
          if (to.line < 3 && document.querySelector &&
            (dialog = cm.display.wrapper.querySelector(".CodeMirror-dialog")) &&
            dialog.getBoundingClientRect().bottom - 4 > cm.cursorCoords(to, "window").top)
            (hiding = dialog).style.opacity = .4
        })
      };
      persistentDialog(cm, getQueryDialog(cm), q, searchNext, function (event, query) {
        var keyName = CodeMirror.keyName(event)
        var extra = cm.getOption('extraKeys'), cmd = (extra && extra[keyName]) || CodeMirror.keyMap[cm.getOption("keyMap")][keyName]
        if (cmd == "findNext" || cmd == "findPrev" ||
          cmd == "findPersistentNext" || cmd == "findPersistentPrev") {
          CodeMirror.e_stop(event);
          startSearch(cm, getSearchState(cm), query);
          cm.execCommand(cmd);
        } else if (cmd == "find" || cmd == "findPersistent") {
          CodeMirror.e_stop(event);
          searchNext(query, event);
        }
      });
      if (immediate && q) {
        startSearch(cm, state, q);
        findNext(cm, rev);
      }
    } else {
      dialog(cm, getQueryDialog(cm), "Search for:", q, function (query) {
        if (query && !state.query) cm.operation(function () {
          startSearch(cm, state, query);
          state.posFrom = state.posTo = cm.getCursor();
          findNext(cm, rev);
        });
      });
    }
  }

  function findNext(cm, rev, callback) {
    cm.operation(function () {
      var state = getSearchState(cm);
      var cursor = getSearchCursor(cm, state.query, rev ? state.posFrom : state.posTo);
      if (!cursor.find(rev)) {
        cursor = getSearchCursor(cm, state.query, rev ? CodeMirror.Pos(cm.lastLine()) : CodeMirror.Pos(cm.firstLine(), 0));
        if (!cursor.find(rev)) return;
      }
      cm.setSelection(cursor.from(), cursor.to());
      cm.scrollIntoView({ from: cursor.from(), to: cursor.to() }, 20);
      state.posFrom = cursor.from(); state.posTo = cursor.to();
      if (callback) callback(cursor.from(), cursor.to())
    });
  }

  function clearSearch(cm) {
    cm.operation(function () {
      var state = getSearchState(cm);
      state.lastQuery = state.query;
      if (!state.query) return;
      state.query = state.queryText = null;
      cm.removeOverlay(state.overlay);
      if (state.annotate) { state.annotate.clear(); state.annotate = null; }
    });
  }

  function el(tag, attrs) {
    var element = tag ? document.createElement(tag) : document.createDocumentFragment();
    for (var key in attrs) {
      element[key] = attrs[key];
    }
    for (var i = 2; i < arguments.length; i++) {
      var child = arguments[i]
      element.appendChild(typeof child == "string" ? document.createTextNode(child) : child);
    }
    return element;
  }

  function getQueryDialog(cm) {
    var label = el("label", { className: "CodeMirror-search-label" },
      cm.phrase("Search:"),
      el("input", {
        type: "text", "style": "width: 10em", className: "CodeMirror-search-field",
        id: "CodeMirror-search-field"
      }));
    label.setAttribute("for", "CodeMirror-search-field");
    return el("", null, label, " ",
      el("span", { style: "color: #666", className: "CodeMirror-search-hint" },
        cm.phrase("(Use /re/ syntax for regexp search)")));
  }
  function getReplaceQueryDialog(cm) {
    return el("", null, " ",
      el("input", { type: "text", "style": "width: 10em", className: "CodeMirror-search-field" }), " ",
      el("span", { style: "color: #666", className: "CodeMirror-search-hint" },
        cm.phrase("(Use /re/ syntax for regexp search)")));
  }
  function getReplacementQueryDialog(cm) {
    return el("", null,
      el("span", { className: "CodeMirror-search-label" }, cm.phrase("With:")), " ",
      el("input", { type: "text", "style": "width: 10em", className: "CodeMirror-search-field" }));
  }
  function getDoReplaceConfirm(cm) {
    return el("", null,
      el("span", { className: "CodeMirror-search-label" }, cm.phrase("Replace?")), " ",
      el("button", {}, cm.phrase("Yes")), " ",
      el("button", {}, cm.phrase("No")), " ",
      el("button", {}, cm.phrase("All")), " ",
      el("button", {}, cm.phrase("Stop")));
  }

  function replaceAll(cm, query, text) {
    cm.operation(function () {
      for (var cursor = getSearchCursor(cm, query); cursor.findNext();) {
        if (typeof query != "string") {
          var match = cm.getRange(cursor.from(), cursor.to()).match(query);
          cursor.replace(text.replace(/\$(\d)/g, function (_, i) { return match[i]; }));
        } else cursor.replace(text);
      }
    });
  }

  function replace(cm, all) {
    if (cm.getOption("readOnly")) return;
    var query = cm.getSelection() || getSearchState(cm).lastQuery;
    var dialogText = all ? cm.phrase("Replace all:") : cm.phrase("Replace:")
    var fragment = el("", null,
      el("span", { className: "CodeMirror-search-label" }, dialogText),
      getReplaceQueryDialog(cm))
    dialog(cm, fragment, dialogText, query, function (query) {
      if (!query) return;
      query = parseQuery(query);
      dialog(cm, getReplacementQueryDialog(cm), cm.phrase("Replace with:"), "", function (text) {
        text = parseString(text)
        if (all) {
          replaceAll(cm, query, text)
        } else {
          clearSearch(cm);
          var cursor = getSearchCursor(cm, query, cm.getCursor("from"));
          var advance = function () {
            var start = cursor.from(), match;
            if (!(match = cursor.findNext())) {
              cursor = getSearchCursor(cm, query);
              if (!(match = cursor.findNext()) ||
                (start && cursor.from().line == start.line && cursor.from().ch == start.ch)) return;
            }
            cm.setSelection(cursor.from(), cursor.to());
            cm.scrollIntoView({ from: cursor.from(), to: cursor.to() });
            confirmDialog(cm, getDoReplaceConfirm(cm), cm.phrase("Replace?"),
              [function () { doReplace(match); }, advance,
              function () { replaceAll(cm, query, text) }]);
          };
          var doReplace = function (match) {
            cursor.replace(typeof query == "string" ? text :
              text.replace(/\$(\d)/g, function (_, i) { return match[i]; }));
            advance();
          };
          advance();
        }
      });
    });
  }

  CodeMirror.commands.find = function (cm) { clearSearch(cm); doSearch(cm); };
  CodeMirror.commands.findPersistent = function (cm) { clearSearch(cm); doSearch(cm, false, true); };
  CodeMirror.commands.findPersistentNext = function (cm) { doSearch(cm, false, true, true); };
  CodeMirror.commands.findPersistentPrev = function (cm) { doSearch(cm, true, true, true); };
  CodeMirror.commands.findNext = doSearch;
  CodeMirror.commands.findPrev = function (cm) { doSearch(cm, true); };
  CodeMirror.commands.clearSearch = clearSearch;
  CodeMirror.commands.replace = replace;
  CodeMirror.commands.replaceAll = function (cm) { replace(cm, true); };
});



// This is from the file searchcursor.js-------------------------------------------------------------------------

(function (searchcursor) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    searchcursor(require("../../lib/codemirror"))
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod)
  else // Plain browser env
    searchcursor(CodeMirror)
})(function (CodeMirror) {
  "use strict"
  var Pos = CodeMirror.Pos

  function regexpFlags(regexp) {
    var flags = regexp.flags
    return flags != null ? flags : (regexp.ignoreCase ? "i" : "")
      + (regexp.global ? "g" : "")
      + (regexp.multiline ? "m" : "")
  }

  function ensureFlags(regexp, flags) {
    var current = regexpFlags(regexp), target = current
    for (var i = 0; i < flags.length; i++) if (target.indexOf(flags.charAt(i)) == -1)
      target += flags.charAt(i)
    return current == target ? regexp : new RegExp(regexp.source, target)
  }

  function maybeMultiline(regexp) {
    return /\\s|\\n|\n|\\W|\\D|\[\^/.test(regexp.source)
  }

  function searchRegexpForward(doc, regexp, start) {
    regexp = ensureFlags(regexp, "g")
    for (var line = start.line, ch = start.ch, last = doc.lastLine(); line <= last; line++, ch = 0) {
      regexp.lastIndex = ch
      var string = doc.getLine(line), match = regexp.exec(string)
      if (match)
        return {
          from: Pos(line, match.index),
          to: Pos(line, match.index + match[0].length),
          match: match
        }
    }
  }

  function searchRegexpForwardMultiline(doc, regexp, start) {
    if (!maybeMultiline(regexp)) return searchRegexpForward(doc, regexp, start)

    regexp = ensureFlags(regexp, "gm")
    var string, chunk = 1
    for (var line = start.line, last = doc.lastLine(); line <= last;) {
      // This grows the search buffer in exponentially-sized chunks
      // between matches, so that nearby matches are fast and don't
      // require concatenating the whole document (in case we're
      // searching for something that has tons of matches), but at the
      // same time, the amount of retries is limited.
      for (var i = 0; i < chunk; i++) {
        if (line > last) break
        var curLine = doc.getLine(line++)
        string = string == null ? curLine : string + "\n" + curLine
      }
      chunk = chunk * 2
      regexp.lastIndex = start.ch
      var match = regexp.exec(string)
      if (match) {
        var before = string.slice(0, match.index).split("\n"), inside = match[0].split("\n")
        var startLine = start.line + before.length - 1, startCh = before[before.length - 1].length
        return {
          from: Pos(startLine, startCh),
          to: Pos(startLine + inside.length - 1,
            inside.length == 1 ? startCh + inside[0].length : inside[inside.length - 1].length),
          match: match
        }
      }
    }
  }

  function lastMatchIn(string, regexp, endMargin) {
    var match, from = 0
    while (from <= string.length) {
      regexp.lastIndex = from
      var newMatch = regexp.exec(string)
      if (!newMatch) break
      var end = newMatch.index + newMatch[0].length
      if (end > string.length - endMargin) break
      if (!match || end > match.index + match[0].length)
        match = newMatch
      from = newMatch.index + 1
    }
    return match
  }

  function searchRegexpBackward(doc, regexp, start) {
    regexp = ensureFlags(regexp, "g")
    for (var line = start.line, ch = start.ch, first = doc.firstLine(); line >= first; line--, ch = -1) {
      var string = doc.getLine(line)
      var match = lastMatchIn(string, regexp, ch < 0 ? 0 : string.length - ch)
      if (match)
        return {
          from: Pos(line, match.index),
          to: Pos(line, match.index + match[0].length),
          match: match
        }
    }
  }

  function searchRegexpBackwardMultiline(doc, regexp, start) {
    if (!maybeMultiline(regexp)) return searchRegexpBackward(doc, regexp, start)
    regexp = ensureFlags(regexp, "gm")
    var string, chunkSize = 1, endMargin = doc.getLine(start.line).length - start.ch
    for (var line = start.line, first = doc.firstLine(); line >= first;) {
      for (var i = 0; i < chunkSize && line >= first; i++) {
        var curLine = doc.getLine(line--)
        string = string == null ? curLine : curLine + "\n" + string
      }
      chunkSize *= 2

      var match = lastMatchIn(string, regexp, endMargin)
      if (match) {
        var before = string.slice(0, match.index).split("\n"), inside = match[0].split("\n")
        var startLine = line + before.length, startCh = before[before.length - 1].length
        return {
          from: Pos(startLine, startCh),
          to: Pos(startLine + inside.length - 1,
            inside.length == 1 ? startCh + inside[0].length : inside[inside.length - 1].length),
          match: match
        }
      }
    }
  }

  var doFold, noFold
  if (String.prototype.normalize) {
    doFold = function (str) { return str.normalize("NFD").toLowerCase() }
    noFold = function (str) { return str.normalize("NFD") }
  } else {
    doFold = function (str) { return str.toLowerCase() }
    noFold = function (str) { return str }
  }

  // Maps a position in a case-folded line back to a position in the original line
  // (compensating for codepoints increasing in number during folding)
  function adjustPos(orig, folded, pos, foldFunc) {
    if (orig.length == folded.length) return pos
    for (var min = 0, max = pos + Math.max(0, orig.length - folded.length); ;) {
      if (min == max) return min
      var mid = (min + max) >> 1
      var len = foldFunc(orig.slice(0, mid)).length
      if (len == pos) return mid
      else if (len > pos) max = mid
      else min = mid + 1
    }
  }

  function searchStringForward(doc, query, start, caseFold) {
    // Empty string would match anything and never progress, so we
    // define it to match nothing instead.
    if (!query.length) return null
    var fold = caseFold ? doFold : noFold
    var lines = fold(query).split(/\r|\n\r?/)

    search: for (var line = start.line, ch = start.ch, last = doc.lastLine() + 1 - lines.length; line <= last; line++, ch = 0) {
      var orig = doc.getLine(line).slice(ch), string = fold(orig)
      if (lines.length == 1) {
        var found = string.indexOf(lines[0])
        if (found == -1) continue search
        var start = adjustPos(orig, string, found, fold) + ch
        return {
          from: Pos(line, adjustPos(orig, string, found, fold) + ch),
          to: Pos(line, adjustPos(orig, string, found + lines[0].length, fold) + ch)
        }
      } else {
        var cutFrom = string.length - lines[0].length
        if (string.slice(cutFrom) != lines[0]) continue search
        for (var i = 1; i < lines.length - 1; i++)
          if (fold(doc.getLine(line + i)) != lines[i]) continue search
        var end = doc.getLine(line + lines.length - 1), endString = fold(end), lastLine = lines[lines.length - 1]
        if (endString.slice(0, lastLine.length) != lastLine) continue search
        return {
          from: Pos(line, adjustPos(orig, string, cutFrom, fold) + ch),
          to: Pos(line + lines.length - 1, adjustPos(end, endString, lastLine.length, fold))
        }
      }
    }
  }

  function searchStringBackward(doc, query, start, caseFold) {
    if (!query.length) return null
    var fold = caseFold ? doFold : noFold
    var lines = fold(query).split(/\r|\n\r?/)

    search: for (var line = start.line, ch = start.ch, first = doc.firstLine() - 1 + lines.length; line >= first; line--, ch = -1) {
      var orig = doc.getLine(line)
      if (ch > -1) orig = orig.slice(0, ch)
      var string = fold(orig)
      if (lines.length == 1) {
        var found = string.lastIndexOf(lines[0])
        if (found == -1) continue search
        return {
          from: Pos(line, adjustPos(orig, string, found, fold)),
          to: Pos(line, adjustPos(orig, string, found + lines[0].length, fold))
        }
      } else {
        var lastLine = lines[lines.length - 1]
        if (string.slice(0, lastLine.length) != lastLine) continue search
        for (var i = 1, start = line - lines.length + 1; i < lines.length - 1; i++)
          if (fold(doc.getLine(start + i)) != lines[i]) continue search
        var top = doc.getLine(line + 1 - lines.length), topString = fold(top)
        if (topString.slice(topString.length - lines[0].length) != lines[0]) continue search
        return {
          from: Pos(line + 1 - lines.length, adjustPos(top, topString, top.length - lines[0].length, fold)),
          to: Pos(line, adjustPos(orig, string, lastLine.length, fold))
        }
      }
    }
  }

  function SearchCursor(doc, query, pos, options) {
    this.atOccurrence = false
    this.afterEmptyMatch = false
    this.doc = doc
    pos = pos ? doc.clipPos(pos) : Pos(0, 0)
    this.pos = { from: pos, to: pos }

    var caseFold
    if (typeof options == "object") {
      caseFold = options.caseFold
    } else { // Backwards compat for when caseFold was the 4th argument
      caseFold = options
      options = null
    }

    if (typeof query == "string") {
      if (caseFold == null) caseFold = false
      this.matches = function (reverse, pos) {
        return (reverse ? searchStringBackward : searchStringForward)(doc, query, pos, caseFold)
      }
    } else {
      query = ensureFlags(query, "gm")
      if (!options || options.multiline !== false)
        this.matches = function (reverse, pos) {
          return (reverse ? searchRegexpBackwardMultiline : searchRegexpForwardMultiline)(doc, query, pos)
        }
      else
        this.matches = function (reverse, pos) {
          return (reverse ? searchRegexpBackward : searchRegexpForward)(doc, query, pos)
        }
    }
  }

  SearchCursor.prototype = {
    findNext: function () { return this.find(false) },
    findPrevious: function () { return this.find(true) },

    find: function (reverse) {
      var head = this.doc.clipPos(reverse ? this.pos.from : this.pos.to);
      if (this.afterEmptyMatch && this.atOccurrence) {
        // do not return the same 0 width match twice
        head = Pos(head.line, head.ch)
        if (reverse) {
          head.ch--;
          if (head.ch < 0) {
            head.line--;
            head.ch = (this.doc.getLine(head.line) || "").length;
          }
        } else {
          head.ch++;
          if (head.ch > (this.doc.getLine(head.line) || "").length) {
            head.ch = 0;
            head.line++;
          }
        }
        if (CodeMirror.cmpPos(head, this.doc.clipPos(head)) != 0) {
          return this.atOccurrence = false
        }
      }
      var result = this.matches(reverse, head)
      this.afterEmptyMatch = result && CodeMirror.cmpPos(result.from, result.to) == 0

      if (result) {
        this.pos = result
        this.atOccurrence = true
        return this.pos.match || true
      } else {
        var end = Pos(reverse ? this.doc.firstLine() : this.doc.lastLine() + 1, 0)
        this.pos = { from: end, to: end }
        return this.atOccurrence = false
      }
    },

    from: function () { if (this.atOccurrence) return this.pos.from },
    to: function () { if (this.atOccurrence) return this.pos.to },

    replace: function (newText, origin) {
      if (!this.atOccurrence) return
      var lines = CodeMirror.splitLines(newText)
      this.doc.replaceRange(lines, this.pos.from, this.pos.to, origin)
      this.pos.to = Pos(this.pos.from.line + lines.length - 1,
        lines[lines.length - 1].length + (lines.length == 1 ? this.pos.from.ch : 0))
    }
  }

  CodeMirror.defineExtension("getSearchCursor", function (query, pos, caseFold) {
    return new SearchCursor(this.doc, query, pos, caseFold)
  })
  CodeMirror.defineDocExtension("getSearchCursor", function (query, pos, caseFold) {
    return new SearchCursor(this, query, pos, caseFold)
  })

  CodeMirror.defineExtension("selectMatches", function (query, caseFold) {
    var ranges = []
    var cur = this.getSearchCursor(query, this.getCursor("from"), caseFold)
    while (cur.findNext()) {
      if (CodeMirror.cmpPos(cur.to(), this.getCursor("to")) > 0) break
      ranges.push({ anchor: cur.from(), head: cur.to() })
    }
    if (ranges.length)
      this.setSelections(ranges, 0)
  })
});



// This is from the file dialog.js-------------------------------------------------------------------------

(function (dialog) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    dialog(require("../../lib/codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod);
  else // Plain browser env
    dialog(CodeMirror);
})(function (CodeMirror) {
  function dialogDiv(cm, template, bottom) {
    var wrap = cm.getWrapperElement();
    var dialog;
    dialog = wrap.appendChild(document.createElement("div"));
    if (bottom)
      dialog.className = "CodeMirror-dialog CodeMirror-dialog-bottom";
    else
      dialog.className = "CodeMirror-dialog CodeMirror-dialog-top";

    if (typeof template == "string") {
      dialog.innerHTML = template;
    } else { // Assuming it's a detached DOM element.
      dialog.appendChild(template);
    }
    CodeMirror.addClass(wrap, 'dialog-opened');
    return dialog;
  }

  function closeNotification(cm, newVal) {
    if (cm.state.currentNotificationClose)
      cm.state.currentNotificationClose();
    cm.state.currentNotificationClose = newVal;
  }

  CodeMirror.defineExtension("openDialog", function (template, callback, options) {
    if (!options) options = {};

    closeNotification(this, null);

    var dialog = dialogDiv(this, template, options.bottom);
    var closed = false, me = this;
    function close(newVal) {
      if (typeof newVal == 'string') {
        inp.value = newVal;
      } else {
        if (closed) return;
        closed = true;
        CodeMirror.rmClass(dialog.parentNode, 'dialog-opened');
        dialog.parentNode.removeChild(dialog);
        me.focus();

        if (options.onClose) options.onClose(dialog);
      }
    }

    var inp = dialog.getElementsByTagName("input")[0], button;
    if (inp) {
      inp.focus();

      if (options.value) {
        inp.value = options.value;
        if (options.selectValueOnOpen !== false) {
          inp.select();
        }
      }

      if (options.onInput)
        CodeMirror.on(inp, "input", function (e) { options.onInput(e, inp.value, close); });
      if (options.onKeyUp)
        CodeMirror.on(inp, "keyup", function (e) { options.onKeyUp(e, inp.value, close); });

      CodeMirror.on(inp, "keydown", function (e) {
        if (options && options.onKeyDown && options.onKeyDown(e, inp.value, close)) { return; }
        if (e.keyCode == 27 || (options.closeOnEnter !== false && e.keyCode == 13)) {
          inp.blur();
          CodeMirror.e_stop(e);
          close();
        }
        if (e.keyCode == 13) callback(inp.value, e);
      });

      if (options.closeOnBlur !== false) CodeMirror.on(dialog, "focusout", function (evt) {
        if (evt.relatedTarget !== null) close();
      });
    } else if (button = dialog.getElementsByTagName("button")[0]) {
      CodeMirror.on(button, "click", function () {
        close();
        me.focus();
      });

      if (options.closeOnBlur !== false) CodeMirror.on(button, "blur", close);

      button.focus();
    }
    return close;
  });

  CodeMirror.defineExtension("openConfirm", function (template, callbacks, options) {
    closeNotification(this, null);
    var dialog = dialogDiv(this, template, options && options.bottom);
    var buttons = dialog.getElementsByTagName("button");
    var closed = false, me = this, blurring = 1;
    function close() {
      if (closed) return;
      closed = true;
      CodeMirror.rmClass(dialog.parentNode, 'dialog-opened');
      dialog.parentNode.removeChild(dialog);
      me.focus();
    }
    buttons[0].focus();
    for (var i = 0; i < buttons.length; ++i) {
      var b = buttons[i];
      (function (callback) {
        CodeMirror.on(b, "click", function (e) {
          CodeMirror.e_preventDefault(e);
          close();
          if (callback) callback(me);
        });
      })(callbacks[i]);
      CodeMirror.on(b, "blur", function () {
        --blurring;
        setTimeout(function () { if (blurring <= 0) close(); }, 200);
      });
      CodeMirror.on(b, "focus", function () { ++blurring; });
    }
  });

  /*
   * openNotification
   * Opens a notification, that can be closed with an optional timer
   * (default 5000ms timer) and always closes on click.
   *
   * If a notification is opened while another is opened, it will close the
   * currently opened one and open the new one immediately.
   */
  CodeMirror.defineExtension("openNotification", function (template, options) {
    closeNotification(this, close);
    var dialog = dialogDiv(this, template, options && options.bottom);
    var closed = false, doneTimer;
    var duration = options && typeof options.duration !== "undefined" ? options.duration : 5000;

    function close() {
      if (closed) return;
      closed = true;
      clearTimeout(doneTimer);
      CodeMirror.rmClass(dialog.parentNode, 'dialog-opened');
      dialog.parentNode.removeChild(dialog);
    }

    CodeMirror.on(dialog, 'click', function (e) {
      CodeMirror.e_preventDefault(e);
      close();
    });

    if (duration)
      doneTimer = setTimeout(close, duration);

    return close;
  });
});
