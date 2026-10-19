#pragma once

/********************************************************************************************\
   ESPEasy will start disabling components when a bootloop is detected.
   These will only be disabled in memory, the disabled state will not be saved.
   So whenever an ESPEasy node suddenly doesn't have some task or controller disabled, 
   the disabled component is likely causing the node to crash.
   This can be some programming error, a configuration error 
   or maybe a bad device returning invalid data.
 \*********************************************************************************************/


void disableFailingComponent();