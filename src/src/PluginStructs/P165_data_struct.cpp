#include "../PluginStructs/P165_data_struct.h"

#ifdef USES_P165

P165_data_struct::P165_data_struct(struct EventStruct *event) {
  if (!validGpio(CONFIG_PIN1)) {
    addLog(LOG_LEVEL_ERROR, F("NeoPixel7Segment: GPIO pin invalid."));
    return;
  }

  _stripType     = P165_CONFIG_STRIP_TYPE;
  _pixelGroups   = P165_CONFIG_GROUPCOUNT;
  _defBrightness = P165_CONFIG_DEF_BRIGHT;
  _maxBrightness = P165_CONFIG_MAX_BRIGHT;
  _fgColor       = P165_CONFIG_FG_COLOR;
  _bgColor       = P165_CONFIG_BG_COLOR;
  # if P165_FEATURE_P073 && P165_EXTRA_FONTS
  _fontset = P165_CONFIG_FONTSET;
  # endif // if P165_FEATURE_P073 && P165_EXTRA_FONTS
  _scrollSpeed      = P165_CONFIG_SCROLLSPEED;
  _suppressLeading0 = P165_GET_FLAG_SUPP0;
  _clearOnExit      = P165_GET_FLAG_CLEAR_EXIT;
  _txtScrolling     = P165_GET_FLAG_SCROLL_TEXT;
  _scrollFull       = P165_GET_FLAG_SCROLL_FULL;
  _stdOffset        = P165_GET_FLAG_STD_OFFSET;

  _periods = false; // If we don't have any digits with a decimal dot, disable using that for displaying dots...

  for (uint8_t grp = 0; grp < _pixelGroups; ++grp) {
    memcpy(&_pixelGroupCfg[grp], &P165_GROUP_CFG(grp), sizeof(P165_GROUP_CFG(grp)));
    _pixelGroupCfg[grp].aoffs = 0;
    _pixelGroupCfg[grp].boffs = 0;
    _totalDigits             += _pixelGroupCfg[grp].dgts;
    _periods                 |= _pixelGroupCfg[grp].dotp > 0; // Check decimal dot pixelsS
  }

  const uint16_t pxlCount = calculateDisplayPixels();         // Needs the _pixelGroupCfg filled

  # if P165_DEBUG_INFO
  addLog(LOG_LEVEL_INFO, strformat(F("NeoPixel7Segment: Start stripe for %d pixels, %d digits."), pxlCount, _totalDigits));
  # endif // if P165_DEBUG_INFO
  strip = new (std::nothrow) NeoPixelBus_wrapper(pxlCount, CONFIG_PIN1, P165_STRIP_TYPE_RGBW == _stripType
                                                                          ? NEO_GRBW + NEO_KHZ800
                                                                          : NEO_GRB + NEO_KHZ800);
  _initialized = (nullptr != strip);
  uint16_t pxlOffset = 0;

  if (_initialized) {
    strip->begin(); // Start the strip
    strip->setBrightness(std::min(_maxBrightness, _defBrightness));

    int8_t fromGrp = 0;
    int8_t toGrp   = _pixelGroups;
    int8_t incGrp  = 1;
    bool   allRTL  = true;

    for (uint8_t grp = 0; grp < _pixelGroups; ++grp) {
      allRTL &= _pixelGroupCfg[grp].rtld;
    }

    if (allRTL) { // Revert group order if all groups have RTL set
      fromGrp = _pixelGroups - 1;
      toGrp   = -1;
      incGrp  = -1;
    }
    uint8_t dgtMap = 0; // starting digit

    for (int8_t grp = fromGrp; grp != toGrp; grp += incGrp) {
      fillSegmentBitmap(grp, _pixelGroupCfg[grp]);

      // Set up digit mapping
      uint8_t gOffs = 0;

      for (uint8_t g = 0; g < grp; ++g) { // Determine digit offset
        gOffs += _pixelGroupCfg[g].dgts;
      }

      for (uint8_t d = 0; d < _pixelGroupCfg[grp].dgts; ++d) {
        showmap[dgtMap] = gOffs + d;
        # if P165_DEBUG_DEBUG
        addLog(LOG_LEVEL_INFO, strformat(F("P165 : showmap digit: %d, to group %d digit: %d"), dgtMap + 1, grp + 1, gOffs + d + 1));
        # endif // if P165_DEBUG_DEBUG
        dgtMap++;
      }

      if (_pixelGroupCfg[grp].offs > 0) {
        _pixelGroupCfg[grp].boffs = pxlOffset;
        # if P165_DEBUG_DEBUG
        addLog(LOG_LEVEL_INFO, strformat(F("P165 : group: %d before offset: %d, length: %d"),
                                         grp + 1, _pixelGroupCfg[grp].boffs, _pixelGroupCfg[grp].offs));
        # endif // if P165_DEBUG_DEBUG
      }
      pxlOffset += _pixelGroupCfg[grp].offs;
      const uint8_t pxlDigit = min(static_cast<uint16_t>(63),
                                   calculateGroupPixels(1, // Count pixels for 1 digit only
                                                        _pixelGroupCfg[grp].wpix,
                                                        _pixelGroupCfg[grp].hpix,
                                                        _pixelGroupCfg[grp].crnr,
                                                        _pixelGroupCfg[grp].dotp,
                                                        0)); // Extra pixels passed separately
      # if P165_DEBUG_DEBUG
      addLog(LOG_LEVEL_INFO, strformat(F("P165 : group: %d -------- start-pixel: %d"), grp + 1, pxlOffset + 1));

      for (uint8_t seg = 0; seg < 8; ++seg) {
        addLog(LOG_LEVEL_INFO, strformat(F("P165 : segment: %c, bits: 0b%s"), 'a' + seg,
                                         ull2String(pxlDigit < 63 ? bitSetULL(_segments[grp][seg], pxlDigit + 1)
                                                                  : _segments[grp][seg], 2).substring(1).c_str()));

        if (pxlDigit < 63) { bitClearULL(_segments[grp][seg], pxlDigit + 1); // Reset additional bit
        }
      }
      # endif // if P165_DEBUG_DEBUG

      display[grp] = new (std::nothrow) Noiasca_NeopixelDisplay(*strip,
                                                                _segments[grp],
                                                                _pixelGroupCfg[grp].dgts,
                                                                pxlDigit,
                                                                pxlOffset,
                                                                _pixelGroupCfg[grp].addn,
                                                                offsetLogic_callback);
      pxlOffset += (pxlDigit * _pixelGroupCfg[grp].dgts);

      if (_pixelGroupCfg[grp].addn > 0) {
        _pixelGroupCfg[grp].aoffs = pxlOffset;
        # if P165_DEBUG_DEBUG
        addLog(LOG_LEVEL_INFO, strformat(F("P165 : group: %d add-on offset: %d, length: %d"),
                                         grp + 1, _pixelGroupCfg[grp].aoffs, _pixelGroupCfg[grp].addn));
        # endif // if P165_DEBUG_DEBUG
      }
      pxlOffset += _pixelGroupCfg[grp].addn;

      _initialized = (nullptr != display[grp]);

      if (_initialized) {
        display[grp]->setColorFont(AdaGFXrgb565ToRgb888(_fgColor));
        display[grp]->setColorBack(AdaGFXrgb565ToRgb888(_bgColor));
      }
    }
  }

  if (!_initialized) { // Clean-up on fail
    for (uint8_t grp = 0; grp < PLUGIN_CONFIGLONGVAR_MAX; ++grp) {
      delete display[grp];
    }
    delete strip;
    addLog(LOG_LEVEL_ERROR, F("NeoPixel7Segment: Initialization failed."));
  }
}

int P165_data_struct::offsetLogic_callback(uint16_t position) {
  int offset = 0;

  // if (position > 1 ) offset = addPixels;
  return offset;
}

P165_data_struct::~P165_data_struct() {
  if (_clearOnExit && (nullptr != strip)) {
    const uint16_t pxlCount = calculateDisplayPixels();

    for (uint16_t pxl = 0; pxl < pxlCount; ++pxl) {
      strip->setPixelColor(pxl, 0);
    }
    strip->show();
  }

  for (uint8_t grp = 0; grp < PLUGIN_CONFIGLONGVAR_MAX; ++grp) {
    delete display[grp];
  }
  delete strip;
}

/********************************************************************
* Initialize a single digit group
********************************************************************/
void P165_data_struct::initDigitGroup(struct EventStruct *event,
                                      uint8_t             grp) {
  P165_SET_CONFIG_WPIXELS(grp, 2); // Width 2 pixels
  P165_SET_CONFIG_HPIXELS(grp, 2); // Height 2 pixels
  P165_SET_CONFIG_CORNER(grp, false);
  P165_SET_CONFIG_DOT(grp, 1);     // 1 pixel per decimal point
  P165_SET_CONFIG_EXTRA(grp, 0);
  P165_SET_CONFIG_OFFSET(grp, 0);
  P165_SET_CONFIG_DIGITS(grp, 1);  // 1 digit per group
  P165_SET_CONFIG_START(grp, false);
  P165_SET_CONFIG_DEND(grp, false);
  P165_SET_CONFIG_RTLD(grp, false);
  P165_SET_CONFIG_SPLTG(grp, false);
}

