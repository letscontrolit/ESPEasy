#ifndef COMMAND_TASKS_H
#define COMMAND_TASKS_H

#include "../Globals/Plugins.h"


const __FlashStringHelper* Command_Task_Clear(struct EventStruct *event,
                                              const char         *Line);
const __FlashStringHelper* Command_Task_ClearAll(struct EventStruct *event,
                                                 const char         *Line);
const __FlashStringHelper* Command_Task_Disable(struct EventStruct *event,
                                                const char         *Line);
const __FlashStringHelper* Command_Task_Enable(struct EventStruct *event,
                                               const char         *Line);
#if FEATURE_PLUGIN_PRIORITY
const __FlashStringHelper* Command_PriorityTask_Disable(struct EventStruct *event,
                                                        const char         *Line);
#endif // if FEATURE_PLUGIN_PRIORITY
const __FlashStringHelper* Command_Task_ValueSet(struct EventStruct *event,
                                                 const char         *Line);
#if FEATURE_STRING_VARIABLES
const __FlashStringHelper* Command_Task_ValueSetDerived(struct EventStruct *event,
                                                        const char         *Line);
const __FlashStringHelper* Command_Task_ValueSetPresentation(struct EventStruct *event,
                                                             const char         *Line);
const __FlashStringHelper* taskValueSetString(struct EventStruct        *event,
                                              const char                *Line,
                                              const __FlashStringHelper *storageTemplate,
                                              const __FlashStringHelper *uomTemplate   = nullptr,
                                              const __FlashStringHelper *vTypeTemplate = nullptr);
#endif // if FEATURE_STRING_VARIABLES
const __FlashStringHelper* Command_Task_ValueToggle(struct EventStruct *event,
                                                    const char         *Line);
const __FlashStringHelper* Command_Task_ValueSetAndRun(struct EventStruct *event,
                                                       const char         *Line);
const __FlashStringHelper* Command_ScheduleTask_Run(struct EventStruct *event,
                                                    const char         *Line);
const __FlashStringHelper* Command_Task_Run(struct EventStruct *event,
                                            const char         *Line);
const __FlashStringHelper* Command_Task_RemoteConfig(struct EventStruct *event,
                                                     const char         *Line);

// bool validTaskVars(struct EventStruct *event, taskIndex_t& taskIndex, unsigned int& varNr);

#endif // ifndef COMMAND_TASKS_H
