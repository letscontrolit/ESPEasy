/*
	To modify the stock configuration without changing the EspEasy.ino file :

	1) rename this file to "Custom.h" (It is ignored by Git)
	2) define your own settings below 
    3) define USE_CUSTOM_H as a build flags. ie : export PLATFORMIO_BUILD_FLAGS="'-DUSE_CUSTOM_H'" 	
*/



// make the compiler show a warning to confirm that this file is inlcuded
#warning "**** Using Settings from Custom.h File ***"

/*

#######################################################################################################
Your Own Default Settings
#######################################################################################################

	You can basically ovveride ALL macro defined in ESPEasy.ino.
	Don't forget to first #undef each existing #define that you add below.
	Here are some examples:
*/


#undef	DEFAULT_NAME
#define DEFAULT_NAME		"MyEspEasyDevice"			// Enter your device friendly name

#undef	DEFAULT_SSID
#define DEFAULT_SSID		"MyHomeSSID"			   // Enter your network SSID

#undef	DEFAULT_KEY
#define DEFAULT_KEY			"MySuperSecretPassword"		// Enter your network WPA key

#undef	DEFAULT_AP_KEY
#define DEFAULT_AP_KEY		"MyOwnConfigPassword"		// Enter network WPA key for AP (config) mode

#ifdef BUILD_GIT
#undef BUILD_GIT
#endif

#define BUILD_GIT "MyBuildNum"