/********************************************************************
* Show the configuration Web UI
********************************************************************/
bool P165_data_struct::plugin_webform_load(struct EventStruct *event) {
  addJavascript();

  const uint8_t grpCount = P165_CONFIG_GROUPCOUNT;

  {
    const __FlashStringHelper *stripOptions[] = { F("GRB"), F("GRBW") };
    const int stripOptionValues[]             = { P165_STRIP_TYPE_RGB, P165_STRIP_TYPE_RGBW };
    addFormSelector(F("Strip Type"), F("stripe"), NR_ELEMENTS(stripOptionValues), stripOptions, stripOptionValues, P165_CONFIG_STRIP_TYPE);
  }

  if ((0 == P165_CONFIG_DEF_BRIGHT) && (0 == P165_CONFIG_MAX_BRIGHT)) {
    P165_CONFIG_DEF_BRIGHT = 40;
    P165_CONFIG_MAX_BRIGHT = 255;
  }

  addFormNumericBox(F("Initial brightness"), F("brght"), P165_CONFIG_DEF_BRIGHT, 0, 255);
  addUnit(F("0..255"));
  addFormNumericBox(F("Maximum allowed brightness"), F("maxbrght"), P165_CONFIG_MAX_BRIGHT, 1, 255);
  addUnit(F("1..255"));

  {
    # if P165_FEATURE_P073
    P073_display_output_selector(F("dspout"), P165_CONFIG_OUTPUTTYPE);
    # else // if P165_FEATURE_P073
    const __FlashStringHelper *displout[] = {
      F("Manual"),
      F("Clock 24h - Blink"),
      F("Clock 24h - No Blink"),
      F("Clock 12h - Blink"),
      F("Clock 12h - No Blink"),
      F("Date"),
    };
    const int disploutOptions[] = {
      P165_DISP_MANUAL,
      P165_DISP_CLOCK24BLNK,
      P165_DISP_CLOCK24,
      P165_DISP_CLOCK12BLNK,
      P165_DISP_CLOCK12,
      P165_DISP_DATE,
    };
    addFormSelector(F("Display Output"), F("dspout"), NR_ELEMENTS(disploutOptions),
                    displout, disploutOptions, P165_CONFIG_OUTPUTTYPE);
    # endif // if P165_FEATURE_P073

    int dgtCount = 0;

    for (uint8_t grp = 0; grp < grpCount; ++grp) {
      dgtCount += P165_GET_CONFIG_DIGITS(grp);
    }
    const int maxOffset = max(1, dgtCount + 1); // Minimum width for standard content is 4 digits
    addFormNumericBox(F("Start at digit"), F("stdoff"), P165_GET_FLAG_STD_OFFSET + 1, 1, max(1, maxOffset - 4),
                      # if FEATURE_TOOLTIPS
                      EMPTY_STRING,
                      # endif // if FEATURE_TOOLTIPS
                      maxOffset <= 5);
    addFormNote(F("Min. 4 digits used for Time/Date Display Output"));
  }

  # if P165_FEATURE_P073 && P165_EXTRA_FONTS
  P073_font_selector(F("fontset"), P165_CONFIG_FONTSET);
  # endif // if P165_FEATURE_P073 && P165_EXTRA_FONTS

  {
    addFormSubHeader(F("Options"));

    addFormCheckBox(F("Suppress leading 0 on day/hour"), F("supp0"),   P165_GET_FLAG_SUPP0);

    addFormCheckBox(F("Scroll text &gt; display width"), F("scrltxt"), P165_GET_FLAG_SCROLL_TEXT);
    addFormCheckBox(F("Scroll text in from right"),      F("scrlfll"), P165_GET_FLAG_SCROLL_FULL);

    if (P165_CONFIG_SCROLLSPEED == 0) { P165_CONFIG_SCROLLSPEED = 10; }
    addFormNumericBox(F("Scroll speed (0.1 sec/step)"), F("scrlspd"), P165_CONFIG_SCROLLSPEED, 1, 600);
    addUnit(F("1..600 = 0.1..60 sec/step"));
    addFormCheckBox(F("Clear display on exit"), F("clrexit"), P165_GET_FLAG_CLEAR_EXIT);
  }

  addFormSubHeader(F("Display"));

  addFormNote(F("Fields with *: When changed will save and reload the page."));

  const __FlashStringHelper *digitOptions[]      = { F("1"), F("2"), F("3"), F("4") };
  const int digitOptionValues[]                  = { 1, 2, 3, 4 };
  const __FlashStringHelper *startPixelOptions[] = { F("Left-top"), F("Right-top") };

  const String fgColor = ADAGFX_WHITE == P165_CONFIG_FG_COLOR || ADAGFX_BLACK == P165_CONFIG_FG_COLOR
                          ? EMPTY_STRING
                          : AdaGFXrgb565ToWebColor(P165_CONFIG_FG_COLOR);

  addFormSelector(F("Number of Groups *"),
                  F("grps"),
                  NR_ELEMENTS(digitOptionValues),
                  digitOptions,
                  digitOptionValues,
                  grpCount,
                  true);

  AdaGFXFormForeAndBackColors(F("fgcolor"),
                              P165_CONFIG_FG_COLOR,
                              F("bgcolor"),
                              P165_CONFIG_BG_COLOR);

  const bool numberPlan = P165_GET_FLAG_NUMBERPLAN > 0;
  addFormSelector_YesNo(F("Show Pixel number-plan *"), F("nbrpln"), P165_GET_FLAG_NUMBERPLAN, true);
  addFormNote(F("When set to Yes will block digit-changes."));

  int totalPixels = 0;
  int totalDigits = 0;
  int8_t fromGrp  = 0;
  int8_t toGrp    = grpCount;
  int8_t incGrp   = 1;
  bool   allRTL   = true;

  for (uint8_t grp = 0; grp < grpCount; ++grp) {
    allRTL &= P165_GET_CONFIG_RTLD(grp);
  }

  addFormSubHeader(F("Groups and Digits"));

  if (allRTL) { // Revert group order if all groups have RTL set
    fromGrp = grpCount - 1;
    toGrp   = -1;
    incGrp  = -1;
    addFormNote(F("Attention, Groups are shown in reverse order!"));
  }

  int16_t dgtOffset = 0;

  for (int8_t grp = fromGrp; grp != toGrp; grp += incGrp) {
    const uint8_t grp10 = grp * 10;

    if (0 == P165_GET_CONFIG_WPIXELS(grp)) { // Check for invalid settings
      initDigitGroup(event, grp);
    }

    const uint8_t grpDgts  = P165_GET_CONFIG_DIGITS(grp);
    const uint8_t grpWPxls = P165_GET_CONFIG_WPIXELS(grp);
    const uint8_t grpHPxls = P165_GET_CONFIG_HPIXELS(grp);
    const uint8_t grpCrnr  = P165_GET_CONFIG_CORNER(grp);
    const uint8_t grpDotP  = P165_GET_CONFIG_DOT(grp);
    const uint8_t grpAddN  = P165_GET_CONFIG_EXTRA(grp);
    const uint8_t grpOffs  = P165_GET_CONFIG_OFFSET(grp);
    const uint8_t grpRtld  = P165_GET_CONFIG_RTLD(grp);
    # if P165_FEATURE_C_CLOCKWISE
    const uint8_t grpCclkw = P165_GET_CONFIG_CCLKW(grp);
    # else // if P165_FEATURE_C_CLOCKWISE
    const uint8_t grpCclkw = false;
    # endif // if P165_FEATURE_C_CLOCKWISE

    if (grp != fromGrp) {
      addFormSeparator(2);
    }

    addRowLabel(concat(F("Group "), grp + 1));

    {
      html_table(F(""));

      addRowLabel(F("Number of Digits *"));
      addSelector(concat(F("dgts"), grp10),
                  NR_ELEMENTS(digitOptionValues),
                  digitOptions,
                  digitOptionValues, nullptr,
                  grpDgts,
                  true, !numberPlan); // 1st and 2nd column
      {
        // 3rd column = "Digit <nr>" / "(Extra)"
        for (uint8_t dgt = 0; dgt < grpDgts; ++dgt) {
          if ((0 == dgt) && grpRtld) { html_TD(); addHtml(F("(Extra)")); }
          html_TD(); addHtml(F("Digit "));
          addHtmlInt(static_cast<int8_t>(dgt + 1));
        }

        if (!grpRtld) { html_TD(); addHtml(F("(Extra)")); }
      }
    }

    addFormNumericBox(F("Segment Width pixels"), concat(F("wdth"), grp10),
                      grpWPxls, 1, P165_SEGMENT_WIDTH_PIXELS,
                      # if FEATURE_TOOLTIPS
                      EMPTY_STRING,
                      # endif // if FEATURE_TOOLTIPS
                      numberPlan);

    dgtOffset += grpOffs;

    const int16_t dgtPxls = calculateGroupPixels(1, // Count single digit without additional pixels
                                                 grpWPxls,
                                                 grpHPxls,
                                                 grpCrnr,
                                                 grpDotP,
                                                 0);

    {
      if (grpRtld) { // Take care of Right To Left configured groups
        dgtOffset += (dgtPxls * (grpDgts - 1));
      }

      for (uint8_t dgt = 0; dgt < grpDgts; ++dgt) {
        drawSevenSegment(dgt, grp10, // 3rd column = subtable with digit
                         grpWPxls,
                         grpHPxls,
                         grpCrnr,
                         grpDotP,
                         grpAddN,
                         grpDgts - 1,
                         dgtOffset,
                         P165_GET_CONFIG_START(grp),
                         P165_GET_CONFIG_DEND(grp),
                         fgColor,
                         numberPlan,
                         grpOffs,
                         P165_GET_CONFIG_SPLTG(grp),
                         grpRtld,
                         grpCclkw);
        dgtOffset += (dgtPxls * (grpRtld ? -1 : 1));
      }

      if (grpRtld) {
        dgtOffset += (dgtPxls * (grpDgts + 1)); // Add 'm all back
      }
      dgtOffset += grpAddN;
    }

    {
      addFormNumericBox(F("Segment Height pixels"), concat(F("hght"), grp10),
                        grpHPxls, 1, P165_SEGMENT_HEIGHT_PIXELS,
                        # if FEATURE_TOOLTIPS
                        EMPTY_STRING,
                        # endif // if FEATURE_TOOLTIPS
                        numberPlan);

      addFormCheckBox(F("Segment Corners overlap"), concat(F("crnr"), grp10),
                      grpCrnr, numberPlan);

      addFormNumericBox(F("Decimal dot pixels"), concat(F("decp"), grp10),
                        grpDotP, 0, P165_SEGMENT_DOT_PIXELS,
                        # if FEATURE_TOOLTIPS
                        EMPTY_STRING,
                        # endif // if FEATURE_TOOLTIPS
                        numberPlan);
    }
    {
      addFormNumericBox(F("Extra pixels after"), concat(F("addn"), grp10),
                        grpAddN, 0, P165_SEGMENT_ADDON_PIXELS,
                        # if FEATURE_TOOLTIPS
                        EMPTY_STRING,
                        # endif // if FEATURE_TOOLTIPS
                        numberPlan);

      const int grpPixels = calculateGroupPixels(grpDgts,
                                                 grpWPxls,
                                                 grpHPxls,
                                                 grpCrnr,
                                                 grpDotP,
                                                 grpAddN);
      totalPixels += grpPixels;
      totalDigits += grpDgts;

      addFormNumericBox(F("Pixels in group"), concat(F("totp"), grp10),
                        grpPixels,
                        0, INT32_MAX,
                        # if FEATURE_TOOLTIPS
                        EMPTY_STRING,
                        # endif // if FEATURE_TOOLTIPS
                        true);
    }
    addHtml(F("<TR><TD style='height:2px' colspan='2'><hr>")); // Reduce height of separator

    {
      addFormNumericBox(F("Pixel-offset before"), concat(F("offs"), grp10),
                        grpOffs, 0, P165_SEGMENT_EXTRA_PIXELS,
                        # if FEATURE_TOOLTIPS
                        EMPTY_STRING,
                        # endif // if FEATURE_TOOLTIPS
                        numberPlan);
      totalPixels += grpOffs;

      addRowLabel(F("Starting pixel"));
      addSelector(concat(F("strt"), grp10),
                  NR_ELEMENTS(startPixelOptions),
                  startPixelOptions,
                  nullptr, nullptr,
                  P165_GET_CONFIG_START(grp), false,
                  !numberPlan);

      addFormCheckBox(F("Split g-segment pixels"), concat(F("spltg"), grp10),
                      P165_GET_CONFIG_SPLTG(grp), numberPlan);

      addFormCheckBox(F("Decimal dot last segment"), concat(F("dend"), grp10),
                      P165_GET_CONFIG_DEND(grp), numberPlan);

      # if P165_FEATURE_C_CLOCKWISE
      addFormCheckBox(F("Numbering counter-clockwise"), concat(F("cclkw"), grp10),
                      grpCclkw, numberPlan);
      # endif // if P165_FEATURE_C_CLOCKWISE

      addFormCheckBox(F("Right to Left digits"), concat(F("rtld"), grp10),
                      grpRtld, numberPlan);

      # if P165_DIGIT_TABLE_H_INT > 17
      int rws = 17; // Above should be fixed number of rows, matching with ~80% digit table size

      for (; rws < P165_DIGIT_TABLE_H_INT; ++rws) {
        html_TR_TD();
        addHtml(F("&nbsp;")); // We need P165_digit_table_h_int rows for the digit table to work as intended
      }
      # endif // if P165_DIGIT_TABLE_H_INT > 17
    }

    html_end_table();

    {
      // Bind handlers on input fields to update the 7 segment simulation and digit counts
      addHtml(F("\n<script type='text/javascript'>"));
      const __FlashStringHelper *_fmt = F("document.getElementById('%s%d').onchange=function(){%s(this.%s,%d,%d,%d,'%s'),"
                                          "dgts(%d,['wdth','hght','decp','addn','offs','dgts'])};");

      //                          fieldname   index        function    fieldattribute   function arguments
      addHtml(strformat(_fmt, FsP(F("wdth")), grp10, FsP(F("chWdth")), FsP(F("value")), grp10, grpDgts, false, "",
                        grpCount));
      addHtml(strformat(_fmt, FsP(F("hght")), grp10, FsP(F("chHght")), FsP(F("value")), grp10, grpDgts, false, "",
                        grpCount));
      addHtml(strformat(_fmt, FsP(F("crnr")), grp10, FsP(F("chCrnr")), FsP(F("checked")), grp10, grpDgts, false, fgColor.c_str(),
                        grpCount));
      addHtml(strformat(_fmt, FsP(F("decp")), grp10, FsP(F("chDecp")), FsP(F("value")), grp10, grpDgts, false, "",
                        grpCount));
      addHtml(strformat(_fmt, FsP(F("addn")), grp10, FsP(F("chAddn")), FsP(F("value")), grp10, grpDgts, true, "",
                        grpCount));
      addHtml(strformat(F("document.getElementById('offs%d').onchange=function(){dgts(%d,['wdth','hght','decp','addn','offs','dgts'])};"),
                        grp10, grpCount));
      addHtml(F("</script>"));
    }
  }

  addFormSeparator(2);

  addFormNumericBox(F("Total digits"), F("totdgt"),
                    totalDigits,
                    0, INT32_MAX,
                    # if FEATURE_TOOLTIPS
                    EMPTY_STRING,
                    # endif // if FEATURE_TOOLTIPS
                    true);
  addFormNumericBox(F("Total pixels to connect"), F("totpx"),
                    totalPixels,
                    0, INT32_MAX,
                    # if FEATURE_TOOLTIPS
                    EMPTY_STRING,
                    # endif // if FEATURE_TOOLTIPS
                    true);

  return true;
}

