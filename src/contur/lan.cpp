#include "contur/lan.h"
#include <iostream>

void LAN::sendData(int data)
{
    std::cout << "LAN send: data = " << data << '\n';
}
