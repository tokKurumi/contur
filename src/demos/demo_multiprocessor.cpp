#include "demos/demos.h"

#include "contur/kernel.h"
#include "contur/mmu.h"
#include "contur/mp_dispatcher.h"

#include <iostream>
#include <valarray>

void demoMultiprocessor(DemoContext &ctx)
{
    char ch;

    /* сравнение методов планирования многопроцессорной
    системы для программ "Р1" и "Р2" создаются объекты процессы
    Tenter — Время входа
    Tbegin — Время начала
    Tservice — Время обслуживания в очередях
    Texec — Время выполнения
    Tterminate — Время завершения
    Tround = Tservice + Texec — Время оборота
    Tround/Tservice — Нормализованное время */

    auto code = std::make_shared<Code>(9);
    code->setCmd(Mov, r1, 9);
    code->setCmd(Mov, r2, 12);
    code->setCmd(Mul, r1, r2);
    code->setCmd(Add, r1, 2);
    code->setCmd(Wmem, r1, 70);
    code->setCmd(Rmem, r3, 70);
    code->setCmd(Int, r1, Dev);
    code->setCmd(Int, r3, Lan);
    code->setCmd(Int, 0, Exit);

    std::cout << '\n';
    std::cout << "Assignment of Processes to Processors Y/N-any" << '\n';
    /* Multiprocessor scheduling (MP) — многопроцессорное
    планирование. Метод Global queue — планирование очередей */
    int maxProcessor = 4; // максимальное количество процессоров
    Scheduler mpScheduler(ctx.cpu(), ctx.maxProcess());
    MPDispatcher mpDispatcher(ctx.maxProcess(), mpScheduler, ctx.mmu(), maxProcessor);
    Kernel mpKernel(mpDispatcher);
    std::valarray<Handle *> vaHandle(ctx.maxProcess());
    std::cin >> ch;
    if (ch == 'Y') {
        Timer::setTime(); // устанавливаем таймер в нулевое значение
        // 1. Состояние процессоров
        mpDispatcher.MPDebug();
        for (int i = 0; i < ctx.maxProcess(); i++) {
            vaHandle[i] = mpKernel.CreateProcess("P", code); // создать процесс
        }

        mpDispatcher.dispatch(); // изменяет состояние системы
        mpDispatcher.dispatch(); // для модели из трех очередей 2 вызова
        mpDispatcher.dispatch();
        for (int i = 0; i < ctx.maxProcess(); i++) {
            vaHandle[i]->ProcessTime();
            mpKernel.TerminateProcess(*vaHandle[i]); // завершить процесс
        }

        ctx.memory().clearMemory();
    }
}
