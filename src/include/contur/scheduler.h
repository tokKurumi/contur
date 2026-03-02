#pragma once

#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <valarray>
#include <vector>

#include "contur/architecture_cpu.h"
#include "contur/cpu.h"
#include "contur/mmu.h"
#include "contur/process.h"
#include "contur/statistic.h"

class Job
{
  private:
    int addr;
    int id;
    ProcessImage *processImage;
    bool done;

  public:
    Job();

    void set(int addr, int id, bool done);
    void set(int addr, int id, bool done, ProcessImage *processImage);

    void setAddr(int addr)
    {
        this->addr = addr;
    }
    int getAddr() const
    {
        return addr;
    }

    void setId(int id)
    {
        this->id = id;
    }
    int getId() const
    {
        return id;
    }

    void setDone(bool done)
    {
        this->done = done;
    }
    bool getDone() const
    {
        return done;
    }

    void setProcessImage(ProcessImage *processImage)
    {
        this->processImage = processImage;
    }
    ProcessImage *getProcessImage()
    {
        return processImage;
    }

    void Debug();
};

class Scheduler
{
  private:
    CPU &cpu;
    int MAX_PROCESS;
    Job *job;
    std::vector<SysReg> sysreg;
    int sliceCPU;

    std::queue<Process *> processQueue[NUMBER_OF_STATE];
    std::shared_ptr<Statistic> statistic;

    int quotaProcess;
    int j;

    void preemptive(ProcessImage &processImage, MMU &mmu);

  public:
    VectorParam vParam;

    Scheduler(CPU &cpu, int MAX_PROCESS);
    Scheduler(Scheduler &scheduler);

    Interrupt execute(Job *job, MMU &mmu);
    bool isJob(Job *job);
    Interrupt execute(int addr, int id, CPU &cpu);
    bool scheduleJob(MMU &mmu, bool priority);
    Interrupt execute(MMU &mmu);

    void setJob(Job *job)
    {
        this->job = job;
    }
    Job *getJob()
    {
        return this->job;
    }

    void setSliceCPU(int sliceCPU)
    {
        this->sliceCPU = sliceCPU;
    }

    int getID();
    SysReg &getSysReg(int ID)
    {
        return sysreg[ID];
    }

    void DebugBlock(int id, CPU &cpu);
    void DebugSysReg(int id);

    void push(State state, ProcessImage &processImage);
    Process *pop(State state);
    void pop(State state, ProcessImage &processImage);
    Process *front(State state);
    bool empty(State state);
    int size(State state);

    void setObservation(ProcessImage &processImage);
    void clearTpredict();
    double getTpredict(const std::string &user, StatTime time);

    void ProcessNext(StatTime time);
    void sortQueue(State state, StatTime time);
    void setTthreshold(StatTime time);

    void methodPreemptive(ProcessImage &processImage, MMU &mmu, StatTime time);
    void paramPreemptive(ProcessImage &processImage, MMU &mmu);

    int getProcess() const
    {
        return MAX_PROCESS;
    }
    void setQuotaProcess(int quotaProcess)
    {
        this->quotaProcess = quotaProcess;
    }

    void DebugQueue(State state);
    void DebugSPNQueue(State state);
    void DebugSPNQueue();
};
