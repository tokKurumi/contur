#include "demos/demos.h"

#include "contur/cpu.h"
#include "contur/dispatcher.h"
#include "contur/kernel.h"
#include "contur/memory.h"
#include "contur/mmu.h"
#include "contur/scheduler.h"

struct DemoContext::Impl {
    Memory &memory;
    MMU &mmu;
    CPU &cpu;
    Scheduler &scheduler;
    Dispatcher &dispatcher;
    Kernel &kernel;
    int maxProcess;
};

DemoContext::DemoContext(
    Memory &memory, MMU &mmu, CPU &cpu, Scheduler &scheduler, Dispatcher &dispatcher, Kernel &kernel, int maxProcess
)
    : impl(std::make_unique<Impl>(Impl{memory, mmu, cpu, scheduler, dispatcher, kernel, maxProcess}))
{
}

DemoContext::~DemoContext() = default;
DemoContext::DemoContext(DemoContext &&) noexcept = default;
DemoContext &DemoContext::operator=(DemoContext &&) noexcept = default;

Memory &DemoContext::memory()
{
    return impl->memory;
}
MMU &DemoContext::mmu()
{
    return impl->mmu;
}
CPU &DemoContext::cpu()
{
    return impl->cpu;
}
Scheduler &DemoContext::scheduler()
{
    return impl->scheduler;
}
Dispatcher &DemoContext::dispatcher()
{
    return impl->dispatcher;
}
Kernel &DemoContext::kernel()
{
    return impl->kernel;
}
int DemoContext::maxProcess() const
{
    return impl->maxProcess;
}
