About Us
********
The ESP Easy project is being handled by the core team of TD-er and Grovkillen.
TD-er being the head of backend and Grovkillen being head of the frontend.
We both dig down into eachothers areas of responsible on a daily basis though.

Other than us we have around 100 voluntears that helping us maintain
and improve the source code.

History
=======
ESP Easy has gone through a lot of changes over the last years and here's some
of the major milestones.

Early days (Nodo Uno)
---------------------
It's a bit vague how it all started but Paul Tonkes (part of the Nodo team)
built a IR (infrared) to Kaku controller because he was not happy with the
commercial version. That unit came to the world named as Nodo (Uno).

Apr 2010 (Nodo Due)
-------------------
Taken from the old Nodo website at the time that the Nodo update was introduced
(named Nodo Due). During this time, a nice production class Arduino Shield was
also built.

Oct 2011 (Joining the Nodo community)
-------------------------------------
The founder of the ESP Easy project, Mvb, read about the Nodo project and
ordered his first Nodo Arduino board from the Nodo shop. He was using it to
control Kaku lights around his house.

Mar 2012 (Nodo Mega)
--------------------
Beta Nodo Mega was released, based on Arduino Mega 2560 board with Ethernet
Shield. Using a hosted web application.

May 2013 (Nodo NES Shield)
--------------------------
The hardware guys from the Nodo team released a shield for Arduino Mega 2560
that contains everything needed to run Nodo Mega with the web application.

Jan 2014 (Exploring new wireless capabilities)
----------------------------------------------
Nodo got its first 2.4GHz communication plugin, using the well known NRF24L01.
This had solved limitations of the 433MHz OOK modules and at the same time it
reduced the load on that frequency so Kaku became more reliable.

Mvb decided to run a setup with two Nodo Mega units in the house. First unit
runs the NES board for 433MHz communication, second board was dedicated for
NRF24L01 communications. Both Mega's could communicate through I2C or Ethernet.

Feb 2015 (Entering the Wifi solution)
-------------------------------------
Nodo got its first ESP Wifi plugin, using it as a serial to Wifi bridge.
At that time we were using the stock firmware that came with the module.

Apr 2015 (ESP Connexio)
-----------------------
The ESP Connexio project was initiated as the first effort to port the original
Nodo code to the ESP platform using early version of the ESP8266 Arduino Core.
The team had to workaround a lot of issues but got it working reasonablely.

May 2015 (ESP Easy initiative)
------------------------------
ESP Easy was initiated, mainly because the Nodo Connexio concept was a bit
over complicated. It had the same eventlist as the original Nodo project.
Quite powerful but also complicated to end users. So the idea was to have
something plug and play to hook up sensors to Domoticz. That descission of
targeting Domoticz is still present in current source code since many plugins
from the early days have the values setup in such a way that Domoticz can
import them with little effort.

Sep 2015 (ESP Easy R020)
------------------------
As of R017 the Nodo Plugin mechanism was also implemented within ESP Easy.
Some other inconvenient bugs were fixed, help buttons added and it was decided
that R020 could be the first production edition.

ESP Easy was launched on Sourceforge as the very first production edition.
No programming required.

Feb 2016 (ESP Easy R078)
------------------------
To make things even more easy, edition R078 was provided as binary images
with a simple installer. No need for a complicated Arduino IDE setup.
And it also introduced OTA, so subsequent updates could be done without
connecting the module to a serial port.

Aug 2016 (ESP Easy R120)
------------------------
Lots of fixes and additional features:

* Rules engine
* Custom dummy device
* GlobalSync
* More commands
* More controllers
* More devices

Estimated user count was at this time around 7 000.

Nov 2016 (ESP Easy Mega)
------------------------
ESP Easy Mega was initiated (only for ESP Modules with at least 1MB flash
memory). We had to many limitations in ESP Easy, due to the code size that
exceeded the 512kB modules flash size. Decided to drop support for these
classic modules and went forward with this version.

Feb 2017 (Change of roles)
--------------------------
Psy0rz took over lead development of ESP Easy Mega (2.0.0), he moved the
source code onto the GitHub page and started to modulerize the code. He also
made ESP Easy support multiple controllers and fallback Wifi. During this time
nightly realeases was introduced and user counts had grown to around 20 000.

Aug 2018 (New core team)
------------------------
With both Mvb and Psy0rz having little time to spend on the project two of the
most eager contributors Grovkillen (mainly wiki documentation and help tools
at that time) and TD-er (had already implemented the event based Wifi and
also introduced chunked webpage) stepped in. The aim for the core team is to
make the ESP Easy OS as great as can be by dedicating full time commitment to
the project.

User count was at this point approximitly around 32 000 and 24 hour download
count around 280.

Nov 2018 (First stable 2+ version)
----------------------------------
The first stable release of the new 2.0.0 version was finally released. Much
of the features added had been in the nightly releases for more than a years
but in the process of modulerizing the source code a lot of bugs were fixed,
much better timing added, but also some new bugs came into the light. The
team wanted to have the official stable release to be just that, stable.

Estimated user count was at this point around 40 000 and 24 hour download count
around 540.
