#include <iostream>

#include "contur/cpu.h"
#include "contur/dispatcher.h"
#include "contur/kernel.h"
#include "contur/memory.h"
#include "contur/mmu.h"
#include "contur/scheduler.h"
#include "demos/demos.h"

int main()
{
    int SIZE_OF_MEMORY_IN_BLOCKS = 100; // размер оперативной памяти в блоках
    int MAX_PROCESS = 3;                // максимальное количество процессов

    Memory memory(SIZE_OF_MEMORY_IN_BLOCKS);
    MMU mmu(memory);

    auto cpu = std::make_unique<CPU>(memory, MAX_PROCESS, std::make_unique<LAN>(), std::make_unique<Device>());

    Scheduler scheduler(*cpu, MAX_PROCESS);
    Dispatcher dispatcher(MAX_PROCESS, scheduler, mmu);
    Kernel kernel(dispatcher);

    DemoContext ctx(memory, mmu, *cpu, scheduler, dispatcher, kernel, MAX_PROCESS);

    bool running = true;
    while (running) {
        char ch;
        std::cout << "\ndialog mode:\n"
                     "1. Computer architecture. execute program 1,2.................enter 1\n"
                     "2. OS architecture. MultiTasking execute programm 1 and 2.....enter 2\n"
                     "3. Model process. Create Process for programm 1 and 2.........enter 3\n"
                     "4. Synchronize process. CriticalSection for programm 1 and 2..enter 4\n"
                     "5. Main memory and Management Memory Unit.....................enter 5\n"
                     "6. Virtual memory and process control block - PCB.............enter 6\n"
                     "7. Scheduling for one processor system. Methods...............enter 7\n"
                     "8. MultiProcessor scheduling. Methods.........................enter 8\n"
                     "0. Exit.......................................................enter 0"
                  << '\n';
        std::cin >> ch;

        switch (ch) {
        case '1':
            demoArchitecture(ctx);
            break;
        case '2':
            demoMultitasking(ctx);
            break;
        case '3':
            demoProcess(ctx);
            break;
        case '4':
            demoSynchronization(ctx);
            break;
        case '5':
            demoMMU(ctx);
            break;
        case '6':
            demoVirtualMemory(ctx);
            break;
        case '7':
            demoScheduling(ctx);
            break;
        case '8':
            demoMultiprocessor(ctx);
            break;
        default:
            running = false;
            break;
        }
    }

    return 0;
}
