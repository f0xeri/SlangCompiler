#include "llvm/Support/InitLLVM.h"
#include "driver/Driver.hpp"

int main(int argc, char **argv) {
    InitLLVM init_llvm(argc, argv);
    Slangc::Driver driver(argc, argv);
    driver.run();
    return 0;
}
