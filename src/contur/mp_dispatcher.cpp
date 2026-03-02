#include "contur/mp_dispatcher.h"
#include <cmath>
#include <iostream>

MPDispatcher::MPDispatcher(int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, Scheduler &scheduler, MMU &mmu, int MAX_PROCESSOR)
    : Dispatcher(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, scheduler, mmu), MAX_PROCESSOR(MAX_PROCESSOR)
{
    vaScheduler.resize(MAX_PROCESSOR);
    vaStatus = std::valarray<bool>(MAX_PROCESSOR);
    for (int i = 0; i < MAX_PROCESSOR; i++) {
        this->vaScheduler[i] = std::make_unique<Scheduler>(scheduler);
        this->vaStatus[i] = true;
    }
}

void MPDispatcher::scheduleProcess(MMU &mmu, bool priority)
{
    std::cout << " *** scheduleProcess - overriding functions *** " << '\n';
    int quotaProcess = static_cast<int>(round(static_cast<double>(scheduler.getProcess()) / MAX_PROCESSOR));
    scheduler.setQuotaProcess(quotaProcess);
    bool maxProcess = false;
    for (int i = 0; i < MAX_PROCESSOR; i++) {
        maxProcess = scheduler.scheduleJob(mmu, priority);
        vaScheduler[i]->setJob(scheduler.getJob());
        vaStatus[i] = false;
        if (!maxProcess) {
            break;
        }
    }
    MPDebug();
    mmu.DebugMemory();
}

Interrupt MPDispatcher::executeProcess(MMU &mmu)
{
    std::cout << " *** executeProcess - overriding functions *** " << '\n';
    Interrupt interrupt;
    for (int i = 0; i < MAX_PROCESSOR; i++) {
        if (!vaStatus[i]) {
            interrupt = vaScheduler[i]->execute(mmu);
            vaStatus[i] = true;
        }
    }
    return interrupt;
}

void MPDispatcher::MPDebug()
{
    for (int i = 0; i < MAX_PROCESSOR; i++) {
        std::cout << "---Processor---" << i << " state = " << vaStatus[i] << '\n';
    }
}
