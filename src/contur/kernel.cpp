#include "contur/kernel.h"

Kernel::Kernel(Dispatcher &dispatcher) : dispatcher(dispatcher)
{
}

Handle *Kernel::CreateProcess(const std::string &user, std::shared_ptr<Memory> code)
{
    ProcessImage *processImage = dispatcher.getVirtualMemory();
    if (processImage == nullptr) {
        return nullptr;
    }
    processImage->setUser(user);
    dispatcher.initProcessID(*processImage, std::move(code));
    return static_cast<Handle *>(processImage);
}

void Kernel::TerminateProcess(Handle &handle)
{
    dispatcher.freeVirtualMemory(handle);
}

Handle &Kernel::CreateCriticalSection()
{
    return dispatcher.getHandleCS();
}

void Kernel::EnterCriticalSection(Handle &handle)
{
    dispatcher.EnterCriticalSection(handle);
}

void Kernel::LeaveCriticalSection(Handle &handle)
{
    dispatcher.LeaveCriticalSection(handle);
}

void Kernel::DebugProcessImage(Handle &handle)
{
    static_cast<ProcessImage &>(handle).Debug();
}