/********************************************************************
* Save the updated settings
********************************************************************/
bool P165_data_struct::plugin_webform_save(struct EventStruct *event) {
  const int16_t grps = getFormItemInt(F("grps"));

  P165_CONFIG_GROUPCOUNT = grps;
  P165_CONFIG_STRIP_TYPE = getFormItemInt(F("stripe"));
  P165_CONFIG_OUTPUTTYPE = getFormItemInt(F("dspout"));
  P165_CONFIG_DEF_BRIGHT = getFormItemInt(F("brght"));
  P165_CONFIG_MAX_BRIGHT = getFormItemInt(F("maxbrght"));
  # if P165_FEATURE_P073 && P165_EXTRA_FONTS
  P165_CONFIG_FONTSET = getFormItemInt(F("fontset"));
  # endif // if P165_FEATURE_P073 && P165_EXTRA_FONTS
  P165_CONFIG_SCROLLSPEED = getFormItemInt(F("scrlspd"));

  P165_SET_FLAG_SUPP0(isFormItemChecked(F("supp0")) ? 1 : 0);
  P165_SET_FLAG_SCROLL_TEXT(isFormItemChecked(F("scrltxt")) ? 1 : 0);
  P165_SET_FLAG_SCROLL_FULL(isFormItemChecked(F("scrlfll")) ? 1 : 0);
  P165_SET_FLAG_CLEAR_EXIT(isFormItemChecked(F("clrexit")) ? 1 : 0);
  int stdoff = 1;

  if (update_whenset_FormItemInt(F("stdoff"), stdoff)) {
    P165_SET_FLAG_STD_OFFSET(stdoff - 1); // Show 1..16, used as offset 0..15
  }

  const bool prevNumberPlan = P165_GET_FLAG_NUMBERPLAN > 0;

  P165_SET_FLAG_NUMBERPLAN(getFormItemInt(F("nbrpln")));

  String color = webArg(F("fgcolor"));

  P165_CONFIG_FG_COLOR = ADAGFX_RED;                // Default to red when empty

  if (!color.isEmpty()) {
    P165_CONFIG_FG_COLOR = AdaGFXparseColor(color); // Reduce to rgb565
  }
  color                = webArg(F("bgcolor"));
  P165_CONFIG_BG_COLOR = AdaGFXparseColor(color);   // Empty = black

  if (!prevNumberPlan) {                            // Don't save now as the read-only inputs will return empty values
    for (int grp = 0; grp < grps; ++grp) {
      const uint8_t grp10 = grp * 10;
      P165_SET_CONFIG_WPIXELS(grp, getFormItemInt(concat(F("wdth"), grp10)));
      P165_SET_CONFIG_HPIXELS(grp, getFormItemInt(concat(F("hght"), grp10)));
      P165_SET_CONFIG_CORNER(grp, isFormItemChecked(concat(F("crnr"), grp10)));
      P165_SET_CONFIG_DOT(grp, getFormItemInt(concat(F("decp"), grp10)));
      P165_SET_CONFIG_EXTRA(grp, getFormItemInt(concat(F("addn"), grp10)));
      P165_SET_CONFIG_OFFSET(grp, getFormItemInt(concat(F("offs"), grp10)));
      P165_SET_CONFIG_DIGITS(grp, getFormItemInt(concat(F("dgts"), grp10)));
      P165_SET_CONFIG_START(grp, getFormItemInt(concat(F("strt"), grp10)));
      P165_SET_CONFIG_DEND(grp, isFormItemChecked(concat(F("dend"), grp10)));
      P165_SET_CONFIG_RTLD(grp, isFormItemChecked(concat(F("rtld"), grp10)));
      P165_SET_CONFIG_SPLTG(grp, isFormItemChecked(concat(F("spltg"), grp10)));
      # if P165_FEATURE_C_CLOCKWISE
      P165_SET_CONFIG_CCLKW(grp, isFormItemChecked(concat(F("cclkw"), grp10)));
      # endif // if P165_FEATURE_C_CLOCKWISE
      # if P165_SEGMENT_WIDTH_PIXELS > 7 || P165_SEGMENT_HEIGHT_PIXELS > 7
      const uint8_t grpPixels = calculateGroupPixels(1,
                                                     P165_GET_CONFIG_WPIXELS(grp),
                                                     P165_GET_CONFIG_HPIXELS(grp),
                                                     P165_GET_CONFIG_CORNER(grp),
                                                     P165_GET_CONFIG_DOT(grp),
                                                     0);

      if (grpPixels > 64) {
        addHtmlError(strformat(F("Error: Group %d &gt; 64 pixels configured (%d)"), grp + 1, grpPixels));
      }
      # endif // if P165_SEGMENT_WIDTH_PIXELS > 7 || P165_SEGMENT_HEIGHT_PIXELS > 7
    }
  }
  return true;
}

/********************************************************************
* Calculate the number of pixels in a group
********************************************************************/
uint16_t P165_data_struct::calculateGroupPixels(const uint8_t count,
                                                const uint8_t wpixels,
                                                const uint8_t hpixels,
                                                const bool    overlap,
                                                const uint8_t decPt,
                                                const uint8_t addN) {
  return count * (3 * wpixels + 4 * hpixels + (overlap ? 6 : 0) + decPt) + addN;
}

/********************************************************************
* Calculate the total number of pixels for the entire display
********************************************************************/
uint16_t P165_data_struct::calculateDisplayPixels() {
  uint16_t result = 0;

  for (uint8_t grp = 0; grp < _pixelGroups; ++grp) {
    result += _pixelGroupCfg[grp].offs;
    result += calculateGroupPixels(_pixelGroupCfg[grp].dgts,
                                   _pixelGroupCfg[grp].wpix,
                                   _pixelGroupCfg[grp].hpix,
                                   _pixelGroupCfg[grp].crnr,
                                   _pixelGroupCfg[grp].dotp,
                                   _pixelGroupCfg[grp].addn);
  }
  return result;
}

/********************************************************************
* Calculate the total number of digits configured
********************************************************************/
uint16_t P165_data_struct::calculateDisplayDigits() {
  uint16_t result = 0;

  for (uint8_t grp = 0; grp < _pixelGroups; ++grp) {
    result += _pixelGroupCfg[grp].dgts;
  }
  return result;
}

// Draw a 7segment digit with optional decimal point and extra pixels, and max 5-wide segments and 5-high segments
constexpr uint16_t P165_digitMask[P165_DIGIT_TABLE_H_INT] = { // Regular digit layout, max size
  0b0111111100,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b0111111100,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b1000000010,
  0b0111111101,                                              // Decimal point pixels
};

constexpr uint8_t P165_extraMask[P165_DIGIT_TABLE_H_INT] = { // Extra pixel table mask
  0b0,
  0b0,
  0b0,
  0b0,
  0b0,
  0b0,
  0b0,
  0b0,
  0b1, // Extra pixels after
  0b0,
  0b0,
  0b0,
  0b0,
  0b0,
  0b0,
  0b0,
  0b0,
};

constexpr uint16_t P165_digitOverlap[P165_DIGIT_TABLE_H_INT] = { // Overlap enabled, overlay these 6 pixels
  0b1000000010,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b1000000010,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b0000000000,
  0b1000000010,
};

