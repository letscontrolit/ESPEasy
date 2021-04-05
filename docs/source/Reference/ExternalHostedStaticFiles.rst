External Hosted Static Files
****************************

Since build mega-20201227, most of the static files are no longer included in those builds that have been optimized to create smaller bin sizes.

Until this change for linking to external CSS/JS files, all these static files were included in the build.
Meaning the build size was larger due to files which your browser could also fetch from another site.

Builds relying on external hosted files, link to a CDN URL of GitHub, which hosts copies of our repository.
Meaning your browser will fetch them from the GitHub servers when needed.
However the "TTL" of those values (the time they can be safely cached by your browser) is set to 1 year.


So what does this al mean?
--------------------------

If you operate your units linking to an external hosted CSS or JS file, your browser must be able to reach the GitHub servers.
But this only has to be done once and after that the browser will cache those files for 1 year (or until you clear your cache).
This means you may need to have an internet connection on the device you use to access the ESPEasy node if those files are not in your browser cache.

There is a way to overcome this limitation, by saving the files using the same names onto your SPIFFS file system of the node.
If the files are present on the local file system, the ESP will serve them to you.

The files are located `here <https://github.com/letscontrolit/ESPEasy/tree/mega/static>`_

These are the file names used to check if the file exists on the local file system.
They are sorted with the most important files on top.

* ``espeasy_default.css``  Without this file the web UI does look rather unusable and horrible.
* ``rules_save.js`` JavaScript to actually perform saving the rules. Rules cannot be saved without this file.
* ``update_sensor_values_device_page.js`` JavaScript to perform value updates of tasks on the "Devices" tab.
* ``fetch_and_parse_log.js`` JavaScript to fetch new logs on the web based log viewer.
* ``reboot.js`` JavaScript to check whether it is best to perform a reboot of the ESP node.
* ``toasting.js`` Showing an acknowledgement toasting message when submitting settings.
* ``github_clipboard.js`` JavaScript to copy only the most interesting values on the sysinfo page to paste on GitHub.
* ``favicon.ico`` The icon shown on the tab of your browser


When storing a local copy of one of these files, make sure you're using the same name as presented here.

.. note::

  When storing files on a node with 1M flash, be aware the small file system size may cause issues, so only store those files which are really needed.
  You can also try to minify the CSS or JS files to make them even smaller.
