Migrate from SPIFFS to LittleFS (ESP32)
=======================================

Since Espressif IDF 5.x framework (ESP32 chips only), the SPIFFS file-system is no longer supported. As ESPEasy is using a specially crafted framework-release that still included SPIFFS support, we have been able to keep support for SPIFFS, but this requires quite some effort. To reduce that effort, and because SPIFFS won't be made available after IDF 5.1.x for ESPEasy (mega-20250430), it is strongly advised to migrate to a LittleFS build.

NB: A build can not have support for both SPIFFS *and* LittleFS. (Technical limitation.)

Using the 4 steps below, a safe (and easy) migration path is available:

1) Upgrade to the last release with SPIFFS support
--------------------------------------------------

For the best experience, and to ensure a stable running system, the last release of ESPEasy **still including SPIFFS** should be used. This can be downloaded from `Github Release mega-20250430 <https://github.com/letscontrolit/ESPEasy/releases/tag/mega-20250430>`_

.. note:: **The last release that includes ESP32 wth SPIFFS support is** `mega-20250430 <https://github.com/letscontrolit/ESPEasy/releases/tag/mega-20250430>`_

To ensure an uninterrupted upgrade, the ESPEasy P2P network feature should be **disabled** by setting the **ESPEasy p2p UDP port** to ``0`` on the Tools/Advanced page (take note of the port number before setting it to 0), and save that setting. This prevents the sometimes busy, high-priority, P2P traffic to interrupt the download process. After the migration is completed this can of course be re-enabled.

After saving the P2P port setting, the ESP must be rebooted to take the unit out of the P2P network.

In the previous paragraph is shown how to select the correct binary for upgrading. After a successful upgrade, the ESP will be rebooted, and should be left running for *at least* 5 minutes to ensure that the WiFi configuration is stored in the NVS (non-volatile storage) partiton of the flash (this is a background process, activated ~5 minutes after a successful connection to WiFI), to facilitate that these settings can be used to reconnect to WiFi after upgrading to a LittleFS build.

2) Backup the current configuration and file-system content
-----------------------------------------------------------

As all ESP32 builds include support for creating a .tar archive backup, this feature should now be used to create an up-to-date system backup, by using the :cyan:`Backup files` button (as opposed to the :cyan:`Save` button that only stores the configuration files), and storing the archive on a persistent storage medium (local disk, USB stick, network storage, etc.).

NB: Depending on the browser used, it may be needed to confirm that the created archive should *really* be stored, so don't forget to confirm that!

3) Upgrade to the matching LittleFS release
-------------------------------------------

After the backup is created, the matching release with LittleFS support can be downloaded from the `Github Releases page <https://github.com/letscontrolit/ESPEasy/releases>`_ (If your build is not (yet) included, then please request it via a support issue. If your build was without the _ETH suffix, then you should pick the build *with* _ETH, as non-Ethernet builds are discontinued to reduce the number of builds.)

As long as the flash partitioning is the same (``4M316k``, ``8M1M`` etc.) the upgrade can be done via OTA. If the partitioning is different, either the `ESPEasy Web Flash tool <https://td-er.nl/ESPEasy/>`_ or the Espressif Flash Download tool should be used to install the update (flash partitioning can not be changed via an OTA update).

Once the new binary is installed, and the ESP rebooted, the unit should automatically connect to WiFi, as it has the last used credentials stored in the NVS partition. Might that fail, the initial setup via the Setup portal should be started.

4) Restore the backup
---------------------

Now that the unit is available via WiFi again, the backup we created in step **2)** can be restored so alle previously configured devices, controllers, rules, settings, etc. are available again.

From the Tools page, select the :cyan:`Load` button, and browse to the location of the backup .tar file to select it. After the restore is finished, the unit **must** be rebooted immediately (a :cyan:`Reboot` button is available), so the freshly restored settings won't be overwritten by any settings kept in memory.

After that mandatory reboot it is the moment to re-enable the P2P network, by restoring the **ESPEasy p2p UDP port** setting, on the Tools/Advanced page, with the port number noted in step **1)**. The default P2P port is ``8266``.

This requires another reboot to activate the P2P network again.

|

When these steps are completed, the migration from SPIFFS to LittleFS is **successfully completed!**

|
