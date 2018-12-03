.. include:: ../Plugin/_plugin_substitutions_p00x.repl
.. _P001_Switch_page:

Switch
======

|P001_typename|
|P001_status|

.. image:: P001_Switch_1.jpg

Introduction
------------

Any switch module can be used as an digital input. In everyday language the switch is often referred to as a "button".
You may use it by simply connect it to either a GPIO that is normally high (pull-up) or normally low (pull-down). Other
types of switches are door switches (reed switch), sound activated switches and pulse activated switches.

Specifications:
 * Give a high or low signal to a GPIO

Wiring
------


.. code-block:: html

  ESP               Switch/button
  GPIO (X)   <-->   Signal

  Power
  3.3V       <-->   Signal
              or
  GND        <-->   Signal


Setup
-----



Rules examples
--------------

.. code-block:: html

    on Button#State=1 do
      timerSet,1,1
    endon

    on rules#timer=1 do
     if [Button#State]=0
      //Action if button is short pressedbuild
     else
      //Action if button is still pressed
     endif
    endon


Indicators (recommended settings)
---------------------------------

.. csv-table::
  :header: "Indicator", "Value Name", "Interval", "Decimals", "Extra information"
  :widths: 8, 5, 5, 5, 40

  "XXXXXX", "N/A", "", "", ""

Where to buy
------------

.. csv-table::
  :header: "Store", "Link"
  :widths: 5, 40

  "Toggle switch","`AliExpress 1 ($) <http://s.click.aliexpress.com/e/fY7OgXO>`_"
  "Float switch","`AliExpress 2 ($) <http://s.click.aliexpress.com/e/clw6aFEY>`_"
  "Membrane switch","`AliExpress 3 ($) <http://s.click.aliexpress.com/e/ZkI1ov6>`_"
  "Capacitive switch","`AliExpress 4 ($) <http://s.click.aliexpress.com/e/cMIem1Nq>`_"
  "Limit switch","`AliExpress 5 ($) <http://s.click.aliexpress.com/e/bWOekFqo>`_"
  "Momentary switch","`AliExpress 6 ($) <http://s.click.aliexpress.com/e/bPEYQDfw>`_ `Banggood 1 ($) <https://www.banggood.com/420pcs-14-Types-Momentary-Tact-Tactile-Push-Button-Switch-SMD-Assortment-Kit-Set-p-1154674.html?p=V3270422659778201806>`_ `Banggood 2 ($) <https://www.banggood.com/100-Pcs-Micro-Switch-Tact-Cap-Slim-Cap-Tactile-Push-Button-Switch-Momentary-Tact-p-1277707.html?p=V3270422659778201806>`_ `Banggood 3 ($) <https://www.banggood.com/36V-2A-12mm-Momentary-Push-Button-Switch-LED-Switch-Waterproof-Switch-p-994547.html?p=V3270422659778201806>`_ `eBay ($) <https://rover.ebay.com/rover/1/711-53200-19255-0/1?icep_id=114&ipn=icep&toolid=20004&campid=5338336929&mpre=https%3A%2F%2Fwww.ebay.com%2Fsch%2Fi.html%3F_from%3DR40%26_trksid%3Dm570.l1313%26_nkw%3DMomentary%2Bswitch%26_sacat%3D0%26LH_TitleDesc%3D0%26_osacat%3D0%26_odkw%3DLimit%2Bswitch>`_"


|affiliate|


More pictures
-------------
