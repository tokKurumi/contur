#include "demos/demos.h"

#include "contur/kernel.h"

#include <iostream>

void demoScheduling(DemoContext &ctx)
{
    char ch;
    Handle *handle_1;
    Handle *handle_2;

    /* 1. для программы "Pl" и "P2" создаются объекты процессы
    сравнение методов планирования
    Tenter — Время входа
    Tbegin — Время начала
    Tservice — Время обслуживания в очередях
    Texec — Время выполнения
    Tterminate — Время завершения
    Tround = Tservice + Texec — Время оборота
    Tround/Tservice — Нормализованное время */

    auto code_1 = std::make_shared<Code>(12);
    // перемещение содержимого второго операнда в первый(регистр)
    code_1->setCmd(Mov, r1, 6);
    code_1->setCmd(Mov, r2, 7);
    code_1->setCmd(Mul, r1, r2); // регистр * регистр
    code_1->setCmd(Add, r1, 3);
    code_1->setCmd(Mov, r2, 5);
    code_1->setCmd(Div, r1, r2);
    code_1->setCmd(Sub, r1, 4);
    code_1->setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
    code_1->setCmd(Rmem, r3, 69);
    code_1->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
    code_1->setCmd(Int, r3, Lan); // посыл содержимого r1 по сети
    code_1->setCmd(Int, 0, Exit);

    auto code_2 = std::make_shared<Code>(9);
    code_2->setCmd(Mov, r1, 9);
    code_2->setCmd(Mov, r2, 12);
    code_2->setCmd(Mul, r1, r2);
    code_2->setCmd(Add, r1, 2);
    code_2->setCmd(Wmem, r1, 70);
    code_2->setCmd(Rmem, r3, 70);
    code_2->setCmd(Int, r1, Dev);
    code_2->setCmd(Int, r3, Lan);
    code_2->setCmd(Int, 0, Exit);

    std::cout << '\n';
    std::cout << "scheduler FCFS Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        Timer::setTime();                                    // устанавливаем таймер в нулевое значение
        handle_1 = ctx.kernel().CreateProcess("P1", code_1); // создать процесс
        handle_2 = ctx.kernel().CreateProcess("P2", code_2); // создать процесс
        ctx.dispatcher().dispatch(); // изменяет состояние системы для модели из трех очередей 2 вызова
        ctx.dispatcher().dispatch();
        handle_1->ProcessTime();
        handle_2->ProcessTime();
        ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
        ctx.kernel().TerminateProcess(*handle_2); // завершить процесс
        /* очереди должны быть пусты после эксперимента
        scheduler.DebugSPNQueue(); */
        ctx.memory().clearMemory();
    }

    std::cout << '\n';
    std::cout << "scheduler RR Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        int slice = 5;
        Timer::setTime(); // устанавливаем таймер в нулевое значение
        // в зависимости от размера программы и квоты, изменяется число строк dispatcher.dispatch();
        ctx.dispatcher().setTimeSlice(slice);
        handle_1 = ctx.kernel().CreateProcess("P1", code_1); // создать процесс
        handle_2 = ctx.kernel().CreateProcess("P2", code_2); // создать процесс
        int disp = 4;                                        // подобрано по диаграмме slice и организации очередей
        for (int i = 0; i < disp; i++) {
            ctx.dispatcher().dispatch(); // изменяет состояние системы
        }

        handle_1->ProcessTime();
        handle_2->ProcessTime();
        ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
        ctx.kernel().TerminateProcess(*handle_2); // завершить процесс
        ctx.memory().clearMemory();
        // очереди должны быть пусты после эксперимента scheduler.DebugSPNQueue();
        ctx.dispatcher().resetTimeSlice();
    }

    std::cout << '\n';
    std::cout << "scheduler SPN (shortest process next) Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        int numProcess = 5; // количество объектов одного процесса для расчета его предсказуемого времени выполнения
        Timer::setTime();   // устанавливаем таймер в нулевое значение
        // включить регистрацию наблюдений
        ctx.dispatcher().setTpredict(TimeExecNotPreem);
        /* для невытесняющего метода
        alfa = 0.8; установлен взвешенный коэффициент 0 < alfa < 1
        в ShortestProcessNext(), открыт отладчик */
        for (int i = 0; i < numProcess; i++) {
            std::cout << '\n';
            std::cout << "--------------------experiment i = " << i << '\n';
            handle_1 = ctx.kernel().CreateProcess("user_1", code_1);
            handle_2 = ctx.kernel().CreateProcess("user_2", code_2);
            ctx.dispatcher().dispatch(); // изменяет состояние системы
            ctx.dispatcher().dispatch();
            if (i < numProcess - 1) {
                // -1, для сохранения информации о последнем эксперименте
                ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
                ctx.kernel().TerminateProcess(*handle_2); // завершить процесс
            }

            // наблюдения сохраняются, только после завершения процесса для первого процесса нет наблюдений
            ctx.memory().clearMemory();
        }

        handle_1->ProcessTime();
        handle_2->ProcessTime();
        ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
        ctx.kernel().TerminateProcess(*handle_2); // завершить процесс
        // scheduler.DebugSPNQueue();
        ctx.dispatcher().resetTpredict(); // выключить регистрацию наблюдений
    }

    std::cout << '\n';
    std::cout << "scheduler SRT (shortest remaining time) Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        // представляет собой вытесняющую версию SPN
        int numProcess = 3; // количество объектов одного процесса для расчета его предсказуемого времени выполнения
        Timer::setTime();   // устанавливаем таймер в нулевое значение
        // включить регистрацию наблюдений процессов
        ctx.dispatcher().setTpredict(TimeExec);
        // и вытеснение
        for (int i = 0; i < numProcess; i++) {
            std::cout << '\n';
            std::cout << "----------------------experiment i = " << i << '\n';
            // параметр 0 включает наименьшее оставшееся время процесса
            handle_1 = ctx.kernel().CreateProcess("user_1", code_1);
            handle_2 = ctx.kernel().CreateProcess("user_2", code_2);
            ctx.dispatcher().dispatch(); // изменяет состояние системы
            ctx.dispatcher().dispatch(); // вытеснение user_1 и выполнение
            ctx.dispatcher().dispatch(); // длинный процесс не в swapOut, не успевает
            if (i < numProcess - 1) {
                // -1, для сохранения информации о последнем эксперименте
                ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
                ctx.kernel().TerminateProcess(*handle_2); // завершить процесс
            }
            // наблюдения сохраняются, только после завершения процесса для первого экземпляра процесса нет наблюдений
            ctx.memory().clearMemory();
        }

        handle_1->ProcessTime();
        handle_2->ProcessTime();
        ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
        ctx.kernel().TerminateProcess(*handle_2); // завершить процесс
        // scheduler.DebugSPNQueue();
        ctx.dispatcher().resetTpredict(); // выключить регистрацию наблюдений
    }

    std::cout << '\n';
    std::cout << "scheduler HRRN (hightst response ratio next) Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        // представляет собой вытесняющую версию SPN
        int numProcess = 3; // количество объектов одного процесса для расчета его предсказуемого времени выполнения
        Timer::setTime();   // устанавливаем таймер в нулевое значение
        // включить регистрацию наблюдений процессов
        ctx.dispatcher().setTpredict(TimeServ);
        // в ShortestProcessNext(), открыт отладчик
        for (int i = 0; i < numProcess; i++) {
            std::cout << '\n';
            std::cout << "--------------------experiment i = " << i << '\n';
            // параметр 0, включает наименьшее оставшееся время процесса
            handle_1 = ctx.kernel().CreateProcess("user_1", code_1);
            handle_2 = ctx.kernel().CreateProcess("user_2", code_2);
            ctx.dispatcher().dispatch(); // изменяет состояние системы
            ctx.dispatcher().dispatch(); // вытеснение user_1 и выполнение
            ctx.dispatcher().dispatch(); // процесс swapOut
            if (i < numProcess - 1) {
                // -1, для сохранения информации о последнем эксперименте
                ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
                ctx.kernel().TerminateProcess(*handle_2); // завершить процесс
                // наблюдения сохраняются, только после завершения процесса для первого экземпляра процесса нет
                // наблюдений
            }

            ctx.memory().clearMemory();
        }

        handle_1->ProcessTime();
        handle_2->ProcessTime();
        ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
        ctx.kernel().TerminateProcess(*handle_2);
        // scheduler.DebugSPNQueue();
        ctx.dispatcher().resetTpredict(); // выключить регистрацию наблюдений
    }

    std::cout << '\n';
    std::cout << "scheduler dynamic priority DP Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        // priority 0..3 индекс массива. TimeSlice — значение элемента в массиве
        auto prSlice = std::make_unique<int[]>(3);
        prSlice[0] = 5;
        prSlice[1] = 3;
        prSlice[2] = 2;
        Timer::setTime(); // устанавливаем таймер в нулевое значение
        // в зависимости от размера программы и TimeSlice изменяется число строк dispatcher.dispatch();
        ctx.dispatcher().setTimeSlice(3, std::move(prSlice));
        handle_1 = ctx.kernel().CreateProcess("P1", code_1); // создать процесс
        ctx.dispatcher().dispatch();                         // изменяет состояние системы
        handle_2 = ctx.kernel().CreateProcess("P2", code_2); // создать процесс
        int disp = 5;
        for (int i = 0; i < disp; i++) {
            ctx.dispatcher().dispatch(); // изменяет состояние системы
        }

        handle_1->ProcessTime();
        handle_2->ProcessTime();
        ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
        ctx.kernel().TerminateProcess(*handle_2); // завершить процесс
        ctx.memory().clearMemory();
        // scheduler.DebugSPNQueue();
        ctx.dispatcher().resetTimeSlice();
    }
}
