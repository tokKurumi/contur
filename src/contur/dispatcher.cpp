#include "contur/dispatcher.h"
#include <iostream>

Dispatcher::Dispatcher(int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, Scheduler &scheduler, MMU &mmu)
    : scheduler(scheduler), mmu(mmu), timeSlice(-1)
{
    virtualMemory = std::make_unique<VirtualMemory>(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES);
    cs = std::make_unique<CS>();
}

ProcessImage *Dispatcher::getVirtualMemory()
{
    int addr = virtualMemory->getAddrFreeImage();
    if (addr == -1) {
        return nullptr;
    }
    virtualMemory->setAddrImage(addr);
    ProcessImage &processImage = virtualMemory->getImage();
    processImage.setVirtualAddr(addr);
    processImage.setTservice(processImage.getTservice() + 1);
    return &processImage;
}

void Dispatcher::freeVirtualMemory(Handle &handle)
{
    ProcessImage &processImage = static_cast<ProcessImage &>(handle);
    State state = processImage.getState();
    int ID = processImage.getID();
    scheduler.getSysReg(ID).setStatus(true);
    scheduler.getSysReg(ID).clearSysReg();
    processImage.resetSysReg();
    scheduler.pop(state, processImage);

    if (scheduler.vParam.getStateTime() != noParam) {
        scheduler.setObservation(processImage);
        if (scheduler.vParam.getState(TimeExecNotPreem)) {
            scheduler.setTthreshold(TimeExec);
        }
        if (scheduler.vParam.getState(TimeExec)) {
            scheduler.setTthreshold(TimeExec);
        }
        if (scheduler.vParam.getState(TimeServ)) {
            scheduler.setTthreshold(TimeServ);
        }
    }

    virtualMemory->clearImage(static_cast<PCB &>(handle).getVirtualAddr());
}

void Dispatcher::initProcessID(ProcessImage &processImage, std::shared_ptr<Memory> code)
{
    int ID = scheduler.getID();
    if (ID >= 0) {
        processImage.setID(ID);
    } else {
        std::cout << "init Process ID > MAX_PROCESS in system" << '\n';
    }

    scheduler.getSysReg(ID).setStatus(false);
    processImage.setSysReg(scheduler.getSysReg(ID));
    processImage.setState(NotRunning);
    processImage.setCode(std::move(code));
    processImage.setTenter(Timer::getTime());
    processImage.setTservice(0);
    scheduler.push(NotRunning, processImage);
}

void Dispatcher::dispatch()
{
    ProcessImage *processImage = nullptr;
    Timer::Tick();

    int size_ = scheduler.size(Running);
    for (int i = 0; i < size_; i++) {
        processImage = static_cast<ProcessImage *>(scheduler.front(Running));
        processImage->setTservice(processImage->getTservice() + 1);
        if (processImage->getState() == ExitProcess) {
            scheduler.pop(Running);
            scheduler.push(ExitProcess, *processImage);
        }
    }

    size_ = scheduler.size(Blocked);
    for (int i = 0; i < size_; i++) {
        processImage = static_cast<ProcessImage *>(scheduler.front(Blocked));
        processImage->setTservice(processImage->getTservice() + 1);
        if (!cs->getCS()) {
            scheduler.pop(Blocked);
            processImage->setState(Running);
            processImage->setFlag(1);
            scheduler.push(ExitProcess, *processImage);
            cs->setCS(true);
            break;
        }
    }

    size_ = scheduler.size(NotRunning);
    for (int i = 0; i < size_; i++) {
        processImage = static_cast<ProcessImage *>(scheduler.front(NotRunning));
        processImage->setTservice(processImage->getTservice() + 1);

        if (processImage->getSysReg() == nullptr) {
            SysReg &sysReg = scheduler.getSysReg(scheduler.getID());
            processImage->setSysReg(sysReg);
        }

        scheduler.pop(NotRunning);

        if (processImage->getFlag() == 1) {
            processImage->setState(Running);
            processImage->setTimeSlice(this->timeSlice);

            if (prSlice.getPrioritySlice()) {
                int pr = processImage->getPriority();
                processImage->setTimeSlice(prSlice.getTimeSlice(pr));
                processImage->setPriority(prSlice.getPriority(pr));
            }

            scheduler.push(Running, *processImage);
        } else {
            processImage->setState(Blocked);
            scheduler.push(Blocked, *processImage);
        }
    }

    bool priority = prSlice.getPrioritySlice();
    if (priority) {
        size_ = scheduler.size(Swapped);
        for (int i = 0; i < size_; i++) {
            processImage = static_cast<ProcessImage *>(scheduler.front(Swapped));
            int pr = processImage->getPriority();
            processImage->setTimeSlice(prSlice.getTimeSlice(pr));
            processImage->setPriority(prSlice.getPriority(pr));
            scheduler.pop(Swapped);
            scheduler.push(Swapped, *processImage);
        }
    }

    if (scheduler.empty(Running) && scheduler.empty(Swapped)) {
        std::cout << "dispatch: empty Running && Swapped" << '\n';
        return;
    }

    scheduleProcess(mmu, priority);
    Interrupt interrupt = executeProcess(mmu);
}

