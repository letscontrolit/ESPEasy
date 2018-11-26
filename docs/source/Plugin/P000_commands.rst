

.. csv-table::
    :header: "Command", "Class", "Purpose", "Syntax"
    :widths: 8, 6, 15, 15

    "Debug","
    :red:`Internal`","
    Change Serial port debug level","
    ``Debug,<1-4>``"
    "
    IP","
    :red:`Internal`","
    Change IP address","
    ``IP,<IP address>``"
    "
    Name","
    :red:`Internal`","
    Set the name of the unit","
    ``Name,<new name>``"
    "
    Password","
    :red:`Internal`","
    Set the password of the unit","
    ``Password,<new password>``"
    "
    Reboot","
    :red:`Internal`","
    Reboot the ESP","
    ``Reboot``"
    "
    Reset","
    :red:`Internal`","
    Reset config to factory default. Caution, all settings will be lost!","
    ``Reset``"
    "
    ResetFlashWriteCounter","
    :red:`Internal`","
    Reset flash write to zero.","
    ``ResetFlashWriteCounter``"
    "
    Rules","
    :red:`Internal`","
    Rules enabled (1) or rules disabled (0)","
    ``Rules,<1/0>``"
    "
    Save","
    :red:`Internal`","
    Save config to persistent flash memory","
    ``Save``"
    "
    Settings","
    :red:`Internal`","
    Show settings on serial terminal","
    ``Settings``"
    "
    TaskClear","
    :red:`Internal`","
    Delete the given task/device","
    ``TaskClear,<task/device nr>``"
    "
    TaskClearAll","
    :red:`Internal`","
    Delete ALL task/device","
    ``TaskClearAll``"
    "
    TaskRun","
    :red:`Internal`","
    Run/excecute the given task/device, use to manually force an update/read of the task.","
    ``TaskRun,<task/device nr>``"
    "
    Unit","
    :red:`Internal`","
    Set the unit number","
    ``Unit,<unit number>``"
    "
    WifiAPKey","
    :red:`Internal`","
    Change AP WPA key","
    ``WifiAPKey,<WPA key>``"
    "
    WifiAPMode","
    :red:`Internal`","
    Force the unit into AP mode.","
    ``WifiAPMode``"
    "
    WifiConnect","
    :red:`Internal`","
    Connect to configured wireless network","
    ``WifiConnect``"
    "
    WifiDisconnect","
    :red:`Internal`","
    Disconnect from wireless network","
    ``WifiDisconnect``"
    "
    WifiKey","
    :red:`Internal`","
    Change WPA key for primary WiFi","
    ``WifiKey,<Wifi WPA key>``"
    "
    WifiKey2","
    :red:`Internal`","
    Change WPA key for secondary WiFi","
    ``WifiKey2,<Wifi WPA key>``"
    "
    WifiScan","
    :red:`Internal`","
    Scan Wireless networks","
    ``WifiScan``"
    "
    WifiSSID","
    :red:`Internal`","
    Change SSID to connect as primary WiFi","
    ``WifiSSID,<SSID>``"
    "
    WifiSSID2","
    :red:`Internal`","
    Change SSID to connect as secondry WiFi","
    ``WifiSSID2,<SSID>``"
    "
    Delay","
    :green:`Rules`","
    Delay rule processing","
    ``Delay,<delay in milliSeconds>``"
    "
    Publish","
    :green:`Rules`","
    Send command using MQTT broker service","
    ``Publish,<topic>,<value>``"
    "
    SendTo","
    :green:`Rules`","
    Send command to other ESP (using UDP)","
    ``SendTo,<unit nr>,<command>``"
    "
    SendToHTTP","
    :green:`Rules`","
    Send command to other network device using HTTP

    ``SendToHTTP,temperatur.nu,80,/rapportera.php?hash=123abc456&t=[temp2#out]``","
    ``SendToHTTP,<IP address>,<Portnumber>,<command>``

    ``SendToHTTP,<domain>,<Portnumber>,</url>``"
    "
    SendToUDP","
    :green:`Rules`","
    Send command to other network device using UDP (non-ESP Easy units)","
    ``SendToUDP,<IP address>,<Portnumber>,<command>``"
    "
    TaskValueSet","
    :green:`Rules`","
    Set values on a **Dummy Task** (device)","
    ``TaskValueSet,<task/device nr>,<value nr>,<value/formula>``"
    "
    TimerSet","
    :green:`Rules`","
    Start a timed event	","
    ``TimerSet,<timernr>,<timeInSeconds>``

    ``TimerSet,<timernr>,0`` disables the timer"
    "
    Event","
    :blue:`Special`","
    Create an event, it's possible to send a float value along as well.","
    See event syntax below..."


Event command
-------------

The event command is a special command used to trigger an event. This event can then be acted upon from the rules.
You can send 0..4 event values along with the event.


.. csv-table::
    :header: "Command", "Rules example"
    :widths: 10, 15

    "
    ``Event,SingleEvent``
    ","

    .. code-block:: html

      on SingleEvent do
        Publish,%sysname%/Info,A single event has been triggered!
      endon


    The event (triggered by any of the launch ways) will make the unit publish a message.
    "
    "
    ``Event,SingleEventValue=123``
    ","

    .. code-block:: html

      on SingleEventValue do
        Publish,%sysname%/Info,An event has been sent (%eventvalue%)!
      endon


    The event value ``123`` is intercepted and published.
    "
    "
    ``Event,MultipleEventValues=123,456,789,999999``
    ","

    .. code-block:: html

      on MultipleEventValues do
        if %eventvalue4%=9999
          Publish,%sysname%/Info,Lets compute %eventvalue1%+%eventvalue2%=[%eventvalue1%+%eventvalue2%]
        else
          Publish,%sysname%/Info,Lets compute %eventvalue3%-%eventvalue2%=[%eventvalue3%-%eventvalue2%]
        endif
      endon


    The event value `9999`` (4) is intercepted and the rule ``Publish, ... %eventvalue1%+%eventvalue2%=[%eventvalue1%+%eventvalue2%]`` is
    triggered, output payload to MQTT would then be ``Lets compute 123+456=579``
    "
