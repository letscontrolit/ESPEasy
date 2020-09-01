.. include:: ../Plugin/_plugin_substitutions_p08x.repl
.. _P086_page:

|P086_typename|
==================================================

|P086_shortinfo|

Plugin details
--------------

Type: |P086_type|

Name: |P086_name|

Status: |P086_status|

GitHub: |P086_github|_

Maintainer: |P086_maintainer|

Introduction
------------

This generic Device work together with the Homie controller (C014) :ref:`C014_page` and helps to handle incoming data. According to the `Homie convention <https://homieiot.github.io/>`_ every value can be announced as "setable" when the ``$setable`` flag is true.

ESPEasy initially designed as a sensor firmware receive data via commands. If you need to send data from your controller to a device you have to prepare the command in your controller software and send it to ESPEasy via ``cmd=`` or trigger rules by ``event=`` and send the command from there. To send values directly this plug-in can be used. It "only" defines the necessary parameters and the Homie controller sends the auto-discover information to the MQTT broker. To translate and direct this information to one or several commands a rule is used.

Setup
-----

The :ref:`C014_page` must be installed and enabled. Multiple instances of the plug-in are possible. One instance of the plug-in can define 4 different values / events.

.. include:: P086_settings.repl

After reboot the controller should receive auto-discover information. Depending on the controller software used correct settings should be selected by default.

Define a rule for every value defined. Every rule should end with the ``HomieValueSet`` command to acknowledge the value back to the controller. The value must not necessarily be the same as the sent value (i.e. after extra limit checks). Numeric values are stored in the device. Due to RAM limitations String values are not saved.
For some controllers or dataset types it is not necessary to acknowledge the new value. They simply assume that the value arrived and processed correctly. Sometimes it could be useful not to acknowledge the value i.e. for slides sending updates frequently. The value can already be renewed when the acknowledgement arrives causing undesirable results or only to process values faster. For Switches it is essential to acknowledge the set state to keep the user aware of the real state and not the assumed state.

Example Rules for every value / event type
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

    // The "homie controller" schedules the event defined in the plug-in.
    // Does NOT store string values or arrays in the "homie receiver" plug-in.
    // Use homieValueSet to save numeric value in the plugin.
    // This will also acknowledge the value back to the controller to get out of "predicted" state.
    // Adjust device and event number according to your configuration

    On eventInteger do
     // insert your code here
     homieValueSet,1,1,%eventvalue1%
    endon

    On eventFloat do
     // insert your code here
     homieValueSet,1,2,%eventvalue1%
    endon

    On eventBoolean do
     // insert your code here
     homieValueSet,1,3,%eventvalue1% // true or false (0/1 is stored in uservar)
    endon

    On eventString do
     // insert your code here
     homieValueSet,1,4,"%eventvalue%" // quotation requited for Strings!
    endon

    On eventEnum do
     // insert your code here
     homieValueSet,2,1,%eventvalue2% // name of picked item (number in %eventvalue1% and stored in uservar)
    endon

    On eventRGB do
     // insert your code here
     NeoPixelAll,%eventvalue1%,%eventvalue2%,%eventvalue3% // example for NeoPixel
     homieValueSet,2,2,%eventvalue1%,%eventvalue2%,%eventvalue3%
    endon

    On eventHSV do
     // insert your code here
     NeoPixelAllHSV,%eventvalue1%,%eventvalue2%,%eventvalue3% // example for NeoPixel
     homieValueSet,2,3,%eventvalue1%,%eventvalue2%,%eventvalue3%
    endon

Examples (coming soon)
----------------------

- A sprinkler system switches 8 valves via relays. The relays are connected by PCF8574 port expander.
- Color LED using NeoPixel RGB & RGBW (WS2812B or SK6812)

Under the hood
--------------

The Homie convention defines several value types and some parameters for every value to enable the controller software to send standardized values to Homie enabled devices. Therefore the device has to send ``$datatype`` and ``$format`` attributes to describe the value type, format and range. This is all done for you by the :ref:`C014_page` according to the settings in this plug-in.

When a message arrives to the ``.../%deviceName%/%eventName%/set`` topic an event is triggered. From there commands can be used to perform certain actions. The parameters are passed by %eventvalue% variables:

- %eventvalue1% holds the numeric values for ``float`` and ``integer``
- %eventvalue1% holds the string value for ``string`` datatypes
- %eventvalue1% holds the number of the selected item of a enumerating list where %eventvalue2% holds the item name
- %eventvalue1%, %eventvalue2%, %eventvalue3% holds the intensity values of rgb or hsv ``color`` datatypes

Use the command ``homieValueSet,deviceNr,eventNr,value`` similar to ``TaskValueSet`` or ``DummieValueSet`` to acknowledge the received or new value back to the controller.

Datatypes (``$datatype`` defaults to ``string``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include:: P086_datatype_attributes.repl

format (``$format`` required for color values)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include:: P086_format_attributes.repl


.. More pictures
.. -------------
