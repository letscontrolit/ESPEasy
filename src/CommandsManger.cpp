#include "CommandsManager.h"

bool CommandsManager::Register(const char * id, ExecuteCommandCall command)
{
    bool result = false;
    auto item =  CommandsManager::commandsRegistry.find(id);
    if(item == CommandsManager::commandsRegistry.end())
    {
        CommandsManager::commandsRegistry[id] = command;
    }    
    return result;
}

bool  CommandsManager::Execute(const char * id, struct EventStruct *event,  const char* line)
{
    bool result = false;
    ExecuteCommandCall command = CommandsManager::commandsRegistry[id];
    if(command)
    {
        result = command(event, line);
    }
    return result;
}