/********************************************************************
* Draw a 7-segment digit-group by creating a table
* with some rows and columns hidden when not max. size
********************************************************************/
void P165_data_struct::drawSevenSegment(const uint8_t  digit,   // Digit
                                        const uint8_t  grp10,   // Group * 10
                                        const uint8_t  wpixels, // width pixels
                                        const uint8_t  hpixels, // heoght pixels
                                        const bool     overlap, // corner overlap
                                        const uint8_t  decPt,   // decimal point pixels
                                        const uint8_t  addN,    // additional pixels
                                        const uint8_t  max,     // max already has 1 subtracted
                                        const uint16_t offset,  // pre-offset
                                        const bool     strt,    // start left-top or right-top
                                        const bool     dend,    // decimal point at end
                                        const String & fgColor, // foreground color
                                        const bool     dspPlan, // show number plan
                                        const int16_t  aOffs,   // addon offset
                                        const bool     splitG,  // split segment G in 2 halves
                                        const bool     rtld,    // direction ltr or rtl
                                        const bool     cclkw) { // counter-clockwise
  constexpr uint8_t hrMax  = NR_ELEMENTS(P165_digitMask) - 1;   // Height row max
  constexpr uint8_t hcMask = hrMax / 2;                         // Height center
  const uint8_t     wrMask = 7;                                 // Width right
  const uint8_t     wdMask = 8;                                 // Width dot pos
  const uint8_t     wcBits = 10;                                // Width column bits

  int8_t tblFrom = 0;
  int8_t tblTo   = 1;
  int8_t tblInc  = 1;

  if ((aOffs >= 0) && (digit == (rtld ? 0 : max))) {
    if (rtld) {
      tblFrom = 1;
      tblTo   = -1;
      tblInc  = -1;
    } else {
      tblTo = 2;
    }
  }

  for (int8_t tbl = tblFrom; tbl != tblTo; tbl += tblInc) {
    const bool drawDigit   = 0 == tbl;
    const uint8_t maskBits = drawDigit ? wcBits : 1;

    addHtml(F("<TD rowspan='" P165_DIGIT_TABLE_HEIGHT "' style='padding:0;'>"));
    addHtml(strformat(F("<table id='d%ctbl%d'>"), drawDigit ? 'g' : 'x', digit + grp10)); // Group is factor 10

    uint8_t hor = 0;
    int8_t  ver = 0;
    uint8_t seg = 0;

    for (uint8_t h = 0; h < NR_ELEMENTS(P165_digitMask); ++h) {
      const bool showRow = !(((h < hcMask) && (hcMask - h >= hpixels) && (h > 1)) ||
                             ((h > hcMask) && (h - hcMask > hpixels) && (h < hrMax)));
      addHtml(F("<TR style='height:" P165_TD_SIZE ";display:"));

      if (!showRow) {
        addHtml(F("none'>"));  // Hide row
      } else {
        addHtml(F("block'>")); // Show row

        if ((h == 0) || (h == hcMask)) {
          ver = 0;             // Restart vertical counter
        } else {
          ver++;
        }
      }

      hor = 0; // Restart horizontal counter

      for (uint8_t w = 0; w < maskBits; ++w) {
        String pIndex;

        const bool showCol = !(w > 0 && w >= wpixels && w < wrMask);

        html_TD(showCol ? (dspPlan && (w == maskBits - 1)
                                        ? F("min-width:" P165_TD_SIZE ";max-width:4rem;font-size:80%;display:inline-block")
                                        : F("width:" P165_TD_SIZE ";font-size:80%;display:inline-block"))
                        : F("width:" P165_TD_SIZE ";display:none"));

        const bool showBit = drawDigit
                            ? (bitRead(P165_digitMask[h], (maskBits - 1) - w) ||
                               (overlap && bitRead(P165_digitOverlap[h], (maskBits - 1) - w)))
                            : bitRead(P165_extraMask[h], (maskBits - 1) - w);

        if (showBit) {
          if (dspPlan && showRow && showCol) {         // Determine segment for pixel-indexes:
            if ((h > 0) && (h < hcMask)) {             // b/f
              if (w == 0) {
                seg = 5;                               // f
              } else {
                seg = 1;                               // b
              }
            } else if ((h > hcMask) && (h < hrMax)) {  // c/e
              if (w == 0) {
                seg = 4;                               // e
              } else {
                seg = 2;                               // c
              }
            } else if ((w > wdMask) && (h == hrMax)) { // Decimal point
              seg = 7;
            } else if (h == hcMask) {                  // g / Additional pixels
              if ((w > wdMask) || !drawDigit) {
                seg = 8;                               // virtual 9th segment
              } else {
                seg = 6;                               // g
              }
            } else if ((w <= wdMask) && (h == hrMax)) {
              seg = 3;                                 // d
            } else {
              seg = 0;                                 // a
            }

            pIndex = calculatePixelIndex(hor,
                                         ver - 1,
                                         seg,
                                         offset,
                                         wpixels,
                                         hpixels,
                                         overlap,
                                         strt,
                                         dend,
                                         decPt,
                                         addN,
                                         splitG,
                                         cclkw);

            // pIndex = strformat(F("%d/%d/%d"), hor, ver - 1, seg); // For debugging only
            hor++;
          }

          if ((w <= wdMask) && drawDigit) {
            if (dspPlan) {
              addHtml(pIndex);
            } else {
              if (!fgColor.isEmpty()) {           // Colored pixel
                addHtml(strformat(F("<span style='color:%s;'>" P165_PIXEL_CHARACTER "</span>"), fgColor.c_str()));
              } else {
                addHtml(F(P165_PIXEL_CHARACTER)); // Pixel
              }
            }
          } else if (h == hcMask) {               // Extra pixels after last digit
            if ((addN > 0) && ((digit == max) || !drawDigit)) {
              if (dspPlan) {
                addHtml(pIndex);
              } else {
                addHtmlInt(addN);   // Show number of pixels
              }
            } else {
              addHtml(F("&nbsp;")); // None
            }
          } else if (h == hrMax) {  // Decimal point
            if (decPt > 0) {
              if (dspPlan) {
                addHtml(pIndex);
              } else {
                addHtmlInt(decPt);  // Show number of pixels
              }
            } else {
              addHtml(F("&nbsp;")); // None
            }
          } else {
            addHtml('?');           // this shouldn't ever show up... ;-)
          }
        } else {
          addHtml(F("&nbsp;"));     // No pixel
        }
      }
    }
    html_end_table();
  }
}

/************************************************************************
 * Segment order maps, depending on configuration settings:
 * - top-left or top-right (0/2)
 * - dot-after-c or dot-at-end (0/1)
 * - g-segment as a single 'block' or split in 2 halves (0/4)
 * - counter-clockwise numbering (0/8)
 ***********************************************************************/

/* *INDENT-OFF* */
const uint8_t P165_segmentMap[][9] PROGMEM = { // 72 or 144 bytes, values: 0..7 segments, 16/26 => 6, 255 = ignore
  { 0, 1,  2,  7,  3,  4,  6,  5,  255 },      // top-left, dot after c, order: a, b, c, h, d, e, g, f, -
  { 0, 1,  2,  3,  4,  6,  5,  7,  255 },      // top-left, dot as last, order: a, b, c, d, e, g, f, h, -
  { 1, 2,  7,  3,  4,  6,  5,  0,  255 },      // top-right, dot after c, order: b, c, h, d, e, g, f, a, -
  { 1, 2,  3,  4,  6,  5,  0,  7,  255 },      // top-right, dot as last, order: b, c, d, e, g, f, a, h, -
  { 0, 1,  16, 2,  7,  3,  4,  26, 5   },      // top-left, dot after c, split g, order: a, b, g1, c, h, d, e, g2, f
  { 0, 1,  16, 2,  3,  4,  26, 5,  7   },      // top-left, dot as last, split g, order: a, b, g1, c, d, e, g2, f, h
  { 1, 16, 2,  7,  3,  4,  26, 5,  0   },      // top-right, dot after c, split g, order: b, g1, c, h, d, e, g2, f, a
  { 1, 16, 2,  3,  4,  26, 5,  0,  7   },      // top-right, dot as last, split g, order: b, g1, c, d, e, g2, f, a, h
  # if P165_FEATURE_C_CLOCKWISE
  { 5, 6,  4,  3,  7,  2,  1,  0,  255 },      // cclkw, top-left, dot after c, order: f, g, e, d, h, c, b, a, -
  { 5, 6,  4,  3,  2,  1,  0,  7,  255 },      // cclkw, top-left, dot as last, order: f, g, e, d, c, b, a, h, -
  { 0, 5,  6,  4,  3,  7,  2,  1,  255 },      // cclkw, top-right, dot after c, order: a, f, g, e, d, h, c, b, -
  { 0, 5,  6,  4,  3,  2,  1,  7,  255 },      // cclkw, top-right, dot as last, order: a, f, g, e, d, c, b, h, -
  { 5, 26, 4,  3,  7,  2,  16, 1,  0   },      // cclkw, top-left, dot after c, split g, order: f, g2, e, d, h, c, g1, b, a
  { 5, 26, 4,  3,  2,  16, 1,  0,  7   },      // cclkw, top-left, dot as last, split g, order: f, g2, e, d, c, g1, b, a, h
  { 0, 5,  26, 4,  3,  7,  2,  16, 1   },      // cclkw, top-right, dot after c, split g, order: a, f, g2, e, d, h, c, g1, b
  { 0, 5,  26, 4,  3,  2,  16, 1,  7   },      // cclkw, top-right, dot as last, split g, order: a, f, g2, e, d, c, g1, b, h
  # endif // if P165_FEATURE_C_CLOCKWISE
};
/* *INDENT-ON* */
constexpr uint8_t P165_segmentCnt = NR_ELEMENTS(P165_segmentMap[0]);

/*************************************************************************
 * Returns the pixel-index as a string to show in the UI for a pixel
 ************************************************************************/
String P165_data_struct::calculatePixelIndex(const uint8_t  hor,     // horizontal 'pixel'
                                             const int8_t   ver,     // vertical 'pixel'
                                             const uint8_t  seg,     // segment
                                             const uint16_t offset,  // pre-offset
                                             const uint8_t  wpixels, // width pixels
                                             const uint8_t  hpixels, // height pixels
                                             const bool     overlap, // corner overlap
                                             const bool     strt,    // start left-top or right-top
                                             const bool     dend,    // decimal point at end
                                             const uint8_t  decPt,   // decimal point pixels
                                             const uint8_t  addN,    // additional pixels
                                             const bool     splitG,  // split G segment in 2 halves
                                             const bool     cclkw) { // counter-clockwise
  int16_t result(offset);
  const uint8_t hpx  = wpixels + (overlap ? 2 : 0);                  // Overlapping pixels checked on horizontal segments
  const uint8_t vpx  = hpixels;                                      // Vertical pixels
  const uint8_t smap = (strt ? 2 : 0) + (dend ? 1 : 0) + (splitG ? 4 : 0) + (cclkw ? 8 : 0);
  const uint8_t rh   = hpx / 2;                                      // Horizontal half part, right
  const uint8_t lh   = hpx - rh;                                     // Horizontal half part, left
  int8_t fh          = 0;                                            // from-h
  int8_t th          = hpx;                                          // to-h
  int8_t ih          = 1;                                            // inc-h
  int8_t vh          = hpx - 1;
  int8_t mh          = -1;                                           // minmax-h

  # if P165_FEATURE_C_CLOCKWISE

  if (cclkw) {
    fh = hpx - 1;
    th = -1;
    ih = -1;
    vh = 0;
    mh = hpx;
  }
  # endif // if P165_FEATURE_C_CLOCKWISE

  result++;                                                      // 1-based index

  for (uint8_t spoint = 0; spoint < P165_segmentCnt; ++spoint) { // loop over all segments a..i
    const uint8_t segment = pgm_read_byte(&(P165_segmentMap[smap][spoint]));

    if ((0 == segment) || (6 == segment)) {                      // horizontal segments a/g
      for (int8_t h = fh; h != th; h += ih) {
        if ((seg == segment) && (hor == h)) {
          return String(result);
        }
        result++;
      }
    } else if (16 == segment) { // horizontal segment g, right half
      for (int8_t h = rh; h < hpx; ++h) {
        if ((6 == seg) && (hor == h)) {
          return String(result);
        }
        result++;
      }
    } else if (26 == segment) { // horizontal segment g, left half
      for (int8_t h = 0; h < hpx - lh; ++h) {
        if ((6 == seg) && (hor == h)) {
          return String(result);
        }
        result++;
      }
    } else if (3 == segment) { // horizontal segment d reversed
      for (int8_t h = vh; h != mh; h -= ih) {
        if ((seg == segment) && (hor == h)) {
          return String(result);
        }
        result++;
      }
    } else if (7 == segment) { // Decimal point segment
      if (seg == segment) {
        String res(result);
        res.reserve(decPt * 4);

        for (uint8_t dp = 1; dp < decPt; ++dp) {
          result++;
          res = strformat(F("%s %d"), res.c_str(), result);
        }
        return res;
      } else {
        result += decPt;
      }
    } else if ((!cclkw && ((4 == segment) || (5 == segment)))   // Vertical segments e/f
               # if P165_FEATURE_C_CLOCKWISE
               || (cclkw && ((1 == segment) || (2 == segment))) // Vertical segments b/c, counter-clockwise
               # endif // if P165_FEATURE_C_CLOCKWISE
               ) {
      for (int8_t v = vpx - 1; v >= 0; --v) {
        if ((seg == segment) && (ver == v)) {
          return String(result);
        }
        result++;
      }
    } else if ((!cclkw && ((1 == segment) || (2 == segment)))   // Vertical segments b/c
               # if P165_FEATURE_C_CLOCKWISE
               || (cclkw && ((4 == segment) || (5 == segment))) // Vertical segments e/f, counter-clockwise
               # endif // if P165_FEATURE_C_CLOCKWISE
               ) {
      // } else if (255 != segment) { // Vertical segments b/c
      for (uint8_t v = 0; v < vpx; ++v) {
        if ((seg == segment) && (ver == v)) {
          return String(result);
        }
        result++;
      }
    }
  }

  if ((seg == 8) && (addN > 0)) {
    String res(result);
    res.reserve(addN * 4);

    for (uint8_t a = 1; a < addN; ++a) {
      result++;
      res = strformat(F("%s %d"), res.c_str(), result);
    }
    return res;
  }
  return F("x"); // We shouldn't get here...
}

