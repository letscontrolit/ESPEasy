# Contributing to ESPEasy

:+1::tada: First off, thanks for taking the time to contribute! :tada::+1:

## Issue tracker

* The issue tracker is NOT a support forum. For support questions go to our forum: https://www.letscontrolit.com/forum/viewforum.php?f=1

* Only post an issue if you're 100% sure you've hit a bug. If your not sure, post on the forum first to see if anyone can confirm it.

This way we keep the issue tracker as short as possible, which saves our developers a lot of time.

Also there are more people on the forum that can help you with support.

## Pull requests

 * Make sure your code compiles.

 * Make sure your code does not have warnings. (Warnings will be treated as errors by our Travis system.)

 * Try to create different pull requests for different features/fixes.

 * Dont combine a lot of different stuff in one huge pull request.

 * The v2.0 branch should get only fixes, new stuff should go into the mega-branch. (We will merge those fixes from v2.0 to mega)

 * Incomplete or unstable plugins should have a PLUGIN_BUILD_DEV #ifdef around them. Also add [DEVELOPMENT] to the name.

 * New plugins that seem to be working correctly should have a PLUGIN_BUILD_TESTING around until they are tested enough.

 * Also see our general guidelines at: https://www.letscontrolit.com/wiki/index.php/ESPEasyDevelopmentGuidelines
