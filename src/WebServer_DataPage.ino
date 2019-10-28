//********************************************************************************
// Web Interface data page
//********************************************************************************
//19480 (11128)

void handle_data() {
  checkRAM(F("handle_data"));
  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_DATA;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  byte page = getFormItemInt(F("page"), 0);

  if (page == 0) {
    page = 1;
  }
  byte setpage = getFormItemInt(F("setpage"), 0);

  if (setpage > 0)
  {
    if (setpage <= (TASKS_MAX / TASKS_PER_PAGE)) {
      page = setpage;
    }
    else {
      page = TASKS_MAX / TASKS_PER_PAGE;
    }
  }

  // taskIndex in the URL is 1 ... TASKS_MAX
  // For use in other functions, set it to 0 ... (TASKS_MAX - 1)
  byte taskIndex = getFormItemInt(F("index"), 0);
  boolean taskIndexNotSet = taskIndex == 0;
  --taskIndex;

  byte DeviceIndex = 0;
  LoadTaskSettings(taskIndex); // Make sure ExtraTaskSettings are up-to-date
  // FIXME TD-er: Might have to clear any caches here.

  // show all tasks as table
  if (taskIndexNotSet)
  {
    html_add_script(true);
    TXBuffer += DATA_UPDATE_SENSOR_VALUES_DEVICE_PAGE_JS;
    html_add_script_end();
    html_table_class_multirow();
    html_TR();

    if (TASKS_MAX != TASKS_PER_PAGE)
    {
      html_add_button_prefix();
      TXBuffer += F("data?setpage=");
    	if (page > 1) {
      	TXBuffer += page - 1;
    	}
    	else {
      	TXBuffer += page;
    	}
      TXBuffer += F("'>&lt;</a>");
      html_add_button_prefix();
      TXBuffer += F("data?setpage=");

    	if (page < (TASKS_MAX / TASKS_PER_PAGE)) {
      	TXBuffer += page + 1;
    	}
    	else {
      	TXBuffer += page;
    	}
    	TXBuffer += F("'>&gt;</a>");
  	}

    html_table_header(F("Name"),70);
    html_table_header(F("Values"));

    for (byte x = (page - 1) * TASKS_PER_PAGE; x < ((page) * TASKS_PER_PAGE); x++)
    {
      if (Settings.TaskDeviceNumber[x] != 0)
      {
        LoadTaskSettings(x);
        DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
				if ((Settings.TaskDeviceEnabled[x]) && (Device[DeviceIndex].ValueCount>0))
				{
	        struct EventStruct TempEvent;
        	byte customValues = false;
	        TempEvent.TaskIndex = x;
	        html_TR_TD();
        	TXBuffer += ExtraTaskSettings.TaskDeviceName;
        	html_TD();

	        String dummy;
        	customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent,dummy);
        	if (!customValues)
        	{
          	for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
          	{
            	if (Settings.TaskDeviceNumber[x] != 0)
            	{
              	if (varNr > 0)
                	TXBuffer += F("<div class='div_br'></div>");
              	TXBuffer += F("<div class='div_l' id='valuename_");
              	TXBuffer  += x;
              	TXBuffer  += '_';
              	TXBuffer  += varNr;
              	TXBuffer  += "'>";
              	TXBuffer += ExtraTaskSettings.TaskDeviceValueNames[varNr];
              	TXBuffer += F(":</div><div class='div_r' id='value_");
              	TXBuffer  += x;
              	TXBuffer  += '_';
              	TXBuffer  += varNr;
              	TXBuffer  += "'>";
              	TXBuffer += formatUserVarNoCheck(x, varNr);
              	TXBuffer += "</div>";
            	}
          	}
        	}
				}
      }
    } // next
    html_end_table();
    html_end_form();

  }

  checkRAM(F("handle_data"));
#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = F("DEBUG: String size:");
    log += String(TXBuffer.sentBytes);
    addLog(LOG_LEVEL_DEBUG_DEV, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}
