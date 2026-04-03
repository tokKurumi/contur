/// @file main.cpp
/// @brief Contur 2 — OS Kernel Simulator interactive TUI entry point.
///
/// Builds a demo kernel with several processes and launches the FTXUI-backed
/// interactive visualiser. The simulation starts paused; press [Space] to
/// begin autoplay or [t] to advance one tick manually.

#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

#include "contur/core/clock.h"

#include "contur/arch/block.h"
#include "contur/arch/instruction.h"
#include "contur/arch/interrupt.h"
#include "contur/arch/register_file.h"
#include "contur/cpu/cpu.h"
#include "contur/dispatch/dispatcher.h"
#include "contur/execution/interpreter_engine.h"
#include "contur/fs/simple_fs.h"
#include "contur/ipc/ipc_manager.h"
#include "contur/kernel/i_kernel.h"
#include "contur/kernel/kernel_builder.h"
#include "contur/kernel/kernel_diagnostics.h"
#include "contur/memory/fifo_replacement.h"
#include "contur/memory/mmu.h"
#include "contur/memory/physical_memory.h"
#include "contur/memory/virtual_memory.h"
#include "contur/scheduling/round_robin_policy.h"
#include "contur/scheduling/scheduler.h"
#include "contur/syscall/syscall_table.h"
#include "contur/tracing/null_tracer.h"
#include "contur/tui/ftxui_app.h"
#include "contur/tui/i_kernel_read_model.h"
#include "contur/tui/i_tui_controller.h"

using namespace contur;

static std::vector<Block> makeProgramAddOnePlusOne()
{
    return {
        {Instruction::Mov, 0, 1, 0},
        {Instruction::Add, 0, 1, 0},
        {Instruction::Halt, 0, 0, 0},
    };
}

static std::vector<Block> makeProgramCounterLoop()
{
    return {
        {Instruction::Mov, 0, 0, 0},
        {Instruction::Add, 0, 1, 0},
        {Instruction::Compare, 0, 16, 0},
        {Instruction::JumpLess, 0, 1, 0},
        {Instruction::Halt, 0, 0, 0},
    };
}

static std::vector<Block> makeProgramCpuHeavy()
{
    return {
        {Instruction::Mov, 1, 2, 0},
        {Instruction::Mov, 2, 3, 0},
        {Instruction::Mul, 1, 2, 1},
        {Instruction::Add, 1, 7, 0},
        {Instruction::Sub, 1, 1, 0},
        {Instruction::Compare, 1, 20, 0},
        {Instruction::JumpLess, 0, 2, 0},
        {Instruction::Halt, 0, 0, 0},
    };
}

static std::vector<Block> makeProgramLongNop(std::size_t nops)
{
    std::vector<Block> code;
    code.reserve(nops + 1);
    for (std::size_t i = 0; i < nops; ++i)
    {
        code.push_back({Instruction::Nop, 0, 0, 0});
    }
    code.push_back({Instruction::Halt, 0, 0, 0});
    return code;
}

static Result<std::unique_ptr<IKernel>> buildDemoKernel()
{
    auto clock = std::make_unique<SimulationClock>();
    auto tracer = std::make_unique<NullTracer>(*clock);
    auto memory = std::make_unique<PhysicalMemory>(64);
    auto replacement = std::make_unique<FifoReplacement>();
    auto mmu = std::make_unique<Mmu>(*memory, std::move(replacement), *tracer);
    auto virtualMem = std::make_unique<VirtualMemory>(*mmu, 1024);
    auto cpu = std::make_unique<Cpu>(*memory);
    auto engine = std::make_unique<InterpreterEngine>(*cpu, *memory);
    auto policy = std::make_unique<RoundRobinPolicy>(4);
    auto scheduler = std::make_unique<Scheduler>(std::move(policy), *tracer);
    auto dispatcher = std::make_unique<Dispatcher>(*scheduler, *engine, *virtualMem, *clock, *tracer);
    auto fs = std::make_unique<SimpleFS>(128);
    auto ipc = std::make_unique<IpcManager>();
    auto syscallTable = std::make_unique<SyscallTable>();

    return KernelBuilder{}
        .withClock(std::move(clock))
        .withMemory(std::move(memory))
        .withMmu(std::move(mmu))
        .withVirtualMemory(std::move(virtualMem))
        .withCpu(std::move(cpu))
        .withExecutionEngine(std::move(engine))
        .withScheduler(std::move(scheduler))
        .withDispatcher(std::move(dispatcher))
        .withTracer(std::move(tracer))
        .withFileSystem(std::move(fs))
        .withIpcManager(std::move(ipc))
        .withSyscallTable(std::move(syscallTable))
        .withDefaultTickBudget(1)
        .build();
}

static void spawnDemoProcesses(IKernel &kernel)
{
    struct Demo
    {
        const char *name;
        PriorityLevel pri;
        std::int32_t nice;
        std::vector<Block> code;
    };

    const std::vector<Demo> demos = {
        {"calc-1-plus-1", PriorityLevel::Realtime, 0, makeProgramAddOnePlusOne()},
        {"counter", PriorityLevel::High, 0, makeProgramCounterLoop()},
        {"worker-1", PriorityLevel::Normal, 0, makeProgramCpuHeavy()},
        {"worker-2", PriorityLevel::Normal, 5, makeProgramLongNop(24)},
        {"background", PriorityLevel::Low, 10, makeProgramLongNop(48)},
        {"idle-task", PriorityLevel::Idle, 19, makeProgramLongNop(96)},
    };

    for (const auto &d : demos)
    {
        ProcessConfig cfg;
        cfg.name = d.name;
        cfg.priority = Priority{d.pri, d.pri, d.nice};
        cfg.code = d.code;
        (void)kernel.createProcess(cfg);
    }
}

int main()
{
    // Build kernel
    auto kernelResult = buildDemoKernel();
    if (kernelResult.isError())
    {
        std::cerr << "Failed to build kernel\n";
        return 1;
    }

    auto kernel = std::move(kernelResult).value();

    // Spawn demo processes
    spawnDemoProcesses(*kernel);

    // Wire up TUI stack
    KernelDiagnostics diagnostics(*kernel);
    KernelReadModel readModel(diagnostics);

    TuiController controller(readModel, [&kernel](std::size_t step) { return kernel->runForTicks(step); }, 512);

    // Launch interactive app
    FtxuiApp app(
        controller,
        FtxuiAppConfig{
            .defaultIntervalMs = 300,
            .defaultStep = 1,
            .frameIntervalMs = 33,
            .minIntervalMs = 50,
            .maxIntervalMs = 2000,
        }
    );

    app.run();

    return 0;
}