/**************************************************************************
 * Setup the pixels to be lit per segment for a digit
 *************************************************************************/
void P165_data_struct::fillSegmentBitmap(const uint8_t       grp,
                                         const PixelGroupCfg pixCfg) {
  if (grp > _pixelGroups) { return; }

  segsize_t pbit    = 0;
  const uint8_t hpx = pixCfg.wpix + (pixCfg.crnr ? 2 : 0); // Overlapping pixels checked on horizontal segments
  const uint8_t vpx = pixCfg.hpix;                         // Vertical pixels = height pixels
  // smap: Determine the segment(part) order
  const uint8_t smap = (pixCfg.strt ? 2 : 0) + (pixCfg.dend ? 1 : 0) + (pixCfg.splt ? 4 : 0) + (pixCfg.cclkw ? 8 : 0);
  const uint8_t rh   = hpx / 2;                            // Horizontal half part, right
  const uint8_t lh   = hpx - rh;                           // Horizontal half part, left

  // loop over all segments a..h but don't exceed 64 bits
  for (uint8_t spoint = 0; spoint < P165_segmentCnt && pbit < 64; ++spoint) {
    const uint8_t segment = pgm_read_byte(&(P165_segmentMap[smap][spoint]));

    if ((0 == segment) || (3 == segment) || (6 == segment)) { // horizontal segments a/d/g
      if (pixCfg.crnr) {
        for (uint8_t h = 0; h < hpx && pbit < 64; ++h) {
          bitSetULL(_segments[grp][segment], pbit);

          if ((0 == segment) && (h == 0)) {       // left pixel (a,f)
            bitSetULL(_segments[grp][5], pbit);
          } else
          if ((0 == segment) && (h == hpx - 1)) { // right pixel (a,b)
            bitSetULL(_segments[grp][1], pbit);
          } else
          if ((3 == segment) && (h == 0)) {       // right pixel (d,c), (other direction)
            bitSetULL(_segments[grp][2], pbit);
          } else
          if ((3 == segment) && (h == hpx - 1)) { // left pixel (d,e), (other direction)
            bitSetULL(_segments[grp][4], pbit);
          } else
          if ((6 == segment) && (h == 0)) {       // left pixel (g,e,f)
            bitSetULL(_segments[grp][4], pbit);
            bitSetULL(_segments[grp][5], pbit);
          } else
          if ((6 == segment) && (h == hpx - 1)) { // right pixel (g,b,c)
            bitSetULL(_segments[grp][1], pbit);
            bitSetULL(_segments[grp][2], pbit);
          }
          pbit++; // next pixel
        }
      } else {
        for (uint8_t h = 0; h < hpx && pbit < 64; ++h) {
          bitSetULL(_segments[grp][segment], pbit);
          pbit++;
        }
      }
    } else if (16 == segment) { // Horizontal segment g, right half
      for (uint8_t h = rh; h < hpx && pbit < 64; ++h) {
        bitSetULL(_segments[grp][6], pbit);

        if (pixCfg.crnr && (h == hpx - 1)) { // right pixel (g,b,c)
          bitSetULL(_segments[grp][1], pbit);
          bitSetULL(_segments[grp][2], pbit);
        }
        pbit++;
      }
    } else if (26 == segment) { // Horizontal segment g, left half
      for (uint8_t h = 0; h < hpx - lh && pbit < 64; ++h) {
        bitSetULL(_segments[grp][6], pbit);

        if (pixCfg.crnr && (h == 0)) { // left pixel (g,e,f)
          bitSetULL(_segments[grp][4], pbit);
          bitSetULL(_segments[grp][5], pbit);
        }
        pbit++;
      }
    } else if (7 == segment) { // Decimal point segment
      for (uint8_t dp = 0; dp < pixCfg.dotp && pbit < 64; ++dp) {
        bitSetULL(_segments[grp][segment], pbit);
        pbit++;
      }
    } else if (255 != segment) { // Vertical segments b/c/e/f
      for (uint8_t v = 0; v < vpx && pbit < 64; ++v) {
        bitSetULL(_segments[grp][segment], pbit);
        pbit++;
      }
    }
  }
}

/****************************************************************************************
 * Fetch from local file system, flash or CDN
 ***************************************************************************************/
void P165_data_struct::addJavascript() {
  serve_JS(JSfiles_e::P165_digit); // Source in static/p165_digit.js, minified script source in src/src/Static/WebStaticData.h
}

/***************************************************************************
 * Update display with standard content once a second
 **************************************************************************/
bool P165_data_struct::plugin_once_a_second(struct EventStruct *event) {
  if (_output == P165_DISP_MANUAL) {
    return false;
  }

  if ((_output == P165_DISP_CLOCK24BLNK) ||
      (_output == P165_DISP_CLOCK12BLNK)) {
    _timesep = !_timesep;
  } else {
    _timesep = true; // On
  }

  if (_output == P165_DISP_DATE) {
    fillBufferWithDate(true, 0, 0, 0,
                       _suppressLeading0,
                       _stdOffset);
  } else {
    fillBufferWithTime(true, 0, 0, 0,
                       !((_output == P165_DISP_CLOCK24BLNK) ||
                         (_output == P165_DISP_CLOCK24)),
                       _suppressLeading0,
                       _stdOffset);
  }
  writeBufferToDisplay();

  // FIXME Determine what group(s) are used and set the extra pixels for those groups
  uint8_t dgts             = 0;
  const uint8_t dgtsNeeded = std::min(_totalDigits, static_cast<uint8_t>(P165_DISP_DATE == _output ? 8 : 6));

  for (uint8_t grp = 0; grp < _pixelGroups; ++grp) {
    dgts += _pixelGroupCfg[grp].dgts;
    addLog(LOG_LEVEL_INFO, strformat(F("P165 : seconds marker %d Group %d, digits %d offset %d"), _timesep, grp, dgts, _stdOffset));

    if ((_stdOffset <= dgts) && (dgtsNeeded - _stdOffset > dgts)) {
      extraPixelsState(grp + 1, _timesep ? 1 : 0, AdaGFXrgb565ToRgb888(_timesep ? _fgColor : _bgColor));
    }
  }

  return true;
}

/***************************************************************************
 * Scroll text in 0.1 second steps
 **************************************************************************/
bool P165_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if ((_output != P165_DISP_MANUAL) || !isScrollEnabled()) {
    return false;
  }

  if (nextScroll()) {
    writeBufferToDisplay();
  }
  return true;
}

/*************************************************************************
 * Handle commands, quite compatible with P073
 ************************************************************************/
const char p165_commands[] PROGMEM =
  "7dtext|"
  # if P165_FEATURE_P073
  "7dfont|"
  # endif // if P165_FEATURE_P073
  "7dbin|"
  "7dextra|"
  "7dbefore|"
  "7don|"
  "7doff|"
  "7db|"
  "7output|"
  "7dgroup|"
  "7color|"
  "7digit|"
  # if P165_FEATURE_DIGITCOLOR
  "7digitcolor|"
  # endif // if P165_FEATURE_DIGITCOLOR
  # if P165_FEATURE_GROUPCOLOR
  "7groupcolor|"
  # endif // if P165_FEATURE_GROUPCOLOR
;
enum class p165_commands_e : int8_t {
  invalid = -1,
  c7dtext,
  # if P165_FEATURE_P073
  c7dfont,
  # endif // if P165_FEATURE_P073
  c7dbin,
  c7dextra,
  c7dbefore,
  c7don,
  c7doff,
  c7db,
  c7output,
  c7group,
  c7color,
  c7digit,
  # if P165_FEATURE_DIGITCOLOR
  c7digitcolor,
  # endif // if P165_FEATURE_DIGITCOLOR
  # if P165_FEATURE_GROUPCOLOR
  c7groupcolor,
  # endif // if P165_FEATURE_GROUPCOLOR
};

