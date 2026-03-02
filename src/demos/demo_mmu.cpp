#include "demos/demos.h"

#include "contur/kernel.h"
#include "contur/mmu.h"

#include <iostream>

void demoMMU(DemoContext &ctx)
{
    char ch;

    std::cout << '\n';
    std::cout << "Management Memory Unit dump Y/N-any" << '\n';
    /* пример программы показывает выполнение основных
    функций MMU — Management Memory Unit */
    auto code = std::make_shared<Code>(3);
    code->setCmd(Mov, r1, 23);
    code->setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
    code->setCmd(Int, 0, Exit);

    std::cin >> ch;
    if (ch == 'Y') {
        std::cout << "1.Create process using Management Memory Unit" << '\n';
        Handle *handle = ctx.kernel().CreateProcess("P1", code); // создать процесс
        ctx.mmu().Debug(*handle);                                // dump MMU — Management Memory Unit

        std::cout << "2. Execute process" << '\n';
        ctx.dispatcher().dispatch(); // изменяет состояние системы 2 вызова

        std::cout << "3. Terminate process" << '\n';
        ctx.kernel().TerminateProcess(*handle); // завершить процесс
        // dump Terminate
        ctx.memory().clearMemory();
    }

    std::cout << "Main memory and MMU Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        std::cout << "1. Create process, load in main memory, execute" << '\n';
        Handle *handle = ctx.kernel().CreateProcess("P1", code); // создать процесс
        ctx.dispatcher().dispatch();                             // изменяет состояние системы 2 вызова
        ctx.memory().debugHeap(); // программа удаляется из памяти; данные расположенные по адресу 70
        ctx.dispatcher().dispatch();

        std::cout << "2. Terminate process" << '\n';
        ctx.kernel().TerminateProcess(*handle); // завершить процесс

        std::cout << "3. Clear all memory" << '\n';
        ctx.memory().clearMemory();
        ctx.memory().debugHeap();
    }
}
