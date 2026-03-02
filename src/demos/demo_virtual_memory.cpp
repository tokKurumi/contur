#include "demos/demos.h"

#include "contur/kernel.h"

#include <iostream>

void demoVirtualMemory(DemoContext &ctx)
{
    char ch;

    std::cout << '\n';
    std::cout << "Virtual memory Y/N-any" << '\n';
    /* пример иллюстрирует выполнение основных функций
    Virtual memory и process control block — PCB, Process Image */
    auto code = std::make_shared<Code>(3);
    code->setCmd(Mov, r1, 23);
    // запись содержимого r1 в память по адресу 70
    code->setCmd(Wmem, r1, 70);
    code->setCmd(Int, 0, Exit);
    std::cin >> ch;
    if (ch == 'Y') {
        // 1. состояние виртуальной памяти в образах
        std::cout << "1. Virtual Memory state" << '\n';
        ctx.dispatcher().DebugVirtualMemory(); // структура виртуальной памяти
        // 2. выделение виртуальной памяти в образах
        std::cout << "2. Create process" << '\n';
        Handle *handle = ctx.kernel().CreateProcess("P1", code); // создать процесс
        ctx.dispatcher().DebugVirtualMemory();
        // изменяет состояние системы и выполнение программы
        std::cout << "3. Process execute" << '\n';
        ctx.dispatcher().dispatch();
        ctx.dispatcher().dispatch();
        std::cout << "4. Terminate process" << '\n';
        ctx.kernel().TerminateProcess(*handle); // завершить процесс
        // 3. освобождение виртуальной памяти в образах
        ctx.dispatcher().DebugVirtualMemory();
        // 4. очистить основную память
        ctx.memory().clearMemory();
    }

    std::cout << '\n';
    std::cout << "dump PCB and process image Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        std::cout << "1. Create process" << '\n';
        Handle *handle = ctx.kernel().CreateProcess("P1", code); // создать процесс
        // 1. состояние блока PCB
        ctx.dispatcher().DebugPCB(*handle);
        std::cout << "2. Process execute" << '\n';
        ctx.dispatcher().dispatch(); // изменяет состояние системы 2 вызова
        // 2. состояние блока PCB после выполнения программы.
        // см. состояние PC
        ctx.dispatcher().DebugPCB(*handle);
        ctx.dispatcher().dispatch();
        std::cout << "3. Terminate process" << '\n';
        ctx.kernel().TerminateProcess(*handle); // завершить процесс
        // 3. состояние блока PCB после удаления программы из системы
        ctx.dispatcher().DebugPCB(*handle);
        ctx.memory().clearMemory();
    }
}