bool P165_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  const String cmd_s = parseString(string, 1);

  const int cmd_i = GetCommandCode(cmd_s.c_str(), p165_commands);

  if (cmd_i < 0) { return false; } // Fail fast

  const bool currentScroll = isScrollEnabled(); // Save current state
  bool newScroll           = currentScroll;     // keep scroll if command changes

  const p165_commands_e cmd = static_cast<p165_commands_e>(cmd_i);

  const String text = parseStringToEndKeepCase(string, 2);
  bool success      = false;
  bool displayon    = false;

  switch (cmd) {
    case p165_commands_e::c7dtext:          // Set text to show
      setScrollEnabled(0 == _currentGroup); // Scrolling allowed for 7dtext command on entire display only
      return plugin_write_7dtext(text);
      break;
    # if P165_FEATURE_P073
    case p165_commands_e::c7dfont: // Select font
      return plugin_write_7dfont(event, text);
      break;
    # endif // if P165_FEATURE_P073
    case p165_commands_e::c7dbin:           // Send binary data
      setScrollEnabled(0 == _currentGroup); // Scrolling allowed for 7dbin command on entire display only
      return plugin_write_7dbin(text);
      break;
    case p165_commands_e::c7don:            // Display on
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("Neo7Seg : Display ON"));
      # endif // ifndef BUILD_NO_DEBUG
      displayon = true;
      success   = true;
      break;
    case p165_commands_e::c7doff: // Display off
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("Neo7Seg : Display OFF"));
      # endif // ifndef BUILD_NO_DEBUG
      displayon = false;
      success   = true;
      break;
    case p165_commands_e::c7db: // Set brightness

      if ((event->Par1 >= 0) && (event->Par1 <= _maxBrightness)) {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("Neo7Seg : Brightness="), event->Par1));
        }
        # endif // ifndef BUILD_NO_DEBUG
        _defBrightness         = event->Par1;
        P165_CONFIG_DEF_BRIGHT = event->Par1;
        displayon              = true;
        success                = true;
      }
      break;
    case p165_commands_e::c7output:                  // 7output,<output> : Select display output

      if ((event->Par1 >= 0) && (event->Par1 < 6)) { // 0:"Manual",1:"Clock 24h - Blink",2:"Clock 24h - No Blink",
                                                     // 3:"Clock 12h - Blink",4:"Clock 12h - No Blink",5:"Date"
        # if P165_DEBUG_INFO

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("Neo7Seg : Display output="), event->Par1));
        }
        # endif // if P165_DEBUG_INFO
        _output                = event->Par1;
        P165_CONFIG_OUTPUTTYPE = event->Par1;
        displayon              = true;
        success                = true;

        if (event->Par1 != 0) { newScroll = false; } // Scrolling only for Manual
      }
      break;
    case p165_commands_e::c7group: // 7group,<grp> : 0 = global, 1..configured groups

      if ((event->Par1 >= 0) && (event->Par1 <= _pixelGroups)) {
        _currentGroup = event->Par1;
        success       = true;
      }
      break;
    case p165_commands_e::c7color: // 7color,fg_r,fg_g,fg_b[,fg_w],bg_r,bg_g,bg_b[,bg_w]
    {
      const bool rgbW    = P165_STRIP_TYPE_RGBW == _stripType;
      uint32_t   fgColor = 0;
      uint32_t   bgColor = 0;
      bool fgSet         = false;
      bool bgSet         = false;

      if (parseRGBWColors(parseStringToEnd(string, 2), rgbW, fgColor, bgColor, fgSet, bgSet)) {
        uint8_t from = 0;
        uint8_t to   = _pixelGroups;

        if (_currentGroup > 0) {
          from = _currentGroup - 1;
          to   = _currentGroup;
        } else {
          if (fgSet) { _fgColor = AdaGFXrgb888ToRgb565(fgColor); }

          if (bgSet) { _bgColor = AdaGFXrgb888ToRgb565(bgColor); }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLog(LOG_LEVEL_INFO, strformat(F("Neo7Dgt: Colors: FG: 0x%08x BG: 0x%08x"), fgColor, bgColor));
          }
        }

        for (uint8_t i = from; i < to; ++i) {
          if (fgSet) {
            display[i]->setColorFont(fgColor);
            success = true;
          }

          if (bgSet) {
            display[i]->setColorBack(bgColor);
            success = true;
          }

          if (success && (_currentGroup > 0) && loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLog(LOG_LEVEL_INFO, strformat(F("Neo7Dgt: Group: %d colors: FG: 0x%08x BG: 0x%08x"), i + 1, fgColor, bgColor));
          }
        }
      }
      break;
    }
    case p165_commands_e::c7digit: // 7digit,<grp>,<dgt>,<char>[.] : write a single character to a digit, with optional period
      // Scrolling not allowed for 7digit command
      newScroll = false;
      success   = plugin_write_7digit(text);
      break;
    case p165_commands_e::c7dextra:  // 7dextra,<grp>,<0|1|2|3|4|5>[,<r>,<g>,<b>[,<w>]] : Set the 'Extra pixels after' on/off
    // with optional color, group 0 sets the color to all groups additional pixels
    case p165_commands_e::c7dbefore: // 7dbefore,<grp>,<0|1|2|3|4|5>[,<r>,<g>,<b>[,<w>]] : Set the 'Pixel-offset before' on/off
    {
      const uint8_t par1     = event->Par1;
      const bool    pxlExtra = p165_commands_e::c7dextra == cmd;

      if (((0 == par1) || ((par1 > 0) && (par1 <= _pixelGroups) &&
                           pxlExtra ? (_pixelGroupCfg[par1 - 1].addn > 0)
                                    : (_pixelGroupCfg[par1 - 1].offs > 0)
                           )) &&                        // single group have extra pixels after?
          !parseString(string, 3).isEmpty()) {          // on/off is given
        const bool rgbW    = P165_STRIP_TYPE_RGBW == _stripType;
        const bool onState = 1 == (event->Par2 & 0x01); // Check for On (1/3/5) or Off (0/2/4)
        uint32_t   fgColor = 0;
        uint32_t   bgColor = 0;                         // Ignored!
        bool fgSet         = false;
        bool bgSet         = false;                     // Ignored!

        parseRGBWColors(parseStringToEnd(string, 4), rgbW, fgColor, bgColor, fgSet, bgSet);
        const uint32_t newColor = onState
                                    ? (fgSet ? fgColor : AdaGFXrgb565ToRgb888(_fgColor))
                                    : (fgSet ? fgColor : AdaGFXrgb565ToRgb888(_bgColor));

        extraPixelsState(par1, event->Par2, newColor, pxlExtra);
        success = true;
      }
      break;
    }
    # if P165_FEATURE_DIGITCOLOR
    case p165_commands_e::c7digitcolor: // 7digitcolor,grp,dgt,[-|fg_r,fg_g,fg_b[,fg_w][,bg_r,bg_g,bg_b[,bg_w]]]
    # endif // if P165_FEATURE_DIGITCOLOR
    # if P165_FEATURE_GROUPCOLOR
    case p165_commands_e::c7groupcolor: // 7groupcolor,grp,[-|fg_r,fg_g,fg_b[,fg_w][,bg_r,bg_g,bg_b[,bg_w]]]
    # endif // if P165_FEATURE_GROUPCOLOR
    # if P165_FEATURE_DIGITCOLOR || P165_FEATURE_GROUPCOLOR
    {
      const bool rgbW     = P165_STRIP_TYPE_RGBW == _stripType;
      uint8_t    grp      = 0;
      uint8_t    dgt      = 0;
      uint32_t   fgColor  = 0;
      uint32_t   bgColor  = 0;
      const bool grpColor =
        #  if P165_FEATURE_GROUPCOLOR
        p165_commands_e::c7groupcolor == cmd;
        #  else // if P165_FEATURE_GROUPCOLOR
        false;
        #  endif // if P165_FEATURE_GROUPCOLOR
      bool fgSet = false;
      bool bgSet = false;

      // First 2 arguments: group and digit
      if ((event->Par1 > 0) && (event->Par1 <= _pixelGroups)) {
        grp = event->Par1;
      }

      if ((grp > 0) && !grpColor && ((event->Par2 > 0) && (event->Par2 <= _pixelGroupCfg[grp - 1].dgts))) {
        dgt = event->Par2;
      }

      if (((0 == dgt) && !grpColor) || (grpColor && (0 == grp))) {
        break; // Invalid grp or dgt arguments
      }

      const uint16_t grp100dgt = 0x100 + (grp << 4) + (dgt - 1);
      const uint16_t grp200dgt = 0x200 + (grp << 4) + (dgt - 1);
      const uint16_t grp300    = 0x300 + (grp << 4);
      const uint16_t grp400    = 0x400 + (grp << 4);
      const uint8_t  parIdx    = grpColor ? 3 : 4;
      const String   par4      = parseString(string, parIdx);

      // Next argument: delete digit/groupcolor?
      if (equals(par4, F("-"))) {                                  // use dash to remove a digit/group color setting
        auto it = digitColors.find(grpColor ? grp300 : grp100dgt); // fg color

        if (it != digitColors.end()) {
          digitColors.erase(it);
        }
        it = digitColors.find(grpColor ? grp400 : grp200dgt); // bg color

        if (it != digitColors.end()) {
          digitColors.erase(it);
        }
        success = true;
      } else
      if (parseRGBWColors(parseStringToEnd(string, parIdx), rgbW, fgColor, bgColor, fgSet, bgSet)) {
        if (fgSet) {
          auto it = digitColors.find(grpColor ? grp300 : grp100dgt);     // fg color

          if (it != digitColors.end()) {                                 // Update fg color
            it->second = fgColor;
          } else {
            digitColors.emplace(grpColor ? grp300 : grp100dgt, fgColor); // New fg color
          }
          success = true;
        }

        if (bgSet) {
          auto it = digitColors.find(grpColor ? grp400 : grp200dgt);     // bg color

          if (it != digitColors.end()) {                                 // Update bg color
            it->second = bgColor;
          } else {
            digitColors.emplace(grpColor ? grp400 : grp200dgt, bgColor); // New bg color
          }
          success = true;
        }
      }
      #  if P165_DEBUG_DEBUG

      for (auto const& it : digitColors) {
        addLog(LOG_LEVEL_INFO, strformat(F("P165 : 7digit/groupcolor key: 0x%03x color: 0x%08x"), it.first, it.second));
      }
      #  endif // if P165_DEBUG_DEBUG
      break;
    }
    # endif // if P165_FEATURE_DIGITCOLOR || P165_FEATURE_GROUPCOLOR
    case p165_commands_e::invalid:
      break;
  }

  if (success) {
    if (displayon) {
      // TODO (On)
    } else {
      // TODO (Off)
    }
    setScrollEnabled(newScroll);

    strip->setBrightness(std::min(_maxBrightness, _defBrightness));
  }
  return success;
}

/**************************************************************************************
 * set the state (color) of the extra befoer/after pixels of 1 or all(0) groups,
 * state: 0/1: all off/on, 2/3: first half, 4/5: second half
 *************************************************************************************/
bool P165_data_struct::extraPixelsState(uint8_t group, uint8_t state, uint32_t color, bool pxlExtra) {
  const uint8_t pxPart  = (state >> 1);               // Check what part to switch, cut off lowest bit
  const uint8_t fromGrp = 0 == group ? 0 : group - 1; // All or 1 group
  const uint8_t toGrp   = 0 == group ? _pixelGroups : group;

  for (uint8_t grp = fromGrp; grp < toGrp; ++grp) {
    const uint8_t xPixels = pxlExtra ? _pixelGroupCfg[grp].addn : _pixelGroupCfg[grp].offs;

    if (xPixels > 0) {
      uint16_t pxFrom = 0;
      uint16_t pxTo   = xPixels;

      if ((xPixels > 1) && (pxPart > 0)) { // 0 = full block
        if (1 == pxPart) {                 // 2/3 first half off/on
          pxTo = xPixels / 2;
        } else {                           // 4/5 second half off/on
          pxFrom = xPixels / 2;
        }
      }
      const uint16_t xOffset = pxlExtra ? _pixelGroupCfg[grp].aoffs : _pixelGroupCfg[grp].boffs;

      for (uint16_t i = pxFrom; i < pxTo; ++i) {
        display[grp]->setPixelColor(i + xOffset, color);
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO,
               strformat(F("Neo7Dgt: Set group %d extra %c pixels %d..%d to color 0x%08x"),
                         grp + 1,
                         pxlExtra ? 'a' : 'b',
                         xOffset + 1 + pxFrom,
                         xOffset + pxTo,
                         color));
      }
    }
  }

  strip->show(); // Show the output
  return true;
}

/**********************************************************************************************
 * Parse a string to foreground and background RGB or RGBW values, each requires 3 or 4 values
 *********************************************************************************************/
bool P165_data_struct::parseRGBWColors(const String& string,
                                       bool          rgbW,
                                       uint32_t    & fgColor,
                                       uint32_t    & bgColor,
                                       bool        & fgSet,
                                       bool        & bgSet) {
  uint32_t clr_r = 0;
  uint32_t clr_g = 0;
  uint32_t clr_b = 0;
  uint32_t clr_w = 0;
  uint8_t  rgbI  = 0;

  String par_r = parseString(string, 1);
  String par_g = parseString(string, 2);
  String par_b = parseString(string, 3);
  String par_w;

  if (rgbW) { par_w = parseString(string, 4); }

  // Foreground color check
  if (!par_r.isEmpty() && !par_g.isEmpty() && !par_b.isEmpty() && (!rgbW || !par_w.isEmpty())) {
    validUIntFromString(par_r, clr_r); // No value check as value is &-limited
    validUIntFromString(par_g, clr_g);
    validUIntFromString(par_b, clr_b);

    if (rgbW) { validUIntFromString(par_w, clr_w); }

    fgColor = ((clr_r & 0xFF) << 16) + ((clr_g & 0xFF) << 8) + (clr_b & 0xFF) + ((clr_w & 0xFF) << 24);
    fgSet   = true;
  }

  // Background color check
  if (rgbW) { rgbI++; } // Extra offset needed
  par_r = parseString(string, 4 + rgbI);
  par_g = parseString(string, 5 + rgbI);
  par_b = parseString(string, 6 + rgbI);

  if (rgbW) { par_w = parseString(string, 7 + rgbI); }

  if (!par_r.isEmpty() && !par_g.isEmpty() && !par_b.isEmpty() && (!rgbW || !par_w.isEmpty())) {
    validUIntFromString(par_r, clr_r); // No value check as value is &-limited
    validUIntFromString(par_g, clr_g);
    validUIntFromString(par_b, clr_b);

    if (rgbW) {
      clr_w = 0;
      validUIntFromString(par_w, clr_w);
    }

    bgColor = ((clr_r & 0xFF) << 16) + ((clr_g & 0xFF) << 8) + (clr_b & 0xFF) + ((clr_w & 0xFF) << 24);
    bgSet   = true;
  }
  return fgSet || bgSet; // Skipping all arguments is not a valid outcome
}

