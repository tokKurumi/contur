add_executable(contur2 main.cpp)
# Link against contur2_tui (which transitively brings contur2_lib).
# Never link the kernel (contur2_lib) directly here — go through the TUI layer.
target_link_libraries(contur2 PRIVATE contur2_tui contur2_demos)
