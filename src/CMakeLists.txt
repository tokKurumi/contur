cmake_minimum_required(VERSION 3.10)

project(contur LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(contur
    main.cpp
    contur/architecture_cpu.cpp
    contur/cpu.cpp
    contur/device.cpp
    contur/dispatcher.cpp
    contur/handle.cpp
    contur/kernel.cpp
    contur/lan.cpp
    contur/memory.cpp
    contur/mmu.cpp
    contur/mp_dispatcher.cpp
    contur/process.cpp
    contur/scheduler.cpp
    contur/statistic.cpp
    contur/virtual_memory.cpp
)

target_include_directories(contur PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