/******************************************************************************************
 * Write some text to the buffer to display. Scroll if that's enabled and text doesn't fit
 *****************************************************************************************/
bool P165_data_struct::plugin_write_7dtext(const String& text) {
  if (_output != P165_DISP_MANUAL) {
    return false;
  }
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, strformat(F("Neo7Seg : Show Text=%s, group %d"), text.c_str(), _currentGroup));
  }
  # endif // ifndef BUILD_NO_DEBUG
  setTextToScroll(EMPTY_STRING);
  const uint8_t bufLen = calculateDisplayDigits();

  if (isScrollEnabled() && (getEffectiveTextLength(text) > bufLen)) {
    setTextToScroll(text);
  } else {
    fillBufferWithString(text);

    writeBufferToDisplay(_currentGroup);
  }
  return true;
}

# if P165_FEATURE_P073

/***********************************************************************
 * Select a font by name or number
 **********************************************************************/
bool P165_data_struct::plugin_write_7dfont(struct EventStruct *event,
                                           const String      & text) {
  if (!text.isEmpty()) {
    const int32_t fontNr = P073_parse_7dfont(event, text);
    #  if P165_DEBUG_INFO

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("P165 7dfont,%s -> %d"), parseString(text, 1).c_str(), fontNr));
    }
    #  endif // if P165_DEBUG_INFO

    if ((fontNr >= 0) && (fontNr <= 3)) {
      _fontset            = fontNr;
      P165_CONFIG_FONTSET = fontNr;
      return true;
    }
  }
  return false;
}

# endif // if P165_FEATURE_P073

/**********************************************************************************************
 * Write binary data into the buffer to display, provide bytes with data matching the segments
 *********************************************************************************************/
bool P165_data_struct::plugin_write_7dbin(const String& text) {
  if (!text.isEmpty()) {
    String  data;
    int32_t byteValue{};
    int     arg      = 1;
    String  argValue = parseString(text, arg);

    while (!argValue.isEmpty()) {
      if (validIntFromString(argValue, byteValue) && (byteValue < 256) && (byteValue > -1)) {
        data += static_cast<char>(byteValue);
      }
      arg++;
      argValue = parseString(text, arg);
    }
    # if P165_DEBUG_INFO
    addLog(LOG_LEVEL_INFO, strformat(F("7dbin: text: %s, data len: %d"), text.c_str(), data.length()));
    # endif // if P165_DEBUG_INFO
    const uint8_t bufLen = calculateDisplayDigits();

    if (!data.isEmpty()) {
      setTextToScroll(EMPTY_STRING); // Clear any scrolling text

      if (isScrollEnabled() && (data.length() > bufLen)) {
        setBinaryData(data);
      } else {
        fillBufferWithString(data, true);

        writeBufferToDisplay(_currentGroup);
      }
      return true;
    }
  }
  return false;
}

/****************************************************************************
 * Write a character to an explicit digit in a group
 ***************************************************************************/
bool P165_data_struct::plugin_write_7digit(const String& text) {
  if (!text.isEmpty()) {
    uint32_t grp      = 0;
    uint32_t dgt      = 0;
    const String data = parseStringKeepCase(text, 3);

    if (validUIntFromString(parseString(text, 1), grp) && (grp > 0) && (grp <= _pixelGroups) &&
        validUIntFromString(parseString(text, 2), dgt) && (dgt > 0) && (dgt <= _pixelGroupCfg[grp - 1].dgts) &&
        !data.isEmpty()) {
      grp--;
      dgt--;
      # if P165_FEATURE_P073
      const uint8_t cdata = P073_mapCharToFontPosition(data.charAt(0), _fontset);
      # else // if P165_FEATURE_P073
      const uint8_t cdata = data.charAt(0);
      # endif // if P165_FEATURE_P073
      writeCharacterToDisplay(grp, dgt, cdata, data.length() > 1 && data.charAt(1) == '.');
      strip->show();
      return true;
    }
  }
  return false;
}

/**********************************************************************************
 * Put a string into the buffer, periods are smashed into the previous digit
 *********************************************************************************/
void P165_data_struct::fillBufferWithString(const String& textToShow,
                                            bool          useBinaryData) {
  _binaryData = useBinaryData;
  clearBuffer();
  const int txtlength = textToShow.length();
  uint8_t   bufLen    = calculateDisplayDigits();

  int p = 0;

  if (_currentGroup > 0) {
    bufLen = _pixelGroupCfg[_currentGroup].dgts;            // Adjust length

    for (uint8_t grp = 0; grp < _currentGroup - 1; ++grp) { // Find current group start digit
      p += _pixelGroupCfg[grp].dgts;                        // Add offset
    }
  }

  uint8_t mp = 0;

  for (int i = 0; i < txtlength && p <= bufLen; ++i) { // p <= bufLen to allow a period after last digit
    mp = showmap[p];                                   // digit mapping

    if (_periods
        && (textToShow.charAt(i) == '.')
        && !_binaryData
        ) {         // If setting periods true
      if (p == 0) { // Text starts with a period, becomes a space with a dot
        showperiods[mp] = true;
        showbuffer[mp]  =
          # if P165_FEATURE_P073
          10  // space in 7dgt fonts
          # else // if P165_FEATURE_P073
          ' ' // space
          # endif // if P165_FEATURE_P073
        ;
        p++;
        mp = showmap[p]; // update
      } else {
        // if (p > 0) {
        mp              = showmap[p - 1];
        showperiods[mp] = true;                           // The period displays as a dot on the previous digit!
      }

      if ((i > 0) && (textToShow.charAt(i - 1) == '.')) { // Handle consecutive periods
        p++;

        if ((p - 1) < bufLen) {
          mp              = showmap[p - 1];
          showperiods[mp] = true; // The period displays as a dot on the previous digit!
        }
      }
    } else if (p < bufLen) {
      # if P165_FEATURE_P073
      showbuffer[mp] = useBinaryData
                        ? textToShow.charAt(i)
                        : P073_mapCharToFontPosition(textToShow.charAt(i), _fontset);
      # else // if P165_FEATURE_P073
      showbuffer[mp] = textToShow.charAt(i);
      # endif // if P165_FEATURE_P073
      p++;
    }
  }
  # if P165_DEBUG_INFO
  logBufferContent(F("7dtext"));
  # endif // if P165_DEBUG_INFO
}

void P165_data_struct::setBinaryData(const String& data) {
  _binaryData = true;
  setTextToScroll(data);
  _binaryData  = true; // is reset in setTextToScroll
  _scrollCount = _scrollSpeed;
  _scrollPos   = 0;
}

void P165_data_struct::fillBufferWithTime(const bool    sevendgt_now,
                                          uint8_t       sevendgt_hours,
                                          uint8_t       sevendgt_minutes,
                                          uint8_t       sevendgt_seconds,
                                          const bool    flag12h,
                                          const bool    suppressLeading0,
                                          const uint8_t offset) {
  clearBuffer();

  if (sevendgt_now) {
    sevendgt_hours   = node_time.hour();
    sevendgt_minutes = node_time.minute();
    sevendgt_seconds = node_time.second();
  }

  if (flag12h && (sevendgt_hours > 12)) {
    sevendgt_hours -= 12; // if flag 12h is TRUE and h>12 adjust subtracting 12
  }

  if (flag12h && (sevendgt_hours == 0)) {
    sevendgt_hours = 12; // if flag 12h is TRUE and h=0  adjust to h=12
  }
  put4NumbersInBuffer(sevendgt_hours, sevendgt_minutes, sevendgt_seconds, -1, suppressLeading0, offset);
}

void P165_data_struct::fillBufferWithDate(const bool    sevendgt_now,
                                          uint8_t       sevendgt_day,
                                          uint8_t       sevendgt_month,
                                          const int     sevendgt_year,
                                          const bool    suppressLeading0,
                                          const uint8_t offset) {
  clearBuffer();
  int sevendgt_year0 = sevendgt_year;

  if (sevendgt_now) {
    sevendgt_day   = node_time.day();
    sevendgt_month = node_time.month();
    sevendgt_year0 = node_time.year();
  } else if (sevendgt_year0 < 100) {
    sevendgt_year0 += 2000;
  }
  const uint8_t sevendgt_year1 = static_cast<uint8_t>(sevendgt_year0 / 100);
  const uint8_t sevendgt_year2 = static_cast<uint8_t>(sevendgt_year0 % 100);

  put4NumbersInBuffer(sevendgt_day, sevendgt_month, sevendgt_year1, sevendgt_year2, suppressLeading0, offset);
}

void P165_data_struct::put4NumbersInBuffer(const uint8_t nr1,
                                           const uint8_t nr2,
                                           const uint8_t nr3,
                                           const int8_t  nr4,
                                           const bool    suppressLeading0,
                                           const uint8_t offset) {
  uint8_t cOffs = 0;

  # if !P165_FEATURE_P073
  cOffs = '0'; // Fallback to ASCII if P073 not available
  # endif // if !P165_FEATURE_P073

  # if P165_DEBUG_DEBUG
  addLog(LOG_LEVEL_INFO,
         strformat(F("P165 : put4NumbersInBuffer: a:%d b:%d c:%d d:%d cOffs:%d offset:%d"),
                   nr1, nr2, nr3, nr4, cOffs, offset));
  # endif // if P165_DEBUG_DEBUG

  // offset shouldn't ever be > buffer size - 4
  showbuffer[showmap[0 + offset]] = static_cast<uint8_t>((nr1 / 10) + cOffs);
  showbuffer[showmap[1 + offset]] = (nr1 % 10) + cOffs;
  showbuffer[showmap[2 + offset]] = static_cast<uint8_t>((nr2 / 10) + cOffs);
  showbuffer[showmap[3 + offset]] = (nr2 % 10) + cOffs;

  if ((nr3 > -1) && ((5 + offset) < _totalDigits)) {
    showbuffer[showmap[4 + offset]] = static_cast<uint8_t>((nr3 / 10) + cOffs);
    showbuffer[showmap[5 + offset]] = (nr3 % 10) + cOffs;
  }

  if ((nr4 > -1) && ((7 + offset) < _totalDigits)) {
    showbuffer[showmap[6 + offset]] = static_cast<uint8_t>((nr4 / 10) + cOffs);
    showbuffer[showmap[7 + offset]] = (nr4 % 10) + cOffs;
  }

  if (suppressLeading0 && (showbuffer[showmap[0 + offset]] == cOffs)) {
    showbuffer[showmap[0 + offset]] =
      # if P165_FEATURE_P073
      10
      # else // if P165_FEATURE_P073
      ' '
      # endif // if P165_FEATURE_P073
    ; // set to space
  }
}

/***********************************************
 * Nothing in the buffer
 **********************************************/
void P165_data_struct::clearBuffer() {
  memset(showbuffer,
         _binaryData ? 0 :
         # if P165_FEATURE_P073
         10,
         # else // if P165_FEATURE_P073
         ' ', // space
         # endif // if P165_FEATURE_P073
         sizeof(showbuffer));

  for (uint8_t i = 0; i < P165_SHOW_BUFFER_SIZE; ++i) {
    showperiods[i] = false;
  }
}

