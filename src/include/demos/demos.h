#pragma once

#include <memory>

class Memory;
class MMU;
class CPU;
class Scheduler;
class Dispatcher;
class Kernel;

class DemoContext
{
  public:
    DemoContext(
        Memory &memory, MMU &mmu, CPU &cpu, Scheduler &scheduler, Dispatcher &dispatcher, Kernel &kernel, int maxProcess
    );
    ~DemoContext();

    DemoContext(const DemoContext &) = delete;
    DemoContext &operator=(const DemoContext &) = delete;
    DemoContext(DemoContext &&) noexcept;
    DemoContext &operator=(DemoContext &&) noexcept;

    Memory &memory();
    MMU &mmu();
    CPU &cpu();
    Scheduler &scheduler();
    Dispatcher &dispatcher();
    Kernel &kernel();
    int maxProcess() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

void demoArchitecture(DemoContext &ctx);
void demoMultitasking(DemoContext &ctx);
void demoProcess(DemoContext &ctx);
void demoSynchronization(DemoContext &ctx);
void demoMMU(DemoContext &ctx);
void demoVirtualMemory(DemoContext &ctx);
void demoScheduling(DemoContext &ctx);
void demoMultiprocessor(DemoContext &ctx);