void Dispatcher::scheduleProcess(MMU &mmu, bool priority)
{
    bool maxProcess = scheduler.scheduleJob(mmu, priority);
}

Interrupt Dispatcher::executeProcess(MMU &mmu)
{
    return scheduler.execute(mmu);
}

Handle &Dispatcher::getHandleCS()
{
    return static_cast<Handle &>(*cs);
}

void Dispatcher::EnterCriticalSection(Handle &handle)
{
    if (!cs->getCS()) {
        static_cast<ProcessImage &>(handle).setFlag(1);
        cs->setCS(true);
    } else {
        static_cast<ProcessImage &>(handle).setFlag(0);
    }
}

void Dispatcher::LeaveCriticalSection(Handle &handle)
{
    cs->setCS(false);
}

void Dispatcher::setTimeSlice(int timeSlice)
{
    this->timeSlice = timeSlice;
}

void Dispatcher::resetTimeSlice()
{
    this->timeSlice = -1;
    this->prSlice.setPrioritySlice(-1, nullptr);
}

void Dispatcher::setTimeSlice(int size, std::unique_ptr<int[]> prSlice)
{
    this->prSlice.setPrioritySlice(size, std::move(prSlice));
}

void Dispatcher::setTpredict(StatTime time)
{
    scheduler.vParam.setState(time);
}

void Dispatcher::resetTpredict()
{
    scheduler.vParam.clearVectorParam();
}

double Dispatcher::getTpredict(const std::string &user, StatTime time)
{
    return scheduler.getTpredict(user, time);
}

void Dispatcher::DebugQueue(State state)
{
    scheduler.DebugQueue(state);
}

void Dispatcher::DebugVirtualMemory()
{
    this->virtualMemory->Debug();
}

void Dispatcher::DebugPCB(Handle &handle)
{
    handle.ProcessTime();
    static_cast<ProcessImage &>(handle).Debug();
}

// Priorityslice

Dispatcher::Priorityslice::Priorityslice() : size(-1), prSlice(nullptr)
{
}

void Dispatcher::Priorityslice::setPrioritySlice(int size, std::unique_ptr<int[]> prSlice)
{
    this->size = size;
    this->prSlice = std::move(prSlice);
}

bool Dispatcher::Priorityslice::getPrioritySlice()
{
    if (prSlice == nullptr) {
        return false;
    }
    return true;
}

int Dispatcher::Priorityslice::getTimeSlice(int priority)
{
    if (size == -1) {
        return -1;
    }
    if (priority == -1) {
        return prSlice[0];
    }
    if (priority == size - 1) {
        return prSlice[size - 1];
    }
    return prSlice[priority + 1];
}

int Dispatcher::Priorityslice::getPriority(int priority)
{
    if (priority == -1) {
        return 0;
    }
    if (priority == size - 1) {
        return size - 1;
    }
    return priority + 1;
}
