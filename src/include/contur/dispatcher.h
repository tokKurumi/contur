#pragma once

#include "contur/architecture_cpu.h"
#include "contur/mmu.h"
#include "contur/process.h"
#include "contur/scheduler.h"
#include "contur/virtual_memory.h"
#include <memory>
#include <string>

class Dispatcher
{
  public:
    Dispatcher(int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, Scheduler &scheduler, MMU &mmu);

    ProcessImage *getVirtualMemory();
    void freeVirtualMemory(Handle &handle);
    void initProcessID(ProcessImage &processImage, std::shared_ptr<Memory> code);

    virtual void dispatch();
    virtual void scheduleProcess(MMU &mmu, bool priority);
    virtual Interrupt executeProcess(MMU &mmu);

    Handle &getHandleCS();
    void EnterCriticalSection(Handle &handle);
    void LeaveCriticalSection(Handle &handle);

    void setTimeSlice(int timeSlice);
    void resetTimeSlice();
    void setTimeSlice(int size, std::unique_ptr<int[]> prSlice);

    void setTpredict(StatTime time);
    void resetTpredict();
    double getTpredict(const std::string &user, StatTime time);

    void DebugQueue(State state);
    void DebugVirtualMemory();
    void DebugPCB(Handle &handle);

    class Priorityslice
    {
      public:
        Priorityslice();
        void setPrioritySlice(int size, std::unique_ptr<int[]> prSlice);
        bool getPrioritySlice();
        int getTimeSlice(int priority);
        int getPriority(int priority);

      private:
        int size;
        std::unique_ptr<int[]> prSlice;
    };

  protected:
    std::unique_ptr<VirtualMemory> virtualMemory;
    Scheduler &scheduler;
    MMU &mmu;
    std::unique_ptr<CS> cs;
    int timeSlice;
    Priorityslice prSlice;
};
