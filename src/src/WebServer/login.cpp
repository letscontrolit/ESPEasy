#include "../WebServer/Login.h"


#include "../WebServer/WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"

#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

#include "../../ESPEasy-Globals.h"




// ********************************************************************************
// Web Interface login page
// ********************************************************************************
void handle_login() {
  checkRAM(F("handle_login"));

  if (!clientIPallowed()) { return; }
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  String webrequest = web_server.arg(F("password"));
  addHtml(F("<form method='post'>"));
  html_table_class_normal();
  addHtml(F("<TR><TD>Password<TD>"));
  addHtml(F("<input class='wide' type='password' name='password' value='"));
  addHtml(webrequest);
  addHtml("'>");
  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_TR_TD();
  html_end_table();
  html_end_form();

  if (webrequest.length() != 0)
  {
    char command[80];
    command[0] = 0;
    webrequest.toCharArray(command, 80);

    // compare with stored password and set timer if there's a match
    if ((strcasecmp(command, SecuritySettings.Password) == 0) || (SecuritySettings.Password[0] == 0))
    {
      WebLoggedIn      = true;
      WebLoggedInTimer = 0;
      addHtml(F("<script>window.location = '.'</script>"));
    }
    else
    {
      addHtml(F("Invalid password!"));

      if (Settings.UseRules)
      {
        String event = F("Login#Failed");

        // TD-er: Do not add to the eventQueue, but execute right now.
        rulesProcessing(event);
      }
    }
  }

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb     = false;
}
