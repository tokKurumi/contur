#pragma once

#include "contur/architecture_cpu.h"
#include "contur/dispatcher.h"
#include "contur/process.h"
#include "contur/scheduler.h"
#include "contur/virtual_memory.h"
#include <cmath>
#include <valarray>

class MPDispatcher : public Dispatcher
{
  public:
    MPDispatcher(int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, Scheduler &scheduler, MMU &mmu, int MAX_PROCESSOR);

    void scheduleProcess(MMU &mmu, bool priority);
    Interrupt executeProcess(MMU &mmu);
    void MPDebug();

  private:
    int MAX_PROCESSOR;
    std::vector<std::unique_ptr<Scheduler>> vaScheduler;
    std::valarray<bool> vaStatus;
};
