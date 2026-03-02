#pragma once

#include "contur/dispatcher.h"
#include "contur/scheduler.h"
#include <memory>
#include <string>

class Kernel
{
  private:
    Dispatcher &dispatcher;

  public:
    Kernel(Dispatcher &dispatcher);

    Handle *CreateProcess(const std::string &user, std::shared_ptr<Memory> code);
    void TerminateProcess(Handle &handle);

    Handle &CreateCriticalSection();
    void EnterCriticalSection(Handle &handle);
    void LeaveCriticalSection(Handle &handle);

    void DebugProcessImage(Handle &handle);
};