/********************************************************************************************
 * Write a single character-mask to a digit
 *******************************************************************************************/
void P165_data_struct::writeCharacterToDisplay(uint8_t group, uint8_t digit, uint8_t character, bool period) {
  uint8_t data =
    # if P165_FEATURE_P073
    _binaryData ?
    # endif // if P165_FEATURE_P073
    character
    # if P165_FEATURE_P073 // Use P073 7-segment fonts and buffer content when available
    : P073_revert7bits(P073_getFontChar(character, _fontset))
    # endif // if P165_FEATURE_P073
  ;

  if (!_binaryData && _periods && period) {
    data |= 0x80;                   // Decimal point On
  }

  display[group]->setCursor(digit); // Have to move the cursor ourselves for writeLowLevel and at pos. 0
  # if P165_FEATURE_P073
  display[group]->writeLowLevel(digit, digit2SegmentMap(group, data));
  # else // if P165_FEATURE_P073

  if (_binaryData) {
    display[group]->writeLowLevel(digit, digit2SegmentMap(group, data));
  } else {
    display[group]->write(data); // Fallback to ASCII in buffer and built-in font if P073 not available
  }
  # endif // if P165_FEATURE_P073
}

/*************************************************************************************
 * Send current buffer content to the display
 ************************************************************************************/
void P165_data_struct::writeBufferToDisplay(uint8_t group) {
  uint8_t i          = 0;
  const uint8_t from = (group == 0 ? 0 : group - 1);
  const uint8_t to   = (group == 0 ? _pixelGroups : std::min(group, _pixelGroups));

  # if P165_DEBUG_DEBUG
  String log;
  # endif // if P165_DEBUG_DEBUG

  for (uint8_t grp = from; grp < to; ++grp) {
    uint32_t fgColor = AdaGFXrgb565ToRgb888(_fgColor);
    uint32_t bgColor = AdaGFXrgb565ToRgb888(_bgColor);
    # if P165_FEATURE_GROUPCOLOR
    bool fgColorGrp       = false;
    bool bgColorGrp       = false;
    const uint16_t grp300 = 0x300 + ((grp + 1) << 4);
    const uint16_t grp400 = 0x400 + ((grp + 1) << 4);
    auto itgrp            = digitColors.find(grp300); // fg color

    if (itgrp != digitColors.end()) {
      fgColor = itgrp->second;
      display[grp]->setColorFont(fgColor);
      fgColorGrp = true;
    }
    itgrp = digitColors.find(grp400); // bg color

    if (itgrp != digitColors.end()) {
      bgColor = itgrp->second;
      display[grp]->setColorBack(bgColor);
      bgColorGrp = true;
    }
    # endif // if P165_FEATURE_GROUPCOLOR

    for (uint8_t dgt = 0; dgt < _pixelGroupCfg[grp].dgts; ++dgt) {
      # if P165_DEBUG_DEBUG

      if ((grp == from) && (dgt == 0)) {
        log += F("P165 : BufToDisp: ");
      }
      # endif // if P165_DEBUG_DEBUG
      # if P165_FEATURE_DIGITCOLOR

      // apply per-digit colormap
      bool fgColorSet          = false;
      bool bgColorSet          = false;
      const uint16_t grp100dgt = 0x100 + ((grp + 1) << 4) + dgt;
      const uint16_t grp200dgt = 0x200 + ((grp + 1) << 4) + dgt;
      auto it                  = digitColors.find(grp100dgt); // fg color

      if (it != digitColors.end()) {
        display[grp]->setColorFont(it->second);
        fgColorSet = true;
      }
      it = digitColors.find(grp200dgt); // bg color

      if (it != digitColors.end()) {
        display[grp]->setColorBack(it->second);
        bgColorSet = true;
      }
      # endif // if P165_FEATURE_DIGITCOLOR

      writeCharacterToDisplay(grp, dgt, showbuffer[i], showperiods[i]);

      # if P165_FEATURE_DIGITCOLOR

      // Restore global colors
      if (fgColorSet) {
        display[grp]->setColorFont(fgColor);
      }

      if (bgColorSet) {
        display[grp]->setColorBack(bgColor);
      }
      # endif // if P165_FEATURE_DIGITCOLOR

      # if P165_DEBUG_DEBUG
      log += strformat(F(" g:%d d:%d 0x%02x"), grp, dgt, showbuffer[i]);
      # endif // if P165_DEBUG_DEBUG
      ++i;
    }
    # if P165_FEATURE_GROUPCOLOR

    // Restore global colors
    if (fgColorGrp) {
      display[grp]->setColorFont(AdaGFXrgb565ToRgb888(_fgColor));
    }

    if (bgColorGrp) {
      display[grp]->setColorBack(AdaGFXrgb565ToRgb888(_bgColor));
    }
    # endif // if P165_FEATURE_GROUPCOLOR
  }
  # if P165_DEBUG_DEBUG
  addLogMove(LOG_LEVEL_INFO, log);
  # endif // if P165_DEBUG_DEBUG
  strip->show(); // Show all content at once
}

/*************************************************************************************
 * fetch the pixels to turn on for a digit
 ************************************************************************************/
segsize_t P165_data_struct::digit2SegmentMap(uint8_t grp, uint8_t digit) {
  segsize_t currentBitmap = 0u; // clear the current bitmap

  for (byte i = 0; i < 8; i++) {
    if (digit & (1UL << i)) {   // UL not necessary, but uses less Flash than if you leave it away
      currentBitmap |= _segments[grp][i];
    }
  }

  # if P165_DEBUG_DEBUG

  const uint8_t pxlDigit = min(static_cast<uint16_t>(63),
                               calculateGroupPixels(1, // Count pixels for 1 digit only
                                                    _pixelGroupCfg[grp].wpix,
                                                    _pixelGroupCfg[grp].hpix,
                                                    _pixelGroupCfg[grp].crnr,
                                                    _pixelGroupCfg[grp].dotp,
                                                    0));

  // Used for debugging
  addLog(LOG_LEVEL_INFO, strformat(F("digit2SegmentMap: digit: 0x%02x, bits: 0b%s"), digit,
                                   ull2String(pxlDigit < 63 ? bitSetULL(currentBitmap, pxlDigit + 1)
                                                            : currentBitmap, 2).substring(1).c_str()));

  if (pxlDigit < 63) { bitClearULL(currentBitmap, pxlDigit + 1); }
  # endif // if P165_DEBUG_DEBUG
  return currentBitmap;
}

/********************************************************************
 * Skip periods as they are shown on previous digit
 *******************************************************************/
int P165_data_struct::getEffectiveTextLength(const String& text) {
  const int textLength = text.length();
  int p                = 0;

  for (int i = 0; i < textLength; ++i) {
    if (_periods && (text.charAt(i) == '.')) { // If setting periods true
      if (p == 0) {                            // Text starts with a period, becomes a space with a dot
        p++;
      }

      if ((i > 0) && (text.charAt(i - 1) == '.')) { // Handle consecutive periods
        p++;
      }
    } else {
      p++;
    }
  }
  return p;
}

/**************************************************************************************
 * Scroll current buffer with text/binary data to display (copied/adjusted from P073)
 *************************************************************************************/
bool P165_data_struct::nextScroll() {
  bool result = false;

  if (isScrollEnabled() && (!_textToScroll.isEmpty())) {
    if ((_scrollCount > 0) && (_scrollCount < 0xFFFF)) { _scrollCount--; }

    if (_scrollCount == 0) {
      _scrollCount = 0xFFFF; // Max value to avoid interference when scrolling long texts
      result       = true;
      const uint8_t  bufToFill = calculateDisplayDigits();
      const uint16_t txtlength = _textToScroll.length();
      clearBuffer();

      uint8_t p = 0;
      uint8_t mp;

      for (int i = _scrollPos; i < txtlength && p <= bufToFill; ++i) { // p <= bufToFill to allow a period after last digit
        mp = showmap[p];

        if (_periods
            && (_textToScroll.charAt(i) == '.')
            && !_binaryData
            ) {         // If setting periods true
          if (p == 0) { // Text starts with a period, becomes a space with a dot
            showperiods[mp] = true;
            p++;
          } else {
            mp              = showmap[p - 1];
            showperiods[mp] = true;                                       // The period displays as a dot on the previous digit!
          }

          if ((i > _scrollPos) && (_textToScroll.charAt(i - 1) == '.')) { // Handle consecutive periods
            mp              = showmap[p - 1];
            showperiods[mp] = true;                                       // The period displays as a dot on the previous digit!
            p++;
          }
        } else if (p < bufToFill) {
          showbuffer[mp] =
            # if P165_FEATURE_P073
            _binaryData ?
            # endif // if P165_FEATURE_P073
            _textToScroll.charAt(i)
            # if P165_FEATURE_P073
            : P073_mapCharToFontPosition(_textToScroll.charAt(i), _fontset)
            # endif // if P165_FEATURE_P073
          ;
          p++;
        }
      }
      _scrollPos++;

      if (_scrollPos > _textToScroll.length() - bufToFill) {
        _scrollPos = 0;            // Restart when all text displayed
      }
      _scrollCount = _scrollSpeed; // Restart countdown
      # if P165_DEBUG_DEBUG
      logBufferContent(F("nextScroll"));
      # endif // if P165_DEBUG_DEBUG
    }
  }
  return result;
}

/***********************************************************************************
 * Set up the string to scroll across the display, with optional prefixed spaces
 **********************************************************************************/
void P165_data_struct::setTextToScroll(const String& text) {
  _textToScroll = String();

  if (!text.isEmpty()) {
    const int bufToFill = calculateDisplayDigits();
    _textToScroll.reserve(text.length() + bufToFill + (_scrollFull ? bufToFill : 0));

    for (int i = 0; _scrollFull && i < bufToFill; ++i) { // Scroll text in from the right, so start with all blancs
      _textToScroll += _binaryData ? (char)0x00 : ' ';
    }
    _textToScroll += text;

    for (int i = 0; i < bufToFill; ++i) { // Scroll text off completely before restarting
      _textToScroll += _binaryData ? (char)0x00 : ' ';
    }
  }
  _scrollCount = _scrollSpeed;
  _scrollPos   = 0;
  _binaryData  = false;
}

void P165_data_struct::setScrollSpeed(uint8_t speed) {
  _scrollSpeed = speed;
  _scrollCount = _scrollSpeed;
  _scrollPos   = 0;
}

bool P165_data_struct::isScrollEnabled() {
  return _txtScrolling && _scrollAllowed;
}

void P165_data_struct::setScrollEnabled(bool scroll) {
  _scrollAllowed = scroll;
}

# if P165_DEBUG_INFO || P165_DEBUG_DEBUG

/**********************************************************************
 * Log the current buffer content with a prefix
 *********************************************************************/
void P165_data_struct::logBufferContent(String prefix) {
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      log.reserve(26 + 4 * _totalDigits)) {
    log = strformat(F("%s buffer: periods: %c"), prefix.c_str(), _periods ? 't' : 'f');

    for (uint8_t i = 0; i < _totalDigits; i++) {
      log += strformat(F("%c0x%X,%c%d(%d)"), i > 0 ? ',' : ' ', showbuffer[i], showperiods[i] ? '.' : ' ', i, showmap[i]);
    }
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

# endif // if P165_DEBUG_INFO || P165_DEBUG_DEBUG

#endif // ifdef USES_P165
