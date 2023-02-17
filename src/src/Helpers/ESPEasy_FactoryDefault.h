#ifndef HELPERS_ESPEASY_FACTORYDEFAULT_H
#define HELPERS_ESPEASY_FACTORYDEFAULT_H


/********************************************************************************************\
   Reset all settings to factory defaults
 \*********************************************************************************************/
void ResetFactory(bool formatFS = true);

/*********************************************************************************************\
   Collect the stored preference for factory default
\*********************************************************************************************/
void applyFactoryDefaultPref();


#endif